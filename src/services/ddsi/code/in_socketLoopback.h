
#ifndef IN_SOCKETLOOPBACK_H
#define IN_SOCKETLOOPBACK_H

#include "os_socket.h"
#include "in_socket.h"

os_boolean
in_socketGetDefaultLoopbackAddress(
    os_socket sockfd,
    struct sockaddr_in *sockAddr);

#endif /* IN_SOCKETLOOPBACK_H */

