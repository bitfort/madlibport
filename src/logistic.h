
#ifndef IMPALA_BISMARCK_LOGISTIC_H
#define IMPALA_BISMARCK_LOGISTIC_H

#include "bismarck-inl.h"

#include "metaport/modules/logreg-inl.h"

namespace madlib {
namespace port {

using namespace impala;
using namespace impala_udf;
using namespace std;
using namespace impala::bismarck;


double LogisticLoss(const StringVal &v, const BooleanVal &y,
                    const StringVal &model) {
}

void LogisticStep(UdfContext* ctx, 
                    const StringVal &val, const BooleanVal &y,
                    StringVal *input) { 
  printf("Logistic Step\n");
  PortAllocator pa(ctx);

  // convert to types that MADlib expects
  madlib::MemHandle<char> state = {input->len, (char*)input->ptr};
  size_t len_val = val.len/sizeof(double);
  double *v = (double*) val.ptr;

  madlib::MemHandle<char> prev = {0, NULL};

  bool lbl = y.val;

  // the label is the first entry in the array
  madlib::MemHandle<char> new_state = 
      madlib::modules::regress::LogrTransition(pa, state, v, 
                                                     len_val, lbl,
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

/*
double LogisticLoss(const INPUT1&, const INPUT2&, 
               const BismarckModel_t&) {
};
*/

void LogisticMerge(UdfContext* ctx, const BismarckModel_t& src, 
                   BismarckModel_t* dst) {
  printf("Logistic Merge\n");
  PortAllocator pa(ctx);
  madlib::MemHandle<char> statea = {dst->len, (char*)dst->ptr};
  madlib::MemHandle<char> stateb = {src.len, (char*)src.ptr};

  madlib::MemHandle<char> combin = 
      madlib::modules::regress::LogrMerge(pa, statea, stateb);
  dst->ptr = (uint8_t*) combin.ptr;
  dst->len = combin.size;
}

void LogisticInit(UdfContext* ctx, StringVal *m) {
  printf("Logistic init\n");
  PortAllocator pa(ctx);
  // get a handle to our allocated state
  madlib::MemHandle<char> state = madlib::modules::regress::LogrInit(pa);
  m->len = state.size;
  m->ptr = reinterpret_cast<uint8_t*>(state.ptr);
}


double LogisticLossMerge(double a, double b) { 
  return a + b;
}


} // namespace port
} // namespace madlib
#endif

