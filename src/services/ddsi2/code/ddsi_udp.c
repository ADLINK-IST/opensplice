#include "os_heap.h"
#include "ddsi_tran.h"
#include "ddsi_udp.h"
#include "ddsi_socket.h"
#include "q_nwif.h"
#include "q_config.h"
#include "q_log.h"

extern void ddsi_factory_conn_init (ddsi_tran_factory_t factory, ddsi_tran_conn_t conn);

typedef struct ddsi_udp_config
{
  os_sockaddr_storage m_mcip;
}
* ddsi_udp_config_t;

typedef struct ddsi_tran_factory * ddsi_udp_factory_t;

typedef struct ddsi_udp_conn
{
  struct ddsi_tran_conn m_base;
  os_socket m_sock;
  int m_diffserv;
}
* ddsi_udp_conn_t;

static struct ddsi_tran_factory ddsi_udp_factory_g;
static struct ddsi_udp_config ddsi_udp_config_g;

static os_ssize_t ddsi_udp_conn_read (ddsi_tran_conn_t conn, unsigned char * buf, os_size_t len)
{
  return ddsi_socket_read (((ddsi_udp_conn_t) conn)->m_sock, buf, len, TRUE);
}

static os_ssize_t ddsi_udp_conn_write (ddsi_tran_conn_t conn, struct msghdr * msg, os_size_t len)
{
  return ddsi_socket_write (((ddsi_udp_conn_t) conn)->m_sock, msg, len, TRUE);
}

static os_handle ddsi_udp_conn_handle (ddsi_tran_base_t base)
{
  return ((ddsi_udp_conn_t) base)->m_sock;
}

static c_bool ddsi_udp_supports (os_uint32 kind)
{
  return
  (
    (!config.useIpv6 && (kind == NN_LOCATOR_KIND_UDPv4))
#if OS_SOCKET_HAS_IPV6
    || (config.useIpv6 && (kind == NN_LOCATOR_KIND_UDPv6))
#endif
  );
}

static int ddsi_udp_conn_locator
(
  ddsi_tran_base_t base,
  nn_locator_t * loc
)
{
  int ret = -1;
  ddsi_udp_conn_t uc = (ddsi_udp_conn_t) base;
  os_sockaddr_storage * addr = (base->m_multicast) ? &ddsi_udp_config_g.m_mcip : &gv.extip;

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
  c_bool mcast = qos ? qos->m_multicast : FALSE;

  /* If port is zero, need to create dynamic port */

  ret = make_socket
  (
    &sock,
    port,
    FALSE,
    mcast,
    mcast ? &ddsi_udp_config_g.m_mcip : NULL,
    NULL
  );

  if (ret == 0)
  {
    uc = (ddsi_udp_conn_t) os_malloc (sizeof (*uc));
    memset (uc, 0, sizeof (*uc));

    uc->m_sock = sock;
    uc->m_diffserv = qos ? qos->m_diffserv : 0;

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
  os_free (conn);
}

void ddsi_udp_init (void)
{
  static c_bool init = FALSE;
  if (! init)
  {
    init = TRUE;
    ddsi_udp_factory_g.m_kind = NN_LOCATOR_KIND_UDPv4;
    ddsi_udp_factory_g.m_typename = "udp";
    ddsi_udp_factory_g.m_connless = TRUE;
    ddsi_udp_factory_g.m_stream = FALSE;
    ddsi_udp_factory_g.m_supports_fn = ddsi_udp_supports;
    ddsi_udp_factory_g.m_create_conn_fn = ddsi_udp_create_conn;
    ddsi_udp_factory_g.m_create_listener_fn = NULL;
    ddsi_udp_factory_g.m_release_conn_fn = ddsi_udp_release_conn;
    ddsi_udp_factory_g.m_release_listener_fn = NULL;
    ddsi_udp_factory_g.m_free_fn = NULL;
#if OS_SOCKET_HAS_IPV6
    if (config.useIpv6)
    {
      ddsi_udp_factory_g.m_kind = NN_LOCATOR_KIND_UDPv6;
    }
#endif

    ddsi_factory_add (&ddsi_udp_factory_g);

    if (strcmp (config.spdpMulticastAddressString, "239.255.0.1") != 0)
    {
      if (!os_sockaddrStringToAddress (config.spdpMulticastAddressString,
        (os_sockaddr*) &ddsi_udp_config_g.m_mcip, !config.useIpv6))
      {
        NN_ERROR1 ("%s: not a valid IP address\n", config.spdpMulticastAddressString);
        exit (1);
      }
    }
    else
    {
#if OS_SOCKET_HAS_IPV6
      if (config.useIpv6)
      {
        /* There isn't a standard IPv6 multicast group for DDSI. For
           some reason, node-local multicast addresses seem to be
           unsupported (ff01::... would be a node-local one I think), so
           instead do link-local. I suppose we could use the hop limit
           to make it node-local.  If other hosts reach us in some way,
           we'll of course respond. */
        os_sockaddrStringToAddress ("ff02::ffff:239.255.0.1", (os_sockaddr *) &ddsi_udp_config_g.m_mcip, 0);
      }
      else
#endif
      {
        os_sockaddrStringToAddress ("239.255.0.1", (os_sockaddr *) &ddsi_udp_config_g.m_mcip, 1);
      }
    }
    sockaddr_set_port (&ddsi_udp_config_g.m_mcip, 0);

    nn_log (LC_INFO | LC_CONFIG, "udp initialized\n");
  }
}

/* SHA1 not available (unoffical build.) */
