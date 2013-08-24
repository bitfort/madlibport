
#include "test-common.h"
#include "modules/regress/linear.cpp"


namespace linreg {
typedef dbconnector::mainmem::MutableByteString LRModel;
typedef MutableNativeColumnVector LRVector;

using namespace madlib;
using namespace madlib::modules::regress;

/*! \brief Transitions the UDA using the given example and label
 * \param step_udf the UDF object that updates the model using an example
 * \param m the intermediate state returned by the last transition step
 * \param vec the example values, as a dense vector
 * \param vec_len the length of vec
 * \param y the label value for the example
 * \return a new intermediate state after "viewing" the example
 */
LRModel Transition(linregr_transition &step_udf, LRModel &m, 
                   double* vec, size_t vec_len, double y) {
  TransparentHandle<double> th(vec);
  MappedColumnVector v(th, vec_len);

  AnyType t1;
  t1 << m;
  t1 << y;
  t1 << v;
  AnyType result = step_udf.run(t1);

  dbconnector::mainmem::MutableByteString modelr = 
      result.getAs<dbconnector::mainmem::MutableByteString>();
  return modelr;
}

/*! \brief Runs the Linear Regression final function on the givne model
 *
 * Returns the solution as a vector; this will have the same dimensions as
 * the examples, it defines a .size() function and overloads operator[]. 
 *
 * The final function calls into Eigen to invert a matrix and compute the
 * final coefficients. 
 * \param m an intermediate state of the UDA
 * \return the linear regression solution
 */
LRVector Final(LRModel &m) {
  linregr_final final;
  AnyType fin_args;
  fin_args << m;
  AnyType fin = final.run(fin_args);
  MutableNativeColumnVector coef = fin[0].getAs<MutableNativeColumnVector>();
  return coef;
}

/*! \brief Tests the UDA transition and Final funciton on a toy example
 */
int linreg_2by2() {
  size_t len = 1024*1024;
  char *bytea = new char[len];
  double *state = reinterpret_cast<double*>(bytea);
  dbconnector::mainmem::MutableByteString model(bytea, len);

  double ex1[2] = {1.0, 2.0};
  double ex2[2] = {2.0, 3.0};
  linregr_transition step_udf;

  LRModel modelr = Transition(step_udf, model, ex1, 2, 5);
  modelr = Transition(step_udf, modelr, ex2, 2, 8);

  LRVector coef = Final(modelr);
  EXPECT_EQ(coef[0], 1.0);
  EXPECT_EQ(coef[1], 2.0);
  return 1;
}
}

int main() {
  using namespace linreg;
  RUNTEST(linreg_2by2)
}
