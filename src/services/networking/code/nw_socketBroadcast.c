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

static int
nw_socketRetrieveBCInterface(
    const char *addressLookingFor,
    int sockfd,
    struct sockaddr_in *sockAddrPrimaryFound,
    struct sockaddr_in *sockAddrBroadcastFound)
{
    int result = SK_FALSE;
    int success;
    sk_interfaceInfo *interfaceList;
    unsigned int usedInterface = 0;
    unsigned int nofInterfaces;
    unsigned int i;
    char *addressDefault;
    int found = SK_FALSE;
    struct sockaddr_in *testAddr;

    success = sk_interfaceInfoRetrieveAllBC(&interfaceList, &nofInterfaces,
                                          sockfd);

    if (success && (nofInterfaces > 0U)) {
        /* Retrieve interface from custom settings */
        if (strncmp(addressLookingFor, NWCF_DEF(Interface),
                    (unsigned int)sizeof(NWCF_DEF(Interface))) == 0) {
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
                            (unsigned int)sizeof(NW_FULL_IP_ADDRESS)) == 0) {
                    usedInterface = i;
                    found = SK_TRUE;
                } else {
                    i++;
                }
            }
            if (!found) {
                NW_REPORT_WARNING_2("retrieving broadcast interface",
                    "Requested interface %s not found or not broadcast enabled, "
                    "using %s instead",
                    addressLookingFor, NWCF_DEF(Interface));
                usedInterface = 0;
            }
        }
        /* Store addresses found for later use */
        *sockAddrPrimaryFound =
            *(struct sockaddr_in *)sk_interfaceInfoGetPrimaryAddress(
                                             interfaceList[usedInterface]);
        fprintf (stderr, "sockAddrPrimaryFound = %s\n", inet_ntoa(sockAddrPrimaryFound->sin_addr));
        *sockAddrBroadcastFound =
            *(struct sockaddr_in *)sk_interfaceInfoGetBroadcastAddress(
                                             interfaceList[usedInterface]);
        result = SK_TRUE;

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


int
nw_socketGetDefaultBroadcastInterface(
    const char *addressLookingFor,
    int sockfd,
    struct sockaddr_in *sockAddrPrimary,
    struct sockaddr_in *sockAddrBroadcast)
{
    /* Evaluate the interfaces only once, after this use previous result */
    static int hadSuccessBefore = SK_FALSE;
    static struct sockaddr_in sockAddrPrimaryFound;
    static struct sockaddr_in sockAddrBroadcastFound;

    if (!hadSuccessBefore) {
        hadSuccessBefore = nw_socketRetrieveBCInterface(addressLookingFor, sockfd,
            &sockAddrPrimaryFound, &sockAddrBroadcastFound);
    }

    if (hadSuccessBefore) {
        *sockAddrPrimary = sockAddrPrimaryFound;
        *sockAddrBroadcast = sockAddrBroadcastFound;
    }

    return hadSuccessBefore;
}

void
nw_socketBroadcastInitialize(
    nw_socket socket)
{
    nw_socketSetBroadcastOption(socket, SK_TRUE);
}

