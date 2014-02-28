/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

#ifndef OS_SOLARIS_SOCKET_H
#define OS_SOLARIS_SOCKET_H

#if defined (__cplusplus)
extern "C" {
#endif

#include <sys/socket.h>
#include <sys/sockio.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* Keep defines before common header */
/* @todo dds#2800 - Sort this out properly */
#define OS_SOLARIS
#define OS_NO_GETIFADDRS
#define OS_NO_NETLINK
#define OS_IFNAMESIZE        	IF_NAMESIZE
#define OS_LIFNAMESIZE        	LIFNAMSIZ
#define OS_SOCKET_HAS_SA_LEN 	0
#define OS_SOCKET_HAS_IPV6      1

#include "../common/include/os_socket.h"

#ifndef INADDR_NONE
#define INADDR_NONE             (in_addr_t)(-1)
#endif
#if defined (__cplusplus)
}
#endif

#endif /* OS_SOLARIS_SOCKET_H */
