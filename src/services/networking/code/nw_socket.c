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
/* Interface */
#include "nw_socket.h"
#include "nw__socket.h"

/* Implementation */
#include <string.h>       /* for memcmp and memset          */
#include "os_heap.h"
#include "os_stdlib.h"
#include "os_socket.h"

#ifndef OSPL_NO_ZLIB
#include "zlib.h"
#endif

/* Descendants */
#include "nw_socketBroadcast.h"
#include "nw_socketMulticast.h"
#include "nw_socketLoopback.h"

#include "nw__plugDataBuffer.h" /* for timestamp */
#include "nw__confidence.h"
#include "nw_socketMisc.h"
#include "nw_socketPartitions.h"
#include "nw_configuration.h"
#include "nw_profiling.h"
#include "nw_misc.h" /* for nw_stringDup and nw_dumpToString */
#include "nw_stringList.h"
#include "nw_report.h"

#define SK_CONTROLPORT(portNr) ((portNr)+1)


/* ------------------------------ Debug function ---------------------------- */
/*
#define NW_HEXDUMP(message, partitionId, data, length)    \
        NW_TRACE_4(Test, 6, "%s: %u bytes to partitionId %u\n%s", message, length, partitionId, nw_dumpToString(data, length));

#define NW_HEXDUMPTO(message, address, data, length)    \
        NW_TRACE_4(Test, 6, "%s: %u bytes to Address %s\n%s", message, length, address, nw_dumpToString(data, length));
*/

#define NW_HEXDUMP(message, partitionId, data, length)    \
        NW_TRACE_3(Test, 6, "%s: %u bytes to partitionId %u", message, length, partitionId);

#define NW_HEXDUMPTO(message, address, data, length)    \
        NW_TRACE_3(Test, 6, "%s: %u bytes to Address %s", message, length, address);

#define NW_HEXDUMPFROM(message, address, data, length) \
        NW_TRACE_3(Test, 6, "%s: %u bytes from address 0x%x", message, length, address);


/* ------------------------------- main class ------------------------------- */

/**
* @brief A networking service socket
*/
NW_STRUCT(nw_socket) {
    char *name;
    /** Data socket, for sending and receiving data */
    os_socket socketData;
    /** Primary address, for destination checking */
    os_sockaddr_storage sockAddrPrimary;
    /** Broadcast address corresponding to this interface */
    os_sockaddr_storage sockAddrBroadcast;
    /** Default address to send data to */
    os_sockaddr_storage sockAddrData;
    /** Control address, for sending and receiving protocol control messages */
    os_sockaddr_storage sockAddrControl;
    /** Parameter idenfitifying socket for control messages (acks etc) */
    sk_bool supportsControl;
    /** Socket for control messages (acks etc) */
    os_socket socketControl;
    /**
    * Cached socket set for select statement
    */
    fd_set sockSet;
    /**
    * Cached 'nfds' (Max socket FD value + 1) value for select call
    */
    os_int maxSockfd;
    /** Socket state and settings */
    os_int loopsback;
    /** List of alternative addresses for sending data */
    nw_socketPartitions partitions;
    sk_bool multicastSupported;
    sk_bool multicastInitialized;

    /**
    * Network interface index #
    */
    os_uint interfaceIndexNo;
    os_uchar currentMulticastTTL;
};



/* ---------------------------- getters/setters ----------------------------- */

/**
* Return the interface index that this nw_socket is recorded to correspond to.
* @param this_ This pointer.
*/
os_uint
nw_socketGetInterfaceIndexNo(nw_socket this_)
{
    if (this_->interfaceIndexNo == 0 && nw_configurationGetIsIPv6())
    {
        /* For some reason on this platform we have not been able to determine the interface index no.
        This might present a problem with IPv6 multicast.*/
        NW_REPORT_WARNING_1("nw_socketGetInterfaceIndexNo",
                                   "Accessing unitialised interface number from nw_socket: %s",
                                   this_->name);
        NW_TRACE_1(Configuration, 2, "nw_socketGetInterfaceIndexNo - Accessing unitialised interface number from nw_socket: %s",
                                   this_->name);
    }
    return this_->interfaceIndexNo;
}

/**
* Record the interface index number that this nw_socket is configured to.
* @param this_ This pointer.
* @param indexNo The index.
*/
void
nw_socketSetInterfaceIndexNo(nw_socket this_, os_uint indexNo)
{
    this_->interfaceIndexNo = indexNo;
}

sk_bool
nw_socketLoopsback(
    nw_socket sock)
{
    return sock->loopsback;
}

/**
* Getter for the socket primary address.
* @param sock Pointer to self
* @return Copy of the socket primary address
* @memberof nw_socket_s
*/
os_sockaddr_storage
nw_socketPrimaryAddress(
    nw_socket sock)
{
    os_sockaddr_storage result;

    if (sock) {
        return sock->sockAddrPrimary;
    } else {
        NW_CONFIDENCE(0);
        memset(&result, 0, sizeof(result));
        return result;
    }
}

/**
* Getter for the socket broadcast address.
* @param sock Pointer to self
* @return Copy of the socket broadcast address
* @memberof nw_socket_s
*/
os_sockaddr_storage
nw_socketBroadcastAddress(
    nw_socket sock)
{
    os_sockaddr_storage result;

    NW_CONFIDENCE(sizeof(sock->sockAddrBroadcast) == sizeof(os_sockaddr_storage));

    if (sock) {
        return sock->sockAddrBroadcast;
    } else {
        NW_CONFIDENCE(0);
        memset(&result, 0, sizeof(result));
        return result;
    }
}

/**
* Getter for the socket data address.
* @param sock Pointer to self
* @return Copy of the socket data address
* @memberof nw_socket_s
*/
os_sockaddr_storage
nw_socketDataAddress(
    nw_socket sock)
{
    os_sockaddr_storage result;

    NW_CONFIDENCE(sizeof(sock->sockAddrData) == sizeof(os_sockaddr_storage));

    if (sock) {
        return sock->sockAddrData;
    } else {
        NW_CONFIDENCE(0);
        memset(&result, 0, sizeof(result));
        return result;
    }

    return result;
}

socklen_t
nw_socketAddressSize(os_sockaddr_storage addr)
{
   os_uint size = 0;
   switch (addr.ss_family) {
     case AF_INET:
       size = sizeof(os_sockaddr_in);
       break;
     case AF_INET6:
       size = sizeof(os_sockaddr_in6);
       break;
     default:
       NW_CONFIDENCE(0);
       size = sizeof(os_sockaddr_storage);
   }
   return size;
}

