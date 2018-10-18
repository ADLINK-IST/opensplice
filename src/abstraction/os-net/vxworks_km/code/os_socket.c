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
/****************************************************************
 * Implementation for time management conforming to             *
 * OpenSplice requirements                                      *
 ****************************************************************/

/** \file
 *  \brief socket management
 */

#include "os_heap.h"
#include <string.h>
#include "os_errno.h"

/* include OS specific socket management implementation		*/
#include "os_socket.h"
#include <selectLib.h>
#if defined ( VXWORKS_55 ) || defined ( VXWORKS_54 )
#include <resolvLib.h>
#endif

#include "../common/code/os_socket_nb.c"

os_socket
os_sockNew(
    int domain,
    int type)
{
    return socket (domain, type, 0);
}

os_result
os_sockBind(
    os_socket s,
    const struct sockaddr *name,
    os_uint32 namelen)
{
    os_result result = os_resultSuccess;

    if (bind(s, (struct sockaddr *)name, namelen) == -1) {
        result = os_resultFail;
    }
    return result;
}

os_result
os_sockGetsockname(
    os_socket s,
    const struct sockaddr *name,
    os_uint32 namelen)
{
    os_result result = os_resultSuccess;
    int len = namelen;

    if (getsockname(s, (struct sockaddr *)name, &len) == -1) {
        result = os_resultFail;
    }
    return result;
}

os_result
os_sockSendto(
    os_socket s,
    const void *msg,
    size_t len,
    const struct sockaddr *to,
    size_t tolen,
    size_t *bytesSent)
{
    ssize_t res = sendto(s, msg, len, 0, to, tolen);
    if (res < 0)
    {
        *bytesSent = 0;
        return os_resultFail;
    }
    else
    {
        *bytesSent = res;
        return os_resultSuccess;
    }
}

os_result
os_sockRecvfrom(
    os_socket s,
    void *buf,
    size_t len,
    struct sockaddr *from,
    size_t *fromlen,
    size_t *bytesRead)
{
    ssize_t res = recvfrom(s, buf, len, 0, from, fromlen);
    if (res < 0)
    {
        *bytesRead = 0;
        return os_resultFail;
    }
    else
    {
        *bytesRead = (size_t)res;
        return os_resultSuccess;
    }
}

os_result
os_sockGetsockopt(
    os_socket s,
    os_int32 level,
    os_int32 optname,
    void *optval,
    os_uint32 *optlen)
{
    os_result result = os_resultSuccess;

    if (getsockopt(s, level, optname, optval, optlen) == -1) {
        result = os_resultFail;
    }
    return result;
}

os_result
os_sockSetsockopt(
    os_socket s,
    os_int32 level,
    os_int32 optname,
    const void *optval,
    os_uint32 optlen)
{
    os_result result = os_resultSuccess;

    if (setsockopt(s, level, optname, optval, optlen) == -1) {
        result = os_resultFail;
    }
    return result;
}

os_result
os_sockFree(
    os_socket s)
{
    os_result result = os_resultSuccess;

    if (close(s) == -1) {
        result = os_resultFail;
    }
    return result;
}

os_int32
os_sockSelect(
    os_int32 nfds,
    fd_set *readfds,
    fd_set *writefds,
    fd_set *errorfds,
    os_duration timeout)
{
    struct timeval t;
    int r;

    assert(OS_DURATION_ISPOSITIVE(timeout));

    t.tv_sec = (long)OS_DURATION_GET_SECONDS(timeout);
    t.tv_usec = (long)(OS_DURATION_GET_NANOSECONDS(timeout) / 1000);
    r = select(nfds, readfds, writefds, errorfds, &t);

    return r;
}

static os_result
os_queryInterfaceAttributes(
    os_socket s,
    struct ifreq *ifr,
    os_ifAttributes *ifElement)
{
    os_result result = os_resultSuccess;
#if ! defined (OSPL_VXWORKS653)
    int retVal;
    struct ifreq ifAttr;

    os_strncpy (ifElement->name, ifr->ifr_name, OS_IFNAMESIZE);
    ifAttr = *ifr;
    memcpy(&ifElement->address, &ifAttr.ifr_addr, sizeof(ifAttr.ifr_addr));
    retVal = ioctl (s, SIOCGIFFLAGS, &ifAttr);
    if (retVal != -1) {
        ifElement->flags = ifAttr.ifr_flags;
    } else {
        result = os_resultFail;
    }
    if (ifElement->flags & IFF_BROADCAST) {
        ifAttr = *ifr;
        retVal = ioctl (s, SIOCGIFBRDADDR, &ifAttr);
        if (retVal != -1) {
            memcpy(&ifElement->broadcast_address, &ifAttr.ifr_broadaddr, sizeof(ifAttr.ifr_broadaddr));
        } else {
            result = os_resultFail;
        }
    } else {
        memset (&ifElement->broadcast_address, 0, sizeof (ifElement->broadcast_address));
    }
    ifAttr = *ifr;
    retVal = ioctl (s, SIOCGIFNETMASK, &ifAttr);
    if (retVal != -1) {
        memcpy(&ifElement->network_mask, &ifAttr.ifr_addr, sizeof(ifAttr.ifr_addr));
    } else {
        result = os_resultFail;
    }
#endif
    return result;

}

