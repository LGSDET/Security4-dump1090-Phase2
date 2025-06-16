extern "C" {
    #include "../TLSsample/tls.h"
    
    SSL_CTX *myInitSSL(void);
    int myFreeSSL(SSL_CTX *ctx, SSL *ssl);
}

#include <gtest/gtest.h>

TEST(TServerTest, InitSSL) {
    SSL_CTX *ctx = myInitSSL();
    ASSERT_NE(ctx, nullptr); // SSL_CTX가 정상적으로 생성되는지 확인
    // 리소스 해제는 생략(테스트 환경에서만 사용)
}