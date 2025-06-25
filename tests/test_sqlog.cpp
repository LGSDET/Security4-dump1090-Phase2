#include <gtest/gtest.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/buffer.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <fstream>
#include <vector>
//#include "sqlog.h"


#define LOG_FILE_BASE_NAME "test_sqlog.log"
#define LOG_FILE_PATH "./"
#define LOG_FILE_SIZE (1024 * 1024) // 1MB
#define LOG_FILE_HASH_EXT ".cs"
#define LOG_KEY_FILE_NAME "lgess2025s4testlogkey.hex"
#define LOG_KEY_FILE_PATH "./"
#define LOG_AES_KEY_LEN 32
#define LOG_AES_GCM_IV_LEN 12
#define LOG_AES_GCM_TAG_LEN 16
#define SQLOG_BUFFER_SIZE 1024
#define SQLOG_LINE_MAX 256

typedef enum {
    LOG_LEVEL_F = 0,
    LOG_LEVEL_E,
    LOG_LEVEL_W,
    LOG_LEVEL_I,
    LOG_LEVEL_D
} LOG_LEVEL;
extern "C" {
    extern unsigned char g_key[LOG_AES_KEY_LEN];

    int SqLog(int log_level, const char *format, ...);
    int SqLog_InitWriting(const char *key_file, const char *logf_path, const char *logf_name);
    int SqLog_InitReading(const char *key_file);
    int WriteLog(const char *message);
    int SqLog_ReadLog(const char *pacCiperText, size_t szCLen, char *pacPlainText, size_t szPLen);
    int SqLog_VerifyIntegrity(const char *log_file_path);
    void SqLog_LogStart(int argc, char *argv[]);
    void SqLog_CloseFiles(void);
    int base64_encode(const unsigned char *in, size_t in_len, char *out, const unsigned int out_len);
    int base64_decode(const char *in, unsigned char *out, unsigned int *out_len);
    int decrypt_gcm(const unsigned char *key, const unsigned char *data, size_t data_len, char *plaintext_out, size_t plaintext_max);
    int rotate_log_file_if_needed(size_t new_entry_size);
    long get_file_size(const char *filename);
    int get_max_log_index(void);
}

#define SqLog_I(...) SqLog(LOG_LEVEL_I, __VA_ARGS__)

void GenerateTestKeyFile(const std::string& path) {
    std::ofstream keyfile(path);
    keyfile << "00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff";
    keyfile.close();
}

class SqLogFullTest : public ::testing::Test {
protected:
    void SetUp() override {
        GenerateTestKeyFile("./lgess2025s4testlogkey.hex");
        SqLog_InitWriting("./lgess2025s4testlogkey.hex", LOG_FILE_PATH, LOG_FILE_BASE_NAME);
    }

    void TearDown() override {
        SqLog_CloseFiles();
        remove((std::string(LOG_FILE_PATH) + LOG_FILE_BASE_NAME).c_str());
        remove((std::string(LOG_FILE_PATH) + LOG_FILE_BASE_NAME + LOG_FILE_HASH_EXT).c_str());
        remove("./lgess2025s4testlogkey.hex");
    }
};

TEST_F(SqLogFullTest, WriteAndVerifyLog) {
    EXPECT_EQ(WriteLog("Test entry 1"), 0);
    EXPECT_EQ(WriteLog("Test entry 2"), 0);
    SqLog_CloseFiles();

    EXPECT_EQ(SqLog_VerifyIntegrity((std::string(LOG_FILE_PATH) + LOG_FILE_BASE_NAME).c_str()), 0);
}

TEST_F(SqLogFullTest, SqLogLogStartAndLevels) {
    char *argv[] = {(char *)"./prog", (char *)"--opt1", (char *)"--opt2"};
    SqLog_LogStart(3, argv);
    SqLog(LOG_LEVEL_F, "fatal message");
    SqLog(LOG_LEVEL_E, "error message");
    SqLog(LOG_LEVEL_W, "warn message");
    SqLog(LOG_LEVEL_I, "info message");
    SqLog(LOG_LEVEL_D, "debug message");
    SqLog(42, "unknown level message");
    SUCCEED();
}
TEST(SqLogNegativeTest, SqLogLogStartReadlinkFails) {
    // 강제로 /proc/self/exe 에 접근 불가 여부를 사전 점검
    if (access("/proc/self/exe", F_OK) != 0) {
        char *argv[] = {(char *)"program"};
        SqLog_LogStart(1, argv);  // 내부적으로 readlink 실패 예상
        SUCCEED();  // 실패하지 않고 처리되면 성공
    } else {
        GTEST_SKIP() << "/proc/self/exe is accessible; cannot trigger readlink failure in this environment.";
    }
}
TEST_F(SqLogFullTest, LongLogTriggersChunking) {
    std::string long_msg(SQLOG_BUFFER_SIZE * 2, 'L');
    EXPECT_EQ(SqLog(LOG_LEVEL_I, "%s", long_msg.c_str()), 0);
}

TEST(SqLogNegativeTest, InitReadingInvalidKeyFile) {
    EXPECT_EQ(SqLog_InitReading("not_found.key"), -1);
}

TEST(SqLogNegativeTest, WriteLogWithoutInit) {
    SqLog_CloseFiles();
    EXPECT_EQ(WriteLog("Should fail"), -1);
}

