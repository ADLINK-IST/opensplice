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
#include "os_heap.h"
#include "ddsi_tran.h"
#include "ddsi_udp.h"
#include "q_nwif.h"
#include "q_config.h"
#include "q_log.h"
#include "q_pcap.h"
#include "os_errno.h"

extern void ddsi_factory_conn_init (ddsi_tran_factory_t factory, ddsi_tran_conn_t conn);

typedef struct ddsi_tran_factory * ddsi_udp_factory_t;

typedef struct ddsi_udp_config
{
  struct nn_group_membership *mship;
}
* ddsi_udp_config_t;

typedef struct ddsi_udp_conn
{
  struct ddsi_tran_conn m_base;
  os_socket m_sock;
#if defined _WIN32 && !defined WINCE
  WSAEVENT m_sockEvent;
#endif
  int m_diffserv;
}
* ddsi_udp_conn_t;

static struct ddsi_udp_config ddsi_udp_config_g;
static struct ddsi_tran_factory ddsi_udp_factory_g;

static os_ssize_t ddsi_udp_conn_read (ddsi_tran_conn_t conn, unsigned char * buf, os_size_t len)
{
  int err;
  os_ssize_t ret;
  struct msghdr msghdr;
  os_sockaddr_storage src;
  struct iovec msg_iov;
  socklen_t srclen = (socklen_t) sizeof (src);

  msg_iov.iov_base = (void*) buf;
  msg_iov.iov_len = len;

  memset (&msghdr, 0, sizeof (msghdr));

  msghdr.msg_name = &src;
  msghdr.msg_namelen = srclen;
  msghdr.msg_iov = &msg_iov;
  msghdr.msg_iovlen = 1;

  do {
    ret = recvmsg(((ddsi_udp_conn_t) conn)->m_sock, &msghdr, 0);
    err = (ret == -1) ? os_getErrno() : 0;
  } while (err == os_sockEINTR);

  if (ret > 0)
  {
    /* Check for udp packet truncation */
    if ((((os_size_t) ret) > len)
#if SYSDEPS_MSGHDR_FLAGS
        || (msghdr.msg_flags & MSG_TRUNC)
#endif
        )
    {
      char addrbuf[INET6_ADDRSTRLEN_EXTENDED];
      sockaddr_to_string_with_port (addrbuf, &src);
      NN_WARNING3 ("%s => %d truncated to %d\n", addrbuf, (int)ret, (int)len);
    }
  }
  else if (err != os_sockENOTSOCK && err != os_sockECONNRESET)
  {
    NN_ERROR3 ("UDP recvmsg sock %d: ret %d errno %d\n", (int) ((ddsi_udp_conn_t) conn)->m_sock, (int) ret, err);
  }
  return ret;
}

static os_ssize_t ddsi_udp_conn_write (ddsi_tran_conn_t conn, const struct msghdr * msg, os_size_t len, os_uint32 flags)
{
  int err;
  os_ssize_t ret;
  unsigned retry = 2;
  int sendflags = 0;
  (void) flags;
  (void) len;
#ifdef MSG_NOSIGNAL
  sendflags |= MSG_NOSIGNAL;
#endif
  do {
    ddsi_udp_conn_t uc = (ddsi_udp_conn_t) conn;
    ret = sendmsg (uc->m_sock, msg, sendflags);
    err = (ret == -1) ? os_getErrno() : 0;
#if defined _WIN32 && !defined WINCE
    if (err == os_sockEWOULDBLOCK) {
      WSANETWORKEVENTS ev;
      WaitForSingleObject(uc->m_sockEvent, INFINITE);
      WSAEnumNetworkEvents(uc->m_sock, uc->m_sockEvent, &ev);
    }
#endif
  } while (err == os_sockEINTR || err == os_sockEWOULDBLOCK || (err == os_sockEPERM && retry-- > 0));
  if (ret > 0 && gv.pcap_fp)
  {
    os_sockaddr_storage sa;
    socklen_t alen = sizeof (sa);
    if (getsockname (((ddsi_udp_conn_t) conn)->m_sock, (struct sockaddr *) &sa, &alen) == -1)
      memset(&sa, 0, sizeof(sa));
    write_pcap_sent (gv.pcap_fp, now (), &sa, msg, (os_size_t) ret);
  }
  else if (ret == -1)
  {
    switch (err)
    {
      case os_sockEINTR:
      case os_sockEPERM:
      case os_sockECONNRESET:
#ifdef os_sockENETUNREACH
      case os_sockENETUNREACH:
#endif
#ifdef os_sockEHOSTUNREACH
      case os_sockEHOSTUNREACH:
#endif
        break;
      default:
        NN_ERROR1("ddsi_udp_conn_write failed with error code %d", err);
    }
  }
  return ret;
}

static os_handle ddsi_udp_conn_handle (ddsi_tran_base_t base)
{
  return ((ddsi_udp_conn_t) base)->m_sock;
}

static c_bool ddsi_udp_supports (os_int32 kind)
{
  return
  (
    (!config.useIpv6 && (kind == NN_LOCATOR_KIND_UDPv4))
#if OS_SOCKET_HAS_IPV6
    || (config.useIpv6 && (kind == NN_LOCATOR_KIND_UDPv6))
#endif
  );
}

static int ddsi_udp_conn_locator (ddsi_tran_base_t base, nn_locator_t *loc)
{
  int ret = -1;
  ddsi_udp_conn_t uc = (ddsi_udp_conn_t) base;
  os_sockaddr_storage * addr = &gv.extip;

  memset (loc, 0, sizeof (*loc));
  if (uc->m_sock != Q_INVALID_SOCKET)
  {
    loc->kind = ddsi_udp_factory_g.m_kind;
    loc->port = uc->m_base.m_base.m_port;

    if (loc->kind == NN_LOCATOR_KIND_UDPv4)
    {
      memcpy (loc->address + 12, &((os_sockaddr_in*) addr)->sin_addr, 4);
    }
    else
    {
      memcpy (loc->address, &((os_sockaddr_in6*) addr)->sin6_addr, 16);
    }
    ret = 0;
  }
  return ret;
}

