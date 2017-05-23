/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
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
#if OS_SOLARIS_VER == 8
/* override the socket receive buffer size for this platform */
#define NWCF_DEF_ReceiveBufferSize    (262144U)
#endif
#include "../common/include/os_socket.h"

#ifndef INADDR_NONE
#if OS_SOLARIS_VER == 8
#define INADDR_NONE ((unsigned long) -1)
#else
#define INADDR_NONE             (in_addr_t)(-1)
#endif
#endif
#if defined (__cplusplus)
}
#endif

#endif /* OS_SOLARIS_SOCKET_H */
