/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
/** \file
 *  \brief WIN32 socket management
 *
 * Implements socket management for WIN32
 */
#include <stdio.h>

#include "code/os__socket.h"

#include "os_report.h"
#include "os_heap.h"
#include "os_stdlib.h"
#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")

#define WORKING_BUFFER_SIZE 15000
#define MAX_TRIES 3

void
os_socketModuleInit()
{
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    wVersionRequested = MAKEWORD (OS_SOCK_VERSION, OS_SOCK_REVISION);

    err = WSAStartup (wVersionRequested, &wsaData);
    if (err != 0) {
    OS_REPORT (OS_FATAL, "os_socketModuleInit", 0, "WSAStartup failed, no compatible socket implementation available");
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
        return;
    }

    /* Confirm that the WinSock DLL supports 2.0.    */
    /* Note that if the DLL supports versions greater    */
    /* than 2.0 in addition to 2.0, it will still return */
    /* 2.0 in wVersion since that is the version we      */
    /* requested.                                        */

    if ((LOBYTE(wsaData.wVersion) != OS_SOCK_VERSION) ||
        (HIBYTE(wsaData.wVersion) != OS_SOCK_REVISION)) {
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
    OS_REPORT (OS_FATAL, "os_socketModuleInit", 1, "WSAStartup failed, no compatible socket implementation available");
        WSACleanup();
        return;
    }
}

void
os_socketModuleExit(void)
{
    WSACleanup();
    return;
}

int
os_sockError(void)
{
    return WSAGetLastError();
}

os_socket
os_sockNew(
    int domain,
    int type)
{
    return socket(domain, type, 0);
}

os_result
os_sockBind(
    os_socket s,
    const struct sockaddr *name,
    os_uint namelen)
{
    os_result result = os_resultSuccess;

    if (bind(s, (struct sockaddr *)name, namelen) == -1) {
        result = os_resultFail;
    }
    return result;
}

os_result
os_sockGetsockname(
    os_socket s,
    const struct sockaddr *name,
    os_uint namelen)
{
    os_result result = os_resultSuccess;
    int len = namelen;

    if (getsockname(s, (struct sockaddr *)name, &len) == -1) {
        result = os_resultFail;
    }
    return result;
}

os_result
os_sockSendto(
    os_socket s,
    const void *msg,
    size_t len,
    const struct sockaddr *to,
    size_t tolen,
    size_t *bytesSent)
{
    int res = sendto(s, msg, len, 0, to, tolen);
    if (res < 0)
    {
        *bytesSent = 0;
        return os_resultFail;
    }
    else
    {
        *bytesSent = res;
        return os_resultSuccess;
    }
}

os_result
os_sockRecvfrom(
    os_socket s,
    void *buf,
    size_t len,
    struct sockaddr *from,
    size_t *fromlen,
    size_t *bytesRead)
{
    int res;
    res = recvfrom(s, buf, (int)len, 0, from, (int *)fromlen);
    if (res == SOCKET_ERROR)
    {
        *bytesRead = 0;
        return os_resultFail;
    }
    else
    {
        *bytesRead = (size_t)res;
        return os_resultSuccess;
    }
}

os_result
os_sockGetsockopt(
    os_socket s,
    os_int32 level,
    os_int32 optname,
    void *optval,
    os_uint *optlen)
{
    os_result result = os_resultSuccess;

    /* On win32 IP_MULTICAST_TTL and IP_MULTICAST_LOOP take DWORD * param
       rather than char * */
    if ( level == IPPROTO_IP
     && ( optname == IP_MULTICAST_TTL
          || optname == IP_MULTICAST_LOOP ) )
    {
       int dwoptlen = sizeof( DWORD );
       DWORD dwoptval = *((os_uchar *)optval);
       if (getsockopt(s, level, optname, (char *)&dwoptval, &dwoptlen) == -1)
       {
          result = os_resultFail;
       }

       assert( dwoptlen == sizeof( DWORD ) );
       *((os_uchar *)optval) = (os_uchar)dwoptval;
       *optlen = sizeof( os_uchar );
    }
    else
    {
       if (getsockopt(s, level, optname, optval, (int *)optlen) == -1)
       {
          result = os_resultFail;
       }
    }

    return result;
}

