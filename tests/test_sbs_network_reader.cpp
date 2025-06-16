extern "C" {
    #define CBSIZE 2048
    typedef struct cbuf
    {
        char buf[CBSIZE];
        int fd;
        unsigned int rpos, wpos;
    } cbuf_t;

    int ReadLine(char *dst, unsigned int size);
    extern int RemoteSocket;
    extern struct cbuf stringbuffer;
}

#include <gtest/gtest.h>
#include <string.h>
#include <unistd.h>

TEST(SbsNetworkReaderTest, ReadLineWithEmptyBuffer) {
    char buf[128] = {0};
    // 버퍼와 소켓을 초기화
    memset(&stringbuffer, 0, sizeof(stringbuffer));
    RemoteSocket = -1; // invalid socket

    // 버퍼가 비어있고 소켓도 없으니 -1 또는 0이 나와야 정상
    int ret = ReadLine(buf, sizeof(buf));
    EXPECT_TRUE(ret <= 0);
}