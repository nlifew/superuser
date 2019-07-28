
#ifndef _su_main_h_
#define _su_main_h_

#include <unistd.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>

#include "utils/log.h"

struct su_args {
    uid_t uid;
    int argc;
    char args[256];
};


int open_local_server(const char* path, struct sockaddr_un* addr, int block)
{
    int fd = -1;

    if (access(path, F_OK) == 0 && unlink(path)) {
        LOGE("can\'t remove existed unix socket channel: %s\n", path);
        goto bail;
    }
    
    if ((fd = socket(AF_UNIX, SOCK_STREAM|SOCK_CLOEXEC, 0)) == -1) {
        LOGE("failed when creating socket\n");
        goto bail;
    }

    addr->sun_family = AF_UNIX;
    strcpy(addr->sun_path, path);

    if (bind(fd, (struct sockaddr*) addr, sizeof(*addr)) == -1) {
        LOGE("failed when binding socket\n");
        goto bail;
    }

    if (listen(fd, block) == -1) {
        LOGE("failed when listening socket\n");
        goto bail;
    }

    chmod(path, 0666);

    LOGD("all things is done, return\n");
    return fd;
bail:
    if (fd != -1) close(fd);
    return -1;
}


int open_local_client(const char* path, struct sockaddr_un* addr)
{
    int fd = socket(AF_UNIX, SOCK_STREAM|SOCK_CLOEXEC, 0);

    if (fd == -1) {
        LOGE("failed when creating socket\n");
        goto bail;
    }

    addr->sun_family = AF_UNIX;
    strcpy(addr->sun_path, path);

    LOGD("all things is done, return\n");
    return fd;
bail:
    if (fd != -1) close(fd);
    return -1;
}

#endif