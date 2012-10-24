/* Interface */
#include "in_socket.h"
#include "in__socket.h"

/* Implementation */
#include <string.h>       /* for memcmp and memset          */
#include <assert.h>
#include "os_heap.h"
#include "os_socket.h"
#include <errno.h>
/* Descendants */
#include "in_socketBroadcast.h"
#include "in_socketMulticast.h"
#include "in_socketLoopback.h"

#include "in_socketMisc.h"
#include "in_socketPartitions.h"
#include "in_configuration.h"
#include "in_profiling.h"
#include "in_misc.h" /* for in_stringDup and in_dumpToString */
#include "in_report.h"
#include "in__locator.h"
#include "in_address.h"


#define IN_CONTROLPORT(portNr) ((portNr)+1)

/* ------------------------------ Debug function ---------------------------- */
#if 1
#define IN_HEXDUMP(message, partitionId, data, length)    \
    {                                        \
        char *str;                           \
        str = in_dumpToString(data, length); \
        IN_TRACE_3(Test, 6, "\n%s\npartitionId = %u\nLength = %u\n", message, partitionId, length); \
        IN_TRACE_1(Test, 6, "%s\n\n", str); \
        os_free(str);                        \
    }
#else
/* #define IN_HEXDUMP(message, data, length) */
#define IN_HEXDUMP(message, partitionId, data, length)    \
        IN_TRACE_3(Test, 6, "%s: %u bytes to partitionId %u\n", message, length, partitionId);
/* #define IN_HEXDUMP(message, data, length)     \
  if ((length != 56) && (length != 248))         \
  {                                              \
      printf("%s: %u bytes\n", message, length); \
  }
*/
#endif

/* ------------------------------- main class ------------------------------- */

/* The structure of the data, to be extended by descendants */

OS_STRUCT(in_socket) {
    OS_EXTENDS(in_object);

    char *name;
    /* Data socket, for sending and receiving data */
    os_socket socketData;
    /* Primary address, for destination checking */
    OS_STRUCT(in_address) sockAddrPrimary;
    /* Broadcast address corresponding to this interface */
    OS_STRUCT(in_address) sockAddrBroadcast;
    /* Control address corresponding to this interface */
    OS_STRUCT(in_address) sockAddrMulti;
    /* Control address corresponding to this interface */
    /* struct sockaddr_in sockAddrControl; */
    /* Default address to send data to */
    in_locator dataUnicastLocator;
    in_locator dataMulticastLocator;

    /* Parameters idenfitifying socket for control messages (acks etc) */
    os_boolean supportsControl;
    os_socket socketControl;
    /* defined in case of "supportsControl==OS_TRUE" */
    in_locator controlUnicastLocator;
    in_locator controlMulticastLocator;

    /* Caching for select statement */
    fd_set sockSet;
    os_int maxSockfd;
    /* Socket state and settings */
    os_boolean loopsback;
    /* List of alternative addresses for sending data */
    /* in_socketPartitions partitions; */
};



/* ---------------------------- getters/setters ----------------------------- */

os_boolean
in_socketLoopsback(
    in_socket sock)
{
    return sock->loopsback;
}


in_address
in_socketPrimaryAddress(
    in_socket sock)
{
    in_address result = NULL;

    assert(sizeof(sock->sockAddrPrimary) == OS_SIZEOF(in_address));

    if (sock) {
        result = &(sock->sockAddrPrimary);
    }

    return result;
}

in_address
in_socketMulticastAddress(
    in_socket sock)
{
    in_address result = NULL;

    assert(sizeof(sock->sockAddrMulti) == OS_SIZEOF(in_address));

    if (sock) {
        result = &(sock->sockAddrMulti);
    }

    return result;
}

in_address
in_socketBroadcastAddress(
    in_socket sock)
{
    in_address result = NULL;

    assert(sizeof(sock->sockAddrBroadcast) == OS_SIZEOF(in_address));

    if (sock) {
        result = &(sock->sockAddrBroadcast);
    }

    return result;
}

in_locator
in_socketGetUnicastDataLocator(
    in_socket sock)
{
    in_locator result = NULL;

    if (sock) {
        result = in_locatorKeep(sock->dataUnicastLocator);
    }

    return result;
}

in_locator
in_socketGetMulticastDataLocator(
    in_socket sock)
{
    in_locator result = NULL;

    if (sock) {
        result = in_locatorKeep(sock->dataMulticastLocator);
    }

    return result;
}

in_locator
in_socketGetUnicastControlLocator(
    in_socket sock)
{
    in_locator result = NULL;

    if (sock && sock->supportsControl) {
        result = in_locatorKeep(sock->controlUnicastLocator);
    }

    return result;
}


in_locator
in_socketGetMulticastControlLocator(
    in_socket sock)
{
    in_locator result = NULL;

    if (sock && sock->supportsControl) {
        result = in_locatorKeep(sock->controlMulticastLocator);
    }

    return result;
}

