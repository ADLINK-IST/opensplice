/* Interface */
#include "in_socketMulticast.h"

/* Implementation */
#include <assert.h>
#include "os_heap.h"
#include "os_socket.h"
#include "in__socket.h"
#include "in_socket.h"
#include "in_socketMisc.h"
#include "in_report.h"
#include "in_configuration.h"

#define IN_FULL_IP_ADDRESS            "255.255.255.255"

static int
in_socketRetrieveMCInterface(
    const char *addressLookingFor,
    int sockfd,
    struct sockaddr_in *sockAddrPrimaryFound,
    struct sockaddr_in *sockAddrBroadcastFound)
{
    int result = FALSE;
    int success;
    in_interfaceInfo *interfaceList;
    unsigned int usedInterface = 0;
    unsigned int nofInterfaces;
    unsigned int i;
    char *addressDefault;
    int found = FALSE;
    struct sockaddr_in *testAddr;

    success = in_interfaceInfoRetrieveAllMC(&interfaceList, &nofInterfaces,
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
                    addressDefault = inet_ntoa(testAddr->sin_addr); /* TODO, critical will be overwritten on next call of ntoa */
                }
                if (strncmp(addressLookingFor,
                            inet_ntoa(testAddr->sin_addr),
                            (unsigned int)sizeof(IN_FULL_IP_ADDRESS)) == 0) {
                    usedInterface = i;
                    found = TRUE;
                } else {
                    i++;
                }
            }
            if (!found) {
                IN_REPORT_WARNING_2("retrieving multicast interface",
                    "Requested interface %s not found or not multicast enabled, "
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
        IN_TRACE_2(Configuration, 2, "Identified multicast enabled interface %s "
            "having primary address %s",
            in_interfaceInfoGetName(interfaceList[usedInterface]),
            inet_ntoa(sockAddrPrimaryFound->sin_addr));

        /* Free mem used */
        in_interfaceInfoFreeAll(interfaceList, nofInterfaces);
    } else {
        IN_REPORT_ERROR("retrieving multicast interface",
                        "No multicast enabled interface found");
    }

    return result;
}

#undef IN_FULL_IP_ADDRESS



static int
in_socketMulticastJoinGroup(
    in_socket socket,
    struct sockaddr_in sockAddrInterface,
    struct sockaddr_in sockAddrMulticast)
{
    int result = TRUE;
    struct ip_mreq mreq;
    os_result retVal;
    os_boolean res;
    os_boolean hasControlSocket;
    os_socket dataSock;
    os_socket controlSock;

    mreq.imr_interface = sockAddrInterface.sin_addr;
    mreq.imr_multiaddr = sockAddrMulticast.sin_addr;

    res = in_socketGetDataSocket(socket, &dataSock);
    assert(res);
    retVal = os_sockSetsockopt(dataSock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
        &mreq, sizeof(mreq));
    IN_REPORT_SOCKFUNC(2, retVal,
                       "join multicast group", "setsockopt");
    IN_TRACE_1(Configuration, 3, "Joined multicast group with address %s",
        inet_ntoa(sockAddrMulticast.sin_addr)); /* TODO abstract from IPv4 */

    hasControlSocket = in_socketGetControlSocket(socket, &controlSock);
    if ((retVal == os_resultSuccess) && hasControlSocket) {
        retVal = os_sockSetsockopt(dataSock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
            &mreq, sizeof(mreq));
        IN_REPORT_SOCKFUNC(2, retVal,
                           "join multicast group", "setsockopt");
        IN_TRACE_1(Configuration, 3, "Joined multicast group with address %s",
            inet_ntoa(sockAddrMulticast.sin_addr)); /* TODO abstract from IPv4 */
    }
    if (retVal != os_resultSuccess) {
        result = FALSE;
    }

    /* Control data currently sent P2P only, so no need to join multicast group */

    return result;
}

static int
in_socketMulticastSetInterface(
    in_socket socket,
    struct sockaddr_in sockAddrInterface)
{
    int result = TRUE;
    os_result retVal;
    os_boolean res;
    os_socket dataSock;

    res = in_socketGetDataSocket(socket, &dataSock);
    assert(res);
    retVal = os_sockSetsockopt(dataSock, IPPROTO_IP, IP_MULTICAST_IF,
        &(sockAddrInterface.sin_addr), sizeof(sockAddrInterface.sin_addr));
    IN_REPORT_SOCKFUNC(2, retVal,
                       "set outgoing multicast interface", "setsockopt");
    IN_TRACE_1(Configuration, 3, "Set outgoing multicast interface to %s",
        inet_ntoa(sockAddrInterface.sin_addr));
    if (retVal != os_resultSuccess) {
        result = FALSE;
    }

    return result;
}


