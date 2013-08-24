#ifndef MADLIB_MODULES_IMPALA_LOGREG_INL_H
#include <cstdio>

#include "udf/udf.h"
#include "metaport/modules/logreg-inl.h"

namespace madlib {
namespace impala {

using namespace impala;
using namespace impala_udf;
using namespace std;

/*! \brief Initializes the UDA state with zeros
 */
void LinRegInit(UdfContext* context, StringVal* m) {
  PortAllocator pa(context);
  // get a handle to our allocated state
  madlib::MemHandle<char> state = madlib::modules::regress::LogrInit(pa);
  m->len = state.size;
  m->ptr = reinterpret_cast<uint8_t*>(state.ptr);
}

/*! \brief Updates the input state with the given value
 */
void LinRegUpdate(UdfContext* context, const StringVal& val, StringVal* input) {
  PortAllocator pa(context);

  // convert to types that MADlib expects
  madlib::MemHandle<char> state = {input->len, (char*)input->ptr};
  size_t len_val = val.len/sizeof(double);
  double *v = (double*) val.ptr;

  madlib::MemHandle<char> prev = {0, NULL};

  // the label is the first entry in the array
  madlib::MemHandle<char> new_state = 
      madlib::modules::regress::LogrTransition(pa, state, &v[1], 
                                                     len_val-1, v[0],
                                                     prev);
  
  // clean up memory if the transition function re-allocated
  if (input->ptr != (uint8_t*) new_state.ptr) {
    pa.Free(input->ptr);
  }

  // state and new_state may or maynot be backed by the same memory
  // In particular, if this is the first call to transition, these will
  // be backed by different memory
  input->ptr = (uint8_t*) new_state.ptr;
  input->len = new_state.size;
}

void LinRegMerge(UdfContext* context, const StringVal& src, StringVal* dst) {
  PortAllocator pa(context);
  madlib::MemHandle<char> statea = {dst->len, (char*)dst->ptr};
  madlib::MemHandle<char> stateb = {src.len, (char*)src.ptr};

  madlib::MemHandle<char> combin = 
      madlib::modules::regress::LogrMerge(pa, statea, stateb);
  dst->ptr = (uint8_t*) combin.ptr;
  dst->len = combin.size;
}

/*! \brief Computes the solution and returns the coefficient vector
 */
StringVal LinRegFinalize(UdfContext* context, const StringVal& input) {
  PortAllocator pa(context);

  // convert to types that MADlib expects
  madlib::MemHandle<char> state = {input.len, (char*)input.ptr};

  madlib::MemHandle<double> coef = 
      madlib::modules::regress::LogrFinal(pa, state);

  printf("ans = %f %f\n", coef.ptr[0], coef.ptr[1]);
  StringVal sv((uint8_t*) coef.ptr, coef.size*sizeof(double));
  return sv;
  return StringVal();
}
} // namespace impala
} // namespace madlib
#endif

