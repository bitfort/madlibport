

#ifndef MADLIB_IMPALA_BISMARCK_INL_H
#define MADLIB_IMPALA_BISMARCK_INL_H

#include <cmath>
#include "bismarck.h"

namespace impala {
namespace bismarck {

// Bismarck UDA

void BismarckInit(UdfContext* ctx, BismarckModel_t* st) {
  st->ptr = NULL;
  st->len = 0;
}

template <typename INPUT1, 
          void (*INIT)(UdfContext*, BismarckModel_t*),
          void (*STEP)(UdfContext*, const INPUT1&, BismarckModel_t*)>
void BismarckStep(UdfContext* ctx, const BismarckModel_t& prev, 
                  const INPUT1& in, 
                  BismarckModel_t* model) {
  // If first tuple, the model will be NULL
  if (model->ptr == NULL) {
    if (prev.ptr == NULL) {
      // Case #1: we have no previous model, so we need an initial model
      INIT(ctx, model);
    } else {
      // Case #2: we have a previous model to seed from
      new (model) StringVal(ctx, prev.len);
      memcpy(model->ptr, prev.ptr, prev.len);
    }
  }

  // take a gradient step and update the model
  STEP(ctx, in, model);
}

template <typename INPUT1, 
          typename INPUT2,
          void (*INIT)(UdfContext*, BismarckModel_t*),
          void (*STEP)(UdfContext*, const INPUT1&, const INPUT2&,
                       BismarckModel_t*)>
void BismarckStep2(UdfContext* ctx, const BismarckModel_t& prev, const INPUT1& in, 
                   const INPUT2& in2,
                  BismarckModel_t* model) {
  // If first tuple, the model will be NULL
  if (model->ptr == NULL) {
    if (prev.ptr == NULL) {
      // Case #1: we have no previous model, so we need an initial model
      INIT(ctx, model);
    } else {
      // Case #2: we have a previous model to seed from
      new (model) StringVal(ctx, prev.len);
      memcpy(model->ptr, prev.ptr, prev.len);
    }
  }
  // take a gradient step and update the model
  STEP(ctx, in, in2, model);
}



BismarckModel_t BismarckFinal(UdfContext* ctx, const BismarckModel_t& mod) {
  StringVal out(ctx, mod.len);
  memcpy(out.ptr, mod.ptr, mod.len);
  return out;
}

template <void (*MERGE)(UdfContext*, const BismarckModel_t&, BismarckModel_t*)>
void BismarckMerge(UdfContext* ctx, const BismarckModel_t& src, BismarckModel_t* dst) {
  if (src.is_null)
    return;
  if (dst->is_null) {
    new (dst) StringVal(ctx, src.len);
    memcpy(dst->ptr, src.ptr, src.len);
    return;
  }

  MERGE(ctx, src, dst);
}


//
// Losss UDA
//


void BismarckLossInit(UdfContext* ctx, BismarckLoss_t* st) {
  double snan = NAN;
  st->val = snan;
}

template <typename INPUT1, 
          double (*LOSS)(const INPUT1&, const BismarckModel_t&),
          double (*MERGE)(double, double)>
void BismarckLossStep(UdfContext* ctx, const BismarckModel_t& mod, 
                      const INPUT1& in, BismarckLoss_t* loss) {
  double ex_loss = LOSS(in, mod);
  if ( std::isnan(loss->val) ) {
    loss->val = ex_loss;
  } else {
    loss->val = MERGE(loss->val, ex_loss);
  }
}


template <typename INPUT1, 
          typename INPUT2,
          double (*LOSS)(const INPUT1&, const INPUT2&, 
                         const BismarckModel_t&),
          double (*MERGE)(double, double)>
void BismarckLossStep(UdfContext* ctx, const BismarckModel_t& mod, 
                      const INPUT1& in, const INPUT2& in2,
                      BismarckLoss_t* loss) {
  double ex_loss = LOSS(in, in2, mod);
  if ( isnan(loss->val) ) {
    loss->val = ex_loss;
  } else {
    loss->val = MERGE(loss->val, ex_loss);
  }
}


BismarckLoss_t BismarckLossFinal(UdfContext* ctx, const BismarckLoss_t& loss) {
  return loss;
}

template <void (*MERGE)(double , double)>
void BismarckLossMerge(UdfContext*, const DoubleVal & src, DoubleVal *dst) {
  dst->val = MERGE(src.val, dst->val);
}


} // namespace bismarck
} // namespace impala

#endif