static int
in_socketMulticastSetOptions(
    in_socket socket,
    int loopsback,
    unsigned int timeToLive)
{
    int result = TRUE;
    os_result retVal = os_resultFail;
    unsigned char flag;
    os_boolean hasDataSocket;
    os_boolean hasControlSocket;
    os_socket dataSocket;
    os_socket controlSocket;

    hasDataSocket = in_socketGetDataSocket(socket, &dataSocket);
    assert(hasDataSocket);
    if (hasDataSocket) {
        flag = (unsigned int)loopsback;
        retVal = os_sockSetsockopt(dataSocket, IPPROTO_IP, IP_MULTICAST_LOOP,
            &flag, sizeof(flag));
        IN_REPORT_SOCKFUNC(2, retVal,
                           "setting multicast loopback option", "setsockopt");
    }

    if ((retVal == os_resultSuccess) && hasDataSocket) {
        flag = timeToLive;
        retVal = os_sockSetsockopt(dataSocket, IPPROTO_IP, IP_MULTICAST_TTL,
            &flag, sizeof(flag));
        IN_REPORT_SOCKFUNC(2, retVal,
                           "setting multicast timetolive option", "setsockopt");
    }

    hasControlSocket = in_socketGetControlSocket(socket, &controlSocket);
    if ((retVal == os_resultSuccess) && hasControlSocket) {
        flag = (unsigned int)loopsback;
        retVal = os_sockSetsockopt(controlSocket, IPPROTO_IP, IP_MULTICAST_LOOP,
            &flag, sizeof(flag));
        IN_REPORT_SOCKFUNC(2, retVal,
                           "setting multicast loopback option", "setsockopt");
    }

    if ((retVal == os_resultSuccess) && hasControlSocket) {
        flag = timeToLive;
        retVal = os_sockSetsockopt(controlSocket, IPPROTO_IP, IP_MULTICAST_TTL,
            &flag, sizeof(flag));
        IN_REPORT_SOCKFUNC(2, retVal,
                           "setting multicast timetolive option", "setsockopt");
    }

    if (retVal != os_resultSuccess) {
        result = FALSE;
    }

    return result;
}


/* ------------------------------- public ----------------------------------- */

os_boolean
in_socketGetDefaultMulticastInterface(
    const char *addressLookingFor,
    os_socket socket,
    struct sockaddr_in *sockAddrPrimary,
    struct sockaddr_in *sockAddrBroadcast)
{
	/* Note: previous version did store the result in static function-local variables.
	 * This caching has been removed for two reasons: 1) static variables have not been
	 * protected against concurrent write, 2) the caching made it almost impossible
	 * to test the operation with different arguments.
	 * */

	os_boolean result =
		in_socketRetrieveMCInterface(addressLookingFor, socket,
            sockAddrPrimary, sockAddrBroadcast);

    return result;
}


void
in_socketMulticastInitialize(
    in_socket sock)
{
    struct sockaddr_in sockAddrPrimary; /* just IPv4  */
    struct sockaddr_in sockAddrMulticast; /* just IPv4  */
    c_ulong timeToLive;

    /*
    timeToLive = INCF_SIMPLE_PARAM(ULong, INCF_ROOT(General), TimeToLive);
    if (timeToLive < INCF_MIN(TimeToLive)) {
        IN_REPORT_WARNING_2("in_socketMulticastNew",
            "specified TimeToLive %d too small, "
            "switching to %d",
            timeToLive, INCF_MIN(TimeToLive));
        timeToLive = INCF_MIN(TimeToLive);
    }
*/
    timeToLive = INCF_DEF(TimeToLive);

    /* TODO, just IPv4 */
    /* Join multicast group and set options */
    sockAddrPrimary.sin_addr.s_addr = /* IPv4 workarround */
    	*((os_uint32*) in_addressIPv4Ptr(in_socketPrimaryAddress(sock)));
    sockAddrMulticast.sin_addr.s_addr = /* IPv4 workarround */
    	*((os_uint32*) in_addressIPv4Ptr(in_socketMulticastAddress(sock)));
    in_socketMulticastJoinGroup(sock, sockAddrPrimary, sockAddrMulticast);
    in_socketMulticastSetInterface(sock, sockAddrPrimary);
    in_socketMulticastSetOptions(sock, in_socketLoopsback(sock), timeToLive);
    /* Multicast socket should be capable of sending broadcast messages as well */
    in_socketSetBroadcastOption(sock, OS_TRUE);
}


/* TODO replace the IPv4 patches */
void
in_socketMulticastAddPartition(
    in_socket sock,
    const char *addressString)
{
    struct sockaddr_in sockAddrPrimary;
    struct sockaddr_in sockAddrMulticast;
    OS_STRUCT(in_address) address;

    if (in_getAddressType(addressString) == IN_ADDRESS_TYPE_MULTICAST) {
        sockAddrPrimary.sin_addr.s_addr =  /* IPv4 workarround */
        	(os_uint32) in_addressIPv4Ptr(in_socketPrimaryAddress(sock));
        in_addressInitFromStringWithDefault(&address,
        		addressString,
        		INCF_DEF(Address));
        sockAddrMulticast.sin_addr.s_addr = /* IPv4 workarround */
        	(os_uint32) in_addressIPv4Ptr(&address);
        in_socketMulticastJoinGroup(sock, sockAddrPrimary, sockAddrMulticast);
    }
}
