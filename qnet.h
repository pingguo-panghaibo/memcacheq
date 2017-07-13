#ifndef _MQ_NET_H
#define _MQ_NET_H
#include <sys/socket.h>
#include <fcntl.h>

#define QNET_OK 0
#define QNET_ERR -1

int netSetBlockState(int fd, int state);
int netFdBlock(int fd);
int netFdNonBlock(int fd);

#endif