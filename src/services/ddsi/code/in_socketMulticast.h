
#ifndef IN_SOCKETMULTICAST_H
#define IN_SOCKETMULTICAST_H

#include "os_socket.h"
#include "in_socket.h"

os_boolean
in_socketGetDefaultMulticastInterface(
    const char *addressLookingFor,
    os_socket sockfd,
    struct sockaddr_in *sockAddrPrimary,
    struct sockaddr_in *sockAddrBroadcasat);


void
in_socketMulticastInitialize(
    in_socket socket);

void
in_socketMulticastAddPartition(
    in_socket sock,
    const char *addressString);


#endif /* IN_SOCKETMULTICAST_H */
