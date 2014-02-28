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

#include <ctype.h>
#include <stddef.h>
#include <assert.h>

#include "os_socket.h"
#include "os_if.h"
#include "os_stdlib.h"
#include "os_heap.h"

#ifndef _WIN32
#include <netdb.h>
#endif

#include "q_log.h"
#include "q_nwif.h"
#include "q_globals.h"
#include "q_config.h"
#include "q_md5.h"
#include "q_unused.h"

void print_sockerror (const char *msg)
{
  int err = os_sockError ();
  NN_ERROR2 ("SOCKET ERROR %s %d\n", msg, err);
}

unsigned short sockaddr_get_port (const os_sockaddr_storage *addr)
{
  if (addr->ss_family == AF_INET)
    return ntohs (((os_sockaddr_in *) addr)->sin_port);
  else
    return ntohs (((os_sockaddr_in6 *) addr)->sin6_port);
}

void sockaddr_set_port (os_sockaddr_storage *addr, unsigned short port)
{
  if (addr->ss_family == AF_INET)
    ((os_sockaddr_in *) addr)->sin_port = htons (port);
  else
    ((os_sockaddr_in6 *) addr)->sin6_port = htons (port);
}

char *sockaddr_to_string_with_port (char addrbuf[INET6_ADDRSTRLEN_EXTENDED], const os_sockaddr_storage *src)
{
  int pos;
  switch (src->ss_family)
  {
    case AF_INET:
      os_sockaddrAddressToString ((const os_sockaddr *) src, addrbuf, INET6_ADDRSTRLEN);
      pos = (int) strlen (addrbuf);
      snprintf (addrbuf + pos, INET6_ADDRSTRLEN_EXTENDED - pos,
                ":%u", ntohs (((os_sockaddr_in *) src)->sin_port));
      break;
#if OS_SOCKET_HAS_IPV6
    case AF_INET6:
      addrbuf[0] = '[';
      os_sockaddrAddressToString ((const os_sockaddr *) src, addrbuf + 1, INET6_ADDRSTRLEN);
      pos = (int) strlen (addrbuf);
      snprintf (addrbuf + pos, INET6_ADDRSTRLEN_EXTENDED - pos,
                "]:%u", ntohs (((os_sockaddr_in6 *) src)->sin6_port));
      break;
#endif
    default:
      NN_WARNING0 ("sockaddr_to_string_with_port: unknown address family\n");
      strcpy (addrbuf, "???");
      break;
  }
  return addrbuf;
}

char *sockaddr_to_string_no_port (char addrbuf[INET6_ADDRSTRLEN_EXTENDED], const os_sockaddr_storage *src)
{
  return os_sockaddrAddressToString ((const os_sockaddr *) src, addrbuf, INET6_ADDRSTRLEN);
}


unsigned short get_socket_port (os_socket socket)
{
  os_sockaddr_storage addr;
  socklen_t addrlen = sizeof (addr);
  if (getsockname (socket, (os_sockaddr *) &addr, &addrlen) < 0)
  {
    print_sockerror ("getsockname");
    return 0;
  }
  switch (addr.ss_family)
  {
    case AF_INET:
      return ntohs (((os_sockaddr_in *) &addr)->sin_port);
#if OS_SOCKET_HAS_IPV6
    case AF_INET6:
      return ntohs (((os_sockaddr_in6 *) &addr)->sin6_port);
#endif
    default:
      abort ();
      return 0;
  }
}


#ifdef SO_NOSIGPIPE
static void set_socket_nosigpipe (os_socket sock)
{
  int val = 1;
  if (os_sockSetsockopt (sock, SOL_SOCKET, SO_NOSIGPIPE, (char*) &val, sizeof (val)) != os_resultSuccess)
  {
    print_sockerror ("SO_NOSIGPIPE");
  }
}
#endif

#ifdef TCP_NODELAY
static void set_socket_nodelay (os_socket sock)
{
  int val = 1;
  if (os_sockSetsockopt (sock, IPPROTO_TCP, TCP_NODELAY, (char*) &val, sizeof (val)) != os_resultSuccess)
  {
    print_sockerror ("TCP_NODELAY");
  }
}
#endif

