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
#include "nw_socketBroadcast.h"

/* Implementation */
#include "os_heap.h"
#include "os_socket.h"
#include "nw__confidence.h"
#include "nw__socket.h"
#include "nw_socketMisc.h"
#include "nw_report.h"
#include "nw_configuration.h"

#define NW_FULL_IP_ADDRESS            "255.255.255.255"

static os_int
nw_socketRetrieveBCInterface(
    const char *addressLookingFor,
    os_int sockfd,
    struct sockaddr_in *sockAddrPrimaryFound,
    struct sockaddr_in *sockAddrBroadcastFound)
{
    os_int result = SK_FALSE;
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
    success = sk_interfaceInfoRetrieveAllBC(&interfaceList, &nofInterfaces,
                                          sockfd);

    if (success && (nofInterfaces > 0U)) {
        /* Retrieve interface from custom settings */
        if (strncmp(addressLookingFor, NWCF_DEF(Interface),
                    (os_uint)sizeof(NWCF_DEF(Interface))) == 0) {
	  /* In case of no none loopback broadcast interfaces */
	  usedInterface = 0;

	  /* Try and find the first non loopback broadcast interface */
	  for ( i = 0; (i < nofInterfaces); i++ ) {
	    testAddr = (struct sockaddr_in *)
	      sk_interfaceInfoGetPrimaryAddress(interfaceList[i]);
	    if ( ntohl(*((os_uint32 *)&(testAddr->sin_addr))) != INADDR_LOOPBACK  ) {
	      usedInterface = i;
	      break;
	    }
	  }

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
                    /* IP address looking for matches with this interface's address */
                    usedInterface = i;
                    found = SK_TRUE;
                } else if (strncmp(addressLookingFor,
                        sk_interfaceInfoGetName(interfaceList[i]),
                        SK_INTF_MAX_NAME_LEN) == 0) {
                    /* Interface name looking for matches with this interface's name */
                    usedInterface = i;
                    found = SK_TRUE;
                } else {
                    i++;
                }
            }
            if (!found && !forced) {
                NW_REPORT_WARNING_2("retrieving broadcast interface",
                    "Requested interface %s not found or not broadcast enabled, "
                    "using %s instead",
                    addressLookingFor, NWCF_DEF(Interface));
                usedInterface = 0;
            }
        }
        /* if the Ethernet adapter that is configured is not available and forced is true report false */
        if (!found && forced) {
            result = SK_FALSE;
            NW_REPORT_WARNING_1("retrieving broadcast interface",
                               "Requested interface %s not found or not broadcast enabled",
                               addressLookingFor);
        } else {
            /* Store addresses found for later use */
            *sockAddrPrimaryFound =
                *(struct sockaddr_in *)sk_interfaceInfoGetPrimaryAddress(
                                                 interfaceList[usedInterface]);
            *sockAddrBroadcastFound =
                *(struct sockaddr_in *)sk_interfaceInfoGetBroadcastAddress(
                                                 interfaceList[usedInterface]);

            result = SK_TRUE;
        }

        /* Diagnostics */
        NW_TRACE_1(Configuration, 2, "Identified broadcast enabled interface %s ",
                   sk_interfaceInfoGetName(interfaceList[usedInterface]));

        /* Individual calls to inet_ntoa are required as a static buffer
           is used to hold to result */
        NW_TRACE_1(Configuration, 2, "primary address %s",
                   inet_ntoa(sockAddrPrimaryFound->sin_addr));

        NW_TRACE_1(Configuration, 2, "broadcast address %s",
                   inet_ntoa(sockAddrBroadcastFound->sin_addr));

        /* Free mem used */
        sk_interfaceInfoFreeAll(interfaceList, nofInterfaces);
    } else {
        NW_REPORT_ERROR("retrieving broadcast interface",
                        "No broadcast enabled interface found");
    }

    return result;
}

#undef NW_FULL_IP_ADDRESS

/**
* Calls nw_socketRetrieveBCInterface and caches the result unless this service
* is configured as an IPv6 service when it just returns SK_FALSE.
* @see nw_socketRetrieveBCInterface()
* @memberof nw_socket_s
*/
os_int
nw_socketGetDefaultBroadcastInterface(
    const char *addressLookingFor,
    os_int sockfd,
    os_sockaddr_storage *sockAddrPrimary,
    os_sockaddr_storage *sockAddrBroadcast)
{
    /* Evaluate the interfaces only once, after this use previous result */
    static os_int hadSuccessBefore = SK_FALSE;
    static os_sockaddr_storage sockAddrPrimaryFound;
    static os_sockaddr_storage sockAddrBroadcastFound;

    if (nw_configurationGetIsIPv6())
    {
        /* No broadcast only interfaces in IPv6 */
        return SK_FALSE;
    }
    else if (!hadSuccessBefore) {
        hadSuccessBefore = nw_socketRetrieveBCInterface(addressLookingFor, sockfd,
            (struct sockaddr_in*) &sockAddrPrimaryFound, (struct sockaddr_in*) &sockAddrBroadcastFound);
    }

    if (hadSuccessBefore) {
        *sockAddrPrimary = sockAddrPrimaryFound;
        *sockAddrBroadcast = sockAddrBroadcastFound;
    }

    return hadSuccessBefore;
}

void
nw_socketBroadcastInitialize(
    nw_socket socket,
    sk_bool receiving)
{
    /* scdds1980 dontroute not always true anymore
    if (! receiving) {
#if ! defined OS_VXWORKS_DEFS_H && ! defined INTEGRITY
       // Set option for avoiding routing to other interfaces
       nw_socketSetDontRouteOption(socket, SK_TRUE);
#endif

    }
    */
    nw_socketSetBroadcastOption(socket, SK_TRUE);
}

