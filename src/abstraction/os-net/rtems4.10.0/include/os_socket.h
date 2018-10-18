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

#ifndef OS_RTEMS_SOCKET_H
#define OS_RTEMS_SOCKET_H

#if defined (__cplusplus)
extern "C" {
#endif

#include <sys/socket.h>
#include <net/if.h>

#ifndef __USE_GNU
/* set the flag and mind to undef the flag later */
#define __USE_GNU
#define UNDEF_USE_GNU
#endif

#include <netinet/in.h>

/* only undef __USE_GNU if explicitly set before */
#ifdef UNDEF_USE_GNU
#undef __USE_GNU
#undef UNDEF_USE_GNU
#endif

#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/uio.h>
#include <unistd.h>

/* Keep defines before common header */
#define OS_NO_NETLINK
#define OS_IFNAMESIZE		IF_NAMESIZE
#define OS_SOCKET_HAS_SA_LEN	1
#define OS_SOCKET_HAS_FREEBSD_STACK 1
#define OS_SOCKET_HAS_IPV6      0
#define OS_NO_SIOCGIFINDEX      1

/* As rtems has no IPV6 but our code is riddled with it 
   we have to mimic up alot of stuff */
#define IPV6_MULTICAST_HOPS 64
#define IPV6_UNICAST_HOPS 64
#define IPV6_JOIN_GROUP 20
#define IPV6_MULTICAST_IF 17
#define IPV6_MULTICAST_LOOP 19

/* override the socket receive buffer size for this platform */
#define NWCF_DEF_ReceiveBufferSize    (65536U)

#include "../common/include/os_socket.h"

struct sockaddr_storage
{
    unsigned char sa_len;
    sa_family_t ss_family;
    os_uint32 ss_align;
    char ss_padding[120];
};


#if defined (__cplusplus)
}
#endif

#endif /* OS_RTEMS_SOCKET_H */
