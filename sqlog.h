#ifndef __SQLOG_H__
#define __SQLOG_H__

#define LOG_FILE_BASE_NAME "dump1090.log"
#define LOG_FILE_PATH "/etc/ssl/dump1090/"
#define LOG_FILE_SIZE (1024 * 1024) // 1MB

#define LOG_FILE_HASH_EXT ".hmac"

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

#ifdef __cplusplus
extern "C" {
#endif

// Global variables for testing
extern unsigned char g_key[LOG_AES_KEY_LEN];

// Main logging function
int SqLog(int log_level, const char *format, ...);

// Initialization functions
int SqLog_InitWriting(const char *key_file, const char *logf_path, const char *logf_name);
int SqLog_InitReading(const char *key_file);

// Logging functions
int WriteLog(const char *message);
int Sqlog_WriteLog(const char *message);  // Alternative name for WriteLog

// Log reading and decryption functions
int SqLog_ReadLog(const char *pacCiperText, size_t szCLen, char *pacPlainText, size_t szPLen);

// Log integrity verification
int SqLog_VerifyIntegrity(const char *log_file_path);

// Log management functions
void SqLog_LogStart(int argc, char *argv[]);
void SqLog_CloseFiles(void);

// Utility functions (for testing purposes)
int hex_to_bin(const char *hex, unsigned char *bin, size_t bin_len);
int load_key(const char *pcFile, unsigned char *pKeyBuf);
int base64_encode(const unsigned char *in, size_t in_len, char *out, const unsigned int out_len);
int base64_decode(const char *in, unsigned char *out, unsigned int *out_len);
int decrypt_gcm(const unsigned char *key, const unsigned char *data, size_t data_len, 
                char *plaintext_out, size_t plaintext_max);
int save_logfile_hmac(void);
int rotate_log_file_if_needed(size_t new_entry_size);
int open_new_log_file(void);
int get_max_log_index(void);
long get_file_size(const char *filename);

#ifdef __cplusplus
}
#endif

// Convenience macros for each log level (support variadic args)
#define SqLog_F(...) SqLog(LOG_LEVEL_F, __VA_ARGS__)
#define SqLog_E(...) SqLog(LOG_LEVEL_E, __VA_ARGS__)
#define SqLog_W(...) SqLog(LOG_LEVEL_W, __VA_ARGS__)
#define SqLog_I(...) SqLog(LOG_LEVEL_I, __VA_ARGS__)
#define SqLog_D(...) SqLog(LOG_LEVEL_D, __VA_ARGS__)

#endif // __SQLOG_H__
