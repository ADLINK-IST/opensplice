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

#ifndef OS_DARWIN_SOCKET_H
#define OS_DARWIN_SOCKET_H

#if defined (__cplusplus)
extern "C" {
#endif

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <net/if.h>

#include <sys/select.h>
#include <sys/sockio.h>
#include <unistd.h>

#include <ifaddrs.h>

#include <../common/include/os_socket.h>

#define OS_IFNAMESIZE		IF_NAMESIZE
#define OS_SOCKET_HAS_SA_LEN	0

#if defined (__cplusplus)
}
#endif

#endif /* OS_DARWIN_SOCKET_H */
