#ifndef STUB_UNISTD_H
#define STUB_UNISTD_H

#ifdef _WIN32

#include <stddef.h>   // size_t
#include <time.h>     // time_t
#include <string.h>   // strncpy

// readlink stub
static inline ssize_t readlink(const char *path, char *buf, size_t bufsize) {
    const char *fake = "C:\\Program Files\\App\\stub.exe";
    size_t len = strlen(fake);
    if (len >= bufsize) len = bufsize - 1;
    strncpy(buf, fake, len);
    buf[len] = '\0';
    return (ssize_t)len;
}

// ✅ usleep stub — just a no-op
static inline int usleep(unsigned int usec) {
    return 0;
}

#endif  // _WIN32

#endif  // STUB_UNISTD_H
