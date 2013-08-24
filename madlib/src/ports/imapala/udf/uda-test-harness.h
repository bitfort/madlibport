// Copyright 2012 Cloudera Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


#ifndef IMPALA_UDA_TEST_HARNESS_H
#define IMPALA_UDA_TEST_HARNESS_H

#include <string>
#include <sstream>
#include <vector>

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include "udf/udf.h"
#include "udf/udf-debug.h"

namespace impala_udf {

// Utility class to help test UDAs. This can be used to test the correctness of the
// UDA, simulating multiple possible distributed execution setups.
// For example, the harness will run in the UDA in single node mode (so merge and
// serialize are unused), single level merge and multi-level merge.
// The test application is responsible for providing the data and expected result.
class UdaTestHarnessUtil {
 public:
  template<typename T>
  static T CreateIntermediate(UdfContext* context, int byte_size) {
    return T();
  }

  template<typename T>
  static void FreeIntermediate(UdfContext* context, const T& v) {
    // No-op
    return;
  }

  // Copy src value into context, returning the new copy. This simulates
  // copying the bytes between nodes.
  template<typename T>
  static T CopyIntermediate(UdfContext* context, int byte_size, const T& src) {
    return src;
  }
};

template<>
BufferVal UdaTestHarnessUtil::CreateIntermediate(
    UdfContext* context, int byte_size) {
  return reinterpret_cast<BufferVal>(context->Allocate(byte_size));
}

template<>
void UdaTestHarnessUtil::FreeIntermediate(
    UdfContext* context, const BufferVal& v) {
  context->Free(v);
}

template<>
BufferVal UdaTestHarnessUtil::CopyIntermediate(
    UdfContext* context, int byte_size, const BufferVal& src) {
  BufferVal v = reinterpret_cast<BufferVal>(context->Allocate(byte_size));
  memcpy(v, src, byte_size);
  return v;
}

template<typename INPUT, typename INTERMEDIATE, typename RESULT>
class UdaTestHarness {
 public:
  typedef void (*InitFn)(UdfContext* context, INTERMEDIATE* result);

  typedef void (*UpdateFn)(UdfContext* context, const INPUT& input,
      INTERMEDIATE* result);

  typedef void (*MergeFn)(UdfContext* context, const INTERMEDIATE& src,
      INTERMEDIATE* dst);

  typedef const INTERMEDIATE (*SerializeFn)(UdfContext* context,
      const INTERMEDIATE& type);

  typedef RESULT (*FinalizeFn)(UdfContext* context, const INTERMEDIATE& value);

  UdaTestHarness(InitFn init_fn, UpdateFn update_fn, MergeFn merge_fn,
      SerializeFn serialize_fn, FinalizeFn finalize_fn)
    : init_fn_(init_fn),
      update_fn_(update_fn),
      merge_fn_(merge_fn),
      serialize_fn_(serialize_fn),
      finalize_fn_(finalize_fn) {
  }

  // This must be called if the INTERMEDIATE is TYPE_FIXED_BUFFER
  void SetIntermediateSize(int byte_size) {
    fixed_buffer_byte_size_ = byte_size;
  }

  enum ExecutionMode {
    ALL = 0,
    SINGLE_NODE = 1,
    ONE_LEVEL = 2,
    TWO_LEVEL = 3,
  };

  // Runs the UDA in all the modes, validating the result is 'expected' each time.
  bool Execute(const std::vector<INPUT>& values, const RESULT& expected,
      ExecutionMode mode = ALL) {
    std::vector<const INPUT*> ptrs;
    ptrs.resize(values.size());
    for (int i = 0; i < values.size(); ++i) {
      ptrs[i] = &values[i];
    }
    return Execute(ptrs, expected, mode);
  }

  // Runs the UDA in all the modes, validating the result is 'expected' each time.
  template<typename T>
  bool Execute(const std::vector<T>& values, const RESULT& expected,
      ExecutionMode mode = ALL) {
    std::vector<const INPUT*> ptrs;
    ptrs.resize(values.size());
    for (int i = 0; i < values.size(); ++i) {
      ptrs[i] = &values[i];
    }
    return Execute(ptrs, expected, mode);
  }

