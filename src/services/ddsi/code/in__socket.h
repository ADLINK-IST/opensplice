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
#ifndef IN__SOCKET_H
#define IN__SOCKET_H

/*#include <netinet/in.h>*/
#include "in_socket.h"
#include "in_socketMisc.h"


/* Getters/setters */

os_boolean           in_socketGetDataSocket(
                      in_socket sock,
                      os_socket *sockfd);

os_boolean           in_socketGetControlSocket(
                      in_socket sock,
                      os_socket *sockfd);

os_boolean           in_socketSetBroadcastOption(
                      in_socket sock,
                      os_boolean enableBroadcast);


#endif /* IN__SOCKET_H */

