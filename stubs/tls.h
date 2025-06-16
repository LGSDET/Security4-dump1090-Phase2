#ifndef TLS_H
#define TLS_H

typedef int SSL_CTX;
typedef int SSL;

//static inline SSL_CTX * myInitSSL(void) { return 0; }
static inline int myFreeSSL(SSL_CTX *ctx, SSL *ssl) { return 0; }
static inline int myAcceptSSL(SSL_CTX *ctx, int client_sock, SSL **ppSsl) { return 0; }

#define SSL_get_error(...) (0)
#define ERR_print_errors_fp(...) ((void)0)
#define SSL_shutdown(...) (0)
#define SSL_free(...) ((void)0)
#define SSL_write(...) (0)

#endif
