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

/****************************************************************
 * Implementation for socket services conforming to             *
 * OpenSplice requirements                                      *
 ****************************************************************/

/** \file os/code/os_socket.c
 *  \brief socket management
 */

#include "os_socket.h"
#include "os_stdlib.h"
#include "os_report.h"
#include "os_heap.h"
#ifdef DO_HOST_BY_NAME
#include <netdb.h>
#endif

#ifndef _VXWORKS
const os_in6_addr os_in6addr_any = IN6ADDR_ANY_INIT;
#else
const os_in6_addr os_in6addr_any = { { 0 } };
#endif

/**
* Compare two socket IP host addresses for equality - does not consider the port number.
* This is a 'straight' equal i.e. family must match and address bytes
* must correspond. So it will not consider the possibility of IPv6 mapped
* IPv4 addresses or anything arcane like that.
* @param thisSock First address
* @param thatSock Second address.
* @return OS_TRUE if equal, OS_FALSE otherwise.
*/
os_boolean
os_sockaddrIPAddressEqual(const os_sockaddr* thisSock,
                           const os_sockaddr* thatSock)
{
    os_sockaddr_in6 * thisV6, * thatV6;
    os_boolean result = OS_FALSE;

    if (thisSock->sa_family == thatSock->sa_family)
    {
        if (thisSock->sa_family == AF_INET)
        {
            /* IPv4 */
            result = (((os_sockaddr_in*)thisSock)->sin_addr.s_addr ==
                     ((os_sockaddr_in*)thatSock)->sin_addr.s_addr ?
                     OS_TRUE: OS_FALSE);
        }
        else
        {
            /* IPv6 */
            thisV6 = (os_sockaddr_in6*) thisSock;
            thatV6 = (os_sockaddr_in6*) thatSock;
            result = (memcmp(&thisV6->sin6_addr.s6_addr, &thatV6->sin6_addr.s6_addr, sizeof(unsigned char) * 16) ?
                     OS_FALSE : OS_TRUE);
        }
    }
    return result;
}

/**
* Convert the provided addressString into a os_sockaddr.
* @return OS_TRUE on successful conversion. OS_FALSE otherwise
* @param addressString The string representation of a network address.
* @param addressOut A pointer to an os_sockaddr. Must be big enough for
* the address type specified by the string. This implies it should
* generally be the address of an os_sockaddr_storage for safety's sake.
* @param isIPv4 Iff the addressString is a hostname specifies whether
* and IPv4 address should be returned. If false an Ipv6 address will be
* requested. If the address is in either valid decimal presentation format
* param will be ignored.
*/
os_boolean
os_sockaddrStringToAddress(const char *addressString,
                           os_sockaddr* addressOut,
                           os_boolean isIPv4)
{
    os_uint32 ipv4IntAddress;
    os_boolean result = OS_FALSE;

    assert(addressOut);
    ipv4IntAddress = inet_addr(addressString);

    if (ipv4IntAddress == htonl(INADDR_NONE)
#ifdef WIN32
        || ipv4IntAddress == htonl(INADDR_ANY) /* Older Windows return this for empty string */
#endif
        )
    {
        /* Try and parse as an IPv6 address */
#ifdef WIN32
        int sslen = sizeof(os_sockaddr_in6);
        if (WSAStringToAddress((LPTSTR) addressString, AF_INET6, NULL, (os_sockaddr*)addressOut, &sslen) == 0)
        {
            result = OS_TRUE;
        }
#else
        if (inet_pton(AF_INET6, addressString, &(((os_sockaddr_in6*)addressOut)->sin6_addr)))
        {
            ((os_sockaddr_in6*)addressOut)->sin6_family = AF_INET6;
            result = OS_TRUE;
        }
#endif /* WIN32 */
    }
    else
    {
        ((os_sockaddr_in*)addressOut)->sin_family = AF_INET;
        ((os_sockaddr_in*)addressOut)->sin_addr.s_addr = ipv4IntAddress;
        result = OS_TRUE;
    }

    if (!result)
    {
#ifdef DO_HOST_BY_NAME
        struct addrinfo template;
        struct addrinfo *resultList;
        int retCode;

        memset (&template, 0, sizeof(template));
        template.ai_family = (isIPv4 ? AF_INET : AF_INET6);
        template.ai_socktype = SOCK_DGRAM;

        retCode = getaddrinfo(addressString, NULL, &template, &resultList);
        if (retCode != 0)
        {
            OS_REPORT_2(OS_WARNING,"os_sockaddrStringToAddress", 0,
                "error calling getaddrinfo(\"%s\"): %s",
                addressString, gai_strerror(retCode));
        }
        else
        {
            if (resultList)
            {
                memcpy(addressOut, resultList->ai_addr, resultList->ai_addrlen);
                /* Ignore other entries, just take first */
                freeaddrinfo(resultList);
            }
            else
            {
                OS_REPORT_1(OS_WARNING,"os_sockaddrStringToAddress", 0,
                      "could not lookup host \"%s\"",
                      addressString);
            }
        }
#else
        isIPv4 = OS_FALSE; /* Assign just to avoid unused arg warning */
#endif
    }

    if (!result)
    {
        OS_REPORT_1(OS_WARNING,"os_sockaddrStringToAddress", 0,
            "ignoring invalid networking address %s",
            addressString);
    }

    return result;
}

