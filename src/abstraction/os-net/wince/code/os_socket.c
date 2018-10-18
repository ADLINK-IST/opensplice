/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
/** \file
 *  \brief WIN32 socket management
 *
 * Implements socket management for WIN32
 */
//#include "stdafx.h"
#include <stdio.h>

#include "code/os__socket.h"
#include "../include/os_socket.h"

#include "os_report.h"
#include "os_heap.h"
#include "os_mutex.h"
#include "os_thread.h"
#include "os_stdlib.h"
#include "os_iterator.h"
#include "os_abstract.h"
#include "os_errno.h"

#include <Windows.h>
#include <iphlpapi.h>
#include <winsock2.h>
#include <Msgqueue.h>
#include <winioctl.h>
#include <ntddndis.h>
#include <nuiouser.h>
#pragma comment(lib, "IPHLPAPI.lib")

#define WORKING_BUFFER_SIZE 15000
#define MAX_TRIES 3

struct socketDetails {
    os_socket s;
    void * multi_IF;
    void * optval;
    int optlen;
}; // struct socketDetails - Used to store all the multicast sockets and related information.

static os_iter socketList = NULL; // socketList iterator
static os_mutex socketListMutex; // mutex used for socket list operations

HANDLE hNdis; // NDIS Handle
HANDLE hTerminationEvent; // Termination Event Handle
HANDLE hMsgQ; // MessageQueue Thread Handle
HANDLE hMulticastRejoinThread = NULL; // Thread that monitors the multicast groups
void * multi_IF = NULL; // multicast interface associated with the multicast socket operations.
BOOL connected = TRUE; // Connection state of the interface

static void NdisRequestNotificationsInit(HANDLE ndis, HANDLE queue, DWORD dwNotificationTypes )
{
    NDISUIO_REQUEST_NOTIFICATION ethernet_notifications = { 0 };
    ethernet_notifications.hMsgQueue = queue;
    ethernet_notifications.dwNotificationTypes = dwNotificationTypes;

    DeviceIoControl( ndis,
                     IOCTL_NDISUIO_REQUEST_NOTIFICATION,
                     &ethernet_notifications,
                     sizeof( NDISUIO_REQUEST_NOTIFICATION ),
                     NULL,
                     0,
                     NULL,
                     NULL );
}

static void NdisRequestNotificationsDeInit(HANDLE ndis)
{
   // stop receiving NDIS power notifications
    DeviceIoControl( ndis,
                     IOCTL_NDISUIO_CANCEL_NOTIFICATION,
                     NULL,
                     0,
                     NULL,
                     0,
                     NULL,
                     NULL );
}

/* freeListItem. precondition : list is locked */
static void freeListItem (struct socketDetails * item)
{
    os_free (item->optval);
    os_free (item->multi_IF);
    os_free(item);
}

/* addSocketToList. precondition : list is locked and the socket does not exist in the list */
static void addSocketToList (os_socket sock, const void * multiIf, const void  *optval, int optlen)
{
    struct socketDetails * sd = malloc (sizeof (struct socketDetails));
    struct ip_mreq * m_req = malloc(optlen);
    struct in_addr*  multicast_IF = malloc (sizeof(multiIf));

    memcpy(multicast_IF, (struct in_addr*)multiIf, sizeof(multiIf));
    memcpy(m_req, (struct ip_mreq*)optval, optlen);

    if (sd)
    {
        sd->s = sock;
        sd->multi_IF = (void *) multicast_IF;
        sd->optval = (void *) m_req;
        sd->optlen = optlen;

        socketList = os_iterAppend(socketList, sd); // Now append to the list.
    }
    else
    {
        OS_REPORT(OS_ERROR, "addSocketToList", 0, "unable to malloc!!!");
    }
}

