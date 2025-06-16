// TEST 할 Function 추가
extern "C" {
    int modesMessageLenByType(int type);
    int cprModFunction(int a, int b);
}

#include <gtest/gtest.h>

// TEST 추가

TEST(Dump1090Test, MessageLenByType) {
    EXPECT_EQ(modesMessageLenByType(17), 112);
    EXPECT_EQ(modesMessageLenByType(4), 56);
}

TEST(Dump1090Test, CprModFunction) {
    EXPECT_EQ(cprModFunction(5, 3), 2);
    EXPECT_EQ(cprModFunction(-5, 3), 1);
}
