/* ----------------------------------------------------------------------- *//**
 *
 * @file dbconnector.hpp
 *
 * @brief This file should be included by user code (and nothing else)
 *
 *//* ----------------------------------------------------------------------- */

#ifndef MADLIB_mainmem_DBCONNECTOR_HPP
#define MADLIB_mainmem_DBCONNECTOR_HPP

// On platforms based on mainmemQL we can include a different set of headers.
// Also, there may be different compatibility headers for other platforms.
#ifndef MADLIB_mainmem_HEADERS
#define MADLIB_mainmem_HEADERS

#include <inttypes.h>
#define int32 int32_t



/*
extern "C" {
    #include <funcapi.h>
    #include <catalog/pg_proc.h>
    #include <catalog/pg_type.h>
    #include <executor/executor.h> // For GetAttributeByNum()
    #include <miscadmin.h>         // Memory allocation, e.g., HOLD_INTERRUPTS
    #include <utils/acl.h>
    #include <utils/array.h>
    #include <utils/builtins.h>    // needed for format_procedure()
    #include <utils/datum.h>
    #include <utils/lsyscache.h>   // for type lookup, e.g., type_is_rowtype
    #include <utils/memutils.h>
    #include <utils/syscache.h>    // for direct access to catalog, e.g., SearchSysCache()
    #include <utils/typcache.h>    // type conversion, e.g., lookup_rowtype_tupdesc
    #include "../../../../methods/svec/src/pg_gp/sparse_vector.h" // Legacy sparse vectors
} // extern "C"
*/

//#include "Compatibility.hpp"

#endif // !defined(MADLIB_mainmem_HEADERS)

//*****************************************************************************
// Note: These are needed for Porting from PSQL
// These are defined by Postgres header files
#define VARHDRSZ   ((int32_t) sizeof(int32_t))
#define MAXIMUM_ALIGNOF 8
#define ALIGNOF_SHORT 2
#define ALIGNOF_INT 4
#define ALIGNOF_DOUBLE 8

//
//*****************************************************************************

// Unfortunately, we have to clean up some #defines in mainmemQL headers. They
// interfere with C++ code.
// From c.h:

// Note: If errors occur in the following include files, it could indicate that
// new macros have been added to mainmemQL header files.
#include <boost/mpl/if.hpp>
#include <boost/any.hpp>
#include <boost/type_traits/is_const.hpp>
#include <boost/type_traits.hpp>
#include <boost/type_traits/remove_cv.hpp>
#include <boost/math/special_functions/fpclassify.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/tr1/functional.hpp>
#include <boost/tr1/array.hpp>
#include <boost/tr1/tuple.hpp>
#include <limits>
#include <stdexcept>
#include <vector>
#include <fstream>

#include <dbal/dbal_proto.hpp>
#include <utils/Reference.hpp>
#include <utils/Math.hpp>

namespace madlib {
template <class T>
struct MemHandle {
  size_t size;
  T* ptr;
};
}



namespace std {
    // Import names from TR1.

    // The following are currently provided by boost.
    using tr1::array;
    using tr1::bind;
    using tr1::function;
    using tr1::get;
    using tr1::make_tuple;
    using tr1::tie;
    using tr1::tuple;
}
// XXX ADDED BY VICTOR
#include "dbal/EigenIntegration/EigenIntegration.hpp"

#if 0
#if !defined(NDEBUG) && !defined(EIGEN_NO_DEBUG)
#define eigen_assert(x) \
    do { \
        if(!Eigen::internal::copy_bool(x)) \
            throw std::runtime_error(std::string( \
                "Internal error. Eigen assertion failed (" \
                EIGEN_MAKESTRING(x) ") in function ") + __PRETTY_FUNCTION__ + \
                " at " __FILE__ ":" EIGEN_MAKESTRING(__LINE__)); \
    } while(false)
#endif // !defined(NDEBUG) && !defined(EIGEN_NO_DEBUG)
#endif

