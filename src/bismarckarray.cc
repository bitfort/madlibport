
#ifndef HAZY_BISMARCK_ARRAY_H
#define HAZY_BISMARCK_ARRAY_H

#define IMPALA_UDF_SDK_BUILD 1

#include <cstdio>
#include "udf/udf.h"
#include <string>
#include <sstream>
#include <iostream>

using namespace impala;
using namespace impala_udf;
using namespace std;


StringVal ToArray(FunctionContext* context, int n, DoubleVal *ints) {
  StringVal s(context, n * sizeof(double));
  double *darr = reinterpret_cast<double*>(s.ptr);
  for (int i = 0; i < n; i++) {
    darr[i] = ints[i].val;
  }
  return s;
}

StringVal PrintArray(FunctionContext* context, const StringVal& arr) {
  double *darr = reinterpret_cast<double*>(arr.ptr);
  int len = arr.len / sizeof(double);
  stringstream ss;
  ss << "<";
  for (int i = 0; i < len; ++i) {
    if (i != 0) ss << ", ";
    ss << darr[i];
  }
  ss << ">";
  string str = ss.str();
  StringVal result(context, str.size());
  memcpy(result.ptr, str.c_str(), str.size());
  return result;
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
