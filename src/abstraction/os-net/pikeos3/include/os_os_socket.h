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

#ifndef OS_PIKEOS_SOCKET_H
#define OS_PIKEOS_SOCKET_H

#if defined (__cplusplus)
extern "C" {
#endif

#include <lwip_config.h>
#include <lwip/sockets.h>
#include <lwip/inet.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <unistd.h>

struct sockaddr_storage {
  u8_t ss_len;
  u8_t ss_family;
  char ss_data[14];
};

#include "../common/include/os_socket.h"

#define OS_IFNAMESIZE LWIP_IFNAME_MAX
#define OS_SOCKET_HAS_SA_LEN	1
#define OS_SOCKET_HAS_IPV6      0
#define OS_NO_GETIFADDRS
#define OS_NO_SIOCGLIFCONF
#define OS_NO_NETLINK
#undef DO_HOST_BY_NAME
#define AF_INET6 10

#define IFF_UP          0x1
#define IFF_BROADCAST   0x2
#define IFF_LOOPBACK    0x8
#define IFF_MULTICAST   0x1000

char *inet_ntop(int af, const void *src, char *dst, socklen_t size);


#if defined (__cplusplus)
}
#endif

#endif /* OS_PIKEOS_SOCKET_H */
