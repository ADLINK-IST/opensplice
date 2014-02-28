#ifndef _DDSI_TRAN_H_
#define _DDSI_TRAN_H_

/* DDSI Transport module */

#include "q_globals.h"
#include "q_protocol.h"

/* Types supporting handles */

#define DDSI_TRAN_CONN 1
#define DDSI_TRAN_LISTENER 2

/* Core types */

typedef struct ddsi_tran_base * ddsi_tran_base_t;
typedef struct ddsi_tran_conn * ddsi_tran_conn_t;
typedef struct ddsi_tran_listener * ddsi_tran_listener_t;
typedef struct ddsi_tran_factory * ddsi_tran_factory_t;
typedef struct ddsi_tran_qos * ddsi_tran_qos_t;

/* Function pointer types */

typedef os_ssize_t (*ddsi_tran_read_fn_t) (ddsi_tran_conn_t , unsigned char *, os_size_t);
typedef os_ssize_t (*ddsi_tran_write_fn_t) (ddsi_tran_conn_t, struct msghdr *, os_size_t);
typedef int (*ddsi_tran_locator_fn_t) (ddsi_tran_base_t, nn_locator_t *);
typedef c_bool (*ddsi_tran_supports_fn_t) (os_uint32);
typedef os_handle (*ddsi_tran_handle_fn_t) (ddsi_tran_base_t);
typedef int (*ddsi_tran_listen_fn_t) (ddsi_tran_listener_t);
typedef void (*ddsi_tran_free_fn_t) (void);
typedef void (*ddsi_tran_get_addr_fn_t) (ddsi_tran_conn_t, os_sockaddr_storage *);
typedef void (*ddsi_tran_accept_fn_t) ( ddsi_tran_listener_t, ddsi_tran_conn_t *);
typedef ddsi_tran_conn_t (*ddsi_tran_create_conn_fn_t) (os_uint32 , ddsi_tran_qos_t);
typedef ddsi_tran_listener_t (*ddsi_tran_create_listener_fn_t) (ddsi_tran_qos_t);
typedef void (*ddsi_tran_release_conn_fn_t) (ddsi_tran_conn_t);
typedef void (*ddsi_tran_release_listener_fn_t) (ddsi_tran_listener_t);

/* Data types */

struct ddsi_tran_base
{
  /* Data */

  os_uint32 m_port;
  os_uint32 m_trantype;
  c_bool m_multicast;

  /* Functions */

  ddsi_tran_locator_fn_t m_locator_fn;
  ddsi_tran_handle_fn_t m_handle_fn;
};

struct ddsi_tran_conn
{
  struct ddsi_tran_base m_base;

  /* Functions */

  ddsi_tran_read_fn_t m_read_fn;
  ddsi_tran_write_fn_t m_write_fn;
  ddsi_tran_get_addr_fn_t m_addr_fn;

  /* Data */

  c_bool m_server;
  c_bool m_connless;
  c_bool m_stream;
  os_uint32 m_count;

  /* Relationships */

  ddsi_tran_factory_t m_factory;
  ddsi_tran_listener_t m_listener;
  ddsi_tran_conn_t m_conn;
};

struct ddsi_tran_listener
{
  struct ddsi_tran_base m_base;

  /* Functions */

  ddsi_tran_listen_fn_t m_listen_fn;
  ddsi_tran_accept_fn_t m_accept_fn;

  /* Relationships */

  ddsi_tran_conn_t m_connections;
  ddsi_tran_factory_t m_factory;
  ddsi_tran_listener_t m_listener;
};

struct ddsi_tran_factory
{
  /* Functions */

  ddsi_tran_create_conn_fn_t m_create_conn_fn;
  ddsi_tran_create_listener_fn_t m_create_listener_fn;
  ddsi_tran_release_conn_fn_t m_release_conn_fn;
  ddsi_tran_release_listener_fn_t m_release_listener_fn;
  ddsi_tran_supports_fn_t m_supports_fn;
  ddsi_tran_free_fn_t m_free_fn;

  /* Data */

  os_uint32 m_kind;
  const char * m_typename;
  c_bool m_connless;
  c_bool m_stream;

  /* Relationships */

  ddsi_tran_factory_t m_factory;
};

struct ddsi_tran_qos
{
  /* QoS Data */

  c_bool m_multicast;
  int m_diffserv;
};

/* Functions and pseudo functions (macro wrappers) */

#define ddsi_tran_type(b) (((ddsi_tran_base_t) (b))->m_trantype)
#define ddsi_tran_port(b) (((ddsi_tran_base_t) (b))->m_port)
int ddsi_tran_locator (ddsi_tran_base_t base, nn_locator_t * loc);
void ddsi_tran_free (ddsi_tran_base_t base);
void ddsi_tran_free_qos (ddsi_tran_qos_t qos);
ddsi_tran_qos_t ddsi_tran_create_qos (void);
os_handle ddsi_tran_handle (ddsi_tran_base_t base);

#define ddsi_factory_create_listener(f,q) (((f)->m_create_listener_fn) (q))
#define ddsi_factory_supports(f,k) (((f)->m_supports_fn) (k))

ddsi_tran_conn_t ddsi_factory_create_conn
(
  ddsi_tran_factory_t factory,
  os_uint32 port,
  ddsi_tran_qos_t qos
);
void ddsi_factory_add (ddsi_tran_factory_t factory);
void ddsi_factory_free (ddsi_tran_factory_t factory);
ddsi_tran_factory_t ddsi_factory_find (const char * type);

#define ddsi_conn_handle(c) (ddsi_tran_handle (&(c)->m_base))
#define ddsi_conn_locator(c,l) (ddsi_tran_locator (&(c)->m_base,(l)))
os_ssize_t ddsi_conn_write (ddsi_tran_conn_t conn, struct msghdr * msg, os_size_t len);
os_ssize_t ddsi_conn_read (ddsi_tran_conn_t conn, unsigned char * buf, os_size_t len);
c_bool ddsi_conn_address (ddsi_tran_conn_t conn, os_sockaddr_storage * addr);
void ddsi_conn_add_ref (ddsi_tran_conn_t conn);
void ddsi_conn_free (ddsi_tran_conn_t conn);

#define ddsi_listener_locator(s,l) (ddsi_tran_locator (&(s)->m_base,(l)))
void ddsi_listener_accept (ddsi_tran_listener_t listener, ddsi_tran_conn_t * conn);
int ddsi_listener_listen (ddsi_tran_listener_t listener);
void ddsi_listener_free (ddsi_tran_listener_t listener);

#endif

/* SHA1 not available (unoffical build.) */
