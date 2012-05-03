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
#include "nw_socketMulticast.h"

/* Implementation */
#include "os_heap.h"
#include "os_socket.h"
#include "os_stdlib.h"
#include "nw__socket.h"
#include "nw_socket.h"
#include "nw_socketMisc.h"
#include "nw__confidence.h"
#include "nw_report.h"
#include "nw_configuration.h"

#include "os_defs.h"

typedef unsigned long long my_foo;

#define NW_FULL_IP_ADDRESS            "255.255.255.255"

/**
* Gets a list of all Multicast enabled Interfaces and interates over them looking for a match
* for the requested network interface.
*/
static os_int
nw_socketRetrieveMCInterface(
    nw_socket this_,
    const char *addressLookingFor,
    os_int sockfd,
    os_sockaddr_storage *sockAddrPrimaryFound,
    os_sockaddr_storage *sockAddrBroadcastFound)
{
    os_int result = SK_TRUE;
    os_int success;
    sk_interfaceInfo *interfaceList;
    os_uint usedInterface = 0;
    os_uint nofInterfaces;
    os_uint i;
    char *addressDefault;
    os_int found = SK_FALSE;
    struct sockaddr_in *testAddr;
    os_sockaddr_storage ipv6SockAddressLookingFor;
    nw_bool forced = SK_FALSE;
    char addressStr[INET6_ADDRSTRLEN];

    forced = NWCF_SIMPLE_ATTRIB(Bool,NWCF_ROOT(General) NWCF_SEP NWCF_NAME(Interface),forced);
    success = sk_interfaceInfoRetrieveAllMC(&interfaceList, &nofInterfaces,
                                          sockfd);

    NW_TRACE_1(Configuration, 6,
                       "nw_socketRetrieveMCInterface: Retrieved %d multicast interfaces",
                       nofInterfaces);

    if (success && (nofInterfaces > 0U)) {
        /* Retrieve interface from custom settings */
        if (strncmp(addressLookingFor, NWCF_DEF(Interface),
                    (os_uint)sizeof(NWCF_DEF(Interface))) == 0) {
            /* If first available is requested use 0'th */
            usedInterface = 0;
        } else {
             success = os_sockaddrStringToAddress(addressLookingFor,
                                                    (os_sockaddr*) &ipv6SockAddressLookingFor,
                                                    !nw_configurationGetIsIPv6());
             i = 0;
             while ((i < nofInterfaces) && !found) {
                if (nw_configurationGetIsIPv6())
                {
                    if (success && os_sockaddrIPAddressEqual((os_sockaddr*) &ipv6SockAddressLookingFor,
                                                (os_sockaddr*) sk_interfaceInfoGetPrimaryAddress(interfaceList[i])))
                    {
                        usedInterface = i;
                        found = SK_TRUE;
                    }
                    else if (strncmp(addressLookingFor,
                                    sk_interfaceInfoGetName(interfaceList[i]),
                                    SK_INTF_MAX_NAME_LEN) == 0)
                    {
                        /* Interface name specified that matches this interface's name */
                        usedInterface = i;
                        found = SK_TRUE;
                        /* @todo Why is this level Test ?? */
                        NW_TRACE_1(Configuration, 3, "Interface name: %s  found in list",addressLookingFor);
                    }
                    else
                    {
                        i++;
                    }
                }
                else
                {
                    testAddr = (struct sockaddr_in *)
                            sk_interfaceInfoGetPrimaryAddress(interfaceList[i]);
                    if (i == 0U) {
                        /* Take the '1st' as a fall back default */
                        /* @todo - Why ?? This is not used. */
                        addressDefault = inet_ntoa(testAddr->sin_addr);
                    }
                    if (strncmp(addressLookingFor,
                                inet_ntoa(testAddr->sin_addr),
                                (os_uint)sizeof(NW_FULL_IP_ADDRESS)) == 0) {
                        /* IP address specified that matches this interface address */
                        usedInterface = i;
                        found = SK_TRUE;
                    } else if (strncmp(addressLookingFor,
                            sk_interfaceInfoGetName(interfaceList[i]),
                            SK_INTF_MAX_NAME_LEN) == 0) {
                        /* Interface name specified that matches this interface's name */
                        usedInterface = i;
                        found = SK_TRUE;
                        /* @todo Below trace makes no real sense. This is a multicast enabled interface
                        It is not a 'multicast address' */
                        NW_TRACE_1(Test, 3, "Multicast adress: %s  found in list",addressLookingFor);
                    } else {
                        i++;
                    }
                }
            }
            if (!found && !forced) {
                NW_REPORT_WARNING_2("retrieving multicast interface",
                    "Requested interface %s not found or not multicast enabled, "
                    "using %s instead",
                    addressLookingFor, NWCF_DEF(Interface));
                usedInterface = 0;
                result = SK_TRUE;
            }
            if (!found && forced) {
                result = SK_FALSE;
                NW_REPORT_WARNING_1("retrieving multicast interface",
                                   "Requested interface %s not found or not multicast enabled",
                                   addressLookingFor);
            }
        }
        if (result) {
            os_sockaddr_storage* tmpAddress;
            /* Store addresses found for later use */
            *sockAddrPrimaryFound = *sk_interfaceInfoGetPrimaryAddress(
                                                 interfaceList[usedInterface]);
            tmpAddress = sk_interfaceInfoGetBroadcastAddress(
                                                 interfaceList[usedInterface]);
            if (tmpAddress)
                *sockAddrBroadcastFound = *tmpAddress;

            nw_socketSetInterfaceIndexNo(this_,
                sk_interfaceInfoGetInterfaceIndexNo(interfaceList[usedInterface]));

            /* Diagnostics */
            NW_TRACE_3(Configuration, 2, "Identified multicast enabled interface #%u (%s)"
                "having primary address %s",
                nw_socketGetInterfaceIndexNo(this_),
                sk_interfaceInfoGetName(interfaceList[usedInterface]),
                os_sockaddrAddressToString((os_sockaddr*) sockAddrPrimaryFound,
                                                                    addressStr,
                                                                    sizeof(addressStr)));
        }
        /* Free mem used */
        sk_interfaceInfoFreeAll(interfaceList, nofInterfaces);

    } else {
        result = SK_FALSE;
    }

    return result;
}

