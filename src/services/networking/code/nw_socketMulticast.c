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

#define NW_FULL_IP_ADDRESS            "255.255.255.255"

static os_int
nw_socketRetrieveMCInterface(
    const char *addressLookingFor,
    os_int sockfd,
    struct sockaddr_in *sockAddrPrimaryFound,
    struct sockaddr_in *sockAddrBroadcastFound)
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
    nw_bool forced = SK_FALSE;

    forced = NWCF_SIMPLE_ATTRIB(Bool,NWCF_ROOT(General) NWCF_SEP NWCF_NAME(Interface),forced);
    success = sk_interfaceInfoRetrieveAllMC(&interfaceList, &nofInterfaces,
                                          sockfd);

    if (success && (nofInterfaces > 0U)) {
        /* Retrieve interface from custom settings */
        if (strncmp(addressLookingFor, NWCF_DEF(Interface),
                    (os_uint)sizeof(NWCF_DEF(Interface))) == 0) {
            usedInterface = 0;

        } else {
             i = 0;
             while ((i < nofInterfaces) && !found) {
                testAddr = (struct sockaddr_in *)
                        sk_interfaceInfoGetPrimaryAddress(interfaceList[i]);
                if (i == 0U) {
                    addressDefault = inet_ntoa(testAddr->sin_addr);
                }
                if (strncmp(addressLookingFor,
                            inet_ntoa(testAddr->sin_addr),
                            (os_uint)sizeof(NW_FULL_IP_ADDRESS)) == 0) {
                    usedInterface = i;
                    found = SK_TRUE;
                } else if (strncmp(addressLookingFor,
                        sk_interfaceInfoGetName(interfaceList[i]),
                        SK_INTF_MAX_NAME_LEN) == 0) {
                    /* Interface name looking for matches with this interface's name */
                    usedInterface = i;
                    found = SK_TRUE;
                    NW_TRACE_1(Test, 3, "Multicast adres: %s  found in list",addressLookingFor);
                } else {
                    i++;
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
			/* Store addresses found for later use */
			*sockAddrPrimaryFound =
				*(struct sockaddr_in *)sk_interfaceInfoGetPrimaryAddress(
												 interfaceList[usedInterface]);
			*sockAddrBroadcastFound =
				*(struct sockaddr_in *)sk_interfaceInfoGetBroadcastAddress(
												 interfaceList[usedInterface]);

			/* Diagnostics */
			NW_TRACE_2(Configuration, 2, "Identified multicast enabled interface %s "
				"having primary address %s",
				sk_interfaceInfoGetName(interfaceList[usedInterface]),
				inet_ntoa(sockAddrPrimaryFound->sin_addr));
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
    struct sockaddr_in sockAddrInterface,
    struct sockaddr_in sockAddrMulticast)
{
    os_int result = SK_TRUE;
    struct ip_mreq mreq;
    os_result retVal;
    sk_bool res;
    os_socket dataSock;
    res = nw_socketGetDataSocket(socket, &dataSock);
    NW_CONFIDENCE(res);

    mreq.imr_interface = sockAddrInterface.sin_addr;
    mreq.imr_multiaddr = sockAddrMulticast.sin_addr;

    retVal = os_sockSetsockopt(dataSock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
        &mreq, sizeof(mreq));
    SK_REPORT_SOCKFUNC(2, retVal,
                       "join multicast group", "setsockopt");
    NW_TRACE_1(Configuration, 3, "Joined multicast group with address %s",
        inet_ntoa(sockAddrMulticast.sin_addr));
    if (retVal != os_resultSuccess) {
        result = SK_FALSE;
    }
    /* Control data currently sent P2P only, so no need to join multicast group */

    return result;
}

static os_int
nw_socketMulticastSetInterface(
    nw_socket socket,
    struct sockaddr_in sockAddrInterface)
{
    os_int result = SK_TRUE;
    os_result retVal;
    sk_bool res;
    os_socket dataSock;

    res = nw_socketGetDataSocket(socket, &dataSock);
    NW_CONFIDENCE(res);
    retVal = os_sockSetsockopt(dataSock, IPPROTO_IP, IP_MULTICAST_IF,
        &(sockAddrInterface.sin_addr), sizeof(sockAddrInterface.sin_addr));
    SK_REPORT_SOCKFUNC(2, retVal,
                       "set outgoing multicast interface", "setsockopt");
    NW_TRACE_1(Configuration, 3, "Set outgoing multicast interface to %s",
        inet_ntoa(sockAddrInterface.sin_addr));
    if (retVal != os_resultSuccess) {
        result = SK_FALSE;
    }

    return result;
}

os_int
nw_socketMulticastSetTTL(
    nw_socket socket,
    c_ulong timeToLive)
{
    os_int result = SK_TRUE;
    os_result retVal = os_resultFail;
    sk_bool hasDataSocket;
    sk_bool hasControlSocket;
    os_socket dataSocket;
    os_socket controlSocket;

    hasDataSocket = nw_socketGetDataSocket(socket, &dataSocket);
    NW_CONFIDENCE(hasDataSocket);

    if (hasDataSocket) {
        retVal = os_sockSetsockopt(dataSocket, IPPROTO_IP, IP_MULTICAST_TTL,
                                   &timeToLive, sizeof(timeToLive));
        SK_REPORT_SOCKFUNC(2, retVal,
                           "setting multicast timetolive option", "setsockopt");
    }

    hasControlSocket = nw_socketGetControlSocket(socket, &controlSocket);
    if ((retVal == os_resultSuccess) && hasControlSocket) {
        retVal = os_sockSetsockopt(controlSocket, IPPROTO_IP, IP_MULTICAST_TTL,
                                   &timeToLive, sizeof(timeToLive));
        SK_REPORT_SOCKFUNC(2, retVal,
                           "setting multicast timetolive option", "setsockopt");
    }

    if (retVal != os_resultSuccess) {
           result = SK_FALSE;
    }

    return result;
}


static os_int
nw_socketMulticastSetOptions(
    nw_socket socket,
    os_int loopsback)
{
    os_int result = SK_TRUE;
    os_result retVal = os_resultFail;
    sk_bool hasDataSocket;
    sk_bool hasControlSocket;
    os_socket dataSocket;
    os_socket controlSocket;

    hasDataSocket = nw_socketGetDataSocket(socket, &dataSocket);
    NW_CONFIDENCE(hasDataSocket);
    if (hasDataSocket) {
        retVal = os_sockSetsockopt(dataSocket, IPPROTO_IP, IP_MULTICAST_LOOP,
                                   &loopsback, sizeof(loopsback));
        SK_REPORT_SOCKFUNC(2, retVal,
                           "setting multicast loopback option", "setsockopt");
    }

    hasControlSocket = nw_socketGetControlSocket(socket, &controlSocket);
    if ((retVal == os_resultSuccess) && hasControlSocket) {
        retVal = os_sockSetsockopt(controlSocket, IPPROTO_IP, IP_MULTICAST_LOOP,
                                   &loopsback, sizeof(loopsback));
        SK_REPORT_SOCKFUNC(2, retVal,
                           "setting multicast loopback option", "setsockopt");
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

os_int
nw_socketGetDefaultMulticastInterface(
    const char *addressLookingFor,
    os_socket socket,
    struct sockaddr_in *sockAddrPrimary,
    struct sockaddr_in *sockAddrBroadcast)
{
    /* Evaluate the interfaces only once, after this use previous result */
    static os_int hadSuccessBefore = SK_FALSE;
    static struct sockaddr_in sockAddrPrimaryFound;
    static struct sockaddr_in sockAddrBroadcastFound;

    if (!hadSuccessBefore) {
        hadSuccessBefore = nw_socketRetrieveMCInterface(addressLookingFor, socket,
            &sockAddrPrimaryFound, &sockAddrBroadcastFound);
    }

    if (hadSuccessBefore) {
        *sockAddrPrimary = sockAddrPrimaryFound;
        *sockAddrBroadcast = sockAddrBroadcastFound;
    }

    return hadSuccessBefore;
}


void
nw_socketMulticastInitialize(
    nw_socket sock,
    sk_bool receiving,
    sk_address address)
{
    struct sockaddr_in sockAddrPrimary;
    struct sockaddr_in sockAddrMulticast;
    c_ulong timeToLive;
    os_result setRes;


    timeToLive = NWCF_DEF(MulticastTimeToLive);

#ifdef OS_SOCKET_BIND_FOR_MULTICAST
    if (receiving) {
        /* Join multicast group, for receiving socket only */
        sockAddrPrimary.sin_addr.s_addr = nw_socketPrimaryAddress(sock);
        sockAddrMulticast.sin_addr.s_addr = address;
        nw_socketMulticastJoinGroup(sock, sockAddrPrimary, sockAddrMulticast);
    } else {
        nw_socketBind(sock);
    }
    /* Do not set outgoing multicast interface, doesn't work yet on windows */
#else
    /* Join multicast group and set options */
    sockAddrPrimary.sin_addr.s_addr = nw_socketPrimaryAddress(sock);
    sockAddrMulticast.sin_addr.s_addr = address;
    if (receiving) {
        nw_socketMulticastJoinGroup(sock, sockAddrPrimary, sockAddrMulticast);
    }
    nw_socketMulticastSetInterface(sock, sockAddrPrimary);
#endif

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
    c_ulong mTTL)
{
    struct sockaddr_in sockAddrPrimary;
    struct sockaddr_in sockAddrMulticast;
    sk_address address;
    sk_bool initialized = FALSE;
    os_result setRes;
    if (!nw_socketGetMulticastSupported(sock)) {
    	NW_REPORT_ERROR("nw_socketMulticastAddPartition", "Partition uses multicast but multicast is not enabled");
    }
    if (sk_getAddressType(addressString) == SK_TYPE_MULTICAST) {

        sockAddrPrimary.sin_addr.s_addr = nw_socketPrimaryAddress(sock);
        address = sk_stringToAddress(addressString, NULL);

        if (!nw_socketGetMulticastInitialized(sock)) {
            nw_socketMulticastInitialize(sock, receiving,address);
            initialized = TRUE;
        }
        if (address) {
            sockAddrMulticast.sin_addr.s_addr = address;


        /* Join multicast group, for receiving socket only */
#ifdef OS_SOCKET_BIND_FOR_MULTICAST
        /* socket already joined a multicast group if nw_socketGetMulticastInitialized is true*/
        if (receiving && !initialized) {
                nw_socketMulticastJoinGroup(sock, sockAddrPrimary, sockAddrMulticast);
        }
#else
        if (receiving && !initialized) {
            nw_socketMulticastJoinGroup(sock, sockAddrPrimary, sockAddrMulticast);
        }
#endif
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
