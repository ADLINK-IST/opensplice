#include <stddef.h>

#include "os_heap.h"
#include "ddsi_tran.h"
#include "ddsi_tcp.h"
#include "ddsi_socket.h"
#include "ut_avl.h"
#include "q_nwif.h"
#include "q_config.h"
#include "q_log.h"
#include "q_entity.h"

typedef struct ddsi_tran_factory * ddsi_tcp_factory_g_t;

typedef struct ddsi_tcp_conn
{
  struct ddsi_tran_conn m_base;
  os_sockaddr_storage m_peer_addr;
  os_socket m_sock;
}
* ddsi_tcp_conn_t;

typedef struct ddsi_tcp_listener
{
  struct ddsi_tran_listener m_base;
  os_socket m_sock;
}
* ddsi_tcp_listener_t;

/* 
  ddsi_tcp_entry: Cached TCP connection for writing. Mutex prevents concurrent
  writes to socket. Is reference counted. Client connections removed from cache
  on failed socket write. Port is actually contained in address but is extracted
  for convenience and for faster cache lookup (see ddsi_tcp_cmp_entry).
  Where connection is server side socket (for bi-dir) is flagged as such to
  avoid connection attempts and for same reason, on failure, is not removed from
  cache but simply flagged as failed (may be subsequently replaced). Similarly
  server side sockets are not closed as are also used in socket wait set that
  manages their lifecycle.
*/

typedef struct ddsi_tcp_entry
{
  os_sockaddr_storage m_addr;
  os_socket m_sock;
  os_uint32 m_port;
  os_uint32 m_refc;
  c_bool m_server;
  os_mutex m_mutex;
}
* ddsi_tcp_entry_t;

typedef struct ddsi_tcp_node
{
  ut_avlNode_t m_avlnode;
  ddsi_tcp_entry_t m_entry;
}
* ddsi_tcp_node_t;

static int ddsi_tcp_cmp_entry (const ddsi_tcp_entry_t e1, const ddsi_tcp_entry_t e2)
{
  if (e1->m_port == e2->m_port)
  {
    return (memcmp (&e1->m_addr, &e2->m_addr, sizeof (e1->m_addr)));
  }
  return (e1->m_port < e2->m_port) ? -1 : 1;
}

static const ut_avlTreedef_t ddsi_tcp_treedef = UT_AVL_TREEDEF_INITIALIZER_INDKEY
(
  offsetof (struct ddsi_tcp_node, m_avlnode),
  offsetof (struct ddsi_tcp_node, m_entry),
  (ut_avlCompare_t) ddsi_tcp_cmp_entry,
  0
);

static os_mutex ddsi_tcp_cache_lock_g;
static ut_avlTree_t ddsi_tcp_cache_g;
static struct ddsi_tran_factory ddsi_tcp_factory_g;
static os_uint32 ddsi_tcp_listener_port_g = 0;

static ddsi_tcp_conn_t ddsi_tcp_new_conn (os_socket sock, os_sockaddr * peer, const socklen_t len);
extern void ddsi_factory_conn_init (ddsi_tran_factory_t factory, ddsi_tran_conn_t conn);

/* Connection cache dump routine for debugging 

static void ddsi_tcp_cache_dump (void)
{
  char buff[64];
  ut_avlIter_t iter;
  ddsi_tcp_node_t n;

  n = ut_avlIterFirst (&ddsi_tcp_treedef, &ddsi_tcp_cache_g, &iter);
  while (n)
  {
    os_sockaddrAddressPortToString ((const os_sockaddr *) &n->m_entry->m_addr, buff, sizeof (buff));
    nn_log (LC_INFO, "TCP Cache: sock %d addr %s\n", n->m_entry->m_sock, buff);
    n = ut_avlIterNext (&iter);
  }
}
*/

static void ddsi_tcp_entry_free (ddsi_tcp_entry_t entry)
{
  if (atomic_dec_u32_nv (&entry->m_refc) == 0)
  {
    if (! entry->m_server)
    {
      os_sockFree (entry->m_sock);
    }
    os_mutexDestroy (&entry->m_mutex);
    os_free (entry);
  }
}

static void ddsi_tcp_node_free (ddsi_tcp_node_t node)
{
  ddsi_tcp_entry_free (node->m_entry);
  os_free (node);
}

