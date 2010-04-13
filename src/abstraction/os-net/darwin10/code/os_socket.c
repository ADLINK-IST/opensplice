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
#include <os_socket.h>

os_sockErrno
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

    if (bind(s, (struct sockaddr *)name, namelen) == -1) {
        result = os_resultFail;
    }
    return result;
}

os_int32
os_sockSendto(
    os_socket s,
    const void *msg,
    os_uint32 len,
    const struct sockaddr *to,
    os_uint32 tolen)
{
    return sendto(s, msg, len, 0, to, tolen);
}

os_int32
os_sockRecvfrom(
    os_socket s,
    void *buf,
    os_uint32 len,
    struct sockaddr *from,
    os_uint32 *fromlen)
{
    return recvfrom(s, buf, len, 0, from, (socklen_t*) fromlen);
}

os_result
os_sockGetsockopt(
    os_socket s,
    os_int32 level,
    os_int32 optname,
    void *optval,
    os_uint32 *optlen)
{
    os_result result = os_resultSuccess;

    if (getsockopt(s, level, optname, optval, (socklen_t*) optlen) == -1) {
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
    os_uint32 optlen)
{
    os_result result = os_resultSuccess;

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
    os_time *timeout)
{
    struct timeval t;
    int r;

    t.tv_sec = (long)timeout->tv_sec;
    t.tv_usec = (long)(timeout->tv_nsec / 1000);
    r = select(nfds, readfds, writefds, errorfds, &t);

    return r;
}

static os_result
os_queryInterfaceAttributes(
	const struct ifaddrs *ifa,
	os_ifAttributes *ifElement)
{
  os_result result = os_resultSuccess;
  strncpy (ifElement->name, ifa->ifa_name, OS_IFNAMESIZE);
  ifElement->address = *ifa->ifa_addr;
  ifElement->flags = ifa->ifa_flags;
  if (ifElement->flags & IFF_BROADCAST)
    ifElement->broadcast_address = *ifa->ifa_broadaddr;
  else
    memset (&ifElement->broadcast_address, 0, sizeof (ifElement->broadcast_address));
  ifElement->network_mask = *ifa->ifa_netmask;
  return result;
}

os_result
os_sockQueryInterfaces(
    os_ifAttributes *ifList,
    os_uint32 listSize,
    os_uint32 *validElements)
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
      result = os_queryInterfaceAttributes (ifa, &ifList[listIndex]);
      if (result == os_resultSuccess)
	listIndex++;
    }
    
    if (result == os_resultSuccess)
      *validElements = listIndex;
  }
  freeifaddrs (ifa_first);
  return result;
}
