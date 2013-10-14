
#ifndef HAZY_BISMARCK_ARRAY_H
#define HAZY_BISMARCK_ARRAY_H

#define IMPALA_UDF_SDK_BUILD 1

#include <cstdio>
#include "udf/udf.h"

using namespace impala;
using namespace impala_udf;


StringVal ToArray(FunctionContext* context, int n, DoubleVal *ints) {
  StringVal s(context, n * sizeof(double));
  double *darr = reinterpret_cast<double*>(s.ptr);
  for (int i = 0; i < n; i++) {
    darr[i] = ints[i].val;
  }
  return s;
}

DoubleVal ArrayGet(FunctionContext* context, const BigIntVal &n, const StringVal &arr) {
  size_t i = n.val;
  double *darr = reinterpret_cast<double*>(arr.ptr);
  return DoubleVal(darr[i]);
}

StringVal AllBytes(FunctionContext* context) {
  StringVal s(context, 256);

  for (int i = 0; i < 256; i++) {
    s.ptr[i] = i;
  }
  return s;
}

#endif
