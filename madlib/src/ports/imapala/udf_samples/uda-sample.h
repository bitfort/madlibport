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


#ifndef IMPALA_UDF_SAMPLE_UDA_H
#define IMPALA_UDF_SAMPLE_UDA_H

#include <udf/udf.h>

using namespace impala_udf;

// These functions implement Impala's UDA contract.
void CountInit(UdfContext* context, BigIntVal* val);
void CountUpdate(UdfContext* context, const AnyVal& input, BigIntVal* val);
void CountMerge(UdfContext* context, const BigIntVal& src, BigIntVal* dst);
BigIntVal CountFinalize(UdfContext* context, const BigIntVal& val);

#endif
