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
#include "os_socket.h"
#include "os_stdlib.h"

int
os_sockError(void)
{
    return errno;
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
    ssize_t res = sendto(s, msg, len, 0, to, tolen);
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
   socklen_t fl = *fromlen;;
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

    if (optname == SO_SNDBUF || optname == SO_RCVBUF)
    {
      if (optlen == 4 && *((unsigned *) optval) == 0)
      {
        /* We know this won't work */
        return os_resultSuccess;
      }
    }

    if (setsockopt(s, level, optname, optval, optlen) == -1) {
        result = os_resultFail;
    }

    if (result == os_resultSuccess && level == SOL_SOCKET && optname == SO_REUSEADDR)
    {
       if (setsockopt(s, level, SO_REUSEPORT, optval, optlen) == -1)
       {
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
    int oldflags;
    os_result r;

    assert(nonblock == OS_FALSE || nonblock == OS_TRUE);

    oldflags = fcntl(s, F_GETFL, 0);
    if(oldflags >= 0){
        if (nonblock == OS_TRUE){
            oldflags |= O_NONBLOCK;
        } else {
            oldflags &= ~O_NONBLOCK;
        }
        if(fcntl (s, F_SETFL, oldflags) == 0){
            r = os_resultSuccess;
        } else {
            r = os_resultFail;
        }
    } else {
        switch(errno){
            case EAGAIN:
                r = os_resultBusy;
                break;
            case EBADF:
                r = os_resultInvalid;
                break;
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
    os_time *timeout)
{
    struct timeval t;
    int r;

    t.tv_sec = (long)timeout->tv_sec;
    t.tv_usec = (long)(timeout->tv_nsec / 1000);
    r = select(nfds, readfds, writefds, errorfds, &t);

    return r;
}

static os_result os_queryInterfaceAttributesIPv4 (const struct ifaddrs *ifa, os_ifAttributes *ifElement)
{
  os_result result = os_resultSuccess;
  strncpy (ifElement->name, ifa->ifa_name, OS_IFNAMESIZE);
  memcpy (&ifElement->address, ifa->ifa_addr, sizeof (os_sockaddr_in));
  ifElement->flags = ifa->ifa_flags;
  if (ifElement->flags & IFF_BROADCAST)
    memcpy (&ifElement->broadcast_address, ifa->ifa_broadaddr, sizeof (os_sockaddr_in));
  else
    memset (&ifElement->broadcast_address, 0, sizeof (ifElement->broadcast_address));
  memcpy (&ifElement->network_mask, ifa->ifa_netmask, sizeof (os_sockaddr_in));
  ifElement->interfaceIndexNo = (os_uint) if_nametoindex (ifa->ifa_name);
  return result;
}

static os_result
os_queryInterfaceAttributesIPv6 (const struct ifaddrs *ifa, os_ifAttributes *ifElement)
{
  os_result result = os_resultSuccess;
  strncpy (ifElement->name, ifa->ifa_name, OS_IFNAMESIZE);
  memcpy (&ifElement->address, ifa->ifa_addr, sizeof (os_sockaddr_in6));
  ifElement->flags = ifa->ifa_flags;
  memset (&ifElement->broadcast_address, 0, sizeof (ifElement->broadcast_address));
  memset (&ifElement->network_mask, 0, sizeof (ifElement->network_mask));
  ifElement->interfaceIndexNo = (os_uint) if_nametoindex (ifa->ifa_name);
  return result;
}

os_result os_sockQueryInterfaces (os_ifAttributes *ifList, os_uint32 listSize, os_uint32 *validElements)
{
  os_result result = os_resultSuccess;
  unsigned int listIndex;
  struct ifaddrs *ifa_first, *ifa;
  if (getifaddrs (&ifa_first) != 0)
  {
    perror ("getifaddrs");
    return os_resultFail;
  }
  listIndex = 0;
  for (ifa = ifa_first; ifa && listIndex < listSize; ifa = ifa->ifa_next)
  {
    if (ifa->ifa_addr && ((struct sockaddr_in *) ifa->ifa_addr)->sin_family == AF_INET)
    {
      /* Get other interface attributes */
      result = os_queryInterfaceAttributesIPv4 (ifa, &ifList[listIndex]);
      if (result == os_resultSuccess)
        listIndex++;
    }

    if (result == os_resultSuccess)
      *validElements = listIndex;
  }
  freeifaddrs (ifa_first);
  return result;
}

os_result os_sockQueryIPv6Interfaces (os_ifAttributes *ifList, os_uint32 listSize, os_uint32 *validElements)
{
  struct ifaddrs* interfaceList = NULL;
  struct ifaddrs* nextInterface = NULL;
  unsigned int listIndex = 0;

  *validElements = 0;
  if (getifaddrs (&interfaceList) != 0)
  {
    return os_resultFail;
  }

  nextInterface = interfaceList;
  while (nextInterface != NULL && listIndex < listSize)
  {
    if (nextInterface->ifa_addr && nextInterface->ifa_addr->sa_family == AF_INET6)
    {
      os_sockaddr_in6 *v6Address = (os_sockaddr_in6 *) nextInterface->ifa_addr;
      if (!IN6_IS_ADDR_UNSPECIFIED (&v6Address->sin6_addr))
      {
        os_result result = os_resultSuccess;
        result = os_queryInterfaceAttributesIPv6 (nextInterface, &ifList[listIndex]);
        if (result == os_resultSuccess)
          listIndex++;
      }
    }
    nextInterface = nextInterface->ifa_next;
  }
  *validElements = listIndex;
  freeifaddrs(interfaceList);
  return os_resultSuccess;
}



void
os_sockQueryInterfaceStatusDeinit(
    void *handle)
{

}

void *
os_sockQueryInterfaceStatusInit(
    const char *ifName)
{
    return NULL;
}


os_result
os_sockQueryInterfaceStatus(
    void *handle,
    os_time timeout,
    os_boolean *status)
{
    *status = OS_FALSE;

    return os_resultFail;
}