os_result
os_sockSetsockopt(
    os_socket s,
    os_int32 level,
    os_int32 optname,
    const void *optval,
    os_uint optlen)
{
    os_result result = os_resultSuccess;
    DWORD dwoptval;

    /* On win32 IP_MULTICAST_TTL and IP_MULTICAST_LOOP take DWORD * param
       rather than char * */
    if ( level == IPPROTO_IP
     && ( optname == IP_MULTICAST_TTL
          || optname == IP_MULTICAST_LOOP ) )
    {
       dwoptval = *((os_uchar *)optval);
       optval = &dwoptval;
       optlen = sizeof( DWORD );
    }
    if (setsockopt(s, level, optname, optval, (int)optlen) == -1) {
        result = os_resultFail;
    }
    return result;
}

os_result
os_sockSetNonBlocking(
    os_socket s,
    os_boolean nonblock)
{
    int result;
    u_long mode;
    os_result r;

    assert(nonblock == OS_FALSE || nonblock == OS_TRUE);

    /* If mode = 0, blocking is enabled,
     * if mode != 0, non-blocking is enabled. */
    mode = (nonblock == OS_TRUE) ? 1 : 0;

    result = ioctlsocket(s, FIONBIO, &mode);
    if (result != SOCKET_ERROR){
        r = os_resultSuccess;
    } else {
        switch(WSAGetLastError()){
            case WSAEINPROGRESS:
                r = os_resultBusy;
                break;
            case WSAENOTSOCK:
                r = os_resultInvalid;
                break;
            case WSANOTINITIALISED:
                OS_REPORT (OS_FATAL, "os_sockSetNonBlocking", 0, "Socket-module not initialised; ensure os_socketModuleInit is performed before using the socket module.");
            default:
                r = os_resultFail;
                break;
        }
    }

    return r;
}

os_result
os_sockFree(
    os_socket s)
{
    os_result result = os_resultSuccess;

    if (closesocket(s) == -1) {
        result = os_resultFail;
    }
    return result;
}

os_int32
os_sockSelect(
    os_int32 nfds,
    fd_set *readfds,
    fd_set *writefds,
    fd_set *errorfds,
    os_time *timeout)
{
    struct timeval t;
    int r;

    t.tv_sec = (long)timeout->tv_sec;
    t.tv_usec = (long)(timeout->tv_nsec / 1000);
    r = select(nfds, readfds, writefds, errorfds, &t);

    return r;
}

static unsigned int
getInterfaceFlags(PIP_ADAPTER_ADDRESSES pAddr)
{
    unsigned int flags = 0;

    if (pAddr->OperStatus == IfOperStatusUp) {
        flags |= IFF_UP;
    }

    if (pAddr->IfType == IF_TYPE_SOFTWARE_LOOPBACK) {
        flags |= IFF_LOOPBACK;
    }

    if (!(pAddr->Flags & IP_ADAPTER_NO_MULTICAST)) {
        flags |= IFF_MULTICAST;
    }

    switch (pAddr->IfType) {
    case IF_TYPE_ETHERNET_CSMACD:
    case IF_TYPE_IEEE80211:
    case IF_TYPE_IEEE1394:
    case IF_TYPE_ISO88025_TOKENRING:
        flags |= IFF_BROADCAST;
        break;
    default:
        flags |= IFF_POINTTOPOINT;
        break;
    }

    return flags;
}