/* removeSocketFromList. precondition : list is locked */
static void removeSocketFromList (os_socket socket)
{
    struct socketDetails * sd;
    os_int32 length = os_iterLength(socketList);

    os_int32 index = 0;

    while (index < length)
    {
        sd = (struct socketDetails *) os_iterObject(socketList, index);

        if (sd->s == socket)
        {
            os_iterTake(socketList, sd);
            freeListItem (sd);
            length = os_iterLength(socketList);
            // If found recalculate the length and do not increment the index
            // so that we do not skip an entry in the list!!!
        }
        else
        {
            index++; // Element not found, so increment the index and process next entry!!!
        }
    }
}

// This to be done on module exit and socketlist is locked.
static void emptySocketList()
{
    struct socketDetails * sd;

    while (os_iterLength(socketList) > 0)
    {
        sd = os_iterTakeFirst(socketList);
        freeListItem(sd);
    }
    os_iterFree (socketList);
}

static os_result checkInterfaceLiveliness (char* iface)
{
    DWORD winresult;
    ULONG flags, bufsize = 0, family;
    BOOL result;

    IP_ADAPTER_ADDRESSES *addresses, *currAddress;
    IP_ADAPTER_INFO *adaptersinfo, *currAdapter;

    // Set the flags to pass to GetAdaptersAddresses
    flags = GAA_FLAG_INCLUDE_PREFIX + GAA_FLAG_SKIP_DNS_SERVER +
        GAA_FLAG_SKIP_UNICAST + GAA_FLAG_SKIP_ANYCAST + GAA_FLAG_SKIP_MULTICAST;

    family = AF_INET;
    addresses = NULL;
    currAddress = NULL;


    bufsize = sizeof (IP_ADAPTER_ADDRESSES) * 2;
    addresses = (IP_ADAPTER_ADDRESSES *) os_malloc(bufsize);
    result = FALSE;

    winresult = GetAdaptersAddresses(family, flags, NULL, addresses, &bufsize);

    if (winresult == ERROR_BUFFER_OVERFLOW)
    {
        os_free (addresses);
        addresses = (IP_ADAPTER_ADDRESSES *) os_malloc (bufsize);
        winresult = GetAdaptersAddresses (family, flags, NULL, addresses, &bufsize);
    }

    if (winresult == NO_ERROR)
    {
        /* Now get the Adapters structure */

        adaptersinfo = NULL;
        bufsize = 0;
        GetAdaptersInfo (adaptersinfo, &bufsize);
        adaptersinfo = os_malloc (bufsize);

        winresult = GetAdaptersInfo (adaptersinfo, &bufsize);

        if (winresult == NO_ERROR)
        {
            currAdapter = adaptersinfo;

            /* For each Adapter */

            while (currAdapter)
            {
                /* Find the corresponding Adresses */
                currAddress = addresses;
                while (currAddress && strcmp (currAddress->AdapterName, currAdapter->AdapterName))
                {
                    currAddress = currAddress->Next;
                }
                if (currAddress && currAdapter->Type == MIB_IF_TYPE_ETHERNET)
                {
                    if (strcmp (iface, currAdapter->IpAddressList.IpAddress.String) == 0) // Found a match for the interface I am checking the liveliness for.
                    {
                        if (currAddress->OperStatus == IfOperStatusUp)
                        {
                            os_free (adaptersinfo);
                            os_free (addresses);
                            return TRUE;
                        }
                        else
                        {
                            os_free (adaptersinfo);
                            os_free (addresses);
                            return FALSE;
                        }
                    }
                }
                currAdapter = currAdapter->Next;
            }
            os_free (adaptersinfo);
        }
        os_free (addresses);
    }
    return result;
}

