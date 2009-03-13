
#ifndef NW_SOCKETBROADCAST_H
#define NW_SOCKETBROADCAST_H

#include "os_socket.h"
#include "nw_socket.h"

int
nw_socketGetDefaultBroadcastInterface(
    const char *addressLookingFor,
    int sockfd,
    struct sockaddr_in *sockAddrPrimary,
    struct sockaddr_in *sockAddrBroadcast);

void
nw_socketBroadcastInitialize(
    nw_socket socket);

                      
#endif /* NW_SOCKETBROADCAST_H */

