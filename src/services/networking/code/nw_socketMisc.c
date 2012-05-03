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
/* interface */
#include "nw_socketMisc.h"

/* implemenation */
#include <string.h>
#include "os_heap.h"
#include "os_socket.h"
#include "nw_stringList.h"
#include "nw_commonTypes.h"
#include "nw__confidence.h"
#ifdef DO_HOST_BY_NAME
#include <netdb.h>
#endif

typedef char sk_interfaceName[SK_INTF_MAX_NAME_LEN];

#define SD_FLAG_IS_SET(flags, flag) ((((os_uint32)(flags) & (os_uint32)(flag))) != 0U)

#define SK_ADDRESS(ptr)         ((size_t)(ptr))
#define SK_POINTER(address)     ((char *)(address))

/* last seen Socket error, only changes are reported in the errorlog */
/* @todo - ??? Not even vaguely thread / service concurrency safe */
os_sockErrno skLastSockError = 0;

/* Helper functions */

sk_addressType
sk_getAddressType(
    const char *addressString)
{
    sk_addressType result = SK_TYPE_UNKNOWN;
    os_sockaddr_storage address;
    os_uint32 addressHostFormat;
    nw_stringList addressList;
    const char *usedAddressString;
    os_boolean convertedOK;

    addressList = nw_stringListNew(addressString, NW_ADDRESS_SEPARATORS);
    if (addressList && (nw_stringListGetSize(addressList) > 0)) {
        /* Take first item to represent this string */
        usedAddressString = nw_stringListGetValue(addressList, 0);
        if (strcmp(usedAddressString, NWCF_BROADCAST_EXPR) == 0) {
            if (nw_configurationGetIsIPv6())
            {
                NW_REPORT_ERROR_1("sk_getAddressType",
                                  "Incorrectly specified address '%s' in IPv6 configured service. Check configuration.",
                                  usedAddressString);
            }
            else
            {
                result = SK_TYPE_BROADCAST;
            }

        } else {
            convertedOK = os_sockaddrStringToAddress(usedAddressString,
                                                    (os_sockaddr*) &address,
                                                    !nw_configurationGetIsIPv6());
            if (convertedOK)
            {
                if (address.ss_family == AF_INET)
                {
                    /* IPv4 address */
                    if (nw_configurationGetIsIPv6())
                    {
                        NW_REPORT_ERROR_1("sk_getAddressType",
                                          "Incorrectly specified IPv4 address %s in IPv6 configured service. Check configuration.",
                                          usedAddressString);
                    }
                    else
                    {
                        addressHostFormat = ntohl(((os_sockaddr_in*)&address)->sin_addr.s_addr);
                        if (IN_CLASSD(addressHostFormat)) {
                            result = SK_TYPE_MULTICAST;
                        } else if (addressHostFormat == INADDR_LOOPBACK) {
                            result = SK_TYPE_LOOPBACK;
                        } else if (IN_CLASSA(addressHostFormat) ||
                                   IN_CLASSB(addressHostFormat) ||
                                   IN_CLASSC(addressHostFormat)) {
                            result = SK_TYPE_UNICAST;
                        }
                    }
                }
                else
                {
                    os_sockaddr_in6* v6Address = (os_sockaddr_in6*) &address;
                    /* IPV6 address */
                    if (nw_configurationGetIsIPv6())
                    {
                        if (IN6_IS_ADDR_MULTICAST(&v6Address->sin6_addr))
                        {
                            result = SK_TYPE_MULTICAST;
                        }
                        else if (IN6_IS_ADDR_LOOPBACK(&v6Address->sin6_addr))
                        {
                            result = SK_TYPE_LOOPBACK;
                        }
                        else
                        {
                            result = SK_TYPE_UNICAST;
                        }
                    }
                    else
                    {
                        NW_REPORT_ERROR_1("sk_getAddressType",
                                          "Incorrectly specified IPv6 address %s in IPv4 configured service. Check configuration.",
                                          usedAddressString);
                    }
                }
            }
        }
    }

    return result;
}