static os_boolean
in_socketSetSendBufferSize(
    in_socket sock,
    os_int bufSize)
{
    os_boolean result = OS_TRUE;
    socklen_t optLen;
    os_result retVal;

    if (sock != NULL) {
        /* Set sendbuffer option */
        optLen = (os_uint)sizeof(bufSize);

        retVal = os_sockSetsockopt(sock->socketData,
                            SOL_SOCKET, SO_SNDBUF,
                            (void *)&bufSize, optLen);
        IN_REPORT_SOCKFUNC(2, retVal,
                           "set socket sendbuffer size", "setsockopt");

        if ((retVal == os_resultSuccess) && (sock->supportsControl)) {
            retVal = os_sockSetsockopt(sock->socketControl,
                                SOL_SOCKET, SO_SNDBUF,
                                (void *)&bufSize, optLen);
            IN_REPORT_SOCKFUNC(2, retVal,
                               "set socket sendbuffer size", "setsockopt");
        }
        if (retVal != os_resultSuccess) {
            result = OS_FALSE;
        }
    } else {
        result = OS_FALSE;
    }

    return result;
}


static os_boolean
in_socketSetReceiveBufferSize(
    in_socket sock,
    os_int bufSize)
{
    os_boolean result = OS_TRUE;
    socklen_t optLen;
    os_int actualSize;
    os_result retVal;

    if (sock != NULL) {
        /* Set receivebuffer option */
        optLen = (socklen_t)sizeof(bufSize);
        retVal = os_sockSetsockopt(sock->socketData,
                            SOL_SOCKET, SO_RCVBUF,
                            (const void *)&bufSize, optLen);
        IN_REPORT_SOCKFUNC(2, retVal,
                           "set socket receivebuffer size", "setsockopt");

        if (retVal == os_resultSuccess) {
            /* The following lines are for tracing purposes only */
            actualSize = 0;
            retVal = os_sockGetsockopt(sock->socketData, SOL_SOCKET, SO_RCVBUF,
                (void *)&actualSize, &optLen);
            IN_REPORT_SOCKFUNC(4, retVal,
                               "get socket receivebuffer size", "getsockopt");
            IN_TRACE_2(Receive, 5, "Receive buffer size set. Requested: %d, actual: %d",
                bufSize, actualSize);

            if ((retVal == os_resultSuccess) && (sock->supportsControl)) {
                retVal = os_sockSetsockopt(sock->socketControl,
                                    SOL_SOCKET, SO_RCVBUF,
                                    (const void *)&bufSize, optLen);
                IN_REPORT_SOCKFUNC(2, retVal,
                                   "set socket receivebuffer size", "setsockopt");
            }
        }
        if (retVal != os_resultSuccess) {
            result = OS_FALSE;
        }
    } else {
        result = OS_FALSE;
    }

    return result;
}


os_boolean
in_socketSetBroadcastOption(
    in_socket sock,
    os_boolean enableBroadcast)
{
    os_boolean result = OS_TRUE;
    socklen_t optLen;
    os_result retVal;

    if (sock != NULL) {
        /* Set broadcast option */
        optLen = (socklen_t)sizeof(enableBroadcast);
        retVal = os_sockSetsockopt(sock->socketData,
                            SOL_SOCKET, SO_BROADCAST,
                            (const void *)&enableBroadcast, optLen);
        IN_REPORT_SOCKFUNC(2, retVal,
                           "set socket broadcast option", "setsockopt");
        if ((retVal == os_resultSuccess) && (sock->supportsControl)) {
            retVal = os_sockSetsockopt(sock->socketControl,
                                SOL_SOCKET, SO_BROADCAST,
                                (const void *)&enableBroadcast, optLen);
            IN_REPORT_SOCKFUNC(2, retVal,
                               "set socket broadcast option", "setsockopt");
        }
        if (retVal != os_resultSuccess) {
            result = OS_FALSE;
        }
    } else {
        result = OS_FALSE;
    }

    return result;
}

static os_boolean
in_socketSetTOS(
    in_socket sock,
    os_int tos)
{
    os_boolean result = OS_TRUE;
    socklen_t optLen;
    os_result retVal;

    if (sock != NULL) {
        /* Set broadcast option */
        optLen = (socklen_t)sizeof(tos);
        retVal = os_sockSetsockopt(sock->socketData,
                            IPPROTO_IP, IP_TOS,
                            (const void *)&tos, optLen);
        IN_REPORT_SOCKFUNC(2, retVal,
                           "set socket type of service", "setsockopt");
        if ((retVal == os_resultSuccess) && (sock->supportsControl)) {
            retVal = os_sockSetsockopt(sock->socketControl,
                                IPPROTO_IP, IP_TOS,
                                (const void *)&tos, optLen);
            IN_REPORT_SOCKFUNC(2, retVal,
                               "set socket type of service", "setsockopt");
        }
        if (retVal != os_resultSuccess) {
            result = OS_FALSE;
        }
    } else {
        result = OS_FALSE;
    }

    return result;
}