#undef NW_FULL_IP_ADDRESS



static os_int
nw_socketMulticastJoinGroup(
    nw_socket socket,
    os_sockaddr_storage sockAddrInterface,
    os_sockaddr_storage sockAddrMulticast)
{
    os_int result = SK_TRUE;
    /* ip_mreq is IP V4 only */
    struct ip_mreq mreq;
    /* This is the Ipv6 version */
    os_ipv6_mreq ipv6mreq;
    os_result retVal;
    sk_bool res;
    os_socket dataSock;
    char addressStr[INET6_ADDRSTRLEN];

    res = nw_socketGetDataSocket(socket, &dataSock);

    NW_CONFIDENCE(res);

    if (sockAddrInterface.ss_family == AF_INET)
    {
        /* IPv4 code */
        mreq.imr_interface = ((os_sockaddr_in*)&sockAddrInterface)->sin_addr;
        mreq.imr_multiaddr = ((os_sockaddr_in*)&sockAddrMulticast)->sin_addr;

        retVal = os_sockSetsockopt(dataSock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
            &mreq, sizeof(mreq));
        SK_REPORT_SOCKFUNC(2, retVal,
                           "join multicast group", "setsockopt");
        NW_TRACE_1(Configuration, 3, "Joined multicast group with address %s",
            inet_ntoa(((os_sockaddr_in*)&sockAddrMulticast)->sin_addr));
        if (retVal != os_resultSuccess) {
            result = SK_FALSE;
        }
        /* Control data currently sent P2P only, so no need to join multicast group */
    }
    else
    {
        /* IPv6 */
        memset (&ipv6mreq, 0, sizeof(ipv6mreq));
        memcpy (&ipv6mreq.ipv6mr_multiaddr,
                  &((os_sockaddr_in6 *)&sockAddrMulticast)->sin6_addr,
                  sizeof(ipv6mreq.ipv6mr_multiaddr));
        ipv6mreq.ipv6mr_interface = nw_socketGetInterfaceIndexNo(socket);

        retVal = os_sockSetsockopt(dataSock,IPPROTO_IPV6, IPV6_JOIN_GROUP,
                                           &ipv6mreq,
                                           sizeof(ipv6mreq));
        SK_REPORT_SOCKFUNC(2, retVal,
                           "join IPv6 multicast group", "setsockopt");
        if (retVal != os_resultSuccess) {
            NW_TRACE_2(Configuration, 3, "Failed to joined IPv6 multicast group with address %s using if #%d",
                os_sockaddrAddressToString((os_sockaddr*) &sockAddrMulticast,
                                                       addressStr,
                                                       sizeof(addressStr)),
                ipv6mreq.ipv6mr_interface);
            result = SK_FALSE;
        }
        else
        {
            NW_TRACE_2(Configuration, 3, "Joined IPv6 multicast group with address %s using if #%d",
                os_sockaddrAddressToString((os_sockaddr*) &sockAddrMulticast,
                                                       addressStr,
                                                       sizeof(addressStr)),
                ipv6mreq.ipv6mr_interface);
        }
    }

    return result;
}

