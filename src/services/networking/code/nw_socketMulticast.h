
#ifndef NW_SOCKETMULTICAST_H
#define NW_SOCKETMULTICAST_H

#include "os_socket.h"
#include "nw_socket.h"

int
nw_socketGetDefaultMulticastInterface(
    const char *addressLookingFor,
    int sockfd,
    struct sockaddr_in *sockAddrPrimary,
    struct sockaddr_in *sockAddrBroadcasat);


void
nw_socketMulticastInitialize(
    nw_socket socket);

void
nw_socketMulticastAddPartition(
    nw_socket sock,
    const char *addressString);


#endif /* NW_SOCKETMULTICAST_H */