static os_boolean
in_socketSetDontRouteOption(
    in_socket sock,
    os_boolean disableRouting)
{
    os_boolean result = OS_TRUE;
    socklen_t optLen;
    os_result retVal;

    if (sock != NULL) {
        /* Set broadcast option */
        optLen = (socklen_t)sizeof(disableRouting);
        retVal = os_sockSetsockopt(sock->socketData,
                            SOL_SOCKET, SO_DONTROUTE,
                            (const void *)&disableRouting, optLen);
        IN_REPORT_SOCKFUNC(2, retVal,
                           "set socket dontroute option", "setsockopt");
        if ((retVal == os_resultSuccess) && (sock->supportsControl)){
            retVal = os_sockSetsockopt(sock->socketControl,
                                SOL_SOCKET, SO_DONTROUTE,
                                (const void *)&disableRouting, optLen);
            IN_REPORT_SOCKFUNC(2, retVal,
                               "set socket dontroute option", "setsockopt");
        }
        if (retVal != os_resultSuccess) {
            result = OS_FALSE;
        }
    } else {
        result = OS_FALSE;
    }

    return result;
}

os_boolean
in_socketGetDataSocket(
    in_socket sock,
    os_socket *socket)
{
    os_boolean result = OS_FALSE;

    assert(sock != NULL);
    assert(socket != NULL);

    if ((sock != NULL) && (socket != NULL)) {
        *socket = sock->socketData;
        result = OS_TRUE;
    }

    return result;
}

os_boolean
in_socketGetControlSocket(
    in_socket sock,
    os_socket *socket)
{
    os_boolean result = OS_FALSE;

    assert(sock != NULL);
    assert(socket != NULL);

    if ((sock != NULL) && (socket != NULL)) {
        result = sock->supportsControl;
        if (result) {
             *socket = sock->socketControl;
        }
    }

    return result;
}

os_boolean
in_socketBind(
    in_socket sock)
{
    os_boolean result = OS_TRUE;
    os_result retVal = os_resultSuccess;
    os_int optVal = 1;
    socklen_t optLen;
    struct sockaddr_storage bindAddress; /* IPv4 & IPv6 */

    if (sock != NULL) {
        /* Avoid already in use error messages */
        optLen = (os_uint)sizeof(optVal);
        retVal = os_sockSetsockopt(sock->socketData,
                            SOL_SOCKET, SO_REUSEADDR,
                            (void *)&optVal, optLen);
        IN_REPORT_SOCKFUNC(2, retVal,
                           "set socket reuse option", "setsockopt");

        /* Transform to "struct sockaddr" format of same kind (v4 or v6)
         * but with unspecified, local IP address */
		in_locatorToSockaddrForAnyAddress(sock->dataUnicastLocator,
				(struct sockaddr*)&bindAddress);

        /* bindAddress = sock->sockAddrPrimary;
        bindAddress.sin_addr.s_addr = htonl(INADDR_ANY); */
        if (retVal == os_resultSuccess) {
            retVal = os_sockBind(sock->socketData,
                (const struct sockaddr *)&bindAddress,
                /* (socklen_t)sizeof(bindAddress)); */
                /* the size must be precise on solaris, otherwise EINV */
				(socklen_t)sizeof(struct sockaddr_in)); /* FIXME encapsulate the struct-size */ 
            IN_REPORT_SOCKFUNC(2, retVal,
                           "bind socket", "bind");

            if ((retVal == os_resultSuccess) && (sock->supportsControl)) {
                retVal = os_sockSetsockopt(sock->socketControl,
                                SOL_SOCKET, SO_REUSEADDR,
                                (void *)&optVal, optLen);
                IN_REPORT_SOCKFUNC(2, retVal,
                               "set socket reuse option", "setsockopt");

                /* this code abstracts from IPv4 and IPv6 */
                in_locatorToSockaddrForAnyAddress(sock->controlUnicastLocator,
							(struct sockaddr*) &bindAddress);

                /*bindAddress = sock->sockAddrControl;
                bindAddress.sin_addr.s_addr = htonl(INADDR_ANY); */
                if (retVal == os_resultSuccess) {
                    retVal = os_sockBind(sock->socketControl,
                        (const struct sockaddr *)&bindAddress,
                        /* (socklen_t)sizeof(bindAddress)); */
                        /* the size must be precise on solaris, otherwise EINV */
                        (socklen_t)sizeof(struct sockaddr_in)); /* FIXME encapsulate the struct-size */ 
                    IN_REPORT_SOCKFUNC(2, retVal,
                                   "bind socket", "bind");
                }
            }
        }
        if (retVal != os_resultSuccess) {
            result = OS_FALSE;
        }
    } else {
       result = OS_FALSE;
    }

    return result;
}



void
in_socketAddPartition(
    in_socket sock,
    in_partitionId partitionId,
    const char *addressString,
    os_boolean connected)
{

    /* Do any multicast related actions if needed */
    /* TODO: remove this check to the correct location. This works but
     *       breaks encapsulation */
    if (partitionId != 0 && connected) {
        /* No need to add first the default partition,
           that already happened with socket initialization */
        in_socketMulticastAddPartition(sock, addressString);
    }
}