  // Runs the UDA in all the modes, validating the result is 'expected' each time.
  bool Execute(const std::vector<const INPUT*>& values, const RESULT& expected,
      ExecutionMode mode = ALL) {
    RESULT result;

    if (mode == ALL || mode == SINGLE_NODE) {
      result = ExecuteSingleNode(values);
      if (error_msg_.empty() && result != expected) {
        std::stringstream ss;
        ss << "UDA failed running in single node execution." << std::endl
           << "Expected: " << DebugString(expected)
           << " Actual: " << DebugString(result);
        error_msg_ = ss.str();
      }
      if (!error_msg_.empty()) return false;
    }

    const int num_nodes[] = { 1, 2, 10, 20, 100 };
    if (mode == ALL || mode == ONE_LEVEL) {
      for (int i = 0; i < sizeof(num_nodes) / sizeof(int); ++i) {
        result = ExecuteOneLevel(values, num_nodes[i]);
        if (error_msg_.empty() && result != expected) {
          std::stringstream ss;
          ss << "UDA failed running in one level distributed mode with "
             << num_nodes[i] << " nodes." << std::endl
             << "Expected: " << DebugString(expected)
             << " Actual: " << DebugString(result);
          error_msg_ = ss.str();
          return false;
        }
      }
      if (!error_msg_.empty()) return false;
    }

    if (mode == ALL || mode == TWO_LEVEL) {
      for (int i = 0; i < sizeof(num_nodes) / sizeof(int); ++i) {
        for (int j = 0; j <= i; ++j) {
          result = ExecuteTwoLevel(values, num_nodes[i], num_nodes[j]);
          if (error_msg_.empty() && result != expected) {
            std::stringstream ss;
            ss << "UDA failed running in two level distributed mode with "
              << num_nodes[i] << " nodes in the first level and "
              << num_nodes[j] << " nodes in the second level." << std::endl
              << "Expected: " << DebugString(expected)
              << " Actual: " << DebugString(result);
            error_msg_ = ss.str();
            return false;
          }
        }
      }
      if (!error_msg_.empty()) return false;
    }
    return true;
  }

  // Returns the failure string if any.
  const std::string& GetErrorMsg() const { return error_msg_; }

 private:
  InitFn init_fn_;
  UpdateFn update_fn_;
  MergeFn merge_fn_;
  SerializeFn serialize_fn_;
  FinalizeFn finalize_fn_;

  // Returns false if there is an error set in the context.
  bool CheckContext(UdfContext* context) {
    if (context->has_error()) {
      std::stringstream ss;
      ss << "UDA set error to: " << context->error_msg();
      error_msg_ = ss.str();
      return false;
    }
    return true;
  }

  // Runs the UDA on a single node. The entire execution happens in 1 context.
  // The UDA does a update on all the input values and then a finalize.
  RESULT ExecuteSingleNode(const std::vector<const INPUT*>& values) {
    boost::scoped_ptr<UdfContext> context(UdfContext::CreateTestContext());
    INTERMEDIATE intermediate =
        UdaTestHarnessUtil::CreateIntermediate<INTERMEDIATE>(
            context.get(), fixed_buffer_byte_size_);

    init_fn_(context.get(), &intermediate);
    if (!CheckContext(context.get())) return RESULT::null();

    for (int i = 0; i < values.size(); ++i) {
      update_fn_(context.get(), *values[i], &intermediate);
    }
    if (!CheckContext(context.get())) return RESULT::null();

    // Single node doesn't need merge or serialize
    RESULT result = finalize_fn_(context.get(), intermediate);
    UdaTestHarnessUtil::FreeIntermediate<INTERMEDIATE>(context.get(), intermediate);
    if (!CheckContext(context.get())) return RESULT::null();
    return result;
  }

