#ifndef MADLIB_MODULES_IMPALA_LINREG_INL_H
#include <cstdio>

#include "linreg.h"
#include "udf/udf.h"
#include "metaport/modules/linreg-inl.h"

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
  madlib::MemHandle<char> state = madlib::modules::regress::LinrInit(pa);
  m->len = state.size;
  m->ptr = reinterpret_cast<uint8_t*>(state.ptr);
}

/*! \brief Updates the input state with the given value
 */
void LinRegUpdate(UdfContext* context, const StringVal& val, const DoubleVal &y, 
                  StringVal* input) {
  PortAllocator pa(context);

  // convert to types that MADlib expects
  madlib::MemHandle<char> state = {(size_t)input->len, (char*)input->ptr};
  size_t len_val = val.len/sizeof(double);
  double *v = (double*) val.ptr;

  // the label is the first entry in the array
  madlib::MemHandle<char> new_state = 
      madlib::modules::regress::LinrTransition(pa, state, &v[0], 
                                                     len_val, y.val);

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
  madlib::MemHandle<char> statea = {(size_t)dst->len, (char*)dst->ptr};
  madlib::MemHandle<char> stateb = {(size_t)src.len, (char*)src.ptr};

  madlib::MemHandle<char> combin = 
      madlib::modules::regress::LinrMerge(pa, statea, stateb);
  dst->ptr = (uint8_t*) combin.ptr;
  dst->len = combin.size;
}

/*! \brief Computes the solution and returns the coefficient vector
 */
StringVal LinRegFinalize(UdfContext* context, const StringVal& input) {
  PortAllocator pa(context);

  // convert to types that MADlib expects
  madlib::MemHandle<char> state = {(size_t)input.len, (char*)input.ptr};

  madlib::MemHandle<double> coef = 
      madlib::modules::regress::LinrFinal(pa, state);

  printf("ans = %f %f\n", coef.ptr[0], coef.ptr[1]);
  StringVal sv((uint8_t*) coef.ptr, coef.size*sizeof(double));
  return sv;
}

} // namespace impala
} // namespace madlib
#endif
