#ifndef STUB_OPENSSL_BUFFER_H
#define STUB_OPENSSL_BUFFER_H

#ifdef _WIN32

#include <stddef.h> 

typedef struct BUF_MEM {
    size_t length;
    char *data;
} BUF_MEM;

static inline void BUF_MEM_free(BUF_MEM *b) {}
static inline BUF_MEM *BUF_MEM_new(void) { return NULL; }

#endif

#endif
