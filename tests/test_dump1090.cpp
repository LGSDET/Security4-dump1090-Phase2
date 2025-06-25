#include <openssl/ssl.h> 
#include <errno.h>
#include <sys/socket.h>

// TEST 할 Function 추가
extern "C" {
    #include "../dump1090.h"
    //SAMPLE
    int modesMessageLenByType(int type);
    int cprModFunction(int a, int b);
    //TEST 추가
    void on_terminate(int signum);
    void SqLog_setup_signal_handlers(void);
    void modesInitNet(void);

    extern struct modesNetService {
    char *descr;
    int *socket;
    int port;
    }modesNetServices[];
    // modesNetServices는 modesInitNet에서 사용되는 서비스 구조체 배열

    // Mockable 함수 포인터
    extern int (*anetTcpServer_ptr)(char*, int, char*);
    extern int (*anetNonBlock_ptr)(char*, int);
    extern SSL_CTX* (*myInitSSL_ptr)(void);
    extern int (*anetTcpAccept_ptr)(char*, int, char*, int*);
    extern int (*myAcceptSSL_ptr)(SSL_CTX*, int, SSL**);
    extern int (*SSL_get_error_ptr)(const SSL*, int);
    extern int (*SSL_shutdown_ptr)(SSL*);
    extern void (*SSL_free_ptr)(SSL*);
    extern int (*anetSetSendBuffer_ptr)(char*, int, int);
    extern int (*close_ptr)(int);
    extern int (*getpeername_ptr)(int, struct sockaddr*, socklen_t*);
    void modesAcceptClients(void);

    void modesFreeClient(int fd);

}

// dump1090.c의 서비스 구조체와 동일하게 정의
const int SERVICE_NUM = 6;
int test_sockets[SERVICE_NUM];
const char* test_descrs[SERVICE_NUM] = {
    "Raw TCP output", "Raw In", "HTTPS", "SBS Out", "TLS Raw output", "TLS SBS output"
};


// Mock
int anetTcpServer_mock(char* err, int port, char* addr) {
    // 포트 9999는 실패, 나머지는 성공
    if (port == 9999) return -1;
    return port + 100; // 임의의 fd
}
int anetNonBlock_mock(char* err, int fd) { return 0; }
SSL_CTX* myInitSSL_mock(void) { return (SSL_CTX*)0x1234; }


int mock_accept_call = 0;
int mock_accept_fd[30] = { -1, -1, -1, -1, -1, -1 };
int mock_accept_errno[30] = { EAGAIN, EAGAIN, EAGAIN, EAGAIN, EAGAIN, EAGAIN };
int anetTcpAccept_mock(char* err, int sock, char* addr, int* port) {
    printf("mock_accept_call=%d, sock=%d, fd=%d\n", mock_accept_call, sock, mock_accept_fd[mock_accept_call]);
    if (mock_accept_call >= 30) {
        printf("mock_accept_call overflow!\n");
        exit(1);
    }
    int idx = mock_accept_call++;
    errno = mock_accept_errno[idx];
    return mock_accept_fd[idx];
}
extern ssize_t (*write_ptr)(int, const void*, size_t);
extern int (*SSL_write_ptr)(SSL*, const void*, int);

int myAcceptSSL_mock(SSL_CTX* ctx, int fd, SSL** ssl) { *ssl = (SSL*)0x1234; return 1; }
int myAcceptSSL_fail_mock(SSL_CTX* ctx, int fd, SSL** ssl) { *ssl = (SSL*)0x1234; return -1; }
int SSL_get_error_mock(const SSL* ssl, int ret) { return 1; }
int SSL_shutdown_mock(SSL* ssl) { return 1; }
void SSL_free_mock(SSL* ssl) {}
int anetSetSendBuffer_mock(char* err, int fd, int size) { return 0; }
int close_mock(int fd) { return 0; }
int getpeername_mock(int fd, struct sockaddr* addr, socklen_t* len) { return 0; }





/////////////////////////////////////

#include <gtest/gtest.h>
#include <cstring>

// Dummy


// SAMPLE
TEST(Dump1090Test, MessageLenByType) {
    EXPECT_EQ(modesMessageLenByType(17), 112);
    EXPECT_EQ(modesMessageLenByType(4), 56);
}

