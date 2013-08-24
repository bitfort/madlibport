/* ----------------------------------------------------------------------- *//**
 *
 * @file ByteString_proto.hpp
 *
 *//* ----------------------------------------------------------------------- */

#ifndef MADLIB_mainmem_BYTESTRING_PROTO_HPP
#define MADLIB_mainmem_BYTESTRING_PROTO_HPP

namespace madlib {

namespace dbconnector {

namespace mainmem {

template <class T>
class Reference;

class ByteString {
public:
    enum { isMutable = dbal::Immutable };
    enum { kEffectiveHeaderSize = 0
        //= ((VARHDRSZ - 1) & ~(MAXIMUM_ALIGNOF - 1)) + MAXIMUM_ALIGNOF 
        };
    
    typedef char char_type;

    ByteString() : mByteString(NULL), len(0) { };
    ByteString(char* inByteString, size_t len_) : mByteString(inByteString),
        len(len_) { };

    const char_type* ptr() const;
    size_t size() const;
    //const bytea* byteString() const;
    const char_type& operator[](size_t inIndex) const;

protected:
    char* mByteString;
    size_t len;
};


class MutableByteString : public ByteString {
    typedef ByteString Base;

public:
    enum { isMutable = dbal::Mutable };

//    MutableByteString(bytea* inByteString);
    MutableByteString() : Base() { }

    MutableByteString(char* inByteString, size_t len_) :
        Base(inByteString, len_) { }

    char_type* ptr() const;
//    bytea* byteString();
    char_type& operator[](size_t inIndex);
};

} // namespace mainmem

} // namespace dbconnector

} // namespace madlib

#endif // defined(MADLIB_mainmem_BYTESTRING_PROTO_HPP)
