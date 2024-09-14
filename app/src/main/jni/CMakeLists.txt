cmake_minimum_required(VERSION 3.4.1)

add_compile_options("-DNO_ANDROID_LOG")

add_library(
        request
        SHARED
        java_request.c
        socket.c
)

target_link_libraries(
        request
        log
)

add_executable(
        libsu.so # lib{name}.so 的才会打包进 apk
        su.c
        su_daemon.c
        su_exec.c
        socket.c
)

target_link_libraries(
        libsu.so
        log
)