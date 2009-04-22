/* Interface */
#include "in_socketLoopback.h"

/* Implementation */
#include <assert.h>
#include <string.h>       /* for memcpy                     */
#include "os_heap.h"
#include "os_socket.h"
#include "in_commonTypes.h"
#include "in__socket.h"
#include "in_socketMisc.h"
#include "in_report.h"

os_boolean
in_socketGetDefaultLoopbackAddress(
    os_socket sockfd,
    struct sockaddr_in *sockAddr)
{
	/* Note: previous version did store the result in static function-local variables.
	 * This caching has been removed for two reasons: 1) static variables have not been
	 * protected against concurrent write, 2) the caching made it almost impossible
	 * to test the operation with different arguments.
	 * */

    /* Normal variables for retrieving the result */
    os_boolean success;
    in_interfaceInfo *interfaceList;
    unsigned int nofInterfaces;

	success = in_interfaceInfoRetrieveAllLoopback(&interfaceList, &nofInterfaces,
										  sockfd);
	if (success) {
		*sockAddr =
			*(struct sockaddr_in *)in_interfaceInfoGetPrimaryAddress(
					interfaceList[0]);

		/* Diagnostics */
		IN_TRACE_2(Configuration, 2, "Identified loopback enabled interface %s "
			 "having primary address %s",
			 in_interfaceInfoGetName(interfaceList[0]),
			 inet_ntoa(sockAddr->sin_addr));

		 /* Free mem used */
		in_interfaceInfoFreeAll(interfaceList, nofInterfaces);
	 }

    return success;
}
