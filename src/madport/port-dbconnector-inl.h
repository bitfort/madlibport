// port of MADlib to Imapala
// author: victor bittorf (bittorf@cs.wisc.edu)

/* MADlib uses a dbconnector as a backend for their C++ UDFs. This
 * file uses the metaport/dbconnector to provide a backend 
 * when porting to impala. Two types must be implemented here:
 * PortArrayType and PortAllactor. 
 */

// flag taken from MADlib codebaes
#define EIGEN_YES_I_KNOW_SPARSE_MODULE_IS_NOT_STABLE_YET
#include <cstdio>


// 3rd party libraries for matrix/vector math
#include <Eigen/Dense>
#include <Eigen/Sparse>


// Our Port types, which the dbconnector will need
// must be imported before dbconnector

// Introduces madlib::port::dbconn::PortAllocator
#include "port-allocator-inl.h"
// Introduces madlib::port::dbconn::PortArrayType
#include "port-arraytype-inl.h"

namespace madlib {
namespace port {
namespace dbconn {

template <class T> 
PortArrayType MakeArray(T* v, size_t l) {
  PortArrayType arr;
  arr.len = l*sizeof(T);
  arr.ptr = (void*) v;
  arr.ndims = 1;
  arr.dims[0] = l;
  return arr;
}

}
}
}


// includes a large chunk of MADlib C++ Code
// Also, boost and Eigen header files
// see ports/metaport/dbconnector/
// Note: this include may conflict with gtest, no known fix
#include <dbconnector/dbconnector.hpp>

// Macros for running tests
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