TEST(Dump1090Test, CprModFunction) {
    EXPECT_EQ(cprModFunction(5, 3), 2);
    EXPECT_EQ(cprModFunction(-5, 3), 1);
}

// TEST 추가

// Death test: on_terminate는 exit(0)으로 프로세스를 종료해야 한다.
TEST(Dump1090Test, OnTerminate_ExitsProcess) {
    EXPECT_EXIT({
        on_terminate(SIGTERM);
    }, ::testing::ExitedWithCode(0), "");
}

TEST(Dump1090Test, SqLogSetupSignalHandlers_NoCrash) {
    // 호출만 해서 예외/크래시가 없는지 확인
    SqLog_setup_signal_handlers();
    SUCCEED();
}

/*                            modesInitNet                            */
// ModesInitNet 테스트를 위한 Mock 설정
void SetupMocks() {
    anetTcpServer_ptr = anetTcpServer_mock;
    anetNonBlock_ptr = anetNonBlock_mock;
    myInitSSL_ptr = myInitSSL_mock;
}

void SetupMocks_modesAcceptClients() {
    anetTcpAccept_ptr = anetTcpAccept_mock;
    anetNonBlock_ptr = anetNonBlock_mock;
    myAcceptSSL_ptr = myAcceptSSL_mock;
    SSL_get_error_ptr = SSL_get_error_mock;
    SSL_shutdown_ptr = SSL_shutdown_mock;
    SSL_free_ptr = SSL_free_mock;
    mock_accept_call = 0;
    anetSetSendBuffer_ptr = anetSetSendBuffer_mock;
    close_ptr = close_mock;
    getpeername_ptr = getpeername_mock;
}
// 테스트용 소켓 변수 선언
int test_ros, test_ris, test_https, test_sbsos;

// 정상 케이스: 모든 포트 정상
TEST(ModesTest, InitNet_AllPortsOpen) {
    SetupMocks();
    for (int i = 0; i < SERVICE_NUM; ++i) {
    modesNetServices[i].socket = &test_sockets[i];
    modesNetServices[i].port = 3000 + i;
    modesNetServices[i].descr = (char*)test_descrs[i];
    }
    // 정상적으로 리턴해야 하므로 death test가 아니라 그냥 호출
    modesInitNet();
    SUCCEED();
}

// 에러 케이스: 첫 번째 포트 열기 실패
TEST(ModesTest, InitNet_PortOpenFail) {
    SetupMocks();
    for (int i = 0; i < SERVICE_NUM; ++i) {
        modesNetServices[i].socket = &test_sockets[i];
        modesNetServices[i].port = 3000 + i;
        modesNetServices[i].descr = (char*)test_descrs[i];
    }
    modesNetServices[0].port = 9999; // 실패 유도
    EXPECT_EXIT({
        modesInitNet();
    }, ::testing::ExitedWithCode(1), "Error opening the listening port");
}


/*                            modesAcceptClients                             */
// 1. 모든 소켓에서 EAGAIN (아무 일도 안 일어남)
TEST(ModesTest, AcceptClients_AllEagain) {
    memset(&Modes, 0, sizeof(Modes));
    memset(modesNetServices, 0, sizeof(*modesNetServices) * SERVICE_NUM);
    SetupMocks_modesAcceptClients();
    for (int i = 0; i < 6; ++i) {
        mock_accept_fd[i] = -1;
        mock_accept_errno[i] = EAGAIN;
    }
    modesNetServices[0].socket = &Modes.ros;
    modesNetServices[1].socket = &Modes.ris;
    modesNetServices[2].socket = &Modes.https;
    modesNetServices[3].socket = &Modes.sbsos;
    modesNetServices[4].socket = &Modes.tlros;
    modesNetServices[5].socket = &Modes.tlsbsos;
    modesAcceptClients();
    SUCCEED();
}

