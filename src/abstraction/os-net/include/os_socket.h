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

#ifndef OS_SOCKET_H
#define OS_SOCKET_H

#include "os_defs.h"
#include "os_time.h"

#if defined (__cplusplus)
extern "C" {
#endif

/* Include OS specific header file              */
#include "include/os_socket.h"
#include "os_if.h"

#ifdef OSPL_BUILD_OSNET
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
* @file
* @addtogroup OS_NET
* @{
*/

/**
* Socket handle type. SOCKET on windows, int otherwise.
*/
#ifdef WIN32
typedef SOCKET os_socket; /* unsigned int */
#else
typedef int os_socket; /* signed */
#endif

/* Indirecting all the socket types. Some IPv6 & protocol agnostic
stuff seems to be not always be available */

typedef struct sockaddr_in os_sockaddr_in;
typedef struct sockaddr os_sockaddr;
/* @todo dds2800 - Recheck macro name when fixed */
#ifndef _VXWORKS /* Originals do not exist or have wrong defs on 'Old' VxWorks */
typedef struct ipv6_mreq os_ipv6_mreq;
typedef struct in6_addr os_in6_addr;
typedef struct sockaddr_storage os_sockaddr_storage;
typedef struct sockaddr_in6 os_sockaddr_in6;
#endif
OS_API extern const os_in6_addr os_in6addr_any;

#define SD_FLAG_IS_SET(flags, flag) ((((os_uint32)(flags) & (os_uint32)(flag))) != 0U)

/**
* Structure to hold a network interface's attributes
*/
typedef struct os_ifAttributes_s {
    /**
    * The network interface name (or at least OS_IFNAMESIZE - 1 charcaters thereof)
    */
    char name[OS_IFNAMESIZE];
    /**
    * Iff the interface is IPv4 holds the ioctl query result flags for this interface.
    */
    os_uint32 flags;
    /**
    * The network interface address of this interface.
    */
    os_sockaddr_storage address;
    /**
    * Iff this is an IPv4 interface, holds the broadcast address for the sub-network this
    * interface is connected to.
    */
    os_sockaddr_storage broadcast_address;
    /**
    * Iff this is an IPv4 interface, holds the subnet mast for this interface
    */
    os_sockaddr_storage network_mask;
    /**
    * Holds the interface index for this interface.
    */
    os_uint interfaceIndexNo;
} os_ifAttributes;

OS_API os_sockErrno
os_sockError(void);

/**
* Fill-in a pre-allocated list of os_ifAttributes_s with details of
* the available IPv4 network interfaces.
* @param listSize Number of elements there is space for in the list.
* @param ifList Pointer to head of list
* @param validElements Out param to hold the number of interfaces found
* whose detauils have been returned.
* @return os_resultSuccess if 0 or more interfaces were found, os_resultFail if
* an error occured.
* @see os_sockQueryIPv6Interfaces
*/
OS_API os_result
os_sockQueryInterfaces(
    os_ifAttributes *ifList,
    os_uint32 listSize,
    os_uint32 *validElements);

/**
* Fill-in a pre-allocated list of os_ifAttributes_s with details of
* the available IPv6 network interfaces.
* @param listSize Number of elements there is space for in the list.
* @param ifList Pointer to head of list
* @param validElements Out param to hold the number of interfaces found
* whose detauils have been returned.
* @return os_resultSuccess if 0 or more interfaces were found, os_resultFail if
* an error occured.
* @see os_sockQueryInterfaces
*/
OS_API os_result
os_sockQueryIPv6Interfaces(
    os_ifAttributes *ifList,
    os_uint32 listSize,
    os_uint32 *validElements);

OS_API os_socket
os_sockNew(
    int domain, /* AF_INET */
    int type    /* SOCK_DGRAM */);

OS_API os_result
os_sockBind(
    os_socket s,
    const struct sockaddr *name,
    os_uint32 namelen);

OS_API os_int32
os_sockSendto(
    os_socket s,
    const void *msg,
    os_uint32 len,
    const struct sockaddr *to,
    os_uint32 tolen);

OS_API os_int32
os_sockRecvfrom(
    os_socket s,
    void *buf,
    os_uint32 len,
    struct sockaddr *from,
    os_uint32 *fromlen);

OS_API os_result
os_sockGetsockopt(
    os_socket s,
    os_int32 level, /* SOL_SOCKET */
    os_int32 optname, /* SO_REUSEADDR, SO_DONTROUTE, SO_BROADCAST, SO_SNDBUF, SO_RCVBUF */
    void *optval,
    os_uint32 *optlen);

OS_API os_result
os_sockSetsockopt(
    os_socket s,
    os_int32 level, /* SOL_SOCKET */
    os_int32 optname, /* SO_REUSEADDR, SO_DONTROUTE, SO_BROADCAST, SO_SNDBUF, SO_RCVBUF */
    const void *optval,
    os_uint32 optlen);

OS_API os_result
os_sockFree(
    os_socket s);

OS_API os_int32
os_sockSelect(
    os_int32 nfds,
    fd_set *readfds,
    fd_set *writefds,
    fd_set *errorfds,
    os_time *timeout);

/* docced in implementation file */
OS_API os_boolean
os_sockaddrIPAddressEqual(const os_sockaddr* this_sock,
                           const os_sockaddr* that_sock);

/**
* Convert a socket address to a string format presentation representation
* @param sa The socket address struct.
* @param buffer A character buffer to hold the string rep of the address.
* @param buflen The (max) size of the buffer
* @return Pointer to start of string
*/
OS_API char*
os_sockaddrAddressToString(const os_sockaddr* sa,
                            char* buffer, size_t buflen);

/* docced in implementation file */
OS_API os_boolean
os_sockaddrStringToAddress(const char *addressString,
                           os_sockaddr* addressOut,
                           os_boolean isIPv4);

/* docced in implementation file */
OS_API os_boolean
os_sockaddrIsLoopback(const os_sockaddr* thisSock);

/* docced in implementation file */
OS_API char*
os_sockErrnoToString(os_sockErrno errNo);

/**
* @}
*/

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* OS_SOCKET_H */