static os_result
addressToIndexAndMask(struct sockaddr *addr, unsigned int *ifIndex, struct sockaddr *mask )
{
    os_result result = os_resultSuccess;
    os_boolean found = OS_FALSE;
    PMIB_IPADDRTABLE pIPAddrTable = NULL;
    DWORD dwSize = 0;
    DWORD i;
    char* errorMessage;
    int errNo;

    if (GetIpAddrTable(pIPAddrTable, &dwSize, 0) == ERROR_INSUFFICIENT_BUFFER) {
        pIPAddrTable = (MIB_IPADDRTABLE *) os_malloc(dwSize);
        if (pIPAddrTable != NULL) {
            if (GetIpAddrTable(pIPAddrTable, &dwSize, 0) != NO_ERROR) {
                errNo = os_sockError();
                errorMessage = os_reportErrnoToString(errNo);
                os_report(OS_ERROR, "addressToIndexAndMask", __FILE__, __LINE__, 0,
                      "GetIpAddrTable failed: %d %s", errNo, errorMessage);
                os_free(errorMessage);
                result = os_resultFail;
            }
        } else {
            os_report(OS_ERROR, "addressToIndexAndMask", __FILE__, __LINE__, 0,
                "Failed to allocate %d bytes for IP address table", dwSize);
            result = os_resultFail;
        }
    } else {
        errNo = os_sockError();
        errorMessage = os_reportErrnoToString(errNo);
        os_report(OS_ERROR, "addressToIndexAndMask", __FILE__, __LINE__, 0,
                    "GetIpAddrTable failed: %d %s", errNo, errorMessage);
        os_free(errorMessage);
        result = os_resultFail;
    }

    if (result == os_resultSuccess) {
        for (i = 0; !found && i < pIPAddrTable->dwNumEntries; i++ ) {
            if (((struct sockaddr_in* ) addr )->sin_addr.s_addr == pIPAddrTable->table[i].dwAddr) {
                *ifIndex = pIPAddrTable->table[i].dwIndex;
                ((struct sockaddr_in*) mask)->sin_addr.s_addr= pIPAddrTable->table[i].dwMask;
                found = OS_TRUE;
            }
        }
    }

    if (pIPAddrTable) {
        os_free(pIPAddrTable);
    }

    if (!found) {
        result = os_resultFail;
    }

    return result;
}



#define MAX_INTERFACES 64
#define INTF_MAX_NAME_LEN 16

