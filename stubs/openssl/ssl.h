#ifndef STUB_OPENSSL_SSL_H
#define STUB_OPENSSL_SSL_H

#ifdef _WIN32

#define SERVER_CERT_FILE  "stub_server.crt"
#define SERVER_KEY_FILE   "stub_server.key"
#define CLIENT_CERT_FILE  "stub_client.crt"


typedef struct SSL       { int dummy; } SSL;
typedef struct SSL_CTX   { int dummy; } SSL_CTX;
typedef struct SSL_METHOD { int dummy; } SSL_METHOD;

static inline int SSL_library_init(void) { return 1; }
static inline void OpenSSL_add_all_algorithms(void) {}
static inline void SSL_load_error_strings(void) {}
static inline const SSL_METHOD *TLS_server_method(void) { return (SSL_METHOD *)0; }

static inline SSL_CTX *SSL_CTX_new(const SSL_METHOD *method) { return (SSL_CTX *)0; }
static inline void SSL_CTX_free(SSL_CTX *ctx) {}
static inline int SSL_CTX_use_certificate_file(SSL_CTX *ctx, const char *file, int type) { return 1; }
static inline int SSL_CTX_use_PrivateKey_file(SSL_CTX *ctx, const char *file, int type) { return 1; }
static inline int SSL_CTX_load_verify_locations(SSL_CTX *ctx, const char *file, const char *path) { return 1; }
static inline void SSL_CTX_set_verify(SSL_CTX *ctx, int mode, void *cb) {}
static inline int SSL_CTX_set_min_proto_version(SSL_CTX *ctx, int version) { return 1; }

static inline SSL *SSL_new(SSL_CTX *ctx) { return (SSL *)0; }
static inline int SSL_set_fd(SSL *ssl, int fd) { return 1; }
static inline int SSL_accept(SSL *ssl) { return 1; }
static inline void SSL_free(SSL *ssl) {}

#define SSL_FILETYPE_PEM 1
#define SSL_VERIFY_PEER 0x01
#define SSL_VERIFY_FAIL_IF_NO_PEER_CERT 0x02
#define TLS1_2_VERSION 0x0303

#endif  // _WIN32

#endif  // STUB_OPENSSL_SSL_H
