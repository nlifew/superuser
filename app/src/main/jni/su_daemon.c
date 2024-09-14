
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "su.h"
#include "log.h"
#include "socket.h"

#define SUPERUSER_PACKAGE_NAME      "cn.nlifew.superuser"
#define SUPERUSER_REQUEST_ACTIVITY  ".ui.RequestActivity"
#define SUPERUSER_RESULT_BROADCAST  ".broadcast.ResultReceiver"


static int handle_client_socket(int server, int client);
static int start_request_activity(uid_t uid, pid_t pid);
static int send_result_broadcast(uid_t uid, int result);
static void exec_su_args(struct ucred* cred, struct su_args* args);

int start_daemon() {
    int fd = -1;
    int status = -1;

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));

    if (getuid() != 0 || getgid() != 0) {
        LOGE("daemon must run with root user\n");
        goto bail;
    }

    if ((fd = open_local_server(LOCAL_SOCKET_PATH, &addr, 5)) == -1) {
        LOGE("failed to open local socket\n");
        goto bail;
    }

    LOGD("ok, now waiting for a new client socket ...\n");

    int client = accept(fd, NULL, NULL);
    status = handle_client_socket(fd, client);

    bail:
    if (fd != -1) close(fd);
    unlink(LOCAL_SOCKET_PATH);
    return status;
}


int handle_client_socket(int server_fd, int client_fd)
{
    int status = -1;

    struct su_args args;
    int bytes;

    if (client_fd == -1) {
        LOGE("invalid client fd: -1\n");
        goto bail;
    }

    LOGD("connect ok, now reading su_args ...\n");

    if ((bytes = read(client_fd, &args, sizeof(args))) != sizeof(args)) {
        LOGE("failed when reading struct su_args info\n"
             "expected %d bytes, actually %d bytes\n", (int) sizeof(args), bytes);
        goto bail;
    }

    LOGD("su_args = {uid = %d, args = %s}\n", args.uid, args.args);
    LOGD("done, check the client\n");

    struct ucred cred;
    socklen_t len = sizeof(cred);
    if (getsockopt(client_fd, SOL_SOCKET, SO_PEERCRED, &cred, &len)) {
        LOGE("failed to getsockopt\n");
        goto bail;
    }

    LOGD("ucred = {pid = %d, uid = %d, gid = %d}\n", cred.pid, cred.uid, cred.gid);
    LOGD("start superuser activity ...\n");

    status = start_request_activity(cred.uid, cred.pid);
    send_result_broadcast(cred.uid, status);

    if (status == -1) {
        LOGE("denyed by SuperUser app\n");
        goto bail;
    }

    pid_t child = fork();

    if (child == 0) {
        // 子进程
        close(client_fd);
        close(server_fd);
        exec_su_args(&cred, &args);
    }

    // 父进程
    LOGD("waiting child process %d ...\n", child);
    if (child != -1) waitpid(child, &status, 0);

    status = WEXITSTATUS(status);
    LOGD("exit code %d\n", status);


bail:
    if (write(client_fd, &status, sizeof(int)) != sizeof(int)) {
        LOGE("failed when send result code to the client\n");
    }
    if (client_fd != -1) close(client_fd);
    return status;
}


int start_request_activity(uid_t uid, pid_t pid)
{
    int status = -1;

    char uid_s[8];
    char path[32];
    snprintf(uid_s, sizeof(uid_s), "%d", uid);
    snprintf(path, sizeof(path), "/dev/su.d.%d", pid);

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
    memset(&addr, 0, sizeof(addr));

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

        snprintf(uid_s, sizeof(uid_s), "%d", uid);
        snprintf(result_s, sizeof(uid_s), "%d", result);

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


/**
 * "su -c id" -> {"su", "-c", "id" }
 * "   su  xxx  " -> { "su", "xxx" }
 * 返回值中每个 string 都必须手动释放掉
 */
static char** split_cmd_line(const char *args) {
    char *str = strdup(args);
    size_t n = 0;
    for (char *s = strtok(str, " "); s != NULL; s = strtok(NULL, " ")) {
        n += 1;
    }
    strcpy(str, args);

    char **array = malloc(sizeof(char *) * (n + 1));
    int arrayIndex = 0;
    for (char *s = strtok(str, " "); s != NULL; s = strtok(NULL, " ")) {
        array[arrayIndex] = strdup(s);
        arrayIndex += 1;
    }
    array[arrayIndex] = NULL;
    free(str);
    return array;
}

void exec_su_args(struct ucred* cred, struct su_args* args)
{
    int status = -1;
    int fin = -1, fout = -1, ferr = -1;

    LOGD("convert args\n");
    char** argv = split_cmd_line(args->args);
    for (int i = 0; argv[i] != NULL; i ++) {
        LOGD("argv[%d] = '%s\n'", i, argv[i]);
    }

    LOGD("open remote stdio\n");
    char buff[64];

    snprintf(buff, sizeof(buff), "/proc/%d/fd/0", cred->pid);
    fin = open(buff, O_RDONLY);

    snprintf(buff, sizeof(buff), "/proc/%d/fd/1", cred->pid);
    fout = open(buff, O_WRONLY);

    snprintf(buff, sizeof(buff), "/proc/%d/fd/2", cred->pid);
    ferr = open(buff, O_WRONLY);

    if (fin == -1 || fout == -1 || ferr == -1) {
        LOGD("failed to open std\n");
        goto bail;
    }

    if (dup2(fin, 0) == -1 || dup2(fout, 1) == -1 || dup2(ferr, 2) == -1) {
        LOGD("failed to dup remote std\n");
        goto bail;
    }

    LOGD("exec su args\n");
    setuid(args->uid);
    chdir("/");
    execvp(argv[0], argv);

bail:
    if (fin != -1) close(fin);
    if (fout != -1) close(fout);
    if (ferr != -1) close(ferr);
    exit(status);
}