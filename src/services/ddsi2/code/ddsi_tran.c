#include "os_heap.h"
#include "ddsi_tran.h"
#include "q_config.h"
#include "q_log.h"

static ddsi_tran_factory_t ddsi_tran_factories = NULL;

void ddsi_factory_add (ddsi_tran_factory_t factory)
{
  factory->m_factory = ddsi_tran_factories;
  ddsi_tran_factories = factory;
}

ddsi_tran_factory_t ddsi_factory_find (const char * type)
{
  ddsi_tran_factory_t factory = ddsi_tran_factories;

  while (factory)
  {
     if (strcmp (factory->m_typename, type) == 0)
     {
        break;
     }
     factory = factory->m_factory;
  }

  return factory;
}

void ddsi_factory_free (ddsi_tran_factory_t factory)
{
  if (factory && factory->m_free_fn)
  {
    (factory->m_free_fn) ();
  }
}

void ddsi_conn_free (ddsi_tran_conn_t conn)
{
  if (conn && atomic_dec_u32_nv (&conn->m_count) == 0)
  {
    (conn->m_factory->m_release_conn_fn) (conn);
  }
}

void ddsi_conn_add_ref (ddsi_tran_conn_t conn)
{
  atomic_inc_u32_nv (&conn->m_count);
}

extern void ddsi_factory_conn_init (ddsi_tran_factory_t factory, ddsi_tran_conn_t conn)
{
  conn->m_count = 1;
  conn->m_connless = factory->m_connless;
  conn->m_stream = factory->m_stream;
  conn->m_factory = factory;
}

os_ssize_t ddsi_conn_read (ddsi_tran_conn_t conn, unsigned char * buf, os_size_t len)
{
  return (conn->m_read_fn) (conn, buf, len);
}

os_ssize_t ddsi_conn_write (ddsi_tran_conn_t conn, struct msghdr * msg, os_size_t len)
{
  os_ssize_t ret = (conn->m_write_fn) (conn, msg, len);

  /* Check that write function is atomic (all or nothing) */

  assert (ret == -1 || (os_size_t) ret == len);
  return ret;
}

c_bool ddsi_conn_address (ddsi_tran_conn_t conn, os_sockaddr_storage * addr)
{
  if (conn->m_addr_fn)
  {
    (conn->m_addr_fn) (conn, addr);
    return TRUE;
  }
  return FALSE;
}

void ddsi_tran_free_qos (ddsi_tran_qos_t qos)
{
  os_free (qos);
}

os_handle ddsi_tran_handle (ddsi_tran_base_t base)
{
  return (base->m_handle_fn) (base);
}

ddsi_tran_qos_t ddsi_tran_create_qos (void)
{
  ddsi_tran_qos_t qos;
  qos = (ddsi_tran_qos_t) os_malloc (sizeof (*qos));
  memset (qos, 0, sizeof (*qos));
  return qos;
}

ddsi_tran_conn_t ddsi_factory_create_conn 
(
  ddsi_tran_factory_t factory,
  os_uint32 port,
  ddsi_tran_qos_t qos
)
{
  return factory->m_create_conn_fn (port, qos);
}

int ddsi_tran_locator (ddsi_tran_base_t base, nn_locator_t * loc)
{
  return (base->m_locator_fn) (base, loc);
}

int ddsi_listener_listen (ddsi_tran_listener_t listener)
{
  return (listener->m_listen_fn) (listener);
}

void ddsi_listener_accept (ddsi_tran_listener_t listener, ddsi_tran_conn_t * conn)
{
  (listener->m_accept_fn) (listener, conn);
}

void ddsi_tran_free (ddsi_tran_base_t base)
{
  if (base && (base->m_trantype == DDSI_TRAN_CONN))
  {
    ddsi_conn_free ((ddsi_tran_conn_t) base);
  }
  else
  {
    ddsi_listener_free ((ddsi_tran_listener_t) base);
  }
}

void ddsi_listener_free (ddsi_tran_listener_t listener)
{
  if (listener)
  {
    (listener->m_factory->m_release_listener_fn) (listener);
  }
}

/* SHA1 not available (unoffical build.) */
