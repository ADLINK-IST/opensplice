/* Interface */
#include "in_socketBroadcast.h"

/* Implementation */
#include <assert.h>

#include "os_heap.h"
#include "os_socket.h"
#include "in__socket.h"
#include "in_socketMisc.h"
#include "in_report.h"
#include "in_configuration.h"

#define IN_FULL_IP_ADDRESS            "255.255.255.255"

static os_boolean
in_socketRetrieveBCInterface(
    const char *addressLookingFor,
    os_socket sockfd,
    struct sockaddr_in *sockAddrPrimaryFound,
    struct sockaddr_in *sockAddrBroadcastFound)
{
    os_boolean result = OS_FALSE;
    os_boolean success;
    in_interfaceInfo *interfaceList;
    unsigned int usedInterface = 0;
    unsigned int nofInterfaces;
    unsigned int i;
    char *addressDefault;
    int found = FALSE;
    struct sockaddr_in *testAddr;

    success = in_interfaceInfoRetrieveAllBC(&interfaceList, &nofInterfaces,
                                          sockfd);

    if (success && (nofInterfaces > 0U)) {
        /* Retrieve interface from custom settings */
        if (strncmp(addressLookingFor, INCF_DEF(Interface),
                    (unsigned int)sizeof(INCF_DEF(Interface))) == 0) {
            usedInterface = 0;
        } else {
             i = 0;
             while ((i < nofInterfaces) && !found) {
                testAddr = (struct sockaddr_in *)
                        in_interfaceInfoGetPrimaryAddress(interfaceList[i]);
                if (i == 0U) {
                    addressDefault = inet_ntoa(testAddr->sin_addr);
                }
                if (strncmp(addressLookingFor,
                            inet_ntoa(testAddr->sin_addr),
                            (unsigned int)sizeof(IN_FULL_IP_ADDRESS)) == 0) {
                    usedInterface = i;
                    found = OS_TRUE;
                } else {
                    i++;
                }
            }
            if (!found) {
                IN_REPORT_WARNING_2("retrieving broadcast interface",
                    "Requested interface %s not found or not broadcast enabled, "
                    "using %s instead",
                    addressLookingFor, INCF_DEF(Interface));
                usedInterface = 0;
            }
        }
        /* Store addresses found for later use */
        *sockAddrPrimaryFound =
            *(struct sockaddr_in *)in_interfaceInfoGetPrimaryAddress(
                                             interfaceList[usedInterface]);
        *sockAddrBroadcastFound =
            *(struct sockaddr_in *)in_interfaceInfoGetBroadcastAddress(
                                             interfaceList[usedInterface]);
        result = TRUE;

        /* Diagnostics */
        IN_TRACE_3(Configuration, 2, "Identified broadcast enabled interface %s "
            "having primary address %s and broadcast address %s",
            in_interfaceInfoGetName(interfaceList[usedInterface]),
            inet_ntoa(sockAddrPrimaryFound->sin_addr),
            inet_ntoa(sockAddrBroadcastFound->sin_addr));

        /* Free mem used */
        in_interfaceInfoFreeAll(interfaceList, nofInterfaces);
    } else {
        IN_REPORT_ERROR("retrieving broadcast interface",
                        "No broadcast enabled interface found");
    }

    return result;
}

#undef IN_FULL_IP_ADDRESS


os_boolean
in_socketGetDefaultBroadcastInterface(
    const char *addressLookingFor,
    os_socket sockfd,
    struct sockaddr_in *sockAddrPrimary,
    struct sockaddr_in *sockAddrBroadcast)
{
	/* Note: previous version did store the result in static function-local variables.
	 * This caching has been removed for two reasons: 1) static variables have not been
	 * protected against concurrent write, 2) the caching made it almost impossible
	 * to test the operation with different arguments.
	 * */
	os_boolean result = in_socketRetrieveBCInterface(addressLookingFor, sockfd,
			sockAddrPrimary, sockAddrBroadcast);

    return result;
}

void
in_socketBroadcastInitialize(
    in_socket socket)
{
    in_socketSetBroadcastOption(socket, OS_TRUE);
}