static os_socket ddsi_tcp_conn_connect (const struct msghdr * msg)
{
  int ret = 0;
  int err;
  os_socket sock = Q_INVALID_SOCKET;

  ret = make_socket (&sock, 0, TRUE, TRUE, NULL, NULL);
  if (ret == 0)
  {
    /* Attempt to connect, expected that may fail */

    do
    {
      ret = connect (sock, msg->msg_name, msg->msg_namelen);
      if (ret == -1)
      {
        err = os_sockError ();
      }
    }
    while ((ret == -1) && (err == os_sockEINTR));

    if (ret == 0)
    {
      char buff[64];
      os_sockaddrAddressPortToString (msg->msg_name, buff, sizeof (buff));
      nn_log (LC_INFO, "tcp connect socket %d port %d to %s\n", sock, get_socket_port (sock), buff);

      /* If not publishing locators, also need to receive on connection */

      if (! config.publish_uc_locators)
      {
        ddsi_tcp_conn_t conn = ddsi_tcp_new_conn (sock, msg->msg_name, msg->msg_namelen);
        os_sockWaitsetAdd (gv.waitset, &conn->m_base.m_base, OS_EVENT_READ);
        os_sockWaitsetTrigger (gv.waitset);
      }
    }
    else
    {
      os_sockFree (sock);
      sock = Q_INVALID_SOCKET;
    }
  }
  return sock;
}

static ddsi_tcp_node_t ddsi_tcp_cache_add 
(
  os_socket sock,
  os_sockaddr_storage * addr,
  ut_avlIPath_t * path
)
{
  char buff[64];
  ddsi_tcp_node_t node = NULL;
  c_bool existing = TRUE;
  
  if (path == NULL)
  {
    struct ddsi_tcp_entry key;

    /* May be replacing existing connection */

    memset (&key, 0, sizeof (key));
    key.m_port = sockaddr_get_port (addr);
    key.m_addr = *addr;
    node = ut_avlLookup (&ddsi_tcp_treedef, &ddsi_tcp_cache_g, &key);
  }

  if (node == NULL)
  {
    existing = FALSE;
    node = os_malloc (sizeof (*node));
    node->m_entry = os_malloc (sizeof (*node->m_entry));
    os_mutexInit (&node->m_entry->m_mutex, &gv.mattr);
    node->m_entry->m_refc = 1;
    node->m_entry->m_port = sockaddr_get_port (addr);
    node->m_entry->m_server = (sock != Q_INVALID_SOCKET);
    node->m_entry->m_addr = *addr;
  }
  node->m_entry->m_sock = sock;

  os_sockaddrAddressPortToString ((const os_sockaddr *) addr, buff, sizeof (buff));
  nn_log 
  (
    LC_INFO,
    "tcp cache added %s socket %d to %s\n",
    node->m_entry->m_server ? "server" : "client",
    sock,
    buff
  );

  if (path)
  {
    ut_avlInsertIPath (&ddsi_tcp_treedef, &ddsi_tcp_cache_g, node, path);
  }
  else if (! existing)
  {
    ut_avlInsert (&ddsi_tcp_treedef, &ddsi_tcp_cache_g, node);
  }
  return node;
}

static void ddsi_tcp_cache_remove (ddsi_tcp_entry_t entry)
{
  char buff[64];
  ddsi_tcp_node_t node;
  ut_avlDPath_t path;

  os_mutexLock (&ddsi_tcp_cache_lock_g);
  node = ut_avlLookupDPath (&ddsi_tcp_treedef, &ddsi_tcp_cache_g, entry, &path);
  if (node)
  {
    os_sockaddrAddressPortToString ((const os_sockaddr *) &entry->m_addr, buff, sizeof (buff));
    nn_log (LC_INFO, "tcp cache removed socket %d to %s\n", entry->m_sock, buff);
    ut_avlDeleteDPath (&ddsi_tcp_treedef, &ddsi_tcp_cache_g, node, &path);
  }
  os_mutexUnlock (&ddsi_tcp_cache_lock_g);
}

