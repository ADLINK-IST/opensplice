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

#ifndef OS_LINUX_SOCKET_H
#define OS_LINUX_SOCKET_H

#if defined (__cplusplus)
extern "C" {
#endif

#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet6/in6.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <unistd.h>
#include <sockLib.h>

/* Keep defines before common header */
/* Best I can tell there's no getifaddrs for RTPs in this version */
#define OS_NO_GETIFADDRS
#define OS_NO_SIOCGLIFCONF
#define OS_NO_NETLINK
#define OS_IFNAMESIZE		IF_NAMESIZE
#define OS_SOCKET_HAS_SA_LEN	1
#define OS_SOCKET_HAS_IPV6      1
#define DO_HOST_BY_NAME
#define OS_SOCKET_HAS_FREEBSD_STACK 1

/* replace functions broken on vxworks 6.6 */
#define OS_INET_NTOP os_inet_ntop
#define OS_INET_PTON os_inet_pton
extern char *os_inet_ntop (int af, const void *src, char *dst, socklen_t size);
extern int os_inet_pton(int af, const char *src, void *dst);

#include "../common/include/os_socket.h"

#if defined (__cplusplus)
}
#endif

#endif /* OS_LINUX_SOCKET_H */
