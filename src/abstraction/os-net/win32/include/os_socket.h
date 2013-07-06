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

#if defined (__cplusplus)
}
#endif

#endif /* OS_WIN32_SOCKET_H */
