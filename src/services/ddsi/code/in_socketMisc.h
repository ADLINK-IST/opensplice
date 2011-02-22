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
#ifndef IN_SOCKETMISC_H
#define IN_SOCKETMISC_H

#include "in_socket.h"
#include "in_address.h"



in_addressType
in_getAddressType(
    const char *addressString);


typedef struct in_interfaceInfo_s *in_interfaceInfo;

#include <errno.h>
#include <string.h>
#include "os_socket.h"
#include "in_report.h"

extern os_sockErrno inLastSockError;

#define MAX_ERROR_BUFFER_SIZE     1024

/* Socketfunctions all return -1 on error */
#define IN_REPORT_SOCKFUNC(level, retval, context,function)          \
    if ((retval) != os_resultSuccess) {                                    \
        os_sockErrno sockError = os_sockError();                             \
        char buf[MAX_ERROR_BUFFER_SIZE];                                                    \
        buf[0] = '\0';                                                      \
        os_strerror(sockError,buf,MAX_ERROR_BUFFER_SIZE);                    \
        if ( sockError != inLastSockError ){                                 \
            IN_REPORT_ERROR_3(context, "%s returned errno %d (%s)", function,  \
                              sockError, buf);   \
            inLastSockError = sockError;   \
        } else {                                                            \
            IN_REPORT_INFO_4(level, "%s: %s returned errno %d (%s)", context, function,    \
                             sockError, buf);                               \
        }                                                                  \
    } else {                                                               \
        IN_REPORT_INFO_2(level, "%s: %s succeeded", context, function);    \
    }

os_result               in_interfaceInfoRetrieveAllBC(
                      in_interfaceInfo **interfaceList /* [nofInterfaces] */,
                      os_uint *nofInterfaces,
                      int sockfd);

os_result               in_interfaceInfoRetrieveAllMC(
                      in_interfaceInfo **interfaceList /* [nofInterfaces] */,
                      os_uint *nofInterfaces,
                      int sockfd);

os_result               in_interfaceInfoRetrieveAllLoopback(
                      in_interfaceInfo **interfaceList /* [nofInterfaces] */,
                      os_uint *nofInterfaces,
                      int sockfd);

void              in_interfaceInfoFreeAll(
                      in_interfaceInfo *interfaceList /* [nofInterfaces] */,
                      os_uint nofInterfaces);

char *            in_interfaceInfoGetName(
                      const in_interfaceInfo interfaceInfo);

/* Currently not used
unsigned short    in_interfaceInfoGetFlags(
                      const in_interfaceInfo interfaceInfo);
*/

struct sockaddr * in_interfaceInfoGetPrimaryAddress(
                      const in_interfaceInfo interfaceInfo);

struct sockaddr * in_interfaceInfoGetBroadcastAddress(
                      const in_interfaceInfo interfaceInfo);

#endif /* IN_SOCKETMISC_H */

