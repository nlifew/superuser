
#ifndef SUPERUSER_SOCKET_H
#define SUPERUSER_SOCKET_H

#include <sys/un.h>

int open_local_server(const char* path, struct sockaddr_un* addr, int block);

int open_local_client(const char* path, struct sockaddr_un* addr);

#endif // SUPERUSER_SOCKET_H