static os_boolean
in_socketPrimaryAddressCompare(
    in_socket sock,
    in_address toCompare)
{
	/* TODO add IPv6 address handling */
    os_boolean result = 0;

    assert(sizeof(sock->sockAddrPrimary) == OS_SIZEOF(in_address));

    if (sock) {
    	result = in_addressEqual(&(sock->sockAddrPrimary), toCompare);
    }

    return result;
}

/* ------------------------------- constructor ------------------------------ */


/* private */
static void
in_socketDeinit(
        in_object _this);

static in_socket
in_socketNew(
	in_configChannel configChannel,
    os_boolean receiving,
    os_boolean supportsControl)
{
	os_char addressStringBuffer[IN_ADDRESS_STRING_LEN];
    in_socket result = NULL;
    os_boolean success = OS_TRUE;
    in_addressType addressType;
    c_ulong bufSizeRequested;
    c_ulong TOSRequested;
    /* Primary address, for destination checking */
    struct sockaddr_in sockAddrPrimary;
     /* Broadcast address corresponding to this interface */
    struct sockaddr_in sockAddrBroadcast;
    const char *defaultAddress =
    	in_configChannelGetGlobalPartitionAddress(configChannel);
    const char *addressLookingFor =
    	in_configChannelGetInterfaceId(configChannel);
    os_ushort portNr =
    	in_configChannelGetPortNr(configChannel);
    os_ushort portNrControl = IN_CONTROLPORT(portNr);
    const os_char *name =
    	in_configChannelGetPathName(configChannel);
    os_int addressFamily;

    assert(portNr < IN_USHORT_MAX);
	assert(defaultAddress != NULL);
	assert(addressLookingFor != NULL);

     /* Control address corresponding to this interface */
    addressFamily =
    	in_addressGetFamilyFromString(defaultAddress);

    result = (in_socket)os_malloc((os_uint)sizeof(*result));
    if (result != NULL) {
        in_objectInit(OS_SUPER(result),
                IN_OBJECT_KIND_SOCKET,
                in_socketDeinit);

        result->name = name ? os_strdup(name) : os_strdup("<channel>");
        result->supportsControl = supportsControl;
        result->socketData = os_sockNew(addressFamily, SOCK_DGRAM);
        IN_REPORT_SOCKFUNC(2, os_resultSuccess,
                           "socket creation", "socket");
        if (supportsControl) {
            result->socketControl = os_sockNew(addressFamily, SOCK_DGRAM);
            IN_REPORT_SOCKFUNC(2, os_resultSuccess,
                               "socket creation", "socket");
            /* TODO: Define os-abstraction for fd-sets */
            if (result->socketData > result->socketControl) {
                result->maxSockfd = result->socketData;
            } else {
                result->maxSockfd = result->socketControl;
            }
        } else {
            result->maxSockfd = result->socketData;
        }
        FD_ZERO(&result->sockSet);

        /* addressLookingFor = INCF_SIMPLE_PARAM(String, INCF_ROOT(General), Interface); */
		addressType = in_getAddressType(defaultAddress);
        switch (addressType) {
            case IN_ADDRESS_TYPE_BROADCAST:
                in_socketGetDefaultBroadcastInterface(addressLookingFor,
                    result->socketData, &sockAddrPrimary,
                    &sockAddrBroadcast);
                in_addressInitFromSockAddr(&result->sockAddrPrimary, &sockAddrPrimary);
                in_addressInitFromSockAddr(&result->sockAddrBroadcast, &sockAddrBroadcast);
                in_addressInitFromSockAddr(&result->sockAddrMulti, &sockAddrBroadcast);
                /* Broadcast sockets usually loop back and can not be stopped
                 * from that */
                result->loopsback = OS_TRUE;

                IN_TRACE_1(Test, 4, "Using broadcast address %s for default partition",
                    in_addressToString(&result->sockAddrMulti, addressStringBuffer, sizeof(addressStringBuffer)));
            break;
            case IN_ADDRESS_TYPE_MULTICAST:
                in_socketGetDefaultMulticastInterface(addressLookingFor,
                    result->socketData, &sockAddrPrimary,
                    &sockAddrBroadcast);
                in_addressInitFromSockAddr(&result->sockAddrPrimary, &sockAddrPrimary);
				in_addressInitFromSockAddr(&result->sockAddrBroadcast, &sockAddrBroadcast);
				in_addressInitFromStringWithDefault(&result->sockAddrMulti, defaultAddress, INCF_DEF(Address));

                /* We can stop multicasting from looping back though */
                result->loopsback = OS_FALSE;
                IN_TRACE_1(Test, 4, "Using multicast address %s for default partition",
                                    in_addressToString(&result->sockAddrMulti, addressStringBuffer, sizeof(addressStringBuffer)));
            break;
            case IN_ADDRESS_TYPE_LOOPBACK:
                in_socketGetDefaultLoopbackAddress(result->socketData,
                    &sockAddrPrimary);
                in_addressInitFromSockAddr(&result->sockAddrPrimary, &sockAddrPrimary);
 				in_addressInitFromSockAddr(&result->sockAddrBroadcast, &sockAddrPrimary);
 				in_addressInitFromSockAddr(&result->sockAddrMulti, &sockAddrPrimary);

 				result->loopsback = OS_TRUE;
                IN_TRACE_1(Test, 4, "Using loopback address %s for default partition",
                                                    in_addressToString(&result->sockAddrMulti, addressStringBuffer, sizeof(addressStringBuffer)));
            break;
            default:
            break;
        }

		result->dataUnicastLocator = in_locatorNew(portNr, &(result->sockAddrPrimary));
		result->dataMulticastLocator = in_locatorNew(portNr, &(result->sockAddrMulti));

        if (supportsControl) {
        	result->controlUnicastLocator = in_locatorNew(portNrControl,
        		&(result->sockAddrPrimary));
        	result->controlMulticastLocator = in_locatorNew(portNrControl,
        		&(result->sockAddrMulti));
        	/* \todo checkme, do we need multicast control transport */
        } else
        {
        	result->controlUnicastLocator = NULL;
			result->controlMulticastLocator = NULL;
        }

        if (receiving) {
            /* Set option to avoid sendbuffer */
        	/* TODO this must not be done in case ReceiveChannel is sending pin-whole messages to peer, to establish session in firewall in between */
            success = success && in_socketSetSendBufferSize(result, 0);
            /* Set option for custom receive buffer size */
            bufSizeRequested = in_configChannelGetReceiveBufferSize(configChannel);
            success = success && in_socketSetReceiveBufferSize(result, (os_int)bufSizeRequested);

            /* Bind to socket */
            success = success && in_socketBind(result);

            if (success) {
                if (!supportsControl) {
                    IN_REPORT_INFO_3(2, "Created and bound receiving socket \"%s\" "
                                        "for interface %s, port %u",
                                     name,
                                     in_addressToString(&result->sockAddrPrimary, addressStringBuffer, sizeof(addressStringBuffer)),
                                     portNr);
                } else {
                    IN_REPORT_INFO_4(2, "Created and bound receiving socket \"%s\" "
                                        "for interface %s, ports %u and %u",
                                     name,
                                     in_addressToString(&result->sockAddrPrimary, addressStringBuffer, sizeof(addressStringBuffer)),
                                     portNr, portNr+1);
                }
                IN_TRACE_1(Test, 1, "Creation and binding of receiving "
                                  "multicast socket \"%s\" succeeded.",
                                  name);
            }
        } else {
            /* Set option to avoid receivebuffer */
            success = success && in_socketSetReceiveBufferSize(result, 0);
            /* Set option for avoiding routing to other interfaces */
#ifndef OS_VXWORKS_DEFS_H
            success = success && in_socketSetDontRouteOption(result, OS_TRUE);
#endif
            /* Set option for custom TOS */
            TOSRequested = in_configChannelGetDifferentiatedServicesField(configChannel);
            success = success && in_socketSetTOS(result, (os_int)TOSRequested);

#ifdef OS_SOCKET_BIND_FOR_MULTICAST
        	/* Fix for windows: Bind to socket before setting Multicast options */
        	if (success && addressType==IN_ADDRESS_TYPE_MULTICAST) {
        		success = in_socketBind(result);
        	}
#endif
            if (success) {
            	IN_REPORT_INFO_3(2, "Created sending socket \"%s\"for "
                                    "interface %s, port %u",
                                     name,
                                     in_addressToString(&result->sockAddrPrimary, addressStringBuffer, sizeof(addressStringBuffer)),
                                     portNr);
            } else {
                IN_REPORT_INFO_4(2, "Created sending socket \"%s\"for "
                                        "interface %s, ports %u and %u",
                                     name,
                                     in_addressToString(&result->sockAddrPrimary, addressStringBuffer, sizeof(addressStringBuffer)),
                                     portNr, portNr+1);
            }
            IN_TRACE_1(Test, 1, "Creation of sending multicast socket \"%s\" succeeded.",
                              name);
        }

        /* TODO, check if this is valid/neccessary for send-sockets */
        switch (addressType) {
            case IN_ADDRESS_TYPE_BROADCAST: in_socketBroadcastInitialize(result); break;
            case IN_ADDRESS_TYPE_MULTICAST: in_socketMulticastInitialize(result); break;
            default: break;
        }

    }

    return result;
}

