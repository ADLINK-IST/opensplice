/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
/** \file
 *  \brief WIN32 socket management
 *
 * Implements socket management for WIN32
 */
#include <stdio.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#if defined(_WIN32_WINNT) && _WIN32_WINNT < 0x0502
/* Minimum Windows Server 2003 SP1, Windows XP SP2 == _WIN32_WINNT_WS03 (0x0502) */
#error _WIN32_WINNT should be at least _WIN32_WINNT_WS03 (0x0502)
#endif

#include <winsock2.h>
#include <Windows.h>
#include <iphlpapi.h>

#include "code/os__socket.h"

#include "os_report.h"
#include "os_heap.h"
#include "os_stdlib.h"

#define WORKING_BUFFER_SIZE 15000
#define MAX_TRIES 3

static FARPROC qwaveQOSCreateHandleFunc = NULL;
static FARPROC qwaveQOSCloseHandleFunc = NULL;
static FARPROC qwaveQOSAddSocketToFlowFunc = NULL;
static FARPROC qwaveQOSSetFlowFunc = NULL;
static HANDLE qwaveDLLModuleHandle = NULL;
static HANDLE qwaveDLLModuleLock = NULL;

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

    qwaveDLLModuleLock = CreateMutex(NULL, FALSE, NULL);
    if (qwaveDLLModuleLock == NULL) {
        OS_REPORT (OS_ERROR, "os_socketModuleInit", 0, "Failed to create mutex");
    }
}

