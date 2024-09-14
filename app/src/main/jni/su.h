//
// Created by Nlifew on 2019/7/28.
//

#ifndef SUPERUSER_SU_DAEMON_H
#define SUPERUSER_SU_DAEMON_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sys/socket.h> // struct ucred

#define LOCAL_SOCKET_PATH   "/dev/su.d"

struct su_args {
    uid_t uid;
    int argc;
    char args[256];
};

int start_daemon();
int exec_su(struct su_args *args);

#endif //SUPERUSER_SU_DAEMON_H
