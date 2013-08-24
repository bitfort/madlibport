
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

#include "modules/regress/logistic.cpp"

using namespace madlib;
using namespace madlib::modules::regress;



typedef MutableArrayHandle<double> Handle_t;
int main( ) {
  printf("Generating Example array...\n");
  ArrayType arr;
  arr.len = 5;
  double arr_back[5] = {0, 0, 0, 0, 0};
  arr.ptr = (void*) arr_back;
  arr.ndims = 1;
  arr.dims[0] = 100;

  arr_back[0] = static_cast<double>(2);

  // step size
  arr_back[1] = 0.1;
  // coef
  arr_back[2] = 0.1;
  arr_back[3] = 0.1;


  *reinterpret_cast<uint64_t*>(&arr_back[4]) = 2;

  Handle_t  arrh (&arr);
  Allocator alloc;


  LogRegrIGDTransitionState<Handle_t> logr(arrh);
  LogRegrIGDTransitionState<Handle_t> prev_logr(arrh);
  logr.initialize(alloc, 2);
  printf("Num rows %%uh = %hu\n", (short)logr.numRows);


  LogRegrIGDTransitionState<Handle_t> logr_next(arrh);
  logr_next.initialize(alloc, 2);

  double ex1[2] = {1.0, 0.0};
  TransparentHandle<double> th(ex1);
  MappedColumnVector v1(th, 2);
  double ex2[2] = {0.0, 1.0};
  TransparentHandle<double> th2(ex2);
  MappedColumnVector v2(th2, 2);
  logregr_igd_step_transition step;


  for (int i = 0; i < 500; i++) {
  printf("(pre-step)  model = %f %f\n", arr_back[2], arr_back[3]);
  printf("(pre-step)  logr.nrows = %hd\n", (short)logr.numRows);
  AnyType t1;
  t1 << logr;
  t1 << true;
  t1 << v1;
  t1 << prev_logr;

  // mutates logr
  logr_next = step.run(t1);
  
  printf("*** coef: %f %f\n", (double)logr_next.coef[0], (double)logr_next.coef[1]);
  AnyType t2;
  t2 << logr_next;
  t2 << false;
  t2 << v2;
  t2 << prev_logr;
  logr = step.run(t2);
  printf("*** coef: %f %f\n", (double)logr.coef[0], (double)logr.coef[1]);
  }
}