os_int
nw_socketSetSendBufferSize(
    nw_socket sock,
    os_int bufSize)
{
    os_int result = SK_TRUE;
    socklen_t optLen;
    os_result retVal;

    if (sock != NULL) {
        /* Set sendbuffer option */
        optLen = (os_uint32)sizeof(bufSize);

        retVal = os_sockSetsockopt(sock->socketData,
                            SOL_SOCKET, SO_SNDBUF,
                            (void *)&bufSize, optLen);
        SK_REPORT_SOCKFUNC(2, retVal,
                           "set socket sendbuffer size", "setsockopt");

        if ((retVal == os_resultSuccess) && (sock->supportsControl)) {
            retVal = os_sockSetsockopt(sock->socketControl,
                                SOL_SOCKET, SO_SNDBUF,
                                (void *)&bufSize, optLen);
            SK_REPORT_SOCKFUNC(2, retVal,
                               "set socket sendbuffer size", "setsockopt");
        }
        if (retVal != os_resultSuccess) {
            result = SK_FALSE;
        }
    } else {
        result = SK_FALSE;
    }

    return result;
}


os_int
nw_socketSetReceiveBufferSize(
    nw_socket sock,
    os_int bufSize)
{
    os_int result = SK_TRUE;
    socklen_t optLen;
    os_int actualSize;
    os_result retVal;

    if (sock != NULL) {
        /* Set receivebuffer option */
        optLen = (socklen_t)sizeof(bufSize);
        retVal = os_sockSetsockopt(sock->socketData,
                            SOL_SOCKET, SO_RCVBUF,
                            (const void *)&bufSize, optLen);
        SK_REPORT_SOCKFUNC(2, retVal,
                           "set socket receivebuffer size", "setsockopt");

        if (retVal == os_resultSuccess) {
            /* The following lines are for tracing purposes only */
            actualSize = 0;
            retVal = os_sockGetsockopt(sock->socketData, SOL_SOCKET, SO_RCVBUF,
                (void *)&actualSize, &optLen);
            SK_REPORT_SOCKFUNC(4, retVal,
                               "get socket receivebuffer size", "getsockopt");
            NW_TRACE_2(Receive, 5, "Receive buffer size set. Requested: %d, actual: %d",
                bufSize, actualSize);

            if ((retVal == os_resultSuccess) && (sock->supportsControl)) {
                retVal = os_sockSetsockopt(sock->socketControl,
                                    SOL_SOCKET, SO_RCVBUF,
                                    (const void *)&bufSize, optLen);
                SK_REPORT_SOCKFUNC(2, retVal,
                                   "set socket receivebuffer size", "setsockopt");
            }
        }
        if (retVal != os_resultSuccess) {
            result = SK_FALSE;
        }
    } else {
        result = SK_FALSE;
    }

    return result;
}


os_int
nw_socketSetBroadcastOption(
    nw_socket sock,
    os_int enableBroadcast)
{
    os_int result = SK_TRUE;
    socklen_t optLen;
    os_result retVal;

    if (!nw_configurationGetIsIPv6() /* No point setting broadcast opt on IPv6 sockets */
        && sock != NULL) {
        optLen = (socklen_t)sizeof(enableBroadcast);
        retVal = os_sockSetsockopt(sock->socketData,
                            SOL_SOCKET, SO_BROADCAST,
                            (const void *)&enableBroadcast, optLen);
        SK_REPORT_SOCKFUNC(2, retVal,
                           "set socket broadcast option", "setsockopt");
        if ((retVal == os_resultSuccess) && (sock->supportsControl)) {
            retVal = os_sockSetsockopt(sock->socketControl,
                                SOL_SOCKET, SO_BROADCAST,
                                (const void *)&enableBroadcast, optLen);
            SK_REPORT_SOCKFUNC(2, retVal,
                               "set socket broadcast option", "setsockopt");
        }
        if (retVal != os_resultSuccess) {
            result = SK_FALSE;
        }
    } else {
        result = SK_FALSE;
    }

    return result;
}

os_int
nw_socketSetTOS(
    nw_socket sock,
    os_int tos)
{
    os_int result = SK_TRUE;
    socklen_t optLen;
    os_result retVal;

    if (nw_configurationGetIsIPv6())
    {
        /* @todo dds2523 Can't find IPv6 alternative. No-op */
    }
    else if (sock != NULL) {
        optLen = (socklen_t)sizeof(tos);
        retVal = os_sockSetsockopt(sock->socketData,
                            IPPROTO_IP, IP_TOS,
                            (const void *)&tos, optLen);
        SK_REPORT_SOCKFUNC(2, retVal,
                           "set socket type of service", "setsockopt");
        if ((retVal == os_resultSuccess) && (sock->supportsControl)) {
            retVal = os_sockSetsockopt(sock->socketControl,
                                IPPROTO_IP, IP_TOS,
                                (const void *)&tos, optLen);
            SK_REPORT_SOCKFUNC(2, retVal,
                               "set socket type of service", "setsockopt");
        }
        if (retVal != os_resultSuccess) {
            result = SK_FALSE;
        }
    } else {
        result = SK_FALSE;
    }

    return result;
}

static int
nw_socketSetTimeToLive(
    nw_socket sock,
    c_ulong timeToLive)
{
    int result = SK_TRUE;
    int opt = (int)timeToLive;
    socklen_t optLen;
    os_result retVal;

    if (sock != NULL) {
        optLen = (socklen_t)sizeof(timeToLive);
        if (nw_configurationGetIsIPv6())
        {
            retVal = os_sockSetsockopt(sock->socketData,
                            IPPROTO_IPV6, IPV6_UNICAST_HOPS,
                            (const void *)&opt, optLen);
            SK_REPORT_SOCKFUNC(2, retVal,
                            "setting IPv6 hop limit option on data socket", "setsockopt");
        }
        else
        {
            retVal = os_sockSetsockopt(sock->socketData,
                            IPPROTO_IP, IP_TTL,
                            (const void *)&opt, optLen);
            SK_REPORT_SOCKFUNC(2, retVal,
                            "set socket time to live", "setsockopt");
        }
        if ((retVal == os_resultSuccess) && (sock->supportsControl)) {
            if (nw_configurationGetIsIPv6())
            {
                retVal = os_sockSetsockopt(sock->socketControl,
                                IPPROTO_IPV6, IPV6_UNICAST_HOPS,
                                (const void *)&opt, optLen);
                SK_REPORT_SOCKFUNC(2, retVal,
                                "setting IPv6 hop limit option on control socket", "setsockopt");
            }
            else
            {
                retVal = os_sockSetsockopt(sock->socketControl,
                                IPPROTO_IP, IP_TTL,
                                (const void *)&opt, optLen);
                SK_REPORT_SOCKFUNC(2, retVal,
                                "set socket time to live", "setsockopt");
            }
        }
        if (retVal != os_resultSuccess) {
            result = SK_FALSE;
        }
    } else {
        result = SK_FALSE;
    }

    return result;
}


