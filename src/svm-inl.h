
#ifndef IMPALA_BISMARCK_SVM_INL_H
#define IMPALA_BISMARCK_SVM_INL_H

#include "linalg-inl.h"

// see for documentation
#include "svm.h"

namespace hazy {
namespace bismarck {

template <class CTX>
double BismarckSVM<CTX>::Loss(const bytea &v, bool y,
                    const bytea &model) {
  size_t model_len, v_len;
  double *modelp, *vp;
  CoerceBytea(model, modelp, model_len);
  CoerceBytea(v, vp, v_len);
  double pred = hazy::simple_dot(modelp, vp, model_len);
  double lbl = y ? 1 : -1;
  pred *= lbl;
  double loss = std::max(1 - pred, static_cast<double>(0.0));
  return loss;
}

template <class CTX>
void BismarckSVM<CTX>::Step(CTX* ctx, const bytea &val, const bool &y, 
                            bytea *input, double step_size, double mu) { 
  size_t len_val, model_len;
  double *v, *model;
  CoerceBytea(val, v, len_val);

  // Check of model is null and needs to be set
  if (input->str == NULL) {
    input->str = (char*) BismarckAllocate<double>(ctx, len_val);
    input->len = len_val * sizeof(double);
    
    // inialize the model to zeros
    for (size_t i = 0; i < len_val; i++) { 
      reinterpret_cast<double*>(input->str)[i] = 0.0;
    }
  }

  CoerceBytea(*input, model, model_len);
  double lbl = y ? 1 : -1;
  // take the SVM IGD Step
  double pred = hazy::simple_dot(model, v, model_len);
  pred *= lbl;

  if (pred < 1)
    // hinge active
    hazy::simple_scale_add(model, v, step_size * lbl, model_len);
  hazy::simple_scale(model, 1 - (step_size * mu), model_len);
}

template <class CTX>
void BismarckSVM<CTX>::Merge(CTX* ctx, const bytea& src, 
                   bytea* dst) {
  // TODO check if dst is initlaized, check if src is intilaized
  // take the mean of the models
  hazy::simple_scale((double*) dst->str, dst->len/sizeof(double), 0.5);
  hazy::simple_scale_add((double*) dst->str, (double*) src.str, 0.5, 
                         dst->len/sizeof(double));

}

template <class CTX>
void BismarckSVM<CTX>::Init(CTX* ctx, bytea *m) {
  m->str = NULL;
  m->len = 0;
}

template <class Context>
bool SVMPredict(Context* ctx, const bytea &ex, const bytea &model) {
  size_t model_len, v_len;
  double *modelp, *vp;
  CoerceBytea(model, modelp, model_len);
  CoerceBytea(ex, vp, v_len);
  double pred = hazy::simple_dot(modelp, vp, model_len);
  return pred > 0;
}

} // namespace bismarck
} // namespace hazy
#endif
