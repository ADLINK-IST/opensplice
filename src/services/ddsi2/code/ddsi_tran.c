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
#include "os_atomics.h"
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
  if (conn)
  {
    if (! conn->m_closed)
    {
      conn->m_closed = TRUE;
      if (conn->m_factory->m_close_conn_fn)
      {
        (conn->m_factory->m_close_conn_fn) (conn);
      }
    }
    if (pa_dec32_nv (&conn->m_count) == 0)
    {
      (conn->m_factory->m_release_conn_fn) (conn);
    }
  }
}

void ddsi_conn_add_ref (ddsi_tran_conn_t conn)
{
  pa_inc32 (&conn->m_count);
}

extern void ddsi_factory_conn_init (ddsi_tran_factory_t factory, ddsi_tran_conn_t conn)
{
  pa_st32 (&conn->m_count, 1);
  conn->m_connless = factory->m_connless;
  conn->m_stream = factory->m_stream;
  conn->m_factory = factory;
}

os_ssize_t ddsi_conn_read (ddsi_tran_conn_t conn, unsigned char * buf, os_size_t len)
{
  return (conn->m_closed) ? -1 : (conn->m_read_fn) (conn, buf, len);
}

os_ssize_t ddsi_conn_write (ddsi_tran_conn_t conn, const struct msghdr * msg, os_size_t len, os_uint32 flags)
{
  os_ssize_t ret = -1;
  if (! conn->m_closed)
  {
    ret = (conn->m_write_fn) (conn, msg, len, flags);
  }

  /* Check that write function is atomic (all or nothing) */

  assert (ret == -1 || (os_size_t) ret == len);
  return ret;
}

c_bool ddsi_conn_peer_locator (ddsi_tran_conn_t conn, nn_locator_t * loc)
{
  if (conn->m_peer_locator_fn)
  {
    (conn->m_peer_locator_fn) (conn, loc);
    return TRUE;
  }
  return FALSE;
}

void ddsi_tran_free_qos (ddsi_tran_qos_t qos)
{
  os_free (qos);
}

int ddsi_conn_join_mc (ddsi_tran_conn_t conn, const nn_locator_t *srcloc, const nn_locator_t *mcloc)
{
  return conn->m_factory->m_join_mc_fn (conn, srcloc, mcloc);
}

int ddsi_conn_leave_mc (ddsi_tran_conn_t conn, const nn_locator_t *srcloc, const nn_locator_t *mcloc)
{
  return conn->m_factory->m_leave_mc_fn (conn, srcloc, mcloc);
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

ddsi_tran_conn_t ddsi_listener_accept (ddsi_tran_listener_t listener)
{
  return (listener->m_accept_fn) (listener);
}

void ddsi_tran_free (ddsi_tran_base_t base)
{
  if (base)
  {
    if (base->m_trantype == DDSI_TRAN_CONN)
    {
      ddsi_conn_free ((ddsi_tran_conn_t) base);
    }
    else
    {
      ddsi_listener_unblock ((ddsi_tran_listener_t) base);
      ddsi_listener_free ((ddsi_tran_listener_t) base);
    }
  }
}

void ddsi_listener_unblock (ddsi_tran_listener_t listener)
{
  if (listener)
  {
    (listener->m_factory->m_unblock_listener_fn) (listener);
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