os_int
nw_socketSetDontRouteOption(
    nw_socket sock,
    os_int disableRouting)
{
    os_int result = SK_TRUE;
    socklen_t optLen;
    os_result retVal;
    os_uint ipv6Flag = 1;

    if (sock != NULL) {
        if (nw_configurationGetIsIPv6() && disableRouting)
        {
            /* @todo dds2523 Was seeing an error on windows w/ SO_DONTROUTE & IPv6. According to
            spec this below is equivalent */
            retVal = os_sockSetsockopt(sock->socketData, IPPROTO_IPV6, IPV6_UNICAST_HOPS,
                                        &ipv6Flag, sizeof(ipv6Flag));
            SK_REPORT_SOCKFUNC(2, retVal,
                               "implicitly setting 'SO_DONTROUTE' on IPv6 dataSocket by setting hop limit of 1", "setsockopt");
            if ((retVal == os_resultSuccess) && (sock->supportsControl)){
                retVal = os_sockSetsockopt(sock->socketControl, IPPROTO_IPV6, IPV6_UNICAST_HOPS,
                                            &ipv6Flag, sizeof(ipv6Flag));
                SK_REPORT_SOCKFUNC(2, retVal,
                               "implicitly setting 'SO_DONTROUTE' on IPv6 controlSocket by setting hop limit of 1", "setsockopt");
            }
        }
        else
        {
            optLen = (socklen_t)sizeof(disableRouting);
            retVal = os_sockSetsockopt(sock->socketData,
                                SOL_SOCKET, SO_DONTROUTE,
                                (const void *)&disableRouting, optLen);
            SK_REPORT_SOCKFUNC(2, retVal,
                               "set socket dontroute option", "setsockopt");
            if ((retVal == os_resultSuccess) && (sock->supportsControl)){
                retVal = os_sockSetsockopt(sock->socketControl,
                                    SOL_SOCKET, SO_DONTROUTE,
                                    (const void *)&disableRouting, optLen);
                SK_REPORT_SOCKFUNC(2, retVal,
                                   "set socket dontroute option", "setsockopt");
            }
        }
        if (retVal != os_resultSuccess) {
            result = SK_FALSE;
        }
    } else {
        result = SK_FALSE;
    }

    return result;
}


#if 0
/* @todo - This could probably be removed ??
The option isn't even SO_DONTFRAG */
static int
nw_socketSetDontFragment(
    nw_socket sock,
    nw_bool dontFragment)
{
    int result = SK_TRUE;
    int opt = dontFragment?1:0;
    socklen_t optLen;
    os_result retVal;

    if (sock != NULL) {
        optLen = (socklen_t)sizeof(opt);
        retVal = os_sockSetsockopt(sock->socketData,
                            IPPROTO_IP, SO_DONTFRAG,
                            (const void *)&opt, optLen);
        SK_REPORT_SOCKFUNC(2, retVal,
                           "set socket dontfragment option", "setsockopt");
        if ((retVal == os_resultSuccess) && (sock->supportsControl)){
            retVal = os_sockSetsockopt(sock->socketControl,
                                IPPROTO_IP, SO_DONTFRAG,
                                (const void *)&opt, optLen);
            SK_REPORT_SOCKFUNC(2, retVal,
                               "set socket dontfragment option", "setsockopt");
        }
        if (retVal != os_resultSuccess) {
            result = SK_FALSE;
        }
    } else {
        result = SK_FALSE;
    }

    return result;
}
#endif

sk_bool
nw_socketGetDataSocket(
    nw_socket sock,
    os_socket *socket)
{
    sk_bool result = SK_FALSE;

    NW_CONFIDENCE(sock != NULL);
    NW_CONFIDENCE(socket != NULL);

    if ((sock != NULL) && (socket != NULL)) {
        *socket = sock->socketData;
        result = SK_TRUE;
    }

    return result;
}

sk_bool
nw_socketGetControlSocket(
    nw_socket sock,
    os_socket *socket)
{
    sk_bool result = SK_FALSE;

    NW_CONFIDENCE(sock != NULL);
    NW_CONFIDENCE(socket != NULL);

    if ((sock != NULL) && (socket != NULL)) {
        result = sock->supportsControl;
        if (result) {
             *socket = sock->socketControl;
        }
    }

    return result;
}

os_int
nw_socketBind(
    nw_socket sock)
{
    os_int result = SK_TRUE;
    os_result retVal = os_resultSuccess;
    os_int optVal = 0;
    socklen_t optLen = 0;
    os_sockaddr_storage bindAddress;

    if (sock != NULL) {

        /* Avoid already in use error messages */
        optLen = (os_uint32)sizeof(optVal);
        optVal = SK_TRUE;
        retVal = os_sockSetsockopt(sock->socketData,
                            SOL_SOCKET, SO_REUSEADDR,
                            (void *)&optVal, optLen);
        SK_REPORT_SOCKFUNC(2, retVal,
                           "set socket reuse option", "setsockopt");

        bindAddress = sock->sockAddrData;
        if (nw_configurationGetIsIPv6())
        {
            NW_CONFIDENCE(bindAddress.ss_family == AF_INET6);
            ((os_sockaddr_in6*)&bindAddress)->sin6_addr = os_in6addr_any;
        }
        else
        {
            NW_CONFIDENCE(bindAddress.ss_family == AF_INET);
            ((os_sockaddr_in*)&bindAddress)->sin_addr.s_addr = htonl(INADDR_ANY);
        }

        if (retVal == os_resultSuccess) {
            retVal = os_sockBind(sock->socketData,
                (const os_sockaddr *)&bindAddress,
                (socklen_t)(bindAddress.ss_family == AF_INET ?
                                sizeof(os_sockaddr_in) : sizeof(os_sockaddr_in6)));
/*          NOTE: This used to be:
 *          retVal = os_sockBind(sock->socketData,
 *                (const struct sockaddr *)&sock->sockAddrData,
 *                (socklen_t)sizeof(sock->sockAddrData)); */
            SK_REPORT_SOCKFUNC(2, retVal,
                           "bind socket", "bind");

            if ((retVal == os_resultSuccess) && (sock->supportsControl)) {

                retVal = os_sockSetsockopt(sock->socketControl,
                                SOL_SOCKET, SO_REUSEADDR,
                                (void *)&optVal, optLen);
                SK_REPORT_SOCKFUNC(2, retVal,
                               "set socket reuse option", "setsockopt");

                bindAddress = sock->sockAddrControl;
                if (nw_configurationGetIsIPv6())
                {
                    NW_CONFIDENCE(bindAddress.ss_family == AF_INET6);
                    ((os_sockaddr_in6*)&bindAddress)->sin6_addr = os_in6addr_any;
                }
                else
                {
                    NW_CONFIDENCE(bindAddress.ss_family == AF_INET);
                    ((os_sockaddr_in*)&bindAddress)->sin_addr.s_addr = htonl(INADDR_ANY);
                }

                if (retVal == os_resultSuccess) {
                    retVal = os_sockBind(sock->socketControl,
                        (const os_sockaddr *)&bindAddress,
                        nw_socketAddressSize(bindAddress));
/*                  NOTE: This used to be:
 *                  retVal = os_sockBind(sock->socketControl,
 *                    (const struct sockaddr *)&sock->sockAddrControl,
 *                    (socklen_t)sizeof(sock->sockAddrControl)); */
                    SK_REPORT_SOCKFUNC(2, retVal,
                                   "bind socket", "bind");
                }
            }
        }
        if (retVal != os_resultSuccess) {
            result = SK_FALSE;
        }
    } else {
       result = SK_FALSE;
    }

    return result;
}



