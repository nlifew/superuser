
#include <string.h>

#include "su.h"


int main(int argc, char const *argv[])
{
    struct su_args args = {
            .uid = 0,
            .argc = 1,
            .args = "sh"
    };

    return argc == 2 && strcmp(argv[1], "--daemon") == 0 ?
           start_daemon() : exec_su(&args);
}