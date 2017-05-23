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
#ifndef OS_WIN32_SOCKET_H
#define OS_WIN32_SOCKET_H

#if defined (__cplusplus)
extern "C" {
#endif

/* to prevent a warning undef _WINSOCKAPI_ if defined */
#ifdef _WINSOCKAPI_
#undef _WINSOCKAPI_
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <ws2tcpip.h>

/* Expected standard definitions */
/*
    AF_INET

    SOCK_DGRAM

    IFF_UP
    IFF_BROADCAST
    IFF_LOOPBACK
    IFF_POINTTOPOINT
    IFF_MULTICAST

    struct sockaddr
    struct sockaddr_in
*/

#define IFF_POINTOPOINT     IFF_POINTTOPOINT

  /* temporary typedef until networking is using abstraction layer */
typedef unsigned long in_addr_t;

/* Windows specific usage of the os_socket API: os_sockBind() should be called
 * before multicast options are set with os_sockSetsockopt() */
#define OS_SOCKET_BIND_FOR_MULTICAST 1

#define OS_IFNAMESIZE           128
#define OS_SOCKET_HAS_SA_LEN    0
#define OS_SOCKET_HAS_IPV6      1
#define DO_HOST_BY_NAME

/* List of socket error numbers */
#define os_sockEAGAIN       WSAEWOULDBLOCK /* Operation would block, or a timeout expired before operation succeeded */
#define os_sockEWOULDBLOCK  WSAEWOULDBLOCK /* Operation would block */
#define os_sockENOMEM       WSABASEERR
#define os_sockENOSR        WSABASEERR
#define os_sockENOENT       WSABASEERR
#define os_sockEPERM        WSABASEERR
#define os_sockEINTR        WSAEINTR
#define os_sockEBADF        WSAEBADF
#define os_sockEACCES       WSAEACCES
#define os_sockEINVAL       WSAEINVAL
#define os_sockEMFILE       WSAEMFILE
#define os_sockENOTSOCK     WSAENOTSOCK
#define os_sockEMSGSIZE     WSAEMSGSIZE
#define os_sockENOPROTOOPT  WSAENOPROTOOPT
#define os_sockEPROTONOSUPPORT  WSAEPROTONOSUPPORT
#define os_sockEADDRINUSE   WSAEADDRINUSE
#define os_sockEADDRNOTAVAIL    WSAEADDRNOTAVAIL
#define os_sockEHOSTUNREACH WSAEHOSTUNREACH
#define os_sockENOBUFS      WSAENOBUFS
#define os_sockECONNRESET   WSAECONNRESET   /* Connection reset by peer */

#if defined NTDDI_VERSION && defined _WIN32_WINNT_WS03 && NTDDI_VERSION >= _WIN32_WINNT_WS03
#define OS_SOCKET_HAS_SSM 1
#else
#define OS_SOCKET_HAS_SSM 0
#endif

#if defined (__cplusplus)
}
#endif

#endif /* OS_WIN32_SOCKET_H */