static os_int
nw_socketMulticastSetInterface(
    nw_socket socket,
    os_sockaddr_storage sockAddrInterface)
{
    os_int result = SK_TRUE;
    os_result retVal;
    sk_bool res;
    os_socket dataSock;
    char addressStr[INET6_ADDRSTRLEN];
    os_uint interfaceNo = 0;

    res = nw_socketGetDataSocket(socket, &dataSock);
    NW_CONFIDENCE(res);

    if (nw_configurationGetIsIPv6())
    {
         /* IPv6 */
        interfaceNo = nw_socketGetInterfaceIndexNo(socket);
        retVal = os_sockSetsockopt(dataSock,IPPROTO_IPV6, IPV6_MULTICAST_IF,
                                           &interfaceNo,
                                           sizeof(interfaceNo));
        SK_REPORT_SOCKFUNC(2, retVal,
                           "nw_socketMulticastSetInterface: Setting IPv6 mcast IF", "setsockopt");
    }
    else
    {
        os_sockaddr_in* ipv4Address = (os_sockaddr_in*) & sockAddrInterface;
        retVal = os_sockSetsockopt(dataSock, IPPROTO_IP, IP_MULTICAST_IF,
                    &(ipv4Address->sin_addr), sizeof(ipv4Address->sin_addr));
        SK_REPORT_SOCKFUNC(2, retVal,
                       "nw_socketMulticastSetInterface: setting outgoing multicast interface", "setsockopt");
    }
    if (retVal != os_resultSuccess) {
        result = SK_FALSE;
        NW_TRACE_2(Configuration, 3, "Setting outgoing multicast interface to %s, interface # (%u) failed",
            os_sockaddrAddressToString((os_sockaddr*) &sockAddrInterface,
                                    addressStr,
                                    sizeof(addressStr)),
            interfaceNo);
    }
    else
    {
        NW_TRACE_2(Configuration, 3, "Setting outgoing multicast interface to %s, interface # (%u) succeeded",
            os_sockaddrAddressToString((os_sockaddr*) &sockAddrInterface,
                                    addressStr,
                                    sizeof(addressStr)),
            interfaceNo);
    }

    return result;
}

os_int
nw_socketMulticastSetTTL(
    nw_socket socket,
    os_uchar timeToLive)
{
    os_int result = SK_TRUE;
    os_result retVal = os_resultFail;
    os_uint ipv6Flag;
    sk_bool hasDataSocket;
    sk_bool hasControlSocket;
    os_socket dataSocket;
    os_socket controlSocket;

    hasDataSocket = nw_socketGetDataSocket(socket, &dataSocket);
    NW_CONFIDENCE(hasDataSocket);
    if (hasDataSocket) {
        ipv6Flag = timeToLive;
        if (nw_configurationGetIsIPv6())
        {
            retVal = os_sockSetsockopt(dataSocket, IPPROTO_IPV6, IPV6_UNICAST_HOPS,
                &ipv6Flag, sizeof(ipv6Flag));
            SK_REPORT_SOCKFUNC(2, retVal,
                               "setting multicast IPv6 hop limit option on dataSocket", "setsockopt");
        }
        else
        {
            retVal = os_sockSetsockopt(dataSocket, IPPROTO_IP, IP_MULTICAST_TTL,
                &timeToLive, sizeof(timeToLive));
            SK_REPORT_SOCKFUNC(2, retVal,
                           "setting multicast timetolive option", "setsockopt");
        }
    }

    hasControlSocket = nw_socketGetControlSocket(socket, &controlSocket);
    if ((retVal == os_resultSuccess) && hasControlSocket) {
        ipv6Flag = timeToLive;
        if (nw_configurationGetIsIPv6())
        {
            retVal = os_sockSetsockopt(controlSocket, IPPROTO_IPV6, IPV6_UNICAST_HOPS,
                &ipv6Flag, sizeof(ipv6Flag));
            SK_REPORT_SOCKFUNC(2, retVal,
                               "setting multicast IPv6 hop limit option on control socket", "setsockopt");
        }
        else
        {
            retVal = os_sockSetsockopt(controlSocket, IPPROTO_IP, IP_MULTICAST_TTL,
                &timeToLive, sizeof(timeToLive));
            SK_REPORT_SOCKFUNC(2, retVal,
                           "setting multicast timetolive option", "setsockopt");
        }
    }

    if (retVal != os_resultSuccess) {
           result = SK_FALSE;
    }

    return result;
}


