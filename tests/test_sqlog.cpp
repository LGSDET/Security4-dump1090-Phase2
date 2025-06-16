extern "C" {
    int SqLog_InitReading(const char *key_file);
}

#include <gtest/gtest.h>

TEST(SqLogTest, InitReadingWithInvalidFile) {
    // 존재하지 않는 파일을 넣으면 -1을 반환해야 함
    EXPECT_EQ(SqLog_InitReading("/tmp/does_not_exist.hex"), -1);
}