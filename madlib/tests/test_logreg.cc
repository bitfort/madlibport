
#include "test-common.h"

#include "modules/regress/logistic.cpp"
namespace logreg {

using namespace madlib;
using namespace madlib::modules::regress;


typedef MutableArrayHandle<double> Handle_t;
typedef LogRegrIGDTransitionState<Handle_t> LRModel;
typedef MutableNativeColumnVector LRVector;

/*! \brief Transitions the UDA using the given example and label
 * \param step_udf the UDF object that updates the model using an example
 * \param m the intermediate state returned by the last transition step
 * \param vec the example values, as a dense vector
 * \param vec_len the length of vec
 * \param y the label value for the example
 * \param prev the rpevious state of the UDA
 * \return a new intermediate state after "viewing" the example
 */
template <class STEP_UDF>
Handle_t Transition(STEP_UDF &step_udf, Handle_t &m, 
                   double* vec, size_t vec_len, bool y, Handle_t &prevh) {
  TransparentHandle<double> th(vec);
  MappedColumnVector v(th, vec_len);

  LRModel mod(m);
  LRModel prev(prevh);

  // Trick it into not resetting the model
  // if numRows is equal to zero; then the UDA transit function will zero
  // the coef and change the step size of 0.01 (hardcoded)
  // TODO FIXME
  // NOt sure how hanlde this situation
  mod.numRows = 1;
  prev.numRows = 1;

  AnyType t1;
  t1 << mod;
  t1 << y;
  t1 << v;
  t1 << prev;
  AnyType result = step_udf.run(t1);
  LRModel rm = result.getAs<LRModel>();
  return rm.storage();
}

/*! \brief Transitions the UDA using the given example and label
 * Does not have a previous state (used for first pass of the UDA)
 * \param step_udf the UDF object that updates the model using an example
 * \param m the intermediate state returned by the last transition step
 * \param vec the example values, as a dense vector
 * \param vec_len the length of vec
 * \param y the label value for the example
 * \param prev the rpevious state of the UDA
 * \return a new intermediate state after "viewing" the example
 */
template <class STEP_UDF>
Handle_t Transition(STEP_UDF &step_udf, Handle_t &m, 
                    double* vec, size_t vec_len, bool y) {
  TransparentHandle<double> th(vec);
  MappedColumnVector v(th, vec_len);

  LRModel mod(m);

  // Trick it into not resetting the model
  // if numRows is equal to zero; then the UDA transit function will zero
  // the coef and change the step size of 0.01 (hardcoded)
  mod.numRows = 1;

  AnyType t1;
  t1 << mod;
  t1 << y;
  t1 << v;
  AnyType result = step_udf.run(t1);
  LRModel rm = result.getAs<LRModel>();
  return rm.storage();
}

template <class STATE>
double Predict(STATE &lr, double* vec) {
  double d = 0;
  for (size_t i = 0; i < lr.coef.size(); i++) {
    d += lr.coef[i] * vec[i];
  }

  return 1.0 / (1.0 + std::exp(- d));
}

/*! \brief Tests the UDA transition and Final funciton on a toy example
 */
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
