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
#ifndef IN_SOCKETLOOPBACK_H
#define IN_SOCKETLOOPBACK_H

#include "os_socket.h"
#include "in_socket.h"

os_boolean
in_socketGetDefaultLoopbackAddress(
    os_socket sockfd,
    struct sockaddr_in *sockAddr);

#endif /* IN_SOCKETLOOPBACK_H */

