/* interface */
#include "in_socketMisc.h"

/* implemenation */
#include <string.h>
#include "os_heap.h"
#include "os_socket.h"
#include "in_commonTypes.h"

/* 44 octets for IPv6 addresses */
#define IN_INTF_MAX_NAME_LEN (INET6_ADDRSTRLEN)
typedef char in_interfaceName[IN_INTF_MAX_NAME_LEN];

#define SD_FLAG_IS_SET(flags, flag) ((((in_uint)(flags) & (in_uint)(flag))) != 0U)

#if 0
#define IN_ADDRESS(ptr)         ((size_t)(ptr))
#define IN_POINTER(address)     ((char *)(address))
#endif

/* last seen Socket error, only changes are reported in the errorlog */
os_sockErrno inLastSockError = 0;
/* Helper functions */

/* \brief converts to IPv6 address format
 *
 * Returns 16 octet vector. Using IPv4-mapping to reprensent
 * IPv4 addresses with IPv6 (RFC3493)*/

in_addressType
in_getAddressType(
    const char *addressString)
{
    in_addressType result = IN_ADDRESS_TYPE_UNKNOWN;
    OS_STRUCT(in_address) address;

    if (strcmp(addressString, INCF_BROADCAST_EXPR) == 0) {
    	/* broadcast is available for IPv4 only */
        result = IN_ADDRESS_TYPE_BROADCAST;
    } else {
    	/* do not care about return value, 0.0.0.0 will succeed in any case */
    	in_addressInitFromStringWithDefault(&address, addressString, "0.0.0.0");
    	result = in_addressGetType(&address);
    }
    /* POST: result of [IN_ADDRESS_TYPE_UNKNOWN, .., IN_ADDRESS_TYPE_MULTICAST] */
    return result;
}

/* interfaceInfo */

struct in_interfaceInfo_s {
    in_interfaceName  name;
    os_ushort    flags;
    struct sockaddr  *primaryAddress;
    struct sockaddr  *broadcastAddress;
};

static in_interfaceInfo
in_interfaceInfoNew(
    const in_interfaceName name,
    const os_ushort flags,
    const struct sockaddr *primaryAddress,
    const struct sockaddr *broadcastAddress)
{
    in_interfaceInfo result = NULL;

    result = (in_interfaceInfo)os_malloc((os_uint)sizeof(*result));

    if (result != NULL) {
        /* Copy fields */
        memcpy(result->name, name, IN_INTF_MAX_NAME_LEN);
        result->name[IN_INTF_MAX_NAME_LEN - 1] = '\0';

        result->flags = flags;

        if (primaryAddress && ((int)primaryAddress->sa_family == AF_INET)) {
            /* Only IPv4 for now */
            result->primaryAddress = (struct sockaddr *)os_malloc(
                                        (os_uint)sizeof(struct sockaddr_in));
            if (result->primaryAddress) {
                memcpy(result->primaryAddress, primaryAddress,
                       (os_uint)sizeof(struct sockaddr_in));
            }
            /* If interface supports broadcast, fill bc info */
            if (broadcastAddress && SD_FLAG_IS_SET(flags, IFF_BROADCAST)) {
                result->broadcastAddress = (struct sockaddr *)os_malloc(
                                              (os_uint)sizeof(struct sockaddr_in));
                if (result->broadcastAddress) {
                    memcpy(result->broadcastAddress, broadcastAddress,
                           (os_uint)sizeof(struct sockaddr_in));
                }
            } else {
                result->broadcastAddress = NULL;
            }
        } else {
            result->primaryAddress = NULL;
            result->broadcastAddress = NULL;
        }
    }

    return result;
}

static void
in_interfaceInfoFree(
    in_interfaceInfo interfaceInfo)
{
    if (interfaceInfo != NULL) {
        if (interfaceInfo->primaryAddress) {
            os_free(interfaceInfo->primaryAddress);
        }
        if (interfaceInfo->broadcastAddress) {
            os_free(interfaceInfo->broadcastAddress);
        }
        os_free(interfaceInfo);
    }
}




/* ------------------------------- Callbacks -------------------------------- */

static void
in_interfaceWalkCountBC(
    os_ifAttributes *intf,
    os_socket sockfd,
    void *actionArg)
{
    unsigned int *count = (unsigned int *)actionArg;

    (void)intf;
    (void)sockfd;

    /* Only interested in broadcast-enabled interfaces for now */
    if (SD_FLAG_IS_SET(intf->flags, IFF_BROADCAST)) {
        (*count)++;
    }
}


