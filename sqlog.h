#ifndef __SQLOG_H__
#define __SQLOG_H__

#define LOG_FILE_BASE_NAME "dump1090.log"
#define LOG_FILE_PATH "/etc/ssl/dump1090/"
#define LOG_FILE_SIZE (1024 * 1024) // 1MB

#define LOG_FILE_HASH_EXT ".cs"

#define LOG_KEY_FILE_NAME "lgess2025s4rpilogkey.hex"
#define LOG_KEY_FILE_PATH "/etc/ssl/dump1090/"

#define LOG_AES_KEY_LEN 32     // 256-bit key
#define LOG_AES_GCM_IV_LEN 12  // Recommended for GCM
#define LOG_AES_GCM_TAG_LEN 16 // 128-bit tag

// Max formatted log message size
#define SQLOG_BUFFER_SIZE 1024
#define SQLOG_LINE_MAX 256

// Log level enum
typedef enum
{
    LOG_LEVEL_F = 0, // FATAL
    LOG_LEVEL_E,     // ERROR
    LOG_LEVEL_W,     // WARN
    LOG_LEVEL_I,     // INFO
    LOG_LEVEL_D      // DEBUG
} LOG_LEVEL;

// Declaration of SqLog function
int SqLog(int log_level, const char *format, ...);

// Convenience macros for each log level (support variadic args)
#define SqLog_F(...) SqLog(LOG_LEVEL_F, __VA_ARGS__)
#define SqLog_E(...) SqLog(LOG_LEVEL_E, __VA_ARGS__)
#define SqLog_W(...) SqLog(LOG_LEVEL_W, __VA_ARGS__)
#define SqLog_I(...) SqLog(LOG_LEVEL_I, __VA_ARGS__)
#define SqLog_D(...) SqLog(LOG_LEVEL_D, __VA_ARGS__)

int SqLog_InitWriting(const char *key_file, const char *logf_path, const char *logf_name);
void SqLog_LogStart(int argc, char *argv[]);
int Sqlog_WriteLog(const char *message);
void SqLog_CloseFiles(void);
int SqLog_VerifyIntegrity(const char *log_file_path);
int WriteLog(const char *message);

int SqLog_InitReading(const char *key_file);
int SqLog_ReadLog(const char *, size_t, char *, size_t);
#endif // __SQLOG_H__
