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

#if !defined(OS_NO_NETLINK) && defined(__gnu_linux__)
#include <asm/types.h>
#include <netinet/in.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#endif

int
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

os_result
os_sockGetsockname(
    os_socket s,
    const struct sockaddr *name,
    os_uint32 namelen)
{
    os_result result = os_resultSuccess;
    socklen_t len = namelen;


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
   ssize_t res;
   socklen_t fl = *fromlen;
   res = recvfrom(s, buf, len, 0, from, &fl);
   if (res < 0)
   {
      *bytesRead = 0;
      return os_resultFail;
   }
   else
   {
      *fromlen=fl;
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
#ifdef OS_NEEDS_SO_REUSEPORT
    if (result == os_resultSuccess && level == SOL_SOCKET && optname == SO_REUSEADDR)
    {
       if (setsockopt(s, level, SO_REUSEPORT, optval, optlen) == -1)
       {
          result = os_resultFail;
       }
    }
#endif
    return result;
}

#include "os_socket_nb.c"

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

#if defined (VXWORKS_RTP)
static os_uint
os_nameToIndex (
    const char *ifname,
    short addressFamily)
{
    os_uint ifno = 0;
    os_socket sk;

    sk = os_sockNew(addressFamily, SOCK_DGRAM);
    if (sk >= 0) {
        struct ifreq ifr;

        memset(&ifr, 0, sizeof(ifr));
        strcpy(ifr.ifr_name, ifname);
        if (ioctl(sk, SIOCGIFINDEX, &ifr) == 0) {
            ifno = ifr.ifr_ifindex;
        }
        os_sockFree(sk);
    }

    return ifno;
}
#endif

#ifndef OS_NO_SIOCGLIFCONF

/* Interface list parsing code using the 'L' form ioctls */

static short int
os_getInterfaceFlagsL(
    os_socket s,
    const os_char *ifName,
    short addressFamily)
{
    struct lifreq lifr;
    short int flags = 0;

    memset(&lifr, 0, sizeof(lifr));
    os_strncpy(lifr.lifr_name, ifName, OS_LIFNAMESIZE);

    if (ioctl (s, SIOCGLIFFLAGS, &lifr) != -1) {
        flags = lifr.lifr_flags;
    }

    return flags;
}

/**
* Populates an os_ifAttributes_s struct for a particular lifreq interface instance.
* @param s A socket fd that is required for ioctl calls
* @param ifr The interface we want to query the attributes of. Its ifr_addr
* will be stored and the ifr_name used for the other ioctl calls.
* @param ifElement The struct to be populated with the interface's attributes
* @return os_resultSuccess on success, os_resultFail otherwise.
*/
static os_result
os_queryInterfaceAttributesL(
    os_socket s,
    struct lifreq *lifr,
    os_ifAttributes *ifElement)
{
    int retVal;
    struct lifreq lifAttr;
    os_result result = os_resultSuccess;

    os_strncpy (ifElement->name, lifr->lifr_name, OS_LIFNAMESIZE);

    if (lifr->lifr_addr.ss_family == AF_INET6)
    {
#if (OS_SOCKET_HAS_IPV6 == 1)
        memcpy(&ifElement->address, &lifr->lifr_addr, sizeof (struct sockaddr_storage));
        /* Just zero these fields for IPv6 */
        memset (&ifElement->broadcast_address, 0, sizeof (ifElement->broadcast_address));
        memset (&ifElement->network_mask, 0, sizeof (&ifElement->network_mask));
        ifElement->flags = os_getInterfaceFlagsL(s, lifr->lifr_name, AF_INET6);
#else
        result = os_resultFail;
#endif
    }
    else
    {
        memcpy(&ifElement->address, &lifr->lifr_addr, lifr->lifr_addrlen);
        lifAttr = *lifr;
        retVal = ioctl (s, SIOCGLIFFLAGS, &lifAttr);
        if (retVal != -1) {
            ifElement->flags = lifAttr.lifr_flags;
        } else {
            result = os_resultFail;
        }
        if (ifElement->flags & IFF_BROADCAST)
        {
            lifAttr = *lifr;
            retVal = ioctl (s, SIOCGLIFBRDADDR, &lifAttr);
            if (retVal != -1) {
                memcpy(&ifElement->broadcast_address, &lifAttr.lifr_broadaddr, sizeof(os_sockaddr_storage));
            } else {
                result = os_resultFail;
            }
        }
        else
        {
            memset (&ifElement->broadcast_address, 0, sizeof (ifElement->broadcast_address));
        }
        lifAttr = *lifr;
        retVal = ioctl (s, SIOCGLIFNETMASK, &lifAttr);
        if (retVal != -1) {
            memcpy(&ifElement->network_mask, &lifAttr.lifr_addr, sizeof(os_sockaddr_storage));
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
    ifElement->interfaceIndexNo = (os_uint) if_nametoindex((const char*)&lifr->lifr_name);
#else
    /* Get the interface index number */
    lifAttr = *lifr;
    retVal = ioctl (s, SIOCGLIFINDEX, &lifAttr);
    if (retVal  != -1)
    {
        ifElement->interfaceIndexNo = (os_uint) lifAttr.lifr_ifindex;
    } else {
        result = os_resultFail;
    }
#endif
    return result;
}


/**
* Use the SIOCGLIFCONF ioctl query on a socket to list available network
* interfaces.
* Is used for IPv6 queries on platforms where getifaddrs is not available
* @param addressFamily Determines the type of socket and hence which
* interfaces you wish to query. AF_INET for IPv4, AF_INET6 for IPv6
* @see os_sockQueryInterfaces
* @see os_sockQueryIPv6Interfaces
*/
static os_result
os_sockQueryInterfacesBaseL(
    os_ifAttributes *ifList,
    os_uint32 listSize,
    os_uint32 *validElements,
    short addressFamily)
{
    os_result result = os_resultSuccess;
    struct lifconf ifc;
    struct lifnum ifn;
    struct lifreq *ifr;
    os_socket ifcs;
    unsigned int listIndex;

    ifcs = os_sockNew (addressFamily, SOCK_DGRAM);
    if (ifcs >= -1) {
        ifn.lifn_family = addressFamily;
        ifn.lifn_flags = 0;
        ioctl (ifcs, SIOCGLIFNUM, &ifn);
        ifc.lifc_len = sizeof (struct lifreq) * ifn.lifn_count;
        ifc.lifc_buf = os_malloc (ifc.lifc_len);
        ifc.lifc_family = addressFamily;
        ifc.lifc_flags = 0;
        ioctl (ifcs, SIOCGLIFCONF, &ifc);
        ifr = ifc.lifc_req;
        listIndex = 0;
        while (
            (char *)ifr < ((char *)ifc.lifc_req) + ifc.lifc_len &&
            listIndex < (unsigned int)ifn.lifn_count &&
            listIndex < listSize) {
            result = os_queryInterfaceAttributesL (ifcs, ifr, &ifList[listIndex]);
            ifr += 1;
            if (result == os_resultSuccess) {
                listIndex += 1;
            }
        }
        *validElements = listIndex;
        os_free (ifc.lifc_buf);
        os_sockFree (ifcs);
    }
    return result;
}

#endif

static short int
os_getInterfaceFlags(
    os_socket s,
    const os_char *ifName,
    short addressFamily)
{
    struct ifreq ifr;
    short int flags = 0;

    memset(&ifr, 0, sizeof(ifr));
    os_strncpy(ifr.ifr_name, ifName, OS_IFNAMESIZE);

    if (ioctl (s, SIOCGIFFLAGS, &ifr) != -1) {
        flags = ifr.ifr_flags;
    }

    if (flags == 0) {
        if (!os_strncasecmp("lo0", ifr.ifr_name, OS_IFNAMESIZE)) {
            flags = IFF_UP | IFF_LOOPBACK;
        } else {
            if (addressFamily == AF_INET6) {
                flags = IFF_MULTICAST | IFF_UP;
            } else {
                flags = IFF_BROADCAST | IFF_MULTICAST | IFF_UP;
            }
        }
    }

    return flags;
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

    if (ifr->ifr_addr.sa_family == AF_INET6)
    {
#if (OS_SOCKET_HAS_IPV6 == 1)
        memcpy(&ifElement->address, &ifr->ifr_addr, sizeof(os_sockaddr_in6));
        /* Just zero these fileds for IPv6 */
        memset (&ifElement->broadcast_address, 0, sizeof (ifElement->broadcast_address));
        memset (&ifElement->network_mask, 0, sizeof (&ifElement->network_mask));
        ifElement->flags = os_getInterfaceFlags(s, ifr->ifr_name, AF_INET6);
#else
        result = os_resultFail;
#endif
    }
    else
    {
        memcpy(&ifElement->address, &ifr->ifr_addr, sizeof(os_sockaddr_in));
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
        if (retVal != -1) {
            ifElement->flags = ifAttr.ifr_flags;
        } else {
            result = os_resultFail;
        }
#endif
        if (ifElement->flags & IFF_BROADCAST)
        {
            ifAttr = *ifr;
            retVal = ioctl (s, SIOCGIFBRDADDR, &ifAttr);
            if (retVal != -1) {
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
        if (retVal != -1) {
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
    if (retVal  != -1)
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
static os_result
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
#if (OS_SOCKET_HAS_FREEBSD_STACK == 1)
                    ifrLen = sizeof(struct ifreq);
                    if (ifr->ifr_addr.sa_len)
                    {
                       ifrLen += ifr->ifr_addr.sa_len - sizeof(struct sockaddr);
                    }
#else
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
#if (OS_SOCKET_HAS_IPV6 == 1)
                    } else if (ifr->ifr_addr.sa_family == AF_INET6) {
                        ifrLen += sizeof(os_sockaddr_in6);

#endif
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
#endif

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
#ifdef OS_NO_GETIFADDRS
    /* If getifaddrs isn't available fall back to ioctl. Might work... */
    return os_sockQueryInterfacesBase(ifList, listSize, validElements, AF_INET);
#else
    struct ifaddrs* interfaceList = NULL;
    struct ifaddrs* nextInterface = NULL;
    unsigned int listIndex = 0;
    unsigned int i;

    *validElements = 0;

    if (getifaddrs (&interfaceList) != 0)
    {
        return os_resultFail;
    }

    nextInterface = interfaceList;

    while (nextInterface != NULL && listIndex < listSize)
    {
        if (nextInterface->ifa_addr &&
            nextInterface->ifa_addr->sa_family == AF_INET)
        {
            os_strncpy(ifList[listIndex].name, nextInterface->ifa_name, OS_IFNAMESIZE);
            memcpy(&ifList[listIndex].address, nextInterface->ifa_addr, sizeof(os_sockaddr_in));
            ifList[listIndex].flags = nextInterface->ifa_flags;

            if (nextInterface->ifa_flags & IFF_BROADCAST)
            {
                memcpy(&ifList[listIndex].broadcast_address, nextInterface->ifa_broadaddr, sizeof(os_sockaddr_in));
            }
            else
            {
                memset (&ifList[listIndex].broadcast_address, 0, sizeof (ifList[listIndex].broadcast_address));
            }

            memcpy(&ifList[listIndex].network_mask, nextInterface->ifa_addr, sizeof(os_sockaddr_in));
#if defined (VXWORKS_RTP)
            ifList[listIndex].interfaceIndexNo = os_nameToIndex(nextInterface->ifa_name, AF_INET);
#else
            ifList[listIndex].interfaceIndexNo = (os_uint) if_nametoindex(nextInterface->ifa_name);
#endif
            ++listIndex;
        }
        nextInterface = nextInterface->ifa_next;
    }

    nextInterface = interfaceList;

    /* Walk list again to find interfaces which are not connected.
     * Note that these interfaces have only an entry for the address family AF_PACKET.
     * The status in the corresponding flag field is set to IFF_DOWN.
     */
    while (nextInterface != NULL && listIndex < listSize)
    {
        if (nextInterface->ifa_addr &&
            nextInterface->ifa_addr->sa_family == AF_PACKET)
        {
            for (i = 0; i < listIndex; i++) {
                if (strcmp(ifList[i].name, nextInterface->ifa_name) == 0) {
                    break;
                }
            }

            if (i == listIndex) {
                os_strncpy(ifList[listIndex].name, nextInterface->ifa_name, OS_IFNAMESIZE);
                memset (&ifList[listIndex].address, 0, sizeof(ifList[listIndex].address));
                ifList[listIndex].flags = 0;
                memset (&ifList[listIndex].broadcast_address, 0, sizeof (ifList[listIndex].broadcast_address));
                memset (&ifList[listIndex].network_mask, 0, sizeof (ifList[listIndex].network_mask));
                ifList[listIndex].interfaceIndexNo = 0;
                ++listIndex;
            }
        }
        nextInterface = nextInterface->ifa_next;
    }


    *validElements = listIndex;
    freeifaddrs(interfaceList);
    return os_resultSuccess;
#endif /*OS_NO_GETIFADDRS */

    return os_resultFail;
}



os_result
os_sockQueryIPv6Interfaces(
    os_ifAttributes *ifList,
    os_uint32 listSize,
    os_uint32 *validElements)
{
/* Implementations in order of preference:
 * getifaddrs() - this method
 * 'L' form ioctls (SIOCGLIFCONF etc) - os_sockQueryInterfacesBaseL
 * older spec ioctls (SIOCGIFCONF etc) - os_sockQueryInterfacesBase
 */
#if (OS_SOCKET_HAS_IPV6 == 1)
#ifdef OS_NO_GETIFADDRS
#ifdef OS_NO_SIOCGLIFCONF
    return os_sockQueryInterfacesBase(ifList, listSize, validElements, AF_INET6);
#else
    return os_sockQueryInterfacesBaseL(ifList, listSize, validElements, AF_INET6);
#endif
#else
    /* getifaddrs is preferable if available */
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
                ifList[listIndex].flags = nextInterface->ifa_flags;
                memset(&ifList[listIndex].broadcast_address, 0, sizeof (ifList[listIndex].broadcast_address));
                memset(&ifList[listIndex].network_mask, 0, sizeof (&ifList[listIndex].network_mask));

#if defined (VXWORKS_RTP)
                ifList[listIndex].interfaceIndexNo = os_nameToIndex(nextInterface->ifa_name, AF_INET6);
#else
                ifList[listIndex].interfaceIndexNo = (os_uint) if_nametoindex(nextInterface->ifa_name);
#endif

                ++listIndex;
            }
        }
        nextInterface = nextInterface->ifa_next;
    }

    *validElements = listIndex;
    freeifaddrs(interfaceList);
    return os_resultSuccess;
#endif /*OS_NO_GETIFADDRS */
#else
    return os_resultFail;
#endif /* OS_SOCKET_HAS_IPV6 */
}


#if !defined(OS_NO_NETLINK) && defined(__gnu_linux__)
typedef struct os_sockQueryInterfaceStatusInfo_s {
    char *ifName;
    int sock;
} os_sockQueryInterfaceStatusInfo;

void
os_sockQueryInterfaceStatusDeinit(
    void *handle)
{
    os_sockQueryInterfaceStatusInfo *info = (os_sockQueryInterfaceStatusInfo *) handle;

    if (info) {
        if (info->ifName) {
            os_free(info->ifName);
        }
        if (info->sock >= 0) {
            close(info->sock);
        }
        os_free(info);
    }
}

void *
os_sockQueryInterfaceStatusInit(
    const char *ifName)
{
    os_sockQueryInterfaceStatusInfo *info = NULL;
    struct sockaddr_nl addr;
    int sock;

    sock = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (sock >= 0) {
        memset(&addr, 0, sizeof(addr));
        addr.nl_family = AF_NETLINK;
        addr.nl_groups = RTMGRP_IPV4_IFADDR;

        if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
            if(close(sock) == -1){
                os_report(OS_WARNING, "os_sockQueryInterfaceStatusInit", __FILE__, __LINE__, 0,
                        "Failed to close socket; close(%d) failed with error: %s",
                        sock, strerror(errno));
            }
            sock = -1;
            os_report(OS_ERROR, "os_sockQueryInterfaceStatusInit", __FILE__, __LINE__, 0,
                      "Failed to bind to NETLINK socket");
        }
    } else {
        os_report(OS_ERROR, "os_sockQueryInterfaceStatusInit", __FILE__, __LINE__, 0,
                  "Failed to create NETLINK socket");
    }

    if (sock >= 0) {
        info = (os_sockQueryInterfaceStatusInfo *) os_malloc(sizeof(os_sockQueryInterfaceStatusInfo));
        if (info) {
            info->sock = sock;
            info->ifName = os_strdup(ifName);
            if (!info->ifName) {
                os_sockQueryInterfaceStatusDeinit(info);
                info = NULL;
                os_report(OS_ERROR, "os_sockQueryInterfaceStatusInit", __FILE__, __LINE__, 0,
                          "Out of resources. Failed to allocate %d bytes for string '%s'",
                          strlen(ifName), ifName);
            }
        } else {
            if(close(sock) == -1){
                os_report(OS_WARNING, "os_sockQueryInterfaceStatusInit", __FILE__, __LINE__, 0,
                        "Failed to close socket; close(%d) failed with error: %s",
                        sock, strerror(errno));
            }
            os_report(OS_ERROR, "os_sockQueryInterfaceStatusInit", __FILE__, __LINE__, 0,
                    "Out of resources. Failed to allocate %d bytes for os_sockQueryInterfaceStatusInfo",
                    sizeof(*info));
        }
    }

    return info;
}

os_result
os_sockQueryInterfaceStatus(
    void *handle,
    os_time timeout,
    os_boolean *status)
{
    os_sockQueryInterfaceStatusInfo *info = (os_sockQueryInterfaceStatusInfo *) handle;
    os_result result = os_resultBusy;
    unsigned int len;
    char buffer[1024];
    struct nlmsghdr *nlh;
    fd_set fdset;
    int r;
    struct timeval t;
    os_time endTime;

    *status = 0;

    if (info && info->sock >= 0) {

        FD_ZERO(&fdset);
        FD_SET(info->sock, &fdset);

        endTime = os_timeAdd(os_timeGet(), timeout);
        do {

            t.tv_sec  = timeout.tv_sec;
            t.tv_usec = timeout.tv_nsec / 1000;

            r = select(info->sock + 1, &fdset, NULL, NULL, &t);
            if (r > 0) {
                nlh = (struct nlmsghdr *)buffer;
                if ((len = recv(info->sock, nlh, sizeof(buffer), 0)) > 0) {
                    while ((result == os_resultBusy) && (NLMSG_OK(nlh, len)) && (nlh->nlmsg_type != NLMSG_DONE)) {
                        char name[IFNAMSIZ];
                        struct ifaddrmsg *ifa;
                        struct rtattr *rth;
                        int rtl;

                        if ((nlh->nlmsg_type == RTM_NEWADDR) || (nlh->nlmsg_type == RTM_DELADDR)) {
                            ifa = (struct ifaddrmsg *) NLMSG_DATA(nlh);
                            rth = IFA_RTA(ifa);
                            rtl = IFA_PAYLOAD(nlh);

                            while ((result == os_resultBusy) && rtl && RTA_OK(rth, rtl)) {
                                if (rth->rta_type == IFA_LOCAL) {
                                    if (if_indextoname(ifa->ifa_index, name) != NULL) {
                                        if (strncmp(info->ifName, name, IFNAMSIZ) == 0) {
                                            if (nlh->nlmsg_type == RTM_NEWADDR) {
                                                *status = 1;
                                            }
                                            result = os_resultSuccess;
                                        }
                                    }
                                }
                                rth = RTA_NEXT(rth, rtl);
                            }
                        }
                        nlh = NLMSG_NEXT(nlh, len);
                    }
                }
                if (result == os_resultBusy) {
                    timeout = os_timeSub(endTime, os_timeGet());
                }
            } else if (r == 0) {
                result = os_resultTimeout;
            } else {
                result = os_resultFail;
            }
        } while ((result == os_resultBusy) && (timeout.tv_sec > 0));
        result = (result == os_resultBusy) ? os_resultTimeout : result;
    } else {
        result = os_resultFail;
    }

    return result;
}

#else

void
os_sockQueryInterfaceStatusDeinit(
    void *handle)
{

}

void *
os_sockQueryInterfaceStatusInit(
    const char *ifName)
{
    return NULL;
}

os_result
os_sockQueryInterfaceStatus(
    void *handle,
    os_time timeout,
    os_boolean *status)
{
    *status = OS_FALSE;

    return os_resultFail;
}


#endif