  // Runs the UDA, simulating a single level aggregation. The values are processed
  // on num_nodes + 1 contexts. There are num_nodes that do update and serialize.
  // There is a final context that does merge and finalize.
  RESULT ExecuteOneLevel(const std::vector<const INPUT*>& values,
      int num_nodes) {
    std::vector<boost::shared_ptr<UdfContext> > contexts;
    std::vector<INTERMEDIATE> intermediates;
    contexts.resize(num_nodes);

    for (int i = 0; i < num_nodes; ++i) {
      UdfContext* cxt = UdfContext::CreateTestContext();
      contexts[i].reset(cxt);
      intermediates.push_back(UdaTestHarnessUtil::CreateIntermediate<INTERMEDIATE>(
            cxt, fixed_buffer_byte_size_));
      init_fn_(cxt, &intermediates[i]);
      if (!CheckContext(cxt)) return RESULT::null();
    }

    boost::scoped_ptr<UdfContext> merge_context(UdfContext::CreateTestContext());
    INTERMEDIATE merged =
        UdaTestHarnessUtil::CreateIntermediate<INTERMEDIATE>(
            merge_context.get(), fixed_buffer_byte_size_);
    init_fn_(merge_context.get(), &merged);
    if (!CheckContext(merge_context.get())) return RESULT::null();

    // Process all the values in the single level num_nodes contexts
    for (int i = 0; i < values.size(); ++i) {
      int target = i % num_nodes;
      update_fn_(contexts[target].get(), *values[i], &intermediates[target]);
    }

    // Merge them all into the final
    for (int i = 0; i < num_nodes; ++i) {
      if (!CheckContext(contexts[i].get())) return RESULT::null();
      if (serialize_fn_ != NULL) {
        INTERMEDIATE serialized = serialize_fn_(contexts[i].get(), intermediates[i]);
        INTERMEDIATE copy =
            UdaTestHarnessUtil::CopyIntermediate<INTERMEDIATE>(
                merge_context.get(), fixed_buffer_byte_size_, serialized);
        UdaTestHarnessUtil::FreeIntermediate<INTERMEDIATE>(
            contexts[i].get(), intermediates[i]);
        merge_fn_(merge_context.get(), copy, &merged);
        if (!CheckContext(contexts[i].get())) return RESULT::null();
      } else {
        merge_fn_(merge_context.get(), intermediates[i], &merged);
        UdaTestHarnessUtil::FreeIntermediate<INTERMEDIATE>(
            contexts[i].get(), intermediates[i]);
      }
      contexts[i].reset();
    }
    if (!CheckContext(merge_context.get())) return RESULT::null();

    RESULT result = finalize_fn_(merge_context.get(), merged);
    UdaTestHarnessUtil::FreeIntermediate<INTERMEDIATE>(merge_context.get(), merged);
    if (!CheckContext(merge_context.get())) return RESULT::null();
    return result;
  }