// We need to make _oldContext volatile because if an exception occurs, the
// register holding its value might have been overwritten (and the variable
// value is read from in the PG_CATCH block). On the other hand, no need to make
// _errorData volatile: If no PG exception occurs, no register will be
// overwritten. If an exception occurs, _errorData will be set before it is read
// again.
#define MADLIB_PG_TRY \
    do { \
        volatile MemoryContext _oldContext \
            = CurrentMemoryContext; \
        ErrorData* _errorData = NULL; \
        PG_TRY();

// CopyErrorData() copies into the current memory context, so we
// need to switch away from the error context first
#define MADLIB_PG_CATCH \
        PG_CATCH(); { \
            MemoryContextSwitchTo(_oldContext); \
            _errorData = CopyErrorData(); \
            FlushErrorState(); \
        } PG_END_TRY(); \
        if (_errorData)

#define MADLIB_PG_END_TRY \
    } while(false)

#define MADLIB_PG_RE_THROW \
    throw PGException(_errorData)

#define MADLIB_PG_ERROR_DATA() \
    _errorData

#define MADLIB_PG_DEFAULT_CATCH_AND_END_TRY \
    MADLIB_PG_CATCH { \
        MADLIB_PG_RE_THROW; \
    } MADLIB_PG_END_TRY

#define MADLIB_WRAP_PG_FUNC(_returntype, _pgfunc, _arglist, _passedlist) \
inline \
_returntype \
madlib ## _ ## _pgfunc _arglist { \
    _returntype _result = static_cast<_returntype>(0); \
    MADLIB_PG_TRY { \
        _result = _pgfunc _passedlist; \
    } MADLIB_PG_DEFAULT_CATCH_AND_END_TRY; \
    return _result; \
}

#define MADLIB_WRAP_VOID_PG_FUNC(_pgfunc, _arglist, _passedlist) \
inline \
void \
madlib ## _ ## _pgfunc _arglist { \
    MADLIB_PG_TRY { \
        _pgfunc _passedlist; \
    } MADLIB_PG_DEFAULT_CATCH_AND_END_TRY; \
}

/**
 * The maximum number of arguments that can be passed to a function via a
 * FunctionHandle.
 * Note that mainmemQL defines FUNC_MAX_ARGS, which we could use here. However,
 * this can be a fairly large number that might exceed BOOST_PP_LIMIT_REPEAT.
 * In fmgr.h, mainmemQL provides support functions (FunctionCall*Coll) for only
 * up to 9 arguments.
 */
#define MADLIB_FUNC_MAX_ARGS 9

/**
 * The maximum number of dimensions in an array
 */
#define MADLIB_MAX_ARRAY_DIMS 2
/* FIXME */


////////////////////////////// METAPORT //////////////////////////////
namespace madlib {
// Import the arary Type from the Meta Port
typedef madlib::port::dbconn::PortArrayType ArrayType;
typedef madlib::port::dbconn::PortAllocator PortAllocator;

/*
struct ArrayType {
  size_t len;
  size_t ndims;
  size_t dims[10];
  void* ptr;
};
*/


/*! Returns an unsigned int which is the number of dimensions of the array
 */
#define ARR_NDIM(a) (madlib::port::dbconn::ArrayNDims(a))
/*! Returns a subscriptable thing which will give the length of the given dim,
 * e.g. ARR_DIMS(a)[1] is the length of the 2nd dimension. 
 */
#define ARR_DIMS(a) (madlib::port::dbconn::ArrayDims(a))
/*! Returns a void* to actual memory
 */
#define ARR_DATA_PTR(a) (madlib::port::dbconn::ArrayPointer(a))

/*! Returns the storage space necessary for the array
 */
#define ARR_OVERHEAD_NONULLS(a) (sizeof(ArrayType))

/*! Import the allocator
 * Assignment, 
 * operator = (const PortAllocator&)
 * void* Allocate(nbytes)
 * void Free(void*)
 * Reallocate(void *inPtr, const size_t inSize)
 *
 */
typedef madlib::port::dbconn::PortAllocator PortAllocator;
// TODO FIXME
typedef int* MemoryContext;
}
////////////////////////////// METAPORT //////////////////////////////


#include "ByteString_proto.hpp"
//#include "NativeRandomNumberGenerator_proto.hpp"
//#include "PGException_proto.hpp"
//#include "OutputStreamBuffer_proto.hpp"
//#include "SystemInformation_proto.hpp"
//#include "TransparentHandle_proto.hpp"
//#include "TypeTraits_proto.hpp"
//#include "UDF_proto.hpp"
#include "dbconnector/TypeTraits_proto.hpp"
#include "dbconnector/AnyType_proto.hpp"
#include "dbconnector/ArrayHandle_proto.hpp"
#include "dbconnector/TransparentHandle_proto.hpp"
#include "dbconnector/Allocator_proto.hpp"
// Need to move FunctionHandle down because it has dependencies
//#include "FunctionHandle_proto.hpp"