void
nw_socketAddPartition(
    nw_socket sock,
    sk_partitionId partitionId,
    const char *addressString,
    sk_bool connected,
    sk_bool compression,
    sk_bool receiving,
    c_ulong mTTL)
{
    os_sockaddr_storage address;
    nw_stringList addressNameList;
    unsigned int size, i;
    const char *currentAddress;
    os_boolean success;
    char addressStr[INET6_ADDRSTRLEN];

    /* don't log for NULL pointer or empty string */
    if (addressString && *addressString ) {
        NW_TRACE_2(Test, 3, "Adding address expression \"%s\" to partition %d",
            addressString, partitionId);
    }

    addressNameList = nw_stringListNew(addressString, NW_ADDRESS_SEPARATORS);

    size = nw_stringListGetSize(addressNameList);
    for (i=0; i<size; i++) {
        success = OS_FALSE;
        currentAddress = nw_stringListGetValue(addressNameList, i);
        if (sk_getAddressType(currentAddress) == SK_TYPE_BROADCAST) {
            address = nw_socketBroadcastAddress(sock);
            success = OS_TRUE;
        } else {
            success = os_sockaddrStringToAddress(currentAddress,
                                                 (os_sockaddr*) &address,
                                                 !nw_configurationGetIsIPv6());
            /* Ignore invalid addresses */
            if (!success ||
                address.ss_family != sock->sockAddrData.ss_family)
            {
                NW_TRACE_2(Test, 4, "Ignoring invalid network address \"%s\" in partition %d",
                    currentAddress, partitionId);
                continue;
            }
        }
        /* Copy the destination port value from the data socket address */
        if (address.ss_family == AF_INET)
        {
            ((os_sockaddr_in*)&address)->sin_port = ((os_sockaddr_in*)&sock->sockAddrData)->sin_port;
        }
        else
        {
            ((os_sockaddr_in6*)&address)->sin6_port = ((os_sockaddr_in6*)&sock->sockAddrData)->sin6_port;
            ((os_sockaddr_in6*)&address)->sin6_scope_id = nw_socketGetInterfaceIndexNo(sock);
        }
        /* Ignore our own address */
        if (nw_socketPrimaryAddressCompare(sock, (os_sockaddr*)&address)
#ifdef NW_LOOPBACK
                && !nw_configurationUseLoopback()
#endif
            ) {

            NW_TRACE_3(Test, 4, "Ignoring localhost \"%s\" (%s) in partition %d",
                        currentAddress,
                        os_sockaddrAddressToString((os_sockaddr*) &address,
                                                                    addressStr,
                                                                    sizeof(addressStr)),
                        partitionId);
        } else {

            NW_TRACE_3(Test, 4, "Adding host \"%s\" (%s) to partition %d",
                        currentAddress,
                        os_sockaddrAddressToString((os_sockaddr*) &address,
                                                                    addressStr,
                                                                    sizeof(addressStr)),
                        partitionId);

            nw_socketPartitionsAdd(sock->partitions, partitionId, address, connected, compression, mTTL);
            /* Do any multicast related actions if needed */
            if (partitionId != 0 && connected) {
                /* No need to add first the default partition,
                   that already happened with socket initialisation */
                nw_socketMulticastAddPartition(sock, currentAddress, receiving, mTTL);
            }
        }
    }
    nw_stringListFree(addressNameList);
}

/**
* Some sort of function to work out if a provided network address
* is equal to loopback, inaddr_any (which the comments are calling 'wildcard'), or
* this nw_socket's primary address.
* @param sock This.
* @toCompare The address we want to check out.
*/
os_int
nw_socketPrimaryAddressCompare(
    nw_socket sock,
    os_sockaddr* toCompare)
{
    os_int result = 0;

    NW_CONFIDENCE(sizeof(sock->sockAddrPrimary) == sizeof(os_sockaddr_storage));

    if (sock) {
        /* First check 'wildcard' (zeroes only) */
        if (toCompare->sa_family == AF_INET6)
        {
            result = IN6_IS_ADDR_UNSPECIFIED(&((os_sockaddr_in6*)toCompare)->sin6_addr);
        }
        else if (toCompare->sa_family == AF_INET)
        {
            result = (INADDR_ANY == ntohl(((os_sockaddr_in*)toCompare)->sin_addr.s_addr));
        }
        if (!result) {
            /* No wildcard, then compare to loopback / localhost */
            result = os_sockaddrIsLoopback(toCompare);
        }
        if (!result) {
            /* None of the above, then compare to this priamry */
            result = os_sockaddrIPAddressEqual(toCompare, (os_sockaddr*)&sock->sockAddrPrimary);
        }
    }

    return result;
}

sk_bool
nw_socketGetMulticastInitialized(
        nw_socket sock)
{
    return sock->multicastInitialized;
}

sk_bool
nw_socketGetMulticastSupported(
        nw_socket sock)
{
    return sock->multicastSupported;
}

void
nw_socketSetMulticastInitialized(
        nw_socket sock,
        sk_bool mi)
{
    sock->multicastInitialized = mi;
}


/* ------------------------------- constructor ------------------------------ */


/* private */

