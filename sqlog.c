#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/buffer.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <ctype.h>

#include "sqlog.h"

static unsigned char g_key[LOG_AES_KEY_LEN];
static FILE *g_log_fp = NULL;
static int g_log_index = 0;
static char g_current_log_filename[512] = {0};
static unsigned int g_log_msg_num = 0;
static const char *g_log_file_path = LOG_FILE_PATH;
static const char *g_log_file_base_name = LOG_FILE_BASE_NAME;

static int hex_to_bin(const char *hex, unsigned char *bin, size_t bin_len);
static int load_key(const char *pcFile, unsigned char *pKeyBuf);
static int base64_encode(const unsigned char *in, size_t in_len, char *out, size_t out_len);
static int base64_decode(const char *in, unsigned char *out, size_t *out_len);
static int decrypt_gcm(const unsigned char *key,
                       const unsigned char *data,
                       size_t data_len,
                       char *plaintext_out,
                       size_t plaintext_max);

// Rename current log to .NNN and start fresh
static int rotate_log_file_if_needed(size_t new_entry_size);
// Opens a new log file based on current index
static int open_new_log_file();

/************************************************************ */
/** Public APIs ********************************************* */
/************************************************************ */
// Writes an encrypted log message to the current file
int WriteLog(const char *message)
{
    if (!g_log_fp)
        return -1;

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    unsigned char iv[LOG_AES_GCM_IV_LEN];
    unsigned char tag[LOG_AES_GCM_TAG_LEN];
    unsigned char ciphertext[1024];
    int outlen = 0, tmplen = 0;

    g_log_msg_num = (g_log_msg_num + 1) % 1000000;
    char numbered_msg[2048];
    snprintf(numbered_msg, sizeof(numbered_msg), "%06u: %s", g_log_msg_num, message);

    RAND_bytes(iv, sizeof(iv));

    EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, sizeof(iv), NULL);
    EVP_EncryptInit_ex(ctx, NULL, NULL, g_key, iv);

    EVP_EncryptUpdate(ctx, ciphertext, &outlen, (unsigned char *)numbered_msg, strlen(numbered_msg));
    EVP_EncryptFinal_ex(ctx, ciphertext + outlen, &tmplen);
    outlen += tmplen;

    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, sizeof(tag), tag);
    EVP_CIPHER_CTX_free(ctx);

    // Compose [IV][CIPHERTEXT][TAG]
    unsigned char combined[sizeof(iv) + outlen + sizeof(tag)];
    memcpy(combined, iv, sizeof(iv));
    memcpy(combined + sizeof(iv), ciphertext, outlen);
    memcpy(combined + sizeof(iv) + outlen, tag, sizeof(tag));

    // Base64 encode
    char encoded[2048];
    if (base64_encode(combined, sizeof(combined), encoded, sizeof(encoded)) != 0)
    {
        return -1;
    }

    // Before writing, check if rotation is needed
    if (rotate_log_file_if_needed(strlen(encoded)) != 0)
    {
        // If we can't open new log file.
        // FIXME later
        return -1;
    }

    // Write to file (1 line per entry)
    fprintf(g_log_fp, "%s\n", encoded);
    fflush(g_log_fp);

    printf("[%p] %s\n", g_log_fp, numbered_msg);

    return 0;
}

int SqLog(int log_level, const char *format, ...)
{
    char msg_buffer[SQLOG_BUFFER_SIZE];
    char final_buffer[SQLOG_BUFFER_SIZE];
    const char *level_str = NULL;
    va_list args;

    switch ((LOG_LEVEL)log_level)
    {
    case LOG_LEVEL_F:
        level_str = "[F] ";
        break;
    case LOG_LEVEL_E:
        level_str = "[E] ";
        break;
    case LOG_LEVEL_W:
        level_str = "[W] ";
        break;
    case LOG_LEVEL_I:
        level_str = "[I] ";
        break;
    case LOG_LEVEL_D:
        level_str = "[D] ";
        break;
    default:
        level_str = "[?] ";
        break;
    }

    // Format the user message
    va_start(args, format);
    vsnprintf(msg_buffer, sizeof(msg_buffer), format, args);
    va_end(args);

    // Get timestamp (yyyy-mm-dd hh:mm:ss.mmm)
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm *tm_info = localtime(&tv.tv_sec);

    char time_str[32];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
    int ms = tv.tv_usec / 1000;

    // Compose final_buffer: "2025-06-06 14:30:12.123 [I] message..."
    int prefix_len = snprintf(final_buffer, sizeof(final_buffer), "%s.%03d %s", time_str, ms, level_str);

    size_t msg_len = strlen(msg_buffer);
    if ((size_t)prefix_len + msg_len < sizeof(final_buffer))
    {
        // Entire message fits
        memcpy(final_buffer + prefix_len, msg_buffer, msg_len + 1);
    }
    else if ((size_t)prefix_len < sizeof(final_buffer))
    {
        // Truncate message
        size_t copy_len = sizeof(final_buffer) - prefix_len - 1;
        memcpy(final_buffer + prefix_len, msg_buffer, copy_len);
        final_buffer[sizeof(final_buffer) - 1] = '\0';
    }
    else
    {
        // Timestamp + level too long
        final_buffer[sizeof(final_buffer) - 1] = '\0';
    }

    // Split into chunks of SQLOG_LINE_MAX and call WriteLog
    size_t len = strlen(final_buffer);
    size_t offset = 0;

    while (offset < len)
    {
        size_t chunk_size = SQLOG_LINE_MAX;
        if (offset + chunk_size > len)
        {
            chunk_size = len - offset;
        }

        char chunk[SQLOG_LINE_MAX + 1];
        memcpy(chunk, final_buffer + offset, chunk_size);
        chunk[chunk_size] = '\0';

        WriteLog(chunk);
        offset += chunk_size;
    }

    return 0;
}

