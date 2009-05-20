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

#ifndef NW__SOCKET_H
#define NW__SOCKET_H

#include "nw_socket.h"
#include "nw_socketMisc.h"

              
/* Getters/setters */

sk_bool           nw_socketGetDataSocket(
                      nw_socket sock,
                      int *sockfd);
                      
sk_bool           nw_socketGetControlSocket(
                      nw_socket sock,
                      int *sockfd);
                      
int               nw_socketSetBroadcastOption(
                      nw_socket sock,
                      int enableBroadcast);

#endif /* NW__SOCKET_H */

