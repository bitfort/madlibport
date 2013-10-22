
#include <cstdio>

#include "udf/udf.h"

using namespace impala;
using namespace impala_udf;

template <class T>
T* BismarckAllocate(FunctionContext* ctx, size_t len) {
  len *= sizeof(T);
  return (T*) ctx->Allocate(len);
}


#include "bismarck.h"
#include "logreg-inl.h"
//#include "udas/logistic.h"


using namespace hazy::bismarck;


bytea StringValToBytea(const StringVal &v) {
  bytea ba;
  ba.str = (char*) v.ptr;
  ba.len = v.len;
  return ba;
}


void LogrInit(FunctionContext* ctx, StringVal *model) {
  printf("SVMInit\n");
  // TODO
  model->ptr = NULL;
  model->len = 0;
}

void LogrUpdate(FunctionContext* ctx,  const StringVal &prev_model, 
             const StringVal &ex, const BooleanVal &label, const DoubleVal &step_size,
             const DoubleVal &mu, StringVal *model) {

  printf("LogrStep\n");
  // If first tuple, the model will be NULL
  if (model->ptr == NULL) {
    if (prev_model.ptr != NULL) {
      printf("No Model, copying from old one.\n");
      // Case #2: we have a previous model to seed from
      new (model) StringVal(ctx, prev_model.len);
      memcpy(model->ptr, prev_model.ptr, prev_model.len);
    }
  }

  // Take the gradietn step
  bytea modela = StringValToBytea(*model);
  BismarckLogr<FunctionContext>::Step(ctx, 
                                     StringValToBytea(ex),
                                     label.val,
                                     &modela,
                                     step_size.val,
                                     mu.val);
  model->ptr = (uint8_t*) modela.str;
  model->len = modela.len;
  printf("Length of model returend is: %lu\n", model->len);
}

void LogrMerge(FunctionContext* ctx, const StringVal &src,
              StringVal *dst) {
  printf("SVMMerge src.size=%lu dst->size=%lu\n", src.len, dst->len);
  new (dst) StringVal(ctx, src.len);
  memcpy(dst->ptr, src.ptr, src.len);
  return;
}

StringVal LogrFinalize(FunctionContext* ctx, const StringVal &model) {
  StringVal s(ctx, model.len);
  s = model;
  printf("SVMFinal size=%lu\n", s.len);
  return s;
}

BooleanVal LogrPredict(FunctionContext* ctx, const StringVal &model, const StringVal &ex) {
  BooleanVal r;

  bytea mod = StringValToBytea(model);
  bytea e = StringValToBytea(ex);

  r.val = _LogrPredict(ctx, e, mod);
  return r;
}

DoubleVal LogrLoss(FunctionContext* ctx, const StringVal &model, const StringVal &ex, const BooleanVal &lbl) {

  printf("Enter LogrLoss w/ len(model)=%lu len(ex)=%lu and lbl=%d\n", model.len, ex.len, lbl.val);
  DoubleVal r;

  bytea mod = StringValToBytea(model);
  bytea e = StringValToBytea(ex);

  r.val = BismarckLogr<FunctionContext>::Loss(e, lbl.val, mod);
  return r;
}


