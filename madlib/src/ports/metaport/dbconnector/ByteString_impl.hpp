/* ----------------------------------------------------------------------- *//**
 *
 * @file ByteString_impl.hpp
 *
 *//* ----------------------------------------------------------------------- */

#ifndef MADLIB_mainmem_BYTESTRING_IMPL_HPP
#define MADLIB_mainmem_BYTESTRING_IMPL_HPP

namespace madlib {

namespace dbconnector {

namespace mainmem {

/*
inline
ByteString::ByteString(const bytea* inByteString)
  : mByteString(inByteString) { }
*/

inline
const ByteString::char_type*
ByteString::ptr() const {
  return mByteString;
    //return reinterpret_cast<const char_type*>(mByteString)
        //+ kEffectiveHeaderSize;
}

inline
size_t
ByteString::size() const {
  return len;
  /*
    return VARSIZE(mByteString) < kEffectiveHeaderSize
        ? 0
        : VARSIZE(mByteString) - kEffectiveHeaderSize;
        */
}

/*
inline
const bytea*
ByteString::byteString() const {
    return mByteString;
}
*/

inline
const ByteString::char_type&
ByteString::operator[](size_t inIndex) const {
    madlib_assert(inIndex < size(), std::runtime_error(
        "Out-of-bounds byte-string access detected."));

    return ptr()[inIndex];
}

/*
inline
MutableByteString::MutableByteString(bytea* inByteString)
  : ByteString(inByteString) { }
  */

inline
ByteString::char_type*
MutableByteString::ptr() const {
  return mByteString;
//    return const_cast<char_type*>(static_cast<const ByteString*>(this)->ptr());
}

#if 0
inline
bytea*
MutableByteString::byteString() {
    return const_cast<bytea*>(Base::mByteString);
}
#endif

inline
ByteString::char_type&
MutableByteString::operator[](size_t inIndex) {
  return ptr()[inIndex];
    //return const_cast<char_type&>(
        //static_cast<const ByteString*>(this)->operator[](inIndex)
    //);
}

} // namespace mainmem

} // namespace dbconnector

} // namespace madlib

#endif // defined(MADLIB_mainmem_ANYTYPE_IMPL_HPP)
