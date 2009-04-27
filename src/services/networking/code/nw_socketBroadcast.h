/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

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

