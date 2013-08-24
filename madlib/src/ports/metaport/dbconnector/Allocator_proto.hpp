/* ----------------------------------------------------------------------- *//**
 *
 * @file Allocator_proto.hpp
 *
 *//* ----------------------------------------------------------------------- */

#ifndef MADLIB_POSTGRES_ALLOCATOR_PROTO_HPP
#define MADLIB_POSTGRES_ALLOCATOR_PROTO_HPP

namespace madlib {

namespace dbconnector {

namespace mainmem {

template <typename T>
class MutableArrayHandle;

class MutableByteString;

/**
 * @brief Main Memory allocator
 *
 * In NewDelete.cpp, we therefore redefine <tt>operator new()</tt> and
 * <tt>operator delete()</tt> to call \c palloc() and \c pfree().
 *
 * @see Allocator::internalAllocate, NewDelete.cpp
 *
 * @internal
 *     To avoid name conflicts, we do not import namespace dbal
 */
class Allocator {
public:
    Allocator() : alloc() { }

#define MADLIB_ALLOCATE_ARRAY_DECL(z, n, _ignored) \
    template <typename T, dbal::MemoryContext MC, \
        dbal::ZeroMemory ZM, dbal::OnMemoryAllocationFailure F> \
    MutableArrayHandle<T> allocateArray( \
        BOOST_PP_ENUM_PARAMS_Z(z, BOOST_PP_INC(n), std::size_t inDim) \
    ) const; \
    \
    template <typename T> \
    MutableArrayHandle<T> allocateArray( \
        BOOST_PP_ENUM_PARAMS_Z(z, BOOST_PP_INC(n), std::size_t inDim) \
    ) const;
    BOOST_PP_REPEAT(MADLIB_MAX_ARRAY_DIMS, MADLIB_ALLOCATE_ARRAY_DECL,
        0 /* ignored */)
#undef MADLIB_ALLOCATE_ARRAY_DECL

    /*! Sets teh backing allocator to delegate allocations to
     */
    void SetPortAllocator(PortAllocator a) {
      alloc = a;
    }
    void SetPortAlloc(PortAllocator a) {
      SetPortAllocator(a);
    }

    template <dbal::MemoryContext MC, dbal::ZeroMemory ZM,
        dbal::OnMemoryAllocationFailure F>
    MutableByteString allocateByteString(std::size_t inSize) const;

    template <dbal::MemoryContext MC, dbal::ZeroMemory ZM,
        dbal::OnMemoryAllocationFailure F>
    void *allocate(const size_t inSize) const;

    template <dbal::MemoryContext MC, dbal::ZeroMemory ZM,
        dbal::OnMemoryAllocationFailure F>
    void *reallocate(void *inPtr, const size_t inSize) const;

    template <dbal::MemoryContext MC>
    void free(void *inPtr) const;

    template <typename T, std::size_t Dimensions, dbal::MemoryContext MC,
        dbal::ZeroMemory ZM, dbal::OnMemoryAllocationFailure F>
    MutableArrayHandle<T> internalAllocateArray(
        const std::array<std::size_t, Dimensions>& inNumElements) const;
protected:

    /*! \brief PortAllocate must implement:
     * Assignment, 
     * operator = (const PortAllocator&)
     * void* Allocate(nbytes)
     * void Free(void*)
     * Reallocate(void *inPtr, const size_t inSize)
     *
     */
    PortAllocator alloc;
};

Allocator& defaultAllocator();

} // namespace postgres

} // namespace dbconnector

} // namespace madlib

#endif // defined(MADLIB_POSTGRES_ALLOCATOR_PROTO_HPP)