static nw_socket
nw_socketNew(
    nw_bool receiving,
    const char *defaultAddress,
    sk_portNr portNr,
    nw_bool supportsControl,
    const char *name)
{
    nw_socket result = NULL;
    os_int success = SK_TRUE;
    char *interfaceLookingFor;
    sk_addressType defaultAddressType;
    const char *defaultNetworkAddress;
    nw_stringList defaultAddressNameList;
    sk_portNr portNrControl = SK_CONTROLPORT(portNr);
    nw_size bufSizeRequested;
    c_bool DontRouteRequested;
    c_bool DontFragRequested;
    c_ulong TOSRequested;
    c_ulong TTLRequested;
    char addressStr[INET6_ADDRSTRLEN];

    defaultAddressNameList = nw_stringListNew(defaultAddress, NW_ADDRESS_SEPARATORS);
    /* Use first entry in stringlist as default address */
    defaultNetworkAddress = nw_stringListGetValue(defaultAddressNameList, 0);

    result = (nw_socket)os_malloc((os_uint32)sizeof(*result));
    if (result != NULL) {
        result->name = nw_stringDup(name);
        result->interfaceIndexNo = 0;
        result->supportsControl = supportsControl;
        result->socketData = os_sockNew((nw_configurationGetIsIPv6() ? AF_INET6 : AF_INET), SOCK_DGRAM);
        SK_REPORT_SOCKFUNC(2, os_resultSuccess,
                           "socket creation", "socket");
        /* create a control socket if required and cache the (highest
        [of the] socket FD[s] + 1) for select */
        if (supportsControl) {
            result->socketControl = os_sockNew((nw_configurationGetIsIPv6() ? AF_INET6 : AF_INET), SOCK_DGRAM);
            SK_REPORT_SOCKFUNC(2, os_resultSuccess,
                               "socket creation", "socket");

            if (result->socketData > result->socketControl) {
                result->maxSockfd = result->socketData + 1;
            } else {
                result->maxSockfd = result->socketControl + 1;
            }
        } else {
            result->maxSockfd = result->socketData + 1;
        }
        result->multicastSupported = SK_FALSE;
        result->multicastInitialized = SK_FALSE;
        result->currentMulticastTTL = NWCF_DEF(MulticastTimeToLive);

        FD_ZERO(&result->sockSet);

        interfaceLookingFor = NWCF_SIMPLE_PARAM(String, NWCF_ROOT(General), Interface);
        defaultAddressType = sk_getAddressType(defaultNetworkAddress);
        switch (defaultAddressType) {
            case SK_TYPE_UNKNOWN:
            case SK_TYPE_UNICAST:
            case SK_TYPE_BROADCAST:
            case SK_TYPE_MULTICAST:
                if (nw_socketGetDefaultMulticastInterface
                            (result,
                             interfaceLookingFor,
                             result->socketData,
                             &result->sockAddrPrimary,
                             &result->sockAddrBroadcast)  != SK_TRUE)
                {
                    if (defaultAddressType == SK_TYPE_MULTICAST) {
                        NW_REPORT_ERROR("socketNew", "No multicastinterface available");
                    }
                    else {
                        if (nw_configurationGetIsIPv6() /* No such thing as IPv6 broadcast */
                            || nw_socketGetDefaultBroadcastInterface(interfaceLookingFor,
                                result->socketData, &result->sockAddrPrimary,
                                &result->sockAddrBroadcast) != SK_TRUE)
                        {
                            NW_REPORT_ERROR("socketNew", "No networkinterface available");
                        }
                    }
                } else {
                    result->multicastSupported = SK_TRUE;
                }

                if (defaultAddressType == SK_TYPE_MULTICAST) {
                    if (! os_sockaddrStringToAddress(defaultNetworkAddress,
                                                 (os_sockaddr*) &result->sockAddrData,
                                                 !nw_configurationGetIsIPv6()))
                    {
                        os_sockaddrStringToAddress(NWCF_DEF(Address),
                                                 (os_sockaddr*) &result->sockAddrData,
                                                 !nw_configurationGetIsIPv6());
                    }
                    result->loopsback = SK_FALSE;
                }
                else {
                    if (nw_configurationGetIsIPv6())
                    {
                        /* Should only really get here if we are doing unicast address list.
                        All Ipv6 interfaces are multicast enabled so presumably can
                        be stopped from looping back. Set sockAddrData to the (arbitrary)
                        link local multicast address - allegedly this is not used. */
                        os_sockaddrStringToAddress("ff02::1",
                                                 (os_sockaddr*) &result->sockAddrData,
                                                 OS_FALSE);
                        result->loopsback = SK_FALSE;
                    }
                    else
                    {
                        result->sockAddrData = result->sockAddrBroadcast;
                        /* Broadcast sockets usually loop back and can not be stopped
                        * from that */
                        result->loopsback = SK_TRUE;
                        NW_TRACE_1(Test, 4, "Using broadcast address %s for default partition",
                            inet_ntoa(((os_sockaddr_in*)&result->sockAddrData)->sin_addr));
                    }
                }
            break;
            case SK_TYPE_LOOPBACK:
                nw_socketGetDefaultLoopbackAddress(result->socketData,
                    &result->sockAddrPrimary);
                result->sockAddrData = result->sockAddrPrimary;
                result->sockAddrBroadcast = result->sockAddrPrimary;
                result->loopsback = SK_TRUE;
            break;
            default:
            break;
        }
        os_free(interfaceLookingFor);

        if (nw_configurationGetIsIPv6())
        {
            ((os_sockaddr_in6*)&result->sockAddrPrimary)->sin6_family = AF_INET6;
            ((os_sockaddr_in6*)&result->sockAddrPrimary)->sin6_port = htons(0); /* Don't care */
            ((os_sockaddr_in6*)&result->sockAddrData)->sin6_family = AF_INET6;
            ((os_sockaddr_in6*)&result->sockAddrData)->sin6_port = htons(portNr);
            result->sockAddrControl = result->sockAddrData;
            ((os_sockaddr_in6*)&result->sockAddrControl)->sin6_port = htons(portNrControl);
        }
        else
        {
            ((os_sockaddr_in*)&result->sockAddrPrimary)->sin_family = AF_INET;
            ((os_sockaddr_in*)&result->sockAddrPrimary)->sin_port = htons(0); /* Don't care */
            ((os_sockaddr_in*)&result->sockAddrData)->sin_family = AF_INET;
            ((os_sockaddr_in*)&result->sockAddrData)->sin_port = htons(portNr);
            result->sockAddrControl = result->sockAddrData;
            ((os_sockaddr_in*)&result->sockAddrControl)->sin_port = htons(portNrControl);
        }
        result->partitions = nw_socketPartitionsNew();

        os_sockaddrAddressToString((os_sockaddr*) &result->sockAddrPrimary,
                                                                    addressStr,
                                                                    sizeof(addressStr));

        if (receiving) {
            /* Set option to avoid sendbuffer */
            success = success && nw_socketSetSendBufferSize(result, 0);
            /* Set option for custom receive buffer size */
            bufSizeRequested = (nw_size)NWCF_SIMPLE_SUBPARAM(Size, name, Rx, ReceiveBufferSize);

            if(bufSizeRequested > OS_MAX_INTEGER(os_int))
            {
                bufSizeRequested = OS_MAX_INTEGER(os_int);
            }

            success = success && nw_socketSetReceiveBufferSize(result, (os_int)bufSizeRequested);

            /* Bind to socket */
            success = success && nw_socketBind(result);

            if (success) {
                if (!supportsControl) {
                    NW_REPORT_INFO_3(2, "Created and bound receiving socket \"%s\" "
                                        "for interface %s, port %u",
                                     name,
                                     addressStr,
                                     portNr);
                    NW_TRACE_4(Configuration, 1, "Created and bound receiving socket \"%s\" "
                                        "for interface %s, port %u, handle %d",
                                     name,
                                     addressStr,
                                     portNr,
                                     result->socketData);
                } else {
                    NW_REPORT_INFO_4(2, "Created and bound receiving socket \"%s\" "
                                        "for interface %s, ports %u and %u",
                                     name,
                                     addressStr,
                                     portNr, portNr+1);
                    NW_TRACE_6(Configuration, 1, "Created and bound receiving socket \"%s\" "
                                        "for interface %s, ports %u and %u, handles %d and %d",
                                     name,
                                     addressStr,
                                     portNr, portNr+1,
                                     result->socketData, result->socketControl);
                }
                NW_TRACE_1(Test, 1, "Creation and binding of receiving "
                                  "socket \"%s\" succeeded.",
                                  name);
            } else {
                if (!supportsControl) {
                    NW_REPORT_ERROR_3("socketNew", "Creation and binding of receiving socket \"%s\" failed "
                                        "for interface %s, port %u",
                                     name,
                                     addressStr,
                                     portNr);
                } else {
                    NW_REPORT_ERROR_4("socketNew", "Creation and binding of receiving socket \"%s\" failed "
                                        "for interface %s, ports %u and %u",
                                     name,
                                     addressStr,
                                     portNr, portNr+1);
                }
            }
        } else {
            /* Set option to avoid receivebuffer */
            success = success && nw_socketSetReceiveBufferSize(result, 0);
            /* Set option for avoiding routing to other interfaces */
#ifndef OS_VXWORKS_DEFS_H
            /* Set options for DONT_ROUTE flag in IP header */
            DontRouteRequested = NWCF_SIMPLE_SUBPARAM(Bool, name, Tx, DontRoute);
            success = success && nw_socketSetDontRouteOption(result, DontRouteRequested);
#endif
            /* Set options for DONT_FRAG flag in IP header */
            DontFragRequested = NWCF_SIMPLE_SUBPARAM(ULong, name, Tx, DontFragment);
            /*success = success && nw_socketSetDontFrag(result, DontFragRequested);*/

            /* Set option for custom TOS */
            TOSRequested = NWCF_SIMPLE_SUBPARAM(ULong, name, Tx, DiffServField);
            success = success && nw_socketSetTOS(result, (os_int)TOSRequested);

            /* Set option for custom TTL */
            TTLRequested = NWCF_SIMPLE_SUBPARAM(ULong, name, Tx, TimeToLive);
            success = success && nw_socketSetTimeToLive(result, TTLRequested);

            if (success) {
                if (!supportsControl) {
                        NW_REPORT_INFO_3(2, "Created sending socket \"%s\"for "
                                            "interface %s, port %u",
                                         name,
                                         addressStr, portNr);
                } else {
                        NW_REPORT_INFO_4(2, "Created sending socket \"%s\"for "
                                            "interface %s, ports %u and %u",
                                         name,
                                         addressStr,
                                         portNr, portNr+1);
                }
                NW_TRACE_1(Test, 1, "Creation of sending socket \"%s\" succeeded.",
                                  name);
            } else {
                if (!supportsControl) {
                        NW_REPORT_ERROR_3("socketNew", "Creation of sending socket \"%s\" failed for "
                                            "interface %s, port %u",
                                         name,
                                         addressStr, portNr);
                } else {
                        NW_REPORT_ERROR_4("socketNew", "Creation of sending socket \"%s\" failed for "
                                            "interface %s, ports %u and %u",
                                         name,
                                         addressStr,
                                         portNr, portNr+1);
                }
            }
        }

        switch (defaultAddressType) {
            case SK_TYPE_BROADCAST:
            case SK_TYPE_UNKNOWN:
            case SK_TYPE_UNICAST:
                nw_socketBroadcastInitialize(result, receiving);
                break;
            case SK_TYPE_MULTICAST:
                nw_socketMulticastInitialize(result, receiving,(os_sockaddr_storage*)&result->sockAddrData);
                break;
            default:
                break;
        }


    }
    nw_stringListFree(defaultAddressNameList);

    return result;
}