static os_int32 rejoinSocketToMulticast (void *obj, void *arg)
{
    int res;
    os_uint sockErr;
    char * iface;
    char* multi_addr;
    int count = 0;

    struct socketDetails * sd = (struct socketDetails*)obj;

    iface = os_strdup(inet_ntoa(((struct ip_mreq*)sd->optval)->imr_interface));
    multi_addr = os_strdup(inet_ntoa(((struct ip_mreq*)sd->optval)->imr_multiaddr));

    while( (!checkInterfaceLiveliness (iface)) && (count < 7) )
    {
        // Keep looping until the interface is awake
        // We do this because on WinCE there is a delay from the link being up and the interface being present of about 15secs
        // The more elegant solution of NotifyAddrChange is a bit problematic as there are 2 interfaces the localhost and the multicast
        // and will only trigger once thus leaving us with the possibility of one of the other interfaces not being up and the sockets not
        // being connected to the group properly leading to communication issues.
        count++;
        Sleep(2000);
    }

    res = setsockopt(sd->s, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*) sd->optval, sd->optlen);
    sockErr = os_getErrno();
    if ( (res == SOCKET_ERROR) && (sockErr > 0) )
    {
        OS_REPORT(OS_ERROR, "rejoinSocketToMulticast-IP_DROP_MEMBERSHIP", 1,
                    "Socket info: %d with multicast group %s and ipv4 interface %s gave error %u",
                    sd->s,
                    multi_addr,
                    iface,
                    sockErr);
        os_setErrno (0);
    }

    res = setsockopt(sd->s, IPPROTO_IP, IP_MULTICAST_IF, (char*) sd->multi_IF, sizeof (sd->multi_IF));
    sockErr = os_getErrno();
    if ( (res == SOCKET_ERROR) && (sockErr > 0) )
    {
        OS_REPORT(OS_ERROR, "rejoinSocketToMulticast-IP_MULTICAST_IF", 1,
                    "Socket info: %d with multicast group %s and ipv4 interface %s gave error %u",
                    sd->s,
                    multi_addr,
                    iface,
                    sockErr);
        os_setErrno (0);
    }

    res = setsockopt(sd->s, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*) sd->optval, sd->optlen);
    sockErr = os_getErrno();
    if ( (res == SOCKET_ERROR) && (sockErr > 0) )
    {
        OS_REPORT(OS_ERROR, "rejoinSocketToMulticast-IP_ADD_MEMBERSHIP", 1,
                    "Socket info: %d with multicast group %s and ipv4 interface %s gave error %u",
                    sd->s,
                    multi_addr,
                    iface,
                    sockErr);
        os_setErrno (0);
    }

    os_free (iface);
    os_free (multi_addr);

    return res;
}

static void osplMulticastGroupRejoin (void *lpParam)
{
    DWORD dwWaitResult;
    NDISUIO_DEVICE_NOTIFICATION notification = { 0 };
    DWORD bytes_read = 0;
    DWORD notification_flags = 0;

    HANDLE handles[2]; // Array to hold termination Event and MessageQueue HANDLES

    handles[0] = hTerminationEvent;
    handles[1] = hMsgQ;

    while (1)
    {
        dwWaitResult = WaitForMultipleObjects (2, handles, FALSE, INFINITE); // Wait for a handle to be triggered
        if (dwWaitResult == WAIT_OBJECT_0 + 0) // Termination Event has been triggered. Time to exit thread!
        {
            return;
        }
        if(dwWaitResult == WAIT_OBJECT_0 + 1) // We have data on the message queue
        {
            while (ReadMsgQueue(handles[1],
                                &notification,
                                sizeof (NDISUIO_DEVICE_NOTIFICATION),
                                &bytes_read,
                                0,
                                &notification_flags) && bytes_read >0 )
            {
                switch (notification.dwNotificationType)
                {
                    /* When I detect those 2 events, it means that there has been a disconnect event
                       and has been followed by a connect event, or the node has woken up from a
                       hibernation event. In both cases we need to perform the same actions:
                       DROP MEMBERSHIP, and ADD MEMBERSHIP to the multicast sockets */
                    case NDISUIO_NOTIFICATION_MEDIA_CONNECT:
                    case NDISUIO_NOTIFICATION_RESET_START:
                        if (!connected)
                        {
                            os_mutexLock(&socketListMutex);
                            os_iterWalk (socketList, rejoinSocketToMulticast, NULL);
                            os_mutexUnlock(&socketListMutex);
                            connected = TRUE;
                        }
                        break;
                    case NDISUIO_NOTIFICATION_MEDIA_DISCONNECT:
                    case NDISUIO_NOTIFICATION_DEVICE_POWER_DOWN:
                        connected = FALSE;
                        break;
                } // switch
            } //  while ReadMsgQueue
        } // if for MessageQueue
    } // while(1)
}

