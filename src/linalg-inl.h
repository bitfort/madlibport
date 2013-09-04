
#ifndef HAZY_BISMARCK_LINALG_INL_H
#define HAZY_BISMARCK_LINALG_INL_H

#include "linalg.h"

namespace hazy {

template <class T>
T simple_dot(const T *x, const T *y, size_t len) {
  T prod = 0;
  for (size_t i = 0; i < len; i++) {
    prod += x[i] * y[i];
  }
  return prod;
}

template <class T, class V>
void simple_scale_add(T *x, const T *y, V a, size_t len) {
  for (size_t i = 0; i < len; i++)
    x[i] += a * y[i];
}

template <class T, class V>
void simple_scale(T *x, V a, size_t len) {
  for (size_t i = 0; i < len; i++)
    x[i] *= a;
}

} // namespace hazy
#endif