static ddsi_tcp_entry_t ddsi_tcp_cache_find (const struct msghdr * msg)
{
  ut_avlIPath_t path;
  ddsi_tcp_node_t node;
  struct ddsi_tcp_entry key;
  ddsi_tcp_entry_t ret;
  os_socket sock = Q_INVALID_SOCKET;

  memset (&key, 0, sizeof (key));
  key.m_port = sockaddr_get_port (msg->msg_name);
  memcpy (&key.m_addr, msg->msg_name, msg->msg_namelen);

  /* Check cache for existing connection to target */

  os_mutexLock (&ddsi_tcp_cache_lock_g);
  node = ut_avlLookupIPath (&ddsi_tcp_treedef, &ddsi_tcp_cache_g, &key, &path);
  if (node == NULL)
  {
    node = ddsi_tcp_cache_add (sock, &key.m_addr, &path);
  }
  ret = node->m_entry;
  atomic_inc_u32_nv (&ret->m_refc);
  os_mutexUnlock (&ddsi_tcp_cache_lock_g);

  /* Attempt to establish new conection */

  os_mutexLock (&ret->m_mutex);
  if ((ret->m_sock == Q_INVALID_SOCKET) && ! ret->m_server)
  {
    ret->m_sock = ddsi_tcp_conn_connect (msg);
    if (ret->m_sock != Q_INVALID_SOCKET)
    {
      os_sockSetNonBlocking (sock, OS_TRUE);
    }
  }

  if (ret->m_sock == Q_INVALID_SOCKET)
  {
    atomic_dec_u32_nv (&ret->m_refc);
    os_mutexUnlock (&ret->m_mutex);
    ret = NULL;
  }

  return ret;
}

static os_ssize_t ddsi_tcp_conn_read
(
  ddsi_tran_conn_t conn,
  unsigned char * buf,
  os_size_t len
)
{
  return ddsi_socket_read (((ddsi_tcp_conn_t) conn)->m_sock, buf, len, FALSE);
}

static os_ssize_t ddsi_tcp_conn_write
(
  ddsi_tran_conn_t base,
  struct msghdr * msg,
  os_size_t len
)
{
  os_ssize_t ret = -1;
  ddsi_tcp_entry_t entry;

  (void) base;

  entry = ddsi_tcp_cache_find (msg);
  if (entry)
  {
    ret = ddsi_socket_write (entry->m_sock, msg, len, FALSE);

    /* Disable if write failed */

    if (ret < 0)
    {
      if (entry->m_server)
      {
        /* Leave in cache to avoid reconnection attempts */

        entry->m_sock = Q_INVALID_SOCKET;
        purge_proxy_participants (&entry->m_addr, entry->m_port);
      }
      else
      {
        ddsi_tcp_cache_remove (entry);
      }
    }
    atomic_dec_u32_nv (&entry->m_refc);
    os_mutexUnlock (&entry->m_mutex);
    if ((ret < 0) && ! entry->m_server)
    {
      ddsi_tcp_entry_free (entry);
    }
  }

  return ret;
}

static os_handle ddsi_tcp_conn_handle (ddsi_tran_base_t base)
{
  return ((ddsi_tcp_conn_t) base)->m_sock;
}

static c_bool ddsi_tcp_supports (os_uint32 kind)
{
  return 
  (
    (!config.useIpv6 && (kind == NN_LOCATOR_KIND_TCPv4))
#if OS_SOCKET_HAS_IPV6
    || (config.useIpv6 && (kind == NN_LOCATOR_KIND_TCPv6))
#endif
  );
}

static int ddsi_tcp_locator (ddsi_tran_base_t base, nn_locator_t * loc)
{
  os_sockaddr_storage * addr = &gv.extip;

  (void) base;
  memset (loc, 0, sizeof (*loc));

  /* Locator is always listener endpoint */

  loc->port = ddsi_tcp_listener_port_g;
  loc->kind = ddsi_tcp_factory_g.m_kind;
  if (loc->kind == NN_LOCATOR_KIND_TCPv4)
  {
    memcpy (loc->address + 12, &((os_sockaddr_in*) addr)->sin_addr, 4);
  }
#if OS_SOCKET_HAS_IPV6
  else
  {
    memcpy (loc->address, &((os_sockaddr_in6*) addr)->sin6_addr, 16);
  }
#endif
  return 0;
}

static ddsi_tran_conn_t ddsi_tcp_create_conn (os_uint32 port, ddsi_tran_qos_t qos)
{
  (void) qos;
  (void) port;

  return (ddsi_tran_conn_t) ddsi_tcp_new_conn (Q_INVALID_SOCKET, NULL, 0);
}