TEST(SqLogNegativeTest, Base64EncodeTooSmallBuffer) {
    const char *in = "test";
    char out[2];
    EXPECT_EQ(base64_encode((const unsigned char *)in, strlen(in), out, sizeof(out)), -1);
}

TEST(SqLogNegativeTest, DecryptGCMTooShort) {
    char plain[64];
    unsigned char invalid_data[8] = {0};
    EXPECT_EQ(decrypt_gcm(g_key, invalid_data, sizeof(invalid_data), plain, sizeof(plain)), -1);
}

TEST(SqLogNegativeTest, Base64DecodeInvalid) {
    const char *bad = "!!!!!badbase64###";
    unsigned char out[64];
    unsigned int out_len = sizeof(out);
    EXPECT_EQ(base64_decode(bad, out, &out_len), -1);
}
TEST(SqLogNegativeTest, Base64DecodeInvalid_adc) {
    const char *bad = "\n";
    unsigned char out[64];
    unsigned int out_len = sizeof(out);
    EXPECT_EQ(base64_decode(bad, out, &out_len), -1);
}
#if 0
TEST(SqLogNegativeTest, RotateLogFileTrigger) {
    GenerateTestKeyFile("./lgess2025s4testlogkey.hex");
    ASSERT_EQ(SqLog_InitWriting("./lgess2025s4testlogkey.hex", LOG_FILE_PATH, LOG_FILE_BASE_NAME), 0);
    std::string big_msg(LOG_FILE_SIZE, 'A');
    EXPECT_EQ(WriteLog(big_msg.c_str()), 0);
    SqLog_CloseFiles();

    std::ifstream f(LOG_FILE_PATH + std::string(LOG_FILE_BASE_NAME) + ".001");
    EXPECT_TRUE(f.good());
    f.close();
    remove("./lgess2025s4testlogkey.hex");
}
#endif
TEST(SqLogNegativeTest, GetFileSizeNonexistent) {
    EXPECT_EQ(get_file_size("/no/such/file"), 0);
}

TEST(SqLogNegativeTest, GetMaxLogIndexFormat) {
    system("touch ./" LOG_FILE_BASE_NAME ".bad");
    EXPECT_GE(get_max_log_index(), 0);
    remove("./" LOG_FILE_BASE_NAME ".bad");
}

TEST(SqLogNegativeTest, SqLogReadLogCorruptInput) {
    char plain[1024];
    const char *bad_data = "!!!invalidbase64###";
    EXPECT_NE(SqLog_ReadLog(bad_data, strlen(bad_data), plain, sizeof(plain)), 0);
}
#if 0
TEST(SqLogNegativeTest, VerifyIntegrityMissingHMAC) {
    GenerateTestKeyFile("./lgess2025s4testlogkey.hex");
    SqLog_InitWriting("./lgess2025s4testlogkey.hex", "./", "missing_cs.log");
    WriteLog("entry");
    SqLog_CloseFiles();
    remove("./missing_cs.log.cs");
    EXPECT_LT(SqLog_VerifyIntegrity("./missing_cs.log"), 0);
    remove("./missing_cs.log");
    remove("./lgess2025s4testlogkey.hex");
}
#endif

TEST(SqLogNegativeTest, SqLogReadLog_Base64DecodeFailsWithInvalidText) {
    char plain[1024];

    // 유효한 입력 (길이 포함), 하지만 base64로 디코딩할 수 없는 문자열
    const char *invalid_base64 = "!!!invalidbase64###";
    size_t input_len = strlen(invalid_base64);

    // 전제 조건: 입력이 null 아님, 길이도 0 아님 → base64_decode 호출됨
    ASSERT_GT(input_len, 0);
    ASSERT_NE(invalid_base64, nullptr);
    ASSERT_NE(plain, nullptr);

    // 이 호출에서 base64_decode() 내부에서 오류 발생하고 -1 리턴 예상
    EXPECT_NE(SqLog_ReadLog(invalid_base64, input_len, plain, sizeof(plain)), 0);
}
TEST(SqLogNegativeTest, SqLogVerifyIntegrity_FopenFails) {
    const char *nonexistent_log = "./this_file_should_not_exist.log";

    // 실제 존재하지 않도록 보장
    remove(nonexistent_log);

    // fopen("rb") 실패 → log_fp == NULL → 함수는 -1 반환해야 함
    EXPECT_EQ(SqLog_VerifyIntegrity(nonexistent_log), -1);
}
#if 1
TEST(SqLogNegativeTest, SqLogVerifyIntegrity_FseekFails) {
    const char *dir_as_log = "./sqlog_test_dir_fake.log";

    // 디렉토리를 로그 파일인 척 생성
    mkdir(dir_as_log, 0755);

    // SqLog_VerifyIntegrity()는 fopen() 성공 후 fseek() 실패하게 됨
    int result = SqLog_VerifyIntegrity(dir_as_log);

    EXPECT_EQ(result, -4);

    // 정리
    rmdir(dir_as_log);
}
#else
TEST(SqLogNegativeTest, SqLogVerifyIntegrity_FseekFails) {
    const char *file_path = "./fseek_fails_testfile.log";

    // 1. FIFO (named pipe)를 생성
    mkfifo(file_path, 0644);

    // 2. 이 FIFO를 log 파일인 척 열고, fseek 시도 → 실패 예상
    int result = SqLog_VerifyIntegrity(file_path);
    EXPECT_EQ(result, -2);  // fseek() 실패 예상값

    // 3. 정리
    unlink(file_path);
}
#endif