void
os_socketModuleExit(void)
{
    if (qwaveDLLModuleHandle) {
        FreeLibrary(qwaveDLLModuleHandle);
    }

    if (qwaveDLLModuleLock) {
        CloseHandle(qwaveDLLModuleLock);
    }

    WSACleanup();
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

    if (bind(s, (struct sockaddr *)name, namelen) == SOCKET_ERROR) {
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

    if (getsockname(s, (struct sockaddr *)name, &len) == SOCKET_ERROR) {
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
       if (getsockopt(s, level, optname, (char *)&dwoptval, &dwoptlen) == SOCKET_ERROR)
       {
          result = os_resultFail;
       }

       assert( dwoptlen == sizeof( DWORD ) );
       *((os_uchar *)optval) = (os_uchar)dwoptval;
       *optlen = sizeof( os_uchar );
    }
    else
    {
       if (getsockopt(s, level, optname, optval, (int *)optlen) == SOCKET_ERROR)
       {
          result = os_resultFail;
       }
    }

    return result;
}


static os_result
os_sockSetDscpValueWithTos(
    os_socket sock,
    DWORD value)
{
    os_result result = os_resultSuccess;

    if (setsockopt(sock, IPPROTO_IP, IP_TOS, (char *)&value, (int)sizeof(value)) == SOCKET_ERROR) {
        int errNo = os_getErrno();
        char *errorMessage = os_strError(errNo);
        OS_REPORT(OS_WARNING, "os_sockSetDscpValue", 0,
                   "Failed to set diffserv value to %ld: %d %s", value, errNo, errorMessage);
        result = os_resultFail;
    }

    return result;
}


static os_result
os_sockLoadQwaveLibrary(void)
{
    if (qwaveDLLModuleLock == NULL) {
        OS_REPORT(OS_WARNING, "os_sockLoadQwaveLibrary", 0,
                "Failed to load QWAVE.DLL for using diffserv on outgoing IP packets");
        goto err_lock;
    }

    WaitForSingleObject(qwaveDLLModuleLock, INFINITE);
    if (qwaveDLLModuleHandle == NULL) {
        if ((qwaveDLLModuleHandle = LoadLibrary("QWAVE.DLL")) == NULL) {
            OS_REPORT(OS_WARNING, "os_sockLoadQwaveLibrary", 0,
                    "Failed to load QWAVE.DLL for using diffserv on outgoing IP packets");
            goto err_load_lib;
        }

        qwaveQOSCreateHandleFunc = GetProcAddress(qwaveDLLModuleHandle, "QOSCreateHandle");
        qwaveQOSCloseHandleFunc = GetProcAddress(qwaveDLLModuleHandle, "QOSCloseHandle");
        qwaveQOSAddSocketToFlowFunc = GetProcAddress(qwaveDLLModuleHandle, "QOSAddSocketToFlow");
        qwaveQOSSetFlowFunc = GetProcAddress(qwaveDLLModuleHandle, "QOSSetFlow");

        if ((qwaveQOSCreateHandleFunc == NULL) || (qwaveQOSCloseHandleFunc == NULL) ||
                (qwaveQOSAddSocketToFlowFunc == NULL) || (qwaveQOSSetFlowFunc == NULL)) {
            OS_REPORT(OS_WARNING, "os_sockLoadQwaveLibrary", 0,
                    "Failed to resolve entry points for using diffserv on outgoing IP packets");
            goto err_find_func;
        }
    }
    ReleaseMutex(qwaveDLLModuleLock);

    return os_resultSuccess;

err_find_func:
err_load_lib:
    ReleaseMutex(qwaveDLLModuleLock);
err_lock:

    return os_resultFail;
}

struct qos_version {
    USHORT MajorVersion;
    USHORT MinorVersion;
};

/* To set the DSCP value on Windows 7 or higher the following functions are used.
 *
 * - BOOL QOSCreateHandle(PQOS_VERSION Version, PHANDLE QOSHandle)
 * - BOOL QOSCloseHandle(HANDLE QOSHandle)
 * - BOOL WINAPI QOSAddSocketToFlow(HANDLE QOSHandle, SOCKET Socket,
 *                                  PSOCKADDR DestAddr, QOS_TRAFFIC_TYPE TrafficType,
 *                                  DWORD Flags, PQOS_FLOWID FlowId)
 * - BOOL WINAPI QOSSetFlow(HANDLE QOSHandle, QOS_FLOWID FlowId,
 *                          QOS_SET_FLOW Operation, ULONG Size, PVOID Buffer,
 *                          DWORD Flags, LPOVERLAPPED Overlapped)
 */


/* QOS_TRAFFIC_TYPE
 *  - QOSTrafficTypeBestEffort      = 0
 *  - QOSTrafficTypeBackground      = 1
 *  - QOSTrafficTypeExcellentEffort = 2
 *  - QOSTrafficTypeAudioVideo      = 3
 *  - QOSTrafficTypeVoice           = 4
 *  - QOSTrafficTypeControl         = 5
 */
#define OS_SOCK_QOS_TRAFFIC_TYPE_BEST_EFFORT      0
#define OS_SOCK_QOS_TRAFFIC_TYPE_BACKGROUND       1
#define OS_SOCK_QOS_TRAFFIC_TYPE_EXCELLENT_EFFORT 2
#define OS_SOCK_QOS_TRAFFIC_TYPE_AUDIO_VIDEO      3
#define OS_SOCK_QOS_TRAFFIC_TYPE_VOICE            4
#define OS_SOCK_QOS_TRAFFIC_TYPE_CONTROL          5

/* Default DSCP values set by QOSAddSocketToFlow
 *  0      : QOSTrafficTypeBestEffort      dscp  0
 *  1 -  8 : QOSTrafficTypeBackground      dscp  8
 *  9 - 40 : QOSTrafficTypeExcellentEffort dscp 40
 * 41 - 55 : QOSTrafficTypeAudioVideo      dscp 40
 * 56      : QOSTrafficTypeVoice           dscp 56
 * 57 - 63 : QOSTrafficTypeControl         dscp 56
 */
#define OS_SOCK_BESTEFFORT_DSCP_VALUE         0
#define OS_SOCK_BACKGROUND_DSCP_VALUE         8
#define OS_SOCK_EXCELLENT_EFFORT_DSCP_VALUE  40
#define OS_SOCK_VOICE_DSCP_VALUE             56

/* QOS_NON_ADAPTIVE_FLOW */
#define OS_SOCK_QOS_NON_ADAPTIVE_FLOW 0x00000002
/* QOSSetOutgoingDSCPValue */
#define OS_SOCK_QOS_SET_OUTGOING_DSCP_VALUE 2

static void
os_sockGetTrafficType(
    DWORD value,
    PLONG trafficType,
    PLONG defaultValue)
{
    if (value == 0) {
        *trafficType = OS_SOCK_QOS_TRAFFIC_TYPE_BEST_EFFORT;
        *defaultValue = OS_SOCK_BESTEFFORT_DSCP_VALUE;
    } else if (value <= OS_SOCK_BACKGROUND_DSCP_VALUE) {
        *trafficType = OS_SOCK_QOS_TRAFFIC_TYPE_BACKGROUND;
        *defaultValue = OS_SOCK_BACKGROUND_DSCP_VALUE;
    } else if (value <= OS_SOCK_EXCELLENT_EFFORT_DSCP_VALUE) {
        *trafficType = OS_SOCK_QOS_TRAFFIC_TYPE_EXCELLENT_EFFORT;
        *defaultValue = OS_SOCK_EXCELLENT_EFFORT_DSCP_VALUE;
    } else if (value < OS_SOCK_VOICE_DSCP_VALUE) {
        *trafficType = OS_SOCK_QOS_TRAFFIC_TYPE_AUDIO_VIDEO;
        *defaultValue = OS_SOCK_EXCELLENT_EFFORT_DSCP_VALUE;
    } else if (value == OS_SOCK_VOICE_DSCP_VALUE) {
        *trafficType = OS_SOCK_QOS_TRAFFIC_TYPE_VOICE;
        *defaultValue = OS_SOCK_VOICE_DSCP_VALUE;
    } else {
        *trafficType = OS_SOCK_QOS_TRAFFIC_TYPE_CONTROL;
        *defaultValue = OS_SOCK_VOICE_DSCP_VALUE;
    }
}


static os_result
os_sockSetDscpValueWithQos(
    os_socket sock,
    DWORD value,
    BOOL setDscpSupported)
{
    os_result result = os_resultSuccess;
    struct qos_version version;
    HANDLE qosHandle = NULL;
    ULONG qosFlowId = 0; /* Flow Id must be 0. */
    BOOL qosResult;
    LONG trafficType;
    LONG defaultDscp;
    char* errorMessage;
    int errNo;
    SOCKADDR_IN sin;

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;

    /* version must be 1.0 */
    version.MajorVersion = 1;
    version.MinorVersion = 0;

    /* Get a handle to the QoS subsystem. */
    qosResult = (BOOL)qwaveQOSCreateHandleFunc(&version, &qosHandle);
    if (!qosResult) {
        errNo = os_getErrno();
        errorMessage = os_strError(errNo);
        OS_REPORT(OS_ERROR, "os_sockSetDscpValue", 0,
                "QOSCreateHandle failed: %d %s", errNo, errorMessage);
        goto err_create_handle;
    }

    os_sockGetTrafficType(value, &trafficType, &defaultDscp);

    /*  Add socket to flow. */
    qosResult = (BOOL)qwaveQOSAddSocketToFlowFunc(qosHandle, sock, (PSOCKADDR)&sin,
            trafficType, OS_SOCK_QOS_NON_ADAPTIVE_FLOW, &qosFlowId);
    if (!qosResult) {
        errNo = os_getErrno();
        errorMessage = os_strError(errNo);
        OS_REPORT(OS_ERROR, "os_sockSetDscpValue", 0,
                "QOSAddSocketToFlow failed: %d %s", errNo, errorMessage);
        qwaveQOSCloseHandleFunc(qosHandle);
        goto err_add_flow;
    }

    if (value != defaultDscp) {

        if (!setDscpSupported) {
            OS_REPORT(OS_WARNING, "os_sockSetDscpValue", 0,
                    "Failed to set diffserv value to %ld value used is %d, not supported on this platform",
                    value, defaultDscp);
            goto err_set_flow;
        }

        /* Try to set DSCP value. Requires administrative rights to succeed */
        qosResult = (BOOL)qwaveQOSSetFlowFunc(qosHandle, qosFlowId, OS_SOCK_QOS_SET_OUTGOING_DSCP_VALUE,
                sizeof(value), &value, 0, NULL);
        if (!qosResult) {
            errNo = os_getErrno();
            if ((errNo == ERROR_ACCESS_DENIED) || (errNo == ERROR_ACCESS_DISABLED_BY_POLICY)) {
                OS_REPORT(OS_WARNING, "os_sockSetDscpValue", 0,
                        "Failed to set diffserv value to %ld value used is %d, not enough privileges",
                        value, defaultDscp);
            } else {
                errorMessage = os_strError(errNo);
                OS_REPORT(OS_ERROR, "os_sockSetDscpValue", 0,
                        "QOSSetFlow failed: %d %s", errNo, errorMessage);
            }
            goto err_set_flow;
        }
    }

    return result;

err_set_flow:
err_add_flow:
err_create_handle:

    return result;
}


static os_result
os_sockSetDscpValue(
    os_socket sock,
    DWORD value)
{
    os_result result = os_resultFail;
    OSVERSIONINFO version;

    version.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if (GetVersionEx(&version)) {
        if (version.dwMajorVersion >= 6) {
            if (os_sockLoadQwaveLibrary() == os_resultSuccess) {
                result = os_sockSetDscpValueWithQos(sock, value, (version.dwMinorVersion != 0));
            }
        } else {
            result = os_sockSetDscpValueWithTos(sock, value);
        }
    } else {
        OS_REPORT(OS_WARNING, "os_sockSetDscpValue", 0,
                  "Failed to get version information");
        result = os_sockSetDscpValueWithTos(sock, value);
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

    if ((level == IPPROTO_IP) && (optname == IP_TOS)) {
        dwoptval = *((os_uchar *)optval);
        if (dwoptval != 0) {
            result = os_sockSetDscpValue(s, dwoptval);
        }
    } else if ((optname == IP_MULTICAST_TTL) || (optname == IP_MULTICAST_LOOP)) {
        /* On win32 IP_MULTICAST_TTL and IP_MULTICAST_LOOP take DWORD * param
           rather than char * */
        dwoptval = *((os_uchar *)optval);
        optval = &dwoptval;
        optlen = sizeof( DWORD );
        if (setsockopt(s, level, optname, optval, (int)optlen) == SOCKET_ERROR) {
            result = os_resultFail;
        }
    } else {
        if (setsockopt(s, level, optname, optval, (int)optlen) == SOCKET_ERROR) {
            result = os_resultFail;
        }
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
    int errNo;

    if (GetIpAddrTable(pIPAddrTable, &dwSize, 0) == ERROR_INSUFFICIENT_BUFFER) {
        pIPAddrTable = (MIB_IPADDRTABLE *) os_malloc(dwSize);
        if (pIPAddrTable != NULL) {
            if (GetIpAddrTable(pIPAddrTable, &dwSize, 0) != NO_ERROR) {
                errNo = os_getErrno();
                os_report(OS_ERROR, "addressToIndexAndMask", __FILE__, __LINE__, 0,
                      "GetIpAddrTable failed: %d %s", errNo, os_strError (errNo));
                result = os_resultFail;
            }
        } else {
            os_report(OS_ERROR, "addressToIndexAndMask", __FILE__, __LINE__, 0,
                "Failed to allocate %d bytes for IP address table", dwSize);
            result = os_resultFail;
        }
    } else {
        errNo = os_getErrno();
        os_report(OS_ERROR, "addressToIndexAndMask", __FILE__, __LINE__, 0,
                    "GetIpAddrTable failed: %d %s", errNo, os_strError (errNo));
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

            /* Get interface flags. */
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

            /* Get interface flags. */
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

            /* Get interface flags. */
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

              /* Get interface flags. */
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
    HANDLE hHandle;
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
        CloseHandle(info->hHandle);
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
        if (os_getErrno() != WSA_IO_PENDING) {
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
        if ((ret != NO_ERROR) && (os_getErrno() != WSA_IO_PENDING)) {
            os_free(info->ifName);
            os_free(info);
            info = NULL;
            os_report(OS_ERROR, "os_sockQueryInterfaceStatusInit", __FILE__, __LINE__, 0,
                      "Failed to administer for network interface address changes");
        } else {
            info->hHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
            if (info->hHandle == NULL) {
                int errNo = os_getErrno();
                char *errorMessage = os_strError(errNo);
                os_report(OS_ERROR, "os_sockQueryInterfaceStatusInit", __FILE__, __LINE__, 0,
                          "CreateEvent failed: %d %s", errNo, errorMessage);

                os_sockQueryInterfaceStatusDeinit(info);
                info = NULL;
            }
        }
    }

    return info;
}


os_result
os_sockQueryInterfaceStatus(
    void *handle,
    os_duration timeout,
    os_boolean *status)
{
    os_sockQueryInterfaceStatusInfo *info = (os_sockQueryInterfaceStatusInfo *) handle;
    os_result result = os_resultFail;
    DWORD t;
    DWORD r;
    HANDLE hHandles[2];

    *status = OS_FALSE;

    if (info) {
        assert(timeout >= 0);
        t = (DWORD)(timeout / OS_DURATION_MILLISECOND);

        hHandles[0] = info->overlap.hEvent;
        hHandles[1] = info->hHandle;
        r = WaitForMultipleObjects(2, hHandles, FALSE, t);
        if (r == WAIT_OBJECT_0) {
            result = os_sockGetInterfaceStatus(info->ifName, status);
            os_sockQueryInterfaceStatusReset(info);
        } else if ((r - WAIT_OBJECT_0) == 1) {
            /* (Mis)using os_resultTimeout to indicate that woken from
             * WaitForMultipleObjects and no status update is available */
            result = os_resultTimeout;
        } else if (r == WAIT_TIMEOUT) {
            result = os_resultTimeout;
        } else {
            result = os_resultFail;
        }
    }

    return result;
}

os_result
os_sockQueryInterfaceStatusSignal(
    void *handle)
{
    os_sockQueryInterfaceStatusInfo *info = (os_sockQueryInterfaceStatusInfo *) handle;
    os_result result = os_resultFail;

    if (info) {
        if (!SetEvent(info->hHandle)) {
            int errNo = os_getErrno();
            char *errorMessage = os_strError(errNo);
            os_report(OS_WARNING, "os_sockQueryInterfaceStatusSignal", __FILE__, __LINE__, 0,
                    "SetEvent failed: %d %s",
                    errNo, errorMessage);
        } else {
            result = os_resultSuccess;
        }
    }
    return result;
}


#undef MAX_INTERFACES
