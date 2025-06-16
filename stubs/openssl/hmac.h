#ifndef STUB_OPENSSL_HMAC_H
#define STUB_OPENSSL_HMAC_H

#ifdef _WIN32

#include <stddef.h>
#include <string.h>

// ✅ Dummy struct definitions for OpenSSL API compatibility
typedef struct HMAC_CTX { int dummy; } HMAC_CTX;
typedef struct EVP_MD   { int dummy; } EVP_MD;

// ✅ Define the max digest size
#define EVP_MAX_MD_SIZE 64  // Safe upper bound for most hash outputs

// ✅ Dummy SHA-256 hash function stub
static inline const EVP_MD *EVP_sha256(void) {
    return (const EVP_MD *)0;
}

// ✅ Dummy HMAC function that fakes output
static inline unsigned char *HMAC(const void *evp_md,
                                  const void *key, int key_len,
                                  const unsigned char *d, int n,
                                  unsigned char *md, unsigned int *md_len) {
    if (md && md_len) {
        *md_len = 32;
        for (unsigned int i = 0; i < *md_len; ++i) {
            md[i] = (unsigned char)(0xA5 ^ i);  // Fake deterministic hash
        }
    }
    return md;
}

#endif  // _WIN32

#endif  // STUB_OPENSSL_HMAC_H