static os_int
nw_socketMulticastSetOptions(
    nw_socket socket,
    os_uchar loopsback)
{
    os_int result = SK_TRUE;
    os_result retVal = os_resultFail;
    sk_bool hasDataSocket;
    sk_bool hasControlSocket;
    os_socket dataSocket;
    os_socket controlSocket;
    os_uint ipv6Flag;

    hasDataSocket = nw_socketGetDataSocket(socket, &dataSocket);
    NW_CONFIDENCE(hasDataSocket);
    if (hasDataSocket) {
        if (nw_configurationGetIsIPv6())
        {
            ipv6Flag = (os_uint)loopsback;
            retVal = os_sockSetsockopt(dataSocket, IPPROTO_IPV6, IPV6_MULTICAST_LOOP,
                &ipv6Flag, sizeof(ipv6Flag));
            SK_REPORT_SOCKFUNC(2, retVal,
                               "setting dataSocket IPv6 multicast loopback option", "setsockopt");
        }
        else
        {
            retVal = os_sockSetsockopt(dataSocket, IPPROTO_IP, IP_MULTICAST_LOOP,
                &loopsback, sizeof(loopsback));
            SK_REPORT_SOCKFUNC(2, retVal,
                               "setting dataSocket multicast loopback option", "setsockopt");
        }
    }

    hasControlSocket = nw_socketGetControlSocket(socket, &controlSocket);
    if ((retVal == os_resultSuccess) && hasControlSocket) {
        if (nw_configurationGetIsIPv6())
        {
            ipv6Flag = (os_uint)loopsback;
            retVal = os_sockSetsockopt(controlSocket, IPPROTO_IPV6, IPV6_MULTICAST_LOOP,
                &ipv6Flag, sizeof(ipv6Flag));
            SK_REPORT_SOCKFUNC(2, retVal,
                               "setting controlSocket IPv6 multicast loopback option", "setsockopt");
        }
        else
        {
            retVal = os_sockSetsockopt(controlSocket, IPPROTO_IP, IP_MULTICAST_LOOP,
                &loopsback, sizeof(loopsback));
            SK_REPORT_SOCKFUNC(2, retVal,
                               "setting controlSocket multicast loopback option", "setsockopt");
        }
    }

    if (retVal != os_resultSuccess) {
        result = SK_FALSE;
    }

    if (result) {
        nw_socketSetMulticastInitialized(socket,SK_TRUE);
    }

    return result;
}


/* ------------------------------- public ----------------------------------- */

/**
* Calls nw_socketRetrieveMCInterface() and caches the result.
* @param this_ The this pointer for the nw_socket that we are initialising.
* @see nw_socketRetrieveMCInterface()
*/
os_int
nw_socketGetDefaultMulticastInterface(
    nw_socket this_,
    const char *addressLookingFor,
    os_socket socket,
    os_sockaddr_storage* sockAddrPrimary,
    os_sockaddr_storage* sockAddrBroadcast)
{
    /* Evaluate the interfaces only once, after this use previous result */
    /* @todo This has to all go. It's just a big bag of wrong */
    static os_int hadSuccessBefore = SK_FALSE;
    static os_sockaddr_storage sockAddrPrimaryFound;
    static os_sockaddr_storage sockAddrBroadcastFound;
    static os_uint interfaceIndexNo = 0;

    if (!hadSuccessBefore) {
        NW_TRACE(Configuration, 6,
                       "nw_socketGetDefaultMulticastInterface: Retrieving default MC interface.");
        hadSuccessBefore = nw_socketRetrieveMCInterface(this_, addressLookingFor, socket,
            &sockAddrPrimaryFound, &sockAddrBroadcastFound);
        if (hadSuccessBefore)
            interfaceIndexNo = nw_socketGetInterfaceIndexNo(this_);
    }

    if (hadSuccessBefore) {
        *sockAddrPrimary = sockAddrPrimaryFound;
        *sockAddrBroadcast = sockAddrBroadcastFound;
        nw_socketSetInterfaceIndexNo(this_, interfaceIndexNo);
    }

    return hadSuccessBefore;
}

