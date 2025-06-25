extern "C" {
    #include "../TLSsample/tls.h"
    
    SSL_CTX *myInitSSL(void);
    int myFreeSSL(SSL_CTX *ctx, SSL *ssl);
    int myAcceptSSL(SSL_CTX *ctx, int client_sock, SSL **ppSsl);
}

#include <gtest/gtest.h>
#include <openssl/ssl.h>
#include <fcntl.h>
#include <unistd.h>

// 1. myInitSSL 정상 동작 (실제 인증서/키 파일 필요)
TEST(TServerTest, MyInitSSL_Success) {
    SSL_CTX *ctx = myInitSSL();
    ASSERT_NE(ctx, nullptr);
    SSL_CTX_free(ctx);
}

// 2. myInitSSL 실패 케이스 (잘못된 인증서/키 파일 환경변수로 유도)
TEST(TServerTest, MyInitSSL_Fail_CertOrKey) {
    // 환경변수로 잘못된 파일 지정 (tls.h에서 #define SERVER_CERT_FILE 등 경로를 환경변수로 바꿔야 함)
    setenv("SERVER_CERT_FILE", "/tmp/not_exist_cert.pem", 1);
    setenv("SERVER_KEY_FILE", "/tmp/not_exist_key.pem", 1);
    // myInitSSL은 실패 시 exit() 호출 → death test 필요
    ASSERT_EXIT(myInitSSL(), ::testing::ExitedWithCode(1), ".*");
}

// 3. myInitSSL 실패 케이스 (잘못된 클라이언트 CA)
TEST(TServerTest, MyInitSSL_Fail_ClientCA) {
    setenv("CLIENT_CERT_FILE", "/tmp/not_exist_ca.pem", 1);
    // myInitSSL은 실패 시 exit() 호출 → death test 필요
    ASSERT_EXIT(myInitSSL(), ::testing::ExitedWithCode(1), ".*");
}

// 2. 정상적인 SSL_CTX와 잘못된 소켓을 넘겼을 때
TEST(TServerTest, MyAcceptSSL_InvalidSocket) {
    SSL_library_init();
    SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());
    ASSERT_NE(ctx, nullptr);

    int fake_sock = open("/dev/null", O_RDWR); // 유효하지 않은 소켓
    SSL *ssl = nullptr;
    int ret = myAcceptSSL(ctx, fake_sock, &ssl);
    EXPECT_LE(ret, 0); // 실패해야 정상

    if (ssl) SSL_free(ssl);
    close(fake_sock);
    SSL_CTX_free(ctx);
}

TEST(TServerTest, MyFreeSSL_NullArgs) {
    // NULL 인자에 대해 segfault 없이 동작하는지 확인
    EXPECT_NO_THROW({
        myFreeSSL(nullptr, nullptr);
    });
}