static void
in_interfaceWalkFillBC(
    os_ifAttributes *intf,
    os_socket sockfd,
    void *actionArg)
{
    in_interfaceInfo **interfaceInfo = (in_interfaceInfo **)actionArg;

    if (SD_FLAG_IS_SET(intf->flags, IFF_BROADCAST)) {
        **interfaceInfo = in_interfaceInfoNew(intf->name, intf->flags,
                              &intf->address, &intf->broadcast_address);
         /* (*interfaceInfo)++ not allowed by QAC */
        *interfaceInfo = &((*interfaceInfo)[1]);
    }
}


static void
in_interfaceWalkCountMC(
    os_ifAttributes *intf,
    os_socket sockfd,
    void *actionArg)
{
    unsigned int *count = (unsigned int *)actionArg;

    (void)intf;
    (void)sockfd;

    /* Only interested in multicast-enabled interfaces for now */
    if (SD_FLAG_IS_SET(intf->flags, IFF_MULTICAST) &&
        !SD_FLAG_IS_SET(intf->flags, IFF_LOOPBACK)) {
        (*count)++;
    }
}


static void
in_interfaceWalkFillMC(
    os_ifAttributes *intf,
    os_socket sockfd,
    void *actionArg)
{
    in_interfaceInfo **interfaceInfo = (in_interfaceInfo **)actionArg;
    struct sockaddr *broadaddr;

    if (SD_FLAG_IS_SET(intf->flags, IFF_MULTICAST) &&
        !SD_FLAG_IS_SET(intf->flags, IFF_LOOPBACK)) {
        if (SD_FLAG_IS_SET(intf->flags, IFF_BROADCAST)) {
            broadaddr = &intf->broadcast_address;
        } else {
            broadaddr = NULL;
        }
        **interfaceInfo = in_interfaceInfoNew(intf->name, intf->flags,
                              &intf->address, broadaddr);
         /* (*interfaceInfo)++ not allowed by QAC */
        *interfaceInfo = &((*interfaceInfo)[1]);
    }
}


static void
in_interfaceWalkCountLoopback(
    os_ifAttributes *intf,
    os_socket sockfd,
    void *actionArg)
{
    unsigned int *count = (unsigned int *)actionArg;

    /* Only interested in broadcast-enabled interfaces for now */
    if (SD_FLAG_IS_SET(intf->flags, IFF_LOOPBACK)) {
        (*count)++;
    }
}


static void
in_interfaceWalkFillLoopback(
    os_ifAttributes *intf,
    os_socket sockfd,
    void *actionArg)
{
    in_interfaceInfo **interfaceInfo = (in_interfaceInfo **)actionArg;

   (void)sockfd;

    if (SD_FLAG_IS_SET(intf->flags, IFF_LOOPBACK)) {
        **interfaceInfo = in_interfaceInfoNew(intf->name, intf->flags,
                              &intf->address, NULL);
         /* (*interfaceInfo)++ not allowed by QAC */
         *interfaceInfo = &((*interfaceInfo)[1]);
    }

}


/* -------------------------------------------------------------------------- */

/* ------------------- Interface retrieval and walk over -------------------- */

#define MAX_INTERFACES 64

typedef void (*in_interfaceWalkFunc)(
                 os_ifAttributes *intf,
                 os_socket sockfd,
                 void *actionArg);

static void
in_interfaceIPv4Walk(
    os_ifAttributes *allInterfaces,
    os_uint nofInterfaces,
    os_socket sockfd,
    const in_interfaceWalkFunc action,
    void *actionArg)
{
    os_uint currentInterface = 0;

    if (action) {
        while (currentInterface < nofInterfaces) {
            if (SD_FLAG_IS_SET(allInterfaces[currentInterface].flags, IFF_UP)) {
                action(&allInterfaces[currentInterface], sockfd, actionArg);
                currentInterface++;
            }
        }
    }
}

#undef IN_MAX

os_result
in_interfaceInfoRetrieveAllBC(
    in_interfaceInfo **interfaceList /* [nofInterfaces */,
    os_uint *nofInterfaces,
    os_socket sockfd)
{
    os_result result;
    os_ifAttributes ifList[MAX_INTERFACES];
    os_uint nofIf = 0;
    in_interfaceInfo *interfaceHelper;

    *interfaceList = NULL;
    *nofInterfaces = 0;

    result = os_sockQueryInterfaces(&ifList[0], (os_uint)MAX_INTERFACES, &nofIf);

    if (result == os_resultSuccess) {
        /* Count the number of valid interfaces */
        in_interfaceIPv4Walk(ifList, nofIf, sockfd,
                             in_interfaceWalkCountBC, nofInterfaces);


        /* Number of IPv4 addresses known, now fill them */
        *interfaceList = (in_interfaceInfo *)os_malloc(
                             (*nofInterfaces) * (os_uint)sizeof(*interfaceList));

        /* Fill the interface info structures */
        interfaceHelper = *interfaceList;
        in_interfaceIPv4Walk(ifList, nofIf, sockfd,
                             in_interfaceWalkFillBC, &interfaceHelper);

    }

    return nofIf;
}

