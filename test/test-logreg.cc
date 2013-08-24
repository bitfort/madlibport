#include <cstdio>

#include "udf/uda-test-harness.h"
#include "madport/port-dbconnector-inl.h"
#include "logistic.h"


using namespace impala;
using namespace impala_udf;
using namespace std;

using namespace impala::bismarck;
using namespace madlib::port;

int TEST_logreg2x2() {
  UdaTestHarness3<StringVal, StringVal, StringVal, StringVal, BooleanVal> test1(
      BismarckInit, 
      BismarckStep2<StringVal, BooleanVal, LogisticInit,
      LogisticStep>,
      BismarckMerge<LogisticMerge>,
      NULL,
      BismarckFinal);

  vector<StringVal> prev_state;
  vector<StringVal> no_nulls;
  vector<BooleanVal> labels;

  // First entry is the label
  double ex1[2] = {1.0, 0.0};
  double ex2[2] = {0.0, 1.0};

  // Create the example input
  no_nulls.resize(2);
  no_nulls[0].ptr = (uint8_t*) ex1;
  no_nulls[0].len = 3 * sizeof(double);
  no_nulls[1].ptr = (uint8_t*) ex2;
  no_nulls[1].len = 3 * sizeof(double);

  prev_state.resize(2);

  labels.resize(2);
  labels[0] = true;
  labels[1] = false;

  // create the expected anser
  StringVal ans;
  double coef[2] = {1.0, 2.0};
  ans.ptr = (uint8_t*) coef;
  ans.len = 2 * sizeof(double);

  //test it
  bool b = test1.Execute(prev_state, no_nulls, labels, ans);
  return (int)b;
}

/*! \brief Tests the UDA transition and Final funciton on a toy example
 */
/*
int logreg_2by2() {
  double arr_back[5] = {0, 0, 0, 0, 0};
  madlib::ArrayType arr;
  arr.len = 5;
  arr.ptr = (void*) arr_back;
  arr.ndims = 1;
  arr.dims[0] = 100;
  *reinterpret_cast<uint64_t*>(&arr_back[4]) = 2;

  arr_back[0] = static_cast<double>(2);
  // step size
  arr_back[1] = 0.1;
  // coef
  arr_back[2] = 0.1;
  arr_back[3] = 0.1;

  double arr2[5];

  madlib::ArrayType arr2_ = MakeArray(arr2, 100);
  Handle_t arrh (&arr);
  Handle_t arrt(&arr2_);

  double ex1[2] = {1.0, 1.0};
  //double ex2[2] = {0.0, 1.0};
  logregr_igd_step_transition step;

  // First pass of UDA has no previous state
  arrt = Transition(step, arrh, ex1, 2, true);



  EXPECT_NEAR(arrt[2], 0.145017, 0.001);
  EXPECT_NEAR(arrt[3], 0.145017, 0.001);


//  printf("predict ex 1 = %f\n", Predict(logr, ex1));
  //printf("predict ex 2 = %f\n", Predict(logr, ex2));

  return 1;
}


int TEST_model_ctor()  {
  printf("Generating Example array...\n");
  ArrayType arr;
  arr.len = 5;
  double arr_back[5] = {0, 0, 0, 0, 0};
  arr.ptr = (void*) arr_back;
  arr.ndims = 1;
  arr.dims[0] = 100;

  arr_back[0] = static_cast<double>(2);

  // step size
  arr_back[1] = 0.5;
  // coef
  arr_back[2] = 0.1;
  arr_back[3] = -0.1;
  *reinterpret_cast<uint64_t*>(&arr_back[4]) = 2;
  Handle_t  arrh (&arr);

  LogRegrIGDTransitionState<Handle_t> logr(arrh);

  EXPECT_EQ((double)logr.coef[0], 0.1);
  EXPECT_EQ((double)logr.coef[1], -0.1);
  EXPECT_EQ((double)logr.stepsize, 0.5);
  return 1;
}
} // namespace logreg

int main() {
  using namespace logreg;
  RUNTEST(TEST_model_ctor)
  RUNTEST(logreg_2by2)
}
*/

int main() {
  RUNTEST(TEST_logreg2x2);
}