static int ddsi_tcp_listen (ddsi_tran_listener_t listener)
{
  return listen (((ddsi_tcp_listener_t) listener)->m_sock, 4);
}

static void ddsi_tcp_accept
(
  ddsi_tran_listener_t listener,
  ddsi_tran_conn_t * conn
)
{
  ddsi_tcp_listener_t tl = (ddsi_tcp_listener_t) listener;
  ddsi_tcp_conn_t tc;
  os_socket sock;
  os_sockaddr_storage addr;
  socklen_t addrlen;
  int err = 0;
  char buff[64];

  memset (&addr, 0, sizeof (addr));
  do
  {
    addrlen = sizeof (addr);
    sock = accept (tl->m_sock, (struct sockaddr *) &addr, &addrlen);
    if (sock == Q_INVALID_SOCKET)
    {
      err = os_sockError ();
    }
  }
  while ((sock == Q_INVALID_SOCKET) && ((err == os_sockEINTR) ||
    (err == os_sockEAGAIN) || (err == os_sockEWOULDBLOCK)));

  if (sock != Q_INVALID_SOCKET)
  {
    os_sockaddrAddressPortToString ((const os_sockaddr *) &addr, buff, sizeof (buff)),

    nn_log
    (
      LC_INFO,
      "tcp accept new socket %d on socket %d port %d from %s\n",
      sock,
      tl->m_sock,
      tl->m_base.m_base.m_port,
      buff
    );

    tc = ddsi_tcp_new_conn (sock, (os_sockaddr*) &addr, addrlen);
    tc->m_base.m_listener = listener;
    tc->m_base.m_conn = listener->m_connections;
    listener->m_connections = &tc->m_base;
    *conn = &tc->m_base;

    /* Add connecting address to cache (for bi-dir) */

    os_mutexLock (&ddsi_tcp_cache_lock_g);
    os_sockSetNonBlocking (sock, OS_TRUE);
    ddsi_tcp_cache_add (sock, &addr, NULL);
    os_mutexUnlock (&ddsi_tcp_cache_lock_g);
  }
  else
  {
    *conn = NULL;
    nn_log 
    (
      LC_FATAL,
      "tcp accept failed on socket %d port %d errno %d\n",
      tl->m_sock,
      tl->m_base.m_base.m_port,
      err
    );
  }
}

static os_handle ddsi_tcp_listener_handle (ddsi_tran_base_t base)
{
  return ((ddsi_tcp_listener_t) base)->m_sock;
}

static void ddsi_tcp_conn_address (ddsi_tran_conn_t conn, os_sockaddr_storage * addr)
{
  ddsi_tcp_conn_t tc = (ddsi_tcp_conn_t) conn;

  assert (tc->m_base.m_server);
  assert (tc->m_sock != Q_INVALID_SOCKET);

  *addr = tc->m_peer_addr;
}

static ddsi_tcp_conn_t ddsi_tcp_new_conn 
(
  os_socket sock,
  os_sockaddr * peer,
  const socklen_t len
)
{
  ddsi_tcp_conn_t tc = (ddsi_tcp_conn_t) os_malloc (sizeof (*tc));

  memset (tc, 0, sizeof (*tc));
  ddsi_factory_conn_init (&ddsi_tcp_factory_g, &tc->m_base);
  tc->m_base.m_base.m_trantype = DDSI_TRAN_CONN;
  tc->m_base.m_base.m_handle_fn = ddsi_tcp_conn_handle;
  tc->m_base.m_base.m_locator_fn = ddsi_tcp_locator;
  tc->m_base.m_read_fn = ddsi_tcp_conn_read;
  tc->m_base.m_write_fn = ddsi_tcp_conn_write;
  tc->m_base.m_addr_fn = ddsi_tcp_conn_address;
  tc->m_sock = sock;

  if (sock != Q_INVALID_SOCKET)
  {
    assert (len <= sizeof (os_sockaddr_storage));
    memcpy (&tc->m_peer_addr, peer, len);
    tc->m_base.m_base.m_port = get_socket_port (sock);
    tc->m_base.m_server = TRUE;
  }

  return tc;
}

