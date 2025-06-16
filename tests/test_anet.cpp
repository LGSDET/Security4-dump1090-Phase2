extern "C" {
    int anetNonBlock(char *err, int fd);
}

#include <gtest/gtest.h>

TEST(AnetTest, InvalidFd) {
    char err[256] = {0};
    // 음수 파일 디스크립터는 실패(-1)해야 정상
    EXPECT_EQ(anetNonBlock(err, -1), -1);
    // 에러 메시지가 제대로 들어갔는지도 확인
    ASSERT_NE(err[0], '\0');
}