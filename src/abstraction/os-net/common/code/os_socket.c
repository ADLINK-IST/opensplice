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
#include "os_socket.h"
#include "os_stdlib.h"

#ifndef OS_NO_GETIFADDRS
#if defined (VXWORKS_RTP)
#include <net/ifaddrs.h>
#else
#include <ifaddrs.h>
#endif
#endif

#ifdef VXWORKS_RTP
#include <sockLib.h>
#include <ioLib.h>
#endif

os_sockErrno
os_sockError(void)
{
    return errno;
}

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

    if (bind(s, (struct sockaddr *)name, (os_uint)namelen) == -1) {
        result = os_resultFail;
    }
    return result;
}

os_int32
os_sockSendto(
    os_socket s,
    const void *msg,
    os_uint32 len,
    const struct sockaddr *to,
    os_uint32 tolen)
{
    return sendto(s, msg, (os_uint)len, 0, to, (os_uint)tolen);
}

os_int32
os_sockRecvfrom(
    os_socket s,
    void *buf,
    os_uint32 len,
    struct sockaddr *from,
    os_uint32 *fromlen)
{
   int res;
   socklen_t fl = *fromlen;;
   res = recvfrom(s, buf, (os_uint)len, 0, from, &fl);
   *fromlen=fl;
   return (os_int32)res;
}

os_result
os_sockGetsockopt(
    os_socket s,
    os_int32 level,
    os_int32 optname,
    void *optval,
    os_uint32 *optlen)
{
    int res;
    socklen_t ol = *optlen;
    res = getsockopt(s, level, optname, optval, &ol);
    *optlen = ol;
    return ( res == -1 ? os_resultFail : os_resultSuccess );
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
    os_time *timeout)
{
    struct timeval t;
    int r;

    t.tv_sec = (long)timeout->tv_sec;
    t.tv_usec = (long)(timeout->tv_nsec / 1000);
    r = select(nfds, readfds, writefds, errorfds, &t);

    return r;
}

/**
* Populates an os_ifAttributes_s struct for a particular ifreq interface instance.
* @param s A socket fd that is required for ioctl calls
* @param ifr The interface we want to query the attributes of. Its ifr_addr will be stored and
* the ifr_name used for the other ioctl calls.
* @param ifElement The struct to be populated with the interface's attributes
* @return os_resultSuccess on success, os_resultFail otherwise.
*/
static os_result
os_queryInterfaceAttributes(
    os_socket s,
    struct ifreq *ifr,
    os_ifAttributes *ifElement)
{
    int retVal;
    struct ifreq ifAttr;
    os_result result = os_resultSuccess;

    os_strncpy (ifElement->name, ifr->ifr_name, OS_IFNAMESIZE);

    memcpy(&ifElement->address, &ifr->ifr_addr,
            ifr->ifr_addr.sa_family == AF_INET6 ? sizeof(os_sockaddr_in6) : sizeof(os_sockaddr_in));
    if (ifr->ifr_addr.sa_family == AF_INET6)
    {
        /* Just zero these fileds for IPv6 */
        ifElement->flags = 0;
        memset (&ifElement->broadcast_address, 0, sizeof (ifElement->broadcast_address));
        memset (&ifElement->network_mask, 0, sizeof (&ifElement->network_mask));
    }
    else
    {
        ifAttr = *ifr;
#ifdef INTEGRITY
        if ( !strcmp("lo0", ifAttr.ifr_name) )
        {
           ifElement->flags = IFF_UP | IFF_LOOPBACK;
        }
        else
        {
           ifElement->flags = IFF_BROADCAST | IFF_MULTICAST | IFF_UP;
        }
#else
        retVal = ioctl (s, SIOCGIFFLAGS, &ifAttr);
        if (retVal == 0) {
            ifElement->flags = ifAttr.ifr_flags;
        } else {
            result = os_resultFail;
        }
#endif
        if (ifElement->flags & IFF_BROADCAST)
        {
            ifAttr = *ifr;
            retVal = ioctl (s, SIOCGIFBRDADDR, &ifAttr);
            if (retVal == 0) {
                memcpy(&ifElement->broadcast_address, &ifAttr.ifr_broadaddr, sizeof(os_sockaddr_in));
            } else {
                result = os_resultFail;
            }
        }
        else
        {
            memset (&ifElement->broadcast_address, 0, sizeof (ifElement->broadcast_address));
        }
        ifAttr = *ifr;
        retVal = ioctl (s, SIOCGIFNETMASK, &ifAttr);
        if (retVal == 0) {
            memcpy(&ifElement->network_mask, &ifAttr.ifr_addr, sizeof(os_sockaddr_in));
        } else {
            result = os_resultFail;
        }
    }
#if defined (OS_NO_SIOCGIFINDEX)
    /* Looks like Greenhills & AIX at least don't have SIOCGIFINDEX */
    /* @todo dds2523 To investigate - note only really required for IPv6 */
    ifElement->interfaceIndexNo = 0;
#elif defined (OS_SOLARIS) || defined (AIX)
    /* Solaris has if_nametoindex */
    ifElement->interfaceIndexNo = (os_uint) if_nametoindex((const char*)&ifr->ifr_name);
#else
    /* Get the interface index number */
    ifAttr = *ifr;
    retVal = ioctl (s, SIOCGIFINDEX, &ifAttr);
    if (retVal == 0)
    {
        ifElement->interfaceIndexNo = (os_uint) ifAttr.ifr_ifindex;
    } else {
        result = os_resultFail;
    }
#endif
    return result;
}

