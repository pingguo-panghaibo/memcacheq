#ifndef _MQ_NET_H
#define _MQ_NET_H
#include <sys/socket.h>
#include <fcntl.h>

#define QNET_OK 0
#define QNET_ERR -1

/*
 * all network function define
 */
int netFdNonBlock(int fd);
int netReuseAddr(int fd);
#endif