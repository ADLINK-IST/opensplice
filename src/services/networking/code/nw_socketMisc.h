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

#ifndef NW_SOCKETMISC_H
#define NW_SOCKETMISC_H

#include "nw_socket.h"

sk_address
sk_stringToAddress(
    const char *addressString,
    const char *addressOnError);


sk_addressType
sk_getAddressType(
    const char *addressString);


typedef struct sk_interfaceInfo_s *sk_interfaceInfo;

#include <errno.h>
#include <string.h>
#include <os_socket.h>
#include "nw_report.h"

#define SK_FALSE (0)
#define SK_TRUE  (!SK_FALSE)

extern os_sockErrno skLastSockError; 

/* Socketfunctions all return -1 on error */
#define SK_REPORT_SOCKFUNC(level, retval, context,function)          \
    if ((retval) != os_resultSuccess) {                                    \
        os_sockErrno sockError = os_sockError();                             \
        if ( sockError != skLastSockError ){                                 \
            NW_REPORT_ERROR_3(context, "%s returned errno %d (%s)", function,  \
                              sockError, strerror(sockError));   \
            skLastSockError = sockError;   \
        } else {                                                            \
            NW_REPORT_INFO_4(level, "%s: %s returned errno %d (%s)", context, function,    \
                             sockError, strerror(sockError));                               \
        }                                                                  \
    } else {                                                               \
        NW_REPORT_INFO_2(level, "%s: %s succeeded", context, function);    \
    }

os_int               sk_interfaceInfoRetrieveAllBC(
                      sk_interfaceInfo **interfaceList /* [nofInterfaces] */,
                      os_uint *nofInterfaces,
                      os_int sockfd);
                      
os_int               sk_interfaceInfoRetrieveAllMC(
                      sk_interfaceInfo **interfaceList /* [nofInterfaces] */,
                      os_uint *nofInterfaces,
                      os_int sockfd);
                      
os_int               sk_interfaceInfoRetrieveAllLoopback(
                      sk_interfaceInfo **interfaceList /* [nofInterfaces] */,
                      os_uint *nofInterfaces,
                      os_int sockfd);
                      
void              sk_interfaceInfoFreeAll(                      
                      sk_interfaceInfo *interfaceList /* [nofInterfaces] */,
                      os_uint nofInterfaces);

char *            sk_interfaceInfoGetName(
                      const sk_interfaceInfo interfaceInfo);
                     
/* Currently not used
unsigned short    sk_interfaceInfoGetFlags(
                      const sk_interfaceInfo interfaceInfo);
*/
                     
struct sockaddr * sk_interfaceInfoGetPrimaryAddress(
                      const sk_interfaceInfo interfaceInfo);
                     
struct sockaddr * sk_interfaceInfoGetBroadcastAddress(
                      const sk_interfaceInfo interfaceInfo);
                       
#endif /* NW_SOCKETMISC_H */

