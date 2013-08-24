


/* Main memory port of MADlib Linear Regression
 */


//#include <regress/linear.cpp>

// Note: If errors occur in the following include files, it could indicate that
// new macros have been added to mainmemQL header files.


#define EIGEN_YES_I_KNOW_SPARSE_MODULE_IS_NOT_STABLE_YET

#include <cstdio>

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <dbconnector/dbconnector.hpp>

#include "modules/regress/linear.cpp"


using namespace madlib;
using namespace madlib::modules::regress;

typedef dbconnector::mainmem::MutableByteString LRModel;
typedef MutableNativeColumnVector LRVector;


/*! Transitions the UDA using the given example and label
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


/*! Runs the Linear Regression final function on the givne model
 * Returns the solution as a vector.
 *
 * TODO not clear how the coef vector is allocated and how it has to be freed
 */
LRVector Final(LRModel &m) {
  linregr_final final;
  AnyType fin_args;
  fin_args << m;
  AnyType fin = final.run(fin_args);
  MutableNativeColumnVector coef = fin[0].getAs<MutableNativeColumnVector>();
  return coef;
}

int main( ) {
  size_t len = 1024*1024;
  char *bytea = new char[len];
  double *state = reinterpret_cast<double*>(bytea);
  dbconnector::mainmem::MutableByteString model(bytea, len);

  printf("model.size() = %lu\n", model.size());


  double ex1[2] = {1.0, 2.0};
  double ex2[2] = {2.0, 3.0};
  linregr_transition step_udf;



  LRModel modelr = Transition(step_udf, model, ex1, 2, 5);
  modelr = Transition(step_udf, modelr, ex2, 2, 8);

  LRVector coef = Final(modelr);
  printf("\ncoef:\n");
  for (int i = 0; i < coef.size(); i++){
    printf("[%02d] %f\n", i, coef[i]);
  }
}