// 2. 첫 번째 소켓에서 fd=10 반환(정상), 나머지는 EAGAIN
TEST(ModesTest, AcceptClients_OneClient) {
    memset(&Modes, 0, sizeof(Modes));
    memset(modesNetServices, 0, sizeof(*modesNetServices) * SERVICE_NUM);
    SetupMocks_modesAcceptClients();

    Modes.ros = 10;      // RAW 소켓
    Modes.ris = 11;
    Modes.https = 12;
    Modes.sbsos = 13;
    Modes.tlros = 20;    // TLS 소켓 (다른 값)
    Modes.tlsbsos = 21;  // TLS 소켓 (다른 값)

    modesNetServices[0].socket = &Modes.ros;
    modesNetServices[1].socket = &Modes.ris;
    modesNetServices[2].socket = &Modes.https;
    modesNetServices[3].socket = &Modes.sbsos;
    modesNetServices[4].socket = &Modes.tlros;
    modesNetServices[5].socket = &Modes.tlsbsos;

    for (int i = 0; i < 30; ++i) {
        mock_accept_fd[i] = -1;
        mock_accept_errno[i] = EAGAIN;
    }
    mock_accept_fd[0] = 10; // RAW 소켓에서만 fd=10 반환
    mock_accept_errno[0] = 0;

    Modes.maxfd = -1;
    Modes.clients[10] = nullptr;
    modesAcceptClients();
    printf("After modesAcceptClients: Modes.clients[10]=%p\n", Modes.clients[10]);
    EXPECT_EQ(Modes.maxfd, 10);
    EXPECT_NE(Modes.clients[10], nullptr);
    printf("Before free: Modes.clients[10]=%p\n", Modes.clients[10]);
    if (Modes.clients[10]) {
        free(Modes.clients[10]);
        Modes.clients[10] = nullptr;
        printf("Freed Modes.clients[10]\n");
    }
    else{
        printf("Modes.clients[10] is NULL, not freeing\n");
    }
}

// 3. fd >= MODES_NET_MAX_FD (최대 fd 초과)
TEST(ModesTest, AcceptClients_MaxFdExceeded) {
    SetupMocks_modesAcceptClients();
    modesNetServices[0].socket = &Modes.ros;
    modesNetServices[1].socket = &Modes.ris;
    modesNetServices[2].socket = &Modes.https;
    modesNetServices[3].socket = &Modes.sbsos;
    modesNetServices[4].socket = &Modes.tlros;
    modesNetServices[5].socket = &Modes.tlsbsos;
    for (int i = 0; i < 6; ++i) {
        test_sockets[i] = 100 + i;
        mock_accept_fd[i] = (i == 0) ? 2000 : -1; // 2000 > 1024
        mock_accept_errno[i] = (i == 0) ? 0 : EAGAIN;
    }
    // 정상적으로 리턴해야 하므로 death test가 아니라 그냥 호출
    modesAcceptClients();
    SUCCEED();
}

// 4. TLS 소켓에서 myAcceptSSL 실패
TEST(ModesTest, AcceptClients_TLSAcceptFail) {
    SetupMocks_modesAcceptClients();
    modesNetServices[0].socket = &Modes.ros;
    modesNetServices[1].socket = &Modes.ris;
    modesNetServices[2].socket = &Modes.https;
    modesNetServices[3].socket = &Modes.sbsos;
    modesNetServices[4].socket = &Modes.tlros;  // TLS 소켓
    modesNetServices[5].socket = &Modes.tlsbsos;
    myAcceptSSL_ptr = myAcceptSSL_fail_mock;
    // TLS 소켓 인덱스는 4, 5
    for (int i = 0; i < 6; ++i) {
        test_sockets[i] = 100 + i;
        mock_accept_fd[i] = (i == 4) ? 11 : -1;
        mock_accept_errno[i] = (i == 4) ? 0 : EAGAIN;
    }
    Modes.maxfd = -1;
    Modes.clients[11] = nullptr;
    modesAcceptClients();
    // 실패 시 클라이언트가 생성되지 않음
    EXPECT_EQ(Modes.clients[11], nullptr);
}


/*                              modesFreeClient                          */

TEST(ModesTest, FreeClient_Normal) {
    memset(&Modes, 0, sizeof(Modes));
    SetupMocks_modesAcceptClients();
    // fd=5에 클라이언트 생성
    int fd = 5;
    Modes.clients[fd] = (struct client*)malloc(sizeof(struct client));
    Modes.clients[fd]->fd = fd;
    Modes.maxfd = fd;
    // 호출
    modesFreeClient(fd);
    EXPECT_EQ(Modes.clients[fd], nullptr);
    EXPECT_EQ(Modes.maxfd, -1);
}