in_socket
in_socketDuplexNew(
	in_configChannel configChannel,
	os_boolean supportsControl)
{
    os_char addressStringBuffer[IN_ADDRESS_STRING_LEN];
    in_socket result = NULL;
    os_boolean success = OS_TRUE;
    in_addressType addressType;
    c_ulong bufSizeRequested;
    c_ulong TOSRequested;
    /* Primary address, for destination checking */
    struct sockaddr_in sockAddrPrimary;
     /* Broadcast address corresponding to this interface */
    struct sockaddr_in sockAddrBroadcast;
    const char *defaultAddress =
    	in_configChannelGetGlobalPartitionAddress(configChannel);
    const char *addressLookingFor =
    	in_configChannelGetInterfaceId(configChannel);
    os_ushort portNr =
    	in_configChannelGetPortNr(configChannel);
    os_ushort portNrControl = IN_CONTROLPORT(portNr);
    const os_char *name =
    	in_configChannelGetPathName(configChannel);
    os_int addressFamily;

    assert(portNr < IN_USHORT_MAX);
	assert(defaultAddress != NULL);
	assert(addressLookingFor != NULL);

     /* Control address corresponding to this interface */
    addressFamily =
    	in_addressGetFamilyFromString(defaultAddress);

    result = (in_socket)os_malloc((os_uint)sizeof(*result));
    if (result != NULL) {
        in_objectInit(OS_SUPER(result),
                 IN_OBJECT_KIND_SOCKET,
                 in_socketDeinit);

        result->name = name ? os_strdup(name) : os_strdup("<channel>");
        result->supportsControl = supportsControl;
        result->socketData = os_sockNew(addressFamily, SOCK_DGRAM);
        IN_REPORT_SOCKFUNC(2, os_resultSuccess,
                           "socket creation", "socket");
        if (supportsControl) {
            result->socketControl = os_sockNew(addressFamily, SOCK_DGRAM);
            IN_REPORT_SOCKFUNC(2, os_resultSuccess,
                               "socket creation", "socket");
            /* TODO: Define os-abstraction for fd-sets */
            if (result->socketData > result->socketControl) {
                result->maxSockfd = result->socketData;
            } else {
                result->maxSockfd = result->socketControl;
            }
        } else {
            result->maxSockfd = result->socketData;
        }
        FD_ZERO(&result->sockSet);

        /* addressLookingFor = INCF_SIMPLE_PARAM(String, INCF_ROOT(General), Interface); */
		addressType = in_getAddressType(defaultAddress);
        switch (addressType) {
            case IN_ADDRESS_TYPE_BROADCAST:
                in_socketGetDefaultBroadcastInterface(addressLookingFor,
                    result->socketData, &sockAddrPrimary,
                    &sockAddrBroadcast);
                in_addressInitFromSockAddr(&result->sockAddrPrimary, &sockAddrPrimary);
                in_addressInitFromSockAddr(&result->sockAddrBroadcast, &sockAddrBroadcast);
                in_addressInitFromSockAddr(&result->sockAddrMulti, &sockAddrBroadcast);
                /* Broadcast sockets usually loop back and can not be stopped
                 * from that */
                result->loopsback = OS_TRUE;

                IN_TRACE_1(Test, 4, "Using broadcast address %s for default partition",
                    in_addressToString(&result->sockAddrMulti, addressStringBuffer, sizeof(addressStringBuffer)));
            break;
            case IN_ADDRESS_TYPE_MULTICAST:
                in_socketGetDefaultMulticastInterface(addressLookingFor,
                    result->socketData, &sockAddrPrimary,
                    &sockAddrBroadcast);
                in_addressInitFromSockAddr(&result->sockAddrPrimary, &sockAddrPrimary);
				in_addressInitFromSockAddr(&result->sockAddrBroadcast, &sockAddrBroadcast);
				in_addressInitFromStringWithDefault(&result->sockAddrMulti, defaultAddress, INCF_DEF(Address));

                /* We can stop multicasting from looping back though */
                result->loopsback = OS_TRUE;
                IN_TRACE_1(Test, 4, "Using multicast address %s for default partition",
                                    in_addressToString(&result->sockAddrMulti, addressStringBuffer, sizeof(addressStringBuffer)));
            break;
            case IN_ADDRESS_TYPE_LOOPBACK:
                in_socketGetDefaultLoopbackAddress(result->socketData,
                    &sockAddrPrimary);
                in_addressInitFromSockAddr(&result->sockAddrPrimary, &sockAddrPrimary);
 				in_addressInitFromSockAddr(&result->sockAddrBroadcast, &sockAddrPrimary);
 				in_addressInitFromSockAddr(&result->sockAddrMulti, &sockAddrPrimary);

 				result->loopsback = OS_TRUE;
                IN_TRACE_1(Test, 4, "Using loopback address %s for default partition",
                                                    in_addressToString(&result->sockAddrMulti, addressStringBuffer, sizeof(addressStringBuffer)));
            break;
            default:
            break;
        }

        result->dataUnicastLocator = in_locatorNew(portNr, &(result->sockAddrPrimary));
        result->dataMulticastLocator = in_locatorNew(portNr, &(result->sockAddrMulti));

        if (supportsControl)
        {
        	result->controlUnicastLocator = in_locatorNew(portNrControl, &(result->sockAddrPrimary));
			result->controlMulticastLocator = in_locatorNew(portNrControl, &(result->sockAddrMulti));
        } else
        {
        	result->controlUnicastLocator = NULL;
			result->controlMulticastLocator = NULL;
        }

        /* Set option for custom receive buffer size */
        bufSizeRequested = in_configChannelGetReceiveBufferSize(configChannel);
        success = success && in_socketSetSendBufferSize(result, (os_int)bufSizeRequested);
        success = success && in_socketSetReceiveBufferSize(result, (os_int)bufSizeRequested);

        /* Bind to socket */
        success = success && in_socketBind(result);

#ifndef OS_VXWORKS_DEFS_H
        success = success && in_socketSetDontRouteOption(result, OS_TRUE);
#endif
        /* Set option for custom TOS */
        TOSRequested = in_configChannelGetDifferentiatedServicesField(configChannel);
        success = success && in_socketSetTOS(result, (os_int)TOSRequested);


        if (success) {
          if (!supportsControl) {
            IN_REPORT_INFO_3(2, "Created and bound duplex socket \"%s\" "
                     "for interface %s, port %u",
                     name,
                     in_addressToString(&result->sockAddrPrimary, addressStringBuffer, sizeof(addressStringBuffer)),
                     portNr);
          } else {
            IN_REPORT_INFO_4(2, "Created and bound duplex socket \"%s\" "
                     "for interface %s, ports %u and %u",
                     name,
                     in_addressToString(&result->sockAddrPrimary, addressStringBuffer, sizeof(addressStringBuffer)),
                     portNr, portNr+1);
          }
          IN_TRACE_1(Test, 1, "Creation and binding of duplex "
                 "multicast socket \"%s\" succeeded.",
                 name);
        }

        /* TODO, check if this is valid/neccessary for send-sockets */
        switch (addressType) {
            case IN_ADDRESS_TYPE_BROADCAST: in_socketBroadcastInitialize(result); break;
            case IN_ADDRESS_TYPE_MULTICAST: in_socketMulticastInitialize(result); break;
            default: break;
        }

    }

    return result;
}


