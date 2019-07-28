//
// Created by Nlifew on 2019/6/22.
//

#ifndef _util_log_h_
#define _util_log_h_

#include <stdio.h>
#include <errno.h>
#include <string.h>

#define LOGI(...) 	printf(__VA_ARGS__)
#define LOGD(...)	printf(__VA_ARGS__)

#define LOGE(...)   { \
    fprintf(stderr, __VA_ARGS__); \
    fprintf(stderr, "errno: %s(%d)\n", strerror(errno), errno); \
}



#endif /* _util_log_h_ */
