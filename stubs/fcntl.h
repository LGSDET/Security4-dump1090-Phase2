#ifndef STUB_FCNTL_H
#define STUB_FCNTL_H

#define F_GETFL    3
#define F_SETFL    4
#define O_NONBLOCK 0x800
#define O_RDONLY   0x0000
#define O_WRONLY   0x0001
#define O_RDWR     0x0002
#define O_CREAT    0x0100

static inline int fcntl(int fd, int cmd, ...) {
    return 0;  // stub
}

#endif
