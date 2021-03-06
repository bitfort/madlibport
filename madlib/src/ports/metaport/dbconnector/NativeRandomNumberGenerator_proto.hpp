/* ----------------------------------------------------------------------- *//**
 *
 * @file NativeRandomNumberGenerator_proto.hpp
 *
 *//* ----------------------------------------------------------------------- */

#ifndef MADLIB_mainmem_NATIVERANDOMNUMBERGENERATOR_PROTO_HPP
#define MADLIB_mainmem_NATIVERANDOMNUMBERGENERATOR_PROTO_HPP

namespace madlib {

namespace dbconnector {

namespace mainmem {

/**
 * @brief Front-end to the RDBMS random number generator
 *
 * This pseudo-RNG is special in that it has no own state. Instead, its state is
 * externally defined. It is therefore not necessary to keep a global instance
 * of this RNG.
 */
class NativeRandomNumberGenerator {
public:
    typedef double result_type;

    NativeRandomNumberGenerator();
    void seed(result_type inSeed);
    result_type operator()();
    static result_type min();
    static result_type max();
};

} // namespace mainmem

} // namespace dbconnector

} // namespace madlib

#endif // defined(MADLIB_mainmem_NATIVERANDOMNUMBERGENERATOR_PROTO_HPP)
