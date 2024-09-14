//
// Created by Nlifew on 2019/7/28.
//

#ifndef SUPERUSER_LOG_H
#define SUPERUSER_LOG_H

#include <string.h>
#include <errno.h>

#if NO_ANDROID_LOG

#include <stdio.h>

#define LOGI(fmt, ...)  fprintf(stderr, fmt, ##__VA_ARGS__)
#define LOGD(fmt, ...)  fprintf(stderr, fmt, ##__VA_ARGS__)
#define LOGE(fmt, ...)  fprintf(stderr, fmt "errno=%d(%s)", ##__VA_ARGS__, errno, strerror(errno))

#else

#include <android/log.h>
#define LOG_TAG "superuser"

#define LOGI(fmt, ...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, fmt, ##__VA_ARGS__)
#define LOGD(fmt, ...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, fmt, ##__VA_ARGS__)
#define LOGE(fmt, ...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, fmt "errno=%d(%s)", ##__VA_ARGS__, errno, strerror(errno))


#endif


#endif //SUPERUSER_LOG_H