/* public */

nw_socket
nw_socketSendNew(
    const char *defaultAddress,
    sk_portNr portNr,
    sk_bool supportsControl,
    const char *pathName)
{
    return nw_socketNew(SK_FALSE, defaultAddress, portNr, supportsControl,
        pathName);
}


/* public */

nw_socket
nw_socketReceiveNew(
    const char *defaultAddress,
    sk_portNr portNr,
    sk_bool supportsControl,
    const char *pathName)
{
    return nw_socketNew(SK_TRUE, defaultAddress, portNr, supportsControl,
        pathName);
}


/* -------------------------------- destructor ------------------------------ */

void
nw_socketFree(
    nw_socket sock)
{
    os_result retVal;

    if (sock) {
        retVal = os_sockFree(sock->socketData);
        SK_REPORT_SOCKFUNC(2, retVal,
            "release socket resources", "close");
        if ((retVal == os_resultSuccess) && (sock->supportsControl)) {
            retVal = os_sockFree(sock->socketControl);
            SK_REPORT_SOCKFUNC(2, retVal,
                "release socket resources", "close");
        }
        os_free(sock->name);
        os_free(sock);
    }
    /* Not interested in any result */
    /* return result */
}


/* ------------------------------- public methods --------------------------- */

sk_length
nw_socketSendData(
    nw_socket sock,
    void *buffer,
    sk_length length)
{
    sk_length result = 0;
    os_int32 sendRes;
    os_int sendToSucceeded;

    NW_CONFIDENCE(sock != NULL);

    NW_HEXDUMP("nw_socketSendData", 0, buffer, length);
    /* Do the writing */
    NW_PROF_LAPSTART(SendTo);
    sendRes = os_sockSendto(sock->socketData, buffer, length,
                     (const struct sockaddr *)&sock->sockAddrData,
                     nw_socketAddressSize(sock->sockAddrData)
                     );
    NW_PROF_LAPSTOP(SendTo);

    if (sendRes > 0) {
        SK_REPORT_SOCKFUNC(6, os_resultSuccess,
                           "sending data to the socket", "sendto");
        sendToSucceeded = SK_TRUE;
    } else {
        SK_REPORT_SOCKFUNC(6, os_resultFail,
                           "sending data to the socket", "sendto");
        sendToSucceeded = SK_FALSE;
    }

    if (sendToSucceeded) {
        result = (sk_length)sendRes;
    }

    return result;
}


