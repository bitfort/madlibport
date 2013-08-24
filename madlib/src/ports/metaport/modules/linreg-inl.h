
// author: victor bittorf (bittorf@cs.wisc.edu)

#ifndef MADLIB_METAPORT_MODULES_LINREG_INL_H
#include "modules/regress/linear.cpp"

namespace madlib {
namespace modules {
namespace regress {

/*! \brief Transitions the UDA using the given example and label
 * \param palloc The Port Allocator to use if allocation is needed
 * \param m the state of the UDA
 * \param vec the example's to process 
 * \param vec_len the length of vec
 * \param y the label for the example (a_i * x = y)
 * \return a new intermediate state after "viewing" the example
 */
MemHandle<char> LinrTransition(PortAllocator palloc, 
                               MemHandle<char> &m, 
                   double* vec, size_t vec_len, double y) {
  linregr_transition step_udf;
  // configure the UDF to use the provided allocator
  // Will segfault if alloc not set.
  step_udf.SetPortAlloc(palloc);

  dbconnector::mainmem::MutableByteString state(m.ptr, m.size);

  // container for the example
  TransparentHandle<double> th(vec);
  MappedColumnVector v(th, vec_len);

  AnyType t1;
  t1 << state;
  t1 << y;
  t1 << v;
  AnyType result = step_udf.run(t1);

  dbconnector::mainmem::MutableByteString modelr = 
      result.getAs<dbconnector::mainmem::MutableByteString>();
  MemHandle<char> out = {modelr.size(), modelr.ptr()};
  return out;
}


/*! \brief Runs the Linear Regression final function on the givne model
 *
 * Returns the solution as a vector; this will have the same dimensions as
 * the examples, it defines a .size() function and overloads operator[]. 
 *
 * The final function calls into Eigen to invert a matrix and compute the
 * final coefficients. 
 * \param state an intermediate state of the UDA
 * \return the linear regression solution
 */
MemHandle<double> LinrFinal(PortAllocator palloc, MemHandle<char> state_) {
  dbconnector::mainmem::MutableByteString state(state_.ptr, state_.size);
  linregr_final final;
  // configure the UDF to use the provided allocator; Will segfault if not set.
  final.SetPortAlloc(palloc);

  AnyType fin_args;
  fin_args << state;
  AnyType fin = final.run(fin_args);
  MutableNativeColumnVector coef = fin[0].getAs<MutableNativeColumnVector>();

  MemHandle<double> out = {coef.size(), 
                         (double*) palloc.Allocate(sizeof(double)*coef.size())};

  memcpy(out.ptr, &coef[0], out.size * sizeof(double));
  // TODO cleanup any memory?

  return out;
}

MemHandle<char> LinrInit(PortAllocator palloc) {
  // MADlib will want some scratch space of zeros to start with
  // It may need as little as 5 double's worth, but it is not clearly documented
  MemHandle<char> out;

  out.size = 100;
  out.ptr = static_cast<char*>(palloc.Allocate(100));
  memset(out.ptr, 0x0, out.size);
  return out;
}

/*! \brief Merges two states together
 */
MemHandle<char> LinrMerge(PortAllocator palloc,
                        MemHandle<char> state_a,
                        MemHandle<char> state_b) {
  dbconnector::mainmem::MutableByteString statea(state_a.ptr, state_a.size);
  dbconnector::mainmem::MutableByteString stateb(state_b.ptr, state_b.size);

  linregr_merge_states merge;
  merge.SetPortAlloc(palloc);

  AnyType args;
  args << statea;
  args << stateb;

  AnyType result = merge.run(args);
  dbconnector::mainmem::MutableByteString modelr = 
      result.getAs<dbconnector::mainmem::MutableByteString>();
  MemHandle<char> out = {modelr.size(), modelr.ptr()};
  return out;
}


} // namespace regress
}
} // namespace madlib
#endif
