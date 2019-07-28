
#ifndef _su_client_h_
#define _su_client_h_

#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "utils/log.h"
#include "su.h"


int handle_server_socket(int fd, struct su_args* args)
{
    int status = -1;
    int bytes;

    LOGD("connect ok, now send su_args struct ...\n");

    if ((bytes = write(fd, args, sizeof(*args))) != sizeof(*args)) {
        LOGE("failed to send su_args struct\n"
            "expected %d bytes, actually %d bytes\n", (int) sizeof(*args), bytes);
        goto bail;
    }

    LOGD("send done, now waiting for exit code ...\n");
    if ((bytes = read(fd, &status, sizeof(int))) != sizeof(int)) {
        status = 1;
        LOGE("failed when receiving exit code\n");
        goto bail;
    }

    LOGD("exit code %d\n", status);
bail:

    return status; 
}

int start_client(struct su_args* args)
{
    int fd = -1;
    int status = -1;

    struct sockaddr_un addr;

    if ((fd = open_local_client(LOCAL_SOCKET_PATH, &addr)) == -1) {
        LOGE("failed when open client socket\n");
        goto bail;
    }

    if (connect(fd, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
        LOGE("failed to connect to the server\n");
        goto bail;
    }

    status = handle_server_socket(fd, args);

bail:
    if (fd != -1) close(fd);
    return status;
}

#endif /* _su_client_h_ */