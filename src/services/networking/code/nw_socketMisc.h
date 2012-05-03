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

#ifndef NW_SOCKETMISC_H
#define NW_SOCKETMISC_H

#include "nw_socket.h"

sk_addressType
sk_getAddressType(
    const char *addressString);


typedef struct sk_interfaceInfo_s *sk_interfaceInfo;

#include <errno.h>
#include <string.h>
#include <os_socket.h>
#include <os_stdlib.h>
#include "nw_report.h"

#define SK_FALSE (0)
#define SK_TRUE  (!SK_FALSE)

extern os_sockErrno skLastSockError;

#define MAX_ERROR_BUFFER_SIZE     1024

/* Socketfunctions all return -1 on error */
#define SK_REPORT_SOCKFUNC(level, retval, context,function)          \
    if ((retval) != os_resultSuccess) { \
        char* errorString;                                    \
        os_sockErrno sockError = os_sockError();                             \
        errorString = os_sockErrnoToString(sockError); \
        if ( sockError != skLastSockError ){                                 \
            NW_REPORT_ERROR_3(context, "%s returned errno %d (%s)", function,  \
                              sockError, errorString);   \
            skLastSockError = sockError;   \
        } else {                                                            \
            NW_REPORT_INFO_4(level, "%s: %s returned errno %d (%s)", context, function,    \
                             sockError, errorString);                               \
        } \
        os_free(errorString);                                                                  \
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

/* Returned string never longer than SK_INTF_MAX_NAME_LEN */
#define SK_INTF_MAX_NAME_LEN (128)
const char *      sk_interfaceInfoGetName(
                      const sk_interfaceInfo interfaceInfo);

/* Currently not used
unsigned short    sk_interfaceInfoGetFlags(
                      const sk_interfaceInfo interfaceInfo);
*/

os_sockaddr_storage * sk_interfaceInfoGetPrimaryAddress(
                      const sk_interfaceInfo interfaceInfo);

os_sockaddr_storage * sk_interfaceInfoGetBroadcastAddress(
                      const sk_interfaceInfo interfaceInfo);

os_uint sk_interfaceInfoGetInterfaceIndexNo(sk_interfaceInfo this_);

void sk_interfaceInfoSetInterfaceIndexNo(sk_interfaceInfo this_, os_uint indexNo);

#endif /* NW_SOCKETMISC_H */