os_result
os_sockQueryInterfaces(
    os_ifAttributes *ifList,
    unsigned int listSize,
    unsigned int *validElements)
{
    os_result result = os_resultSuccess;
    DWORD filter;
    PIP_ADAPTER_ADDRESSES pAddresses = NULL;
    PIP_ADAPTER_ADDRESSES pCurrAddress = NULL;
    PIP_ADAPTER_UNICAST_ADDRESS pUnicast = NULL;
    unsigned long outBufLen = WORKING_BUFFER_SIZE;
    int retVal;
    int iterations = 0;
    int listIndex = 0;

    filter = GAA_FLAG_INCLUDE_PREFIX | GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER;

    do {
        pAddresses = (IP_ADAPTER_ADDRESSES *) os_malloc(outBufLen);
        if (!pAddresses) {
            os_report(OS_ERROR, "os_sockQueryInterfaces", __FILE__, __LINE__, 0,
                "Failed to allocate %d bytes for Adapter addresses", outBufLen);
            return os_resultFail;
        }
        retVal = GetAdaptersAddresses(AF_INET, filter, NULL, pAddresses, &outBufLen);
        if (retVal == ERROR_BUFFER_OVERFLOW) {
            os_free(pAddresses);
            pAddresses = NULL;
            outBufLen <<= 1; /* double the buffer just to be save.*/
        } else {
            break;
        }
        iterations++;
    } while ((retVal == ERROR_BUFFER_OVERFLOW) && (iterations < MAX_TRIES));

    if (retVal != ERROR_SUCCESS) {
        if (pAddresses) {
            os_free(pAddresses);
            pAddresses = NULL;
        }
        os_report(OS_ERROR, "os_sockQueryInterfaces", __FILE__, __LINE__, 0,
                "Failed to GetAdaptersAddresses");
        return os_resultFail;
    }

    for (pCurrAddress = pAddresses; pCurrAddress; pCurrAddress = pCurrAddress->Next) {
        IP_ADAPTER_PREFIX *firstPrefix = NULL;

        if (pCurrAddress->Length >= sizeof(IP_ADAPTER_ADDRESSES)) {
            firstPrefix = pCurrAddress->FirstPrefix;
        }

        if (pCurrAddress->OperStatus != IfOperStatusUp) {
            continue;
        }

        for (pUnicast = pCurrAddress->FirstUnicastAddress; pUnicast; pUnicast = pUnicast->Next) {
            unsigned int ipv4Index;
            struct sockaddr_in ipv4Netmask;

            if (pUnicast->Address.lpSockaddr->sa_family != AF_INET) {
                continue;
            }

            ipv4Index = 0;
            memset(&ipv4Netmask, 0, sizeof(ipv4Netmask));
            if (addressToIndexAndMask((struct sockaddr *) pUnicast->Address.lpSockaddr,
                        &ipv4Index, (struct sockaddr *) &ipv4Netmask) != os_resultSuccess) {
                continue;
            }

            snprintf(ifList[listIndex].name, OS_IFNAMESIZE, "%wS", pCurrAddress->FriendlyName);

            // Get interface flags.
            ifList[listIndex].flags = getInterfaceFlags(pCurrAddress);
            ifList[listIndex].interfaceIndexNo = ipv4Index;

            memcpy(&ifList[listIndex].address, pUnicast->Address.lpSockaddr, pUnicast->Address.iSockaddrLength);
            memcpy(&ifList[listIndex].broadcast_address, pUnicast->Address.lpSockaddr, pUnicast->Address.iSockaddrLength);
            memcpy(&ifList[listIndex].network_mask, pUnicast->Address.lpSockaddr, pUnicast->Address.iSockaddrLength);

            ((struct sockaddr_in *)(&(ifList[listIndex].broadcast_address)))->sin_addr.s_addr =
                ((struct sockaddr_in *)(&(ifList[listIndex].address)))->sin_addr.s_addr | ~(ipv4Netmask.sin_addr.s_addr);
            ((struct sockaddr_in *)&(ifList[listIndex].network_mask))->sin_addr.s_addr = ipv4Netmask.sin_addr.s_addr;

            listIndex++;
        }
    }

    for (pCurrAddress = pAddresses; pCurrAddress; pCurrAddress = pCurrAddress->Next) {
        if (pCurrAddress->OperStatus != IfOperStatusUp) {
            snprintf(ifList[listIndex].name, OS_IFNAMESIZE, "%wS", pCurrAddress->FriendlyName);

            // Get interface flags.
            ifList[listIndex].flags = getInterfaceFlags(pCurrAddress);
            ifList[listIndex].interfaceIndexNo = 0;
            memset (&ifList[listIndex].address, 0, sizeof(ifList[listIndex].address));
            memset (&ifList[listIndex].broadcast_address, 0, sizeof (ifList[listIndex].broadcast_address));
            memset (&ifList[listIndex].network_mask, 0, sizeof (ifList[listIndex].network_mask));

            listIndex++;
        }
    }

    if (pAddresses) {
        os_free(pAddresses);
    }

    *validElements = listIndex;

    return result;
}