#if ! defined (OSPL_VXWORKS653)
os_result
os_sockQueryInterfaces(
    os_ifAttributes *ifList,
    os_uint32 listSize,
    os_uint32 *validElements)
{
    os_result result = os_resultSuccess;
    struct ifconf ifc;
    struct ifreq *ifr;
    int bufLen = 1000;
    os_socket ifcs;
    int retVal;
    unsigned int listIndex;
    unsigned int ifrLen;

    ifcs = os_sockNew (AF_INET, SOCK_DGRAM);
    if (ifcs >= -1) {
        ifc.ifc_len = bufLen;
        ifc.ifc_buf = os_malloc (ifc.ifc_len);
        while (ifc.ifc_len == bufLen) {
            memset(ifc.ifc_buf, 0, bufLen);
            retVal = ioctl (ifcs, SIOCGIFCONF, &ifc);
            if (ifc.ifc_len < bufLen) {
                listIndex = 0;
                ifr = (struct ifreq *)ifc.ifc_buf;
                /* returned smaller than provided */
                while ((listIndex < listSize) &&
                      ((char *)ifr < ((char *)ifc.ifc_buf + ifc.ifc_len)) &&
                      (result == os_resultSuccess)) {
                    ifrLen = (unsigned int)sizeof(ifr->ifr_name);
#if (OS_SOCKET_HAS_SA_LEN == 1)
                    if (sizeof(struct sockaddr) > ifr->ifr_addr.sa_len) {
                        ifrLen += (unsigned int)sizeof(struct sockaddr);
                    } else {
                        ifrLen += ifr->ifr_addr.sa_len;
                    }
#else
                    if (ifr->ifr_addr.sa_family == AF_INET) {
                        ifrLen += sizeof(struct sockaddr_in);
                    } else if (ifr->ifr_addr.sa_family == AF_INET6) {
                        ifrLen += sizeof(struct sockaddr_in6);
                    } else {
                        ifrLen += (unsigned int)sizeof(struct sockaddr);
                    }
#endif
                    if (ifr->ifr_addr.sa_family == AF_INET) {
                        /* Get other interface attributes */
                        result = os_queryInterfaceAttributes (ifcs, ifr,
                            &ifList[listIndex]);
                        if (result == os_resultSuccess) {
                            listIndex++;
                        }
                    }

                    ifr = (struct ifreq *)((char *)ifr + ifrLen);
                }
                if (result == os_resultSuccess) {
                    *validElements = listIndex;
                }
            } else {
                os_free (ifc.ifc_buf);
                bufLen += 1000;
                ifc.ifc_len = bufLen;
                ifc.ifc_buf = os_malloc (ifc.ifc_len);
            }
        }
        os_free (ifc.ifc_buf);
        os_sockFree (ifcs);
    }
    return result;
}
#else
extern const os_if_struct ospl_if_info;

os_result
os_sockQueryInterfaces(
    os_ifAttributes *ifList,
    os_uint32 listSize,
    os_uint32 *validElements)
{
    strncpy ((&ifList[0])->name, ospl_if_info.name, OS_IFNAMESIZE);
    inet_aton(ospl_if_info.address, &((struct sockaddr_in*)(&(&ifList[0])->address))->sin_addr);
    inet_aton(ospl_if_info.netmask, &((struct sockaddr_in*)(&(&ifList[0])->network_mask))->sin_addr);
    inet_aton(ospl_if_info.broadcast, &((struct sockaddr_in*)(&(&ifList[0])->broadcast_address))->sin_addr);
    ((struct sockaddr_in*)(&(&ifList[0])->broadcast_address))->sin_family = AF_INET;
    ((struct sockaddr_in*)(&(&ifList[0])->network_mask))->sin_family = AF_INET;
    ((struct sockaddr_in*)(&(&ifList[0])->address))->sin_family = AF_INET;
    (&ifList[0])->flags = ospl_if_info.flags;

    *validElements = 1;
    return os_resultSuccess;
}
#endif

os_result
os_sockQueryIPv6Interfaces(
    os_ifAttributes *ifList,
    os_uint32 listSize,
    os_uint32 *validElements)
{
    os_result result = os_resultFail;
    *validElements = 0;
    OS_REPORT(OS_ERROR, "os_sockQueryIPv6Interfaces", 0, "This platform does not support IPv6");
    return result;
}

void
os_sockQueryInterfaceStatusDeinit(
    void *handle)
{
    OS_UNUSED_ARG(handle);
}

void *
os_sockQueryInterfaceStatusInit(
    const char *ifName)
{
    OS_UNUSED_ARG(ifName);
    return NULL;
}

os_result
os_sockQueryInterfaceStatus(
    void *handle,
    os_duration timeout,
    os_boolean *status)
{
    OS_UNUSED_ARG(handle);
    OS_UNUSED_ARG(timeout);
    *status = OS_FALSE;

    return os_resultFail;
}

os_result
os_sockQueryInterfaceStatusSignal(void *handle)
{
    OS_UNUSED_ARG(handle);
    return os_resultFail;
}

#include "../vxworks/code/os_inet.c"
