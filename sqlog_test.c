#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/buffer.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdarg.h>

#include "sqlog.h"

int main()
{
    if (InitLogFromFile(LOG_KEY_FILE_PATH LOG_KEY_FILE_NAME) != 0)
    {
        fprintf(stderr, "InitLog failed\n");
        return 1;
    }

    for (int i = 0; i < 30; ++i)
    {
        char msg[128];
        snprintf(msg, sizeof(msg), "Log entry %d: Hello world!\n", i);
        WriteLog(msg);
    }
    EnLog_F("%s:%d\n", __func__, __LINE__);
    EnLog_E("%s:%d\n", __func__, __LINE__);
    EnLog_W("%s:%d\n", __func__, __LINE__);
    EnLog_I("%s:%d\n", __func__, __LINE__);
    EnLog_D("%s:%d\n", __func__, __LINE__);

    return 0;
}
