/** \file os/win32/code/os_socket.c
 *  \brief WIN32 socket management
 *
 * Implements socket management for WIN32
 */
#include <stdio.h>

#include "code/os__socket.h"

#include <os_report.h>
#include <os_heap.h>
#include <os_stdlib.h>

static reportfn_t os_report;
static mallocfn_t os_malloc;


void
os_socketModuleInit( reportfn_t reportfn, mallocfn_t mallocfn )
{
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;
    os_report = reportfn;
    os_malloc = mallocfn;

    wVersionRequested = MAKEWORD (OS_SOCK_VERSION, OS_SOCK_REVISION);
 
    err = WSAStartup (wVersionRequested, &wsaData);
    if (err != 0) {
	OS_REPORT (OS_FATAL, "os_socketModuleInit", 0, "WSAStartup failed, no compatible socket implementation available");
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
        return;
    }
 
    /* Confirm that the WinSock DLL supports 2.0.	 */
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

    if (getsockopt(s, level, optname, optval, (int *)optlen) == -1) {
        result = os_resultFail;
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
    INTERFACE_INFO *allInterfacesBuf;
    INTERFACE_INFO *intf;
    unsigned long returnedBytes;
    unsigned int listIndex;
    os_socket ifcs;
    int retVal;

    
    ifcs = os_sockNew (AF_INET, SOCK_DGRAM);
    if (ifcs >= -1) {
        allInterfacesBuf = os_malloc(MAX_INTERFACES * sizeof(INTERFACE_INFO));
        memset(allInterfacesBuf, 0, MAX_INTERFACES * sizeof(INTERFACE_INFO));
        retVal = WSAIoctl(ifcs, SIO_GET_INTERFACE_LIST, NULL, 0, allInterfacesBuf,
                     MAX_INTERFACES * sizeof(INTERFACE_INFO), &returnedBytes, 0, 0);
        *validElements = returnedBytes/sizeof(INTERFACE_INFO);
        listIndex = 0;
        while ((listIndex < listSize) && (listIndex < *validElements)) {
            intf = &allInterfacesBuf[listIndex];
            _snprintf(ifList[listIndex].name,
                      OS_IFNAMESIZE, "0x%x",
                      ntohl(intf->iiAddress.AddressIn.sin_addr.S_un.S_addr));
            ifList[listIndex].flags = intf->iiFlags;
            ifList[listIndex].address = intf->iiAddress.Address;

            ifList[listIndex].broadcast_address = intf->iiBroadcastAddress.Address;
            ((struct sockaddr_in *)(&(ifList[listIndex].broadcast_address)))->sin_addr.S_un.S_addr =
                ((struct sockaddr_in *)(&(ifList[listIndex].address)))->sin_addr.S_un.S_addr |
                    ~(intf->iiNetmask.AddressIn.sin_addr.S_un.S_addr);

            ifList[listIndex].network_mask = intf->iiNetmask.Address;
            listIndex++;
        }
        os_sockFree (ifcs);
    }
    return result;
}

#undef MAX_INTERFACES