TEST(ModesTest, FreeClient_MaxfdUpdate) {
    memset(&Modes, 0, sizeof(Modes));
    SetupMocks_modesAcceptClients();
    // fd=5, fd=3에 클라이언트 생성, maxfd=5
    Modes.clients[3] = (struct client*)malloc(sizeof(struct client));
    Modes.clients[3]->fd = 3;
    Modes.clients[5] = (struct client*)malloc(sizeof(struct client));
    Modes.clients[5]->fd = 5;
    Modes.maxfd = 5;
    // fd=5 해제 → maxfd=3으로 갱신
    modesFreeClient(5);
    EXPECT_EQ(Modes.clients[5], nullptr);
    EXPECT_EQ(Modes.maxfd, 3);
}

TEST(ModesTest, FreeClient_AlreadyNull) {
    memset(&Modes, 0, sizeof(Modes));
    SetupMocks_modesAcceptClients();
    // 이미 NULL인 fd
    int fd = 7;
    Modes.clients[fd] = nullptr;
    Modes.maxfd = fd;
    // 호출 (free(NULL) 안전)
    modesFreeClient(fd);
    EXPECT_EQ(Modes.clients[fd], nullptr);
    EXPECT_EQ(Modes.maxfd, -1);
}

TEST(ModesTest, FreeClient_TLS) {
    memset(&Modes, 0, sizeof(Modes));
    SetupMocks_modesAcceptClients();
    int fd = 10;
    Modes.clients[fd] = (struct client*)malloc(sizeof(struct client));
    Modes.clients[fd]->fd = fd;
    Modes.maxfd = fd;
    Modes.ssl[fd] = (SSL*)0x1234; // TLS 연결 존재
    // 호출
    modesFreeClient(fd);
    EXPECT_EQ(Modes.clients[fd], nullptr);
    EXPECT_EQ(Modes.ssl[fd], nullptr);
    EXPECT_EQ(Modes.maxfd, -1);
}

TEST(ModesTest, FreeClient_SqlogError) {
    memset(&Modes, 0, sizeof(Modes));
    SetupMocks_modesAcceptClients();
    int fd = 8;
    Modes.clients[fd] = (struct client*)malloc(sizeof(struct client));
    Modes.clients[fd]->fd = fd;
    Modes.maxfd = fd;
    // getpeername, getsockname 등 mock에서 실패하도록 설정 가능
    // (실제 로그만 찍히고 동작은 동일)
    modesFreeClient(fd);
    EXPECT_EQ(Modes.clients[fd], nullptr);
    EXPECT_EQ(Modes.maxfd, -1);
}

TEST(ModesTest, FreeClient_NonMaxfd) {
    memset(&Modes, 0, sizeof(Modes));
    SetupMocks_modesAcceptClients();
    // fd=2, fd=4, fd=6에 클라이언트 생성, maxfd=6
    Modes.clients[2] = (struct client*)malloc(sizeof(struct client));
    Modes.clients[4] = (struct client*)malloc(sizeof(struct client));
    Modes.clients[6] = (struct client*)malloc(sizeof(struct client));
    Modes.maxfd = 6;
    // fd=4만 해제 → maxfd는 6 유지
    modesFreeClient(4);
    EXPECT_EQ(Modes.clients[4], nullptr);
    EXPECT_EQ(Modes.maxfd, 6);
}

/*                              modesSendAllClients                          */

TEST(ModesTest, SendAllClients_NoClients) {
    memset(&Modes, 0, sizeof(Modes));
    SetupMocks_modesAcceptClients();
    Modes.maxfd = 5;
    // 모든 clients가 NULL
    for (int i = 0; i <= Modes.maxfd; ++i) Modes.clients[i] = nullptr;
    modesSendAllClients(Modes.ros, (void*)"MSG", 3);
    SUCCEED();
}

