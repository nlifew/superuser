

#ifndef _su_daemon_h_
#define _su_daemon_h_

#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "su.h"
#include "utils/log.h"
#include "activity.h"


#define LOCAL_SOCKET_PATH   "/dev/su.d"

int start_daemon();
int handle_client_socket(int server, int client);
void exec_su_args(struct ucred* cred, struct su_args* args);


int start_daemon()
{
    int fd = -1;
    int status = -1;

    struct sockaddr_un addr;

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


void exec_su_args(struct ucred* cred, struct su_args* args)
{
    int status = -1;

    LOGD("convert args\n");

    char** argv = (char**) malloc(sizeof(char*)*(args->argc + 1));
    if (argv == NULL) {
        LOGE("failed to malloc memery %lu bytes\n", sizeof(char*)*(args->argc + 1));
        goto bail;
    }
    char* tmp = args->args;
    for (int i = 0; i < args->argc; i++) {
        argv[i] = tmp;
        tmp = strchr(tmp, ' ');
        *(tmp++) = '\0';
    }
    argv[args->argc] = NULL;


    LOGD("open remote stdio");

    char buff[64];
    int fin = -1, fout = -1, ferr = -1;

    sprintf(buff, "/proc/%d/fd/0", cred->pid);
    fin = open(buff, O_RDONLY);

    sprintf(buff, "/proc/%d/fd/1", cred->pid);
    fout = open(buff, O_WRONLY);

    sprintf(buff, "/proc/%d/fd/2", cred->pid);
    ferr = open(buff, O_WRONLY);

    if (fin == -1 || fout == -1 || ferr == -1) {
        LOGD("failed to open std\n");
        goto bail;
    }

    if (dup2(fin, 0) == -1 || dup2(fout, 1) == -1 || dup2(ferr, 2) == -1) {
        LOGD("failed to dup remote std\n");
        goto bail;
    }

    setuid(args->uid);
    chdir("/");
    execvp(argv[0], argv);

bail:
    if (fin != -1) close(fin);
    if (fout != -1) close(fout);
    if (ferr != -1) close(ferr);
    if (argv != NULL) free(argv);

    exit(status);
}


#endif /* _su_daemon_h_ */