static int set_rcvbuf (os_socket socket)
{
  int ReceiveBufferSize;
  os_uint32 optlen = (os_uint32) sizeof (ReceiveBufferSize);
  if (os_sockGetsockopt (socket, SOL_SOCKET, SO_RCVBUF, (char *) &ReceiveBufferSize, &optlen) != os_resultSuccess)
  {
    print_sockerror ("get SO_RCVBUF");
    return -2;
  }
  if (ReceiveBufferSize < config.socket_min_rcvbuf_size)
  {
    /* make sure the receive buffersize is at least the minimum required */
    ReceiveBufferSize = config.socket_min_rcvbuf_size;
    if (os_sockSetsockopt (socket, SOL_SOCKET, SO_RCVBUF, (const char *) &ReceiveBufferSize, sizeof (ReceiveBufferSize)) != os_resultSuccess)
    {
      print_sockerror ("SO_RCVBUF");
      return -2;
    }

    /* Problem is: O/Ss tend to silently cap the buffer size.  The
       only way to make sure is to read the option value back and
       check it is now set correctly. */
    if (os_sockGetsockopt (socket, SOL_SOCKET, SO_RCVBUF, (char *) &ReceiveBufferSize, &optlen) != os_resultSuccess)
    {
      print_sockerror ("get SO_RCVBUF");
      return -2;
    }
    if (ReceiveBufferSize < config.socket_min_rcvbuf_size)
    {
      NN_ERROR2 ("Failed to increase socket receive buffer size to %d bytes, continuing with %d bytes\n", config.socket_min_rcvbuf_size, ReceiveBufferSize);
    }
  }
  return 0;
}

static int set_sndbuf (os_socket socket)
{
  int SendBufferSize;
  os_uint32 optlen = (os_uint32) sizeof(SendBufferSize);
  if( os_sockGetsockopt(socket, SOL_SOCKET, SO_SNDBUF,(char *)&SendBufferSize, &optlen) != os_resultSuccess)
  {
    print_sockerror ("get SO_SNDBUF");
    return -2;
  }
  if ( SendBufferSize < config.socket_min_sndbuf_size )
  {
    /* make sure the send buffersize is at least the minimum required */
    SendBufferSize = config.socket_min_sndbuf_size;
    if (os_sockSetsockopt (socket, SOL_SOCKET, SO_SNDBUF, (const char *)&SendBufferSize, sizeof (SendBufferSize)) != os_resultSuccess)
    {
      print_sockerror ("SO_SNDBUF");
      return -2;
    }
  }
  return 0;
}

static int maybe_set_dont_route (os_socket socket)
{
  if (config.dontRoute)
  {
#if OS_SOCKET_HAS_IPV6
    if (config.useIpv6)
    {
      os_uint ipv6Flag = 1;
      if (os_sockSetsockopt (socket, IPPROTO_IPV6, IPV6_UNICAST_HOPS, &ipv6Flag, sizeof (ipv6Flag)))
      {
        print_sockerror ("IPV6_UNICAST_HOPS");
        return -2;
      }
    }
    else
#endif
    {
      int one = 1;
      if (os_sockSetsockopt (socket, SOL_SOCKET, SO_DONTROUTE, (char *) &one, sizeof (one)) != os_resultSuccess)
      {
        print_sockerror ("SO_DONTROUTE");
        return -2;
      }
    }
  }
  return 0;
}

static int set_reuse_options (os_socket socket)
{
  /* Set REUSEADDR and REUSEPORT (if available on platform) for
     multicast sockets, leave unicast sockets alone. */
  int one = 1;

  if (os_sockSetsockopt (socket, SOL_SOCKET, SO_REUSEADDR, (char *) &one, sizeof (one)) != os_resultSuccess)
  {
    print_sockerror ("SO_REUSEADDR");
    return -2;
  }
#ifdef SO_REUSEPORT
  if (os_sockSetsockopt (socket, SOL_SOCKET, SO_REUSEPORT, (char *) &one, sizeof (one)) != os_resultSuccess)
  {
    print_sockerror ("SO_REUSEPORT");
    return -2;
  }
#endif
  return 0;
}

static int interface_in_recvips_p (const struct nn_interface *interf)
{
  struct ospl_in_addr_node *nodeaddr;
  for (nodeaddr = gv.recvips; nodeaddr; nodeaddr = nodeaddr->next)
  {
    if (os_sockaddrIPAddressEqual ((const os_sockaddr *) &nodeaddr->addr, (const os_sockaddr *) &interf->addr))
      return 1;
  }
  return 0;
}