// This function creates all the necessary Windows handles for our listening thread!
static void createSocketManagementThreadHandles()
{
    DWORD dwNotificationTypes;
    MSGQUEUEOPTIONS options = { 0 };

    // Create Termination Event
    hTerminationEvent = CreateEvent (NULL,
                                     FALSE,
                                     FALSE,
                                     TEXT("TerminationEvent"));

    // Create NDIS Handle
    hNdis = CreateFile (NDISUIO_DEVICE_NAME,
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        INVALID_HANDLE_VALUE);

    dwNotificationTypes = NDISUIO_NOTIFICATION_DEVICE_POWER_DOWN |
                          NDISUIO_NOTIFICATION_DEVICE_POWER_UP |
                          NDISUIO_NOTIFICATION_MEDIA_CONNECT |
                          NDISUIO_NOTIFICATION_MEDIA_DISCONNECT |
                          NDISUIO_NOTIFICATION_RESET_START ;

    if ( hNdis != INVALID_HANDLE_VALUE )
    {
        options.dwSize          = sizeof( MSGQUEUEOPTIONS );
        options.cbMaxMessage    = sizeof( NDISUIO_DEVICE_NOTIFICATION );
        options.bReadAccess     = TRUE;
        options.dwFlags         = MSGQUEUE_NOPRECOMMIT;

        hMsgQ = CreateMsgQueue (NULL, &options);

        if ( hMsgQ != NULL )
        {
            NdisRequestNotificationsInit(hNdis, hMsgQ, dwNotificationTypes);
        }
    }
}

void
os_socketModuleInit()
{
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    // Setup the mutex attributes for the locking of the socketList table
    os_mutexInit(&socketListMutex, NULL);

     // Create the socketList table
    socketList = os_iterNew(NULL);

    // Create multicast socket list management thread handles.
    createSocketManagementThreadHandles();

    if (hMulticastRejoinThread == NULL)
    {
        DWORD threadId = 0;
        hMulticastRejoinThread = CreateThread(NULL,
                                        (SIZE_T)128*1024,
                                        (LPTHREAD_START_ROUTINE)osplMulticastGroupRejoin,
                                        NULL,
                                        0,
                                        &threadId);
        if (hMulticastRejoinThread == NULL)
        {
            OS_REPORT(OS_ERROR, "os_socketModuleInit", 0, "osplMulticastGroupRejoin thread failed to be created at startup!!!");
        }
        else
        {
           OS_REPORT (OS_INFO, "os_socketModuleInit", 0, "started osplMulticastGroupRejoin thread 0x"PA_ADDRFMT"", threadId);
        }
    }

    wVersionRequested = MAKEWORD (OS_SOCK_VERSION, OS_SOCK_REVISION);

    err = WSAStartup (wVersionRequested, &wsaData);
    if (err != 0) {
        // OS_REPORT (OS_FATAL, "os_socketModuleInit", 0, "WSAStartup failed, no compatible socket implementation available");
        // mha - temp commented out
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
        NdisRequestNotificationsDeInit(hNdis);
        CloseHandle(hNdis);
        CloseHandle(hTerminationEvent);
        CloseHandle(hMsgQ);
        CloseHandle(hMulticastRejoinThread);
        os_mutexDestroy(&socketListMutex);

        return;
    }

    /* Confirm that the WinSock DLL supports 2.0.        */
    /* Note that if the DLL supports versions greater    */
    /* than 2.0 in addition to 2.0, it will still return */
    /* 2.0 in wVersion since that is the version we      */
    /* requested.                                        */

    if ((LOBYTE(wsaData.wVersion) != OS_SOCK_VERSION) ||
        (HIBYTE(wsaData.wVersion) != OS_SOCK_REVISION)) {
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
        // OS_REPORT (OS_FATAL, "os_socketModuleInit", 1, "WSAStartup failed, no compatible socket implementation available");
        // mha - temp commented out
        WSACleanup();

        NdisRequestNotificationsDeInit(hNdis);
        CloseHandle(hNdis);
        CloseHandle(hTerminationEvent);
        CloseHandle(hMsgQ);
        CloseHandle(hMulticastRejoinThread);
        os_mutexDestroy(&socketListMutex);

        return;
    }
}

