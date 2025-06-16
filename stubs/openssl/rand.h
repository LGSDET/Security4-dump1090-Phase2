#ifndef STUB_OPENSSL_RAND_H
#define STUB_OPENSSL_RAND_H

#ifdef _WIN32

// Stub for RAND_bytes()
static inline int RAND_bytes(unsigned char *buf, int num) {
    for (int i = 0; i < num; ++i) buf[i] = (unsigned char)(i & 0xFF);
    return 1;  // success
}

#endif

#endif  // STUB_OPENSSL_RAND_H