  // Runs the UDA, simulating a two level aggregation with num1 in the first level and
  // num2 in the second. The values are processed in num1 + num2 contexts.
  RESULT ExecuteTwoLevel(const std::vector<const INPUT*>& values,
      int num1, int num2) {
    std::vector<boost::shared_ptr<UdfContext> > level1_contexts, level2_contexts;
    std::vector<INTERMEDIATE> level1_intermediates, level2_intermediates;
    level1_contexts.resize(num1);
    level2_contexts.resize(num2);

    // Initialize the intermediate contexts and intermediate result buffers
    for (int i = 0; i < num1; ++i) {
      UdfContext* cxt = UdfContext::CreateTestContext();
      level1_contexts[i].reset(cxt);
      level1_intermediates.push_back(
          UdaTestHarnessUtil::CreateIntermediate<INTERMEDIATE>(
              cxt, fixed_buffer_byte_size_));
      init_fn_(cxt, &level1_intermediates[i]);
      if (!CheckContext(cxt)) return RESULT::null();
    }
    for (int i = 0; i < num2; ++i) {
      UdfContext* cxt = UdfContext::CreateTestContext();
      level2_contexts[i].reset(cxt);
      level2_intermediates.push_back(
          UdaTestHarnessUtil::CreateIntermediate<INTERMEDIATE>(
              cxt, fixed_buffer_byte_size_));
      init_fn_(cxt, &level2_intermediates[i]);
      if (!CheckContext(cxt)) return RESULT::null();
    }

    // Initialize the final context and final intermediate buffer
    boost::scoped_ptr<UdfContext> final_context(UdfContext::CreateTestContext());
    INTERMEDIATE final_intermediate =
        UdaTestHarnessUtil::CreateIntermediate<INTERMEDIATE>(
            final_context.get(), fixed_buffer_byte_size_);
    init_fn_(final_context.get(), &final_intermediate);
    if (!CheckContext(final_context.get())) return RESULT::null();

    // Assign all the input values to level 1 updates
    for (int i = 0; i < values.size(); ++i) {
      int target = i % num1;
      update_fn_(level1_contexts[target].get(),
          *values[i], &level1_intermediates[target]);
    }

    // Serialize the level 1 intermediates and merge them with a level 2 intermediate
    for (int i = 0; i < num1; ++i) {
      if (!CheckContext(level1_contexts[i].get())) return RESULT::null();
      int target = i % num2;
      if (serialize_fn_ != NULL) {
        INTERMEDIATE serialized =
            serialize_fn_(level1_contexts[i].get(), level1_intermediates[i]);
        INTERMEDIATE copy =
            UdaTestHarnessUtil::CopyIntermediate<INTERMEDIATE>(
                level1_contexts[i].get(), fixed_buffer_byte_size_, serialized);
        UdaTestHarnessUtil::FreeIntermediate<INTERMEDIATE>(
            level1_contexts[i].get(), level1_intermediates[i]);
        merge_fn_(level2_contexts[target].get(),
            copy, &level2_intermediates[target]);
        if (!CheckContext(level1_contexts[i].get())) return RESULT::null();
      } else {
        merge_fn_(level2_contexts[target].get(),
            level1_intermediates[i], &level2_intermediates[target]);
        UdaTestHarnessUtil::FreeIntermediate<INTERMEDIATE>(
            level1_contexts[i].get(), level1_intermediates[i]);
      }
      level1_contexts[i].reset();
    }

    // Merge all the level twos into the final
    for (int i = 0; i < num2; ++i) {
      if (!CheckContext(level2_contexts[i].get())) return RESULT::null();
      if (serialize_fn_ != NULL) {
        INTERMEDIATE serialized =
            serialize_fn_(level2_contexts[i].get(), level2_intermediates[i]);
        INTERMEDIATE copy =
            UdaTestHarnessUtil::CopyIntermediate<INTERMEDIATE>(
                level2_contexts[i].get(), fixed_buffer_byte_size_, serialized);
        UdaTestHarnessUtil::FreeIntermediate<INTERMEDIATE>(
            level2_contexts[i].get(), level2_intermediates[i]);
        merge_fn_(final_context.get(), copy, &final_intermediate);
        if (!CheckContext(level2_contexts[i].get())) return RESULT::null();
      } else {
        merge_fn_(final_context.get(), level2_intermediates[i], &final_intermediate);
        UdaTestHarnessUtil::FreeIntermediate<INTERMEDIATE>(
            level2_contexts[i].get(), level2_intermediates[i]);
      }
      level2_contexts[i].reset();
    }
    if (!CheckContext(final_context.get())) return RESULT::null();

    RESULT result = finalize_fn_(final_context.get(), final_intermediate);
    UdaTestHarnessUtil::FreeIntermediate<INTERMEDIATE>(
        final_context.get(), final_intermediate);
    if (!CheckContext(final_context.get())) return RESULT::null();
    return result;
  }

  // Buffer len for intermediate results if the type is TYPE_FIXED_BUFFER
  int fixed_buffer_byte_size_;

  // Error message if anything went wrong during the execution.
  std::string error_msg_;
};

}

#endif

