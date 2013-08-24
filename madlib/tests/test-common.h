
#define EIGEN_YES_I_KNOW_SPARSE_MODULE_IS_NOT_STABLE_YET
#include <cstdio>

#include <Eigen/Dense>
#include <Eigen/Sparse>

namespace madlib {
namespace port {
namespace dbconn {


struct PortAllocator { 
  void* Allocate(size_t s) const { return malloc(s); }
  void Free(void*v) const { free(v); }
  void* Realloc(void* p, size_t s) const { return realloc(p, s); } 
};

struct PortArrayType {
  size_t len;
  size_t ndims;
  size_t dims[10];
  void* ptr;
};

size_t* ArrayDims(const PortArrayType *a) {
  // ugly comptability hack to work with MADlib
  PortArrayType *a_nonc = const_cast<PortArrayType*>(a);
  return a_nonc->dims;
}
size_t ArrayNDims(const PortArrayType *a) {
  return a->ndims;
}

void* ArrayPointer(const PortArrayType *a) {
  return a->ptr;
}

template <class T> 
PortArrayType MakeArray(T* v, size_t l) {
  PortArrayType arr;
  arr.len = l*sizeof(T);
  arr.ptr = (void*) v;
  arr.ndims = 1;
  arr.dims[0] = l;
}

}
}
}


// include the files we will be using
#include <dbconnector/dbconnector.hpp>

using namespace madlib;
using madlib::port::dbconn::MakeArray;

// Execute a test case; result returned as a boolean
#define RUNTEST(blah) \
    if (blah()) { printf("%-10s " # blah "\n", "ok"); } \
    else { printf("%-10s " # blah "\n", "FAIL**"); }


#define EXPECT_EQ(a, b) \
    if (a != b) { \
      std::cout << "Expected (" # b ") " << b << "   got (" # a ") " << a << std::endl; \
      return 0; \
    }

#define EXPECT_NEAR(a, b, tol) \
    if ( std::abs(a - b) > tol) { \
      std::cout << "Expected (" # b ") " << b << "   got (" # a ") " << a << std::endl; \
      return 0; \
    }


#define RUN_TEST_SET(name) \
    printf("    --------- " # name "\n"); \
    name();