/**
* @param address The multicast group address
*/
void
nw_socketMulticastInitialize(
    nw_socket sock,
    sk_bool receiving,
    os_sockaddr_storage* address)
{
    os_sockaddr_storage sockAddrPrimary;
    os_sockaddr_storage sockAddrMulticast;
    os_uchar timeToLive;
    os_result setRes;

    timeToLive = NWCF_DEF(MulticastTimeToLive);
    sockAddrPrimary = nw_socketPrimaryAddress(sock);

    NW_CONFIDENCE(sockAddrPrimary.ss_family);
    NW_CONFIDENCE(sockAddrPrimary.ss_family == AF_INET || sockAddrPrimary.ss_family == AF_INET6);

    if (receiving) {
        /* Join multicast group, for receiving socket only */
        sockAddrMulticast = *address;
        nw_socketMulticastJoinGroup(sock, sockAddrPrimary, sockAddrMulticast);
    }
#ifdef OS_SOCKET_BIND_FOR_MULTICAST
    else
    {
        nw_socketBind(sock);
    }
#endif
    nw_socketMulticastSetInterface(sock, sockAddrPrimary);

    setRes = nw_socketMulticastSetTTL(sock, timeToLive);
    if (setRes == os_resultFail) {
        SK_REPORT_SOCKFUNC(1, os_resultFail,
                        "setting multicast TTL failed", "MulticastInitialize");
    }
    nw_socketMulticastSetOptions(sock, nw_socketLoopsback(sock));
    /* Multicast socket should be capable of sending broadcast messages as well */
    nw_socketSetBroadcastOption(sock, SK_TRUE);
}

void
nw_socketMulticastAddPartition(
    nw_socket sock,
    const char *addressString,
    sk_bool receiving,
    os_uchar mTTL)
{
    os_sockaddr_storage sockAddrPrimary;
    os_sockaddr_storage sockAddrMulticast;
    os_sockaddr_storage address;
    c_ulong timeToLive;
    sk_bool initialized = FALSE;
    os_boolean success;
    os_result setRes;

    if (!nw_socketGetMulticastSupported(sock)) {
        NW_REPORT_ERROR("nw_socketMulticastAddPartition", "Partition uses multicast but multicast is not enabled");
    }
    if (sk_getAddressType(addressString) == SK_TYPE_MULTICAST) {

        sockAddrPrimary = nw_socketPrimaryAddress(sock);
        success = os_sockaddrStringToAddress(addressString,
                                            (os_sockaddr*) &address,
                                            !nw_configurationGetIsIPv6());

        if (!nw_socketGetMulticastInitialized(sock)) {
            nw_socketMulticastInitialize(sock, receiving, &address);
            initialized = TRUE;
        }
        if (success) {
            sockAddrMulticast = address;

        /* Join multicast group, for receiving socket only */
        /* socket already joined a multicast group if nw_socketGetMulticastInitialized is true*/
        if (receiving && !initialized) {
            nw_socketMulticastJoinGroup(sock, sockAddrPrimary, sockAddrMulticast);
        }
        setRes = nw_socketMulticastSetTTL(sock, mTTL);
        if (setRes == os_resultFail) {
            SK_REPORT_SOCKFUNC(1, os_resultFail,
                            "setting multicast TTL failed", "MulticastAddPartition");
        }
        nw_socketMulticastSetOptions(sock,nw_socketLoopsback(sock));
        } else {
            NW_TRACE_1(Test, 4, "nw_socketMulticastAddPartition: "
                                "Ignored invalid multicast addess %s", addressString);
        }

    }
}
