/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
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