static ddsi_tran_conn_t ddsi_udp_create_conn
(
  os_uint32 port,
  ddsi_tran_qos_t qos
)
{
  int ret;
  os_socket sock;
  ddsi_udp_conn_t uc = NULL;
  c_bool mcast = (c_bool) (qos ? qos->m_multicast : FALSE);

  /* If port is zero, need to create dynamic port */

  ret = make_socket
  (
    &sock,
    (unsigned short) port,
    FALSE,
    mcast
  );

  if (ret == 0)
  {
    uc = (ddsi_udp_conn_t) os_malloc (sizeof (*uc));
    memset (uc, 0, sizeof (*uc));

    uc->m_sock = sock;
    uc->m_diffserv = qos ? qos->m_diffserv : 0;
#if defined _WIN32 && !defined WINCE
    uc->m_sockEvent = WSACreateEvent();
    WSAEventSelect(uc->m_sock, uc->m_sockEvent, FD_WRITE);
#endif

    ddsi_factory_conn_init (&ddsi_udp_factory_g, &uc->m_base);
    uc->m_base.m_base.m_port = get_socket_port (sock);
    uc->m_base.m_base.m_trantype = DDSI_TRAN_CONN;
    uc->m_base.m_base.m_multicast = mcast;
    uc->m_base.m_base.m_handle_fn = ddsi_udp_conn_handle;
    uc->m_base.m_base.m_locator_fn = ddsi_udp_conn_locator;

    uc->m_base.m_read_fn = ddsi_udp_conn_read;
    uc->m_base.m_write_fn = ddsi_udp_conn_write;

    nn_log
    (
      LC_INFO,
      "ddsi_udp_create_conn %s socket %d port %d\n",
      mcast ? "multicast" : "unicast",
      uc->m_sock,
      uc->m_base.m_base.m_port
    );
  }
  else
  {
    if (config.participantIndex != PARTICIPANT_INDEX_AUTO)
    {
      NN_ERROR2
      (
        "UDP make_socket failed for %s port %d\n",
        mcast ? "multicast" : "unicast",
        port
      );
    }
  }

  return uc ? &uc->m_base : NULL;
}

static int ddsi_udp_join_mc (ddsi_tran_conn_t conn, const nn_locator_t *srcloc, const nn_locator_t *mcloc)
{
  ddsi_udp_conn_t uc = (ddsi_udp_conn_t) conn;
  os_sockaddr_storage mcip, srcip;
  nn_loc_to_address (&mcip, mcloc);
  if (srcloc)
    nn_loc_to_address (&srcip, srcloc);
  return join_mcgroups (ddsi_udp_config_g.mship, uc->m_sock, srcloc ? &srcip : NULL, &mcip);
}

static int ddsi_udp_leave_mc (ddsi_tran_conn_t conn, const nn_locator_t *srcloc, const nn_locator_t *mcloc)
{
  ddsi_udp_conn_t uc = (ddsi_udp_conn_t) conn;
  os_sockaddr_storage mcip, srcip;
  nn_loc_to_address (&mcip, mcloc);
  if (srcloc)
    nn_loc_to_address (&srcip, srcloc);
  return leave_mcgroups (ddsi_udp_config_g.mship, uc->m_sock, srcloc ? &srcip : NULL, &mcip);
}

static void ddsi_udp_release_conn (ddsi_tran_conn_t conn)
{
  ddsi_udp_conn_t uc = (ddsi_udp_conn_t) conn;
  nn_log
  (
    LC_INFO,
    "ddsi_udp_release_conn %s socket %d port %d\n",
    conn->m_base.m_multicast ? "multicast" : "unicast",
    uc->m_sock,
    uc->m_base.m_base.m_port
  );
  os_sockFree (uc->m_sock);
#if defined _WIN32 && !defined WINCE
  WSACloseEvent(uc->m_sockEvent);
#endif
  os_free (conn);
}

int ddsi_udp_init (void)
{
  static c_bool init = FALSE;
  if (! init)
  {
    init = TRUE;
    memset (&ddsi_udp_factory_g, 0, sizeof (ddsi_udp_factory_g));
    ddsi_udp_factory_g.m_kind = NN_LOCATOR_KIND_UDPv4;
    ddsi_udp_factory_g.m_typename = "udp";
    ddsi_udp_factory_g.m_connless = TRUE;
    ddsi_udp_factory_g.m_supports_fn = ddsi_udp_supports;
    ddsi_udp_factory_g.m_create_conn_fn = ddsi_udp_create_conn;
    ddsi_udp_factory_g.m_release_conn_fn = ddsi_udp_release_conn;
    ddsi_udp_factory_g.m_join_mc_fn = ddsi_udp_join_mc;
    ddsi_udp_factory_g.m_leave_mc_fn = ddsi_udp_leave_mc;
#if OS_SOCKET_HAS_IPV6
    if (config.useIpv6)
    {
      ddsi_udp_factory_g.m_kind = NN_LOCATOR_KIND_UDPv6;
    }
#endif

    ddsi_factory_add (&ddsi_udp_factory_g);

    ddsi_udp_config_g.mship = new_group_membership();

    nn_log (LC_INFO | LC_CONFIG, "udp initialized\n");
  }
  return 0;
}

/* SHA1 not available (unoffical build.) */