static int bind_socket (os_socket socket, unsigned short port, const char * address)
{
  int rc;
#if OS_SOCKET_HAS_IPV6
  if (config.useIpv6)
  {
    os_sockaddr_in6 socketname;
    memset (&socketname, 0, sizeof (socketname));
    socketname.sin6_family = AF_INET6;
    socketname.sin6_port = htons (port);
    if (address)
    {
#ifdef WIN32
      int sslen = sizeof (socketname);
      WSAStringToAddress ((LPTSTR) address, AF_INET6, NULL, (os_sockaddr*) &socketname, &sslen);
#else
      inet_pton (AF_INET6, address, &(socketname.sin6_addr));
#endif
    }
    else
    {
      socketname.sin6_addr = os_in6addr_any;
    }
    rc = os_sockBind (socket, (struct sockaddr *) &socketname, sizeof (socketname));
  }
  else
#endif
  {
    struct sockaddr_in socketname;
    socketname.sin_family = AF_INET;
    socketname.sin_port = htons (port);
    socketname.sin_addr.s_addr = (address == NULL) ? htonl (INADDR_ANY) : inet_addr (address);
    rc = os_sockBind (socket, (struct sockaddr *) &socketname, sizeof (socketname));
  }
  if (rc != os_resultSuccess)
  {
    if (os_sockError () != os_sockEADDRINUSE)
      print_sockerror ("bind");
    return -1;
  }
  return 0;
}

static int join_mcgroup (os_socket socket, const os_sockaddr_storage *mcip, const struct nn_interface *interf)
{
  /* Note: interf == NULL indicates default address for multicast */
  int rc;

#if OS_SOCKET_HAS_IPV6
  if (config.useIpv6)
  {
    os_ipv6_mreq ipv6mreq;
    memset (&ipv6mreq, 0, sizeof (ipv6mreq));
    memcpy (&ipv6mreq.ipv6mr_multiaddr, &((os_sockaddr_in6 *) mcip)->sin6_addr, sizeof (ipv6mreq.ipv6mr_multiaddr));
    ipv6mreq.ipv6mr_interface = interf ? interf->if_index : 0;
    rc = os_sockSetsockopt (socket, IPPROTO_IPV6, IPV6_JOIN_GROUP, &ipv6mreq, sizeof (ipv6mreq));
  }
  else
#endif
  {
    struct ip_mreq mreq;
    mreq.imr_multiaddr = ((os_sockaddr_in *) mcip)->sin_addr;
    if (interf)
      mreq.imr_interface = ((os_sockaddr_in *) &interf->addr)->sin_addr;
    else
      mreq.imr_interface.s_addr = htonl (INADDR_ANY);
    rc = os_sockSetsockopt (socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *) &mreq, sizeof (mreq));
  }

  if (rc != os_resultSuccess)
  {
    const char *op = config.useIpv6 ? "IPV6_JOIN_GROUP" : "IP_ADD_MEMBERSHIP";
    int err = os_sockError ();
    char buf1[INET6_ADDRSTRLEN_EXTENDED];
    sockaddr_to_string_no_port (buf1, mcip);
    if (interf)
    {
      char buf2[INET6_ADDRSTRLEN];
      sockaddr_to_string_no_port (buf2, &interf->addr);
      if (err == os_sockEADDRINUSE)
        NN_WARNING3 ("%s for %s failed on interface with address %s: already bound\n", op, buf1, buf2);
      else
        NN_WARNING4 ("%s for %s failed on interface with address %s (errno %d)\n", op, buf1, buf2, err);
    }
    else
    {
      if (err == os_sockEADDRINUSE)
        NN_WARNING2 ("%s for %s failed on default interface: already bound\n", op, buf1);
      else
        NN_WARNING3 ("%s for %s failed on default interface (errno %d)\n", op, buf1, err);
    }
    return -2;
  }
  return 0;
}

#if OS_SOCKET_HAS_IPV6
static int set_mc_options_transmit_ipv6 (os_socket socket)
{
  os_uint interfaceNo = gv.interfaceNo;
  os_uint ttl = config.multicast_ttl;
  unsigned loop;
  if (os_sockSetsockopt (socket, IPPROTO_IPV6, IPV6_MULTICAST_IF, &interfaceNo, sizeof (interfaceNo)) != os_resultSuccess)
  {
    print_sockerror ("IPV6_MULTICAST_IF");
    return -2;
  }
  if (os_sockSetsockopt (socket, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, (char *) &ttl, sizeof (ttl)) != os_resultSuccess)
  {
    print_sockerror ("IPV6_MULTICAST_HOPS");
    return -2;
  }
  loop = config.enableMulticastLoopback;
  if (os_sockSetsockopt (socket, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &loop, sizeof (loop)) != os_resultSuccess)
  {
    print_sockerror ("IPV6_MULTICAST_LOOP");
    return -2;
  }
  return 0;
}
#endif