/**
* Check this address to see if it represents loopback.
* @return OS_TRUE if it does. OS_FALSE otherwise, or if unknown address type.
* @param thisSock A pointer to an os_sockaddr to be checked.
*/
os_boolean
os_sockaddrIsLoopback(const os_sockaddr* thisSock)
{
    os_boolean result = OS_FALSE;

    if (thisSock->sa_family == AF_INET6)
    {
        result = IN6_IS_ADDR_LOOPBACK(&((os_sockaddr_in6*)thisSock)->sin6_addr) ? OS_TRUE : OS_FALSE;
    }
    else if (thisSock->sa_family == AF_INET)
    {
        result = (INADDR_LOOPBACK == ntohl(((os_sockaddr_in*)thisSock)->sin_addr.s_addr)) ? OS_TRUE : OS_FALSE;
    }

    return result;
}

/**
* Convert the specified socket error to a string.
* @return Heap allocated string representation. Caller owns and must os_free
* @see os_sockError
* @param errNo The error number
*/
char*
os_sockErrnoToString(os_sockErrno errNo)
{
    char* result;
#if defined (WIN32) || defined (WINCE)
    LPVOID lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_IGNORE_INSERTS |
                    FORMAT_MESSAGE_MAX_WIDTH_MASK,
                    NULL,
                    errNo,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR) &lpMsgBuf,
                    0, NULL);
    result = os_strdup(lpMsgBuf);
    LocalFree(lpMsgBuf);
#else
    /* @todo This isn't threadsafe. Should replace w/ strerror_r */
    result = os_strdup(strerror(errNo));
#endif
    return result;
}

char*
os_sockaddrAddressToString(const struct sockaddr* sa,
                            char* buffer, size_t buflen)
{
#if defined (WIN32) || (WINCE)
    socklen_t structLength;
    int errorCode;

    os_strncpy(buffer, "Unknown address family", buflen);

    structLength = (sa->sa_family == AF_INET6 ? sizeof (os_sockaddr_in6) : sizeof (os_sockaddr_in));
    if (errorCode = getnameinfo(sa,
                   structLength,
                   buffer,
                   buflen,
                   NULL,
                   0,
                   NI_NUMERICHOST))
    {
        char* errorString;
        errorString = os_sockErrnoToString(WSAGetLastError());
        OS_REPORT_2(OS_ERROR,"os_sockaddrAddressToString", 0,
                "error calling getnameinfo to stringify network address. Error: %d (%s)",
                WSAGetLastError(), errorString);
        os_free(errorString);
    }
#else
    switch(sa->sa_family) {
        case AF_INET:
            inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr),
                    buffer, buflen);
            break;

        case AF_INET6:
            inet_ntop(AF_INET6, &(((os_sockaddr_in6 *)sa)->sin6_addr),
                    buffer, buflen);
            break;

        default:
            os_strncpy(buffer, "Unknown address family", buflen);
    }
#endif
    return buffer;
}

/* include OS specific socket management implementation		*/
#include "code/os_socket.c"
