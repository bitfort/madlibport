
#ifndef HAZY_BISMARCK_COMMON_H
#define HAZY_BISMARCK_COMMON_H

namespace hazy {
namespace bismarck {


struct bytea {
  char* str;
  size_t len;
};

template <class T>
void CoerceBytea(bytea a, T *&outp, size_t &out_len) {
  outp = reinterpret_cast<T*>(a.str);
  out_len = a.len / sizeof(T);
}

} // bismarck
} // hazy
#endif
