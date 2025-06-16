#ifndef STUB_OPENSSL_EVP_H
#define STUB_OPENSSL_EVP_H

#ifdef _WIN32

#include <stddef.h>
#include <string.h>

typedef struct EVP_CIPHER_CTX { int dummy; } EVP_CIPHER_CTX;
typedef struct EVP_CIPHER     { int dummy; } EVP_CIPHER;
typedef struct EVP_MD         { int dummy; } EVP_MD;

#define EVP_CTRL_GCM_SET_IVLEN 0
#define EVP_CTRL_GCM_GET_TAG   1
#define EVP_CTRL_GCM_SET_TAG   2  // ✅ 복호화 인증 태그 설정용
#define EVP_MAX_MD_SIZE        64

static inline EVP_CIPHER_CTX *EVP_CIPHER_CTX_new(void) { return (EVP_CIPHER_CTX *)0; }
static inline void EVP_CIPHER_CTX_free(EVP_CIPHER_CTX *ctx) {}

static inline const EVP_CIPHER *EVP_aes_256_gcm(void) { return (EVP_CIPHER *)0; }

static inline int EVP_EncryptInit_ex(EVP_CIPHER_CTX *ctx, const EVP_CIPHER *cipher,
                                     void *impl, const unsigned char *key, const unsigned char *iv) {
    return 1;
}
static inline int EVP_EncryptUpdate(EVP_CIPHER_CTX *ctx, unsigned char *out, int *outlen,
                                    const unsigned char *in, int inlen) {
    if (out && outlen) {
        *outlen = inlen;
        memcpy(out, in, inlen);  // fake copy
    }
    return 1;
}
static inline int EVP_EncryptFinal_ex(EVP_CIPHER_CTX *ctx, unsigned char *out, int *outlen) {
    if (out && outlen) *outlen = 0;
    return 1;
}
static inline int EVP_CIPHER_CTX_ctrl(EVP_CIPHER_CTX *ctx, int type, int arg, void *ptr) {
    return 1;
}

// ✅ 추가된 복호화 함수들
static inline int EVP_DecryptInit_ex(EVP_CIPHER_CTX *ctx, const EVP_CIPHER *cipher,
                                     void *impl, const unsigned char *key, const unsigned char *iv) {
    return 1;
}
static inline int EVP_DecryptUpdate(EVP_CIPHER_CTX *ctx, unsigned char *out, int *outlen,
                                    const unsigned char *in, int inlen) {
    if (out && outlen) {
        *outlen = inlen;
        memcpy(out, in, inlen);  // fake copy
    }
    return 1;
}
static inline int EVP_DecryptFinal_ex(EVP_CIPHER_CTX *ctx, unsigned char *out, int *outlen) {
    if (out && outlen) *outlen = 0;
    return 1;  // return <= 0 to simulate failure if needed
}

#endif  // _WIN32

#endif  // STUB_OPENSSL_EVP_H