void SqLog_LogStart(int argc, char *argv[])
{
    // 1. retrieve the execution file name
    char exe_path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
    if (len != -1)
    {
        exe_path[len] = '\0';
    }
    else
    {
        strncpy(exe_path, argv[0], sizeof(exe_path));
        exe_path[sizeof(exe_path) - 1] = '\0';
    }

    // 2. make whole command line string
    char cmdline[4096] = {0};
    for (int i = 0; i < argc; i++)
    {
        strcat(cmdline, argv[i]);
        if (i < argc - 1)
            strcat(cmdline, " ");
    }

    // 3. commit to the log
    SqLog_I("Newly started\n");
    SqLog_I("Executable Path: %s\n", exe_path);
    SqLog_I("Command Line: %s\n", cmdline);
}

// Initializes encryption key and opens log file for writing
int SqLog_InitWriting(const char *key_file, const char *logf_path, const char *logf_name)
{
    if (load_key(key_file, g_key) != 0)
    {
        return -1;
    }

    g_log_file_path = logf_path;
    g_log_file_base_name = logf_name;
    g_log_index = 0;
    if (open_new_log_file() != 0)
        return -1;

    return 0;
}

int SqLog_InitReading(const char *key_file)
{
    if (load_key(key_file, g_key) != 0)
    {
        return -1;
    }
    return 0;
}

int SqLog_ReadLog(const char *pacCiperText, size_t szCLen,
                  char *pacPlainText, size_t szPLen)
{
    unsigned char decoded[2048];
    size_t decoded_len;

    (void)szCLen;

    decoded_len = sizeof(decoded);
    if (base64_decode(pacCiperText, decoded, &decoded_len) != 0)
    {
        fprintf(stderr, "[!] Failed to base64-decode line\n");
        return -1;
    }

    if (decrypt_gcm(g_key, decoded, decoded_len, pacPlainText, szPLen) != 0)
    {
        fprintf(stderr, "[!] Failed to decrypt line\n");
        return -1;
    }
    return 0;
}

/************************************************************ */
/** File handling ******************************************* */
/************************************************************ */

// Returns current log file size
static long get_file_size(const char *filename)
{
    struct stat st;
    if (stat(filename, &st) != 0)
        return 0;
    return st.st_size;
}

// Builds log filename with optional index suffix
static void build_log_filename(int index, char *out, size_t out_len)
{
    if (index == 0)
    {
        snprintf(out, out_len, "%s%s", g_log_file_path, g_log_file_base_name);
    }
    else
    {
        snprintf(out, out_len, "%s%s.%03d", g_log_file_path, g_log_file_base_name, index);
    }
}

// Find the highest existing log file index: dump1090.log.001, .002, ...
static int get_max_log_index()
{
    DIR *dir = opendir(g_log_file_path);
    if (!dir)
        return 0;

    struct dirent *entry;
    int max_index = 0;

    // Extract base filename (e.g., "dump1090.log" -> "dump1090")
    const char *filename = strrchr(g_log_file_path, '/');
    filename = filename ? filename + 1 : g_log_file_path;

    char base_name[256];
    strncpy(base_name, filename, sizeof(base_name));
    base_name[sizeof(base_name) - 1] = '\0';

    size_t base_len = strlen(base_name);

    while ((entry = readdir(dir)) != NULL)
    {
        // Check prefix match
        if (strncmp(entry->d_name, base_name, base_len) == 0)
        {
            const char *suffix = entry->d_name + base_len;

            // Check if suffix is like ".###"
            if (*suffix == '.' && isdigit((unsigned char)suffix[1]))
            {
                int idx = 0;
                if (sscanf(suffix + 1, "%03d", &idx) == 1 && idx > max_index)
                {
                    max_index = idx;
                }
            }
        }
    }

    closedir(dir);
    return max_index;
}

