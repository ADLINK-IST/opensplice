/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2010 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

#ifndef NW_SOCKETLOOPBACK_H
#define NW_SOCKETLOOPBACK_H

#include "os_socket.h"
#include "nw_socket.h"

os_int
nw_socketGetDefaultLoopbackAddress(
    os_int sockfd,
    struct sockaddr_in *sockAddr);
                       
#endif /* NW_SOCKETLOOPBACK_H */