os_result
os_sockQueryIPv6Interfaces (
    os_ifAttributes *ifList,
    unsigned int listSize,
    unsigned int *validElements)
{
    os_result result = os_resultSuccess;
    ULONG filter;
    PIP_ADAPTER_ADDRESSES pAddresses = NULL;
    PIP_ADAPTER_ADDRESSES pCurrAddress = NULL;
    PIP_ADAPTER_UNICAST_ADDRESS pUnicast = NULL;
    ULONG outBufLen = WORKING_BUFFER_SIZE;
    ULONG retVal;
    int iterations = 0;
    int listIndex = 0;

    filter = GAA_FLAG_INCLUDE_PREFIX | GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER;

    do {
        pAddresses = (IP_ADAPTER_ADDRESSES *) os_malloc(outBufLen);
        if (!pAddresses) {
            os_report(OS_ERROR, "os_sockQueryIPv6Interfaces", __FILE__, __LINE__, 0,
                "Failed to allocate %d bytes for Adapter addresses", outBufLen);
            return os_resultFail;
        }
        retVal = GetAdaptersAddresses(AF_INET6, filter, NULL, pAddresses, &outBufLen);
        if (retVal == ERROR_BUFFER_OVERFLOW) {
            os_free(pAddresses);
            pAddresses = NULL;
            outBufLen <<= 1; /* double the buffer just to be save.*/
        } else {
            break;
        }
        iterations++;
    } while ((retVal == ERROR_BUFFER_OVERFLOW) && (iterations < MAX_TRIES));

    if (retVal != ERROR_SUCCESS) {
        if (pAddresses) {
            os_free(pAddresses);
            pAddresses = NULL;
        }
        os_report(OS_ERROR, "os_sockQueryIPv6Interfaces", __FILE__, __LINE__, 0,
                "Failed to GetAdaptersAddresses");
        return os_resultFail;
    }

    for (pCurrAddress = pAddresses; pCurrAddress; pCurrAddress = pCurrAddress->Next) {
        DWORD ipv6IfIndex  = 0;
        IP_ADAPTER_PREFIX *firstPrefix = NULL;

        if (pCurrAddress->Length >= sizeof(IP_ADAPTER_ADDRESSES)) {
            ipv6IfIndex = pCurrAddress->Ipv6IfIndex;
            firstPrefix = pCurrAddress->FirstPrefix;
        }

        if (((ipv6IfIndex == 1) && (pCurrAddress->IfType != IF_TYPE_SOFTWARE_LOOPBACK)) || (pCurrAddress->IfType == IF_TYPE_TUNNEL)) {
            continue;
        }

        if (pCurrAddress->OperStatus != IfOperStatusUp) {
            continue;
        }

        for (pUnicast = pCurrAddress->FirstUnicastAddress; pUnicast; pUnicast = pUnicast->Next) {
            IP_ADAPTER_PREFIX *prefix;
            IN6_ADDR mask;
            struct sockaddr_in6 *sa6;
            struct sockaddr_in6 ipv6Netmask;

            if (pUnicast->Address.lpSockaddr->sa_family != AF_INET6) {
                continue;
            }

            snprintf(ifList[listIndex].name, OS_IFNAMESIZE, "%wS", pCurrAddress->FriendlyName);

            // Get interface flags.
            ifList[listIndex].flags = getInterfaceFlags(pCurrAddress);
            ifList[listIndex].interfaceIndexNo = (os_uint) pCurrAddress->Ipv6IfIndex;

            memcpy(&ifList[listIndex].address, pUnicast->Address.lpSockaddr, pUnicast->Address.iSockaddrLength);
            memcpy(&ifList[listIndex].broadcast_address, pUnicast->Address.lpSockaddr, pUnicast->Address.iSockaddrLength);
            memcpy(&ifList[listIndex].network_mask, pUnicast->Address.lpSockaddr, pUnicast->Address.iSockaddrLength);

            sa6 = (struct sockaddr_in6 *)&ifList[listIndex].network_mask;
            memset(&sa6->sin6_addr.s6_addr, 0xFF, sizeof(sa6->sin6_addr.s6_addr));

            for (prefix = firstPrefix; prefix; prefix = prefix->Next) {
                unsigned int l, i;
                if ((prefix->PrefixLength == 0) || (prefix->PrefixLength > 128) ||
                    (pUnicast->Address.iSockaddrLength != prefix->Address.iSockaddrLength) ||
                    (memcmp(pUnicast->Address.lpSockaddr, prefix->Address.lpSockaddr, pUnicast->Address.iSockaddrLength) == 0)){
                    continue;
                }

                memset(&ipv6Netmask, 0, sizeof(ipv6Netmask));
                ipv6Netmask.sin6_family = AF_INET6;

                l = prefix->PrefixLength;
                for (i = 0; l > 0; l -= 8, i++) {
                    ipv6Netmask.sin6_addr.s6_addr[i] = (l >= 8) ? 0xFF : ((0xFF << (8-l)) & 0xFF);
                }

                for (i = 0; i < 16; i++) {
                    mask.s6_addr[i] =
                        ((struct sockaddr_in6 *)pUnicast->Address.lpSockaddr)->sin6_addr.s6_addr[i] & ipv6Netmask.sin6_addr.s6_addr[i];
                }

                if (memcmp(((struct sockaddr_in6 *)prefix->Address.lpSockaddr)->sin6_addr.s6_addr,
                            mask.s6_addr, sizeof(ipv6Netmask.sin6_addr)) == 0) {
                    memcpy(&sa6->sin6_addr.s6_addr, &ipv6Netmask.sin6_addr.s6_addr, sizeof(sa6->sin6_addr.s6_addr));
                }
            }
            listIndex++;
        }
    }

    for (pCurrAddress = pAddresses; pCurrAddress; pCurrAddress = pCurrAddress->Next) {
        if (pCurrAddress->OperStatus != IfOperStatusUp) {
            snprintf(ifList[listIndex].name, OS_IFNAMESIZE, "%wS", pCurrAddress->FriendlyName);

              // Get interface flags.
              ifList[listIndex].flags = getInterfaceFlags(pCurrAddress);
              ifList[listIndex].interfaceIndexNo = 0;
              memset (&ifList[listIndex].address, 0, sizeof(ifList[listIndex].address));
              memset (&ifList[listIndex].broadcast_address, 0, sizeof (ifList[listIndex].broadcast_address));
              memset (&ifList[listIndex].network_mask, 0, sizeof (ifList[listIndex].network_mask));

              listIndex++;
        }
    }

    if (pAddresses) {
        os_free(pAddresses);
    }

    *validElements = listIndex;

    return result;
}

