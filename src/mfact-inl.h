
#ifndef IMPALA_BISMARCK_MFACT_INL_H
#define IMPALA_BISMARCK_MFACT_INL_H

#include "linalg-inl.h"

// see for documentation
#include "mfact.h"

namespace hazy {
namespace bismarck {

/*! returns the size (in bytes) of the matrix factorization model
 */
size_t SizeOfMFModel(size_t rows, size_t cols, size_t rank) {
  return sizeof(double) * 4 + sizeof(double) * (rows * rank + cols * rank);
}

/*! Gets pointers into the model where the rows, cols, and rank are stored
 * This works on an unitialized model
 */
void MFMetaPointers(void* model, uint64_t *&rows, 
                    uint64_t *&cols, uint64_t *&rank, double *&mean) {
  uint64_t *arr = reinterpret_cast<uint64_t*>(model);
  double *darr = reinterpret_cast<double*>(model);
  rows = &arr[0];
  cols = &arr[1];
  rank = &arr[2];
  mean = &darr[3];
}

/*! Gets pointers to L and R, only use on a model that has rows, cols, and rank
 * already set.
 */
void MFLRPointers(void* model, double *&L, double *&R) {
  uint64_t *rank, *cols, *rows;
  double *mean;
  MFMetaPointers(model, rows, cols, rank, mean);

  double *arr = reinterpret_cast<double*>(model);
  size_t len_L = *rows * *rank;
  // offset by 3 because row, cols, and rank are first 24 bytes. 
  L = &arr[4];
  R = &arr[4 + len_L];
}

template <class CTX>
void InitModel(CTX* ctx, bytea *m, size_t rows, size_t cols, 
               size_t rank, double mean) {
  // Allocate room to store rank, rows & cols, and the L & R matrix
  
  m->str = BismarckAllocate<char>(ctx, SizeOfMFModel(rows, cols, rank));
  uint64_t *rank_, *cols_, *rows_;
  double *mean_;
  MFMetaPointers(m->str, rows_, cols_, rank_, mean_);
  *rank_ = rank;
  *rows_ = rows;
  *cols_ = cols;
  *mean_ = mean;

  double *L, *R;
  MFLRPointers(m->str, L, R);

  // TODO zero model
  for (size_t i = 0; i < (rows * rank); i++) {
    L[i] = 0.01;
  }
  for (size_t i = 0; i < (cols * rank); i++) {
    R[i] = 0.01;
  }

}

template <class CTX>
double BismarckMF<CTX>::Loss(size_t row, size_t col, double val,
                     const bytea &model) {
  // TODO
  return 0.0;
}

/*! Takes an IGD step for matrix factorization
 */
void MFIGDStep(double *Li, double *Ri, double val, 
               double mean, double mu, double step, size_t rank,
               size_t row_deg, size_t col_deg, double *swap) {
  // use the model to predict val 
  double pred = simple_dot(Li, Ri, rank) + mean;

  // determine how far off our predicted value is from the actual value
  double err = pred - val;

  // save a copy of L because L will be used when updaing R
  memcpy(swap, Li, sizeof(double) * rank);

  // take an IGD step with L
  double scal_L = 1 - mu * step / row_deg;
  simple_scale(Li, scal_L, rank);
  simple_scale_add(Li, Ri, -1 * step * err, rank);

  // take an IGD stpe with R
  double scal_R = 1 - mu * step / col_deg;
  simple_scale(Ri, scal_R, rank);
  simple_scale_add(Ri, swap, -1 * step * err, rank);
}


template <class CTX>
void BismarckMF<CTX>::Step(CTX* ctx, 
                   size_t row, size_t col, double val,
                   bytea *mod, double step, double mu, double mean, 
                   size_t rank, const bytea &rowdeg, const bytea &coldeg) {
  size_t nrows, ncols;
  uint64_t *rowd, *cold;
  CoerceBytea(rowdeg, rowd, nrows);
  CoerceBytea(coldeg, cold, ncols);

  // left and right matrix, row major
  // L is nrows by rank; R is ncols by rank
  double *L, *R;

  // Check of model is null and needs to be set
  if (mod->str == NULL) {
    InitModel(ctx, mod, nrows, ncols, rank, mean);
  }

  // set L and R to point into model
  MFLRPointers(mod->str, L, R);

  // extract the row and column from L and R we are going to modify, both 
  // are of length rank
  double *Li = &L[row * rank];
  double *Ri = &R[col * rank];

  // we need some swap space for the update
  double *swap = BismarckAllocate<double>(ctx, rank);

  MFIGDStep(Li, Ri, val, mean, mu, step, rank, rowd[row], cold[col], swap);
  // TODO free swap
  
}

template <class CTX>
void BismarckMF<CTX>::Merge(CTX* ctx, const bytea& src, 
                   bytea* dst) {
  // take the mean of the models

}

template <class CTX>
void BismarckMF<CTX>::Init(CTX* ctx, bytea *m) {
  m->str = NULL;
  m->len = 0;
}


template <class CTX>
double MFPredict(CTX* ctx, const bytea &model, size_t row, size_t col) {
  // left and right matrix, row major
  // L is nrows by rank; R is ncols by rank
  double *L, *R;
  uint64_t *rank, *cols, *rows;
  double *mean;

  // set L and R to point into model
  MFLRPointers(model.str, L, R);
  MFMetaPointers(model.str, rows, cols, rank, mean);

  // extract the row and column from L and R we are going to modify, both 
  // are of length rank
  double *Li = &L[row * *rank];
  double *Ri = &R[col * *rank];

  // use the model to predict val 
  double pred = simple_dot(Li, Ri, *rank) + *mean;
  return pred;
}


template <class CTX>
bytea MFStrL(CTX* ctx, const bytea &model) {
  uint64_t *rank, *cols, *rows;
  double *mean;
  MFMetaPointers(model.str, rows, cols, rank, mean);

  double *L, *R;

  // set L and R to point into model
  MFLRPointers(model.str, L, R);

  printf("L = \n");
  for (size_t i = 0; i < *rows; i++) {
    for (size_t j = 0; j < *rank; j++) {
      printf("%f ", L[i * *rank + j]);
    }
    printf("\n");
  }
}

template <class CTX>
bytea MFStrR(CTX* ctx, const bytea &model) {
  uint64_t *rank, *cols, *rows;
  double *mean;
  MFMetaPointers(model.str, rows, cols, rank, mean);

  double *L, *R;

  // set L and R to point into model
  MFLRPointers(model.str, L, R);

  printf("R = \n");
  for (size_t i = 0; i < *cols; i++) {
    for (size_t j = 0; j < *rank; j++) {
      printf("%f ", R[i * *rank + j]);
    }
    printf("\n");
  }
}


} // namespace port
} // namespace madlib
#endif
