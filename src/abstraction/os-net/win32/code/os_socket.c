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

os_sockErrno
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

os_int32
os_sockSendto(
    os_socket s,
    const void *msg,
    os_uint len,
    const struct sockaddr *to,
    os_uint tolen)
{
    return sendto(s, msg, len, 0, to, tolen);
}

os_int32
os_sockRecvfrom(
    os_socket s,
    void *buf,
    os_uint len,
    struct sockaddr *from,
    os_uint *fromlen)
{
    return recvfrom(s, buf, len, 0, from, (int *)fromlen);
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

#define MAX_INTERFACES 64
#define INTF_MAX_NAME_LEN 16

os_result
os_sockQueryInterfaces(
    os_ifAttributes *ifList,
    os_uint listSize,
    os_uint *validElements)
{

    os_result result = os_resultSuccess;
    os_result addressInfoResult =0;
    INTERFACE_INFO *allInterfacesBuf;
    INTERFACE_INFO *intf;
    unsigned long returnedBytes;
    unsigned int listIndex;
    os_socket ifcs;
    int retVal, done;
    char* errorMessage;
    os_sockErrno errNo;

    PIP_ADAPTER_ADDRESSES pAddresses = NULL;
    PIP_ADAPTER_ADDRESSES pCurrAddresses = NULL;
    PIP_ADAPTER_UNICAST_ADDRESS pUnicast = NULL;
    unsigned long outBufLen = 0;
    /* Set the flags to pass to GetAdaptersAddresses*/
    unsigned long flags = GAA_FLAG_INCLUDE_PREFIX;
    /* Doesn't matter what value of family you use. WSAIoctl w/ SIO_GET_INTERFACE_LIST
    only returns IPv4 addresses */
    unsigned long family = AF_UNSPEC;
    int i = 0;

    *validElements = 0;
    listIndex = 0;

    outBufLen = sizeof (IP_ADAPTER_ADDRESSES);
    pAddresses = (IP_ADAPTER_ADDRESSES *) os_malloc(outBufLen);
    if (GetAdaptersAddresses(family, flags, NULL, pAddresses, &outBufLen)
                                == ERROR_BUFFER_OVERFLOW) {
        os_free(pAddresses);
        pAddresses = (IP_ADAPTER_ADDRESSES *) os_malloc(outBufLen);
    }
    addressInfoResult = GetAdaptersAddresses(family, flags, NULL, pAddresses, &outBufLen);

    /* List the IPv4 interfaces */
    ifcs = os_sockNew (AF_INET, SOCK_DGRAM);
    if (ifcs != INVALID_SOCKET) {
        allInterfacesBuf = os_malloc(MAX_INTERFACES * sizeof(INTERFACE_INFO));
        memset(allInterfacesBuf, 0, MAX_INTERFACES * sizeof(INTERFACE_INFO));
        retVal = WSAIoctl(ifcs, SIO_GET_INTERFACE_LIST, NULL, 0, allInterfacesBuf,
                     MAX_INTERFACES * sizeof(INTERFACE_INFO), &returnedBytes, 0, 0);

        if (retVal == SOCKET_ERROR && WSAGetLastError() == WSAEFAULT)
        {
            /* The buffer wasn't big enough. returnedBytes will now contain the required size
            so we can reallocate & try again */
            os_free(allInterfacesBuf);
            allInterfacesBuf = os_malloc(returnedBytes);
            memset(allInterfacesBuf, 0, returnedBytes);
            retVal = WSAIoctl(ifcs, SIO_GET_INTERFACE_LIST, NULL, 0, allInterfacesBuf,
                     returnedBytes, &returnedBytes, 0, 0);
        }

        if (retVal == SOCKET_ERROR)
        {
            errNo = os_sockError();
            errorMessage = os_sockErrnoToString(errNo);
            os_report(OS_ERROR, "os_sockQueryInterfaces", __FILE__, __LINE__, 0,
                      "Socket error calling WSAIoctl for IPv4 interfaces: %d %s", errNo, errorMessage);
            os_free(errorMessage);
            /* @todo Is it right to return a fail here ? Need to check on box w/ no IPv4 interfaces */
            result = os_resultFail;
        }
        else
        {
            *validElements = returnedBytes/sizeof(INTERFACE_INFO);
        }
        while ((listIndex < listSize) && (listIndex < *validElements)) {
            done = 0;
            intf = &allInterfacesBuf[listIndex];

            if (addressInfoResult == NO_ERROR) {
                pCurrAddresses = pAddresses;
                while (pCurrAddresses && !done) {
                    /* adapter needs to be enabled*/
                    if (pCurrAddresses->OperStatus == IfOperStatusUp) {
                        pUnicast = pCurrAddresses->FirstUnicastAddress;
                        while (pUnicast && !done) {
                            /* check if interface ip matches adapter ip */
                            if (os_sockaddrIPAddressEqual((os_sockaddr*) &intf->iiAddress.AddressIn,
                                                           (os_sockaddr*) pUnicast->Address.lpSockaddr))
                            {
                                snprintf(ifList[listIndex].name,
                                          OS_IFNAMESIZE, "%wS",
                                          pCurrAddresses->FriendlyName);
                                ifList[listIndex].interfaceIndexNo =
                                            (os_uint) pCurrAddresses->Ipv6IfIndex;
                                done = 1;
                            }
                            pUnicast = pUnicast->Next;
                        }
                    }
                    pCurrAddresses = pCurrAddresses->Next;
                }
            }
            /* if no name is found set this */
            if (!done) {
                snprintf(ifList[listIndex].name,
                          OS_IFNAMESIZE, "0x%x",
                          ntohl(intf->iiAddress.AddressIn.sin_addr.S_un.S_addr));
                os_report(OS_WARNING,
                         "os_sockQueryInterfaces", __FILE__, __LINE__, 0,
                         "Unable to determine IPv4 adapter name. Setting instead to adapter address %s",
                         ifList[listIndex].name);
            }
            ifList[listIndex].flags = intf->iiFlags;
            ifList[listIndex].address = *((os_sockaddr_storage*) &intf->iiAddress);
            ifList[listIndex].broadcast_address = *((os_sockaddr_storage*) &intf->iiBroadcastAddress);
            ((os_sockaddr_in *)(&(ifList[listIndex].broadcast_address)))->sin_addr.S_un.S_addr =
                ((os_sockaddr_in *)(&(ifList[listIndex].address)))->sin_addr.S_un.S_addr |
                    ~(intf->iiNetmask.AddressIn.sin_addr.S_un.S_addr);
            ifList[listIndex].network_mask = *((os_sockaddr_storage*) &intf->iiNetmask);

            listIndex++;
        }
        os_sockFree (ifcs);
    }

    if (addressInfoResult == NO_ERROR) {
        os_free(pAddresses);
    }

    return result;
}

os_result
os_sockQueryIPv6Interfaces(
    os_ifAttributes *ifList,
    os_uint32 listSize,
    os_uint32 *validElements)
{
    os_result result = os_resultSuccess;
    os_result addressInfoResult =0;
    unsigned long returnedBytes;
    unsigned int listIndex;
    os_socket ifcs;
    int retVal, done;
    char* errorMessage;
    os_sockErrno errNo;

    PIP_ADAPTER_ADDRESSES pAddresses = NULL;
    PIP_ADAPTER_ADDRESSES pCurrAddresses = NULL;
    PIP_ADAPTER_UNICAST_ADDRESS pUnicast = NULL;
    unsigned long outBufLen = 0;
    /* Set the flags to pass to GetAdaptersAddresses*/
    unsigned long flags = GAA_FLAG_INCLUDE_PREFIX;
    int i = 0;

    /* IPv6 addition */
    SOCKET_ADDRESS_LIST* ipv6InterfaceList;

    *validElements = 0;
    listIndex = 0;

    outBufLen = sizeof (IP_ADAPTER_ADDRESSES);
    pAddresses = (IP_ADAPTER_ADDRESSES *) os_malloc(outBufLen);
    if (GetAdaptersAddresses(AF_INET6, flags, NULL, pAddresses, &outBufLen)
                                == ERROR_BUFFER_OVERFLOW) {
        os_free(pAddresses);
        pAddresses = (IP_ADAPTER_ADDRESSES *) os_malloc(outBufLen);
    }
    addressInfoResult = GetAdaptersAddresses(AF_INET6, flags, NULL, pAddresses, &outBufLen);

    /* Now do the IPv6 interfaces */
    ifcs = os_sockNew (AF_INET6, SOCK_DGRAM);

    if (ifcs != INVALID_SOCKET)
    {
        /* List returned from this control code query will need to be sized as 1 * SOCKET_ADDRESS_LIST + n * SOCKET_ADDRESS */
        ipv6InterfaceList = os_malloc(sizeof(SOCKET_ADDRESS_LIST) + ((MAX_INTERFACES - 1) * sizeof(SOCKET_ADDRESS)));
        memset(ipv6InterfaceList, 0, sizeof(SOCKET_ADDRESS_LIST) + ((MAX_INTERFACES - 1) * sizeof(SOCKET_ADDRESS)));
        retVal = WSAIoctl(ifcs, SIO_ADDRESS_LIST_QUERY, NULL, 0, ipv6InterfaceList,
                            sizeof(SOCKET_ADDRESS_LIST) + ((MAX_INTERFACES - 1) * sizeof(SOCKET_ADDRESS)),
                            &returnedBytes, 0, 0);

        if (retVal == SOCKET_ERROR && WSAGetLastError() == WSAEFAULT)
        {
            /* The buffer wasn't big enough. returnedBytes will now contain the required size
            so we can reallocate & try again */
            os_free(ipv6InterfaceList);
            ipv6InterfaceList = os_malloc(returnedBytes);
            memset(ipv6InterfaceList, 0, returnedBytes);
            retVal = WSAIoctl(ifcs, SIO_ADDRESS_LIST_QUERY, NULL, 0, ipv6InterfaceList,
                                returnedBytes, &returnedBytes, 0, 0);
        }

        if (retVal == SOCKET_ERROR)
        {
            errNo = os_sockError();
            errorMessage = os_sockErrnoToString(errNo);
            os_report(OS_ERROR, "os_sockQueryInterfaces", __FILE__, __LINE__, 0,
                      "Socket error calling WSAIoctl for IPv6 interfaces: %d %s", errNo, errorMessage);
            os_free(errorMessage);
            /* @todo Is it right to return a fail here ? Need to check on box w/ no IPv6 interfaces */
            result = os_resultFail;
        }
        else
        {
            for (i = 0; i < ipv6InterfaceList->iAddressCount; ++i)
            {
                if (ipv6InterfaceList->Address[i].lpSockaddr->sa_family == AF_INET6
                    && ! (IN6_IS_ADDR_UNSPECIFIED(&((os_sockaddr_in6 *)&ipv6InterfaceList->Address[i].lpSockaddr)->sin6_addr)))
                {
                    done = 0;
                    if (addressInfoResult == NO_ERROR) {
                        pCurrAddresses = pAddresses;
                        while (pCurrAddresses && !done) {
                            /* adapter needs to be enabled*/
                            if (pCurrAddresses->OperStatus == IfOperStatusUp) {
                                pUnicast = pCurrAddresses->FirstUnicastAddress;
                                while (pUnicast && !done) {
                                    /* check if interface ip matches adapter ip */
                                    if (os_sockaddrIPAddressEqual((os_sockaddr*) ipv6InterfaceList->Address[i].lpSockaddr,
                                                                   (os_sockaddr*) pUnicast->Address.lpSockaddr))
                                    {
                                        snprintf(ifList[listIndex].name,
                                                  OS_IFNAMESIZE, "%wS",
                                                  pCurrAddresses->FriendlyName);
                                        ifList[listIndex].interfaceIndexNo =
                                            (os_uint) pCurrAddresses->Ipv6IfIndex;
                                        done = 1;
                                    }
                                    pUnicast = pUnicast->Next;
                                }
                            }
                            pCurrAddresses = pCurrAddresses->Next;
                        }
                    }

                    /* if no name was found set this interface name to string
                    representation of the IPv6 address */
                    if (!done) {
                        os_sockaddrAddressToString((os_sockaddr*) ipv6InterfaceList->Address[i].lpSockaddr,
                                                    ifList[listIndex].name,
                                                    OS_IFNAMESIZE);
                        os_report(OS_WARNING,
                                 "os_sockQueryInterfaces", __FILE__, __LINE__, 0,
                                 "Unable to determine IPv6 adapter name. Setting instead to adapter address %s",
                                 ifList[listIndex].name);
                    }

                    ifList[listIndex].flags = 0;
                    ifList[listIndex].address = *((os_sockaddr_storage*) ipv6InterfaceList->Address[i].lpSockaddr);
                    listIndex++;
                    ++(*validElements);
                }
            }
        }
        os_sockFree (ifcs);
    }

    if (addressInfoResult == NO_ERROR) {
        os_free(pAddresses);
    }

    return result;
}


/* We need this on windows to make sure the main thread of MFC applications
 * calls os_osInit().
 */
BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  /* handle to DLL module */
    DWORD fdwReason,     /* reason for calling function */
    LPVOID lpReserved )  /* reserved */
{
    /* Perform actions based on the reason for calling.*/
    switch( fdwReason ) {
    case DLL_PROCESS_ATTACH:
        /* Initialize once for each new process.
         * Return FALSE to fail DLL load.
         */
        /* os_init will be called when the library is loaded */
        os_socketModuleInit();
    break;
    case DLL_THREAD_ATTACH:
         /* Do thread-specific initialization.
          */
    break;
    case DLL_THREAD_DETACH:
         /* Do thread-specific cleanup.
          */
    break;
    case DLL_PROCESS_DETACH:
        /* Perform any necessary cleanup.
         */
        os_socketModuleExit();
    break;
    }
    return TRUE;  /* Successful DLL_PROCESS_ATTACH.*/
}

#undef MAX_INTERFACES
