//
// Created by Nlifew on 2019/7/28.
//

#ifndef SUPERUSER_SU_H
#define SUPERUSER_SU_H


#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "log.h"

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

#endif //SUPERUSER_SU_H
