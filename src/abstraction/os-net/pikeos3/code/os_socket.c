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
#include "os_socket.h"
#include "os_stdlib.h"
#include "os_abstract.h"
#include <lwip/netif.h>

char * inet_ntop(int af, const void *vsrc, char *dst, socklen_t size)
{
    static const char fmt[] = "%u.%u.%u.%u";
    char tmp[sizeof "255.255.255.255"];
    int l;
    const unsigned char *src = (const unsigned char *)vsrc;

    if (af != AF_INET)
    {
        return NULL;
    }
    l = sprintf(tmp, fmt, src[0], src[1], src[2], src[3]);
    if (l <= 0 || (socklen_t) l >= size)
    {
       return (NULL);
    }
    os_strncpy(dst, tmp, size);
    return (dst);
}

os_socket
os_sockNew(
    int domain,
    int type)
{
    return socket (domain, type, 0);
}

os_result
os_sockBind(
    os_socket s,
    const struct sockaddr *name,
    os_uint32 namelen)
{
    os_result result = os_resultSuccess;

    if (bind(s, (struct sockaddr *)name, (os_uint)namelen) == -1) {
        result = os_resultFail;
    }
    return result;
}

os_result
os_sockGetsockname(
    os_socket s,
    const struct sockaddr *name,
    os_uint32 namelen)
{
    os_result result = os_resultSuccess;
    socklen_t len = namelen;

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
/* sendto on pikeos takes non-const pointers */
    ssize_t res = sendto(s, (void *)msg, len, 0, (struct sockaddr *)to, tolen);
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
   ssize_t res;
   socklen_t fl = *fromlen;
   res = recvfrom(s, buf, len, 0, from, &fl);
   if (res < 0)
   {
      *bytesRead = 0;
      return os_resultFail;
   }
   else
   {
      *fromlen=fl;
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
    os_uint32 *optlen)
{
    int res;
    socklen_t ol = *optlen;

    if (optname == SO_SNDBUF || optname == SO_RCVBUF)
    {
        /* We know this won't work */
        return os_resultSuccess;
    }
    res = getsockopt(s, level, optname, optval, &ol);
    *optlen = ol;
    return ( res == -1 ? os_resultFail : os_resultSuccess );
}

os_result
os_sockSetsockopt(
    os_socket s,
    os_int32 level,
    os_int32 optname,
    const void *optval,
    os_uint32 optlen)
{
    os_result result = os_resultSuccess;

    if (optname == SO_SNDBUF || optname == SO_RCVBUF || optname == SO_DONTROUTE || optname == SO_REUSEADDR)
    {
        /* We know this won't work */
        return os_resultSuccess;
    }

    if (setsockopt(s, level, optname, optval, optlen) == -1) {
        result = os_resultFail;
    }
    return result;
}

os_result
os_sockFree(
    os_socket s)
{
    os_result result = os_resultSuccess;

    if (close(s) == -1) {
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

static void os_pikeos_copy_ethaddress(
        os_sockaddr_storage *dest,
        u32_t src)
{
/* ipv4 only */
  os_sockaddr_in *dest4 = (os_sockaddr_in *)dest;
  dest4->sin_len = sizeof (os_sockaddr_in);
  dest4->sin_family = AF_INET;
  dest4->sin_port = 0;
  dest4->sin_addr.s_addr = src;
}

static os_result
os_queryInterfaceAttributes(
        const struct netif *iface,
        os_ifAttributes *ifElement)
{
  strncpy (ifElement->name, iface->name, OS_IFNAMESIZE);
  ifElement->interfaceIndexNo = iface->num;

  os_pikeos_copy_ethaddress (&ifElement->address, iface->ip_addr.addr);
  os_pikeos_copy_ethaddress (&ifElement->network_mask, iface->netmask.addr);
  os_pikeos_copy_ethaddress (&ifElement->broadcast_address, iface->ip_addr.addr | ~iface->netmask.addr);

  ifElement->flags = 0;
  if (iface->flags & NETIF_FLAG_UP)
  {
    ifElement->flags |= IFF_UP;
  }
  if (iface->flags & NETIF_FLAG_BROADCAST)
  {
    ifElement->flags |= IFF_BROADCAST;
  }
  if (iface->flags & NETIF_FLAG_IGMP)
  {
    ifElement->flags |= IFF_MULTICAST;
  }
  if (iface->ip_addr.addr == INADDR_LOOPBACK)
  {
    ifElement->flags |= IFF_LOOPBACK;
  }
  return os_resultSuccess;
}

os_result
os_sockQueryInterfaces(
    os_ifAttributes *ifList,
    os_uint32 listSize,
    os_uint32 *validElements)
{
  os_result result = os_resultSuccess;
  os_uint32 listIndex;
  struct netif *source;

  source = netif_list;
  listIndex = 0;

  while (listIndex < listSize && source)
  {
    result = os_queryInterfaceAttributes (source, &ifList[listIndex]);
    source = source->next;
    if (result == os_resultSuccess)
    {
      listIndex++;
    }
  }

  *validElements = listIndex;

  return os_resultSuccess;
}

os_result
os_sockQueryIPv6Interfaces(
    os_ifAttributes *ifList,
    os_uint32 listSize,
    os_uint32 *validElements)
{
    return os_resultFail;
}

os_result
os_sockSetNonBlocking(
    os_socket s,
    os_boolean nonblock)
{
    os_result r;

    assert(nonblock == OS_FALSE || nonblock == OS_TRUE);

    if (ioctl (s, FIONBIO, (char *)&nonblock) >= 0) {
        r = os_resultSuccess;
    } else {
        r = os_resultFail;
    }
    return r;
}

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
    OS_UNUSES_ARG(timeout);
    *status = OS_FALSE;

    return os_resultFail;
}

os_result
os_sockQueryInterfaceStatusSignal(void *handle)
{
    OS_UNUSED_ARG(handle);
    return os_resultFail;
}
