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

/* @todo dds#2800 - Sort this out properly */
#define OS_SOLARIS

#define OS_NO_GETIFADDRS

#include "../common/include/os_socket.h"

#define OS_IFNAMESIZE        	IF_NAMESIZE
#define OS_SOCKET_HAS_SA_LEN 	0
#define INADDR_NONE             (in_addr_t)(-1)

#if defined (__cplusplus)
}
#endif

#endif /* OS_SOLARIS_SOCKET_H */