static int set_mc_options_transmit_ipv4 (os_socket socket)
{
  unsigned char ttl = config.multicast_ttl;
  unsigned char loop;

  if (os_sockSetsockopt (socket, IPPROTO_IP, IP_MULTICAST_IF, (char *) &((os_sockaddr_in *) &gv.ownip)->sin_addr, sizeof (((os_sockaddr_in *) &gv.ownip)->sin_addr)) != os_resultSuccess)
  {
    print_sockerror ("IP_MULTICAST_IF");
    return -2;
  }
  if (os_sockSetsockopt (socket, IPPROTO_IP, IP_MULTICAST_TTL, (char *) &ttl, sizeof (ttl)) != os_resultSuccess)
  {
    print_sockerror ("IP_MULICAST_TTL");
    return -2;
  }
  loop = config.enableMulticastLoopback;

  if (os_sockSetsockopt (socket, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof (loop)) != os_resultSuccess)
  {
    print_sockerror ("IP_MULTICAST_LOOP");
    return -2;
  }
  return 0;
}

static int set_mc_options_transmit (os_socket socket)
{
#if OS_SOCKET_HAS_IPV6
  if (config.useIpv6)
  {
    return set_mc_options_transmit_ipv6 (socket);
  }
  else
#endif
  {
    return set_mc_options_transmit_ipv4 (socket);
  }
}

int join_mcgroups (os_socket socket, const os_sockaddr_storage *mcip)
{
  int rc;
  switch (gv.recvips_mode)
  {
    case RECVIPS_MODE_NONE:
      break;
    case RECVIPS_MODE_ANY:
      /* User has specified to use the OS default interface */
      if ((rc = join_mcgroup (socket, mcip, NULL)) < 0)
        return rc;
      break;
    case RECVIPS_MODE_PREFERRED:
      if (gv.interfaces[gv.selected_interface].mc_capable)
        return join_mcgroup (socket, mcip, &gv.interfaces[gv.selected_interface]);
      return 0;
    case RECVIPS_MODE_ALL:
    case RECVIPS_MODE_SOME:
      {
        int i, fails = 0, oks = 0;
        for (i = 0; i < gv.n_interfaces; i++)
        {
          if (gv.interfaces[i].mc_capable)
          {
            if (gv.recvips_mode == RECVIPS_MODE_ALL || interface_in_recvips_p (&gv.interfaces[i]))
            {
              if ((rc = join_mcgroup (socket, mcip, &gv.interfaces[i])) < 0)
                fails++;
              else
                oks++;
            }
          }
        }
        if (fails > 0)
        {
          if (oks > 0)
            TRACE (("multicast join failed for some but not all interfaces, proceeding\n"));
          else
            return -2;
        }
      }
      break;
  }
  return 0;
}

int make_socket
(
  os_socket * sock,
  unsigned short port,
  c_bool stream,
  c_bool reuse,
  const os_sockaddr_storage * mcip,
  const char * address
)
{
  int rc = -2;

  *sock = os_sockNew ((config.useIpv6 ? AF_INET6 : AF_INET), stream ? SOCK_STREAM : SOCK_DGRAM);

  if (! Q_VALID_SOCKET (*sock))
  {
    print_sockerror ("socket");
    return rc;
  }

  if (port && reuse && ((rc = set_reuse_options (*sock)) < 0))
  {
    goto fail;
  }

  if
  (
    (rc = set_rcvbuf (*sock) < 0) ||
    (rc = set_sndbuf (*sock) < 0) ||
    ((rc = maybe_set_dont_route (*sock)) < 0) ||
    ((rc = bind_socket (*sock, port, address)) < 0)
  )
  {
    goto fail;
  }

  if (! stream)
  {
    if ((rc = set_mc_options_transmit (*sock)) < 0)
    {
      goto fail;
    }
  }

  if (stream)
  {
#ifdef SO_NOSIGPIPE
    set_socket_nosigpipe (*sock);
#endif
#ifdef TCP_NODELAY
    if (config.tcp_nodelay)
    {
      set_socket_nodelay (*sock);
    }
#endif
  }

  if (mcip && ((rc = join_mcgroups (*sock, mcip)) < 0))
  {
    goto fail;
  }

  return 0;

fail:

  os_sockFree (*sock);
  *sock = Q_INVALID_SOCKET;
  return rc;
}