/* public */

in_socket
in_socketSendNew(
	in_configChannel configChannel,
    os_boolean supportsControl)
{
    return in_socketNew(configChannel,
    		OS_FALSE,
    		supportsControl);
}


/* public */

in_socket
in_socketReceiveNew(
	in_configChannel configChannel,
    os_boolean supportsControl)
{
    return in_socketNew(configChannel,
    		OS_TRUE,
    		supportsControl);
}


/* -------------------------------- destructor ------------------------------ */

static void
in_socketDeinit(
        in_object _this)
{
    in_socket sock =
        (in_socket) _this;

    os_result retVal;

    if (sock) {
        in_locatorFree(sock->dataUnicastLocator);
        in_locatorFree(sock->dataMulticastLocator);

        retVal = os_sockFree(sock->socketData);
        IN_REPORT_SOCKFUNC(2, retVal,
            "release socket resources", "close");
        if ((retVal == os_resultSuccess) && (sock->supportsControl)) {
            in_locatorFree(sock->controlUnicastLocator);
            in_locatorFree(sock->controlMulticastLocator);

            retVal = os_sockFree(sock->socketControl);
            IN_REPORT_SOCKFUNC(2, retVal,
                "release socket resources", "close");
        }
        os_free(sock->name);
    }
    /* Not interested in any result */
    /* return result */
}


