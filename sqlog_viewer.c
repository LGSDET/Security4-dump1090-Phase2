#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

#include "sqlog.h"

int main(int argc, char *argv[])
{
#if 1
    const char *keyfile = LOG_KEY_FILE_PATH LOG_KEY_FILE_NAME;
    const char *logfile = LOG_FILE_PATH LOG_FILE_BASE_NAME;
#else
    const char *keyfile = "./lgess2025s4testlogkey.hex";
    const char *logfile = "test.log";
#endif

    if (argc == 2) {
        logfile = argv[1];
    } else if (argc == 3) {
        keyfile = argv[1];
        logfile = argv[2];
    } else if (argc == 1) {
        // maintain the default value
    }else {
        fprintf(stderr,
            "Usage:\n"
            "  %s [logfile]\n"
            "  %s [keyfile] [logfile]\n", argv[0], argv[0]);
        return -1;
    }

    if (SqLog_InitReading(keyfile) != 0) {
        fprintf(stderr, "Failed to initialize log reading with key: %s\n", keyfile);
        return -1;
    }

    printf("Reading log file: %s\n", logfile);
    FILE *fp = fopen(logfile, "r");
    if (!fp)
    {
        fprintf(stderr, "Failed to open log file: %s\n", logfile);
        return 1;
    }

    char line[4096];
    char plaintext[2048];

    while (fgets(line, sizeof(line), fp))
    {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n')
            line[len - 1] = '\0'; // remove newline

#if 1
        if(SqLog_ReadLog(line, len, plaintext, sizeof(plaintext)) == 0)
        {
            printf("%s", plaintext);
        }
#else
        decoded_len = sizeof(decoded);
        if (base64_decode(line, decoded, &decoded_len) != 0)
        {
            fprintf(stderr, "[!] Failed to base64-decode line\n");
            continue;
        }

        if (decrypt_gcm(key, decoded, decoded_len, plaintext, sizeof(plaintext)) != 0)
        {
            fprintf(stderr, "[!] Failed to decrypt line\n");
            continue;
        }
#endif

    }

    fclose(fp);
    return 0;
}
