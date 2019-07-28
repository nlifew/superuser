//
// Created by Nlifew on 2019/7/28.
//


#include <jni.h>
#include "log.h"
#include "su.h"

JNIEXPORT void JNICALL
Java_cn_nlifew_superuser_ui_RequestActivity_replySocketResult
        (JNIEnv* env, jclass type, jstring _path, jint result)
{
    const char* path = (*env)->GetStringUTFChars(env, _path, NULL);

    int fd = -1;

    struct sockaddr_un addr;
    socklen_t len = sizeof(addr);

    if ((fd = open_local_client(path, &addr)) == -1 ||
        connect(fd, (struct sockaddr*) &addr, len) == -1 ||
        write(fd, &result, sizeof(result)) != sizeof(result) ) {

        LOGE("failed to write result %d to socket %s\n", result, path);
    }

bail:
    if (fd != -1) close(fd);
    (*env)->ReleaseStringUTFChars(env, _path, path);
}