void
in_socketFree(
    in_socket sock)
{
    in_objectFree(OS_SUPER(sock));
}


/* ------------------------------- public methods --------------------------- */

os_boolean
in_socketSupportsControl(
		in_socket sock)
{
	return sock->supportsControl;
}

in_long
in_socketSendDataTo(
    in_socket sock,
    in_locator receiver,
    void *buffer,
    os_size_t length)
{
    in_long result = 0;
    os_int32 sendRes;
    os_boolean sendToSucceeded;
    struct sockaddr destAddr;
   
    assert(sock != NULL);
    assert(receiver != NULL);



    /* First check if we have to slow down because of too quick sending */
    /* in_brakeSlowDown(sock->brake); */
    { 
    	char buf[IN_ADDRESS_STRING_LEN+1]; /* stringified IPv6 address must fit into */
   
    	IN_TRACE_2(Send,6,"in_socketSendDataTo ---  ip: %s port: %d",
    			in_addressToString(in_locatorGetIp(receiver), buf, IN_ADDRESS_STRING_LEN+1)
    			,in_locatorGetPort(receiver));
    	IN_HEXDUMP("in_socketSendDataTo", 0, buffer, length);
    }
	/* Then do the writing */
    IN_PROF_LAPSTART(SendTo);
    in_locatorToSockaddr(receiver, &destAddr);
    sendRes = os_sockSendto(sock->socketData, buffer, length,
                     &destAddr,
                     (socklen_t)sizeof(destAddr)
                     );
    IN_PROF_LAPSTOP(SendTo);

    if(sendRes == -1)
    {
        /* TODO REMOVE THIS!!! not os independant!! needed now for debugging*/
        IN_TRACE_2(Send,6,"in_socketSendDataTo ---  result: %d --- errno: %d",sendRes, errno/*, os_strerror( errno)*/);
    } else
    {
        IN_TRACE_1(Send,6,"in_socketSendDataTo ---  result: %d ",sendRes);
    }

    if (sendRes > 0) {
        IN_REPORT_SOCKFUNC(6, os_resultSuccess,
                               "sending data to the socket", "sendto");
        sendToSucceeded = OS_TRUE;
    } else {
        IN_REPORT_SOCKFUNC(6, os_resultFail,
                               "sending data to the socket", "sendto");
        sendToSucceeded = OS_FALSE;
    }

    if (sendToSucceeded) {
        result = (in_long)sendRes;
        /* And update the brake for the next time */
        /* in_brakeAddUnits(sock->brake, length); */
    }

    return result;
}


