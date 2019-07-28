//
// Created by Nlifew on 2019/7/28.
//

#ifndef SUPERUSER_LOG_H
#define SUPERUSER_LOG_H

#include <string.h>
#include <errno.h>
#include <android/log.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "log", __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "log", __VA_ARGS__)

#define LOGE(...) { \
__android_log_print(ANDROID_LOG_ERROR, "log", __VA_ARGS__); \
__android_log_print(ANDROID_LOG_ERROR, "errno: %s(%d)", strerror(errno), errno); \
}


#endif //SUPERUSER_LOG_H
