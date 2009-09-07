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
#include "nw_socketLoopback.h"

/* Implementation */
#include <string.h>       /* for memcpy                     */
#include "os_heap.h"
#include "os_socket.h"
#include "nw_commonTypes.h"
#include "nw__socket.h"
#include "nw__confidence.h"
#include "nw_socketMisc.h"
#include "nw_report.h"

os_int
nw_socketGetDefaultLoopbackAddress(
    os_int sockfd,
    struct sockaddr_in *sockAddr)
{
    /* Statics for storing the result */
    static struct sockaddr_in sockAddrFound;
    static os_int hadSuccessBefore = SK_FALSE;
    /* Normal variables for retrieving the result */
    os_int success;
    sk_interfaceInfo *interfaceList;
    os_uint nofInterfaces;
    
    if (!hadSuccessBefore) {
         success = sk_interfaceInfoRetrieveAllLoopback(&interfaceList, &nofInterfaces, 
                                              sockfd);
         if (success) {
             sockAddrFound = 
                 *(struct sockaddr_in *)sk_interfaceInfoGetPrimaryAddress(
                                          interfaceList[0]);
             hadSuccessBefore = SK_TRUE;
             
             /* Diagnostics */
             NW_TRACE_2(Configuration, 2, "Identified loopback enabled interface %s "
                 "having primary address %s",
                 sk_interfaceInfoGetName(interfaceList[0]),
                 inet_ntoa(sockAddrFound.sin_addr));
                                       
             /* Free mem used */
             sk_interfaceInfoFreeAll(interfaceList, nofInterfaces);
         }
    }

    if (hadSuccessBefore) {
        *sockAddr = sockAddrFound;
    }
    
    return hadSuccessBefore;
}
