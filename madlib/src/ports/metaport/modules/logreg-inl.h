
// author: victor bittorf (bittorf@cs.wisc.edu)

#ifndef MADLIB_METAPORT_MODULES_LOGREG_INL_H

#include "modules/regress/logistic.cpp"

namespace madlib {
namespace modules {
namespace regress {

typedef MutableArrayHandle<double> LogrHandle_t;
typedef LogRegrIGDTransitionState<LogrHandle_t> LogrModel;


/*! \brief Updates the logsitic model using the given example
 * Note: new state may be backed by new memory than the previous state. 
 * \param pa the allocator to use if new state needs to be created
 * \param mh the handle to the current state
 * \param vec the example's values
 * \param vec_len the length of vec
 * \param y the label for the example
 * \param prevh state from the previous pass of the UDA
 * \return a handle to the new state
 */
//template <class STEP_UDF>
MemHandle<char> LogrTransition(PortAllocator pa, MemHandle<char> mh, 
                   double* vec, size_t vec_len, bool y, MemHandle<char> prevh) {
  // Create the state for the MADlib UDA
  madlib::ArrayType state_arr; 
  state_arr.len = mh.size;
  state_arr.ptr = static_cast<void*>(mh.ptr);
  state_arr.dims[0] = mh.size;
  state_arr.ndims = 1;
  LogrHandle_t state_(&state_arr);
  LogrModel state(state_);

  logregr_igd_step_transition step;
  step.SetPortAllocator(pa);
  TransparentHandle<double> th(vec);
  MappedColumnVector v(th, vec_len);

  AnyType t1;
  t1 << state;
  t1 << y;
  t1 << v;

  // if this is the second pass of the UDA, MADlib wants access to previous
  // state; this keeps continutity between invocations of the UDA
  if ((prevh.size != 0) && (prevh.ptr != NULL)) {
    madlib::ArrayType pstate_arr; 
    pstate_arr.len = prevh.size;
    pstate_arr.ptr = static_cast<void*>(prevh.ptr);
    pstate_arr.dims[0] = prevh.size;
    pstate_arr.ndims = 1;
    LogrHandle_t pstate_(&pstate_arr);
    LogrModel prev(pstate_);
    t1 << prev;
  } else {
    AnyType null;
    t1 << null;
  }

  AnyType result = step.run(t1);
  LogrModel rm = result.getAs<LogrModel>();
  printf("Model : %f %f\n", rm.coef[0], rm.coef[1]);
  LogrHandle_t res = rm.storage();
  MemHandle<char> r = {res.size(), (char*) &res[0]};
  return r;
}

/*! \brief Creates an initial state for Logistic, which is empty zeros
 */
MemHandle<char> LogrInit(PortAllocator pa) {
  MemHandle<char> m;
  m.ptr = static_cast<char*>(pa.Allocate(5*sizeof(double)));
  m.size = 5*sizeof(double);
  memset(m.ptr, 0x0, m.size);
  return m;
}

/*! \brief Copies the coef out of the state into a new handle
 */
MemHandle<double> LogrFinal(PortAllocator pa, MemHandle<char> mh) {
  // Create the state for the MADlib UDA
  madlib::ArrayType state_arr; 
  state_arr.len = mh.size;
  state_arr.ptr = static_cast<void*>(mh.ptr);
  state_arr.dims[0] = mh.size;
  state_arr.ndims = 1;
  LogrHandle_t state_(&state_arr);
  LogrModel state(state_);

  MemHandle<double> coef;
  coef.size = state.coef.size();
  coef.ptr = static_cast<double*>(pa.Allocate(coef.size * sizeof(double)));

  memcpy(coef.ptr, &state.coef[0], coef.size * sizeof(double));
  return coef;
}

MemHandle<char> LogrMerge(PortAllocator pa, MemHandle<char> a,
                          MemHandle<char> b) {
  // Create the state for the MADlib UDA
  madlib::ArrayType st; 
  st.len = a.size;
  st.ptr = static_cast<void*>(a.ptr);
  st.dims[0] = a.size;
  st.ndims = 1;
  LogrHandle_t state_(&st);
  LogrModel state(state_);

  // Create second state
  madlib::ArrayType st2; 
  st2.len = b.size;
  st2.ptr = static_cast<void*>(b.ptr);
  st2.dims[0] = b.size;
  st2.ndims = 1;
  LogrHandle_t state_2(&st2);
  LogrModel state2(state_2);


  AnyType t1;
  t1 << state;
  t1 << state2;

  // Get merged state
  logregr_cg_step_merge_states merge;
  merge.SetPortAllocator(pa);
  AnyType result = merge.run(t1);
  LogrModel rm = result.getAs<LogrModel>();
  LogrHandle_t res = rm.storage();
  MemHandle<char> r = {res.size(), (char*) &res[0]};
  return r;
}


/*
template <class STATE>
double Predict(STATE &lr, double* vec) {
  double d = 0;
  for (size_t i = 0; i < lr.coef.size(); i++) {
    d += lr.coef[i] * vec[i];
  }

  return 1.0 / (1.0 + std::exp(- d));
}
*/

} // namespace regress
}
} // namespace madlib
#endif