TEST(ModesTest, SendAllClients_NonTLS_WriteSuccess) {
    memset(&Modes, 0, sizeof(Modes));
    SetupMocks_modesAcceptClients();
    Modes.maxfd = 2;
    int called[3] = {0,0,0};
    // mock write 함수
    auto old_write = write_ptr;
    write_ptr = [](int, const void*, size_t len) -> ssize_t { return len; };
    for (int i = 0; i <= Modes.maxfd; ++i) {
        Modes.clients[i] = (struct client*)malloc(sizeof(struct client));
        Modes.clients[i]->service = Modes.ros;
    }
    modesSendAllClients(Modes.ros, (void*)"MSG", 3);
    for (int i = 0; i <= Modes.maxfd; ++i){
        free(Modes.clients[i]);
        Modes.clients[i] = nullptr;
    } 
    write_ptr = old_write;
    SUCCEED();
}

TEST(ModesTest, SendAllClients_NonTLS_WriteFail) {
    memset(&Modes, 0, sizeof(Modes));
    SetupMocks_modesAcceptClients();
    Modes.maxfd = 1;
    // write 실패 (nwritten != len)
    auto old_write = write_ptr;
    write_ptr = [](int fd, const void* buf, size_t len) -> ssize_t {
        return 1; // 실패
    };
    Modes.clients[0] = (struct client*)malloc(sizeof(struct client));
    Modes.clients[0]->service = Modes.ros;
    Modes.clients[1] = (struct client*)malloc(sizeof(struct client));
    Modes.clients[1]->service = Modes.ros;
    modesSendAllClients(Modes.ros, (void*)"MSG", 3);
    free(Modes.clients[0]);
    Modes.clients[0] = nullptr;
    free(Modes.clients[1]);
    Modes.clients[1] = nullptr;
    write_ptr = old_write;
    SUCCEED();
}

TEST(ModesTest, SendAllClients_TLS_SSLWriteSuccess) {
    memset(&Modes, 0, sizeof(Modes));
    SetupMocks_modesAcceptClients();
    Modes.maxfd = 0;
    Modes.clients[0] = (struct client*)malloc(sizeof(struct client));
    Modes.clients[0]->service = Modes.tlros;
    Modes.ssl[0] = (SSL*)0x1234;
    // SSL_write 성공
    auto old_SSL_write = SSL_write_ptr;
    SSL_write_ptr = [](SSL* ssl, const void* buf, int len) -> int { return len; };
    modesSendAllClients(Modes.tlros, (void*)"MSG", 3);
    free(Modes.clients[0]);
    Modes.clients[0] = nullptr;
    SSL_write_ptr = old_SSL_write;
    SUCCEED();
}

TEST(ModesTest, SendAllClients_TLS_SSLWriteFail) {
    memset(&Modes, 0, sizeof(Modes));
    SetupMocks_modesAcceptClients();
    Modes.maxfd = 0;
    Modes.clients[0] = (struct client*)malloc(sizeof(struct client));
    Modes.clients[0]->service = Modes.tlros;
    Modes.ssl[0] = (SSL*)0x1234;
    // SSL_write 실패 (nwritten <= 0)
    auto old_SSL_write = SSL_write_ptr;
    SSL_write_ptr = [](SSL* ssl, const void* buf, int len) -> int { return -1; };
    modesSendAllClients(Modes.tlros, (void*)"MSG", 3);
    free(Modes.clients[0]);
    Modes.clients[0] = nullptr;
    SSL_write_ptr = old_SSL_write;
    SUCCEED();
}

TEST(ModesTest, SendAllClients_TLS_SSLWritePartial) {
    memset(&Modes, 0, sizeof(Modes));
    SetupMocks_modesAcceptClients();
    Modes.maxfd = 0;
    Modes.clients[0] = (struct client*)malloc(sizeof(struct client));
    Modes.clients[0]->service = Modes.tlros;
    Modes.ssl[0] = (SSL*)0x1234;
    // SSL_write partial (nwritten != len)
    auto old_SSL_write = SSL_write_ptr;
    SSL_write_ptr = [](SSL* ssl, const void* buf, int len) -> int { return 1; };
    modesSendAllClients(Modes.tlros, (void*)"MSG", 3);
    free(Modes.clients[0]);
    Modes.clients[0] = nullptr;
    SSL_write_ptr = old_SSL_write;
    SUCCEED();
}
