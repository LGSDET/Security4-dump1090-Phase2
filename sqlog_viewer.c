#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

#include "sqlog.h"

// Get user confirmation
static int ask_user_confirmation(const char *message)
{
    char response[10];
    printf("%s (y/n): ", message);
    fflush(stdout);

    if (fgets(response, sizeof(response), stdin) != NULL)
    {
        return (response[0] == 'y' || response[0] == 'Y');
    }
    return 0;
}

int main(int argc, char *argv[])
{
#if 1
    const char *keyfile = LOG_KEY_FILE_PATH LOG_KEY_FILE_NAME;
    const char *logfile = LOG_FILE_PATH LOG_FILE_BASE_NAME;
#else
    const char *keyfile = "./lgess2025s4testlogkey.hex";
    const char *logfile = "test.log";
#endif

    if (argc == 2)
    {
        logfile = argv[1];
    }
    else if (argc == 3)
    {
        keyfile = argv[1];
        logfile = argv[2];
    }
    else if (argc != 1)
    {
        fprintf(stderr,
                "Usage:\n"
                "  %s [logfile]\n"
                "  %s [keyfile] [logfile]\n",
                argv[0], argv[0]);
        return -1;
    }

    if (SqLog_InitReading(keyfile) != 0)
    {
        fprintf(stderr, "Failed to initialize log reading with key: %s\n", keyfile);
        return -1;
    }

    // Perform integrity check
    printf("Performing integrity check...\n");
    printf("Log file: %s\n", logfile);
    printf("----------------------------------------\n");

    int integrity_result = SqLog_VerifyIntegrity(logfile);

    switch (integrity_result)
    {
    case 0:
        break;
    case -1:
        fprintf(stderr, "[ERROR] Cannot open log file\n");
        return 1;
    case -2:
        fprintf(stderr, "[ERROR] Log decryption failed - file may be corrupted\n");
        if (!ask_user_confirmation("Continue reading anyway?"))
        {
            return 1;
        }
        break;
    case -3:
        fprintf(stderr, "[ERROR] No valid log entries found\n");
        if (!ask_user_confirmation("Continue anyway?"))
        {
            return 1;
        }
        break;
    case -4:
        fprintf(stderr, "[WARNING] Hash file not found - cannot verify integrity\n");
        if (!ask_user_confirmation("Continue reading without integrity verification?"))
        {
            return 1;
        }
        break;
    case -5:
        fprintf(stderr, "[ERROR] Hash file corrupted\n");
        if (!ask_user_confirmation("Continue reading anyway?"))
        {
            return 1;
        }
        break;
    case -6:
        fprintf(stderr, "[CRITICAL] Log integrity verification FAILED!\n");
        fprintf(stderr, "[WARNING] The log file may have been tampered with!\n");
        if (!ask_user_confirmation("Continue reading potentially compromised log?"))
        {
            return 1;
        }
        break;
    default:
        fprintf(stderr, "[ERROR] Unknown integrity check error (%d)\n", integrity_result);
        if (!ask_user_confirmation("Continue anyway?"))
        {
            return 1;
        }
        break;
    }

    // Read log file
    printf("----------------------------------------\n");
    printf("Reading log file: %s\n", logfile);
    printf("----------------------------------------\n");

    FILE *fp = fopen(logfile, "r");
    if (!fp)
    {
        fprintf(stderr, "Failed to open log file: %s\n", logfile);
        return 1;
    }

    char line[4096];
    char plaintext[2048];
    int line_count = 0;
    int error_count = 0;

    while (fgets(line, sizeof(line), fp))
    {
        line_count++;
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n')
            line[len - 1] = '\0';

        if (SqLog_ReadLog(line, len, plaintext, sizeof(plaintext)) == 0)
        {
            printf("%s", plaintext);
        }
        else
        {
            fprintf(stderr, "[!] Line %d: Failed to decrypt log entry\n", line_count);
            error_count++;
        }
    }

    fclose(fp);

    printf("----------------------------------------\n");
    printf("Log reading completed.\n");
    printf("Total lines processed: %d\n", line_count);
    if (error_count > 0)
    {
        printf("Decryption errors: %d\n", error_count);
    }

    return 0;
}