static os_result
os_sockGetInterfaceStatus (
    const char *ifName,
    os_boolean *status)
{
    os_boolean result = os_resultSuccess;
    PIP_ADAPTER_ADDRESSES pAddresses = NULL;
    PIP_ADAPTER_ADDRESSES pCurrAddress = NULL;
    PIP_ADAPTER_UNICAST_ADDRESS pUnicast = NULL;
    unsigned long outBufLen = WORKING_BUFFER_SIZE;
    int retVal;
    int iterations = 0;
    int listIndex = 0;

    do {
        pAddresses = (IP_ADAPTER_ADDRESSES *) os_malloc(outBufLen);
        if (!pAddresses) {
            os_report(OS_ERROR, "os_sockQueryInterfaces", __FILE__, __LINE__, 0,
                "Failed to allocate %d bytes for Adapter addresses", outBufLen);
            return os_resultFail;
        }
        retVal = GetAdaptersAddresses(AF_INET, 0, NULL, pAddresses, &outBufLen);
        if (retVal == ERROR_BUFFER_OVERFLOW) {
            os_free(pAddresses);
            pAddresses = NULL;
            outBufLen <<= 1; /* double the buffer just to be save.*/
        } else {
            break;
        }
        iterations++;
    } while ((retVal == ERROR_BUFFER_OVERFLOW) && (iterations < MAX_TRIES));

    for (pCurrAddress = pAddresses; pCurrAddress; pCurrAddress = pCurrAddress->Next) {
        char buffer[OS_IFNAMESIZE];
        snprintf(buffer, sizeof(buffer), "%wS", pCurrAddress->FriendlyName);
        if (strcmp(ifName, buffer) == 0) {
            if (pCurrAddress->OperStatus == IfOperStatusUp) {
                *status = OS_TRUE;
                break;
            }
        }
    }

    if (pAddresses) {
        os_free(pAddresses);
    }

    return result;
}

