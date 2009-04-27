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
/* interface */
#include "nw_socketMisc.h"

/* implemenation */
#include <string.h>
#include "os_heap.h"
#include "os_socket.h"
#include "nw_commonTypes.h"
#include "nw__confidence.h"

#define SK_INTF_MAX_NAME_LEN 16
typedef char sk_interfaceName[SK_INTF_MAX_NAME_LEN];

#define SD_FLAG_IS_SET(flags, flag) ((((unsigned int)(flags) & (unsigned int)(flag))) != 0U)

#define SK_ADDRESS(ptr)         ((size_t)(ptr))
#define SK_POINTER(address)     ((char *)(address))

/* last seen Socket error, only changes are reported in the errorlog */
os_sockErrno skLastSockError = 0;


/* Helper functions */

sk_address
sk_stringToAddress(
    const char *addressString,
    const char *addressOnError)
{
    sk_address result;

    result = (sk_address)inet_addr(addressString);
    
    if (result == (sk_address)(-1)) {
        NW_REPORT_WARNING_2("sk_stringToAddress",
            "invalid networking address %s specified, "
            "switching to %s",
            addressString, addressOnError);
        result = inet_addr(addressOnError);
    }
    NW_CONFIDENCE(result != (in_addr_t)-1);
    
    return result;
}

sk_addressType
sk_getAddressType(
    const char *addressString)
{
    sk_addressType result = SK_TYPE_UNKNOWN;
    sk_address address;
    sk_address addressHostFormat;
    
    if (strcmp(addressString, NWCF_BROADCAST_EXPR) == 0) {
        result = SK_TYPE_BROADCAST;
    } else {
        address = sk_stringToAddress(addressString, "0.0.0.0");
        addressHostFormat = ntohl(address);
        if (IN_CLASSD(addressHostFormat)) {
            result = SK_TYPE_MULTICAST;
        } else if (addressHostFormat == INADDR_LOOPBACK) {
            result = SK_TYPE_LOOPBACK;
        }
    }
    
    return result;
}

/* interfaceInfo */

struct sk_interfaceInfo_s {
    sk_interfaceName  name;
    unsigned short    flags;
    struct sockaddr  *primaryAddress;
    struct sockaddr  *broadcastAddress;
};