/* interfaceInfo */

struct sk_interfaceInfo_s {
    sk_interfaceName  name;
    unsigned short    flags;
    os_sockaddr_storage  *primaryAddress;
    os_sockaddr_storage  *broadcastAddress;
    os_uint interfaceIndexNo;
};

static sk_interfaceInfo
sk_interfaceInfoNew(
    const sk_interfaceName name,
    const unsigned short flags,
    const os_sockaddr_storage *primaryAddress,
    const os_sockaddr_storage *broadcastAddress)
{
    sk_interfaceInfo result = NULL;

    result = (sk_interfaceInfo)os_malloc((os_uint32)sizeof(*result));

    if (result != NULL) {
        /* Copy fields */
        memcpy(result->name, name, SK_INTF_MAX_NAME_LEN);
        result->name[SK_INTF_MAX_NAME_LEN - 1] = '\0';

        result->flags = flags;

        if (primaryAddress) {
            result->primaryAddress = (os_sockaddr_storage *)os_malloc(
                                        (os_uint32)sizeof(os_sockaddr_storage));
            if (result->primaryAddress) {
                memcpy(result->primaryAddress, primaryAddress,
                       (os_uint32)sizeof(os_sockaddr_storage));
            }
            /* If interface is IPv4 & supports broadcast, fill bc info */
            if (primaryAddress->ss_family == AF_INET &&
                broadcastAddress && SD_FLAG_IS_SET(flags, IFF_BROADCAST)) {
                result->broadcastAddress = (os_sockaddr_storage *)os_malloc(
                                              (os_uint32)sizeof(os_sockaddr_storage));
                if (result->broadcastAddress) {
                    memcpy(result->broadcastAddress, broadcastAddress,
                           (os_uint32)sizeof(struct sockaddr_in));
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
sk_interfaceInfoFree(
    sk_interfaceInfo interfaceInfo)
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
sk_interfaceWalkCountBC(
    os_ifAttributes *intf,
    os_int sockfd,
    void *actionArg)
{
    os_uint *count = (os_uint *)actionArg;

    (void)intf;
    (void)sockfd;

    /* Only interested in broadcast-enabled interfaces for now */
    if (SD_FLAG_IS_SET(intf->flags, IFF_BROADCAST)) {
        (*count)++;
    }
}


static void
sk_interfaceWalkFillBC(
    os_ifAttributes *intf,
    os_int sockfd,
    void *actionArg)
{
    sk_interfaceInfo **interfaceInfo = (sk_interfaceInfo **)actionArg;

    if (SD_FLAG_IS_SET(intf->flags, IFF_BROADCAST)) {
        **interfaceInfo = sk_interfaceInfoNew(intf->name, intf->flags,
                              &intf->address, &intf->broadcast_address);
         /* (*interfaceInfo)++ not allowed by QAC */
        *interfaceInfo = &((*interfaceInfo)[1]);
    }
}


static void
sk_interfaceWalkCountMC(
    os_ifAttributes *intf,
    os_int sockfd,
    void *actionArg)
{
    os_uint *count = (os_uint *)actionArg;

    (void)intf;
    (void)sockfd;
    if (nw_configurationGetIsIPv6() && !os_sockaddrIsLoopback((os_sockaddr*)&intf->address))
    {
        /* AFAIK all Ipv6 interfaces are multicast capable */
        (*count)++;
    }
    else
    {
#ifndef INTEGRITY
        /* Only interested in multicast-enabled interfaces for now */
        if (SD_FLAG_IS_SET(intf->flags, IFF_MULTICAST) &&
            !SD_FLAG_IS_SET(intf->flags, IFF_LOOPBACK)) {
            (*count)++;
        }
#else
        if (!SD_FLAG_IS_SET(intf->flags, IFF_LOOPBACK)) {
            (*count)++;
        }
#endif
    }
}


static void
sk_interfaceWalkFillMC(
    os_ifAttributes *intf,
    os_int sockfd,
    void *actionArg)
{
    sk_interfaceInfo **interfaceInfo = (sk_interfaceInfo **)actionArg;
    os_sockaddr_storage *broadaddr;

    if (nw_configurationGetIsIPv6())
    {
        /* AFAIK all Ipv6 interfaces are multicast capable */
        if (!os_sockaddrIsLoopback((os_sockaddr*)&intf->address))
        {
            broadaddr = NULL;
            **interfaceInfo = sk_interfaceInfoNew(intf->name, intf->flags,
                                  &intf->address, broadaddr);
            (**interfaceInfo)->interfaceIndexNo = intf->interfaceIndexNo;
             /* (*interfaceInfo)++ not allowed by QAC */
            *interfaceInfo = &((*interfaceInfo)[1]);
        }
    }
    else
    {
        if (SD_FLAG_IS_SET(intf->flags, IFF_MULTICAST) &&
            !SD_FLAG_IS_SET(intf->flags, IFF_LOOPBACK)) {
            if (SD_FLAG_IS_SET(intf->flags, IFF_BROADCAST)) {
                broadaddr = &intf->broadcast_address;
            } else {
                broadaddr = NULL;
            }
            **interfaceInfo = sk_interfaceInfoNew(intf->name, intf->flags,
                                  &intf->address, broadaddr);
            (**interfaceInfo)->interfaceIndexNo = intf->interfaceIndexNo;
             /* (*interfaceInfo)++ not allowed by QAC */
            *interfaceInfo = &((*interfaceInfo)[1]);
        }
    }
}


static void
sk_interfaceWalkCountLoopback(
    os_ifAttributes *intf,
    os_int sockfd,
    void *actionArg)
{
    os_uint *count = (os_uint *)actionArg;

    (void)sockfd;
    (void)intf;

    if (nw_configurationGetIsIPv6()
        && os_sockaddrIsLoopback((os_sockaddr*)&intf->address))
    {
        (*count)++;
    }
    else if (SD_FLAG_IS_SET(intf->flags, IFF_LOOPBACK)) {
        (*count)++;
    }
}


static void
sk_interfaceWalkFillLoopback(
    os_ifAttributes *intf,
    os_int sockfd,
    void *actionArg)
{
    sk_interfaceInfo **interfaceInfo = (sk_interfaceInfo **)actionArg;

   (void)sockfd;

    if (nw_configurationGetIsIPv6()
        && os_sockaddrIsLoopback((os_sockaddr*)&intf->address))
    {
        **interfaceInfo = sk_interfaceInfoNew(intf->name, intf->flags,
                              &intf->address, NULL);
         *interfaceInfo = &((*interfaceInfo)[1]);
    }
    else if (SD_FLAG_IS_SET(intf->flags, IFF_LOOPBACK)) {
        **interfaceInfo = sk_interfaceInfoNew(intf->name, intf->flags,
                              &intf->address, NULL);
         *interfaceInfo = &((*interfaceInfo)[1]);
    }
}


/* -------------------------------------------------------------------------- */

/* ------------------- Interface retrieval and walk over -------------------- */

#define MAX_INTERFACES 64

typedef void (*sk_interfaceWalkFunc)(
                 os_ifAttributes *intf,
                 os_int sockfd,
                 void *actionArg);

static void
sk_interfaceIPv4Walk(
    os_ifAttributes *allInterfaces,
    os_uint32 nofInterfaces,
    os_int sockfd,
    const sk_interfaceWalkFunc action,
    void *actionArg)
{
    os_uint32 currentInterface = 0;

    if (action) {
        while (currentInterface < nofInterfaces) {
            if (nw_configurationGetIsIPv6()
                || SD_FLAG_IS_SET(allInterfaces[currentInterface].flags, IFF_UP)) {
                action(&allInterfaces[currentInterface], sockfd, actionArg);
            }
            currentInterface++;
        }
    }
}

#undef SK_MAX

os_int
sk_interfaceInfoRetrieveAllBC(
    sk_interfaceInfo **interfaceList /* [nofInterfaces */,
    os_uint *nofInterfaces,
    os_int sockfd)
{
    os_result result = os_resultFail;
    os_ifAttributes ifList[MAX_INTERFACES];
    os_uint32 nofIf = 0;
    sk_interfaceInfo *interfaceHelper;
#ifdef INTEGRITY
    char *addressLookingFor;
#endif

    *interfaceList = NULL;
    *nofInterfaces = 0;

    if (! nw_configurationGetIsIPv6())
    {
        result = os_sockQueryInterfaces(&ifList[0], (os_uint32)MAX_INTERFACES, &nofIf);
    }

    if (result == os_resultSuccess) {
        /* Count the number of valid interfaces */
        sk_interfaceIPv4Walk(ifList, nofIf, sockfd,
                             sk_interfaceWalkCountBC, nofInterfaces);

#ifdef INTEGRITY
        if ( *nofInterfaces > 1 )
        {
           addressLookingFor = NWCF_SIMPLE_PARAM(String, NWCF_ROOT(General), Interface);
           if ( strcmp( addressLookingFor, NWCF_FIRSTAVAILABLE_EXPR)==0 )
           {
              /* Failure, address must be specified for integrity */
              NW_REPORT_ERROR("Reading network config", "Configuring of <NetworkInterfaceAddress> is mandatory for the networking service on GHS Integrity on systems with multiple interfaces.");
              *nofInterfaces = 0;
           }
        }
#endif


        if ( *nofInterfaces > 0 )
        {
           /* Number of IPv4 addresses known, now fill them */
           *interfaceList = (sk_interfaceInfo *)os_malloc(
              (*nofInterfaces) * (os_uint32)sizeof(*interfaceList));

           /* Fill the interface info structures */
           interfaceHelper = *interfaceList;
           sk_interfaceIPv4Walk(ifList, nofIf, sockfd,
                                sk_interfaceWalkFillBC, &interfaceHelper);

        }
    }

    return nofIf;
}

os_int
sk_interfaceInfoRetrieveAllMC(
    sk_interfaceInfo **interfaceList /* [nofInterfaces */,
    os_uint *nofInterfaces,
    os_int sockfd)
{
    os_result result;
    os_ifAttributes ifList[MAX_INTERFACES];
    os_uint32 nofIf;
    sk_interfaceInfo *interfaceHelper;
#ifdef INTEGRITY
    char *addressLookingFor;
#endif

    *interfaceList = NULL;
    *nofInterfaces = 0;

    if (nw_configurationGetIsIPv6())
    {
        result = os_sockQueryIPv6Interfaces(&ifList[0], (os_uint32)MAX_INTERFACES, &nofIf);
    }
    else
    {
        result = os_sockQueryInterfaces(&ifList[0], (os_uint32)MAX_INTERFACES, &nofIf);
    }

    if (result == os_resultSuccess) {
        /* Count the number of valid interfaces */
        sk_interfaceIPv4Walk(ifList, nofIf, sockfd,
                             sk_interfaceWalkCountMC, nofInterfaces);

    NW_TRACE_1(Configuration, 6,
                       "sk_interfaceInfoRetrieveAllMC: Retrieved %d multicast interfaces",
                       *nofInterfaces);

#ifdef INTEGRITY
    if ( *nofInterfaces > 1 )
    {
       addressLookingFor = NWCF_SIMPLE_PARAM(String, NWCF_ROOT(General), Interface);
           if ( strcmp( addressLookingFor, NWCF_FIRSTAVAILABLE_EXPR)==0 )
           {
              /* Failure, address must be specified for integrity */
              NW_REPORT_ERROR("Reading network config", "Configuring of <NetworkInterfaceAddress> is mandatory for the networking service on GHS Integrity on systems with multiple interfaces.");
           }
        }
#endif

        /* Number of IPv4 addresses known, now fill them */
        *interfaceList = (sk_interfaceInfo *)os_malloc(
                             (*nofInterfaces) * (os_uint32)sizeof(*interfaceList));

        /* Fill the interface info structures */
        interfaceHelper = *interfaceList;
        sk_interfaceIPv4Walk(ifList, nofIf, sockfd,
                             sk_interfaceWalkFillMC, &interfaceHelper);

    }

    return nofIf;
}

os_int
sk_interfaceInfoRetrieveAllLoopback(
    sk_interfaceInfo **interfaceList /* [nofInterfaces */,
    os_uint *nofInterfaces,
    os_int sockfd)
{
    os_result result;
    os_ifAttributes ifList[MAX_INTERFACES];
    os_uint32 nofIf;
    sk_interfaceInfo *interfaceHelper;

    *interfaceList = NULL;
    *nofInterfaces = 0;

    if (nw_configurationGetIsIPv6())
    {
        result = os_sockQueryIPv6Interfaces(&ifList[0], (os_uint32)MAX_INTERFACES, &nofIf);
    }
    else
    {
        result = os_sockQueryInterfaces(&ifList[0], (os_uint32)MAX_INTERFACES, &nofIf);
    }

    if (result == os_resultSuccess) {
        /* Count the number of valid interfaces */
        sk_interfaceIPv4Walk(ifList, nofIf, sockfd,
                             sk_interfaceWalkCountLoopback, nofInterfaces);


        /* Number of IPv4 addresses known, now fill them */
        *interfaceList = (sk_interfaceInfo *)os_malloc(
            (*nofInterfaces) * (os_uint32)sizeof(*interfaceList));

        /* Fill the interface info structures */
        interfaceHelper = *interfaceList;
        sk_interfaceIPv4Walk(ifList, nofIf, sockfd,
                             sk_interfaceWalkFillLoopback, &interfaceHelper);

    }

    return nofIf;
}

/* -------------------------------------------------------------------------- */

/* ---------------------- Interface info members and getters ---------------- */


void
sk_interfaceInfoFreeAll(
    sk_interfaceInfo *interfaceList /* [nofInterfaces */,
    os_uint nofInterfaces)
{
    os_uint i;

    if (interfaceList) {
        for (i=0; i<nofInterfaces; i++) {
            sk_interfaceInfoFree(interfaceList[i]);
        }
        os_free(interfaceList);
    }
}

const char *
sk_interfaceInfoGetName(
    const sk_interfaceInfo interfaceInfo)
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
sk_interfaceInfoGetFlags(
    const sk_interfaceInfo interfaceInfo)
{
    short result = 0;

    if (interface) {
        result = interfaceInfo->flags;
    }

    return result;
}
#endif


os_sockaddr_storage *
sk_interfaceInfoGetPrimaryAddress(
    const sk_interfaceInfo interfaceInfo)
{
    os_sockaddr_storage *result = NULL;

    if (interfaceInfo != NULL) {
        result = interfaceInfo->primaryAddress;
    }

    return result;
}


os_sockaddr_storage *
sk_interfaceInfoGetBroadcastAddress(
    const sk_interfaceInfo interfaceInfo)
{
    os_sockaddr_storage *result = NULL;

    if (interfaceInfo != NULL) {
         result = interfaceInfo->broadcastAddress;
    }

    return result;
}

os_uint
sk_interfaceInfoGetInterfaceIndexNo(sk_interfaceInfo this_)
{
    return this_->interfaceIndexNo;
}

void
sk_interfaceInfoSetInterfaceIndexNo(sk_interfaceInfo this_, os_uint indexNo)
{
    this_->interfaceIndexNo = indexNo;
}