typedef struct os_sockQueryInterfaceStatusInfo_s {
    char *ifName;
    OVERLAPPED overlap;
} os_sockQueryInterfaceStatusInfo;


void
os_sockQueryInterfaceStatusDeinit(
    void *handle)
{
    os_sockQueryInterfaceStatusInfo *info = (os_sockQueryInterfaceStatusInfo *) handle;

    if (info) {
        if (info->ifName) {
            os_free(info->ifName);
        }
        CancelIPChangeNotify(&info->overlap);
        os_free(info);
    }
}

static void
os_sockQueryInterfaceStatusReset(
    os_sockQueryInterfaceStatusInfo *info)
{
    HANDLE hand = NULL;
    DWORD ret;

    (void)WSAResetEvent(info->overlap.hEvent);
    (void)CancelIPChangeNotify(&info->overlap);
    ret = NotifyAddrChange(&hand, &info->overlap);
    if (ret != NO_ERROR) {
        if (WSAGetLastError() != WSA_IO_PENDING) {
            os_report(OS_ERROR, "os_sockQueryInterfaceStatusReset", __FILE__, __LINE__, 0,
                          "Failed to reset notifications for network interface address changes");
        }
    }
}

void *
os_sockQueryInterfaceStatusInit(
    const char *ifName)
{
    os_sockQueryInterfaceStatusInfo *info = NULL;
    HANDLE hand = NULL;
    DWORD ret;

    info = (os_sockQueryInterfaceStatusInfo *) os_malloc(sizeof(os_sockQueryInterfaceStatusInfo));
    if (info) {
        memset(info, 0, sizeof(os_sockQueryInterfaceStatusInfo));
        info->ifName = os_strdup(ifName);
        if (!info->ifName) {
            os_free(info);
            info = NULL;
            os_report(OS_ERROR, "os_sockQueryInterfaceStatusInit", __FILE__, __LINE__, 0,
                      "Failed to allocate os_sockQueryInterfaceStatusInfo");
        }
    }

    if (info) {
        info->overlap.hEvent = WSACreateEvent();
        ret = NotifyAddrChange(&hand, &info->overlap);
        if (ret != NO_ERROR) {
            if (WSAGetLastError() != WSA_IO_PENDING) {
                os_free(info->ifName);
                os_free(info);
                info = NULL;
                os_report(OS_ERROR, "os_sockQueryInterfaceStatusInit", __FILE__, __LINE__, 0,
                          "Failed to administer for network interface address changes");
            }
        }
    }

    return info;
}


os_result
os_sockQueryInterfaceStatus(
    void *handle,
    os_time timeout,
    os_boolean *status)
{
    os_sockQueryInterfaceStatusInfo *info = (os_sockQueryInterfaceStatusInfo *) handle;
    os_result result = os_resultFail;
    DWORD t;

    *status = OS_FALSE;

    if (info) {
        assert(timeout.tv_sec >= 0);
        t = (DWORD)timeout.tv_sec;
        t = t * 1000 + timeout.tv_nsec / 1000000;

        if (WaitForSingleObject(info->overlap.hEvent, t) == WAIT_OBJECT_0) {
            result = os_sockGetInterfaceStatus(info->ifName, status);
            os_sockQueryInterfaceStatusReset(info);
        } else {
            result = os_resultTimeout;
        }
    }

    return result;
}

#undef MAX_INTERFACES