int find_own_ip (const char *requested_address)
{
  const char *sep = " ";
  char last_if_name[80] = "";
  int quality = -1;
  os_result res;
  int i;
  unsigned int nif;
  os_ifAttributes *ifs;
  int maxq_list[MAX_INTERFACES];
  int maxq_count = 0;
  int maxq_strlen = 0;
  int selected_idx = -1;
  char addrbuf[INET6_ADDRSTRLEN_EXTENDED];

  if ((ifs = os_malloc (MAX_INTERFACES * sizeof (*ifs))) == NULL)
  {
    NN_FATAL0 ("ddsi2: insufficient memory for enumerating network interfaces\n");
    return 0;
  }

  nn_log (LC_CONFIG, "interfaces:");

  if (config.useIpv6)
    res = os_sockQueryIPv6Interfaces (ifs, (os_uint32) MAX_INTERFACES, &nif);
  else
    res = os_sockQueryInterfaces (ifs, (os_uint32) MAX_INTERFACES, &nif);
  if (res != os_resultSuccess)
  {
    NN_ERROR1 ("os_sockQueryInterfaces: %d\n", (int) res);
    os_free (ifs);
    return 0;
  }

  gv.n_interfaces = 0;
  for (i = 0; i < (int) nif; i++, sep = ", ")
  {
    os_sockaddr_storage tmpip, tmpmask;
    char if_name[sizeof (last_if_name)];
    int q = 0;

    os_strncpy (if_name, ifs[i].name, sizeof (if_name) - 1);
    if_name[sizeof (if_name) - 1] = 0;

    if (strcmp (if_name, last_if_name))
      nn_log (LC_CONFIG, "%s%s", sep, if_name);
    os_strcpy (last_if_name, if_name);

    /* interface must be up */
    if ((ifs[i].flags & IFF_UP) == 0)
    {
      nn_log (LC_CONFIG, " (interface down)");
      continue;
    }

    tmpip = ifs[i].address;
    tmpmask = ifs[i].network_mask;
    sockaddr_to_string_no_port (addrbuf, &tmpip);
    nn_log (LC_CONFIG, " %s", addrbuf);

    if (ifs[i].flags & IFF_LOOPBACK)
    {
      /* Loopback device has the lowest priority of every interface
         available, because the other interfaces at least in principle
         allow communicating with other machines. */
      q += 0;
#if OS_SOCKET_HAS_IPV6
      if (!(tmpip.ss_family == AF_INET6 && IN6_IS_ADDR_LINKLOCAL (&((os_sockaddr_in6 *) &tmpip)->sin6_addr)))
        q += 1;
#endif
    }
    else
    {
#if OS_SOCKET_HAS_IPV6
      /* We accept link-local IPv6 addresses, but an interface with a
         link-local address will end up lower in the ordering than one
         with a global address.  When forced to use a link-local
         address, we restrict ourselves to operating on that one
         interface only and assume any advertised (incoming) link-local
         address belongs to that interface.  FIXME: this is wrong, and
         should be changed to tag addresses with the interface over
         which it was received.  But that means proper multi-homing
         support and has quite an impact in various places, not least of
         which is the abstraction layer. */
      if (!(tmpip.ss_family == AF_INET6 && IN6_IS_ADDR_LINKLOCAL (&((os_sockaddr_in6 *) &tmpip)->sin6_addr)))
        q += 5;
#endif

      /* We strongly prefer a multicast capable interface, if that's
         not available anything that's not point-to-point, or else we
         hope IP routing will take care of the issues. */
      if (ifs[i].flags & IFF_MULTICAST)
        q += 4;
      else if (!(ifs[i].flags & IFF_POINTOPOINT))
        q += 3;
      else
        q += 2;
    }

    nn_log (LC_CONFIG, "(q%d)", q);
    if (q == quality) {
      maxq_list[maxq_count] = gv.n_interfaces;
      maxq_strlen += 2 + strlen (if_name);
      maxq_count++;
    } else if (q > quality) {
      maxq_list[0] = gv.n_interfaces;
      maxq_strlen += 2 + strlen (if_name);
      maxq_count = 1;
      quality = q;
    }

    gv.interfaces[gv.n_interfaces].addr = tmpip;
    gv.interfaces[gv.n_interfaces].netmask = tmpmask;
    gv.interfaces[gv.n_interfaces].mc_capable = ((ifs[i].flags & IFF_MULTICAST) != 0);
    gv.interfaces[gv.n_interfaces].point_to_point = ((ifs[i].flags & IFF_POINTOPOINT) != 0);
    gv.interfaces[gv.n_interfaces].if_index = ifs[i].interfaceIndexNo;
    gv.interfaces[gv.n_interfaces].name = os_strdup (if_name);
    gv.n_interfaces++;
  }
  nn_log (LC_CONFIG, "\n");
  os_free (ifs);

  if (requested_address == NULL)
  {
    if (maxq_count > 1)
    {
      const int idx = maxq_list[0];
      char *names;
      sockaddr_to_string_no_port (addrbuf, &gv.interfaces[idx].addr);
      if ((names = os_malloc (maxq_strlen + 1)) == NULL)
        NN_WARNING2 ("using network interface %s (%s) out of multiple candidates\n",
                     gv.interfaces[idx].name, addrbuf);
      else
      {
        int p = 0;
        for (i = 0; i < maxq_count; i++)
          p += snprintf (names + p, maxq_strlen - p, ", %s", gv.interfaces[maxq_list[i]].name);
        NN_WARNING3 ("using network interface %s (%s) selected arbitrarily from: %s\n",
                     gv.interfaces[idx].name, addrbuf, names + 2);
        os_free (names);
      }
    }

    if (maxq_count > 0)
      selected_idx = maxq_list[0];
    else
      NN_ERROR0 ("failed to determine default own IP address\n");
  }
  else
  {
    os_sockaddr_storage req;
    if (!os_sockaddrStringToAddress (config.networkAddressString, (os_sockaddr *) &req, !config.useIpv6))
    {
      /* Presumably an interface name */
      for (i = 0; i < gv.n_interfaces; i++)
        if (strcmp (gv.interfaces[i].name, config.networkAddressString) == 0)
          break;
    }
    else
    {
      /* Try an exact match on the address */
      for (i = 0; i < gv.n_interfaces; i++)
        if (os_sockaddrIPAddressEqual ((os_sockaddr *) &gv.interfaces[i].addr, (os_sockaddr *) &req))
          break;
      if (i == gv.n_interfaces && !config.useIpv6)
      {
        /* Try matching on network portion only, where the network
           portion is based on the netmask of the interface under
           consideration */
        for (i = 0; i < gv.n_interfaces; i++)
        {
          os_sockaddr_storage req1 = req, ip1 = gv.interfaces[i].addr;
          assert (req1.ss_family == AF_INET);
          assert (ip1.ss_family == AF_INET);

          /* If the host portion of the requested address is non-zero,
             skip this interface */
          if (((os_sockaddr_in *) &req1)->sin_addr.s_addr &
              ~((os_sockaddr_in *) &gv.interfaces[i].netmask)->sin_addr.s_addr)
            continue;

          ((os_sockaddr_in *) &req1)->sin_addr.s_addr &=
            ((os_sockaddr_in *) &gv.interfaces[i].netmask)->sin_addr.s_addr;
          ((os_sockaddr_in *) &ip1)->sin_addr.s_addr &=
            ((os_sockaddr_in *) &gv.interfaces[i].netmask)->sin_addr.s_addr;
          if (os_sockaddrIPAddressEqual ((os_sockaddr *) &ip1, (os_sockaddr *) &req1))
            break;
        }
      }
    }

    if (i < gv.n_interfaces)
      selected_idx = i;
    else
      NN_ERROR1 ("%s: does not match an available interface\n", config.networkAddressString);
  }

  if (selected_idx < 0)
    return 0;
  else
  {
    gv.ownip = gv.interfaces[selected_idx].addr;
    sockaddr_set_port (&gv.ownip, 0);
    gv.selected_interface = selected_idx;
    gv.interfaceNo = gv.interfaces[selected_idx].if_index;
#if OS_SOCKET_HAS_IPV6
    if (config.useIpv6)
    {
      assert (gv.ownip.ss_family == AF_INET6);
      gv.ipv6_link_local =
        IN6_IS_ADDR_LINKLOCAL (&((os_sockaddr_in6 *) &gv.ownip)->sin6_addr) != 0;
    }
    else
    {
      gv.ipv6_link_local = 0;
    }
#endif
    nn_log (LC_CONFIG, "selected interface: %s (index %u)\n",
            gv.interfaces[selected_idx].name, (unsigned) gv.interfaceNo);
    return 1;
  }
}

/* SHA1 not available (unoffical build.) */
