

#define _GNU_SOURCE

#include <string.h>

#include "utils/log.h"
#include "su.h"
#include "daemon.h"
#include "client.h"

#define SU_VERSION "1.0.0"

void usage()
{
    LOGI("usage\n");
}


int main(int argc, char const *argv[])
{
    struct su_args args = {
        .uid = 0,
        .argc = 1,
        .args = "sh "
    };
    
    return argc == 2 && strcmp(argv[1], "--daemon") == 0 ?
        start_daemon() : start_client(&args);    
}