/**
* Use the SIOCGIFCONF ioctl query on a socket to list available network interfaces.
* Is used for IPv4 address querying & IPv6 queries on platforms
* where getifaddrs is not available
* @param addressFamily Determines the type of socket and hence which
* interfaces you wish to query. AF_INET for IPv4, AF_INET6 for IPv6
* @see os_sockQueryInterfaces
* @see os_sockQueryIPv6Interfaces
*/
os_result
os_sockQueryInterfacesBase(
    os_ifAttributes *ifList,
    os_uint32 listSize,
    os_uint32 *validElements,
    short addressFamily)
{
    os_result result = os_resultSuccess;
    struct ifconf ifc;
    struct ifreq *ifr;
    int bufLen = 1000;
    os_socket ifcs;
    unsigned int listIndex;
    unsigned int ifrLen;

    ifcs = os_sockNew (addressFamily, SOCK_DGRAM);
    if (ifcs >= -1) {
        ifc.ifc_len = bufLen;
        ifc.ifc_buf = os_malloc (ifc.ifc_len);
        while (ifc.ifc_len == bufLen) {
            memset(ifc.ifc_buf, 0, bufLen);
            ioctl (ifcs, SIOCGIFCONF, &ifc);
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
                        ifrLen += sizeof(os_sockaddr_in6);
                    } else {
                        ifrLen += (unsigned int)sizeof(struct sockaddr);
                    }
#endif
                   /*
                    * For some platforms (e.g. 64 bit), the sockaddr members may not be the longest members
                    * of the Union. In that case the "sizeof"-size should be used.
                    */
                   if (ifrLen < sizeof(struct ifreq)) {
                      ifrLen = sizeof(struct ifreq);
                   }

                   if (ifr->ifr_addr.sa_family == addressFamily) {
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

os_result
os_sockQueryInterfaces(
    os_ifAttributes *ifList,
    os_uint32 listSize,
    os_uint32 *validElements)
{
    return os_sockQueryInterfacesBase(ifList, listSize, validElements, AF_INET);
}

os_result
os_sockQueryIPv6Interfaces(
    os_ifAttributes *ifList,
    os_uint32 listSize,
    os_uint32 *validElements)
{
#ifdef OS_NO_GETIFADDRS
    /* If getifaddrs isn't available fall back to ioctl. Might work... */
    return os_sockQueryInterfacesBase(ifList, listSize, validElements, AF_INET6);
#else
    /* SIOCGIFCONF doesn't list Ipv6 interfaces on Linux. getifaddrs is preferable
    if available anyway */
    struct ifaddrs* interfaceList = NULL;
    struct ifaddrs* nextInterface = NULL;
    unsigned int listIndex = 0;

    *validElements = 0;

    if (getifaddrs (&interfaceList) != 0)
    {
        return os_resultFail;
    }

    nextInterface = interfaceList;

    while (nextInterface != NULL && listIndex < listSize)
    {
        if (nextInterface->ifa_addr &&
            nextInterface->ifa_addr->sa_family == AF_INET6)
        {
            os_sockaddr_in6* v6Address;

            v6Address = (os_sockaddr_in6 *) nextInterface->ifa_addr;

            if (!IN6_IS_ADDR_UNSPECIFIED(&v6Address->sin6_addr))
            {
                os_strncpy(ifList[listIndex].name, nextInterface->ifa_name, OS_IFNAMESIZE);
                memcpy(&ifList[listIndex].address, v6Address, sizeof (os_sockaddr_in6));
                ifList[listIndex].flags = 0;
                memset(&ifList[listIndex].broadcast_address, 0, sizeof (ifList[listIndex].broadcast_address));
                memset(&ifList[listIndex].network_mask, 0, sizeof (&ifList[listIndex].network_mask));
                ifList[listIndex].interfaceIndexNo = (os_uint) if_nametoindex(nextInterface->ifa_name);
                ++listIndex;
            }
        }
        nextInterface = nextInterface->ifa_next;
    }

    *validElements = listIndex;
    freeifaddrs(interfaceList);
    return os_resultSuccess;
#endif /*OS_NO_GETIFADDRS */
}
