

#include <unistd.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <sys/time.h>

#include "utils/log.h"
#include "su.h"

#define SUPERUSER_PACKAGE_NAME      "cn.nlifew.superuser"
#define SUPERUSER_REQUEST_ACTIVITY  ".ui.RequestActivity"
#define SUPERUSER_RESULT_BROADCAST  ".broadcast.ResultReceiver"


int start_request_activity(uid_t uid, pid_t pid)
{
    int status = -1;

    char uid_s[8];
    char path[32];
    sprintf(uid_s, "%d", uid);
    sprintf(path, "/dev/su.d.%d", pid);

    const pid_t child = fork();
    if (child == -1) {
        LOGE("failed to fork child process\n");
        return -1;
    }
    else if (child == 0) {
        execlp("am", "am", "start", 
            "-n", SUPERUSER_PACKAGE_NAME "/" SUPERUSER_REQUEST_ACTIVITY,
            "--ei", "caller_uid", uid_s,
            "--es", "socket_path", path,
            NULL);
        exit(errno);
    }

    waitpid(child, &status, 0);
    status = WEXITSTATUS(status);

    LOGD("am exit code %d\n", status);

    if (status != 0) {
        return -1;
    }

    int fd = -1;
    int client = -1;
    struct sockaddr_un addr;

    if ((fd = open_local_server(path, &addr, 1)) == -1) {
        LOGE("failed to open tmp socket: %s\n", path);
        return -1;
    }

    LOGD("socket ready, count 20 sec ...\n");

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    struct timeval time = {
        .tv_sec     =   20,
        .tv_usec    =   0
    };

    if ((status = select(fd + 1, &fds, NULL, NULL, &time)) <= 0) {
        status = -1;
        LOGE("timeout, give up this socket \n");
        goto bail;
    }

    if ((client = accept(fd, NULL, NULL)) == -1) {
        LOGE("failed to accept client socket\n");
        goto bail;
    }

    if (read(client, &status, sizeof(status)) != sizeof(status)) {
        status = -1;
        LOGE("failed to read result data from client\n");
        goto bail;
    }
bail:
    unlink(path);
    if (fd != -1) close(fd);
    if (client != -1) close(client);
    return status;
}


int send_result_broadcast(uid_t uid, int result)
{
    pid_t child = fork();
    if (child == -1) {
        LOGE("failed to fork child process \n");
        return -1;
    }
    else if (child == 0) {

        char uid_s[8];
        char result_s[8];

        sprintf(uid_s, "%d", uid);
        sprintf(result_s, "%d", result);

        execlp("am", "am", "broadcast", 
            "-n", SUPERUSER_PACKAGE_NAME "/" SUPERUSER_RESULT_BROADCAST, 
            "-a", SUPERUSER_PACKAGE_NAME "/" SUPERUSER_RESULT_BROADCAST, 
            "--ei", "caller_uid", uid_s, 
            "--ei", "su_result", result_s, 
            NULL);
        exit(errno);
    }

    int status;
    waitpid(child, &status, 0);
    status = WEXITSTATUS(status);

    LOGD("send broadcast result : %d\n", status);

    return status == 0 ? 0 : -1;
}