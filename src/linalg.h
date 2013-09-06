
#ifndef HAZY_BISMARCK_LINALG_H
#define HAZY_BISMARCK_LINALG_H

#include <cmath>

#include <algorithm>

namespace hazy {


/*! \brief Computes the dot product of two vectors
 */
template <class T>
T simple_dot(const T *x, const T *y, size_t len);


/*! \brief Preforms a simple scale-and-add operation
 * This is x = x + a * y
 */
template <class T, class V>
void simple_scale_add(T *x, const T *y, V a, size_t len);


/*! \brief Preforms a simple scale-and-add operation
 * This is x = a * x 
 */
template <class T, class V>
void simple_scale(T *x, V a, size_t len);



} // namespace hazy
#endif

