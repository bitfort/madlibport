

#ifndef HAZY_BISMARCK_DEGREE_INL_H
#define HAZY_BISMARCK_DEGREE_INL_H

namespace hazy {
namespace bismarck {

template <class CTX>
void DegInit(CTX* ctx, bytea *m) {
  m->str = NULL;
  m->len = 0;
}

template <class CTX>
void DegRealloc(CTX *ctx, bytea *m, size_t len) {
  size_t dlen;
  uint64_t *d;
  CoerceBytea(m, d, dlen);

  uint64_t *neu = BismarckAllocate<uint64_t>(ctx, len);
  // zero it
  memset(neu, 0, len * sizeof(uint64_t));
  if (dlen > 0) {
    // copy old contents
    memcpy(neu, d, dlen*sizeof(uint64_t));
    BismarckFree(ctx, d);
    m->str = (char*) neu;
    m->len = sizeof(uint64_t) * len;
  }
}

template <class CTX>
void DegUpdate(CTX* ctx, bytea *m, size_t n) {
  size_t dlen;
  uint64_t *d;
  CoerceBytea(*m, d, dlen);

  if (n >= dlen) {
    // re-allocate with more memory
    DegRealloc(ctx, m, n+1);
    // the pointer d is no longer valid (it was freed), get it again
    CoerceBytea(m, d, dlen);
  }

  // count this one
  d[n]++;
}

template <class CTX>
bytea DegFinal(CTX* ctx, const bytea &m) {
  // TODO what else to do?
  return m;
}

template <class CTX>
void DegMerge(CTX* ctx, const bytea &m, bytea *dst) {
  // See if dest is big enough, if not, resize
  size_t dlen, slen;
  uint64_t *d, *s;
  CoerceBytea(*dst, d, dlen);
  CoerceBytea(m, s, slen);

  if (slen > dlen) {
    DegRealloc(ctx, *dst, slen);
    // the pointer d is no longer valid (it was freed), get it again
    CoerceBytea(*dst, d, dlen);
  }

  for (size_t i = 0; i < slen; i++) {
    d[i] += s[i];
  }
}

template <class CTX>
bytea DegSerial(CTX* ctx, const bytea &m) {
  return m;
}

}
}
#endif
