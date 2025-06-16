#ifndef STUB_OPENSSL_BIO_H
#define STUB_OPENSSL_BIO_H

#ifdef _WIN32

#include <stddef.h>  // for size_t

typedef struct BIO         { int dummy; } BIO;
typedef struct BUF_MEM     { size_t length; char *data; } BUF_MEM;

#define BIO_FLAGS_BASE64_NO_NL 0x100

static inline BIO *BIO_new(const void *type) { return (BIO *)0; }
static inline void *BIO_f_base64(void) { return 0; }
static inline void *BIO_s_mem(void) { return 0; }
static inline BIO *BIO_push(BIO *b1, BIO *b2) { return b1; }
static inline void BIO_set_flags(BIO *b, int flags) {}
static inline int BIO_write(BIO *b, const void *data, int len) { return len; }
static inline int BIO_flush(BIO *b) { return 1; }
static inline void BIO_get_mem_ptr(BIO *b, BUF_MEM **pp) {
    static BUF_MEM buf = {0};
    *pp = &buf;
}
static inline void BIO_free_all(BIO *b) {}

static inline BIO *BIO_new_mem_buf(const void *buf, int len) { return (BIO *)0; }
static inline int BIO_read(BIO *b, void *out, int len) { return len; }

#endif  // _WIN32

#endif  // STUB_OPENSSL_BIO_H
