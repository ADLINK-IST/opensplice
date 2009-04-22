
#ifndef IN_SOCKETBROADCAST_H
#define IN_SOCKETBROADCAST_H

#include "os_socket.h"
#include "in_socket.h"

os_boolean
in_socketGetDefaultBroadcastInterface(
    const char *addressLookingFor,
    os_socket sockfd,
    struct sockaddr_in *sockAddrPrimary,
    struct sockaddr_in *sockAddrBroadcast);

void
in_socketBroadcastInitialize(
    in_socket socket);


#endif /* IN_SOCKETBROADCAST_H */