// Rename current log to .NNN and start fresh
static int rotate_log_file_if_needed(size_t new_entry_size)
{
    long size = get_file_size(g_current_log_filename);
    if (size + new_entry_size < LOG_FILE_SIZE)
        return 0;

    // Close current file
    if (g_log_fp)
    {
        fclose(g_log_fp);
        g_log_fp = NULL;
    }

    // Find next index
    int max_idx = get_max_log_index();
    char rotated_name[512];
    snprintf(rotated_name, sizeof(rotated_name),
             "%s%s.%03d", g_log_file_path, g_log_file_base_name, max_idx + 1);
    // Rename current log to rotated
    printf("%s ren [%s] ==> [%s]\n", __func__, g_current_log_filename, rotated_name);
    rename(g_current_log_filename, rotated_name);

    // Open new log file as dump1090.log
    snprintf(g_current_log_filename, sizeof(g_current_log_filename),
             "%s%s", g_log_file_path, g_log_file_base_name);

    g_log_fp = fopen(g_current_log_filename, "wb");
    printf("%s --> %s, %p\n", __func__, g_current_log_filename, g_log_fp);
    return g_log_fp ? 0 : -1;
}

// Opens a new log file based on current index
static int open_new_log_file()
{
    build_log_filename(g_log_index, g_current_log_filename, sizeof(g_current_log_filename));
    g_log_fp = fopen(g_current_log_filename, "ab");
    printf("%s --> %s\n", __func__, g_current_log_filename);
    return g_log_fp != NULL ? 0 : -1;
}

/************************************************************ */
/** Key handling ******************************************** */
/************************************************************ */
static int load_key(const char *pcFile, unsigned char *pKeyBuf)
{
    char key_path[512];

    printf("key file=%s\n", pcFile);
    // Use default path if pcFile is NULL
    if (pcFile == NULL)
    {
        snprintf(key_path, sizeof(key_path), "%s%s", LOG_KEY_FILE_PATH, LOG_KEY_FILE_NAME);
        pcFile = key_path;
    }

    FILE *kf = fopen(pcFile, "r");
    if (!kf)
    {
        fprintf(stderr, "Failed to load AES key from %s\n", pcFile);
        return -1;
    }

    char hex_key[LOG_AES_KEY_LEN * 2 + 1] = {0};
    if (!fgets(hex_key, sizeof(hex_key), kf))
    {
        fclose(kf);
        return -1;
    }
    fclose(kf);

    if (strlen(hex_key) < LOG_AES_KEY_LEN * 2)
        return -1;

    return hex_to_bin(hex_key, pKeyBuf, LOG_AES_KEY_LEN);
}

/************************************************************ */
/** Base64 ************************************************** */
/************************************************************ */

static int base64_encode(const unsigned char *in, size_t in_len, char *out, size_t out_len)
{
    BIO *bio, *b64;
    BUF_MEM *buffer_ptr;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bio);
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL); // No newlines

    BIO_write(b64, in, in_len);
    BIO_flush(b64);
    BIO_get_mem_ptr(b64, &buffer_ptr);

    if (buffer_ptr->length >= out_len)
    {
        BIO_free_all(b64);
        return -1;
    }

    memcpy(out, buffer_ptr->data, buffer_ptr->length);
    out[buffer_ptr->length] = '\0';

    BIO_free_all(b64);
    return 0;
}

int base64_decode(const char *in, unsigned char *out, size_t *out_len)
{
    BIO *bio, *b64;
    int decoded_len;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new_mem_buf(in, -1);
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); // No newlines

    decoded_len = BIO_read(bio, out, *out_len);
    if (decoded_len < 0)
    {
        BIO_free_all(bio);
        return -1;
    }

    *out_len = decoded_len;
    BIO_free_all(bio);
    return 0;
}

// Converts hex string to binary buffer
static int hex_to_bin(const char *hex, unsigned char *bin, size_t bin_len)
{
    for (size_t i = 0; i < bin_len; ++i)
    {
        if (sscanf(&hex[i * 2], "%2hhx", &bin[i]) != 1)
            return -1;
    }
    return 0;
}

/************************************************************ */
/** Encrpytion/Decryption *********************************** */
/************************************************************ */
// Decrypts a GCM-encrypted log entry
static int decrypt_gcm(const unsigned char *key,
                       const unsigned char *data,
                       size_t data_len,
                       char *plaintext_out,
                       size_t plaintext_max)
{
    (void)plaintext_max; // Suppress unused parameter warning

    if (data_len < LOG_AES_GCM_IV_LEN + LOG_AES_GCM_TAG_LEN)
        return -1;

    const unsigned char *iv = data;
    const unsigned char *ciphertext = data + LOG_AES_GCM_IV_LEN;
    size_t ciphertext_len = data_len - LOG_AES_GCM_IV_LEN - LOG_AES_GCM_TAG_LEN;
    const unsigned char *tag = data + LOG_AES_GCM_IV_LEN + ciphertext_len;

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    int outlen = 0, tmplen = 0;

    EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, LOG_AES_GCM_IV_LEN, NULL);
    EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv);

    EVP_DecryptUpdate(ctx, (unsigned char *)plaintext_out, &outlen, ciphertext, ciphertext_len);
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, LOG_AES_GCM_TAG_LEN, (void *)tag);

    if (EVP_DecryptFinal_ex(ctx, (unsigned char *)plaintext_out + outlen, &tmplen) <= 0)
    {
        EVP_CIPHER_CTX_free(ctx);
        return -1; // Decryption failed (tag mismatch)
    }

    outlen += tmplen;
    plaintext_out[outlen] = '\0';

    EVP_CIPHER_CTX_free(ctx);
    return 0;
}
