
#ifndef IMPALA_BISMARCK_LOGISTIC_H
#define IMPALA_BISMARCK_LOGISTIC_H

#include "bismarck-inl.h"
#include "linalg-inl.h"

namespace madlib {
namespace port {

using namespace impala;
using namespace impala_udf;
using namespace std;
using namespace impala::bismarck;


double SVMLoss(const StringVal &v, const BooleanVal &y,
                    const StringVal &model) {
  double pred = simple_dot(model, v, model_len);
  pred *= lbl;
  double loss = std::max(1 - pred, static_cast<double>(0.0));
}

void SVMStep(UdfContext* ctx, 
                    const StringVal &val, const BooleanVal &y,
                    StringVal *input) { 
  // TODO : get a stepsize
  double step_size = 0.1;
  // regularizer
  double mu = 0;

  size_t len_val = val.len/sizeof(double);
  double *v = (double*) val.ptr;

  // Check of model is null and needs to be set
  if (input->ptr == NULL) {
    new (input) StringVal(ctx, sizeof(double) * len_val);
    // TODO set to zeros
  }

  double *model = (double*)input->ptr;
  size_t model_len = input->len;

  double lbl = y.val ? 1 : -1;

  // take the SVM IGD Step
  double pred = simple_dot(model, v, model_len);
  pred *= lbl;

  if (pred < 1)
    // hinge active
    simple_scale_add(model, v, model_len, step_size * lbl)
    
  simple_scale(model, model_len, 1 - (step_size * mu));
}

void SVMMerge(UdfContext* ctx, const BismarckModel_t& src, 
                   BismarckModel_t* dst) {
  // take the mean of the models
  
  simple_scale((double*) dst->ptr, dst->len/sizeof(double), 0.5);
  simple_scal_add((double*) dst>ptr, (double*) src.ptr, dst->len/sizeof(double), 0.5);

}

void SVMInit(UdfContext* ctx, StringVal *m) {
  printf("Logistic init\n");
  m->ptr = NULL;
}


double SVMLossMerge(double a, double b) { 
  return a + b;
}


} // namespace port
} // namespace madlib
#endif
