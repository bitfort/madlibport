
#ifndef HAZY_BISMARCK_COMMON_H
#define HAZY_BISMARCK_COMMON_H

namespace hazy {
namespace bismarck {


struct bytea {
  char* str;
  size_t len;
};

void CoerceBytea(bytea a, double *&outp, size_t &out_len) {
  outp = reinterpret_cast<double*>(a.str);
  out_len = a.len / sizeof(double);
}

} // bismarck
} // hazy
#endif
