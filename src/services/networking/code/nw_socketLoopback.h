
#ifndef NW_SOCKETLOOPBACK_H
#define NW_SOCKETLOOPBACK_H

#include "os_socket.h"
#include "nw_socket.h"

int
nw_socketGetDefaultLoopbackAddress(
    int sockfd,
    struct sockaddr_in *sockAddr);
                       
#endif /* NW_SOCKETLOOPBACK_H */

