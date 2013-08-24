
#include <cstdio>

#include "udf/uda-test-harness.h"
#include "madport/port-dbconnector-inl.h"
#include "src/linreg-inl.h"


using namespace impala;
using namespace impala_udf;
using namespace std;

/* Exeutes a simple test using a 2x2 example
 */
int TEST_linreg() {
  UdaTestHarness2<StringVal, StringVal, StringVal, DoubleVal> test1(
      madlib::impala::LinRegInit, madlib::impala::LinRegUpdate, 
      madlib::impala::LinRegMerge, 
      NULL, madlib::impala::LinRegFinalize);
  vector<StringVal> no_nulls;
  vector<DoubleVal> labels;

  labels.resize(2);
  labels[0] = 7.0;
  labels[1] = 19.0;
  // First entry is the label
  double ex1[3] = {1.0, 3.0};
  double ex2[3] = {5.0, 7.0};

  // Create the example input
  no_nulls.resize(2);
  no_nulls[0].ptr = (uint8_t*) ex1;
  no_nulls[0].len = 2 * sizeof(double);
  no_nulls[1].ptr = (uint8_t*) ex2;
  no_nulls[1].len = 2 * sizeof(double);

  // create the expected anser
  StringVal ans;
  double coef[2] = {1.0, 2.0};
  ans.ptr = (uint8_t*) coef;
  ans.len = 2 * sizeof(double);

  //test it
  bool b = test1.Execute(no_nulls, labels, ans);
  return (int)b;
}


int main(int argc, char** argv) {
  //impala::InitGoogleLoggingSafe(argv[0]);
//  ::testing::InitGoogleTest(&argc, argv);
  //return RUN_ALL_TESTS();
  RUNTEST(TEST_linreg);
}

