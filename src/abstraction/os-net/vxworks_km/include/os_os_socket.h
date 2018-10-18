/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#ifndef OS_VXWORKS_SOCKET_H
#define OS_VXWORKS_SOCKET_H

#if defined (__cplusplus)
extern "C" {
#endif

#include <vxWorks.h>
#include <sockLib.h>
#include <ioLib.h>
#include <netinet/in.h>
#if ! defined (OSPL_VXWORKS653)
#include <sys/socket.h>
  /*#include <sys/sockio.h>*/
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#else
#include <socket.h>
#include <if.h>
#include <inet.h>
#endif
#ifdef VXWORKS_55
#include <iosLib.h>
#endif

#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 46 /* strlen("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255") + 1 */
#endif

/* Values of these of no importance */
#ifndef IPPROTO_IPV6
#define IPPROTO_IPV6 1
#endif
#ifndef IPV6_UNICAST_HOPS
#define IPV6_UNICAST_HOPS 1
#endif
#ifndef IPV6_JOIN_GROUP
#define IPV6_JOIN_GROUP 1
#endif
#ifndef IPV6_MULTICAST_IF
#define IPV6_MULTICAST_IF 1
#endif
#ifndef IPV6_MULTICAST_LOOP
#define IPV6_MULTICAST_LOOP 1
#endif
#ifndef IN6_IS_ADDR_UNSPECIFIED
#define IN6_IS_ADDR_UNSPECIFIED
#endif
#ifndef IN6_IS_ADDR_MULTICAST
#define IN6_IS_ADDR_MULTICAST
#endif
#ifndef IN6_IS_ADDR_LOOPBACK
#define IN6_IS_ADDR_LOOPBACK
#endif

#if defined ( VXWORKS_55 ) || defined ( VXWORKS_54 )
typedef unsigned int		in_addr_t;
#endif

#if !defined ( VXWORKS_GTE_6 ) || defined (OSPL_VXWORKS653)
typedef unsigned int              socklen_t;
#if defined (OSPL_VXWORKS653)
#define OS_INET_NTOP os_inet_ntop
#define OS_INET_PTON os_inet_pton
#endif
#endif

/* Keep defines before common header */
#define OS_NO_NETLINK
#define OS_IFNAMESIZE           IFNAMSIZ
#define OS_SOCKET_HAS_SA_LEN 	1

#if defined ( VXWORKS_55 ) || defined (VXWORKS_54 ) || defined (OSPL_VXWORKS653)
#define OS_SOCKET_HAS_IPV6      0
#define OS_CUSTOM_HOSTBYNAME
#else
#define OS_SOCKET_HAS_IPV6      1
#endif

#include "../common/include/os_socket.h"

/* replace functions broken/missing on vxworks 5.5 */
#define OS_INET_NTOP os_inet_ntop
#define OS_INET_PTON os_inet_pton

extern char * os_inet_ntop (int af, const void *src, char *dst, socklen_t size);
extern int os_inet_pton(int af, const char *src, void *dst);

#ifdef VXWORKS_55
#undef os_sockEBADF 
#define os_sockEBADF        S_iosLib_INVALID_FILE_DESCRIPTOR
#endif

#if defined (__cplusplus)
}
#endif

#endif /* OS_VXWORKS_SOCKET_H */
