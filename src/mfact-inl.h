
#ifndef IMPALA_BISMARCK_MFACT_INL_H
#define IMPALA_BISMARCK_MFACT_INL_H

#include "bismarck-inl.h"
#include "linalg-inl.h"

namespace madlib {
namespace port {

using namespace impala;
using namespace impala_udf;
using namespace std;
using namespace impala::bismarck;

/*! returns the size (in bytes) of the matrix factorization model
 */
size_t SizeOfMFModel(size_t rows, size_t cols, size_t rank) {
  return sizeof(double) * 3 + sizeof(double) * (rows * rank + cols * rank);
}

/*! Gets pointers into the model where the rows, cols, and rank are stored
 * This works on an unitialized model
 */
void MFMetaPointers(void* model, uint64_t *&rows, 
                    uint64_t *&cols, uint64_t *&rank) {
  uint64_t *arr = reinterpret_cast<uint64_t*>(model);
  rows = &arr[0];
  cols = &arr[1];
  rank = &arr[2];
}

/*! Gets pointers to L and R, only use on a model that has rows, cols, and rank
 * already set.
 */
void MFLRPointers(void* model, double *&L, double *&R) {
  uint64_t *rank, *cols, *rows;
  MFMetaPointers(model, rows, cols, rank);

  double *arr = reinterpret_cast<double*>(model);
  size_t len_L = *rows * *rank;
  // offset by 3 because row, cols, and rank are first 24 bytes. 
  L = &arr[3];
  R = &arr[3 + len_L];
}

void InitModel(UdfContext* ctx, StringVal *m, size_t rows, size_t cols, 
               size_t rank) {
  // Allocate room to store rank, rows & cols, and the L & R matrix
  
  m.ptr = ctx->Allocate(SizeOfMFModel(rows, cols, rank));
  uint64_t *rank_, *cols_, *rows_;
  MFMetaPointers(m.ptr, rows_, cols_, rank_);
  *rank_ = rank;
  *rows_ = rows;
  *cols_ = cols;

  // TODO zero model
}

double MFLoss(const StringVal &v, const BooleanVal &y,
                    const StringVal &model) {
  // TODO
}

void MFStep(UdfContext* ctx, 
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
  }

  long rowi = e.row;
  long coli = e.col;

  vector::FVector<double> &Li = m.L[rowi];
  vector::FVector<double> &Rj = m.R[coli];

  double err = vector::Dot(Li, Rj) + task.mean - e.value;
  double er = -(task.step_size * err);

  double scal = 1 - task.mu * task.step_size / task.L_degree[rowi];

  double swap_[task.rank];
  vector::FVector<double> swapL(task.rank, swap_);

  vector::ScaleInto(Li, scal, swapL);
  vector::ScaleAndAdd(swapL, Rj, er);
  double scal_r = 1 - task.mu * task.step_size / task.R_degree[coli];
  vector::Scale(Rj, scal_r);
  ScaleAndAdd(Rj, Li, er);
  vector::CopyInto(swapL, Li);
  double *model = (double*)input->ptr;
  size_t model_len = input->len;
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
