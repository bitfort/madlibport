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

#include <gtest/gtest.h>
#include <iostream>
#include "udf/udf-test-harness.h"
//#include "util/logging.h"

using namespace impala;
using namespace impala_udf;
using namespace std;

DoubleVal ZeroUdf(UdfContext* context) {
  return DoubleVal(0);
}

StringVal LogUdf(UdfContext* context, const StringVal& arg1) {
  cerr << (arg1.is_null ? "NULL" : string((char*)arg1.ptr, arg1.len)) << endl;
  return arg1;
}

StringVal UpperUdf(UdfContext* context, const StringVal& input) {
  if (input.is_null) return StringVal::null();
  // Create a new StringVal object that's the same length as the input
  StringVal result = StringVal(context, input.len);
  for (int i = 0; i < input.len; ++i) {
    result.ptr[i] = toupper(input.ptr[i]);
  }
  return result;
}

FloatVal Min3(UdfContext* context, const FloatVal& f1,
    const FloatVal& f2, const FloatVal& f3) {
  bool is_null = true;
  float v;
  if (!f1.is_null) {
    if (is_null) {
      v = f1.val;
      is_null = false;
    } else {
      v = std::min(v, f1.val);
    }
  }
  if (!f2.is_null) {
    if (is_null) {
      v = f2.val;
      is_null = false;
    } else {
      v = std::min(v, f2.val);
    }
  }
  if (!f3.is_null) {
    if (is_null) {
      v = f3.val;
      is_null = false;
    } else {
      v = std::min(v, f3.val);
    }
  }
  return is_null ? FloatVal::null() : FloatVal(v);
}

IntVal ValidateUdf(UdfContext* context) {
  EXPECT_EQ(context->version(), UdfContext::v1_2);
  EXPECT_FALSE(context->has_error());
  EXPECT_TRUE(context->error_msg() == NULL);
  return IntVal::null();
}

IntVal ValidateFail(UdfContext* context) {
  EXPECT_FALSE(context->has_error());
  EXPECT_TRUE(context->error_msg() == NULL);
  context->SetError("Fail");
  EXPECT_TRUE(context->has_error());
  EXPECT_TRUE(strcmp(context->error_msg(), "Fail") == 0);
  return IntVal::null();
}

IntVal ValidateMem(UdfContext* context) {
  EXPECT_TRUE(context->Allocate(0) == NULL);
  uint8_t* buffer = context->Allocate(10);
  EXPECT_TRUE(buffer != NULL);
  memset(buffer, 0, 10);
  context->Free(buffer);
  return IntVal::null();
}

TEST(UdfTest, TestUdfContext) {
  EXPECT_TRUE(UdfTestHarness::ValidateUdf<IntVal>(ValidateUdf, IntVal::null()));
  EXPECT_FALSE(UdfTestHarness::ValidateUdf<IntVal>(ValidateFail, IntVal::null()));
  EXPECT_TRUE(UdfTestHarness::ValidateUdf<IntVal>(ValidateMem, IntVal::null()));
}

TEST(UdfTest, TestValidate) {
  EXPECT_TRUE(UdfTestHarness::ValidateUdf<DoubleVal>(ZeroUdf, DoubleVal(0)));
  EXPECT_FALSE(UdfTestHarness::ValidateUdf<DoubleVal>(ZeroUdf, DoubleVal(10)));

  EXPECT_TRUE((UdfTestHarness::ValidateUdf<StringVal, StringVal>(
      LogUdf, StringVal("abcd"), StringVal("abcd"))));

  EXPECT_TRUE((UdfTestHarness::ValidateUdf<StringVal, StringVal>(
      UpperUdf, StringVal("abcd"), StringVal("ABCD"))));

  EXPECT_TRUE((UdfTestHarness::ValidateUdf<FloatVal, FloatVal, FloatVal, FloatVal>(
      Min3, FloatVal(1), FloatVal(2), FloatVal(3), FloatVal(1))));
  EXPECT_TRUE((UdfTestHarness::ValidateUdf<FloatVal, FloatVal, FloatVal, FloatVal>(
      Min3, FloatVal(1), FloatVal::null(), FloatVal(3), FloatVal(1))));
  EXPECT_TRUE((UdfTestHarness::ValidateUdf<FloatVal, FloatVal, FloatVal, FloatVal>(
      Min3, FloatVal::null(), FloatVal::null(), FloatVal::null(), FloatVal::null())));
}

int main(int argc, char** argv) {
//  impala::InitGoogleLoggingSafe(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

