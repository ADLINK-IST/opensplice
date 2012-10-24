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

#ifndef NW_SOCKETMULTICAST_H
#define NW_SOCKETMULTICAST_H

#include "os_socket.h"
#include "nw_socket.h"

os_int
nw_socketGetDefaultMulticastInterface(
    nw_socket this_,
    const char *addressLookingFor,
    os_socket sockfd,
    os_sockaddr_storage* sockAddrPrimary,
    os_sockaddr_storage* sockAddrBroadcasat);

void
nw_socketMulticastInitialize(
    nw_socket socket,
    sk_bool receiving,
    os_sockaddr_storage* address);

void
nw_socketMulticastAddPartition(
    nw_socket sock,
    const char *addressString,
    sk_bool receiving,
    os_uchar mTTL);

os_int
nw_socketMulticastSetTTL(
    nw_socket socket,
    os_uchar timeToLive);



#endif /* NW_SOCKETMULTICAST_H */