sk_length
nw_socketSendDataTo(
    nw_socket sock,
    os_sockaddr_storage receiverAddress,
    void *buffer,
    sk_length length)
{
    sk_length result = 0;
    os_int32 sendRes;
    os_int sendToSucceeded;
    os_sockaddr_storage sockAddrP2P;
    char addressStr[INET6_ADDRSTRLEN];

    NW_CONFIDENCE(sock != NULL);
    NW_CONFIDENCE(sizeof(receiverAddress) == sizeof(sockAddrP2P));

    NW_HEXDUMPTO("nw_socketSendDataTo", os_sockaddrAddressToString((os_sockaddr*) &receiverAddress,
                                                                    addressStr,
                                                                    sizeof(addressStr)),
                                        buffer,
                                        length);
    /* Do the writing */
    NW_PROF_LAPSTART(SendTo);
    sockAddrP2P = sock->sockAddrData;
    sendRes = os_sockSendto(sock->socketData, buffer, length,
                     (const struct sockaddr *)&sockAddrP2P,
                     nw_socketAddressSize(sockAddrP2P)
                     );
    NW_PROF_LAPSTOP(SendTo);

    if (sendRes > 0) {
        SK_REPORT_SOCKFUNC(6, os_resultSuccess,
                               "sending data to the socket", "sendto");
        sendToSucceeded = SK_TRUE;
    } else {
        SK_REPORT_SOCKFUNC(6, os_resultFail,
                               "sending data to the socket", "sendto");
        sendToSucceeded = SK_FALSE;
    }

    if (sendToSucceeded) {
        result = (sk_length)sendRes;
    }

    return result;
}

sk_length
nw_socketSendDataToPartition(
    nw_socket sock,
    sk_partitionId partitionId,
    plugSendStatistics pss,
    void *buffer,
    sk_length* length   /*in/out*/)
{
    sk_length result = 0;
    os_int32 sendRes = 0, sendResAll = 0;
    os_int sendToSucceeded;
    nw_addressList addressList = NULL;
    os_sockaddr_storage sockAddrForPartition;
    nw_bool found;
    sk_bool compression = FALSE;
    unsigned long zlen;
    unsigned char *zbuff;
    int zresult;
    char addressStr[INET6_ADDRSTRLEN];
    c_ulong mTTL;
    os_result setRes;

    NW_CONFIDENCE(sock != NULL);

    NW_HEXDUMP("nw_socketSendDataToPartition", partitionId, buffer, length);
    /* Do the writing */
    NW_PROF_LAPSTART(SendTo);

    found = nw_socketPartitionsLookup(sock->partitions, partitionId,
        &addressList, &compression, &mTTL);
    /* NW_CONFIDENCE(found); */

#ifndef OSPL_NO_ZLIB
    /* Compress the payload if so enabled, unless it's just little */

    if (*length > 255 && compression)
    {
        zlen = *length;
        zbuff = (unsigned char *)alloca (zlen);
        zresult = compress (zbuff, &zlen, buffer, *length);
        if (zresult == Z_OK)
        {
            if (pss != NULL)
            {
               pss->nofBytesBeforeCompression += *length;
               pss->nofBytesAfterCompression += zlen;
            }
            buffer = zbuff;
            *length = zlen;
        }
        else
        {
            /* Z_BUF_ERROR is expected if the data is uncompressable as we
               have only given a target buffer of the original data size */
            if (zresult != Z_BUF_ERROR)
            {
               NW_REPORT_WARNING_1("Sending packet", "Compression failed (error code %d), sending uncompressed.", zresult);
            }
        }
    }
#endif   /* OSPL_NO_ZLIB */

    sendResAll = 0;
    while (addressList) {
        sockAddrForPartition = nw_addressListGetAddress(addressList);

        addressList = nw_addressListGetNext(addressList);

        NW_TRACE_1(Test, 5, "SendTo expanded to %s",
                        os_sockaddrAddressToString((os_sockaddr*) &sockAddrForPartition,
                                                                    addressStr,
                                                                    sizeof(addressStr)));
        NW_STAMP(nw_plugDataBuffer(buffer),NW_BUF_TIMESTAMP_SEND);
        /* check if current socket has the right multicast TimetoLive value */
        if (sock->currentMulticastTTL != mTTL) {
            NW_TRACE_2(Test, 4, "socket: "
                       "Changed mTTL from %d to %d", sock->currentMulticastTTL,mTTL);
            setRes = nw_socketMulticastSetTTL(sock, mTTL);
            if (setRes == os_resultFail) {
                SK_REPORT_SOCKFUNC(1, os_resultFail,
                                "setting multicast TTL failed", "sendto");
            } else {
                sock->currentMulticastTTL = mTTL;
            }
        }

        sendRes = os_sockSendto(sock->socketData, buffer, *length,
          (const os_sockaddr *)&sockAddrForPartition,
          nw_socketAddressSize(sockAddrForPartition));
        NW_PROF_LAPSTOP(SendTo);
        if (sendRes > 0) {
            if (sendResAll >= 0 ) {
                sendResAll = sendRes;
            }
        } else {
            SK_REPORT_SOCKFUNC(1, os_resultFail,
                "sending data to the socket", "sendto");
            sendToSucceeded = SK_FALSE;
            sendResAll = sendRes;
        }
    }

    if (sendResAll > 0) {
        SK_REPORT_SOCKFUNC(5, os_resultSuccess,
             "sending data to the socket", "sendto");
        sendToSucceeded = SK_TRUE;
    }

    if (sendToSucceeded) {
        result = (sk_length)sendRes;
    }

    return result;
}


sk_length
nw_socketSendControl(
    nw_socket sock,
    void *buffer,
    sk_length length)
{
    sk_length result = 0;
    os_int32 sendRes;
    os_int sendToSucceeded;

    NW_CONFIDENCE(sock != NULL);

    /* Control message */
    NW_CONFIDENCE(sock->supportsControl);
    NW_HEXDUMP("nw_socketSendControl", 0, buffer, length);
    sendRes = os_sockSendto(sock->socketControl, buffer, length,
                     (const os_sockaddr *)&sock->sockAddrControl,
                     nw_socketAddressSize(sock->sockAddrControl)
                     );
    if (sendRes > 0) {
        SK_REPORT_SOCKFUNC(6, os_resultSuccess,
                       "sending control message to the socket", "sendto");
        sendToSucceeded = SK_TRUE;
    } else {
        SK_REPORT_SOCKFUNC(6, os_resultFail,
                       "sending control message to the socket", "sendto");
        sendToSucceeded = SK_FALSE;
    }

    if (sendToSucceeded) {
        result = (sk_length)sendRes;
    }

    return result;
}