// Several backend functions (APIs) need a wrapper, so that they can be called
// safely from a C++ context.
//#include "Backend.hpp"

namespace madlib {

using dbconnector::mainmem::Allocator;
using dbconnector::mainmem::AnyType;
using dbconnector::mainmem::ArrayHandle;
using dbconnector::mainmem::TransparentHandle;
using dbconnector::mainmem::MutableArrayHandle;
// Import MADlib types into madlib namespace
using dbconnector::mainmem::AnyType;
using dbconnector::mainmem::ByteString;
//using dbconnector::mainmem::FunctionHandle;
using dbconnector::mainmem::MutableArrayHandle;
using dbconnector::mainmem::MutableByteString;
//using dbconnector::mainmem::NativeRandomNumberGenerator;

// Import MADlib functions into madlib namespace
using dbconnector::mainmem::AnyType_cast;
using dbconnector::mainmem::defaultAllocator;
//using dbconnector::mainmem::funcPtr;
using dbconnector::mainmem::Null;

namespace dbconnector {

namespace mainmem {

extern std::ostream dbout;
//extern std::ostream dberr;

} // namespace mainmem

} // namespace dbconnector

// Import MADlib global variables into madlib namespace
using dbconnector::mainmem::dbout;
std::string errstr;
std::stringstream dberr;
//using dbconnector::mainmem::dberr;

} // namespace madlib

#include <dbal/dbal_impl.hpp>

// FIXME: The following include should be further up. Currently dependent on
// dbal_impl.hpp which depends on the memory allocator.
#include "EigenIntegration_proto.hpp"

#include "TypeTraits_impl.hpp"
#include "AnyType_impl.hpp"
#include "Allocator_impl.hpp"
#include "ArrayHandle_impl.hpp"
#include "ByteString_impl.hpp"
#include "EigenIntegration_impl.hpp"
//#include "FunctionHandle_impl.hpp"
//#include "NativeRandomNumberGenerator_impl.hpp"
//#include "OutputStreamBuffer_impl.hpp"
#include "TransparentHandle_impl.hpp"
#include "TypeTraits_impl.hpp"
//#include "UDF_impl.hpp"
//#include "SystemInformation_impl.hpp"

namespace madlib {

typedef dbal::DynamicStructRootContainer<
    ByteString, dbconnector::mainmem::TypeTraits> RootContainer;
typedef dbal::DynamicStructRootContainer<
    MutableByteString, dbconnector::mainmem::TypeTraits> MutableRootContainer;

} // namespace madlib

// TODO FIXME

#define DECLARE_UDF(_module, _name)  \
    namespace madlib { \
    namespace modules { \
    namespace _module { \
    struct _name : public Allocator/* :  public dbconnector::mainmem::UDF */ { \
        inline _name() { }  \
        AnyType run(AnyType &args); \
        inline void *SRF_init(AnyType&) {return NULL;}; \
        inline AnyType SRF_next(void *, bool *){return AnyType();}; \
    }; \
    } \
    } \
    }

#define DECLARE_SR_UDF(_module, _name) \
    namespace madlib { \
    namespace modules { \
    namespace _module { \
    struct _name : public dbconnector::mainmem::UDF { \
        inline _name() { }  \
        inline AnyType run(AnyType &){return AnyType();}; \
        void *SRF_init(AnyType &args); \
        AnyType SRF_next(void *user_fctx, bool *is_last_call); \
    }; \
    } \
    } \
    }

#define DECLARE_UDF_EXTERNAL(_module, _name) \
    namespace external { \
        extern "C" { \
            PG_FUNCTION_INFO_V1(_name); \
            Datum _name(PG_FUNCTION_ARGS) { \
                return madlib::dbconnector::mainmem::UDF::call< \
                    madlib::modules::_module::_name>(fcinfo); \
            } \
        } \
    }

#endif // defined(MADLIB_mainmem_DBCONNECTOR_HPP)