os_result
in_interfaceInfoRetrieveAllMC(
    in_interfaceInfo **interfaceList /* [nofInterfaces */,
    os_uint *nofInterfaces,
    os_socket sockfd)
{
    os_result result;
    os_ifAttributes ifList[MAX_INTERFACES];
    os_uint nofIf;
    in_interfaceInfo *interfaceHelper;

    *interfaceList = NULL;
    *nofInterfaces = 0;

    result = os_sockQueryInterfaces(&ifList[0], (os_uint)MAX_INTERFACES, &nofIf);

    if (result == os_resultSuccess) {
        /* Count the number of valid interfaces */
        in_interfaceIPv4Walk(ifList, nofIf, sockfd,
                             in_interfaceWalkCountMC, nofInterfaces);


        /* Number of IPv4 addresses known, now fill them */
        *interfaceList = (in_interfaceInfo *)os_malloc(
                             (*nofInterfaces) * (os_uint)sizeof(*interfaceList));

        /* Fill the interface info structures */
        interfaceHelper = *interfaceList;
        in_interfaceIPv4Walk(ifList, nofIf, sockfd,
                             in_interfaceWalkFillMC, &interfaceHelper);

    }

    return nofIf;
}

os_result
in_interfaceInfoRetrieveAllLoopback(
    in_interfaceInfo **interfaceList /* [nofInterfaces */,
    os_uint *nofInterfaces,
    os_socket sockfd)
{
    os_result result;
    os_ifAttributes ifList[MAX_INTERFACES];
    os_uint nofIf;
    in_interfaceInfo *interfaceHelper;

    *interfaceList = NULL;
    *nofInterfaces = 0;

    result = os_sockQueryInterfaces(&ifList[0], (os_uint)MAX_INTERFACES, &nofIf);

    if (result == os_resultSuccess) {
        /* Count the number of valid interfaces */
        in_interfaceIPv4Walk(ifList, nofIf, sockfd,
                             in_interfaceWalkCountLoopback, nofInterfaces);


        /* Number of IPv4 addresses known, now fill them */
        *interfaceList = (in_interfaceInfo *)os_malloc(
            (*nofInterfaces) * (os_uint)sizeof(*interfaceList));

        /* Fill the interface info structures */
        interfaceHelper = *interfaceList;
        in_interfaceIPv4Walk(ifList, nofIf, sockfd,
                             in_interfaceWalkFillLoopback, &interfaceHelper);

    }

    return nofIf;
}

/* -------------------------------------------------------------------------- */

/* ---------------------- Interface info members and getters ---------------- */


void
in_interfaceInfoFreeAll(
    in_interfaceInfo *interfaceList /* [nofInterfaces */,
    os_uint nofInterfaces)
{
    os_uint i;

    if (interfaceList) {
        for (i=0; i<nofInterfaces; i++) {
            in_interfaceInfoFree(interfaceList[i]);
        }
        os_free(interfaceList);
    }
}


char *
in_interfaceInfoGetName(
    const in_interfaceInfo interfaceInfo)
{
    char *result = NULL;

    if (interfaceInfo != NULL) {
         result = interfaceInfo->name;
    }

    return result;
}

#if 0
/* Currently not used */
unsigned short
in_interfaceInfoGetFlags(
    const in_interfaceInfo interfaceInfo)
{
    short result = 0;

    if (interface) {
        result = interfaceInfo->flags;
    }

    return result;
}
#endif


struct sockaddr *
in_interfaceInfoGetPrimaryAddress(
    const in_interfaceInfo interfaceInfo)
{
    struct sockaddr *result = NULL;

    if (interfaceInfo != NULL) {
        result = interfaceInfo->primaryAddress;
    }

    return result;
}


struct sockaddr *
in_interfaceInfoGetBroadcastAddress(
    const in_interfaceInfo interfaceInfo)
{
    struct sockaddr *result = NULL;

    if (interfaceInfo != NULL) {
         result = interfaceInfo->broadcastAddress;
    }

    return result;
}
