/* ----------------------------------------------------------------------- *//**
 *
 * @file PGException_proto.hpp
 *
 *//* ----------------------------------------------------------------------- */

#ifndef MADLIB_mainmem_PGEXCEPTION_PROTO_HPP
#define MADLIB_mainmem_PGEXCEPTION_PROTO_HPP

namespace madlib {

namespace dbconnector {

namespace mainmem {

/**
 * @brief Unspecified mainmemQL backend expcetion
 */
class PGException : public std::runtime_error {
public:
    explicit 
    PGException()
      : std::runtime_error("The backend raised an exception.") { }
    
    // FIXME: Do something useful with inErrorData
    PGException(ErrorData* /* inErrorData */)
      : std::runtime_error("The backend raised an exception.") { }
};

} // namespace mainmem

} // namespace dbconnector

} // namespace madlib

#endif // defined(MADLIB_mainmem_FUNCTIONHANDLE_PROTO_HPP)