static ddsi_tran_listener_t ddsi_tcp_create_listener (ddsi_tran_qos_t qos)
{
  os_int32 ret;
  os_socket sock;
  ddsi_tcp_listener_t impl = NULL;

  (void) qos;

  ret = make_socket (&sock, config.tcp_port, TRUE, TRUE, NULL, NULL);

  if (ret >= 0)
  {
    impl = (ddsi_tcp_listener_t) os_malloc (sizeof (*impl));

    impl->m_sock = sock;

    impl->m_base.m_base.m_port = get_socket_port (sock);
    impl->m_base.m_base.m_trantype = DDSI_TRAN_LISTENER;
    impl->m_base.m_base.m_handle_fn = ddsi_tcp_listener_handle;
    impl->m_base.m_base.m_locator_fn = ddsi_tcp_locator;

    impl->m_base.m_listen_fn = ddsi_tcp_listen;
    impl->m_base.m_accept_fn = ddsi_tcp_accept;

    impl->m_base.m_listener = NULL;
    impl->m_base.m_connections = NULL;
    impl->m_base.m_factory = &ddsi_tcp_factory_g;

    ddsi_tcp_listener_port_g = impl->m_base.m_base.m_port;

    nn_log
    (
      LC_INFO,
      "tcp create listener on socket %d port %d\n",
      sock,
      impl->m_base.m_base.m_port
    );
  }

  return impl ? &impl->m_base : NULL;
}

static void ddsi_tcp_release_conn (ddsi_tran_conn_t conn)
{
  ddsi_tcp_conn_t tc = (ddsi_tcp_conn_t) conn;
  os_sockFree (tc->m_sock);
  os_free (tc);
}

static void ddsi_tcp_release_listener (ddsi_tran_listener_t listener)
{
  ddsi_tcp_listener_t tl = (ddsi_tcp_listener_t) listener;
  os_sockFree (tl->m_sock);
  os_free (tl);
}

static void ddsi_tcp_release_factory (void)
{
  ut_avlFree (&ddsi_tcp_treedef, &ddsi_tcp_cache_g, (void (*) (void*)) ddsi_tcp_node_free);
  os_mutexDestroy (&ddsi_tcp_cache_lock_g);
}

void ddsi_tcp_init (void)
{
  static c_bool init = FALSE;
  if (!init)
  {
    init = TRUE;
    ddsi_tcp_factory_g.m_kind = NN_LOCATOR_KIND_TCPv4;
    ddsi_tcp_factory_g.m_typename = "tcp";
    ddsi_tcp_factory_g.m_stream = TRUE;
    ddsi_tcp_factory_g.m_connless = FALSE;
    ddsi_tcp_factory_g.m_supports_fn = ddsi_tcp_supports;
    ddsi_tcp_factory_g.m_create_listener_fn = ddsi_tcp_create_listener;
    ddsi_tcp_factory_g.m_create_conn_fn = ddsi_tcp_create_conn;
    ddsi_tcp_factory_g.m_release_conn_fn = ddsi_tcp_release_conn;
    ddsi_tcp_factory_g.m_release_listener_fn = ddsi_tcp_release_listener;
    ddsi_tcp_factory_g.m_free_fn = ddsi_tcp_release_factory;
    ddsi_factory_add (&ddsi_tcp_factory_g);

#if OS_SOCKET_HAS_IPV6
    if (config.useIpv6)
    {
      ddsi_tcp_factory_g.m_kind = NN_LOCATOR_KIND_TCPv6;
    }
#endif

    ut_avlInit (&ddsi_tcp_treedef, &ddsi_tcp_cache_g);
    os_mutexInit (&ddsi_tcp_cache_lock_g, &gv.mattr);

    nn_log (LC_INFO | LC_CONFIG, "tcp initialized\n");

    if (config.tcp_locators == DDSI_TCP_LOCATORS_NONE)
    {
      config.publish_uc_locators = FALSE;
    }
    else
    {
      /* If using real local unicast locators, must run listener */

      if (config.tcp_port == -1)
      {
        config.tcp_port = 0;
      }
    }

    /* TCP effects what features are supported/required */

    config.suppress_spdp_multicast = TRUE;
    config.many_sockets_mode = FALSE;
    config.allowMulticast = FALSE;
  }
}

/* SHA1 not available (unoffical build.) */