in_long
in_socketSendControlTo(
		in_socket sock,
		in_locator receiver,
		void *buffer,
		os_size_t length)
{
    in_long result = 0;
    os_int32 sendRes;
    os_boolean sendToSucceeded;
    struct sockaddr destAddr;

    assert(sock != NULL);
    assert(receiver != NULL);

    /* Control message */
    assert(sock->supportsControl);
    IN_HEXDUMP("in_socketSendControlTo", 0, buffer, length);

	in_locatorToSockaddr(receiver, &destAddr);
    sendRes = os_sockSendto(sock->socketControl, buffer, length,
                     &destAddr,
                     (socklen_t)sizeof(destAddr));

    if (sendRes > 0) {
        IN_REPORT_SOCKFUNC(6, os_resultSuccess,
                       "sending control message to the socket", "sendto");
        sendToSucceeded = OS_TRUE;
    } else {
        IN_REPORT_SOCKFUNC(6, os_resultFail,
                       "sending control message to the socket", "sendto");
        sendToSucceeded = OS_FALSE;
    }

    if (sendToSucceeded) {
        result = (in_long)sendRes;
    }

    return result;
}

in_long
in_socketReceive(
    in_socket sock,
    in_locator senderLocator, /* in/out */
    void *buffer,
    os_size_t length,
    os_boolean *isControl,
    const os_time *timeOut)
{
    in_long result = 0;
    os_int32 recvRes = 0;
    os_int32 selectRes;
    struct sockaddr_in6 sockAddr; /* TODO sockaddr_storage */
    os_int fromLen = (os_int)sizeof(sockAddr);
    os_boolean readDone = OS_FALSE;
    os_time tmpTimeOut = *timeOut;
#ifdef IN_DEBUGGING
    os_boolean control = OS_FALSE;
#endif

    assert(sock != NULL);
    assert(senderLocator != NULL);

    FD_SET(sock->socketData, &sock->sockSet);
    if (sock->supportsControl) {
        FD_SET(sock->socketControl, &sock->sockSet);
    }

    /* Stop all profiling here because the select is a blocking call */
    /* IN_PROF_LAPSTOP(BridgeRead_1); */
    /* IN_PROF_LAPSTOP(PlugRead_1); */
    /* Use tmpTimeOut because select modifies timeOut param under Linux */
    selectRes = os_sockSelect(sock->maxSockfd+1, &sock->sockSet, NULL, NULL, &tmpTimeOut);

    if (selectRes > 0) {

        /* IN_PROF_LAPSTART(RecvFrom); */
        if (sock->supportsControl) {
            if (FD_ISSET(sock->socketControl, &sock->sockSet)) {
                assert(sock->supportsControl);
                recvRes = os_sockRecvfrom(sock->socketControl, buffer, length,
                                   (struct sockaddr *)&sockAddr, (socklen_t *)&fromLen);
                readDone = OS_TRUE;
                *isControl = OS_TRUE;
#ifdef IN_DEBUGGING
                control = OS_TRUE;
#endif
            }
        }
        if (readDone != OS_TRUE) {
            assert(FD_ISSET(sock->socketData, &sock->sockSet));
            recvRes = os_sockRecvfrom(sock->socketData, buffer, length,
                               (struct sockaddr *)&sockAddr, (socklen_t *)&fromLen);
            *isControl = OS_FALSE;
        }

        /* IN_PROF_LAPSTOP(RecvFrom); */
        if (recvRes > 0) {
			result = (in_long)recvRes;
			in_locatorInitFromSockaddr(senderLocator,
					(struct sockaddr *)&sockAddr);

#ifdef IN_DEBUGGING
			if (control) {
				IN_HEXDUMP("in_socketReceiveControl", 0, buffer, result);
			} else {
				IN_HEXDUMP("in_socketReceiveData", 0, buffer, result);
			}
#endif

			/* Resume profiling because we have actually received something
			 * relevant */
			/* IN_PROF_LAPSTART(BridgeRead_2); */
			/* IN_PROF_LAPSTART(PlugRead_2); */

		   IN_REPORT_SOCKFUNC(6, os_resultSuccess,
						  "receiving data from socket", "recvfrom");
        } else {
           IN_REPORT_SOCKFUNC(6, os_resultFail,
                              "receiving data from socket", "recvfrom");
        }
    } else {
        if (selectRes < 0) {
            IN_REPORT_SOCKFUNC(6, os_resultFail,
                           "receiving data from socket", "select");
        }
    }
    return result;
}