void
os_socketModuleExit(void)
{
    SetEvent(hTerminationEvent);

    os_mutexLock(&socketListMutex);
    emptySocketList();
    os_mutexUnlock(&socketListMutex);

    os_mutexDestroy(&socketListMutex);
    os_free (multi_IF);

    NdisRequestNotificationsDeInit(hNdis);
    CloseHandle(hNdis);
    CloseHandle(hTerminationEvent);
    CloseHandle(hMsgQ);
    CloseHandle(hMulticastRejoinThread);

    WSACleanup(); // Shutdown Winsock

    return;
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
    int res = recvfrom(s, buf, len, 0, from, (int *)fromlen);
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
    DWORD dwoptval;
    int res;

    /* Certain options we use aren't supported in CE - fail silently. */
    if (level == SOL_SOCKET) {
        if (optname == SO_DONTROUTE || optname == SO_SNDBUF) {
            return os_resultSuccess;
        }
    }

    if (level == IPPROTO_IP) {
        /* On win32 IP_MULTICAST_TTL take DWORD * param rather than char * */
        if (optname == IP_MULTICAST_TTL) {
            dwoptval = *((os_uchar *)optval);
            optval = &dwoptval;
            optlen = sizeof( DWORD );
        } else if (optname == IP_TTL || optname == IP_MULTICAST_LOOP || optname == IP_TOS) {
            return os_resultSuccess;
        }
    }

    res = setsockopt(s, level, optname, optval, (int)optlen);

    if (res == -1) {
        return os_resultFail;
    }
    else
    {
        if (level == IPPROTO_IP)
        {
            if (optname == IP_MULTICAST_IF)
            {
                multi_IF = malloc (optlen);
                memcpy(multi_IF, (struct in_addr*)optval, optlen);
            }
            if (optname == IP_ADD_MEMBERSHIP)
            {
                os_mutexLock(&socketListMutex);
                addSocketToList(s, multi_IF, optval, (int)optlen); //Add socket to list
                os_mutexUnlock(&socketListMutex);
            }
        }
        return os_resultSuccess;
    }
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
        switch(os_getErrno()){
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

    os_mutexLock(&socketListMutex);
    removeSocketFromList(s);
    os_mutexUnlock(&socketListMutex);

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
    os_duration timeout)
{
    struct timeval t;
    int r;

    assert(OS_DURATION_ISPOSITIVE(timeout));

    t.tv_sec = (long)OS_DURATION_GET_SECONDS(timeout);
    t.tv_usec = (long)(OS_DURATION_GET_NANOSECONDS(timeout) / 1000);
    r = select(nfds, readfds, writefds, errorfds, &t);

    return r;
}


/* Fills in a sockaddr with an ipv4 address */

void fillSA (struct sockaddr *sa, unsigned long address)
{
    struct sockaddr_in *s_in = (struct sockaddr_in *)sa;
    s_in->sin_family = AF_INET;
    s_in->sin_port = 0;
    s_in->sin_addr.s_addr = address;
}

os_result
os_sockQueryInterfaces(
    os_ifAttributes *ifList,
    os_uint listSize,
    os_uint *validElements)
{
    os_result result;
    ULONG family = AF_INET;
    ULONG flags =
        GAA_FLAG_INCLUDE_PREFIX + GAA_FLAG_SKIP_DNS_SERVER +
        GAA_FLAG_SKIP_UNICAST + GAA_FLAG_SKIP_ANYCAST + GAA_FLAG_SKIP_MULTICAST;
    ULONG bufsize = WORKING_BUFFER_SIZE;
    os_uint iterations =0;
    DWORD winresult;
    IP_ADAPTER_ADDRESSES *addresses, *currAddress;
    IP_ADAPTER_INFO *adaptersinfo, *currAdapter;
    unsigned long addr, mask;
    os_uint i;

    *validElements = 0;
    result = os_resultFail;

    /* Get the Addresses structure */

    do {
        addresses = (IP_ADAPTER_ADDRESSES *) os_malloc(bufsize);
        winresult = GetAdaptersAddresses (family, flags, NULL, addresses, &bufsize);

        if (winresult == ERROR_BUFFER_OVERFLOW) {
            os_free(addresses);
            addresses = NULL;
            bufsize <<= 1; /* double the buffer just to be save.*/
        } else {
            break;
        }
        iterations++;

    } while ((winresult == ERROR_BUFFER_OVERFLOW) && (iterations < MAX_TRIES));

    if (winresult == NO_ERROR)
    {
        /* Now get the Adapters structure */

        adaptersinfo = NULL;
        bufsize = 0;
        GetAdaptersInfo (adaptersinfo, &bufsize);
        adaptersinfo = os_malloc (bufsize);

        winresult = GetAdaptersInfo (adaptersinfo, &bufsize);
        if (winresult == NO_ERROR)
        {
            i = 0;
            currAdapter = adaptersinfo;

            /* For each Adapter */

            while (currAdapter && i < listSize)
            {
                /* Find the corresponding Adresses */

                currAddress = addresses;
                while (currAddress && strcmp (currAddress->AdapterName, currAdapter->AdapterName))
                {
                    currAddress = currAddress->Next;
                }
                if (currAddress && currAddress->OperStatus == IfOperStatusUp)
                {
                    /* If we found it and it's up, populate our data */

                    addr = inet_addr (currAdapter->IpAddressList.IpAddress.String);
                    mask = inet_addr (currAdapter->IpAddressList.IpMask.String);
                    if (addr != INADDR_NONE && mask != INADDR_NONE)
                    {
                        fillSA (&ifList[i].address, addr);
                        fillSA (&ifList[i].network_mask, mask);
                        fillSA (&ifList[i].broadcast_address, addr | ~mask);
                        _snprintf (ifList[i].name, OS_IFNAMESIZE, "%s", currAddress->AdapterName);
                        ifList[i].flags = IFF_UP;
                        switch (currAdapter->Type)
                        {
                            case MIB_IF_TYPE_LOOPBACK:
                                ifList[i].flags |= IFF_LOOPBACK;
                                break;
                            case MIB_IF_TYPE_PPP:
                                ifList[i].flags |= IFF_POINTTOPOINT;
                                break;
                            default:
                                ifList[i].flags |= IFF_BROADCAST;
                                /* The functions above do not provide the information of
                                 * whether the interface supports multicast.  For now assume
                                 * that it does.  See scarab 2191
                                 */
                                ifList[i].flags |= IFF_MULTICAST;
                        }
                        i++;
                    }
                }
                currAdapter = currAdapter->Next;
            }
            *validElements = i;
            result = os_resultSuccess;
        }
        os_free (adaptersinfo);
    }
    os_free (addresses);
    return result;
}

os_result
os_sockQueryIPv6Interfaces(
    os_ifAttributes *ifList,
    os_uint32 listSize,
    os_uint32 *validElements)
{
    os_result result = os_resultFail;
    *validElements = 0;
    OS_REPORT(OS_ERROR, "os_sockQueryIPv6Interfaces", 0, "This platform does not support IPv6");
    return result;
}

#undef MAX_INTERFACES

void
os_sockQueryInterfaceStatusDeinit(
    void *handle)
{
    OS_UNUSED_ARG(handle);
}

void *
os_sockQueryInterfaceStatusInit(
    const char *ifName)
{
    OS_UNUSED_ARG(ifName);
    return NULL;
}

os_result
os_sockQueryInterfaceStatus(
    void *handle,
    os_duration timeout,
    os_boolean *status)
{
    OS_UNUSED_ARG(handle);
    OS_UNUSED_ARG(timeout);
    *status = OS_FALSE;

    return os_resultFail;
}

os_result
os_sockQueryInterfaceStatusSignal(void *handle)
{
    OS_UNUSED_ARG(handle);
    return os_resultFail;
}
