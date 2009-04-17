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
#ifndef OS_SOCKET_H
#define OS_SOCKET_H

#include <os_defs.h>
#include <os_time.h>

#if defined (__cplusplus)
extern "C" {
#endif

/* Include OS specific header file              */
#include <include/os_socket.h>
#include <os_if.h>

#ifdef OSPL_BUILD_OSNET
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

typedef struct os_ifAttributes_s {
    char name[OS_IFNAMESIZE];
    os_uint32 flags;
    struct sockaddr address;
    struct sockaddr broadcast_address;
    struct sockaddr network_mask;
} os_ifAttributes;

OS_API os_sockErrno
os_sockError(void);

OS_API os_result
os_sockQueryInterfaces(
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

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* OS_SOCKET_H */
