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

#include <iostream>

#include <udf/uda-test-harness.h>
#include "uda-sample.h"

using namespace impala;
using namespace impala_udf;
using namespace std;

int main(int argc, char** argv) {
  // Use the UDA test harness to validate the COUNT UDA.
  UdaTestHarness<AnyVal, BigIntVal, BigIntVal> test(
      CountInit, CountUpdate, CountMerge, NULL, CountFinalize);

  // Run the UDA over 10000 non-null values
  vector<IntVal> no_nulls;
  no_nulls.resize(10000);
  if (!test.Execute<IntVal>(no_nulls, BigIntVal(no_nulls.size()))) {
    cerr << test.GetErrorMsg() << endl;
    return 1;
  }

  // Run the UDA with some nulls
  vector<IntVal> some_nulls;
  some_nulls.resize(10000);
  int expected = some_nulls.size();
  for (int i = 0; i < some_nulls.size(); i += 100) {
    some_nulls[i] = IntVal::null();
    --expected;
  }
  if (!test.Execute(some_nulls, BigIntVal(expected))) {
    cerr << test.GetErrorMsg() << endl;
    return 1;
  }

  cerr << "Tests passed." << endl;
  return 0;
}
