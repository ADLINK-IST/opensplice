
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

in_locator
in_socketGetMulticastControlLocator(in_socket sock);


#endif /* IN__SOCKET_H */