sk_length
nw_socketSendControlTo(
    nw_socket sock,
    os_sockaddr_storage receiverAddress,
    void *buffer,
    sk_length length)
{
    sk_length result = 0;
    os_int32 sendRes;
    os_int sendToSucceeded;
    os_sockaddr_storage sockAddrP2P;
    char addressStr[INET6_ADDRSTRLEN];

    NW_CONFIDENCE(sock != NULL);

    /* Control message */
    NW_CONFIDENCE(sock->supportsControl);
    NW_HEXDUMPTO("nw_socketSendControlTo", os_sockaddrAddressToString((os_sockaddr*) &receiverAddress,
                                                                    addressStr,
                                                                    sizeof(addressStr)),
                                           buffer,
                                           length);
    sockAddrP2P = sock->sockAddrControl;
    if (receiverAddress.ss_family != sockAddrP2P.ss_family)
    {
        SK_REPORT_SOCKFUNC(6, os_resultFail,
                               "sending control message to the socket", "sendto");
        sendToSucceeded = SK_FALSE;
        return result;
    }
    else if (receiverAddress.ss_family == AF_INET)
    {
        ((os_sockaddr_in*)&sockAddrP2P)->sin_addr.s_addr = ((os_sockaddr_in*)&receiverAddress)->sin_addr.s_addr;
    }
    else
    {
        ((os_sockaddr_in6*)&sockAddrP2P)->sin6_addr = ((os_sockaddr_in6*)&receiverAddress)->sin6_addr;
    }
    sendRes = os_sockSendto(sock->socketControl, buffer, length,
                     (const os_sockaddr *)&sockAddrP2P,
                     nw_socketAddressSize(sockAddrP2P)
                     );

    if (sendRes > 0) {
        SK_REPORT_SOCKFUNC(6, os_resultSuccess,
                               "sending control message to the socket", "sendto");
        sendToSucceeded = SK_TRUE;
    } else {
        SK_REPORT_SOCKFUNC(6, os_resultFail,
                               "sending control message to the socket", "sendto");
        sendToSucceeded = SK_FALSE;
    }

    if (sendToSucceeded) {
        result = (sk_length)sendRes;
    }

    return result;
}


sk_length
nw_socketReceive(
    nw_socket sock,
    os_sockaddr_storage *senderAddress,
    void *vbuffer,
    sk_length length,
    os_time *timeOut,
    plugReceiveStatistics prs)
{
    unsigned char *buffer = vbuffer;
    sk_length result = 0;
    os_int32 recvRes = 0;
    os_int32 selectRes;
    os_int fromLen = (os_int)sizeof(os_sockaddr_storage);
    os_int ownMessage;
    nw_bool readDone = FALSE;
    os_time tmpTimeOut = *timeOut;
    unsigned char *zbuff;
    unsigned long zlen;
    int zresult;
#ifdef NW_DEBUGGING
    nw_bool control = FALSE;
    char addressStr[INET6_ADDRSTRLEN];
#endif

    NW_CONFIDENCE(sock != NULL);
    NW_CONFIDENCE(senderAddress != NULL);
    NW_CONFIDENCE(sock->socketData);
    NW_CONFIDENCE(!sock->supportsControl || sock->socketControl);

    memset(senderAddress, 0, sizeof(os_sockaddr_storage));
    FD_SET(sock->socketData, &sock->sockSet);
    if (sock->supportsControl) {
        FD_SET(sock->socketControl, &sock->sockSet);
    }

    /* Stop all profiling here because the select is a blocking call */
    /* Use tmpTimeOut becuase select modifies timeOut param under Linux */
    selectRes = os_sockSelect(sock->maxSockfd, &sock->sockSet, NULL, NULL, &tmpTimeOut);

    if (selectRes > 0) {
        if (sock->supportsControl) {
            if (FD_ISSET(sock->socketControl, &sock->sockSet)) {
                NW_CONFIDENCE(sock->supportsControl);
                recvRes = os_sockRecvfrom(sock->socketControl, buffer, length,
                                   (os_sockaddr *)senderAddress, (socklen_t *)&fromLen);
                readDone = TRUE;
#ifdef NW_DEBUGGING
                control = TRUE;
#endif
            }
        }
        if (readDone != TRUE) {
            NW_CONFIDENCE(FD_ISSET(sock->socketData, &sock->sockSet));
            recvRes = os_sockRecvfrom(sock->socketData, buffer, length,
                               (os_sockaddr *)senderAddress, (socklen_t *)&fromLen);
        }

#ifndef OSPL_NO_ZLIB

        if (recvRes > 0) {
            /* OSPL messages begin with 'S', unless they are compressed */
            if (buffer[0] != 'S') {
                zbuff = (unsigned char *)alloca (length);
                zlen = length;
                zresult = uncompress (zbuff, &zlen, buffer, recvRes);
                if (zresult == Z_OK) {
                    if (prs != NULL) {
                       prs->nofBytesBeforeDecompression += recvRes;
                       prs->nofBytesAfterDecompression += zlen;
                    }
                    memcpy (buffer, zbuff, zlen);
                    recvRes = zlen;
                } else {
                    /* It could be a secure packet or corrupt. Handle this in
                       the upper layer, but report on zlib errors */
                    if (zresult != Z_DATA_ERROR) {
                       /* Decompressor failed. Drop the packet. */
                       NW_REPORT_ERROR_1 ("Read from socket", "Decompression failed with code %d - dropping packet.", zresult);
                       recvRes = 0;
                    }
                }
            }
        }
#endif   /* OSPL_NO_ZLIB */

        if ((os_int)nw_configurationLoseReceivedMessage()) {
           recvRes = 0;
        }


        if (recvRes > 0) {
            if (sock->loopsback) {
#ifdef NW_LOOPBACK
                if (nw_configurationUseLoopback()) {
                    /* Loopback always simulates that data comes from the network */
                    SK_REPORT_SOCKFUNC(2, os_resultSuccess, /* TODO increase level */
                            "UseLoopback", "true");
                    ownMessage = SK_FALSE;
                } else {
                    SK_REPORT_SOCKFUNC(2, os_resultSuccess, /* TODO increase level */
                            "UseLoopback", "false");
                    ownMessage = os_sockaddrIPAddressEqual((os_sockaddr*)senderAddress,
                                                            (os_sockaddr*)&sock->sockAddrPrimary);
                }
#else
                ownMessage = os_sockaddrIPAddressEqual((os_sockaddr*)senderAddress,
                                                        (os_sockaddr*) &sock->sockAddrPrimary);
#endif
            } else {
                ownMessage = SK_FALSE;
            }
            if (!ownMessage) {
                result = (sk_length)recvRes;
#ifdef NW_DEBUGGING
                if (control) {
                    NW_HEXDUMPFROM("nw_socketReceiveControl", os_sockaddrAddressToString((os_sockaddr*) senderAddress,
                                                                    addressStr,
                                                                    sizeof(addressStr)), buffer, result);
                } else {
                    NW_HEXDUMPFROM("nw_socketReceiveData", os_sockaddrAddressToString((os_sockaddr*) senderAddress,
                                                                    addressStr,
                                                                    sizeof(addressStr)), buffer, result);
                }
#endif
                /* Resume profiling because we have actually received something
                 * relevant */

               SK_REPORT_SOCKFUNC(6, os_resultSuccess,
                              "receiving data from socket", "recvfrom");
            }
        } else {
           SK_REPORT_SOCKFUNC(6, os_resultFail,
                              "receiving data from socket", "recvfrom");
        }
    } else {

        if (selectRes < 0) {
            SK_REPORT_SOCKFUNC(6, os_resultFail,
                           "receiving data from socket", "select");
        }
    }
    return result;
}
