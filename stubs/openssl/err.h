#ifndef STUB_OPENSSL_ERR_H
#define STUB_OPENSSL_ERR_H

#ifdef _WIN32

#include <stdio.h>

static inline void ERR_print_errors_fp(FILE *fp) {
    // no-op stub
}

#endif  // _WIN32

#endif  // STUB_OPENSSL_ERR_H
