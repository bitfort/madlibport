
#ifndef HAZY_BISMARCK_LOGISTIC_INL_H
#define HAZY_BISMARCK_LOGISTIC_INL_H

#include "linalg-inl.h"

// see for documentation
#include "logreg.h"

namespace hazy {
namespace bismarck {

template <class CTX>
double BismarckLogr<CTX>::Loss(const bytea &v, 
                     bool y,
                     const bytea &model) {
  size_t model_len, v_len;
  double *modelp, *vp;
  CoerceBytea(model, modelp, model_len);
  CoerceBytea(v, vp, v_len);
  double pred = hazy::simple_dot(modelp, vp, model_len);
  double lbl = y ? 1 : -1;

  double loss = std::log(1 + std::exp(-1.0 * lbl * pred));
  return loss;
}

template <class CTX>
void BismarckLogr<CTX>::Step(CTX* ctx, 
                   const bytea& val, const bool &y,
                   bytea *input, double step, double mu) {
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
  double ebx = std::exp(-1.0 * pred);

  double scal = (- 1.0 / (1.0 + ebx) + lbl);

  hazy::simple_scale_add(model, v, step * scal, model_len);
}

template <class CTX>
void BismarckLogr<CTX>::Merge(CTX* ctx, const bytea& src, const bytea &dst) {
  // TODO check if dst is initlaized, check if src is intilaized
  // take the mean of the models
  hazy::simple_scale((double*) dst.str, dst.len/sizeof(double), 0.5);
  hazy::simple_scale_add((double*) dst.str, (double*) src.str, 0.5, 
                         dst.len/sizeof(double));
}

template <class CTX>
void BismarckLogr<CTX>::Init(CTX* ctx, bytea *m) {
  m->str = NULL;
  m->len = 0;
}

/*! \brief Predicts the label for the example using the given model
 */
template <class Context>
bool _LogrPredict(Context* ctx, const bytea &ex, const bytea &model) {
  size_t model_len, v_len;
  double *modelp, *vp;
  CoerceBytea(model, modelp, model_len);
  CoerceBytea(ex, vp, v_len);
  double pred = hazy::simple_dot(modelp, vp, model_len);
  double loss = 1.0 / (1.0 + std::exp(-1.0 * pred)); 
  return loss > 0.5;
}


} // namespace port
} // namespace madlib
#endif