static sk_interfaceInfo
sk_interfaceInfoNew(
    const sk_interfaceName name,
    const unsigned short flags,
    const struct sockaddr *primaryAddress,
    const struct sockaddr *broadcastAddress)
{
    sk_interfaceInfo result = NULL;
    
    result = (sk_interfaceInfo)os_malloc((os_uint32)sizeof(*result));
    
    if (result != NULL) {
        /* Copy fields */
        memcpy(result->name, name, SK_INTF_MAX_NAME_LEN);
        result->name[SK_INTF_MAX_NAME_LEN - 1] = '\0';
        
        result->flags = flags;
        
        if (primaryAddress && ((int)primaryAddress->sa_family == AF_INET)) {
            /* Only IPv4 for now */
            result->primaryAddress = (struct sockaddr *)os_malloc(
                                        (os_uint32)sizeof(struct sockaddr_in));
            if (result->primaryAddress) {
                memcpy(result->primaryAddress, primaryAddress, 
                       (os_uint32)sizeof(struct sockaddr_in));
            }
            /* If interface supports broadcast, fill bc info */
            if (broadcastAddress && SD_FLAG_IS_SET(flags, IFF_BROADCAST)) {
                result->broadcastAddress = (struct sockaddr *)os_malloc(
                                              (os_uint32)sizeof(struct sockaddr_in));
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
    int sockfd,
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
sk_interfaceWalkFillBC(
    os_ifAttributes *intf,
    int sockfd,
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
    int sockfd,
    void *actionArg)
{
    unsigned int *count = (unsigned int *)actionArg;

    (void)intf;
    (void)sockfd;

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


static void
sk_interfaceWalkFillMC(
    os_ifAttributes *intf,
    int sockfd,
    void *actionArg)
{
    sk_interfaceInfo **interfaceInfo = (sk_interfaceInfo **)actionArg;
    struct sockaddr *broadaddr;
    
    if (SD_FLAG_IS_SET(intf->flags, IFF_MULTICAST) &&
        !SD_FLAG_IS_SET(intf->flags, IFF_LOOPBACK)) {
        if (SD_FLAG_IS_SET(intf->flags, IFF_BROADCAST)) {
            broadaddr = &intf->broadcast_address;
        } else {
            broadaddr = NULL;
        }
        **interfaceInfo = sk_interfaceInfoNew(intf->name, intf->flags,
                              &intf->address, broadaddr);
         /* (*interfaceInfo)++ not allowed by QAC */
        *interfaceInfo = &((*interfaceInfo)[1]);
    }
}    


static void
sk_interfaceWalkCountLoopback(
    os_ifAttributes *intf,
    int sockfd,
    void *actionArg)
{
    unsigned int *count = (unsigned int *)actionArg;

    (void)sockfd;
    (void)intf;

    /* Only interested in broadcast-enabled interfaces for now */
    if (SD_FLAG_IS_SET(intf->flags, IFF_LOOPBACK)) {
        (*count)++;
    }
}    


static void
sk_interfaceWalkFillLoopback(
    os_ifAttributes *intf,
    int sockfd,
    void *actionArg)
{
    sk_interfaceInfo **interfaceInfo = (sk_interfaceInfo **)actionArg;
    
   (void)sockfd;

    if (SD_FLAG_IS_SET(intf->flags, IFF_LOOPBACK)) {
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
                 int sockfd,
                 void *actionArg);

static void
sk_interfaceIPv4Walk(
    os_ifAttributes *allInterfaces,
    os_uint32 nofInterfaces,
    int sockfd,
    const sk_interfaceWalkFunc action,
    void *actionArg)
{
    os_uint32 currentInterface = 0;
    
    if (action) {
        while (currentInterface < nofInterfaces) {
            if (SD_FLAG_IS_SET(allInterfaces[currentInterface].flags, IFF_UP)) {
                action(&allInterfaces[currentInterface], sockfd, actionArg);                
            }
            currentInterface++;
        }
    }
}

#undef SK_MAX
                      
int
sk_interfaceInfoRetrieveAllBC(
    sk_interfaceInfo **interfaceList /* [nofInterfaces */,
    unsigned int *nofInterfaces,
    int sockfd)
{
    os_result result;
    os_ifAttributes ifList[MAX_INTERFACES];
    os_uint32 nofIf = 0;
    sk_interfaceInfo *interfaceHelper;
#ifdef INTEGRITY
    char *addressLookingFor;
#endif
    
    *interfaceList = NULL;
    *nofInterfaces = 0;

    result = os_sockQueryInterfaces(&ifList[0], (os_uint32)MAX_INTERFACES, &nofIf);

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

int
sk_interfaceInfoRetrieveAllMC(
    sk_interfaceInfo **interfaceList /* [nofInterfaces */,
    unsigned int *nofInterfaces,
    int sockfd)
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
    
    result = os_sockQueryInterfaces(&ifList[0], (os_uint32)MAX_INTERFACES, &nofIf);
    
    if (result == os_resultSuccess) {
        /* Count the number of valid interfaces */
        sk_interfaceIPv4Walk(ifList, nofIf, sockfd, 
                             sk_interfaceWalkCountMC, nofInterfaces);
                                
        
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

int
sk_interfaceInfoRetrieveAllLoopback(
    sk_interfaceInfo **interfaceList /* [nofInterfaces */,
    unsigned int *nofInterfaces,
    int sockfd)
{
    os_result result;
    os_ifAttributes ifList[MAX_INTERFACES];
    os_uint32 nofIf;
    sk_interfaceInfo *interfaceHelper;
    
    *interfaceList = NULL;
    *nofInterfaces = 0;
    
    result = os_sockQueryInterfaces(&ifList[0], (os_uint32)MAX_INTERFACES, &nofIf);
    
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
    unsigned int nofInterfaces)
{
    unsigned int i;
    
    if (interfaceList) {
        for (i=0; i<nofInterfaces; i++) {
            sk_interfaceInfoFree(interfaceList[i]);
        }
        os_free(interfaceList);
    }
}
    

char *
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

                     
struct sockaddr *
sk_interfaceInfoGetPrimaryAddress(
    const sk_interfaceInfo interfaceInfo)
{
    struct sockaddr *result = NULL;
    
    if (interfaceInfo != NULL) {
        result = interfaceInfo->primaryAddress;
    }
    
    return result;
}
    
                     
struct sockaddr *
sk_interfaceInfoGetBroadcastAddress(
    const sk_interfaceInfo interfaceInfo)
{
    struct sockaddr *result = NULL;
    
    if (interfaceInfo != NULL) {
         result = interfaceInfo->broadcastAddress;
    }
    
    return result;
}    
