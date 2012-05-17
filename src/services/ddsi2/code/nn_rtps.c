/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

/* All messages are formatted as RTPS_HEADER xxx, where xxx is
   dependent on the type of message; data messages have for xxx:
   INFO_DST INFO_TS DATA, where the INFO_DST is skipped during normal
   sends, but included when resending. Also, there is only one reader
   of the event queue and it is allowed to rewrite the message to get
   the destination fields set properly on resends (yuck!, but it does
   make sense here and now). */

#ifndef _REENTRANT
#define _REENTRANT 1
#endif

#include <ctype.h>
#include <stddef.h>

#include "os_heap.h"
#include "os_mutex.h"
#include "os_cond.h"
#include "os_thread.h"
#include "os_socket.h"
#include "os_if.h"

#include "v_group.h"
#include "v_partition.h"
#include "v_groupSet.h"
#include "v_entity.h"

/* for printing message header info */
#include "v_state.h"
#include "v_topic.h"

#include "nn_md5.h"

#include "nn_avl.h"
#include "nn_osplser.h"
#include "nn_protocol.h"
#include "nn_rtps.h"
#include "nn_misc.h"

#ifndef OS_WIN32_IF_H
#include <netdb.h>
#endif

#include "nn_config.h"
#include "nn_log.h"

#include "nn_mlv.h"
#include "nn_plist.h"
#include "nn_unused.h"
#include "nn_groupset.h"
#include "nn_bswap.h"
#include "nn_lat_estim.h"
#include "nn_bitset.h"
#include "nn_xevent.h"
#include "nn_align.h"
#include "nn_addrset.h"
#include "nn_ddsi_discovery.h"
#include "nn_radmin.h"
#include "nn_error.h"

#include "nn_osplserModule.h"

#include "sysdeps.h"

#define MAX_SEQ_NUMBER 0x7fffffffffffffffll

#define PGUIDPREFIX(gp) (gp).u[0], (gp).u[1], (gp).u[2]
#define PGUID(g) PGUIDPREFIX ((g).prefix), (g).entityid.u

struct proxy_endpoint_common;

#include "nn_rtps_private.h"

static c_base ospl_base;
static v_kernel ospl_kernel;
static c_collectionType ospl_qostype;

struct proxypptree proxypptree;
struct proxyrdtree proxyrdtree;
struct proxywrtree proxywrtree;
struct participant_avltree pptree;

struct xeventq *xevents;

static os_socket discsock_uc, discsock_mc, datasock_uc, datasock_mc;

struct in_addr ownip, mcip;
nn_locator_t loc_meta_mc, loc_meta_uc, loc_default_mc, loc_default_uc;
nn_locator_udpv4_t udpv4_meta_mc, udpv4_meta_uc, udpv4_default_mc, udpv4_default_uc;

struct addrset *as_disc_init;
struct addrset *as_disc;

/* Many sockets mode: */
static int participant_set_changed;
static int nparticipants = 0;
static struct participant **participant_set;

static os_threadId xmit_tid;
static os_threadId recv_tid;
os_mutex lock;
os_cond evcond;
os_cond throttle_cond;
static int keepgoing = 1;
static int startup_mode;

static os_int64 tstart;

static int domainid = 0;
static int participantid = -1;

#if DO_VGLOBALS
static void vglobals (void);
#endif

nn_xqos_t default_xqos_rd;
nn_xqos_t default_xqos_wr;
nn_xqos_t spdp_endpoint_xqos;
nn_xqos_t builtin_endpoint_xqos_rd;
nn_xqos_t builtin_endpoint_xqos_wr;

/* SPDP packets get very special treatment (they're the only packets
   we accept from writers we don't know) and have their very own
   do-nothing defragmentation and reordering thingummies. */
static struct nn_defrag *spdp_defrag;
static struct nn_reorder *spdp_reorder;
static struct nn_dqueue *spdp_dqueue;

/* Built-in stuff other than SPDP gets funneled through the builtins
   delivery queue; currently just SEDP and PMD */
static struct nn_dqueue *builtins_dqueue;

/* User data goes through another one (just one, for now) */
static struct nn_dqueue *user_dqueue;

/* Transmit side: pools for the serializer & transmit messages and a
   transmit queue */
serstatepool_t serpool;
struct nn_xmsgpool *xmsgpool;

/* number of up, non-loopback, IPv4 interfaces */
int n_interfaces;
struct nn_interface interfaces[MAX_INTERFACES];

extern v_networkId myNetworkId;

static void write_pmd_message (struct participant *pp);
static int sedp_write_writer (struct writer *wr, const struct writer *info);
static int sedp_write_reader (struct writer *wr, const struct reader *info);
static int remove_acked_messages (struct writer *wr);

static void remove_proxy_writer_from_reader (struct reader *rd, struct proxy_writer *pwr);
static void remove_proxy_reader_from_writer (struct writer *wr, struct proxy_reader *prd);

static struct writer *new_writer_unl (struct participant *pp, nn_entityid_t id, C_STRUCT (v_topic) const * const ospl_topic, const nn_xqos_t *xqos);
static struct reader *new_reader_unl (struct participant *pp, nn_entityid_t id, C_STRUCT (v_topic) const * const ospl_topic, const nn_xqos_t *xqos);

static void handle_SEDP (const struct receiver_state *rst, unsigned statusinfo, const void *vdata, int len);
static void handle_PMD (const struct receiver_state *rst, unsigned statusinfo, const void *vdata, int len);

static int make_heartbeat_ack_required (struct writer *wr);

static int transmit_sample (struct nn_xpack *xp, struct writer *w, os_int64 seq, serdata_t serdata, struct proxy_reader *prd);
static int write_sample (struct nn_xpack *xp, struct writer *wr, serdata_t serdata);

static int do_groupwrite (v_group g, void *vmsg);
static void deliver_regular_unlocked (struct nn_rsample_chain *sc);

static void print_sockerror (const char *msg)
{
  int err = os_sockError ();
  nn_log (LC_ERROR, "SOCKET ERROR %s %d\n", msg, err);
}

#if 0
static double randexp (double inv_lambda)
{
  int i;
  do i = random (); while (i == 0);
  /* i in [1,2**31-1]; therefore i / 2**31 in [2**-31,1-2**-31];
     therefore log(x) approximately in (-21.5,-4.7e-10), i.e. no
     random() value causes anything outside of the normal operating
     range of doubles */
  return -log (i / 2147483648.0) * inv_lambda;
}

static os_int64 randtnext (os_int64 tlast, double minintv, double maxintv, double avgintv)
{
  const double inv_lambda = tavgintv;
  const double intv = randexp (inv_lambda);
  if (intv < minintv) intv = minintv;
  else if (intv > maxintv) intv = maxintv;
  return tlast + (os_int64) (intv * 1e9);
}
#endif

int vendor_is_rti (nn_vendorid_t vendor)
{
  const nn_vendorid_t rti = NN_VENDORID_RTI;
  return vendor.id[0] == rti.id[0] && vendor.id[1] == rti.id[1];
}

int vendor_is_twinoaks (nn_vendorid_t vendor)
{
  const nn_vendorid_t twinoaks = NN_VENDORID_TWINOAKS;
  return vendor.id[0] == twinoaks.id[0] && vendor.id[1] == twinoaks.id[1];
}

int vendor_is_prismtech (nn_vendorid_t vendor)
{
  const nn_vendorid_t prismtech = NN_VENDORID_PRISMTECH;
  return vendor.id[0] == prismtech.id[0] && vendor.id[1] == prismtech.id[1];
}

int is_own_vendor (nn_vendorid_t vendor)
{
  const nn_vendorid_t ownid = MY_VENDOR_ID;
  return vendor.id[0] == ownid.id[0] && vendor.id[1] == ownid.id[1];
}

static nn_entityid_t entityid_from_u (unsigned u)
{
  nn_entityid_t id;
  id.u = u;
  return id;
}

static int compare_guid (const void *va, const void *vb)
{
  return memcmp (va, vb, sizeof (nn_guid_t));
}

static int compare_seq (const void *va, const void *vb)
{
  const os_int64 *a = va;
  const os_int64 *b = vb;
  return (*a == *b) ? 0 : (*a < *b) ? -1 : 1;
}

int is_writer_entityid (nn_entityid_t id)
{
  switch (id.u & NN_ENTITYID_KIND_MASK)
  {
    case NN_ENTITYID_KIND_READER_WITH_KEY:
    case NN_ENTITYID_KIND_READER_NO_KEY:
      return 0;
    case NN_ENTITYID_KIND_WRITER_WITH_KEY:
    case NN_ENTITYID_KIND_WRITER_NO_KEY:
      return 1;
    default:
      return 0;
  }
}

static int is_builtin_entityid (nn_entityid_t id)
{
  return (id.u & NN_ENTITYID_SOURCE_MASK) == NN_ENTITYID_SOURCE_BUILTIN;
}

static os_int64 fromSN (const nn_sequence_number_t sn)
{
  return ((os_int64) sn.high << 32) | sn.low;
}

nn_sequence_number_t nn_toSN (os_int64 n)
{
  nn_sequence_number_t x;
  x.high = (int) (n >> 32);
  x.low = (unsigned) n;
  return x;
}

static nn_sequence_number_t toSN (os_int64 n)
{
  return nn_toSN (n);
}

static double time_to_double (os_int64 t)
{
  return t / 1e9;
}

static void debug_print_rawdata (const char *msg, const void *data, int len)
{
  const unsigned char *c = data;
  int i;
  nn_log (LC_TRACE, "%s<", msg);
  for (i = 0; i < len; i++)
  {
    if (32 < c[i] && c[i] <= 127)
      nn_log (LC_TRACE, " %c", (i > 0 && (i%4) == 0) ? " " : "", c[i]);
    else
      nn_log (LC_TRACE, "%s\\x%02x", (i > 0 && (i%4) == 0) ? " " : "", c[i]);
  }
  nn_log (LC_TRACE, ">");
}

static int make_socket (os_socket *socket, unsigned short port, const struct in_addr *mcip)
{
  struct sockaddr_in socketname;

  *socket = os_sockNew (AF_INET, SOCK_DGRAM);

  if (!*socket)
  {
    print_sockerror ("socket");
    return -2;
  }
#if 0 /* really #if unix */
  if (*socket >= FD_SETSIZE)
  {
    nn_log (LC_FATAL, "ddsi2: fatal: numerical value of file descriptor too large for select\n");
    abort ();
  }
#endif
  socketname.sin_family = AF_INET;
  socketname.sin_port = htons (port);
  socketname.sin_addr.s_addr = htonl (INADDR_ANY);
  if (config.dontRoute)
  {
    int one = 1;
    if (os_sockSetsockopt (*socket, SOL_SOCKET, SO_DONTROUTE, (char *) &one, sizeof (one)) != os_resultSuccess)
    {
      print_sockerror ("SO_DONTROUTE");
      return -2;
    }
  }
  if (mcip)
  {
    int one = 1;
    if (os_sockSetsockopt (*socket, SOL_SOCKET, SO_REUSEADDR, (char *) &one, sizeof (one)) != os_resultSuccess)
    {
      print_sockerror ("SO_REUSEADDR");
      return -2;
    }
#ifdef SO_REUSEPORT
    if (os_sockSetsockopt (*socket, SOL_SOCKET, SO_REUSEPORT, (char *) &one, sizeof (one)) != os_resultSuccess)
    {
      print_sockerror ("SO_REUSEPORT");
      return -2;
    }
#endif
  }
  if (os_sockBind (*socket, (struct sockaddr *) &socketname, sizeof (socketname)) != os_resultSuccess)
  {
    if (os_sockError () != os_sockEADDRINUSE)
      print_sockerror ("bind");
    return -1;
  }
  if (mcip)
  {
    unsigned char ttl = 32;
    struct ip_mreq mreq;
    unsigned char loop;
    int i;
    /* ownip is now only used for preferred multicast interface on send ... */
    if (os_sockSetsockopt (*socket, IPPROTO_IP, IP_MULTICAST_IF, (char *) &ownip, sizeof (ownip)) != os_resultSuccess)
    {
      print_sockerror ("IP_MULTICAST_IF");
      return -2;
    }
    /* ... but we add membership on all multi-cast capable network interfaces */
    mreq.imr_multiaddr = *mcip;
    for (i = 0; i < n_interfaces; i++)
    {
      if (interfaces[i].mc_capable)
      {
        mreq.imr_interface = interfaces[i].addr;
        if (os_sockSetsockopt (*socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *) &mreq, sizeof (mreq)) != os_resultSuccess)
        {
          print_sockerror ("IP_ADD_MEMBERSHIP");
          return -2;
        }
      }
    }
    if (os_sockSetsockopt (*socket, IPPROTO_IP, IP_MULTICAST_TTL, (char *) &ttl, sizeof (ttl)) != os_resultSuccess)
    {
      print_sockerror ("IP_MULICAST_TTL");
      return -2;
    }
    loop = config.enableMulticastLoopback;
    if (os_sockSetsockopt (*socket, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof (loop)) != os_resultSuccess)
    {
      print_sockerror ("IP_MULTICAST_LOOP");
    }
  }
  return 0;
}

/* ///
/// */

static int is_wildcard_partition (const char *str)
{
  return strchr (str, '*') || strchr (str, '?');
}

static int partition_patmatch_p (const char *pat, const char *name)
{
  /* pat may be a wildcard expression, name must not be */
  if (!is_wildcard_partition (pat))
    /* no wildcard in pat => must equal name */
    return (strcmp (pat, name) == 0);
  else if (is_wildcard_partition (name))
    /* (we know: wildcard in pat) => wildcard in name => no match */
    return 0;
  else
  {
    /* quick hack: pattern matcher blatantly stolen from the kernel */
    const char *nameRef = NULL;
    const char *ptnRef = NULL;
    while (*name != 0 && *pat != 0)
    {
      if (*pat == '*')
      {
        pat++;
        while (*name != 0 && *name != *pat)
          name++;
        if (*name != 0)
        {
          nameRef = name+1;
          ptnRef = pat-1;
        }
      }
      else if (*pat == '?')
      {
        pat++;
        name++;
      }
      else if (*pat++ != *name++)
      {
        if (nameRef == NULL)
          return 0;
        name = nameRef;
        pat = ptnRef;
        nameRef = NULL;
      }
    }
    if (*name != 0)
      return 0;
    else
    {
      while (*pat == '*')
        pat++;
      return (*pat == 0);
    }
  }
}

static int partitions_match_default (const nn_xqos_t *x)
{
  int i;
  if (!(x->present & QP_PARTITION) || x->partition.n == 0)
    return 1;
  for (i = 0; i < x->partition.n; i++)
    if (partition_patmatch_p (x->partition.strs[i], ""))
      return 1;
  return 0;
}

static int partitions_match_p (const nn_xqos_t *a, const nn_xqos_t *b)
{
  if (!(a->present & QP_PARTITION) || a->partition.n == 0)
    return partitions_match_default (b);
  else if (!(b->present & QP_PARTITION) || b->partition.n == 0)
    return partitions_match_default (a);
  else
  {
    int i, j;
    for (i = 0; i < a->partition.n; i++)
      for (j = 0; j < b->partition.n; j++)
      {
        if (partition_patmatch_p (a->partition.strs[i], b->partition.strs[j]) ||
            partition_patmatch_p (b->partition.strs[j], a->partition.strs[i]))
          return 1;
      }
    return 0;
  }
}

static int partition_match_based_on_wildcard_in_left_operand (const nn_xqos_t *a, const nn_xqos_t *b, const char **realname)
{
  assert (partitions_match_p (a, b));
  if (!(a->present & QP_PARTITION) || a->partition.n == 0)
  {
    return 0;
  }
  else if (!(b->present & QP_PARTITION) || b->partition.n == 0)
  {
    /* Either A explicitly includes the default partition, or it is a
       wildcard that matches it */
    int i;
    for (i = 0; i < a->partition.n; i++)
      if (strcmp (a->partition.strs[i], "") == 0)
        return 0;
    *realname = "";
    return 1;
  }
  else
  {
    int i, j, maybe_yes = 0;
    for (i = 0; i < a->partition.n; i++)
      for (j = 0; j < b->partition.n; j++)
      {
        if (partition_patmatch_p (a->partition.strs[i], b->partition.strs[j]))
        {
          if (!is_wildcard_partition (a->partition.strs[i]))
            return 0;
          else
          {
            *realname = b->partition.strs[j];
            maybe_yes = 1;
          }
        }
      }
    return maybe_yes;
  }
}

static int ddsi_duration_is_lt (nn_duration_t a0, nn_duration_t b0)
{
  /* inf counts as <= inf */
  const os_int64 a = nn_from_ddsi_duration (a0);
  const os_int64 b = nn_from_ddsi_duration (b0);
  if (a == T_NEVER)
    return 0;
  else if (b == T_NEVER)
    return 1;
  else
    return a < b;
}

static int qos_match_p (const nn_xqos_t *rd, const nn_xqos_t *wr)
{
#ifndef NDEBUG
  unsigned musthave = (QP_RXO_MASK | QP_PARTITION | QP_TOPIC_NAME | QP_TYPE_NAME);
  assert ((rd->present & musthave) == musthave);
  assert ((wr->present & musthave) == musthave);
#endif
  if (rd->reliability.kind > wr->reliability.kind)
    return 0;
  if (rd->durability.kind > wr->durability.kind)
    return 0;
  if (rd->presentation.access_scope > wr->presentation.access_scope)
    return 0;
  if (rd->presentation.coherent_access > wr->presentation.coherent_access)
    return 0;
  if (rd->presentation.ordered_access > wr->presentation.ordered_access)
    return 0;
  if (ddsi_duration_is_lt (rd->deadline.deadline, wr->deadline.deadline))
    return 0;
  if (ddsi_duration_is_lt (rd->latency_budget.duration, wr->latency_budget.duration))
    return 0;
  if (rd->ownership.kind != wr->ownership.kind)
    return 0;
  if (rd->liveliness.kind > wr->liveliness.kind)
    return 0;
  if (ddsi_duration_is_lt (rd->liveliness.lease_duration, wr->liveliness.lease_duration))
    return 0;
  if (rd->destination_order.kind > wr->destination_order.kind)
    return 0;
  if (strcmp (rd->topic_name, wr->topic_name) != 0)
    return 0;
  if (strcmp (rd->type_name, wr->type_name) != 0)
    return 0;
  if (!partitions_match_p (rd, wr))
    return 0;
  return 1;
}

static int dds_attach_proxy_writer_walkpprd (void *vnode, void *arg)
{
  struct reader *rd = vnode;
  struct proxy_writer *pwr = arg;
  assert (is_writer_entityid (pwr->c.guid.entityid));
  if (is_builtin_entityid (pwr->c.guid.entityid) || is_builtin_entityid (rd->guid.entityid))
    return AVLWALK_CONTINUE;
  if (qos_match_p (rd->xqos, pwr->c.xqos))
    add_proxy_writer_to_reader (rd, pwr);
  return AVLWALK_CONTINUE;
}

static int dds_attach_proxy_reader_walkppwr (void *vnode, void *arg)
{
  struct writer *wr = vnode;
  struct proxy_reader *prd = arg;
  assert (!is_writer_entityid (prd->c.guid.entityid));
  if (is_builtin_entityid (prd->c.guid.entityid) || is_builtin_entityid (wr->guid.entityid))
    return AVLWALK_CONTINUE;
  if (qos_match_p (prd->c.xqos, wr->xqos))
    add_proxy_reader_to_writer (wr, prd);
  return AVLWALK_CONTINUE;
}

static int dds_attach_proxy_reader_walkpp (void *vnode, void *arg)
{
  struct participant *pp = vnode;
  struct proxy_reader *prd = arg;
  assert (!is_writer_entityid (prd->c.guid.entityid));
  avl_walk (&pp->writers, dds_attach_proxy_reader_walkppwr, prd);
  return AVLWALK_CONTINUE;
}

static int dds_attach_proxy_writer_walkpp (void *vnode, void *arg)
{
  struct participant *pp = vnode;
  struct proxy_writer *pwr = arg;
  assert (is_writer_entityid (pwr->c.guid.entityid));
  avl_walk (&pp->readers, dds_attach_proxy_writer_walkpprd, pwr);
  return AVLWALK_CONTINUE;
}

static void dds_attach_proxy_reader (struct proxy_reader *prd)
{
  avl_walk (&pptree, dds_attach_proxy_reader_walkpp, prd);
}

static void dds_attach_proxy_writer (struct proxy_writer *pwr)
{
  avl_walk (&pptree, dds_attach_proxy_writer_walkpp, pwr);
}

static int dds_attach_reader_to_proxies_helper (void *vnode, void *varg)
{
  struct proxy_writer *pwr = vnode;
  struct reader *rd = varg;
  assert (is_writer_entityid (pwr->c.guid.entityid));
  dds_attach_proxy_writer_walkpprd (rd, pwr);
  return AVLWALK_CONTINUE;
}

static void dds_attach_reader_to_proxies (struct reader *rd)
{
  avl_walk (&proxywrtree, dds_attach_reader_to_proxies_helper, rd);
}

static int dds_attach_writer_to_proxies_helper (void *vnode, void *varg)
{
  struct proxy_reader *prd = vnode;
  struct writer *wr = varg;
  assert (!is_writer_entityid (prd->c.guid.entityid));
  dds_attach_proxy_reader_walkppwr (wr, prd);
  return AVLWALK_CONTINUE;
}

static void dds_attach_writer_to_proxies (struct writer *wr)
{
  avl_walk (&proxyrdtree, dds_attach_writer_to_proxies_helper, wr);
}

static const char *durability_to_string (nn_durability_kind_t k)
{
  switch (k)
  {
    case NN_VOLATILE_DURABILITY_QOS: return "volatile";
    case NN_TRANSIENT_LOCAL_DURABILITY_QOS: return "transient-local";
    case NN_TRANSIENT_DURABILITY_QOS: return "transient";
    case NN_PERSISTENT_DURABILITY_QOS: return "persistent";
  }
  abort (); return 0;
}

static void maybe_set_reader_in_sync (struct rhc_writers_node *wn)
{
  assert (!wn->in_sync);
  if (nn_reorder_next_seq (wn->u.not_in_sync.reorder) > wn->u.not_in_sync.end_of_tl_seq)
    wn->in_sync = 1;
}

/** PROXIES ******************************************************************/

static int init_proxy_endpoint_common (struct proxy_endpoint_common *ep, nn_guid_t guid, struct proxy_participant *pp, struct addrset *as, nn_xqos_t *xqos)
{
  if (is_builtin_entityid (guid.entityid))
    assert ((xqos->present & (QP_TOPIC_NAME | QP_TYPE_NAME)) == 0);
  else
    assert ((xqos->present & (QP_TOPIC_NAME | QP_TYPE_NAME)) == (QP_TOPIC_NAME | QP_TYPE_NAME));

  assert (xqos->aliased == 0);
  ep->guid = guid;
  ep->xqos = xqos;

  assert (ep->xqos->present & QP_LIVELINESS);
  if (ep->xqos->liveliness.kind != NN_AUTOMATIC_LIVELINESS_QOS)
    nn_log (LC_TRACE, " FIXME: only AUTOMATIC liveliness supported");
  ep->tlease_dur = nn_from_ddsi_duration (ep->xqos->liveliness.lease_duration);
  if (ep->tlease_dur == 0)
  {
    nn_log (LC_TRACE, " FIXME: treating lease_duration=0 as inf");
    ep->tlease_dur = T_NEVER;
  }
  ep->as = ref_addrset (as);
  ep->pp = ref_proxy_participant (pp);
  ep->tlease_end = add_duration_to_time (now (), ep->tlease_dur);
  return 0;
}

struct proxy_reader *new_proxy_reader (nn_guid_t guid, struct proxy_participant *pp, struct addrset *as, nn_xqos_t *xqos)
{
  struct proxy_reader *prd;
  avlparent_t parent;
  assert (!is_writer_entityid (guid.entityid));
  if (avl_lookup (&proxyrdtree, &guid, &parent) != NULL)
    return NULL;
  if ((prd = os_malloc (sizeof (*prd))) == NULL)
    return NULL;
  avl_init_node (&prd->c.avlnode, parent);
  avl_init (&prd->writers, offsetof (struct whc_readers_node, proxyavlnode), offsetof (struct whc_readers_node, writer_guid), compare_guid, 0, 0);
  if (init_proxy_endpoint_common (&prd->c, guid, pp, as, xqos) != 0)
  {
    os_free (prd);
    return NULL;
  }
  avl_insert (&proxyrdtree, prd);
  return prd;
}

struct proxy_writer *new_proxy_writer (nn_guid_t guid, struct proxy_participant *pp, struct addrset *as, nn_xqos_t *xqos)
{
  struct proxy_writer *pwr;
  avlparent_t parent;
  int isreliable;

  assert (is_writer_entityid (guid.entityid));
  if (avl_lookup (&proxywrtree, &guid, &parent) != NULL)
    return NULL;
  if ((pwr = os_malloc (sizeof (*pwr))) == NULL)
    return NULL;
  avl_init_node (&pwr->c.avlnode, parent);
  pwr->n_reliable_readers = 0;
  avl_init (&pwr->readers, offsetof (struct rhc_writers_node, proxyavlnode), offsetof (struct rhc_writers_node, reader_guid), compare_guid, 0, 0);
  pwr->groups = nn_groupset_new ();
  if (init_proxy_endpoint_common (&pwr->c, guid, pp, as, xqos) != 0)
  {
    os_free (pwr);
    return NULL;
  }
  pwr->v_message_qos = new_v_message_qos (pwr->c.xqos, ospl_qostype);
  pwr->last_seq = 0;
  pwr->nackfragcount = 0;
  if (!is_builtin_entityid (guid.entityid) &&
      nn_from_ddsi_duration (pwr->c.xqos->latency_budget.duration) == 0 &&
      pwr->c.xqos->transport_priority.value >= config.synchronous_delivery_priority_threshold)
    pwr->deliver_synchronously = 1;
  else
    pwr->deliver_synchronously = 0;
  avl_insert (&proxywrtree, pwr);

  isreliable = (pwr->c.xqos->reliability.kind != NN_BEST_EFFORT_RELIABILITY_QOS);
  pwr->defrag = nn_defrag_new (
          isreliable ? NN_DEFRAG_DROP_LATEST : NN_DEFRAG_DROP_OLDEST,
          isreliable ? config.defrag_reliable_maxsamples : config.defrag_unreliable_maxsamples);
  pwr->reorder = nn_reorder_new (
          isreliable ? NN_REORDER_MODE_NORMAL : NN_REORDER_MODE_MONOTONICALLY_INCREASING,
          config.primary_reorder_maxsamples);
  return pwr;
}

static void augment_proxy_endpoint_common (struct proxy_endpoint_common *epc)
{
  os_int64 x = epc->tlease_end;
  nn_guid_t xid = epc->guid;
  if (epc->avlnode.left)
  {
    if (epc->avlnode.left->min_tlease_end < x)
    {
      x = epc->avlnode.left->min_tlease_end;
      xid = epc->avlnode.left->guid_min_tlease_end;
    }
  }
  if (epc->avlnode.right)
  {
    if (epc->avlnode.right->min_tlease_end < x)
    {
      x = epc->avlnode.right->min_tlease_end;
      xid = epc->avlnode.right->guid_min_tlease_end;
    }
  }
  epc->min_tlease_end = x;
  epc->guid_min_tlease_end = xid;
}

static void augment_proxy_reader (void *vnode)
{
  struct proxy_reader *prd = vnode;
  augment_proxy_endpoint_common (&prd->c);
}

static void augment_proxy_writer (void *vnode)
{
  struct proxy_writer *pwr = vnode;
  augment_proxy_endpoint_common (&pwr->c);
}

static void remove_xevents_for_proxy_reader (UNUSED_ARG (struct proxy_reader *prd))
{
#if 0
  /* FIXME: walking all events to get rid of messages scheduled for
     transmission to this endpoint is a bit ugly, the more because it
     is _highly_ unlikely we'll find one ... */
  struct xevent *ev, *nev;
  ev = avl_findmin (&xevents->xevents);
  while (ev)
  {
    nev = avl_findsucc (&xevents->xevents, ev);
    switch (ev->kind)
    {
      case XEVK_DATA_RESEND:
        if (ev->u.data_resend.prd == prd)
          delete_xevent (ev);
        break;
      case XEVK_GAP:
        if (ev->u.gap.prd == prd)
          delete_xevent (ev);
        break;
      default:
        break;
    }
    ev = nev;
  }
#endif
}

static void fini_proxy_endpoint_common (struct proxy_endpoint_common *ep)
{
  nn_xqos_fini (ep->xqos);
  os_free (ep->xqos);
  unref_addrset (ep->as);
  unref_proxy_participant (ep->pp);
}

static void free_proxy_writer (void *vpwr)
{
  struct proxy_writer *pwr = vpwr;
  int tsec, tusec;
  time_to_sec_usec (&tsec, &tusec, now ());
  nn_log (LC_DISCOVERY, "%d.%06d free_proxy_writer(%x:%x:%x:%x)\n", tsec, tusec, PGUID (pwr->c.guid));
  while (!avl_empty (&pwr->readers))
    remove_proxy_writer_from_reader (pwr->readers.root->reader, pwr);
  fini_proxy_endpoint_common (&pwr->c);
  nn_groupset_free (pwr->groups);
  c_free (pwr->v_message_qos);

  nn_defrag_free (pwr->defrag);
  nn_reorder_free (pwr->reorder);

  os_free (pwr);
}

static void free_proxy_reader (void *vprd)
{
  struct proxy_reader *prd = vprd;
  int tsec, tusec;
  time_to_sec_usec (&tsec, &tusec, now ());
  nn_log (LC_DISCOVERY, "%d.%06d free_proxy_reader(%x:%x:%x:%x)\n", tsec, tusec, PGUID (prd->c.guid));
  remove_xevents_for_proxy_reader (prd);
  while (!avl_empty (&prd->writers))
    remove_proxy_reader_from_writer (prd->writers.root->writer, prd);
  fini_proxy_endpoint_common (&prd->c);
  os_free (prd);
}

static void cleanup_dead_proxies (void)
{
  os_int64 t = now ();
  while (!avl_empty (&proxyrdtree) && proxyrdtree.root->c.min_tlease_end < t)
  {
    struct proxy_reader *prd =
      avl_lookup (&proxyrdtree, &proxyrdtree.root->c.guid_min_tlease_end, NULL);
    assert (prd != NULL);
    nn_log (LC_DISCOVERY, "cleanup_dead_proxies: reader %x:%x:%x:%x\n", PGUID (prd->c.guid));
    avl_delete (&proxyrdtree, prd);
  }
  while (!avl_empty (&proxywrtree) && proxywrtree.root->c.min_tlease_end < t)
  {
    struct proxy_writer *pwr =
      avl_lookup (&proxywrtree, &proxywrtree.root->c.guid_min_tlease_end, NULL);
    assert (pwr != NULL);
    nn_log (LC_DISCOVERY, "cleanup_dead_proxies: writer %x:%x:%x:%x\n", PGUID (pwr->c.guid));
    avl_delete (&proxywrtree, pwr);
  }
  while (!avl_empty (&proxypptree) && proxypptree.root->min_tlease_end < t)
  {
    struct proxy_participant *pp =
      avl_lookup (&proxypptree, &proxypptree.root->guid_min_tlease_end, NULL);
    assert (pp != NULL);
    nn_log (LC_DISCOVERY, "cleanup_dead_proxies: participant %x:%x:%x:%x\n", pp->guid.prefix.u[0], pp->guid.prefix.u[1], pp->guid.prefix.u[2], pp->guid.entityid.u);
    avl_delete (&proxypptree, pp);
  }
}

/** WRITER *******************************************************************/

static void free_writer (void *vnode)
{
  struct writer *wr = vnode;
  wr->dying = 1;
  if (!is_builtin_entityid (wr->guid.entityid))
    sedp_write_writer (wr->participant->sedp_writer_writer, wr);
  if (wr->heartbeat_xevent)
    delete_xevent (wr->heartbeat_xevent);
  avl_free (&wr->readers);
  avl_free (&wr->whc_tlidx); /* tlidx must go before seq, because of xref */
  avl_free (&wr->whc_seq);
  if (wr->topic)
    freetopic (wr->topic);
  unref_addrset (wr->as); /* must remain until readers gone (rebuilding of addrset) */
  nn_xqos_fini (wr->xqos);
  os_free (wr->xqos);
  os_free (wr);
}

static void free_reader (void *vnode)
{
  struct reader *rd = vnode;
  rd->dying = 1;
  if (!is_builtin_entityid (rd->guid.entityid))
    sedp_write_reader (rd->participant->sedp_reader_writer, rd);
  if (rd->topic)
    freetopic (rd->topic);
  avl_free (&rd->writers);
  nn_groupset_free (rd->matching_groups);
  nn_xqos_fini (rd->xqos);
  os_free (rd->xqos);
  os_free (rd);
}

static int attach_new_pp_to_proxypp_helper (void *vnode, void *varg)
{
  struct proxy_participant *proxypp = vnode;
  struct participant *pp = varg;
  struct proxy_reader *prd;
  struct proxy_writer *pwr;
  nn_guid_t guid1 = proxypp->guid;
  if (pp->spdp_pp_reader)
  {
    guid1.entityid.u = NN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER;
    if ((pwr = avl_lookup (&proxywrtree, &guid1, NULL)) != NULL)
      add_proxy_writer_to_reader (pp->spdp_pp_reader, pwr);
  }
  guid1.entityid.u = NN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_READER;
  if ((prd = avl_lookup (&proxyrdtree, &guid1, NULL)) != NULL)
    add_proxy_reader_to_writer (pp->spdp_pp_writer, prd);
  if (pp->sedp_writer_reader)
  {
    guid1.entityid.u = NN_ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER;
    if ((pwr = avl_lookup (&proxywrtree, &guid1, NULL)) != NULL)
      add_proxy_writer_to_reader (pp->sedp_writer_reader, pwr);
  }
  guid1.entityid.u = NN_ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER;
  if ((prd = avl_lookup (&proxyrdtree, &guid1, NULL)) != NULL)
    add_proxy_reader_to_writer (pp->sedp_writer_writer, prd);
  if (pp->sedp_reader_reader)
  {
    guid1.entityid.u = NN_ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER;
    if ((pwr = avl_lookup (&proxywrtree, &guid1, NULL)) != NULL)
      add_proxy_writer_to_reader (pp->sedp_reader_reader, pwr);
  }
  guid1.entityid.u = NN_ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_READER;
  if ((prd = avl_lookup (&proxyrdtree, &guid1, NULL)) != NULL)
    add_proxy_reader_to_writer (pp->sedp_reader_writer, prd);
  if (pp->participant_message_reader)
  {
    guid1.entityid.u = NN_ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_WRITER;
    if ((pwr = avl_lookup (&proxywrtree, &guid1, NULL)) != NULL)
      add_proxy_writer_to_reader (pp->participant_message_reader, pwr);
  }
  guid1.entityid.u = NN_ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_READER;
  if ((prd = avl_lookup (&proxyrdtree, &guid1, NULL)) != NULL)
    add_proxy_reader_to_writer (pp->participant_message_writer, prd);
  return AVLWALK_CONTINUE;
}

static void attach_new_pp_to_proxypp (struct participant *pp)
{
  avl_walk (&proxypptree, attach_new_pp_to_proxypp_helper, pp);
}

static void augment_writer (void *vnode)
{
  struct writer *wr = vnode;
  os_int64 mld = wr->lease_duration;
  if (wr->avlnode.left && wr->avlnode.left->min_lease_duration < mld)
    mld = wr->avlnode.left->min_lease_duration;
  if (wr->avlnode.right && wr->avlnode.right->min_lease_duration < mld)
    mld = wr->avlnode.right->min_lease_duration;
  wr->min_lease_duration = mld;
}

static void augment_reader (void *vnode)
{
  struct reader *rd = vnode;
  os_int64 mld = rd->lease_duration;
  if (rd->avlnode.left && rd->avlnode.left->min_lease_duration < mld)
    mld = rd->avlnode.left->min_lease_duration;
  if (rd->avlnode.right && rd->avlnode.right->min_lease_duration < mld)
    mld = rd->avlnode.right->min_lease_duration;
  rd->min_lease_duration = mld;
}

static struct participant *new_participant_unl (nn_guid_prefix_t idprefix, unsigned flags)
{
  struct participant *pp;
  avlparent_t parent;
  nn_guid_t guid;
  int tsec, tusec;
  time_to_sec_usec (&tsec, &tusec, now ());
  assert ((flags & ~RTPS_PF_NO_BUILTIN_READERS) == 0);
  assert (config.max_participants == 0 || nparticipants < config.max_participants);
  guid.prefix = idprefix;
  guid.entityid.u = NN_ENTITYID_PARTICIPANT;
  nn_log (LC_DISCOVERY, "%d.%06d new_participant(%x:%x:%x:%x, %x)\n", tsec, tusec, PGUID (guid), flags);
  if (avl_lookup (&pptree, &guid, &parent) != NULL)
    abort ();
  pp = os_malloc (sizeof (*pp));
  avl_init_node (&pp->avlnode, parent);
  pp->guid = guid;
  avl_init (&pp->writers, offsetof (struct writer, avlnode), offsetof (struct writer, guid), compare_guid, augment_writer, free_writer);
  avl_init (&pp->readers, offsetof (struct reader, avlnode), offsetof (struct reader, guid), compare_guid, augment_reader, free_reader);

  pp->dying = 0;
  pp->lease_duration = 11 * T_SECOND; /* FIXME: fixed duration is a hack */

  if (config.many_sockets_mode)
  {
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof (addr);
    if (make_socket (&pp->sock, 0, NULL) < 0)
      exit (1);
    if (getsockname (pp->sock, (struct sockaddr *) &addr, &addrlen) < 0)
      print_sockerror ("getsockname");
    pp->sockloc = loc_default_uc;
    pp->sockloc.port = ntohs (addr.sin_port);
    pp->participant_set_index = nparticipants;
    participant_set[pp->participant_set_index] = pp;
    participant_set_changed = 1;
  }

  nparticipants++;
  avl_insert (&pptree, pp);

  pp->pmd_update_xevent = qxev_pmd_update (xevents, (pp->lease_duration == T_NEVER) ? T_NEVER : 0, pp);

  pp->bes = 0;

  /* SPDP writer: reference as_disc instead of having a private
     address set */
  pp->spdp_pp_writer = new_writer_unl (pp, entityid_from_u (NN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER), NULL, &spdp_endpoint_xqos);
  unref_addrset (pp->spdp_pp_writer->as);
  pp->spdp_pp_writer->as = ref_addrset (as_disc);
  pp->bes |= NN_DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER;

  pp->sedp_reader_writer = new_writer_unl (pp, entityid_from_u (NN_ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER), NULL, &builtin_endpoint_xqos_wr);
  pp->bes |= NN_DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER;
  pp->sedp_writer_writer = new_writer_unl (pp, entityid_from_u (NN_ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER), NULL, &builtin_endpoint_xqos_wr);
  pp->bes |= NN_DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER;

  pp->participant_message_writer = new_writer_unl (pp, entityid_from_u (NN_ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_WRITER), NULL, &builtin_endpoint_xqos_wr);
  pp->bes |= NN_BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_WRITER;

  if (flags & RTPS_PF_NO_BUILTIN_READERS)
  {
    pp->spdp_pp_reader = NULL;
    pp->sedp_reader_reader = NULL;
    pp->sedp_writer_reader = NULL;
    pp->participant_message_reader = NULL;
  }
  else
  {
    pp->spdp_pp_reader = new_reader_unl (pp, entityid_from_u (NN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_READER), NULL, &spdp_endpoint_xqos);
    pp->bes |= NN_DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR;
    pp->sedp_reader_reader = new_reader_unl (pp, entityid_from_u (NN_ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_READER), NULL, &builtin_endpoint_xqos_rd);
    pp->bes |=NN_DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR;
    pp->sedp_writer_reader = new_reader_unl (pp, entityid_from_u (NN_ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER), NULL, &builtin_endpoint_xqos_rd);
    pp->bes |= NN_DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR;

    pp->participant_message_reader = new_reader_unl (pp, entityid_from_u (NN_ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_READER), NULL, &builtin_endpoint_xqos_rd);
    pp->bes |= NN_BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_READER;
  }

  attach_new_pp_to_proxypp (pp);

  {
    serdata_t d = construct_spdp_sample_alive (pp);
    write_sample (NULL, pp->spdp_pp_writer, d);
  }

  pp->spdp_xevent = qxev_spdp (xevents, 0, pp, NULL);
  return pp;
}

struct participant *new_participant (nn_guid_prefix_t idprefix, unsigned flags)
{
  struct participant *pp;
  os_mutexLock (&lock);
  pp = new_participant_unl (idprefix, flags);
  os_mutexUnlock (&lock);
  return pp;
}

static void free_participant (void *vpp)
{
  struct participant *pp = vpp;
  pp->dying = 1;

  {
    serdata_t serdata;
    serdata = construct_spdp_sample_dead (pp);
    write_sample (NULL, pp->spdp_pp_writer, serdata);
  }

  assert (nparticipants > 0);
  --nparticipants;
  avl_free (&pp->readers);
  avl_free (&pp->writers);
  delete_xevent (pp->spdp_xevent);
  delete_xevent (pp->pmd_update_xevent);
  if (config.many_sockets_mode)
  {
    os_sockFree (pp->sock);
    if (pp->participant_set_index < nparticipants)
    {
      struct participant *pp1 = participant_set[nparticipants];
      pp1->participant_set_index = pp->participant_set_index;
      participant_set[pp1->participant_set_index] = pp1;
    }
    participant_set_changed = 1;
  }
  os_free (pp);
}

void delete_participant (struct participant *pp)
{
  int tsec, tusec;
  time_to_sec_usec (&tsec, &tusec, now ());
  os_mutexLock (&lock);
  assert (avl_lookup (&pptree, &pp->guid, NULL) == (void *) pp);
  nn_log (LC_DISCOVERY, "%d.%06d DELETE_PARTICIPANT %x:%x:%x:%x\n", tsec, tusec, PGUID (pp->guid));
  avl_delete (&pptree, pp);
  os_mutexUnlock (&lock);
}

void delete_reader (struct reader *rd)
{
  struct participant *pp;
  double age;
  int tsec, tusec;
  time_to_sec_usec (&tsec, &tusec, now ());
  os_mutexLock (&lock);
  pp = rd->participant;
  nn_log (LC_DISCOVERY, "%d.%06d DELETE_READER %x:%x:%x:%x\n", tsec, tusec, PGUID (rd->guid));
  age = time_to_double (now () - rd->tcreate);
  nn_log (LC_TRACE, "reader %x:%x:%x:%x age %g ndelivered %lld\n", PGUID (rd->guid), age, rd->ndelivered);
  assert (avl_lookup (&pp->readers, &rd->guid, NULL) == (void *) rd);
  avl_delete (&pp->readers, rd);
  os_mutexUnlock (&lock);
}

void delete_writer (struct writer *wr)
{
  struct participant *pp;
  double age;
  int tsec, tusec;
  time_to_sec_usec (&tsec, &tusec, now ());
  os_mutexLock (&lock);
  wr->deleting = 1;
  while (wr->throttling > 0)
    os_condWait (&throttle_cond, &lock);
  pp = wr->participant;
  nn_log (LC_DISCOVERY, "%d.%06d DELETE_WRITER %x:%x:%x:%x\n", tsec, tusec, PGUID (wr->guid));
  age = time_to_double (now () - wr->tcreate);
  nn_log (LC_TRACE, "writer %x:%x:%x:%x age %g seq %lld\n", PGUID (wr->guid), age, wr->seq);
  assert (avl_lookup (&pp->writers, &wr->guid, NULL) == (void *) wr);
  avl_delete (&pp->writers, wr);
  os_mutexUnlock (&lock);
}

static void augment_whc_readers_node (void *vnode)
{
  struct whc_readers_node *n = vnode;
  os_int64 s = n->seq;
  if (n->avlnode.left && n->avlnode.left->seq < s)
    s = n->avlnode.left->seq;
  if (n->avlnode.right && n->avlnode.right->seq < s)
    s = n->avlnode.right->seq;
  n->min_seq = s;
}

static void augment_whc_seq_node (void *vnode)
{
  struct whc_node *n = vnode;
  if (n->avlnode_seq.left)
    n->minseq = n->avlnode_seq.left->minseq;
  else
    n->minseq = n->seq;
  if (n->avlnode_seq.right)
    n->maxseq = n->avlnode_seq.right->maxseq;
  else
    n->maxseq = n->seq;
}

static void add_writer_for_sedp (struct writer *wr)
{
  if (is_builtin_entityid (wr->guid.entityid))
    /* Don't add entries for builtin entities: them aint necessary,
       and it gets rid of a bootstrap issue */
    return;
  sedp_write_writer (wr->participant->sedp_writer_writer, wr);
}

static void add_reader_for_sedp (struct reader *rd)
{
  if (is_builtin_entityid (rd->guid.entityid))
    /* see add_writer_for_sedp() */
    return;
  sedp_write_reader (rd->participant->sedp_reader_writer, rd);
}

static void free_whc_seq_node (void *vnode)
{
  struct whc_node *n = vnode;
  assert (!n->in_tlidx);
  serdata_unref (n->serdata);
  os_free (n);
}

static void free_whc_tlidx_node (void *vnode)
{
  /* All msgs in the WHC are in the tree ordered on sequence numbers,
     some of them also are in the one ordered on key value for
     tracking transient-local data. Freeing a TLIDX node reduces to
     marking the node as *not* in the tlidx, actually releasing the
     memory is done by freeing the SEQ node. */
  struct whc_node *n = vnode;
  assert (n->in_tlidx);
  n->in_tlidx = 0;
}

static int rebuild_writer_addrset_walkrd (void *vnode, void *varg)
{
  const struct whc_readers_node *n = vnode;
  struct addrset *dst = varg;
  nn_locator_udpv4_t addr;
  if (addrset_any_mc (n->proxy_reader->c.as, &addr))
    add_to_addrset (dst, &addr);
  else if (addrset_any_uc (n->proxy_reader->c.as, &addr))
    add_to_addrset (dst, &addr);
  return AVLWALK_CONTINUE;
}

static void rebuild_writer_addrset (struct writer *wr)
{
  unref_addrset (wr->as);
  wr->as = new_addrset ();
  if (!avl_empty (&wr->readers))
  {
    nn_locator_udpv4_t addr;
    if (wr->readers.root->avlnode.height == 1 &&
        addrset_any_uc (wr->readers.root->proxy_reader->c.as, &addr))
      add_to_addrset (wr->as, &addr);
    else
      avl_walk (&wr->readers, rebuild_writer_addrset_walkrd, wr->as);
  }
}

static void free_whc_readers_node (void *vnode)
{
  struct whc_readers_node *n = vnode;
  avl_delete (&n->proxy_reader->writers, n);
  rebuild_writer_addrset (n->writer);
  remove_acked_messages (n->writer);
  nn_lat_estim_fini (&n->hb_to_ack_latency);
  os_free (n);
}

static int set_topic_type_name (nn_xqos_t *xqos, topic_t topic)
{
  if (!(xqos->present & QP_TYPE_NAME) && topic)
  {
    xqos->present |= QP_TYPE_NAME;
    xqos->type_name = os_strdup (topic_typename (topic));
  }
  if (!(xqos->present & QP_TOPIC_NAME) && topic)
  {
    xqos->present |= QP_TOPIC_NAME;
    xqos->topic_name = os_strdup (topic_name (topic));
  }
  return 0;
}

static int new_reader_writer_common (nn_guid_t *pguid, topic_t *ptopic, const struct participant *pp, nn_entityid_t id, C_STRUCT (v_topic) const * const ospl_topic, const nn_xqos_t *xqos)
{
  nn_guid_t guid;
  topic_t topic;
  int tsec, tusec;
  const char *partition = "(default)";
  const char *partition_suffix = "";
  time_to_sec_usec (&tsec, &tusec, now ());
  assert (is_builtin_entityid (id) ? (ospl_topic == NULL) : (ospl_topic != NULL));
  if (ospl_topic == NULL)
    topic = NULL;
  else if ((topic = deftopic (ospl_topic, NULL)) == NULL)
    return -1;
  guid = pp->guid;
  guid.entityid = id;
  if (is_builtin_entityid (id))
  {
    /* continue printing it as not being in a partition, the actual
       value doesn't matter because it is never matched based on QoS
       settings */
    partition = "(null)";
  }
  else if ((xqos->present & QP_PARTITION) && xqos->partition.n > 0 && strcmp (xqos->partition.strs[0], "") != 0)
  {
    partition = xqos->partition.strs[0];
    if (xqos->partition.n > 1)
      partition_suffix = "+";
  }
  nn_log (LC_DISCOVERY, "%d.%06d new_%s(%x:%x:%x:%x, %s%s.%s/%s)\n", tsec, tusec, is_writer_entityid (id) ? "writer" : "reader", PGUID (guid), partition, partition_suffix, topic ? topic_name (topic) : "(null)", topic ? topic_typename (topic) : "(null)");
  *pguid = guid;
  *ptopic = topic;
  return 0;
}

static struct writer *new_writer_unl (struct participant *pp, nn_entityid_t id, C_STRUCT (v_topic) const * const ospl_topic, const nn_xqos_t *xqos)
{
  struct writer *wr;
  avlparent_t pp_parent;
  nn_guid_t guid;
  topic_t topic;
  assert (is_writer_entityid (id));
  if (new_reader_writer_common (&guid, &topic, pp, id, ospl_topic, xqos) != 0)
    return NULL;
  if (avl_lookup (&pp->writers, &guid, &pp_parent) != NULL)
    abort ();
  wr = os_malloc (sizeof (*wr));
  avl_init_node (&wr->avlnode, pp_parent);
  wr->participant = pp;
  wr->guid = guid;
  wr->seq = 0;
  wr->hbcount = 0;
  wr->hbfragcount = 0;
  wr->tcreate = now ();
  wr->throttling = 0;
  wr->deleting = 0;
  wr->dying = 0;
  wr->hb_before_next_msg = 0;
  wr->xqos = os_malloc (sizeof (*wr->xqos));
  nn_xqos_copy (wr->xqos, xqos);
  nn_xqos_mergein_missing (wr->xqos, &default_xqos_wr);
  assert (wr->xqos->aliased == 0);
  set_topic_type_name (wr->xqos, topic);
  if (config.enabled_logcats & LC_TRACE)
  {
    LOGBUF_DECLNEW (qosbuf);
    nn_logb (qosbuf, LC_TRACE, "WRITER %x:%x:%x:%x QOS={", PGUID (wr->guid));
    nn_logb_xqos (qosbuf, LC_TRACE, wr->xqos);
    nn_logb (qosbuf, LC_TRACE, "}\n");
    nn_logb_flush (qosbuf);
    LOGBUF_FREE (qosbuf);
  }
  assert (wr->xqos->present & QP_RELIABILITY);
  wr->reliable = (wr->xqos->reliability.kind != NN_BEST_EFFORT_RELIABILITY_QOS);
  assert (wr->xqos->present & QP_DURABILITY);
  wr->handle_as_transient_local = (wr->xqos->durability.kind == NN_TRANSIENT_LOCAL_DURABILITY_QOS);
  wr->with_key = ((id.u & NN_ENTITYID_KIND_MASK) == NN_ENTITYID_KIND_WRITER_WITH_KEY);
  /* Startup mode causes the writer to treat data in its WHC as if
     transient-local, for the first few seconds after startup of the
     DDSI service. It is done for all writers, except: volatile
     best-effort and real transient-local writers. */
  wr->startup_mode = startup_mode &&
    (wr->xqos->durability.kind >= NN_TRANSIENT_DURABILITY_QOS ||
     (wr->xqos->durability.kind == NN_VOLATILE_DURABILITY_QOS &&
      wr->xqos->reliability.kind != NN_BEST_EFFORT_RELIABILITY_QOS));
  wr->topic = topic;
  wr->as = new_addrset ();
  wr->heartbeat_xevent = wr->reliable ? qxev_heartbeat (xevents, T_NEVER, wr) : NULL;
  wr->t_of_last_heartbeat = 0;
  wr->whc_seq_size = 0;
  wr->whc_tlidx_size = 0;
  assert (wr->xqos->present & QP_LIVELINESS);
  if (wr->xqos->liveliness.kind != NN_AUTOMATIC_LIVELINESS_QOS ||
      nn_from_ddsi_duration (wr->xqos->liveliness.lease_duration) != T_NEVER)
  {
    nn_log (LC_INFO, "writer %x:%x:%x:%x: incorrectly treating it as of automatic liveliness kind with lease duration = inf (%d, %lld)\n", PGUID (guid), (int) wr->xqos->liveliness.kind, nn_from_ddsi_duration (wr->xqos->liveliness.lease_duration));
  }
  wr->lease_duration = T_NEVER; /* FIXME */
  avl_init (&wr->whc_seq, offsetof (struct whc_node, avlnode_seq), offsetof (struct whc_node, seq), compare_seq, augment_whc_seq_node, free_whc_seq_node);
  avl_init_indkey (&wr->whc_tlidx, offsetof (struct whc_node, avlnode_tlidx), offsetof (struct whc_node, serdata), (int (*) (const void *, const void *)) serdata_cmp, 0, free_whc_tlidx_node);
  avl_init (&wr->readers, offsetof (struct whc_readers_node, avlnode), offsetof (struct whc_readers_node, reader_guid), compare_guid, augment_whc_readers_node, free_whc_readers_node);
  avl_insert (&pp->writers, wr);
  add_writer_for_sedp (wr);
  dds_attach_writer_to_proxies (wr);
  if (wr->lease_duration != T_NEVER)
    resched_xevent_if_earlier (wr->participant->pmd_update_xevent, 0);
  return wr;
}

struct writer *new_writer (struct participant *pp, nn_entityid_t id, C_STRUCT (v_topic) const * const ospl_topic, const nn_xqos_t *xqos)
{
  struct writer *wr;
  os_mutexLock (&lock);
  wr = new_writer_unl (pp, id, ospl_topic, xqos);
  os_mutexUnlock (&lock);
  return wr;
}

static void create_a_group (const char *name, topic_t topic)
{
  v_partitionQos pqos;
  v_partition part;
  v_group group;
  nn_xqos_t dummy_part_xqos;
  struct participant *pp;
  struct proxy_writer *pwr;

  /* any non-wildcard one will do */
  /* create it -- partitions require a v_partitionQos parameter, but
     that (thankfully!) isn't used by it ... phew! */
  nn_log (LC_TRACE, "create_a_group: %s.%s\n", name, topic_name (topic));
  memset (&pqos, 0, sizeof (pqos));
  part = v_partitionNew (ospl_kernel, name, pqos);
  group = v_groupSetCreate (ospl_kernel->groupSet, part, topic_ospl_topic (topic));
  /* Assuming the kernel will send a group creation event to all
     interested parties, which includes our own local discovery -- so
     that'll then notify the kernel we are interested in it.

     Nonetheless, there are the local readers' "matching_groups" to be
     updated, and, consequently, the local proxy-writers' "groups",
     too. Eventually all this will have to be cleaned up, it'll be
     really too messy and time consuming once qos changes are allowed. */
  nn_xqos_init_empty (&dummy_part_xqos);
  dummy_part_xqos.present = QP_PARTITION;
  dummy_part_xqos.partition.n = 1;
  dummy_part_xqos.partition.strs = (char **) &name;
  for (pp = avl_findmin (&pptree); pp; pp = avl_findsucc (&pptree, pp))
  {
    struct reader *rd;
    for (rd = avl_findmin (&pp->readers); rd; rd = avl_findsucc (&pp->readers, rd))
    {
      if (rd->topic == topic && partitions_match_p (rd->xqos, &dummy_part_xqos))
        nn_groupset_add_group (rd->matching_groups, group);
    }
  }
  for (pwr = avl_findmin (&proxywrtree); pwr; pwr = avl_findsucc (&proxywrtree, pwr))
  {
    if (!avl_empty (&pwr->readers) && pwr->readers.root->reader->topic == topic &&
        partitions_match_p (pwr->c.xqos, &dummy_part_xqos))
      nn_groupset_add_group (pwr->groups, group);
  }
}

static const char *any_nonwildcard_partition (const nn_partition_qospolicy_t *ps)
{
  int i;
  for (i = 0; i < ps->n; i++)
    if (!is_wildcard_partition (ps->strs[i]))
      return ps->strs[i];
  return NULL;
}

void add_proxy_reader_to_writer (struct writer *wr, struct proxy_reader *prd)
{
  struct whc_readers_node *n;
  avlparent_t parent, parent_pe;
  os_int64 tnow = now ();
  if (wr->guid.entityid.u == NN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER)
  {
    /* SPDP isn't implicitly matched, because: (1) we have to accept
       messages from unknown writers and thus have no use for the
       matching, and (2) we have to send messages to all addresses in
       as_disc, regardless of the existence of readers. */
    return;
  }
  nn_log (LC_TRACE, " [wr %x:%x:%x:%x -> prd %x:%x:%x:%x]",
          PGUID (wr->guid), PGUID (prd->c.guid));
  if (avl_lookup (&wr->readers, &prd->c.guid, &parent) != NULL)
    /* attempting to add a proxy reader multiple times has no effect */
    return;
  if (avl_lookup (&prd->writers, &wr->guid, &parent_pe) != NULL)
    /* if it isn't in one tree, it mustn't be in the other one */
    abort ();
  n = os_malloc (sizeof (*n));
  avl_init_node (&n->avlnode, parent);
  avl_init_node (&n->proxyavlnode, parent_pe);
  n->writer = wr;
  /* pretend a best-effort reader has acked all messages that can ever
     be sent; a reliable one hasn't acked a thing yet; WHC decides
     there are no matched reliable readers when min ack'd sequence
     number over all matched readers is MAX_SEQ_NUMBER */
  n->seq = (prd->c.xqos->reliability.kind != NN_BEST_EFFORT_RELIABILITY_QOS) ? 0 : MAX_SEQ_NUMBER;
  n->pure_ack_received = 0;
  n->reader_guid = prd->c.guid;
  n->writer_guid = wr->guid;
  n->proxy_reader = prd;
  n->last_acknack = 0;
  n->last_nackfrag = 0;
  nn_lat_estim_init (&n->hb_to_ack_latency);
  n->hb_to_ack_latency_tlastlog = now ();
  avl_insert (&wr->readers, n);
  avl_insert (&prd->writers, n);
  rebuild_writer_addrset (wr);
  if (!avl_empty (&wr->whc_seq) && wr->heartbeat_xevent)
  {
    /* new reader & data available -> make sure the next heartbeat is
       sent very soon */
    int delta = 10;
    if (vendor_is_twinoaks (prd->c.pp->vendor))
      /* needed at some point, so still here */
      delta = 100;
    resched_xevent_if_earlier (wr->heartbeat_xevent, tnow + delta * T_MILLISECOND);
  }

  /* for proxy readers using a non-wildcard partition matching a
     wildcard partition at the writer (and only a wildcard partition),
     ensure that a matching non-wildcard partition exists */
  if (!is_builtin_entityid (wr->guid.entityid))
  {
    const char *realname = NULL;
    if (partition_match_based_on_wildcard_in_left_operand (wr->xqos, prd->c.xqos, &realname))
    {
      assert (realname != NULL);
      create_a_group (realname, wr->topic);
    }
  }
}

static void remove_proxy_reader_from_writer (struct writer *wr, struct proxy_reader *prd)
{
  struct whc_readers_node *n;
  n = avl_lookup (&wr->readers, &prd->c.guid, NULL);
  assert (n != NULL);
  assert (avl_lookup (&prd->writers, &wr->guid, NULL) == n);
  avl_delete (&wr->readers, n);
}

#if 0
static int prettyprint_whc_helper (void *vnode, UNUSED_ARG (void *varg))
{
  static char tmp[1024];
  struct whc_node *node = vnode;
  int tmpsize = sizeof (tmp), res;
  res = prettyprint_serdata (tmp, tmpsize, node->serdata);
  assert (res >= 0);
  nn_log (LC_TRACE, "  seq %lld [%s] %s%s\n", node->seq, node->in_tlidx ? "tl" : "", tmp, res >= tmpsize ? " (trunc)" : "");
  return AVLWALK_CONTINUE;
}

static void prettyprint_whc (struct writer *wr)
{
  nn_log (LC_TRACE, "WHC writer %x:%x:%x:%x seq#%d tlidx#%d:\n", PGUID (wr->guid), wr->whc_seq_size, wr->whc_tlidx_size);
  avl_walk (&wr->whc_seq, prettyprint_whc_helper, NULL);
  assert (wr->whc_seq_size >= wr->whc_tlidx_size);
}
#else
static void prettyprint_whc (UNUSED_ARG_NDEBUG (struct writer *wr))
{
  assert (wr->whc_seq_size >= wr->whc_tlidx_size);
}
#endif

static int have_reliable_subs (const struct writer *wr)
{
  if (avl_empty (&wr->readers) || wr->readers.root->min_seq == MAX_SEQ_NUMBER)
    return 0;
  else
    return 1;
}

static int have_reliable_subs_proxy (const struct proxy_writer *pwr)
{
  return pwr->n_reliable_readers > 0;
}

static int add_sample_to_whc (struct writer *wr, os_int64 seq, serdata_t serdata)
{
  /* serdata's reference count on input is transferred to the whc, cos
     no-one cares about it afterwards */
  struct whc_node *newn = NULL;

#if 0
  nn_log (LC_TRACE, "add_msg_to_whc: seq %lld key", seq);
  debug_print_rawdata ("", serdata->key, sizeof (serdata->key));
  nn_log (LC_TRACE, "\n");
#endif

  /* An unreliable writer may not have any reliable subs */
  assert (wr->reliable || have_reliable_subs (wr) == 0);
  if ((wr->reliable && have_reliable_subs (wr)) ||
      wr->handle_as_transient_local ||
      wr->startup_mode)
  {
    avlparent_t parent;
    if (avl_lookup (&wr->whc_seq, &seq, &parent) != NULL)
      abort ();
    newn = os_malloc (sizeof (*newn));
    avl_init_node (&newn->avlnode_seq, parent);
    newn->seq = seq;
    newn->in_tlidx = 0; /* if transient_local it will always become 1 in this call */
    newn->serdata = serdata_ref (serdata);
    avl_insert (&wr->whc_seq, newn);
    wr->whc_seq_size++;
    /*nn_log (LC_TRACE, "add_msg_to_whc: insert %p in seq\n", newn);*/
  }

  if (wr->handle_as_transient_local || wr->startup_mode)
  {
    avlparent_t parent;
    struct whc_node *oldtln = avl_lookup (&wr->whc_tlidx, serdata, &parent);
    /* Dispose & unregister don't go into the TLidx, or it'll keep
       growing. Well, actually, they should be in there for a while,
       or else a late reader might get a hopelessly delayed earlier
       sample and think it is still alive. So do insert them for
       now. */
    int insert_in_tlidx;
    insert_in_tlidx = 1;
#if 0
      ((serdata->v.msginfo.statusinfo &
        (NN_STATUSINFO_DISPOSE | NN_STATUSINFO_UNREGISTER)) == 0);
#endif
    if (insert_in_tlidx)
    {
      avl_init_node (&newn->avlnode_tlidx, parent);
      newn->in_tlidx = 1;
      avl_insert (&wr->whc_tlidx, newn);
      wr->whc_tlidx_size++;
    }
    /*nn_log (LC_TRACE, "add_msg_to_whc: insert %p in tlidx\n", newn);*/
    if (oldtln != NULL)
    {
      /*nn_log (LC_TRACE, "add_msg_to_whc: delete old %p from tlidx\n", oldtln);*/
      avl_delete (&wr->whc_tlidx, oldtln);
      wr->whc_tlidx_size--;
      /* taking oldtln out of tlidx means it may no longer be needed;
         if so, remove it from whc_seq */
      if (config.aggressive_keep_last1_whc ||
          (!have_reliable_subs (wr) || oldtln->seq < wr->readers.root->min_seq))
      {
        /*nn_log (LC_TRACE, "add_msg_to_whc: delete old %p from seq\n", oldtln);*/
        avl_delete (&wr->whc_seq, oldtln);
        wr->whc_seq_size--;
      }
    }
  }

  if (wr->reliable || wr->handle_as_transient_local || wr->startup_mode)
    prettyprint_whc (wr);

  /* new data -> make sure heartbeat is scheduled for soonish transmission */
  if (wr->heartbeat_xevent)
    resched_xevent_if_earlier (wr->heartbeat_xevent, now () + 50 * T_MILLISECOND);
  return 0;
}

static int whc_delete_tlidx_entry_helper (UNUSED_ARG (void *node), UNUSED_ARG (void *arg))
{
  return AVLWALK_DELETE;
}

static int whc_end_startup_mode_drop_tlidx (struct writer *wr)
{
  assert (wr->reliable);
  assert (wr->startup_mode && !wr->handle_as_transient_local);
  wr->startup_mode = 0;
  avl_walk (&wr->whc_tlidx, whc_delete_tlidx_entry_helper, NULL);
  /* updating whc_tlidx_size not done by free function cos it can't:
     the whc nodes don't contain a back pointer */
  wr->whc_tlidx_size = 0;
  return remove_acked_messages (wr);
}

static void free_rhc_writers_node (void *vnode)
{
  struct rhc_writers_node *n = vnode;
  if (n->acknack_xevent)
    delete_xevent (n->acknack_xevent);
  avl_delete (&n->proxy_writer->readers, n);
  nn_reorder_free (n->u.not_in_sync.reorder);
  os_free (n);
}

static struct reader *new_reader_unl (struct participant *pp, nn_entityid_t id, C_STRUCT (v_topic) const * const ospl_topic, const nn_xqos_t *xqos)
{
  struct reader *rd;
  avlparent_t pp_parent;
  nn_guid_t guid;
  topic_t topic;
  assert (!is_writer_entityid (id));
  if (new_reader_writer_common (&guid, &topic, pp, id, ospl_topic, xqos) != 0)
    return NULL;
  if (avl_lookup (&pp->readers, &guid, &pp_parent) != NULL)
    abort ();
  rd = os_malloc (sizeof (*rd));
  avl_init_node (&rd->avlnode, pp_parent);
  rd->participant = pp;
  rd->guid = guid;
  rd->tcreate = now ();
  rd->ndelivered = 0;
  rd->dying = 0;
  rd->xqos = os_malloc (sizeof (*rd->xqos));
  nn_xqos_copy (rd->xqos, xqos);
  nn_xqos_mergein_missing (rd->xqos, &default_xqos_rd);
  assert (rd->xqos->aliased == 0);
  set_topic_type_name (rd->xqos, topic);
  if (config.enabled_logcats & LC_TRACE)
  {
    LOGBUF_DECLNEW (qosbuf);
    nn_logb (qosbuf, LC_TRACE, "READER %x:%x:%x:%x QOS={", PGUID (rd->guid));
    nn_logb_xqos (qosbuf, LC_TRACE, rd->xqos);
    nn_logb (qosbuf, LC_TRACE, "}\n");
    nn_logb_flush (qosbuf);
    LOGBUF_FREE (qosbuf);
  }
  assert (rd->xqos->present & QP_RELIABILITY);
  rd->reliable = (rd->xqos->reliability.kind != NN_BEST_EFFORT_RELIABILITY_QOS);
  assert (rd->xqos->present & QP_DURABILITY);
  rd->handle_as_transient_local = (rd->xqos->durability.kind == NN_TRANSIENT_LOCAL_DURABILITY_QOS);
  rd->topic = topic;
  rd->matching_groups = nn_groupset_new ();
  if (!is_builtin_entityid (rd->guid.entityid))
    nn_groupset_fromqos (rd->matching_groups, ospl_kernel, rd->xqos);
  assert (rd->xqos->present & QP_LIVELINESS);
  if (rd->xqos->liveliness.kind != NN_AUTOMATIC_LIVELINESS_QOS ||
      nn_from_ddsi_duration (rd->xqos->liveliness.lease_duration) != T_NEVER)
  {
    nn_log (LC_INFO, "reader %x:%x:%x:%x: incorrectly treating it as of automatic liveliness kind with lease duration = inf (%d, %lld)\n", PGUID (guid), (int) rd->xqos->liveliness.kind, nn_from_ddsi_duration (rd->xqos->liveliness.lease_duration));
  }
  rd->lease_duration = T_NEVER; /* FIXME */
  avl_init (&rd->writers, offsetof (struct rhc_writers_node, avlnode), offsetof (struct rhc_writers_node, writer_guid), compare_guid, 0, free_rhc_writers_node);
  avl_insert (&pp->readers, rd);
  add_reader_for_sedp (rd);
  dds_attach_reader_to_proxies (rd);
  return rd;
}

struct reader *new_reader (struct participant *pp, nn_entityid_t id, C_STRUCT (v_topic) const * const ospl_topic, const nn_xqos_t *xqos)
{
  struct reader *rd;
  os_mutexLock (&lock);
  rd = new_reader_unl (pp, id, ospl_topic, xqos);
  os_mutexUnlock (&lock);
  return rd;
}

static int add_matching_groups_helper (v_group g, void *varg)
{
  /* Damn. Didn't think it through all the way, so now I need to check
     partition matching *again* :-( Eventually, a real solution will
     be implemented. */
  struct proxy_writer *pwr = varg;
  const char *pname = v_partitionName (g->partition);
  nn_xqos_t tmp;
  tmp.present = QP_PARTITION;
  tmp.partition.n = 1;
  tmp.partition.strs = (char **) &pname;
  if (partitions_match_p (pwr->c.xqos, &tmp))
    return nn_groupset_add_group (pwr->groups, g);
  else
    return 0;
}

void add_proxy_writer_to_reader (struct reader *rd, struct proxy_writer *pwr)
{
  struct rhc_writers_node *n;
  avlparent_t parent, parent_pe;
  os_int64 tnow;
  os_int64 last_deliv_seq;
  int delta = 10;
  if (rd->guid.entityid.u == NN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_READER)
  {
    /* SPDP isn't implicitly matched, because: (1) we have to accept
       messages from unknown writers and thus have no use for the
       matching, and (2) we have to send messages to all addresses in
       as_disc, regardless of the existence of readers. */
    return;
  }
  tnow = now ();
  if (vendor_is_twinoaks (pwr->c.pp->vendor))
    delta = 100;
  nn_log (LC_TRACE, " [pwr %x:%x:%x:%x -> rd %x:%x:%x:%x]", PGUID (pwr->c.guid), PGUID (rd->guid));
  /* can't match a reliable reader with a best-effort writer */
  assert (!rd->reliable || (pwr->c.xqos->reliability.kind != NN_BEST_EFFORT_RELIABILITY_QOS));
  if (avl_lookup (&rd->writers, &pwr->c.guid, &parent) != NULL)
    /* attempting to add a proxy reader multiple times has no effect */
    return;
  if (avl_lookup (&pwr->readers, &rd->guid, &parent_pe) != NULL)
    /* if it isn't in one tree, it mustn't be in the other one */
    abort ();
  if (!is_builtin_entityid (rd->guid.entityid) && nn_groupset_empty (rd->matching_groups))
  {
    const char *name;
    assert (pwr->c.xqos->present & QP_PARTITION);
    name = any_nonwildcard_partition (&pwr->c.xqos->partition);
    assert (name != NULL);
    create_a_group (name, rd->topic);
    assert (!nn_groupset_empty (rd->matching_groups));
  }
  nn_groupset_foreach (rd->matching_groups, add_matching_groups_helper, pwr);
  n = os_malloc (sizeof (*n));
  avl_init_node (&n->avlnode, parent);
  avl_init_node (&n->proxyavlnode, parent_pe);
  n->reader = rd;
  n->proxy_writer = pwr;
  n->writer_guid = pwr->c.guid;
  n->reader_guid = rd->guid;
  /* We track the last heartbeat count value per reader--proxy-writer
     pair, so that we can correctly handle directed heartbeats. The
     only reason to bother is to prevent a directed heartbeat (with
     the FINAL flag clear) from causing AckNacks from all readers
     instead of just the addressed ones.

     If we don't mind those extra AckNacks, we could track the count
     at the proxy-writer and simply treat all incoming heartbeats as
     undirected. */
  n->last_heartbeat = 0;
  n->hb_timestamp = 0;
  last_deliv_seq = nn_reorder_next_seq (pwr->reorder) - 1;
  if (!rd->handle_as_transient_local || last_deliv_seq == 0)
  {
    /* not treated as transient-local or proxy-writer hasn't seen any
       data yet, in which case this reader is in sync with the proxy
       writer even though it is a transient-local one. */
    n->in_sync = 1;
  }
  else if (!config.conservative_builtin_reader_startup &&
           is_builtin_entityid (rd->guid.entityid) &&
           !avl_empty (&pwr->readers))
  {
    /* builtins really don't care about multiple copies */
    n->in_sync = 1;
  }
  else
  {
    /* normal transient-local */
    n->in_sync = 0;
    n->u.not_in_sync.end_of_tl_seq = last_deliv_seq;
  }
  n->count = 0;
  /* Spec says we may send a pre-emptive AckNack (8.4.2.3.4), hence we
     schedule it for delta * T_MILLISECOND */
  n->acknack_xevent = rd->reliable ? qxev_acknack (xevents, tnow + delta * T_MILLISECOND, n) : NULL;
  n->u.not_in_sync.reorder = nn_reorder_new (rd->reliable ? NN_REORDER_MODE_NORMAL : NN_REORDER_MODE_MONOTONICALLY_INCREASING, config.secondary_reorder_maxsamples);
  if (rd->reliable)
    pwr->n_reliable_readers++;
  avl_insert (&rd->writers, n);
  avl_insert (&pwr->readers, n);
}

static void remove_proxy_writer_from_reader (struct reader *rd, struct proxy_writer *pwr)
{
  struct rhc_writers_node *n;
  n = avl_lookup (&rd->writers, &pwr->c.guid, NULL);
  assert (n != NULL);
  assert (avl_lookup (&pwr->readers, &rd->guid, NULL) == n);
  avl_delete (&rd->writers, n);
  if (rd->reliable)
    pwr->n_reliable_readers--;
}

/* ///
/// */

static void chk_no_such_writer (nn_guid_t guid)
{
  struct participant *pp;
  struct writer *wr;
  nn_guid_t ppguid;
  ppguid = guid;
  ppguid.entityid.u = NN_ENTITYID_PARTICIPANT;
  if ((pp = avl_lookup (&pptree, &ppguid, NULL)) != NULL)
  {
    nn_log (LC_TRACE, "WEIRD: local participant does exist\n");
    if ((wr = avl_lookup (&pp->writers, &guid, NULL)) != NULL)
      nn_log (LC_TRACE, "WEIRDER: local writer does exist as well\n");
    /*abort ();*/
  }
}

static void chk_no_such_reader (nn_guid_t guid)
{
  struct participant *pp;
  struct reader *rd;
  nn_guid_t ppguid;
  ppguid = guid;
  ppguid.entityid.u = NN_ENTITYID_PARTICIPANT;
  if ((pp = avl_lookup (&pptree, &ppguid, NULL)) != NULL)
  {
    nn_log (LC_TRACE, "WEIRD: local participant does exist\n");
    if ((rd = avl_lookup (&pp->readers, &guid, NULL)) != NULL)
      nn_log (LC_TRACE, "WEIRDER: local reader does exist as well\n");
    /*abort ();*/
  }
}

static void bswapSN (nn_sequence_number_t *sn)
{
  sn->high = bswap4 (sn->high);
  sn->low = bswap4u (sn->low);
}

static void bswap_sequence_number_set_hdr (nn_sequence_number_set_t *snset)
{
  bswapSN (&snset->bitmap_base);
  snset->numbits = bswap4u (snset->numbits);
}

static void bswap_sequence_number_set_bitmap (nn_sequence_number_set_t *snset)
{
  int i, n = (snset->numbits + 31) / 32;
  for (i = 0; i < n; i++)
    snset->bits[i] = bswap4u (snset->bits[i]);
}

static void bswap_fragment_number_set_hdr (nn_fragment_number_set_t *fnset)
{
  fnset->bitmap_base = bswap4u (fnset->bitmap_base);
  fnset->numbits = bswap4u (fnset->numbits);
}

static void bswap_fragment_number_set_bitmap (nn_fragment_number_set_t *fnset)
{
  int i, n = (fnset->numbits + 31) / 32;
  for (i = 0; i < n; i++)
    fnset->bits[i] = bswap4u (fnset->bits[i]);
}

static nn_entityid_t to_entityid (unsigned u)
{
  nn_entityid_t e;
  e.u = u;
  return e;
}

static int add_Gap (struct nn_xmsg *msg, struct whc_readers_node *wrn, os_int64 start, os_int64 base, int numbits, const unsigned *bits)
{
  struct nn_xmsg_marker sm_marker;
  Gap_t *gap;
  assert (numbits > 0);
  if ((gap = nn_xmsg_append_aligned (msg, &sm_marker, GAP_SIZE (numbits), 4)) == NULL)
    return NN_ERR_OUT_OF_MEMORY;
  nn_xmsg_submsg_init (msg, sm_marker, SMID_GAP);
  gap->readerId = nn_hton_entityid (wrn->reader_guid.entityid);
  gap->writerId = nn_hton_entityid (wrn->writer_guid.entityid);
  gap->gapStart = toSN (start);
  gap->gapList.bitmap_base = toSN (base);
  gap->gapList.numbits = numbits;
  memcpy (gap->gapList.bits, bits, NN_SEQUENCE_NUMBER_SET_BITS_SIZE (numbits));
  nn_xmsg_submsg_setnext (msg, sm_marker);
  return 0;
}

static int make_heartbeat_ack_required (struct writer *wr)
{
  os_int64 tnow = now ();
  if (tnow >= wr->t_of_last_heartbeat + 100 * T_MILLISECOND)
    return 1;
  else if (wr->whc_seq_size - wr->whc_tlidx_size < config.whc_highwater_mark/4)
    return 0;
  else if (wr->whc_seq_size - wr->whc_tlidx_size < config.whc_highwater_mark/2)
    return tnow >= wr->t_of_last_heartbeat + 20 * T_MILLISECOND;
  else if (wr->whc_seq_size - wr->whc_tlidx_size < 2*config.whc_highwater_mark/3)
    return tnow >= wr->t_of_last_heartbeat + 2 * T_MILLISECOND;
  else
    return 1;
}

static int add_Heartbeat (struct nn_xmsg *msg, struct writer *wr, int *hbansreq, nn_entityid_t dst)
{
  struct nn_xmsg_marker sm_marker;
  Heartbeat_t *hb;

  assert (wr->reliable);
  if (!config.respond_to_rti_init_zero_ack_with_invalid_heartbeat)
  {
    /* We're not really allowed to generate heartbeats when the WHC is
       empty, but it appears RTI sort-of needs them ... */
    assert (!avl_empty (&wr->whc_seq));
  }

  if ((hb = nn_xmsg_append_aligned (msg, &sm_marker, sizeof (Heartbeat_t), 4)) == NULL)
    return NN_ERR_OUT_OF_MEMORY;
  nn_xmsg_submsg_init (msg, sm_marker, SMID_HEARTBEAT);

  if (*hbansreq < 0)
    *hbansreq = make_heartbeat_ack_required (wr);
  if (! *hbansreq)
    hb->smhdr.flags |= HEARTBEAT_FLAG_FINAL;

  hb->readerId = nn_hton_entityid (dst);
  hb->writerId = nn_hton_entityid (wr->guid.entityid);
  if (!avl_empty (&wr->whc_seq))
  {
    hb->firstSN = toSN (wr->whc_seq.root->minseq);
    hb->lastSN = toSN (wr->whc_seq.root->maxseq);
  }
  else /* Thank you, RTI */
  {
    hb->firstSN = toSN (wr->seq + 1);
    hb->lastSN = toSN (wr->seq);
  }
  hb->count = ++wr->hbcount;
  nn_xmsg_submsg_setnext (msg, sm_marker);
  return 0;
}

static int add_AckNack (struct nn_xmsg *msg, struct rhc_writers_node *rwn, int *isnack)
{
  /* If rwn->count == 0, no heartbeat has been received by this reader
     yet, so it is a pre-emptive one. NACKing data now will most
     likely cause another NACK upon reception of the first heartbeat,
     and so cause the data to be resent twice. */
  const int max_numbits = 256; /* as spec'd */
  struct nn_reorder *reorder;
  nn_count_t *countp;
  AckNack_t *an;
  struct nn_xmsg_marker sm_marker;
  int numbits;
  unsigned i;

  /* if in sync, look at proxy writer status, else look at
     proxy-writer--reader match status */
  if (rwn->in_sync)
    reorder = rwn->proxy_writer->reorder;
  else
    reorder = rwn->u.not_in_sync.reorder;

  if ((an = nn_xmsg_append_aligned (msg, &sm_marker, ACKNACK_SIZE_MAX, 4)) == NULL)
    return NN_ERR_OUT_OF_MEMORY;
  nn_xmsg_submsg_init (msg, sm_marker, SMID_ACKNACK);
  an->readerId = nn_hton_entityid (rwn->reader_guid.entityid);
  an->writerId = nn_hton_entityid (rwn->writer_guid.entityid);

  /* Make bitmap; note that we've made sure to have room the for
     maximum bitmap size. */
  numbits = nn_reorder_nackmap (reorder, rwn->proxy_writer->last_seq, &an->readerSNState, max_numbits);

  /* Let caller know whether it is a nack, and set final if it isn't.
     Who cares about an answer to an acknowledgment!? -- actually,
     that'd a very useful feature in combination with directed
     heartbeats, or somesuch, to get reliability guarantees. */
  *isnack = (numbits > 0);
  if (! *isnack)
    an->smhdr.flags |= ACKNACK_FLAG_FINAL;

  /* If we refuse to send invalid AckNacks, grow a length-0 bitmap and
     zero-fill it. Cleared bits are meaningless (DDSI 2.1, table 8.33,
     although RTI seems to think otherwise). */
  if (numbits == 0 && config.acknack_numbits_emptyset > 0)
  {
    an->readerSNState.numbits = config.acknack_numbits_emptyset;
    nn_bitset_zero (an->readerSNState.numbits, an->readerSNState.bits);
  }

  /* Count field is at a variable offset ... silly DDSI spec. */
  countp =
    (nn_count_t *) ((char *) an + offsetof (AckNack_t, readerSNState) +
                    NN_SEQUENCE_NUMBER_SET_SIZE (an->readerSNState.numbits));
  *countp = ++rwn->count;

  /* Reset submessage size, now that we know the real size, and update
     the offset to the next submessage. */
  nn_xmsg_shrink (msg, sm_marker, ACKNACK_SIZE (an->readerSNState.numbits));
  nn_xmsg_submsg_setnext (msg, sm_marker);

  nn_log (LC_TRACE, "acknack %x:%x:%x:%x -> %x:%x:%x:%x: #%d:%lld/%d:", PGUID (rwn->reader_guid), PGUID (rwn->writer_guid), rwn->count, fromSN (an->readerSNState.bitmap_base), an->readerSNState.numbits);
  for (i = 0; i != an->readerSNState.numbits; i++)
    nn_log (LC_TRACE, "%c", nn_bitset_isset (numbits, an->readerSNState.bits, i) ? '1' : '0');
  nn_log (LC_TRACE, "\n");
  return 0;
}

static int is_mcaddr (const nn_locator_udpv4_t *addr)
{
  return IN_MULTICAST (ntohl (addr->address));
}

static os_size_t handle_xevk_msg (UNUSED_ARG (logbuf_t lb), struct nn_xpack *xp, struct xevent *ev)
{
  nn_xpack_addmsg (xp, ev->u.msg.msg);
  delete_xevent (ev);
  return 0;
}

static os_size_t handle_xevk_heartbeat (logbuf_t lb, UNUSED_ARG (struct nn_xpack *xp), struct xevent *ev)
{
  struct writer *wr = ev->u.heartbeat.wr;
  struct nn_xmsg *msg;
  int hbansreq;
  int tsec, tusec;
  os_int64 tnow = now ();
  time_to_sec_usec (&tsec, &tusec, tnow);

  assert (wr->reliable);
  if (avl_empty (&wr->whc_seq) || avl_empty (&wr->readers))
  {
    /* Can't transmit a valid heartbeat if there is no data; and it
       wouldn't actually be sent anywhere if there are no readers, so
       there is little point in processing the xevent all the time.

       Note that add_msg_to_whc and add_proxy_reader_to_writer will
       perform a reschedule. 8.4.2.2.3: need not (can't, really!) send
       a heartbeat if no data is available */
    nn_logb (lb, LC_TRACE, "resched: never (whc empty or no readers)\n");
    resched_xevent (ev, T_NEVER);
    return 0;
  }

  if ((msg = nn_xmsg_new (xmsgpool, datasock_mc, &wr->guid.prefix, sizeof (InfoTS_t) + sizeof (Heartbeat_t))) == NULL)
    goto outofmem;
  if (nn_xmsg_add_timestamp (msg, tnow) < 0)
    goto outofmem;
  nn_xmsg_setdstN (msg, wr->as);

  hbansreq = -1; /* auto-decide */
  if (add_Heartbeat (msg, wr, &hbansreq, to_entityid (NN_ENTITYID_UNKNOWN)) < 0)
    goto outofmem;

  nn_logb (lb, LC_TRACE, "%d.%06d queue heartbeat(wr %x:%x:%x:%x) msg %p\n",
           tsec, tusec, PGUID (wr->guid), msg);
  qxev_msg (xevents, 0, msg);

  if (hbansreq)
    wr->t_of_last_heartbeat = now ();

 outofmem:
  if (avl_empty (&wr->whc_seq) || avl_empty (&wr->readers))
  {
    /* see top of function */
    assert (0);
  }
  else if (wr->readers.root->min_seq >= wr->whc_seq.root->maxseq)
  {
    /* Slightly different from requiring an empty whc_seq: if it is
       transient_local, whc_seq usually won't be empty even when all
       msgs have been ack'd. 8.4.2.2.3: need not send heartbeats
       when all messages have been acknowledged. */
    nn_logb (lb, LC_TRACE, "resched: never (all acked)\n");
    resched_xevent (ev, T_NEVER);
  }
  else
  {
    /* 8.4.2.2.3: must send _periodic_ heartbeats */
    os_int64 tnow = now ();
    os_int64 mindelta = 10 * T_MILLISECOND;
    os_int64 maxdelta = (hbansreq ? 50 : 200) * T_MILLISECOND;
    os_int64 tnext = ev->tsched + maxdelta;
    if (tnext < tnow + mindelta)
      tnext = tnow + mindelta;
    nn_logb (lb, LC_TRACE, "resched: in %gs (%lld < %lld)\n",
             time_to_double (tnext - tnow),
             wr->readers.root->min_seq, wr->whc_seq.root->maxseq);
    resched_xevent (ev, tnext);
  }
  return 0;
}

static os_size_t handle_xevk_acknack (logbuf_t lb, UNUSED_ARG (struct nn_xpack *xp), struct xevent *ev)
{
  struct rhc_writers_node *rwn = ev->u.acknack.rwn;
  nn_locator_udpv4_t addr;
  struct nn_xmsg *msg;

  if (addrset_any_uc (rwn->proxy_writer->c.as, &addr) ||
      addrset_any_mc (rwn->proxy_writer->c.as, &addr))
  {
    int isnack;

    if ((msg = nn_xmsg_new (xmsgpool, datasock_mc, &rwn->reader_guid.prefix, ACKNACK_SIZE_MAX)) == NULL)
      goto outofmem;
    nn_xmsg_setdst1 (msg, &rwn->writer_guid.prefix, &addr);
    if (add_AckNack (msg, rwn, &isnack) < 0)
      goto outofmem;
    if (config.enabled_logcats & LC_TRACE)
    {
      int tsec, tusec;
      time_to_sec_usec (&tsec, &tusec, now ());
      nn_logb (lb, LC_TRACE, "%d.%06d queue acknack(rd %x:%x:%x:%x -> pwr %x:%x:%x:%x)\n",
               tsec, tusec, PGUID (rwn->reader->guid),
               PGUID (rwn->proxy_writer->c.guid));
    }
    qxev_msg (xevents, 0, msg);
  }
  else
  {
    if (config.enabled_logcats & LC_TRACE)
    {
      int tsec, tusec;
      time_to_sec_usec (&tsec, &tusec, now ());
      nn_logb (lb, LC_TRACE, "%d.%06d skip acknack(rd %x:%x:%x:%x -> pwr %x:%x:%x:%x): no address\n",
               tsec, tusec, PGUID (rwn->reader->guid),
               PGUID (rwn->proxy_writer->c.guid));
    }
  }

  /* not allowed to spontaneously send AckNacks, but we want to keep
     the event structure for eventual rescheduling upon receipt of a
     Heartbeat */
  resched_xevent (ev, T_NEVER);
  return 0;

 outofmem:
  /* What to do if out of memory?  Crash or burn? */
  if (msg)
    nn_xmsg_free (msg);
  resched_xevent (ev, 100 * T_MILLISECOND);
  return 0;
}

static size_t handle_xevk_spdp (logbuf_t lb, UNUSED_ARG (struct nn_xpack *xp), struct xevent *ev)
{
  struct participant *pp = ev->u.spdp.pp;
  struct writer *wr = pp->spdp_pp_writer;
  struct proxy_reader *prd;
  struct whc_node *whcn;
  os_int64 tnow = now ();
  os_int64 mindelta = 10 * T_MILLISECOND;
  os_int64 tnext = ev->tsched + (pp->lease_duration / 10) * 9;
  int tsec, tusec;

  time_to_sec_usec (&tsec, &tusec, tnow);

  if (tnext < tnow + mindelta)
    tnext = tnow + mindelta;
  nn_logb (lb, LC_TRACE, "%d.%06d xmit spdp %x:%x:%x:%x (resched %gs)\n",
           tsec, tusec, PGUID (pp->guid), time_to_double (tnext - tnow));

  if (ev->u.spdp.dest == NULL)
    prd = NULL;
  else
  {
    nn_guid_t guid;
    guid.prefix = ev->u.spdp.dest->guid.prefix;
    guid.entityid.u = NN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_READER;
    prd = avl_lookup (&proxyrdtree, &guid, NULL);
    if (prd == NULL)
    {
      nn_logb (lb, LC_TRACE, "xmit spdp: no proxy reader %x:%x:%x:%x\n", PGUID (guid));
      goto skip;
    }
  }

  whcn = avl_findmax (&wr->whc_seq);
  assert (whcn != NULL);
  transmit_sample (NULL, wr, whcn->seq, whcn->serdata, prd);

 skip:
  if (ev != pp->spdp_xevent)
    delete_xevent (ev);
  else
    resched_xevent (ev, tnext);
  return 0;
}

static size_t handle_xevk_pmd_update (logbuf_t lb, UNUSED_ARG (struct nn_xpack *xp), struct xevent *ev)
{
  os_int64 intv, tnext;
  struct participant *pp = ev->u.pmd_update.pp;
  int tsec, tusec;
  time_to_sec_usec (&tsec, &tusec, now ());
  write_pmd_message (pp);
  intv = pp->lease_duration;
  if (!avl_empty (&pp->writers) && pp->writers.root->min_lease_duration < intv)
    intv = pp->writers.root->min_lease_duration;
  if (intv == T_NEVER)
  {
    tnext = T_NEVER;
    nn_logb (lb, LC_TRACE, "%d.%06d resched pmd(%x:%x:%x:%x): never\n", tsec, tusec, PGUID (pp->guid));
  }
  else
  {
    os_int64 tnow = now (), mindelta = 100 * T_MILLISECOND;
    tnext = ev->tsched + intv - 100 * T_MILLISECOND;
    if (tnext < tnow + mindelta)
      tnext = tnow + mindelta;
    nn_logb (lb, LC_TRACE, "%d.%06d resched pmd(%x:%x:%x:%x): %gs\n", tsec, tusec, PGUID (pp->guid), time_to_double (tnext - tnow));
  }
  resched_xevent (ev, tnext);
  return 0;
}

static int info_nn_log (const char *fmt, ...)
{
  va_list ap;
  int n;
  va_start (ap, fmt);
  n = nn_vlog (LC_INFO, fmt, ap);
  va_end (ap);
  return n;
}

static size_t handle_xevk_info (logbuf_t lb, UNUSED_ARG (struct nn_xpack *xp), struct xevent *ev)
{
  static os_uint64 seq0, seq1;
  struct mlv_stats st;
  int tsec, tusec;
  time_to_sec_usec (&tsec, &tusec, now ());
  mlv_stats (&st);
  nn_logb (lb, LC_INFO, "%d.%06d MLV %d %d %d %d #%llu\n", tsec, tusec, st.current, st.nblocks, st.nzeroblocks, st.nwhiledisabled, st.seq);
  nn_logb_flush (lb);
  mlv_printlive (seq0, seq1, info_nn_log);
  seq0 = seq1;
  seq1 = st.seq + 1;
  resched_xevent (ev, now () + 60 * T_SECOND);
  return 0;
}

static size_t handle_xevk_end_startup_mode (logbuf_t lb, UNUSED_ARG (struct nn_xpack *xp), struct xevent *ev)
{
  int tsec, tusec;
  struct participant *pp;
  assert (startup_mode);
  time_to_sec_usec (&tsec, &tusec, now ());
  nn_logb (lb, LC_TRACE, "%d.%06d end startup mode\n", tsec, tusec);
  startup_mode = 0;
  for (pp = avl_findmin (&pptree); pp; pp = avl_findsucc (&pptree, pp))
  {
    struct writer *wr;
    for (wr = avl_findmin (&pp->writers); wr; wr = avl_findsucc (&pp->writers, wr))
    {
      if (!wr->startup_mode)
        nn_logb (lb, LC_TRACE, "  wr %x:%x:%x:%x skipped\n", PGUID (wr->guid));
      else
      {
        int n;
        n = whc_end_startup_mode_drop_tlidx (wr);
        nn_logb (lb, LC_TRACE, "  wr %x:%x:%x:%x dropped %d entr%s\n", PGUID (wr->guid), n, n == 1 ? "y" : "ies");
      }
    }
  }
  delete_xevent (ev);
  return 0;
}

static size_t handle_xevents (struct nn_xpack *xp)
{
  os_int64 t = now ();
  int nhandled = 0;
  size_t nbytes = 0;
  LOGBUF_DECLNEW (lb);
  while (nhandled++ < config.max_xevents_batch_size && earliest_in_xeventq (xevents) <= t)
  {
    struct xevent *ev = next_from_xeventq (xevents);
    switch (ev->kind)
    {
      case XEVK_MSG:
        nbytes += handle_xevk_msg (lb, xp, ev);
        break;
      case XEVK_HEARTBEAT:
        nbytes += handle_xevk_heartbeat (lb, xp, ev);
        break;
      case XEVK_ACKNACK:
        nbytes += handle_xevk_acknack (lb, xp, ev);
        break;
      case XEVK_SPDP:
        nbytes += handle_xevk_spdp (lb, xp, ev);
        break;
      case XEVK_PMD_UPDATE:
        nbytes += handle_xevk_pmd_update (lb, xp, ev);
        break;
      case XEVK_INFO:
        nbytes += handle_xevk_info (lb, xp, ev);
        break;
      case XEVK_END_STARTUP_MODE:
        nbytes += handle_xevk_end_startup_mode (lb, xp, ev);
        break;
    }
    nn_logb_flush (lb);
  }
  LOGBUF_FREE (lb);
  nn_xpack_send (xp);
  return nbytes;
}

static void write_pmd_message (struct participant *pp)
{
  const unsigned pmd_kind = PARTICIPANT_MESSAGE_DATA_KIND_AUTOMATIC_LIVELINESS_UPDATE;
  struct writer *wr = pp->participant_message_writer;
  struct pmd_s *pl;
  serdata_t serdata;
  os_int64 tnow = now ();
  int tsec, tusec;
  time_to_sec_usec (&tsec, &tusec, tnow);

  /* It ain't petty: we now use the serializer for PMD, but for that
     we must provide fully typed data, or else the serializer won't be
     able to handle the sequence.  That means we must construct it in
     shared memory, even though it is short-lived, private data. */
  pl = c_new (osplser_topicpmd_type);
  /* Guid prefix: endianness causes troubles, which we can avoid using
     hton_guid_prefix, but the kind we have to do by hand */
  {
    nn_guid_prefix_t gp = nn_hton_guid_prefix (pp->guid.prefix);
    pl->a = gp.u[0];
    pl->b = gp.u[1];
    pl->c = gp.u[2];
  }
  pl->kind = toBE4u (pmd_kind);
  pl->value = c_newSequence ((c_collectionType) osplser_topicpmd_value_type, 1);
  memset (pl->value, 0, c_sequenceSize ((c_sequence) pl->value));
  /* Use serialize_raw to avoid having to also create a v_message */
  serdata = serialize_raw (serpool, osplser_topicpmd, pl, 0, tnow, NULL);
  /* Free it - we only need the serialized form from now on */
  c_free (pl);

  write_sample (NULL, wr, serdata);
}

static int transmit_fragment (struct nn_xpack *xp, struct writer *wr, os_int64 seq, struct serdata *serdata, unsigned fragnum, struct proxy_reader *prd)
{
  /* We always fragment into FRAGMENT_SIZEd fragments, which are near
     the smallest allowed fragment size & can't be bothered (yet) to
     put multiple fragments into one DataFrag submessage if it makes
     sense to send large messages, as it would e.g. on GigE with jumbo
     frames.  If the sample is small enough to fit into one Data
     submessage, we require fragnum = 0 & generate a Data instead of a
     DataFrag.

     Note: fragnum is 0-based here, 1-based in DDSI. But 0-based is
     much easier ...

     Expected inline QoS size: header(4) + statusinfo(8) + keyhash(20)
     + sentinel(4). Plus some spare cos I can't be bothered. */
  const int set_smhdr_flags_asif_data = config.buggy_datafrag_flags_mode;
  const int expected_inline_qos_size = 4+8+20+4 + 32;
  struct nn_xmsg_marker sm_marker;
  struct nn_xmsg *msg;
  void *sm;
  Data_DataFrag_common_t *ddcmn;
  int fragging;
  unsigned fragstart, fraglen;

  if ((msg = nn_xmsg_new (xmsgpool, datasock_mc, &wr->guid.prefix, sizeof (InfoTimestamp_t) + sizeof (DataFrag_t) + expected_inline_qos_size)) == NULL)
    return NN_ERR_OUT_OF_MEMORY;
  if (prd)
    nn_xmsg_setdstPRD (msg, prd);
  else
    nn_xmsg_setdstN (msg, wr->as);

  fragging = (NN_XMSG_MAX_FRAGMENT_SIZE < serdata_size (serdata));
  assert (fragnum * NN_XMSG_MAX_FRAGMENT_SIZE < serdata_size (serdata));

  /* Timestamp only needed once, for the first fragment */
  if (fragnum == 0)
  {
    if (nn_xmsg_add_timestamp (msg, serdata->v.msginfo.timestamp) < 0)
      goto outofmem;
  }

  sm = nn_xmsg_append_aligned (msg, &sm_marker, fragging ? sizeof (DataFrag_t) : sizeof (Data_t), 4);
  if (sm == NULL)
    goto outofmem;
  ddcmn = sm;

  if (!fragging)
  {
    const unsigned contentflag = serdata_is_key (serdata) ? DATA_FLAG_KEYFLAG : DATA_FLAG_DATAFLAG;
    Data_t *data = sm;
    nn_xmsg_submsg_init (msg, sm_marker, SMID_DATA);
    ddcmn->smhdr.flags |= DATA_FLAG_INLINE_QOS | contentflag;

    fragstart = 0;
    fraglen = serdata_size (serdata);
    ddcmn->octetsToInlineQos = (char *) (data+1) - ((char *) &ddcmn->octetsToInlineQos + 2);
  }
  else
  {
    const unsigned contentflag =
      set_smhdr_flags_asif_data
      ? (serdata_is_key (serdata) ? DATA_FLAG_KEYFLAG : DATA_FLAG_DATAFLAG)
      : (serdata_is_key (serdata) ? DATAFRAG_FLAG_KEYFLAG : 0);
    DataFrag_t *frag = sm;
    nn_xmsg_submsg_init (msg, sm_marker, SMID_DATA_FRAG);
    ddcmn->smhdr.flags |= contentflag;
    if (fragnum == 0)
      ddcmn->smhdr.flags |= DATAFRAG_FLAG_INLINE_QOS;

    frag->fragmentStartingNum = fragnum + 1;
    frag->fragmentsInSubmessage = 1;
    frag->fragmentSize = NN_XMSG_MAX_FRAGMENT_SIZE;
    frag->sampleSize = serdata_size (serdata);

    fragstart = fragnum * NN_XMSG_MAX_FRAGMENT_SIZE;
    fraglen = NN_XMSG_MAX_FRAGMENT_SIZE;
    if (fragstart + fraglen > serdata_size (serdata))
      fraglen = serdata_size (serdata) - fragstart;
    ddcmn->octetsToInlineQos = (char *) (frag+1) - ((char *) &ddcmn->octetsToInlineQos + 2);
  }

  ddcmn->extraFlags = 0;
  ddcmn->readerId = nn_hton_entityid (prd ? prd->c.guid.entityid : to_entityid (NN_ENTITYID_UNKNOWN));
  ddcmn->writerId = nn_hton_entityid (wr->guid.entityid);
  ddcmn->writerSN = toSN (seq);

  if (fragnum > 0)
  {
    assert (fragging);
    assert (!(ddcmn->smhdr.flags & DATAFRAG_FLAG_INLINE_QOS));
  }
  else
  {
    nn_prismtech_writer_info_t *wri;
    assert (ddcmn->smhdr.flags & DATA_FLAG_INLINE_QOS);
    if (nn_xmsg_addpar_keyhash (msg, serdata) < 0)
      goto outofmem;
    if (serdata->v.msginfo.statusinfo && nn_xmsg_addpar_statusinfo (msg, serdata->v.msginfo.statusinfo) < 0)
      goto outofmem;
    /* If it's 0 or 1, we know the proper calls have been made */
    assert (serdata->v.msginfo.have_wrinfo == 0 || serdata->v.msginfo.have_wrinfo == 1);
    if (serdata->v.msginfo.have_wrinfo)
    {
      if ((wri = nn_xmsg_addpar (msg, PID_PRISMTECH_WRITER_INFO, sizeof (*wri))) == NULL)
        goto outofmem;
      *wri = serdata->v.msginfo.wrinfo;
    }
    if (nn_xmsg_addpar_sentinel (msg) < 0)
      goto outofmem;
  }

  nn_xmsg_serdata (msg, serdata, fragstart, fraglen);
  nn_xmsg_submsg_setnext (msg, sm_marker);

  if (config.enabled_logcats & LC_TRACE)
  {
    int tsec, tusec;
    time_to_sec_usec (&tsec, &tusec, now ());
    nn_log (LC_TRACE, "%d.%06d queue data%s %x:%x:%x:%x #%lld/%u[%u..%u)\n",
            tsec, tusec, fragging ? "frag" : "", PGUID (wr->guid),
            seq, fragnum+1, fragstart, fragstart + fraglen);
  }

  if (xp)
    return nn_xpack_addmsg (xp, msg);
  else
  {
    qxev_msg (xevents, 0, msg);
    return 0;
  }

 outofmem:
  nn_xmsg_free (msg);
  return NN_ERR_OUT_OF_MEMORY;
}

static void add_HeartbeatFrag (struct nn_xpack *xp, struct writer *wr, os_int64 seq, unsigned fragnum, struct proxy_reader *prd)
{
  struct nn_xmsg_marker sm_marker;
  struct nn_xmsg *msg;
  HeartbeatFrag_t *hbf;
  assert (xp != NULL);
  if ((msg = nn_xmsg_new (xmsgpool, datasock_mc, &wr->guid.prefix, sizeof (HeartbeatFrag_t))) == NULL)
    return; /* ignore out-of-memory: HeartbeatFrag is only advisory anyway */
  if (prd)
    nn_xmsg_setdstPRD (msg, prd);
  else
    nn_xmsg_setdstN (msg, wr->as);
  hbf = nn_xmsg_append_aligned (msg, &sm_marker, sizeof (HeartbeatFrag_t), 4);
  if (hbf == NULL)
  {
    nn_xmsg_free (msg);
    return;
  }
  nn_xmsg_submsg_init (msg, sm_marker, SMID_HEARTBEAT_FRAG);
  hbf->readerId = nn_hton_entityid (prd ? prd->c.guid.entityid : to_entityid (NN_ENTITYID_UNKNOWN));
  hbf->writerId = nn_hton_entityid (wr->guid.entityid);
  hbf->writerSN = toSN (seq);
  hbf->lastFragmentNum = fragnum + 1; /* network format is 1 based */
  hbf->count = ++wr->hbfragcount;
  nn_xmsg_submsg_setnext (msg, sm_marker);
  nn_xpack_addmsg (xp, msg);
}

static int transmit_sample (struct nn_xpack *xp, struct writer *wr, os_int64 seq, serdata_t serdata, struct proxy_reader *prd)
{
  unsigned i, sz, nfrags;
  sz = serdata_size (serdata);
  nfrags = (sz + NN_XMSG_MAX_FRAGMENT_SIZE - 1) / NN_XMSG_MAX_FRAGMENT_SIZE;
  for (i = 0; i < nfrags; i++)
  {
    /* Ignore out-of-memory errors: we can't do anything about it, and
       eventually we'll have to retry.  But if a packet went out and
       we haven't yet completed transmitting a fragmented message, add
       a HeartbeatFrag. */
    if (transmit_fragment (xp, wr, seq, serdata, i, prd) > 0)
    {
      if (nfrags > 1 && i + 1 < nfrags)
        add_HeartbeatFrag (xp, wr, seq, i, prd);
    }
  }

  /* wr->heartbeat_xevent != NULL <=> wr is reliable */
  if (wr->heartbeat_xevent &&
      (wr->whc_seq_size - wr->whc_tlidx_size) > 0 &&
      ((wr->whc_seq_size - wr->whc_tlidx_size) % (config.whc_highwater_mark/4)) == 0)
  {
    if (xp == NULL)
    {
      nn_log (LC_TRACE, "transmit_sample: scheduling heartbeat for %x:%x:%x:%x\n", PGUID (wr->guid));
      resched_xevent_if_earlier (wr->heartbeat_xevent, 0);
    }
    else
    {
      struct nn_xmsg *msg;
      nn_log (LC_TRACE, "transmit_sample: appending heartbeat for %x:%x:%x:%x\n", PGUID (wr->guid));
      if ((msg = nn_xmsg_new (xmsgpool, datasock_mc, &wr->guid.prefix, sizeof (InfoTS_t) + sizeof (Heartbeat_t))) != NULL)
      {
        int hbansreq = 1;
        nn_xmsg_setdstN (msg, wr->as);
        if (add_Heartbeat (msg, wr, &hbansreq, to_entityid (NN_ENTITYID_UNKNOWN)) < 0)
          nn_xmsg_free (msg);
        else
        {
          if (hbansreq)
            wr->t_of_last_heartbeat = now ();
          nn_xpack_addmsg (xp, msg);
          nn_log (LC_TRACE, "transmit_sample: sending msg for %x:%x:%x:%x immediately\n", PGUID (wr->guid));
          nn_xpack_send (xp);
        }
      }
    }
  }
  return 0;
}

static int write_sample (struct nn_xpack *xp, struct writer *wr, serdata_t serdata)
{
  /* DCPS write/dispose/writeDispose/unregister and the with_timestamp
     variants, all folded into a single function (all the message info
     and the payload is encoded in serdata).

     If sending fails (out-of-memory is the only real one), don't
     report an error, as we can always try again. If adding the sample
     to the WHC fails, however, we never accepted the sample and do
     return an error.

     If xp = NULL => queue the events, else simply pack them. */
  int res;
  os_int64 seq;
  seq = ++wr->seq;

  assert (serdata_refcount_is_1 (serdata));
  if (config.enabled_logcats & LC_TRACE)
  {
    char ppbuf[1024];
    int tsec, tusec, res;
    const char *tname = wr->topic ? topic_name (wr->topic) : "(null)";
    const char *ttname = wr->topic ? topic_typename (wr->topic) : "(null)";
    time_to_sec_usec (&tsec, &tusec, now ());
    res = prettyprint_serdata (ppbuf, sizeof (ppbuf), serdata);
    nn_log (LC_TRACE, "%d.%06d write sample %x:%x:%x:%x #%lld: ST%d %s/%s:%s%s\n",
            tsec, tusec, PGUID (wr->guid), seq, serdata->v.msginfo.statusinfo,
            tname, ttname, ppbuf,
            res < (int) sizeof (ppbuf) ? "" : " (trunc)");
  }

  res = add_sample_to_whc (wr, seq, serdata);
  if (res >= 0)
    transmit_sample (xp, wr, seq, serdata, NULL);
  serdata_unref (serdata);
  return res;
}

static int sedp_write_endpoint_endoflife (struct writer *wr, nn_guid_t epguid)
{
  struct nn_xmsg *mpayload;
  unsigned payload_sz;
  char *payload_blob;
  nn_plist_t ps;
  serdata_t serdata;
  serstate_t serstate;
  nn_guid_t kh;
  nn_log (LC_DISCOVERY, "sedp: write end-of-life for %x:%x:%x:%x via %x:%x:%x:%x\n",
          PGUID (epguid), PGUID (wr->guid));

  if ((mpayload = nn_xmsg_new (xmsgpool, 0, &wr->guid.prefix, 0)) == NULL)
    return NN_ERR_OUT_OF_MEMORY;
  nn_plist_init_empty (&ps);
  ps.present |= PP_ENDPOINT_GUID;
  ps.endpoint_guid = epguid;
  if (nn_plist_addtomsg (mpayload, &ps, ~0u, ~0u) < 0 ||
      nn_xmsg_addpar_sentinel (mpayload) < 0)
  {
    nn_xmsg_free (mpayload);
    return NN_ERR_OUT_OF_MEMORY;
  }

  if ((serstate = serstate_new (serpool, NULL)) == NULL)
  {
    nn_xmsg_free (mpayload);
    return NN_ERR_OUT_OF_MEMORY;
  }
  payload_blob = nn_xmsg_payload (&payload_sz, mpayload);
  serstate_append_blob (serstate, 4, payload_sz, payload_blob);
  kh = nn_hton_guid (epguid);
  serstate_set_key (serstate, 1, 16, &kh);
  serstate_set_msginfo (serstate, NN_STATUSINFO_DISPOSE | NN_STATUSINFO_UNREGISTER, now (), NULL);
  serdata = serstate_fix (serstate);
  nn_xmsg_free (mpayload);
  return write_sample (NULL, wr, serdata);
}

static int sedp_write_endpoint (struct writer *wr, nn_guid_t epguid, UNUSED_ARG_NDEBUG (const topic_t eptopic), const nn_xqos_t *xqos)
{
  const nn_xqos_t *defqos = is_writer_entityid (epguid.entityid) ? &default_xqos_wr : &default_xqos_rd;
  const nn_vendorid_t my_vendor_id = MY_VENDOR_ID;
  struct nn_xmsg *mpayload;
  unsigned qosdiff;
  nn_guid_t kh;
  nn_plist_t ps;
  serstate_t serstate;
  serdata_t serdata;
  void *payload_blob;
  unsigned payload_sz;
  assert (eptopic);

  nn_plist_init_empty (&ps);
  ps.present |= PP_PROTOCOL_VERSION | PP_VENDORID | PP_ENDPOINT_GUID;
  ps.protocol_version.major = RTPS_MAJOR;
  ps.protocol_version.minor = RTPS_MINOR;
  ps.vendorid = my_vendor_id;
  ps.endpoint_guid = epguid;

  qosdiff = nn_xqos_delta (xqos, defqos, ~0u);
  if (config.explicitly_publish_qos_set_to_default)
    qosdiff = ~0u;

  /* The message is only a temporary thing, used only for encoding the
     QoS and other settings. So the header fields aren't really
     important, except that they need to be set to reasonable things
     or it'll crash */
  mpayload = nn_xmsg_new (xmsgpool, 0, &wr->guid.prefix, 0);
  nn_plist_addtomsg (mpayload, &ps, PP_PROTOCOL_VERSION | PP_VENDORID | PP_ENDPOINT_GUID, 0);
  nn_xqos_addtomsg (mpayload, xqos, qosdiff);
  nn_xmsg_addpar_sentinel (mpayload);

  /* Then we take the payload from the message and turn it into a
     serdata, and then we can write it as normal data */
  serstate = serstate_new (serpool, NULL);
  payload_blob = nn_xmsg_payload (&payload_sz, mpayload);
  serstate_append_blob (serstate, 4, payload_sz, payload_blob);
  kh = nn_hton_guid (epguid);
  serstate_set_key (serstate, 0, 16, &kh);
  serstate_set_msginfo (serstate, 0, now (), NULL);
  serdata = serstate_fix (serstate);
  nn_xmsg_free (mpayload);

  nn_log (LC_DISCOVERY, "sedp: write for %x:%x:%x:%x via %x:%x:%x:%x\n", PGUID (epguid), PGUID (wr->guid));
  return write_sample (NULL, wr, serdata);
}

static int sedp_write_writer (struct writer *wr, const struct writer *info)
{
  int res;
  assert (!is_builtin_entityid (info->guid.entityid));
  assert (info->topic);
  if (!info->dying)
    res = sedp_write_endpoint (wr, info->guid, info->topic, info->xqos);
  else
    res = sedp_write_endpoint_endoflife (wr, info->guid);
  return res;
}

static int sedp_write_reader (struct writer *wr, const struct reader *info)
{
  int res;
  assert (!is_builtin_entityid (info->guid.entityid));
  assert (info->topic);
  if (!info->dying)
    res = sedp_write_endpoint (wr, info->guid, info->topic, info->xqos);
  else
    res = sedp_write_endpoint_endoflife (wr, info->guid);
  return res;
}

static int writer_must_throttle (const struct writer *wr)
{
  return wr->whc_seq_size - wr->whc_tlidx_size > config.whc_highwater_mark;
}

static int writer_may_continue (const struct writer *wr)
{
  return wr->whc_seq_size - wr->whc_tlidx_size < config.whc_lowwater_mark;
}

static void throttle_writer (struct writer *wr)
{
  int xt = xeventq_must_throttle (xevents);
  int wt = writer_must_throttle (wr);
  while (keepgoing && (xt || wt))
  {
    int tsec, tusec;
    time_to_sec_usec (&tsec, &tusec, now ());
    nn_log (LC_TRACE, "%d.%06d writer %x:%x:%x:%x waiting for whc|evq to shrink below low-water mark (whc %d xevq %d)\n", tsec, tusec, PGUID (wr->guid), wr->whc_seq_size - wr->whc_tlidx_size, xeventq_size (xevents));
    wr->throttling++;
    if (xt && wt)
    {
      while (keepgoing && !(xeventq_may_continue (xevents) && writer_may_continue (wr)))
        os_condWait (&throttle_cond, &lock);
    }
    else if (xt)
    {
      while (keepgoing && !xeventq_may_continue (xevents))
        os_condWait (&throttle_cond, &lock);
    }
    else /* wt */
    {
      while (keepgoing && !writer_may_continue (wr))
        os_condWait (&throttle_cond, &lock);
    }
    wr->throttling--;
    if (wr->deleting && wr->throttling == 0)
      os_condBroadcast (&throttle_cond);

    time_to_sec_usec (&tsec, &tusec, now ());
    nn_log (LC_TRACE, "%d.%06d writer %x:%x:%x:%x done waiting for whc|evq to shrink below low-water mark (whc %d xevq %d)\n", tsec, tusec, PGUID (wr->guid), wr->whc_seq_size - wr->whc_tlidx_size, xeventq_size (xevents));

    xt = xeventq_must_throttle (xevents);
    wt = writer_must_throttle (wr);
  }
}

int rtps_write (struct nn_xpack *xp, struct writer *wr, C_STRUCT (v_message) const *msg)
{
  int r;
  serdata_t serdata;
  os_mutexLock (&lock);
  throttle_writer (wr);
  /* Can't handle all node states ... */
  switch (v_nodeState ((v_message) msg))
  {
    case L_WRITE:
    case L_WRITE | L_DISPOSED:
      serdata = serialize (serpool, wr->topic, msg);
      break;
    case L_DISPOSED:
    case L_UNREGISTER:
      serdata = serialize_key (serpool, wr->topic, msg);
      break;
    default:
      nn_log (LC_WARNING, "rtps_write: unhandled message state: %u\n", (unsigned) v_nodeState ((v_message) msg));
      r = -1;
      goto out;
  }
  if (serdata == NULL)
  {
    nn_log (LC_WARNING, "serialization failed\n");
    r = -1;
    goto out;
  }
#ifndef NDEBUG
  if ((config.enabled_logcats & LC_TRACE) && (v_nodeState ((v_message) msg) & L_WRITE))
    assert (serdata_verify (serdata, msg));
#endif
  /* If out-of-order mode enabled, go through the timed event queue.
     Unfortunately, that does also affect the relative order of
     initial transmission of data and everything else, but so be
     it. */
  r = write_sample (config.xmit_out_of_order ? NULL : xp, wr, serdata);
 out:
  os_mutexUnlock (&lock);
  return r;
}

static int valid_sequence_number_set (const nn_sequence_number_set_t *snset)
{
  if (fromSN (snset->bitmap_base) <= 0)
    return 0;
  if (snset->numbits <= 0 || snset->numbits > 256)
    return 0;
  return 1;
}

static int valid_fragment_number_set (const nn_fragment_number_set_t *fnset)
{
  if (fnset->bitmap_base <= 0)
    return 0;
  if (fnset->numbits <= 0 || fnset->numbits > 256)
    return 0;
  return 1;
}

static int valid_AckNack (AckNack_t *msg, int size, int byteswap)
{
  nn_count_t *count; /* this should've preceded the bitmap */
  if (size < (int) ACKNACK_SIZE (0))
    /* note: sizeof(*msg) is not sufficient verification, but it does
       suffice for verifying all fixed header fields exist */
    return 0;
  if (byteswap)
  {
    bswap_sequence_number_set_hdr (&msg->readerSNState);
    /* bits[], count deferred until validation of fixed part */
  }
  msg->readerId = nn_ntoh_entityid (msg->readerId);
  msg->writerId = nn_ntoh_entityid (msg->writerId);
  /* Validation following 8.3.7.1.3 + 8.3.5.5 */
  if (!valid_sequence_number_set (&msg->readerSNState))
  {
    if (NN_STRICT_P)
      return 0;
    else
    {
      /* RTI generates AckNacks with bitmapBase = 0 and numBits = 0
         (and possibly others that I don't know about) - their
         Wireshark RTPS dissector says that such a message has a
         length-0 bitmap, which is to expected given the way the
         length is computed from numbits */
      if (fromSN (msg->readerSNState.bitmap_base) == 0 &&
          msg->readerSNState.numbits == 0)
        ; /* accept this one known case */
      else if (msg->readerSNState.numbits == 0)
        ; /* maybe RTI, definitely Twinoaks */
      else
        return 0;
    }
  }
  /* Given the number of bits, we can compute the size of the AckNack
     submessage, and verify that the submessage is large enough */
  if (size < (int) ACKNACK_SIZE (msg->readerSNState.numbits))
    return 0;
  count = (nn_count_t *) ((char *) &msg->readerSNState +
                          NN_SEQUENCE_NUMBER_SET_SIZE (msg->readerSNState.numbits));
  if (byteswap)
  {
    bswap_sequence_number_set_bitmap (&msg->readerSNState);
    *count = bswap4 (*count);
  }
  return 1;
}

static int valid_Gap (Gap_t *msg, int size, int byteswap)
{
  if (size < (int) GAP_SIZE (0))
    return 0;
  if (byteswap)
  {
    bswapSN (&msg->gapStart);
    bswap_sequence_number_set_hdr (&msg->gapList);
  }
  msg->readerId = nn_ntoh_entityid (msg->readerId);
  msg->writerId = nn_ntoh_entityid (msg->writerId);
  if (fromSN (msg->gapStart) <= 0)
    return 0;
  if (!valid_sequence_number_set (&msg->gapList))
  {
    if (NN_STRICT_P || msg->gapList.numbits != 0)
      return 0;
  }
  if (size < (int) GAP_SIZE (msg->gapList.numbits))
    return 0;
  if (byteswap)
    bswap_sequence_number_set_bitmap (&msg->gapList);
  return 1;
}

static int valid_InfoDST (InfoDST_t *msg, int size, UNUSED_ARG (int byteswap))
{
  if (size < (int) sizeof (*msg))
    return 0;
  return 1;
}

static int valid_InfoSRC (InfoSRC_t *msg, int size, UNUSED_ARG (int byteswap))
{
  if (size < (int) sizeof (*msg))
    return 0;
  return 1;
}

static int valid_InfoTS (InfoTS_t *msg, int size, int byteswap)
{
  if (!(msg->smhdr.flags & INFOTS_INVALIDATE_FLAG))
  {
    if (size < (int) sizeof (InfoTS_t))
      return 0;
    if (byteswap)
    {
      msg->time.seconds = bswap4 (msg->time.seconds);
      msg->time.fraction = bswap4u (msg->time.fraction);
    }
  }
  return 1;
}

static int valid_Heartbeat (Heartbeat_t *msg, int size, int byteswap)
{
  if (size < (int) sizeof (*msg))
    return 0;
  if (byteswap)
  {
    bswapSN (&msg->firstSN);
    bswapSN (&msg->lastSN);
    msg->count = bswap4 (msg->count);
  }
  msg->readerId = nn_ntoh_entityid (msg->readerId);
  msg->writerId = nn_ntoh_entityid (msg->writerId);
  /* Validation following 8.3.7.5.3 */
  if (fromSN (msg->firstSN) <= 0 ||
      /* fromSN (msg->lastSN) <= 0 || -- implicit in last < first */
      fromSN (msg->lastSN) < fromSN (msg->firstSN))
  {
    if (NN_STRICT_P)
      return 0;
    else
    {
      /* Note that we don't actually know the set of all possible
         malformed messages that we have to process, so we stick to
         the ones we've seen */
      if (fromSN (msg->firstSN) == fromSN (msg->lastSN) + 1)
        ; /* ok */
      else
        return 0;
    }
  }
  return 1;
}

static int valid_HeartbeatFrag (HeartbeatFrag_t *msg, int size, int byteswap)
{
  if (size < (int) sizeof (*msg))
    return 0;
  if (byteswap)
  {
    bswapSN (&msg->writerSN);
    msg->lastFragmentNum = bswap4u (msg->lastFragmentNum);
    msg->count = bswap4 (msg->count);
  }
  msg->readerId = nn_ntoh_entityid (msg->readerId);
  msg->writerId = nn_ntoh_entityid (msg->writerId);
  if (fromSN (msg->writerSN) <= 0 || msg->lastFragmentNum == 0)
    return 0;
  return 1;
}

static int valid_NackFrag (NackFrag_t *msg, int size, int byteswap)
{
  nn_count_t *count; /* this should've preceded the bitmap */
  if (size < (int) NACKFRAG_SIZE (0))
    /* note: sizeof(*msg) is not sufficient verification, but it does
       suffice for verifying all fixed header fields exist */
    return 0;
  if (byteswap)
  {
    bswapSN (&msg->writerSN);
    bswap_fragment_number_set_hdr (&msg->fragmentNumberState);
    /* bits[], count deferred until validation of fixed part */
  }
  msg->readerId = nn_ntoh_entityid (msg->readerId);
  msg->writerId = nn_ntoh_entityid (msg->writerId);
  /* Validation following 8.3.7.1.3 + 8.3.5.5 */
  if (!valid_fragment_number_set (&msg->fragmentNumberState))
    return 0;
  /* Given the number of bits, we can compute the size of the Nackfrag
     submessage, and verify that the submessage is large enough */
  if (size < (int) NACKFRAG_SIZE (msg->fragmentNumberState.numbits))
    return 0;
  count = (nn_count_t *) ((char *) &msg->fragmentNumberState +
                          NN_FRAGMENT_NUMBER_SET_SIZE (msg->fragmentNumberState.numbits));
  if (byteswap)
  {
    bswap_fragment_number_set_bitmap (&msg->fragmentNumberState);
    *count = bswap4 (*count);
  }
  return 1;
}

static int valid_Data (const struct receiver_state *rst, struct nn_rmsg *rmsg, Data_t *msg, int size, int byteswap, struct nn_rsample_info *sampleinfo, char **payloadp)
{
  /* on success: sampleinfo->{rst,statusinfo,pt_wr_info_zoff,bswap,complex_qos} all set */
  char *ptr;

  if (size < (int) sizeof (*msg))
    return 0; /* too small even for fixed fields */
  /* D=1 && K=1 is invalid in this version of the protocol */
  if ((msg->x.smhdr.flags & (DATA_FLAG_DATAFLAG | DATA_FLAG_KEYFLAG)) ==
      (DATA_FLAG_DATAFLAG | DATA_FLAG_KEYFLAG))
    return 0;
  if (byteswap)
  {
    msg->x.extraFlags = bswap2u (msg->x.extraFlags);
    msg->x.octetsToInlineQos = bswap2u (msg->x.octetsToInlineQos);
    bswapSN (&msg->x.writerSN);
  }
  msg->x.readerId = nn_ntoh_entityid (msg->x.readerId);
  msg->x.writerId = nn_ntoh_entityid (msg->x.writerId);

  sampleinfo->rst = (struct receiver_state *) rst; /* drop const */
  sampleinfo->seq = fromSN (msg->x.writerSN);
  sampleinfo->fragsize = 0; /* for unfragmented data, fragsize = 0 works swell */
  sampleinfo->bswap = byteswap;

  if ((msg->x.smhdr.flags & (DATA_FLAG_INLINE_QOS | DATA_FLAG_DATAFLAG | DATA_FLAG_KEYFLAG)) == 0)
  {
    /* no QoS, no payload, so octetsToInlineQos will never be used
       though one would expect octetsToInlineQos and size to be in
       agreement or octetsToInlineQos to be 0 or so */
    *payloadp = NULL;
    sampleinfo->size = 0; /* size is full payload size, no payload & unfragmented => size = 0 */
    sampleinfo->statusinfo = 0;
    sampleinfo->pt_wr_info_zoff = NN_OFF_TO_ZOFF (0);
    sampleinfo->complex_qos = 0;
    return 1;
  }

  /* QoS and/or payload, so octetsToInlineQos must be within the
     msg; since the serialized data and serialized parameter lists
     have a 4 byte header, that one, too must fit */
  if ((int) (offsetof (Data_DataFrag_common_t, octetsToInlineQos) + sizeof (msg->x.octetsToInlineQos) + 4) > size)
    return 0;

  ptr = (char *) msg + offsetof (Data_DataFrag_common_t, octetsToInlineQos) + sizeof (msg->x.octetsToInlineQos) + msg->x.octetsToInlineQos;
  if (msg->x.smhdr.flags & DATA_FLAG_INLINE_QOS)
  {
    nn_plist_src_t src;
    src.protocol_version = rst->protocol_version;
    src.vendorid = rst->vendor;
    src.encoding = (msg->x.smhdr.flags & SMFLAG_ENDIANNESS) ? PL_CDR_LE : PL_CDR_BE;
    src.buf = ptr;
    src.bufsz = (char *) msg + size - src.buf; /* end of message, that's all we know */
    /* just a quick scan, gathering only what we _really_ need */
    if ((ptr = nn_plist_quickscan (sampleinfo, rmsg, &src)) == NULL)
      return 0;
  }
  else
  {
    sampleinfo->statusinfo = 0;
    sampleinfo->pt_wr_info_zoff = NN_OFF_TO_ZOFF (0);
    sampleinfo->complex_qos = 0;
  }

  if (!(msg->x.smhdr.flags & (DATA_FLAG_DATAFLAG | DATA_FLAG_KEYFLAG)))
  {
    /*nn_log (LC_TRACE, "no payload\n");*/
    *payloadp = NULL;
    sampleinfo->size = 0;
  }
  else if ((char *) ptr + 4 - (char *) msg > size)
  {
    /* no space for the header */
    return 0;
  }
  else
  {
    struct CDRHeader *hdr;
    sampleinfo->size = (char *) msg + size - (char *) ptr;
    *payloadp = ptr;
    hdr = (struct CDRHeader *) ptr;
    switch (hdr->identifier)
    {
      case CDR_BE:
      case CDR_LE:
        break;
      case PL_CDR_BE:
      case PL_CDR_LE:
        break;
      default:
        return 0;
    }
  }
  return 1;
}

static int valid_DataFrag (const struct receiver_state *rst, struct nn_rmsg *rmsg, DataFrag_t *msg, int size, int byteswap, struct nn_rsample_info *sampleinfo, char **payloadp)
{
  /* on success: sampleinfo->{rst,statusinfo,pt_wr_info_zoff,bswap,complex_qos} all set */
  const int interpret_smhdr_flags_asif_data = config.buggy_datafrag_flags_mode;
  char *ptr;
  os_uint32 payloadsz;

  if (size < (int) sizeof (*msg))
    return 0; /* too small even for fixed fields */

  if (interpret_smhdr_flags_asif_data)
  {
    /* D=1 && K=1 is invalid in this version of the protocol */
    if ((msg->x.smhdr.flags & (DATA_FLAG_DATAFLAG | DATA_FLAG_KEYFLAG)) ==
        (DATA_FLAG_DATAFLAG | DATA_FLAG_KEYFLAG))
      return 0;
  }

  if (byteswap)
  {
    msg->x.extraFlags = bswap2u (msg->x.extraFlags);
    msg->x.octetsToInlineQos = bswap2u (msg->x.octetsToInlineQos);
    bswapSN (&msg->x.writerSN);
    msg->fragmentStartingNum = bswap4u (msg->fragmentStartingNum);
    msg->fragmentsInSubmessage = bswap2u (msg->fragmentsInSubmessage);
    msg->fragmentSize = bswap2u (msg->fragmentSize);
    msg->sampleSize = bswap4u (msg->sampleSize);
  }
  msg->x.readerId = nn_ntoh_entityid (msg->x.readerId);
  msg->x.writerId = nn_ntoh_entityid (msg->x.writerId);

  if (NN_STRICT_P && msg->fragmentSize < 1024)
    /* spec says fragments must > 1kB; not allowing 1024 bytes is IMHO
       totally ridiculous; and I really don't care how small the
       fragments anyway */
    return 0;
  if (msg->fragmentSize == 0 || msg->fragmentStartingNum == 0 || msg->fragmentsInSubmessage == 0)
    return 0;
  if (NN_STRICT_P && msg->fragmentSize >= msg->sampleSize)
    /* may not fragment if not needed -- but I don't care */
    return 0;
  if ((msg->fragmentStartingNum + msg->fragmentsInSubmessage - 2) * msg->fragmentSize >= msg->sampleSize)
    /* starting offset of last fragment must be within sample, note:
       fragment numbers are 1-based */
    return 0;

  sampleinfo->rst = (struct receiver_state *) rst; /* drop const */
  sampleinfo->seq = fromSN (msg->x.writerSN);
  sampleinfo->fragsize = msg->fragmentSize;
  sampleinfo->size = msg->sampleSize;
  sampleinfo->bswap = byteswap;

  if (interpret_smhdr_flags_asif_data)
  {
    if ((msg->x.smhdr.flags & (DATA_FLAG_DATAFLAG | DATA_FLAG_KEYFLAG)) == 0)
      /* may not fragment if not needed => surely _some_ payload must be present! */
      return 0;
  }

  /* QoS and/or payload, so octetsToInlineQos must be within the msg;
     since the serialized data and serialized parameter lists have a 4
     byte header, that one, too must fit */
  if ((int) (offsetof (Data_DataFrag_common_t, octetsToInlineQos) + sizeof (msg->x.octetsToInlineQos) + 4) >= size)
    return 0;

  /* Quick check inline QoS if present, collecting a little bit of
     information on it.  The only way to find the payload offset if
     inline QoSs are present. */
  ptr = (char *) msg + offsetof (Data_DataFrag_common_t, octetsToInlineQos) + sizeof (msg->x.octetsToInlineQos) + msg->x.octetsToInlineQos;
  if (msg->x.smhdr.flags & DATAFRAG_FLAG_INLINE_QOS)
  {
    nn_plist_src_t src;
    src.protocol_version = rst->protocol_version;
    src.vendorid = rst->vendor;
    src.encoding = (msg->x.smhdr.flags & SMFLAG_ENDIANNESS) ? PL_CDR_LE : PL_CDR_BE;
    src.buf = ptr;
    src.bufsz = (char *) msg + size - src.buf; /* end of message, that's all we know */
    /* just a quick scan, gathering only what we _really_ need */
    if ((ptr = nn_plist_quickscan (sampleinfo, rmsg, &src)) == NULL)
      return 0;
  }
  else
  {
    sampleinfo->statusinfo = 0;
    sampleinfo->pt_wr_info_zoff = NN_OFF_TO_ZOFF (0);
    sampleinfo->complex_qos = 0;
  }

  *payloadp = ptr;
  payloadsz = (char *) msg + size - (char *) ptr;
  if ((os_uint32) msg->fragmentsInSubmessage * msg->fragmentSize <= payloadsz)
    ; /* all spec'd fragments fit in payload */
  else if ((os_uint32) (msg->fragmentsInSubmessage - 1) * msg->fragmentSize >= payloadsz)
    return 0; /* I can live with a short final fragment, but _only_ the final one */
  else if ((os_uint32) ((msg->fragmentStartingNum - 1) * msg->fragmentSize + payloadsz >= msg->sampleSize))
    ; /* final fragment is long enough to cover rest of sample */
  else
    return 0;
  if (msg->fragmentStartingNum == 1)
  {
    struct CDRHeader *hdr = (struct CDRHeader *) ptr;
    if ((char *) ptr + 4 - (char *) msg > size)
    {
      /* no space for the header -- technically, allowing small
         fragments would also mean allowing a partial header, but I
         prefer this */
      return 0;
    }
    switch (hdr->identifier)
    {
      case CDR_BE:
      case CDR_LE:
        break;
      case PL_CDR_BE:
      case PL_CDR_LE:
        break;
      default:
        return 0;
    }
  }
  return 1;
}

struct remove_acked_messages_helper_arg {
  os_int64 keepseq;
  int count;
};

static int remove_acked_messages_helper (void *vnode, void *varg)
{
  struct whc_node *node = vnode;
  struct remove_acked_messages_helper_arg *arg = varg;
  if (node->seq >= arg->keepseq)
    return AVLWALK_ABORT;
  else if (node->in_tlidx)
    return AVLWALK_CONTINUE;
  else
  {
    arg->count++;
    return AVLWALK_DELETE | AVLWALK_CONTINUE;
  }
}

static int remove_acked_messages (struct writer *wr)
{
  struct remove_acked_messages_helper_arg arg;
  if (avl_empty (&wr->readers))
    arg.keepseq = MAX_SEQ_NUMBER;
  else
    arg.keepseq = wr->readers.root->min_seq + 1;
  arg.count = 0;
  /* walk is in-order (yay!) */
  avl_walk (&wr->whc_seq, remove_acked_messages_helper, &arg);
  assert (arg.count <= wr->whc_seq_size);
  wr->whc_seq_size -= arg.count;
  assert (wr->whc_tlidx_size <= wr->whc_seq_size);
  /* when transitioning from >= low-water to < low-water, signal
     anyone waiting in throttle_writer() */
  if (wr->throttling)
  {
    int newsize = wr->whc_seq_size - wr->whc_tlidx_size;
    int oldsize = newsize + arg.count;
    if (newsize < config.whc_lowwater_mark && oldsize >= config.whc_lowwater_mark)
      os_condBroadcast (&throttle_cond);
  }
  return arg.count;
}

static os_int64 grow_gap_to_next_seq (const struct writer *wr, os_int64 seq)
{
  struct whc_node *n;
  if ((n = avl_lookup_succeq (&wr->whc_seq, &seq)) == NULL)
    /* wr->seq is last transmitted; gap ends at gapend-1, so +1 */
    return wr->seq + 1;
  else
    return n->seq;
}

static int acknack_is_nack (const AckNack_t *msg)
{
  unsigned x = 0, mask;
  int i;
  if (msg->readerSNState.numbits == 0)
    /* Disallowed by the spec, but RTI appears to require them (and so
       even we generate them) */
    return 0;
  for (i = 0; i < (int) NN_SEQUENCE_NUMBER_SET_BITS_SIZE (msg->readerSNState.numbits) / 4 - 1; i++)
    x |= msg->readerSNState.bits[i];
  if ((msg->readerSNState.numbits % 32) == 0)
    mask = ~0u;
  else
    mask = ~(~0u >> (msg->readerSNState.numbits % 32));
  x |= msg->readerSNState.bits[i] & mask;
  return x != 0;
}

static void force_heartbeat_to_peer (struct whc_readers_node *rn, UNUSED_ARG (os_int64 tsched), int hbansreq)
{
  struct writer *wr = rn->writer;
  struct nn_xmsg *m;
  assert (wr->reliable);
  /* If WHC is empty, we can't generate a valid heartbeat, so we
     postpone it until just before transmitting the next data message
     from this writer (and just after adding that message to the
     WHC). */
  if (!config.respond_to_rti_init_zero_ack_with_invalid_heartbeat)
  {
    if (avl_empty (&wr->whc_seq))
    {
      wr->hb_before_next_msg = 1;
      return;
    }
  }
  /* Send a Heartbeat just to this peer */
  m = nn_xmsg_new (xmsgpool, datasock_mc, &wr->guid.prefix, 0);
  nn_xmsg_setdstPRD (m, rn->proxy_reader);
  add_Heartbeat (m, wr, &hbansreq, rn->proxy_reader->c.guid.entityid);
  nn_log (LC_TRACE, "force_heartbeat_to_peer: queue %x:%x:%x:%x -> %x:%x:%x:%x\n",
          PGUID (wr->guid), PGUID (rn->reader_guid));
  qxev_msg (xevents, 0, m);
}

static int handle_AckNack (struct receiver_state *rst, const AckNack_t *msg, UNUSED_ARG (int size))
{
  struct proxy_reader *prd;
  struct whc_readers_node *rn;
  nn_guid_t src, dst;
  os_int64 seqbase;
  nn_count_t *countp;
  os_int64 gapstart = -1, gapend = -1;
  int gapnumbits = 0;
  unsigned gapbits[256 / 32];
  int whc_was_not_empty;
  int accelerate_rexmit = 0;
  int is_pure_ack;
  int numbits;
  int msgs_sent;
  os_int64 max_seq_in_reply;
  int i;
  int log_lat_estimate = 0;
  memset (gapbits, 0, sizeof (gapbits));
  countp = (nn_count_t *) ((char *) msg + offsetof (AckNack_t, readerSNState) + NN_SEQUENCE_NUMBER_SET_SIZE (msg->readerSNState.numbits));
  src.prefix = rst->source_guid_prefix;
  src.entityid = msg->readerId;
  dst.prefix = rst->dest_guid_prefix;
  dst.entityid = msg->writerId;
  nn_log (LC_TRACE, "ACKNACK(#%d:%lld/%d:", *countp, fromSN (msg->readerSNState.bitmap_base), msg->readerSNState.numbits);
  for (i = 0; i < (int) msg->readerSNState.numbits; i++)
    nn_log (LC_TRACE, "%c", nn_bitset_isset (msg->readerSNState.numbits, msg->readerSNState.bits, i) ? '1' : '0');

  os_mutexLock (&lock);
  if ((prd = avl_lookup (&proxyrdtree, &src, NULL)) == NULL)
  {
    nn_log (LC_TRACE, " %x:%x:%x:%x? -> %x:%x:%x:%x)", PGUID (src), PGUID (dst));
    goto out;
  }
  if ((rn = avl_lookup (&prd->writers, &dst, NULL)) == NULL)
  {
    nn_log (LC_TRACE, " %x:%x:%x:%x -> %x:%x:%x:%x?)", PGUID (src), PGUID (dst));
    chk_no_such_writer (dst);
    goto out;
  }
  if (!rn->writer->reliable)
  {
    nn_log (LC_TRACE, " %x:%x:%x:%x -> %x:%x:%x:%x not a reliable writer!)", PGUID (src), PGUID (dst));
    goto out;
  }
  whc_was_not_empty = !avl_empty (&rn->writer->whc_seq);
  if (*countp <= rn->last_acknack)
  {
    nn_log (LC_TRACE, " [%x:%x:%x:%x -> %x:%x:%x:%x])", PGUID (src), PGUID (dst));
    goto out;
  }
  rn->last_acknack = *countp;
  if (config.meas_hb_to_ack_latency && valid_ddsi_timestamp (rst->timestamp))
  {
    os_int64 tstamp = now ();
    nn_lat_estim_update (&rn->hb_to_ack_latency, tstamp - nn_from_ddsi_time (rst->timestamp));
    if (tstamp > rn->hb_to_ack_latency_tlastlog + 10 * T_SECOND)
    {
      log_lat_estimate = 1;
      rn->hb_to_ack_latency_tlastlog = tstamp;
    }
  }
  seqbase = fromSN (msg->readerSNState.bitmap_base);
  nn_log (LC_TRACE, " %x:%x:%x:%x -> %x:%x:%x:%x", PGUID (src), PGUID (dst));
  if (seqbase - 1 > rn->seq)
  {
    os_int64 n_ack = (seqbase - 1) - rn->seq;
    int n;
    rn->seq = seqbase - 1;
    avl_augment_update (&rn->writer->readers, rn);
    n = remove_acked_messages (rn->writer);
    nn_log (LC_TRACE, " ACK%lld RM%d", n_ack, n);
  }
  numbits = (int) msg->readerSNState.numbits;
  is_pure_ack = !acknack_is_nack (msg);
  msgs_sent = 0;
  max_seq_in_reply = 0;
  if (seqbase == 0 && numbits == 0)
  {
    nn_log (LC_TRACE, " RTI-zero-ack");
    if (avl_empty (&rn->writer->whc_seq))
    {
      nn_log (LC_TRACE, " whc empty");
      force_heartbeat_to_peer (rn, -2, 0);
    }
    else
    {
      nn_log (LC_TRACE, " rebase");
      force_heartbeat_to_peer (rn, -2, 0);
      numbits = config.accelerate_rexmit_block_size;
      seqbase = rn->writer->whc_seq.root->minseq;
    }
  }
  else if (!rn->pure_ack_received)
  {
    if (is_pure_ack)
    {
      nn_log (LC_TRACE, " happy now");
      rn->pure_ack_received = 1;
    }
    else if ((int) msg->readerSNState.numbits < config.accelerate_rexmit_block_size)
    {
      nn_log (LC_TRACE, " accelerating");
      accelerate_rexmit = 1;
      if (accelerate_rexmit && numbits < config.accelerate_rexmit_block_size)
        numbits = config.accelerate_rexmit_block_size;
    }
    else
    {
      nn_log (LC_TRACE, " complying");
    }
  }
  for (i = 0; i < numbits && seqbase + i <= rn->writer->seq; i++)
  {
    /* Accelerated schedule may run ahead of sequence number set
       contained in the acknack, and assumes all messages beyond the
       set are NAK'd -- don't feel like tracking where exactly we left
       off ... */
    if (i >= (int) msg->readerSNState.numbits || nn_bitset_isset (numbits, msg->readerSNState.bits, i))
    {
      os_int64 seq = seqbase + i;
      struct whc_node *whcn;
      if ((whcn = avl_lookup (&rn->writer->whc_seq, &seq, NULL)) != NULL)
      {
        nn_log (LC_TRACE, " RX%lld", seqbase + i);
        transmit_sample (NULL, rn->writer, seq, whcn->serdata, rn->proxy_reader);
        max_seq_in_reply = seqbase + i;
        msgs_sent++;
      }
      else if (gapstart == -1)
      {
        nn_log (LC_TRACE, " M%lld", seqbase + i);
        gapstart = seqbase + i;
        gapend = gapstart + 1;
      }
      else if (seqbase + i == gapend)
      {
        nn_log (LC_TRACE, " M%lld", seqbase + i);
        gapend = seqbase + i + 1;
      }
      else if (seqbase + i - gapend < 256)
      {
        int idx = (int) (seqbase + i - gapend);
        nn_log (LC_TRACE, " M%lld", seqbase + i);
        gapnumbits = idx + 1;
        nn_bitset_set (gapnumbits, gapbits, idx);
      }
    }
  }
  if (gapstart > 0)
  {
    struct nn_xmsg *m;
    if (gapend == seqbase + msg->readerSNState.numbits)
    {
      gapend = grow_gap_to_next_seq (rn->writer, gapend);
    }
    if (gapnumbits == 0)
    {
      gapnumbits = 1;
      nn_bitset_set (gapnumbits, gapbits, 0);
      gapend--;
    }
    /* The non-bitmap part of a gap message says everything <=
       gapend-1 is no more (so the maximum sequence number it informs
       the peer of is gapend-1); each bit adds one sequence number to
       that. */
    if (gapend-1 + gapnumbits > max_seq_in_reply)
      max_seq_in_reply = gapend-1 + gapnumbits;
    nn_log (LC_TRACE, " XGAP%lld..%lld/%d:", gapstart, gapend, gapnumbits);
    for (i = 0; i < gapnumbits; i++)
      nn_log (LC_TRACE, "%c", nn_bitset_isset (gapnumbits, gapbits, i) ? '1' : '0');
    m = nn_xmsg_new (xmsgpool, datasock_mc, &rn->writer_guid.prefix, 0);
    nn_xmsg_setdstPRD (m, rn->proxy_reader);
    add_Gap (m, rn, gapstart, gapend, gapnumbits, gapbits);
    qxev_msg (xevents, 0, m);
    msgs_sent++;
  }
  /* If rexmits and/or a gap message were sent, and if the last
     sequence number that we're informing the Nack'ing peer about is
     less than the last sequence number transmitted by the writer,
     tell the peer to acknowledge quickly. Not sure if that helps, but
     it might ... [NB writer->seq is the last msg sent so far] */
  if (msgs_sent && max_seq_in_reply < rn->writer->seq)
  {
    nn_log (LC_TRACE, " HEARTBEAT(#sent:%d maxseq:%lld<%lld)", msgs_sent, max_seq_in_reply, rn->writer->seq);
    force_heartbeat_to_peer (rn, 0, 1);
  }

  if (log_lat_estimate)
  {
    logcat_t cat = (config.enabled_logcats & LC_TRACE) ? LC_TRACE : LC_INFO;
    char tag[4*9];
    LOGBUF_DECLNEW (lb);
    snprintf (tag, sizeof (tag), "%x:%x:%x:%x", PGUID (src));
    if (nn_lat_estim_log (lb, cat, tag, &rn->hb_to_ack_latency))
    {
      if (cat != LC_TRACE)
        nn_logb (lb, cat, "\n");
      nn_logb_flush (lb);
    }
    LOGBUF_FREE (lb);
  }
  nn_log (LC_TRACE, ")");
 out:
  os_mutexUnlock (&lock);
  return 1;
}

struct handle_Heartbeat_helper_arg {
  struct receiver_state *rst;
  const Heartbeat_t *msg;
  nn_guid_t src, dst;
  struct proxy_writer *pwr;
  os_int64 firstseq;
};

struct nn_dqueue *dqueue_for_proxy_writer (struct proxy_writer *pwr)
{
  return is_builtin_entityid (pwr->c.guid.entityid) ? builtins_dqueue : user_dqueue;
}

static void handle_forall_destinations (const nn_guid_t *src, const nn_guid_t *dst, struct proxy_writer *pwr, avlwalk_fun_t fun, void *arg, int print_addrs)
{
  /* prefix:  id:   to:
     0        0     all matched readers
     0        !=0   all matched readers with entityid id
     !=0      0     to all matched readers in addressed participant
     !=0      !=0   to the one addressed reader
  */
  const int haveprefix =
    !(dst->prefix.u[0] == 0 && dst->prefix.u[1] == 0 && dst->prefix.u[2] == 0);
  const int haveid = !(dst->entityid.u == NN_ENTITYID_UNKNOWN);
  switch ((haveprefix << 1) | haveid)
  {
    case (0 << 1) | 0: /* all: full treewalk */
      if (print_addrs) nn_log (LC_TRACE, "%x:%x:%x:%x -> %x:%x:%x:%x:", PGUID (*src), PGUID (*dst));
      avl_walk (&pwr->readers, fun, arg);
      break;
    case (0 << 1) | 1: /* all with correct entityid: special filtering treewalk */
      {
        struct rhc_writers_node *wn;
        if (print_addrs) nn_log (LC_TRACE, "%x:%x:%x:%x -> %x:%x:%x:%x:", PGUID (*src), PGUID (*dst));
        wn = avl_findmin (&pwr->readers);
        while (wn)
        {
          if (wn->reader_guid.entityid.u == dst->entityid.u)
            fun (wn, arg);
          wn = avl_findsucc (&pwr->readers, wn);
        }
      }
      break;
    case (1 << 1) | 0: /* all within one participant: walk a range of keyvalues */
      {
        nn_guid_t a, b;
        if (print_addrs) nn_log (LC_TRACE, "%x:%x:%x:%x -> %x:%x:%x:%x:", PGUID (*src), PGUID (*dst));
        a = *dst; a.entityid.u = 0;
        b = *dst; b.entityid.u = ~0;
        avl_walkrange (&pwr->readers, &a, &b, fun, arg);
      }
      break;
    case (1 << 1) | 1: /* fully addressed: dst should exist (but for removal) */
      {
        struct rhc_writers_node *wn;
        if ((wn = avl_lookup (&pwr->readers, dst, NULL)) != NULL)
        {
          if (print_addrs) nn_log (LC_TRACE, "%x:%x:%x:%x -> %x:%x:%x:%x:", PGUID (*src), PGUID (*dst));
          fun (wn, arg);
        }
        else
        {
          if (print_addrs) nn_log (LC_TRACE, "%x:%x:%x:%x -> %x:%x:%x:%x?", PGUID (*src), PGUID (*dst));
          chk_no_such_reader (*dst);
        }
      }
      break;
    default:
      abort ();
  }
}

static int handle_Heartbeat_helper (void *vnode, void *varg)
{
  struct rhc_writers_node *wn = vnode;
  struct handle_Heartbeat_helper_arg *arg = varg;
  os_int64 refseq;

  /* Not supposed to respond to repeats and old heartbeats */
  if (arg->msg->count <= wn->last_heartbeat)
  {
    nn_log (LC_TRACE, " (%x:%x:%x:%x)", PGUID (wn->reader->guid));
    return 1;
  }
  wn->last_heartbeat = arg->msg->count;

  /* Reference sequence number for determining whether or not to
     Ack/Nack unfortunately depends on whether the reader is in
     sync. */
  if (wn->in_sync)
  {
    refseq = nn_reorder_next_seq (wn->proxy_writer->reorder) - 1;
    nn_log (LC_TRACE, " %x:%x:%x:%x@%lld(sync)", PGUID (wn->reader->guid), refseq);
  }
  else
  {
    refseq = nn_reorder_next_seq (wn->u.not_in_sync.reorder) - 1;
    nn_log (LC_TRACE, " %x:%x:%x:%x@%lld", PGUID (wn->reader->guid), refseq);
  }

  /* Reschedule AckNack transmit if deemed appropriate; unreliable
     readers have acknack_xevent == NULL and can't do this.  We don't
     really need to send a nack from each reader that is in sync --
     indeed, we could simply ignore the destination address in the
     messages we receive and only ever nack each sequence number once,
     regardless of which readers care about it. */
  if (wn->acknack_xevent)
  {
    if (wn->proxy_writer->last_seq > refseq || !(arg->msg->smhdr.flags & HEARTBEAT_FLAG_FINAL))
    {
      if (wn->proxy_writer->last_seq > refseq)
        nn_log (LC_TRACE, "/NAK");
      if (resched_xevent_if_earlier (wn->acknack_xevent, 0))
      {
        if (config.meas_hb_to_ack_latency && valid_ddsi_timestamp (arg->rst->timestamp))
          wn->hb_timestamp = nn_from_ddsi_time (arg->rst->timestamp);
      }
    }
    else
    {
      resched_xevent_if_earlier (wn->acknack_xevent, now () + 7 * T_SECOND);
    }
  }
  return AVLWALK_CONTINUE;
}

static int handle_Heartbeat (struct receiver_state *rst, const Heartbeat_t *msg, UNUSED_ARG (int size), struct nn_rmsg *rmsg)
{
  /* We cheat: and process the heartbeat for _all_ readers, always, to
     take care of the samples with sequence numbers that become
     deliverable because of the heartbeat.

     A heartbeat that states [a,b] is the smallest interval in which
     the range of available sequence numbers is is interpreted here as
     a gap [1,a).

     See also handle_Gap.  */
  struct handle_Heartbeat_helper_arg arg;
  struct proxy_writer *pwr;
  os_int64 firstseq = fromSN (msg->firstSN);
  arg.rst = rst;
  arg.msg = msg;
  arg.src.prefix = rst->source_guid_prefix;
  arg.src.entityid = msg->writerId;
  arg.dst.prefix = rst->dest_guid_prefix;
  arg.dst.entityid = msg->readerId;
  arg.firstseq = firstseq;
  nn_log (LC_TRACE, "HEARTBEAT(%s#%d:%lld..%lld ", msg->smhdr.flags & HEARTBEAT_FLAG_FINAL ? "F" : "", msg->count, arg.firstseq, fromSN (msg->lastSN));

  os_mutexLock (&lock);
  if ((pwr = avl_lookup (&proxywrtree, &arg.src, NULL)) == NULL)
  {
    nn_log (LC_TRACE, "%x:%x:%x:%x? -> %x:%x:%x:%x)", PGUID (arg.src), PGUID (arg.dst));
    goto out;
  }
  arg.pwr = pwr;

  if (fromSN (msg->lastSN) > pwr->last_seq)
    pwr->last_seq = fromSN (msg->lastSN);

  nn_defrag_notegap (pwr->defrag, 1, firstseq);

  {
    struct nn_dqueue *dqueue = dqueue_for_proxy_writer (pwr);
    struct nn_rdata *gap;
    struct rhc_writers_node *wn;
    struct nn_rsample_chain sc;
    int refc_adjust = 0;

    gap = nn_rdata_newgap (rmsg);

    if (nn_reorder_gap (&sc, pwr->reorder, gap, 1, firstseq, &refc_adjust) == NN_REORDER_DELIVER)
    {
      if (pwr->deliver_synchronously)
        deliver_regular_unlocked (&sc);
      else
        nn_dqueue_enqueue (dqueue, &sc);
    }

    for (wn = avl_findmin (&pwr->readers); wn; wn = avl_findsucc (&pwr->readers, wn))
    {
      struct nn_reorder *ro = wn->u.not_in_sync.reorder;
      if (wn->in_sync)
        continue;
      if (nn_reorder_gap (&sc, ro, gap, 1, firstseq, &refc_adjust) == NN_REORDER_DELIVER)
        nn_dqueue_enqueue (dqueue, &sc);
      maybe_set_reader_in_sync (wn);
    }
    nn_fragchain_adjust_refcount (gap, refc_adjust);
  }

  handle_forall_destinations (&arg.src, &arg.dst, pwr, handle_Heartbeat_helper, &arg, 1);
  nn_log (LC_TRACE, ")");

 out:
  os_mutexUnlock (&lock);
  return 1;
}

static int handle_HeartbeatFrag (struct receiver_state *rst, const HeartbeatFrag_t *msg, UNUSED_ARG (int size), UNUSED_ARG (struct nn_rmsg *rmsg))
{
  /* We cheat: and process the heartbeat for _all_ readers, always, to
     take care of the samples with sequence numbers that become
     deliverable because of the heartbeat.

     A heartbeat that states [a,b] is the smallest interval in which
     the range of available sequence numbers is is interpreted here as
     a gap [1,a).

     See also handle_Gap.  */
  os_int64 seq = fromSN (msg->writerSN);
  nn_fragment_number_t fragnum = msg->lastFragmentNum - 1;
  nn_guid_t src, dst;
  struct proxy_writer *pwr;
  nn_locator_udpv4_t addr;

  src.prefix = rst->source_guid_prefix;
  src.entityid = msg->writerId;
  dst.prefix = rst->dest_guid_prefix;
  dst.entityid = msg->readerId;

  os_mutexLock (&lock);
  nn_log (LC_TRACE, "HEARTBEATFRAG(#%d:%lld/[1,%u]", msg->count, seq, fragnum+1);
  if ((pwr = avl_lookup (&proxywrtree, &src, NULL)) == NULL)
    nn_log (LC_TRACE, " %x:%x:%x:%x? -> %x:%x:%x:%x", PGUID (src), PGUID (dst));
  else
  {
    nn_log (LC_TRACE, " %x:%x:%x:%x -> %x:%x:%x:%x", PGUID (src), PGUID (dst));

    /* Defragmenting happens at the proxy writer, readers have nothing
       to do with it.  AckNacks get sent from the timed-event thread,
       and a Heartbeat only schedules an AckNack event, but here we sent
       a response immediately. */
    if (avl_empty (&pwr->readers))
      nn_log (LC_TRACE, " no readers");
    else if (!(addrset_any_uc (pwr->c.as, &addr) ||
               addrset_any_mc (pwr->c.as, &addr)))
      nn_log (LC_TRACE, " no address for writer");
    else
    {
      struct rhc_writers_node *rwn = pwr->readers.root;
      union {
        struct nn_fragment_number_set set;
        char pad[NN_FRAGMENT_NUMBER_SET_SIZE (256)];
      } u;
      int numbits = nn_defrag_nackmap (pwr->defrag, seq, fragnum, &u.set, 256);
      int i;
      if (numbits >= 0)
      {
        /* We use 0-based fragment numbers, but externally have to
           provide 1-based fragment numbers */
        u.set.bitmap_base++;
      }
      else
      {
        /* If we don't know the sample, NACK all advertised fragments
           (or 256 if more than 256 fragments were published already) */
        numbits = fragnum + 1;
        if (numbits > 256)
          numbits = 256;
        u.set.bitmap_base = 1;
        nn_bitset_zero (numbits, u.set.bits);
        for (i = 0; i < numbits; i++)
          nn_bitset_set (numbits, u.set.bits, i);
        u.set.numbits = numbits;
      }
      if (numbits > 0)
      {
        struct nn_xmsg_marker sm_marker;
        struct nn_xmsg *ans;
        NackFrag_t *nf;
        nn_count_t *countp;
        ans = nn_xmsg_new (xmsgpool, datasock_mc, &rwn->reader_guid.prefix, 0);
        nn_xmsg_setdst1 (ans, &pwr->c.guid.prefix, &addr);
        if ((nf = nn_xmsg_append_aligned (ans, &sm_marker, NACKFRAG_SIZE (numbits), 4)) == NULL)
          return NN_ERR_OUT_OF_MEMORY;
        nn_xmsg_submsg_init (ans, sm_marker, SMID_NACK_FRAG);
        nf->readerId = nn_hton_entityid (rwn->reader_guid.entityid);
        nf->writerId = nn_hton_entityid (pwr->c.guid.entityid);
        nf->writerSN = msg->writerSN;
        nf->fragmentNumberState.bitmap_base = u.set.bitmap_base;
        nf->fragmentNumberState.numbits = u.set.numbits;
        memcpy (nf->fragmentNumberState.bits, u.set.bits, NN_FRAGMENT_NUMBER_SET_BITS_SIZE (numbits));
        countp = (nn_count_t *) ((char *) nf + offsetof (NackFrag_t, fragmentNumberState) + NN_FRAGMENT_NUMBER_SET_SIZE (nf->fragmentNumberState.numbits));
        *countp = ++pwr->nackfragcount;

        nn_log (LC_TRACE, " responding with #%d:%lld/%u/%d:", *countp, fromSN (nf->writerSN), nf->fragmentNumberState.bitmap_base, nf->fragmentNumberState.numbits);
        for (i = 0; i < (int) nf->fragmentNumberState.numbits; i++)
          nn_log (LC_TRACE, "%c", nn_bitset_isset (nf->fragmentNumberState.numbits, nf->fragmentNumberState.bits, i) ? '1' : '0');

        nn_xmsg_submsg_setnext (ans, sm_marker);
        qxev_msg (xevents, 0, ans);
      }
    }
  }

  nn_log (LC_TRACE, ")\n");
  os_mutexUnlock(&lock);
  return 1;
}

static int handle_NackFrag (struct receiver_state *rst, const NackFrag_t *msg, UNUSED_ARG (int size))
{
  struct proxy_reader *prd;
  struct whc_readers_node *rn;
  struct whc_node *whcn;
  nn_guid_t src, dst;
  nn_count_t *countp;
  os_int64 seq = fromSN (msg->writerSN);
  int i;

  countp = (nn_count_t *) ((char *) msg + offsetof (NackFrag_t, fragmentNumberState) + NN_FRAGMENT_NUMBER_SET_SIZE (msg->fragmentNumberState.numbits));
  src.prefix = rst->source_guid_prefix;
  src.entityid = msg->readerId;
  dst.prefix = rst->dest_guid_prefix;
  dst.entityid = msg->writerId;
  nn_log (LC_TRACE, "NACKFRAG(#%d:%lld/%u/%d:", *countp, seq, msg->fragmentNumberState.bitmap_base, msg->fragmentNumberState.numbits);
  for (i = 0; i < (int) msg->fragmentNumberState.numbits; i++)
    nn_log (LC_TRACE, "%c", nn_bitset_isset (msg->fragmentNumberState.numbits, msg->fragmentNumberState.bits, i) ? '1' : '0');

  os_mutexLock (&lock);
  if ((prd = avl_lookup (&proxyrdtree, &src, NULL)) == NULL)
  {
    nn_log (LC_TRACE, " %x:%x:%x:%x? -> %x:%x:%x:%x)", PGUID (src), PGUID (dst));
    goto out;
  }
  if ((rn = avl_lookup (&prd->writers, &dst, NULL)) == NULL)
  {
    nn_log (LC_TRACE, " %x:%x:%x:%x -> %x:%x:%x:%x?)", PGUID (src), PGUID (dst));
    chk_no_such_writer (dst);
    goto out;
  }
  if (!rn->writer->reliable)
  {
    nn_log (LC_TRACE, " %x:%x:%x:%x -> %x:%x:%x:%x not a reliable writer)", PGUID (src), PGUID (dst));
    goto out;
  }
  if (*countp <= rn->last_nackfrag)
  {
    nn_log (LC_TRACE, " [%x:%x:%x:%x -> %x:%x:%x:%x])", PGUID (src), PGUID (dst));
    goto out;
  }
  rn->last_nackfrag = *countp;
  nn_log (LC_TRACE, " %x:%x:%x:%x -> %x:%x:%x:%x", PGUID (src), PGUID (dst));
  if ((whcn = avl_lookup (&rn->writer->whc_seq, &seq, NULL)) == NULL)
  {
    static unsigned zero = 0;
    struct nn_xmsg *m;
    nn_log (LC_TRACE, " msg not available: scheduling Gap\n");
    m = nn_xmsg_new (xmsgpool, datasock_mc, &rn->writer_guid.prefix, 0);
    nn_xmsg_setdstPRD (m, rn->proxy_reader);
    /* length-1 bitmap with the bit clear avoids the illegal case of a
       length-0 bitmap */
    add_Gap (m, rn, seq, seq+1, 1, &zero);
    qxev_msg (xevents, 0, m);
  }
  else
  {
    const unsigned base = msg->fragmentNumberState.bitmap_base - 1;
    nn_log (LC_TRACE, "scheduling requested frags ...\n");
    for (i = 0; i < (int) msg->fragmentNumberState.numbits; i++)
      if (nn_bitset_isset (msg->fragmentNumberState.numbits, msg->fragmentNumberState.bits, i))
        transmit_fragment (NULL, rn->writer, seq, whcn->serdata, base + i, rn->proxy_reader);
  }
  nn_log (LC_TRACE, ")\n");
 out:
  os_mutexUnlock (&lock);
  return 1;
}

static int handle_InfoDST (struct receiver_state *rst, const InfoDST_t *msg, UNUSED_ARG (int size))
{
  rst->dest_guid_prefix = nn_ntoh_guid_prefix (msg->guid_prefix);
  nn_log (LC_TRACE, "INFODST(%x:%x:%x)", rst->dest_guid_prefix.u[0], rst->dest_guid_prefix.u[1], rst->dest_guid_prefix.u[2]);
  return 1;
}

static int handle_InfoSRC (struct receiver_state *rst, const InfoSRC_t *msg, UNUSED_ARG (int size))
{
  rst->source_guid_prefix = nn_ntoh_guid_prefix (msg->guid_prefix);
  rst->protocol_version = msg->version;
  rst->vendor = msg->vendorid;
  nn_log (LC_TRACE, "INFOSRC(%x:%x:%x vendor %d.%d)", rst->source_guid_prefix.u[0], rst->source_guid_prefix.u[1], rst->source_guid_prefix.u[2], rst->vendor.id[0], rst->vendor.id[1]);
  return 1;
}

static int handle_InfoTS (struct receiver_state *rst, const InfoTS_t *msg, UNUSED_ARG (int size))
{
  nn_log (LC_TRACE, "INFOTS(");
  if (msg->smhdr.flags & INFOTS_INVALIDATE_FLAG)
  {
    rst->timestamp = invalid_ddsi_timestamp;
    nn_log (LC_TRACE, "invalidate");
  }
  else
  {
    rst->timestamp = msg->time;
    nn_log (LC_TRACE, "%d.%09d", rst->timestamp.seconds, (int) (rst->timestamp.fraction / 4.294967296));
  }
  nn_log (LC_TRACE, ")");
  return 1;
}

static void handle_one_gap (struct proxy_writer *pwr, struct rhc_writers_node *wn, os_int64 a, os_int64 b, struct nn_rdata *gap, int *refc_adjust)
{
  struct nn_dqueue *dqueue = dqueue_for_proxy_writer (pwr);
  struct nn_rsample_chain sc;

  nn_defrag_notegap (pwr->defrag, a, b);

  if (nn_reorder_gap (&sc, pwr->reorder, gap, a, b, refc_adjust) == NN_REORDER_DELIVER)
  {
    if (pwr->deliver_synchronously)
      deliver_regular_unlocked (&sc);
    else
      nn_dqueue_enqueue (dqueue, &sc);
  }

  /* Out-of-sync readers never deal with samples with a sequence
     number beyond end_of_tl_seq -- and so it needn't be bothered
     with gaps that start beyond that number */
  if (!wn->in_sync && a <= wn->u.not_in_sync.end_of_tl_seq)
  {
    if (nn_reorder_gap (&sc, wn->u.not_in_sync.reorder, gap, a, b, refc_adjust) == NN_REORDER_DELIVER)
      nn_dqueue_enqueue (dqueue, &sc);

    /* Upon receipt of data a reader can only become in-sync if there
       is something to deliver; for missing data, you just don't know.
       The return value of reorder_gap _is_ sufficiently precise, but
       why not simply check?  It isn't a very expensive test. */
    maybe_set_reader_in_sync (wn);
  }
}

static int handle_Gap (struct receiver_state *rst, const Gap_t *msg, UNUSED_ARG (int size), struct nn_rmsg *rmsg)
{
  /* Option 1: Process the Gap for the proxy writer and all
     out-of-sync readers: what do I care which reader is being
     addressed?  Either the sample can still be reproduced by the
     writer, or it can't be anymore.

     Option 2: Process the Gap for the proxy writer and for the
     addressed reader if it happens to be out-of-sync.

     Obviously, both options differ from the specification, but we
     don't have much choice: there is no way of addressing just a
     single in-sync reader, and if that's impossible than we might as
     well ignore the destination completely.

     Option 1 can be fairly expensive if there are many readers, so we
     do option 2. */

  struct proxy_writer *pwr;
  struct rhc_writers_node *wn;
  nn_guid_t src, dst;
  os_int64 gapstart, listbase, last_included_rel;
  int i;
  src.prefix = rst->source_guid_prefix;
  src.entityid = msg->writerId;
  dst.prefix = rst->dest_guid_prefix;
  dst.entityid = msg->readerId;
  gapstart = fromSN (msg->gapStart);
  listbase = fromSN (msg->gapList.bitmap_base);
  nn_log (LC_TRACE, "GAP(%lld..%lld/%d ", gapstart, listbase, msg->gapList.numbits);

  os_mutexLock (&lock);
  if ((pwr = avl_lookup (&proxywrtree, &src, NULL)) == NULL)
  {
    nn_log (LC_TRACE, "%x:%x:%x:%x? -> %x:%x:%x:%x)", PGUID (src), PGUID (dst));
    goto out;
  }
  if ((wn = avl_lookup (&pwr->readers, &dst, NULL)) == NULL)
  {
    nn_log (LC_TRACE, "%x:%x:%x:%x -> %x:%x:%x:%x?)", PGUID (src), PGUID (dst));
    chk_no_such_reader (dst);
    goto out;
  }
  nn_log (LC_TRACE, "%x:%x:%x:%x -> %x:%x:%x:%x", PGUID (src), PGUID (dst));

  /* There is no _good_ reason for a writer to start the bitmap with a
     1 bit, but check for it just in case, to reduce the number of
     sequence number gaps to be processed. */
  for (i = 0; i < (int) msg->gapList.numbits; i++)
    if (!nn_bitset_isset (msg->gapList.numbits, msg->gapList.bits, i))
      break;
  last_included_rel = i - 1;

  /* Notify reordering in proxy writer & and the addressed reader (if
     it is out-of-sync, &c.), while delivering samples that become
     available because preceding ones are now known to be missing. */
  {
    int refc_adjust = 0;
    struct nn_rdata *gap;
    gap = nn_rdata_newgap (rmsg);
    handle_one_gap (pwr, wn, gapstart, listbase + i, gap, &refc_adjust);
    while (i < (int) msg->gapList.numbits)
    {
      if (!nn_bitset_isset (msg->gapList.numbits, msg->gapList.bits, i))
        i++;
      else
      {
        int j;
        for (j = i+1; j < (int) msg->gapList.numbits; j++)
          if (!nn_bitset_isset (msg->gapList.numbits, msg->gapList.bits, j))
            break;
        handle_one_gap (pwr, wn, listbase + i, listbase + j, gap, &refc_adjust);
        last_included_rel = j - 1;
        i = j;
      }
    }
    nn_fragchain_adjust_refcount (gap, refc_adjust);
  }

  /* If the last sequence number explicitly included in the set is
     beyond the last sequence number we know exists, update the
     latter.  Note that a sequence number _not_ included in the set
     doesn't tell us anything.  */
  if (listbase + last_included_rel > pwr->last_seq)
    pwr->last_seq = listbase + last_included_rel;
  nn_log (LC_TRACE, ")");

 out:
  os_mutexUnlock (&lock);
  return 1;
}

int nn_loc_to_loc_udpv4 (nn_locator_udpv4_t *dst, const nn_locator_t *src)
{
  if (src->kind != NN_LOCATOR_KIND_UDPv4)
    return 0;
  else
  {
    memcpy (&dst->address, src->address + 12, 4);
    dst->port = src->port;
    return 1;
  }
}

static void handle_PMD (UNUSED_ARG (const struct receiver_state *rst), unsigned statusinfo, const void *vdata, int len)
{
  const struct CDRHeader *data = vdata; /* built-ins not deserialized (yet) */
  const int bswap = (data->identifier == CDR_LE) ^ PLATFORM_IS_LITTLE_ENDIAN;
  struct proxy_participant *pp;
  nn_guid_t ppguid;
  nn_log (LC_TRACE, " PMD ST%x", statusinfo);
  if (data->identifier != CDR_LE && data->identifier != CDR_BE)
  {
    nn_log (LC_TRACE, " PMD data->identifier %d !?\n", ntohs (data->identifier));
    return;
  }
  switch (statusinfo & (NN_STATUSINFO_DISPOSE | NN_STATUSINFO_UNREGISTER))
  {
    case 0:
      if (offsetof (ParticipantMessageData_t, value) > len - sizeof (struct CDRHeader))
        debug_print_rawdata (" SHORT1", data, len);
      else
      {
        const ParticipantMessageData_t *pmd = (ParticipantMessageData_t *) (data + 1);
        nn_guid_prefix_t p = nn_ntoh_guid_prefix (pmd->participantGuidPrefix);
        unsigned kind = ntohl (pmd->kind);
        unsigned length = bswap ? bswap4u (pmd->length) : pmd->length;
        nn_log (LC_TRACE, " pp %x:%x:%x kind %u data %d", p.u[0], p.u[1], p.u[2], kind, length);
        if (len - sizeof (struct CDRHeader) - offsetof (ParticipantMessageData_t, value) < length)
          debug_print_rawdata (" SHORT2", pmd->value, len - sizeof (struct CDRHeader) - offsetof (ParticipantMessageData_t, value));
        else
          debug_print_rawdata ("", pmd->value, length);
        ppguid.prefix = p;
        ppguid.entityid.u = NN_ENTITYID_PARTICIPANT;
        if ((pp = avl_lookup (&proxypptree, &ppguid, NULL)) == NULL)
          nn_log (LC_TRACE, " PPunknown");
        else
          assert_pp_and_all_ep_liveliness (pp);
      }
      break;

    case NN_STATUSINFO_DISPOSE:
    case NN_STATUSINFO_UNREGISTER:
    case NN_STATUSINFO_DISPOSE | NN_STATUSINFO_UNREGISTER:
      /* Serialized key; BE or LE doesn't matter as both fields are
         defined as octets.  */
      if (len < (int) (sizeof (struct CDRHeader) + sizeof (nn_guid_prefix_t)))
        debug_print_rawdata (" SHORT3", data, len);
      else
      {
        ppguid.prefix = nn_ntoh_guid_prefix (*((nn_guid_prefix_t *) (data + 1)));
        ppguid.entityid.u = NN_ENTITYID_PARTICIPANT;
        if ((pp = avl_lookup (&proxypptree, &ppguid, NULL)) != NULL)
        {
          nn_log (LC_TRACE, " sched_delete");
          pp->tlease_end = 0;
          avl_augment_update (&proxypptree, pp);
        }
      }
      break;

    default:
      assert (0);
      break;
  }
  nn_log (LC_TRACE, "\n");
}

int get_udpv4_locator (nn_locator_udpv4_t *loc, nn_locators_t *locs)
{
  struct nn_locators_one *l;
  nn_locator_udpv4_t first, samenet;
  int first_set = 0, samenet_set = 0;
  /* We only copy first to *loc if first_set is true, but gcc flow
     analysis doesn't figure that out. So initialize first to prevent
     compiler warnings */
  memset (&first, 0, sizeof (first));
  memset (&samenet, 0, sizeof (samenet));
  /* Preferably an (the first) address that matches a network we are
     on; if none does, pick the first. No multicast locator ever will
     match, so the first one will be used. */
  for (l = locs->first; l != NULL; l = l->next)
  {
    nn_locator_udpv4_t tmp;
    int i;
    if (l->loc.kind != NN_LOCATOR_KIND_UDPv4)
      continue;
    nn_loc_to_loc_udpv4 (&tmp, &l->loc);
    if (!first_set)
    {
      first = tmp;
      first_set = 1;
    }
    for (i = 0; i < n_interfaces; i++)
    {
      if ((tmp.address & interfaces[i].netmask.s_addr) ==
          (interfaces[i].addr.s_addr & interfaces[i].netmask.s_addr))
      {
        if (interfaces[i].addr.s_addr == ownip.s_addr)
        {
          /* matches the preferred interface -> the very best situation */
          *loc = tmp;
          return 1;
        }
        else if (!samenet_set)
        {
          /* on a network we're connected to */
          samenet = tmp;
          samenet_set = 1;
        }
      }
    }
  }
  if (samenet_set) {            /* prefer a directly connected network */
    *loc = samenet;
    return 1;
  } else if (first_set) {       /* else IPv4 address we found will have to do */
    *loc = first;
    return 1;
  }
  return 0;
}

static int handle_SEDP_alive (nn_plist_t *datap)
{
#define E(msg, lbl) do { nn_log (LC_TRACE, (msg)); goto lbl; } while (0)
  struct proxy_participant *pp;
  nn_guid_t ppguid;
  nn_xqos_t *xqos;
  int reliable;
  avlparent_t parent;
  struct addrset *as;
  int result = 0;

  assert (datap);

  if (!(datap->present & PP_ENDPOINT_GUID))
    E (" no guid?", done);
  nn_log (LC_TRACE, " %x:%x:%x:%x", PGUID (datap->endpoint_guid));

  ppguid.prefix = datap->endpoint_guid.prefix;
  ppguid.entityid.u = NN_ENTITYID_PARTICIPANT;
  if (avl_lookup (&pptree, &ppguid, NULL))
    E (" local pp?", done);
  if ((pp = avl_lookup (&proxypptree, &ppguid, NULL)) == NULL)
    E (" unknown proxy pp?", done);
  if (is_builtin_entityid (datap->endpoint_guid.entityid))
    E (" built-in", done);
  if (!(datap->qos.present & QP_TOPIC_NAME))
    E (" no topic?", done);
  if (!(datap->qos.present & QP_TYPE_NAME))
    E (" no typename?", done);
  if (!(datap->qos.present & QP_RELIABILITY))
    reliable = 0;
  else
    reliable = (datap->qos.reliability.kind == NN_RELIABLE_RELIABILITY_QOS);
  if (!(datap->qos.present & QP_DURABILITY))
    datap->qos.durability.kind = NN_VOLATILE_DURABILITY_QOS;

  nn_log (LC_TRACE, " %s %s %s: %s%s.%s/%s",
          reliable ? "reliable" : "best-effort",
          durability_to_string (datap->qos.durability.kind),
          is_writer_entityid (datap->endpoint_guid.entityid) ? "writer" : "reader",
          ((!(datap->qos.present & QP_PARTITION) ||
            datap->qos.partition.n == 0 ||
            *datap->qos.partition.strs[0] == '\0')
           ? "(default)" : datap->qos.partition.strs[0]),
          ((datap->qos.present & QP_PARTITION) && datap->qos.partition.n > 1) ? "+" : "",
          datap->qos.topic_name, datap->qos.type_name);

  if (!is_writer_entityid (datap->endpoint_guid.entityid) &&
      (datap->present & PP_EXPECTS_INLINE_QOS) &&
      datap->expects_inline_qos)
  {
    E ("******* AARGH - it expects inline QoS ********\n", done);
  }

  {
    int known;
    if (is_writer_entityid (datap->endpoint_guid.entityid))
      known = (avl_lookup (&proxywrtree, &datap->endpoint_guid, &parent) != NULL);
    else
      known = (avl_lookup (&proxyrdtree, &datap->endpoint_guid, &parent) != NULL);
    if (known)
    {
      result = 1;
      nn_log (LC_TRACE, " known");
      goto done;
    }
  }

  nn_log (LC_TRACE, " NEW");

  /* FIXME: can crash on out of memory (as, xqos) */
  as = new_addrset ();
  {
    nn_locator_udpv4_t loc;
    if ((datap->present & PP_UNICAST_LOCATOR) &&
        get_udpv4_locator (&loc, &datap->unicast_locators))
      add_to_addrset (as, &loc);
    else
      copy_addrset_into_addrset_uc (as, pp->as_default);
    if (config.allowMulticast)
    {
      if ((datap->present & PP_MULTICAST_LOCATOR) &&
          get_udpv4_locator (&loc, &datap->multicast_locators))
        add_to_addrset (as, &loc);
      else
        copy_addrset_into_addrset_mc (as, pp->as_default);
    }
  }
  nn_log_addrset (LC_TRACE, " (as", as);
  nn_log (LC_TRACE, ")");

  xqos = os_malloc (sizeof (*xqos));
  nn_xqos_copy (xqos, &datap->qos);
  if (is_writer_entityid (datap->endpoint_guid.entityid))
  {
    struct proxy_writer *pwr;
    nn_xqos_mergein_missing (xqos, &default_xqos_wr);
    if (config.enabled_logcats & LC_TRACE)
    {
      LOGBUF_DECLNEW (qosbuf);
      nn_logb (qosbuf, LC_TRACE, "QOS={");
      nn_logb_xqos (qosbuf, LC_TRACE, xqos);
      nn_logb (qosbuf, LC_TRACE, "}");
      nn_logb_flush (qosbuf);
      LOGBUF_FREE (qosbuf);
    }
    pwr = new_proxy_writer (datap->endpoint_guid, pp, as, xqos);
    /* cheating a bit: discover it once, match with all locals (and
       automatically bind it to any future new matching locals) */
    dds_attach_proxy_writer (pwr);
  }
  else
  {
    struct proxy_reader *prd;
    nn_xqos_mergein_missing (xqos, &default_xqos_rd);
    if (config.enabled_logcats & LC_TRACE)
    {
      LOGBUF_DECLNEW (qosbuf);
      nn_logb (qosbuf, LC_TRACE, "QOS={");
      nn_logb_xqos (qosbuf, LC_TRACE, xqos);
      nn_logb (qosbuf, LC_TRACE, "}");
      nn_logb_flush (qosbuf);
      LOGBUF_FREE (qosbuf);
    }
    prd = new_proxy_reader (datap->endpoint_guid, pp, as, xqos);
    /* cheating a bit: discover it once, match with all locals (and
       automatically bind it to any future new matching locals) */
    dds_attach_proxy_reader (prd);
  }

  unref_addrset (as);
  result = 1;

 done:
  return result;
#undef E
}

static void handle_SEDP_dead (nn_plist_t *datap)
{
  if (!(datap->present & PP_ENDPOINT_GUID))
  {
    nn_log (LC_TRACE, " no guid?");
    return;
  }

  nn_log (LC_TRACE, " %x:%x:%x:%x", PGUID (datap->endpoint_guid));
  if (is_writer_entityid (datap->endpoint_guid.entityid))
  {
    struct proxy_writer *pwr;
    if ((pwr = avl_lookup (&proxywrtree, &datap->endpoint_guid, NULL)) == NULL)
      nn_log (LC_TRACE, " unknown");
    else
    {
      nn_log (LC_TRACE, " sched_delete");
      pwr->c.tlease_end = 0;
      avl_augment_update (&proxywrtree, pwr);
    }
  }
  else
  {
    struct proxy_reader *prd;
    if ((prd = avl_lookup (&proxyrdtree, &datap->endpoint_guid, NULL)) == NULL)
      nn_log (LC_TRACE, " unknown");
    else
    {
      nn_log (LC_TRACE, " sched_delete");
      prd->c.tlease_end = 0;
      avl_augment_update (&proxyrdtree, prd);
    }
  }
}

static void handle_SEDP (const struct receiver_state *rst, unsigned statusinfo, const void *vdata, int len)
{
  const struct CDRHeader *data = vdata; /* built-ins not deserialized (yet) */
  nn_log (LC_TRACE, " SEDP ST%x", statusinfo);
  if (data == NULL)
  {
    nn_log (LC_TRACE, " no payload?\n");
    return;
  }
  else
  {
    nn_plist_t decoded_data;
    nn_plist_src_t src;
    src.protocol_version = rst->protocol_version;
    src.vendorid = rst->vendor;
    src.encoding = data->identifier;
    src.buf = (char *) data + 4;
    src.bufsz = len - 4;
    if (nn_plist_init_frommsg (&decoded_data, NULL, ~0u, ~0u, &src) < 0)
    {
      nn_log (LC_TRACE, " ");
      nn_log (LC_WARNING, "SEDP(vendor %d.%d): invalid qos/parameters\n", src.vendorid.id[0], src.vendorid.id[1]);
      return;
    }

    switch (statusinfo & (NN_STATUSINFO_DISPOSE | NN_STATUSINFO_UNREGISTER))
    {
      case 0:
        handle_SEDP_alive (&decoded_data);
        break;

      case NN_STATUSINFO_DISPOSE:
      case NN_STATUSINFO_UNREGISTER:
      case (NN_STATUSINFO_DISPOSE | NN_STATUSINFO_UNREGISTER):
        handle_SEDP_dead (&decoded_data);
        break;
    }

    nn_plist_fini (&decoded_data);
    nn_log (LC_TRACE, "\n");
  }
}

static int do_groupwrite (v_group g, void *vmsg)
{
  v_message msg = vmsg;
  nn_log (LC_TRACE, "{%p}", g);
  v_groupWrite (g, msg, NULL, myNetworkId);
  return 0;
}

static struct proxy_writer *proxy_writer_from_dqelem (const struct nn_rsample_info *sampleinfo, const struct nn_rdata *fragchain)
{
#if 0 /* can't safely deref proxy writer unless I make sure it is not
         freed while there are still items pointing to it in the
         delivery queue -- another alternative: near-perfect hashing
         and lookups, that'd probably nearly as fast as doing all the
         work while freeing */
  return sampleinfo->proxy_writer;
#else /* this only guarantees we get _a_ proxy writer, not that it is
         the right one (if a writer disappears and a new one appears
         with the same guid ...) */
  Data_DataFrag_common_t *msg = (Data_DataFrag_common_t *) NN_RMSG_PAYLOADOFF (fragchain->rmsg, NN_RDATA_SUBMSG_OFF (fragchain));
  nn_guid_t src;
  struct proxy_writer *pwr;
  src.prefix = sampleinfo->rst->source_guid_prefix;
  src.entityid = msg->writerId;
  if ((pwr = avl_lookup (&proxywrtree, &src, NULL)) == NULL)
    nn_log (LC_TRACE, "user_dqueue_handler: proxy writer %x:%x:%x:%x no longer exists\n", PGUID (src));
  return pwr;
#endif
}

static unsigned char normalize_data_datafrag_flags (const SubmessageHeader_t *smhdr, int datafrag_as_data)
{
  switch ((SubmessageKind_t) smhdr->submessageId)
  {
    case SMID_DATA:
      return smhdr->flags;
    case SMID_DATA_FRAG:
      if (datafrag_as_data)
        return smhdr->flags;
      else
      {
        unsigned char common = smhdr->flags & DATA_FLAG_INLINE_QOS;
        if (smhdr->flags & DATAFRAG_FLAG_KEYFLAG)
          return common | DATA_FLAG_KEYFLAG;
        else
          return common | DATA_FLAG_DATAFLAG;
      }
    default:
      assert (0);
      return 0;
  }
}

static int builtins_dqueue_handler (const struct nn_rsample_info *sampleinfo, const struct nn_rdata *fragchain, UNUSED_ARG (void *vqarg))
{
  struct proxy_writer *pwr;
  struct {
    struct CDRHeader cdr;
    nn_parameter_t p_endpoint_guid;
    char kh[16];
    nn_parameter_t p_sentinel;
  } keyhash_payload;
  const char *datap = NN_RMSG_PAYLOADOFF (fragchain->rmsg, NN_RDATA_PAYLOAD_OFF (fragchain));
  int datasz = fragchain->maxp1;
  unsigned statusinfo;
  int need_keyhash;
  Data_DataFrag_common_t *msg;
  unsigned char data_smhdr_flags;
  nn_plist_t qos;

  /* no fragments accepted yet; gaps have been filtered out already */
  assert (fragchain->min == 0);
  assert (fragchain->maxp1 == sampleinfo->size);
  assert (fragchain->nextfrag == NULL);

  os_mutexLock (&lock);
  if ((pwr = proxy_writer_from_dqelem (sampleinfo, fragchain)) == NULL)
    goto done;
  assert (is_builtin_entityid (pwr->c.guid.entityid));

  /* Luckily, most of the Data and DataFrag headers are the same - and
     in particular, all that we care about here is the same.  The
     key/data flags of DataFrag are different from those of Data, but
     DDSI2 used to treat them all as if they are data :( so now,
     instead of splitting out all the code, we reformat these flags
     from the submsg to always conform to that of the "Data"
     submessage regardless of the input. */
  msg = (Data_DataFrag_common_t *) NN_RMSG_PAYLOADOFF (fragchain->rmsg, NN_RDATA_SUBMSG_OFF (fragchain));
  data_smhdr_flags = normalize_data_datafrag_flags (&msg->smhdr, config.buggy_datafrag_flags_mode);

  /* If there is no payload, it is either a completely invalid message
     or a dispose/unregister in RTI style. We assume the latter,
     consequently expect to need the keyhash.  Then, if sampleinfo
     says it is a complex qos, or the keyhash is required, extract all
     we need from the inline qos. */
  need_keyhash = (datasz == 0 || (data_smhdr_flags & (DATA_FLAG_KEYFLAG | DATA_FLAG_DATAFLAG)) == 0);
  if (!(sampleinfo->complex_qos || need_keyhash))
  {
    nn_plist_init_empty (&qos);
    statusinfo = sampleinfo->statusinfo;
  }
  else
  {
    nn_plist_src_t src;
    int qos_offset = NN_RDATA_SUBMSG_OFF (fragchain) + offsetof (Data_DataFrag_common_t, octetsToInlineQos) + sizeof (msg->octetsToInlineQos) + msg->octetsToInlineQos;
    src.protocol_version = sampleinfo->rst->protocol_version;
    src.vendorid = sampleinfo->rst->vendor;
    src.encoding = (msg->smhdr.flags & SMFLAG_ENDIANNESS) ? PL_CDR_LE : PL_CDR_BE;
    src.buf = NN_RMSG_PAYLOADOFF (fragchain->rmsg, qos_offset);
    src.bufsz = NN_RDATA_PAYLOAD_OFF (fragchain) - qos_offset;
    if (nn_plist_init_frommsg (&qos, NULL, PP_STATUSINFO | PP_KEYHASH, 0, &src) < 0)
    {
      nn_log (LC_WARNING, "data(builtin, vendor %d.%d): %x:%x:%x:%x #%lld: invalid inline qos\n",
              src.vendorid.id[0], src.vendorid.id[1],
              PGUID (pwr->c.guid), sampleinfo->seq);
      goto done;
    }
    /* Complex qos bit also gets set when statusinfo bits other than
       dispose/unregister are set.  They are not currently defined,
       but this may save us if they do get defined one day. */
    statusinfo = (qos.present & PP_STATUSINFO) ? qos.statusinfo : 0;
  }

  if (avl_empty (&pwr->readers))
  {
    /* wasn't empty when enqueued, but needn't still be */
    goto done;
  }

  /* Built-ins still do their own deserialization. */
  assert (pwr->readers.root->reader->topic == NULL);
  if (statusinfo == 0)
  {
    if (datasz == 0 || !(data_smhdr_flags & DATA_FLAG_DATAFLAG))
    {
      nn_log (LC_WARNING, "data(builtin, vendor %d.%d): %x:%x:%x:%x #%lld: "
              "built-in data but no payload\n",
              sampleinfo->rst->vendor.id[0], sampleinfo->rst->vendor.id[1],
              PGUID (pwr->c.guid), sampleinfo->seq);
      goto done;
    }
  }
  else if (datasz)
  {
    /* Raw data must be full payload for write, just keys for
       dispose and unregister. First has been checked; the second
       hasn't been checked fully yet. */
    if (!(data_smhdr_flags & DATA_FLAG_KEYFLAG))
    {
      nn_log (LC_WARNING, "data(builtin, vendor %d.%d): %x:%x:%x:%x #%lld: "
              "dispose/unregister of built-in data but payload not just key\n",
              sampleinfo->rst->vendor.id[0], sampleinfo->rst->vendor.id[1],
              PGUID (pwr->c.guid), sampleinfo->seq);
      goto done;
    }
  }
  else if ((qos.present & PP_KEYHASH) && !NN_STRICT_P)
  {
    /* For SEDP, fake a parameter list with just a keyhash.  For PMD,
       just use the keyhash directly.  Too hard to fix everything at
       the same time ... */
    if (pwr->c.guid.entityid.u == NN_ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_WRITER)
    {
      datap = qos.keyhash.value;
      datasz = sizeof (qos.keyhash);
    }
    else
    {
      keyhash_payload.cdr.identifier = PLATFORM_IS_LITTLE_ENDIAN ? PL_CDR_LE : PL_CDR_BE;
      keyhash_payload.cdr.options = 0;
      keyhash_payload.p_endpoint_guid.parameterid = PID_ENDPOINT_GUID;
      keyhash_payload.p_endpoint_guid.length = sizeof (nn_keyhash_t);
      memcpy (keyhash_payload.kh, &qos.keyhash, sizeof (qos.keyhash));
      keyhash_payload.p_sentinel.parameterid = PID_SENTINEL;
      keyhash_payload.p_sentinel.length = 0;
      datap = (char *) &keyhash_payload;
      datasz = sizeof (keyhash_payload);
    }
  }
  else
  {
    nn_log (LC_WARNING, "data(builtin, vendor %d.%d): %x:%x:%x:%x #%lld: "
              "dispose/unregister with no content\n",
            sampleinfo->rst->vendor.id[0], sampleinfo->rst->vendor.id[1],
            PGUID (pwr->c.guid), sampleinfo->seq);
    goto done;
  }

  switch (pwr->c.guid.entityid.u)
  {
    case NN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER:
      nn_log (LC_ERROR, "data(builtin, vendor %d.%d): %x:%x:%x:%x #%lld: SPDP should not be here\n",
              sampleinfo->rst->vendor.id[0], sampleinfo->rst->vendor.id[1],
              PGUID (pwr->c.guid), sampleinfo->seq);
      break;
    case NN_ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER:
    case NN_ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER:
      handle_SEDP (sampleinfo->rst, statusinfo, datap, datasz);
      break;
    case NN_ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_WRITER:
      handle_PMD (sampleinfo->rst, statusinfo, datap, datasz);
      break;
    default:
      nn_log (LC_WARNING, "data(builtin, vendor %d.%d): %x:%x:%x:%x #%lld: not handled\n",
              sampleinfo->rst->vendor.id[0], sampleinfo->rst->vendor.id[1],
              PGUID (pwr->c.guid), sampleinfo->seq);
      break;
  }

 done:
  os_mutexUnlock (&lock);
  return 0;
}

static int defragment (char **datap, const struct nn_rdata *fragchain, os_uint32 sz)
{
  if (fragchain->nextfrag == NULL)
  {
    *datap = NN_RMSG_PAYLOADOFF (fragchain->rmsg, NN_RDATA_PAYLOAD_OFF (fragchain));
    return 0;
  }
  else
  {
    char *buf;
    if ((buf = os_malloc (sz)) != NULL)
    {
      os_uint32 off = 0;
      while (fragchain)
      {
        assert (fragchain->min <= off);
        assert (fragchain->maxp1 <= sz);
        if (fragchain->maxp1 > off)
        {
          /* only copy if this fragment adds data */
          const char *payload = NN_RMSG_PAYLOADOFF (fragchain->rmsg, NN_RDATA_PAYLOAD_OFF (fragchain));
          memcpy (buf + off, payload + off - fragchain->min, fragchain->maxp1 - off);
          off = fragchain->maxp1;
        }
        fragchain = fragchain->nextfrag;
      }
    }
    *datap = buf;
    return 1;
  }
}

static void nn_guid_to_ospl_gid (v_gid *gid, const nn_guid_t *guid)
{
  /* Try hard to fake a writer id for OpenSplice based on a GUID. All
     systems I know of have something resembling a host/system id in
     the first 32 bits, so copy that as the system id and copy half of
     an MD5 hash into the remaining 64 bits. Now if only OpenSplice
     would all 96 bits as a key, we'd be doing reasonably well ...

     OpenSplice DDSI2 always copies the id using the
     PRISMTECH_WRITER_INFO parameter. */
  union { os_uint32 u[4]; unsigned char md5[16]; } hash;
  md5_state_t md5st;
  md5_init (&md5st);
  md5_append (&md5st, (unsigned char *) guid, sizeof (*guid));
  md5_finish (&md5st, hash.md5);
  gid->systemId = guid->prefix.u[0];
  gid->localId = hash.u[0];
  gid->serial = hash.u[1];
}

static int user_dqueue_handler_unlocked (const struct nn_rsample_info *sampleinfo, const struct nn_rdata *fragchain)
{
  struct proxy_writer *pwr;
  unsigned statusinfo;
  Data_DataFrag_common_t *msg;
  unsigned char data_smhdr_flags;
  v_message vmsg = NULL;
  nn_plist_t qos;
  topic_t topic = NULL;
  const char *failmsg = NULL;
  int need_keyhash;

  /* Fragments are now handled by copying the message to freshly
     malloced memory ... that'll have to change eventually */
  assert (fragchain->min == 0);

  if ((pwr = proxy_writer_from_dqelem (sampleinfo, fragchain)) == NULL)
    return 0;
  assert (!is_builtin_entityid (pwr->c.guid.entityid));

  /* Luckily, the Data header (up to inline QoS) is a prefix of the
     DataFrag header, so for the fixed-position things that we're
     interested in here, both can be treated as Data submessages. */
  msg = (Data_DataFrag_common_t *) NN_RMSG_PAYLOADOFF (fragchain->rmsg, NN_RDATA_SUBMSG_OFF (fragchain));
  data_smhdr_flags = normalize_data_datafrag_flags (&msg->smhdr, config.buggy_datafrag_flags_mode);

  /* If there is no payload, it is either a completely invalid message
     or a dispose/unregister in RTI style. We assume the latter,
     consequently expect to need the keyhash.  Then, if sampleinfo
     says it is a complex qos, or the keyhash is required, extract all
     we need from the inline qos. */
  need_keyhash = (sampleinfo->size == 0 || (data_smhdr_flags & (DATA_FLAG_KEYFLAG | DATA_FLAG_DATAFLAG)) == 0);
  if (!(sampleinfo->complex_qos || need_keyhash))
  {
    nn_plist_init_empty (&qos);
    statusinfo = sampleinfo->statusinfo;
  }
  else
  {
    nn_plist_src_t src;
    int qos_offset = NN_RDATA_SUBMSG_OFF (fragchain) + offsetof (Data_DataFrag_common_t, octetsToInlineQos) + sizeof (msg->octetsToInlineQos) + msg->octetsToInlineQos;
    src.protocol_version = sampleinfo->rst->protocol_version;
    src.vendorid = sampleinfo->rst->vendor;
    src.encoding = (msg->smhdr.flags & SMFLAG_ENDIANNESS) ? PL_CDR_LE : PL_CDR_BE;
    src.buf = NN_RMSG_PAYLOADOFF (fragchain->rmsg, qos_offset);
    src.bufsz = NN_RDATA_PAYLOAD_OFF (fragchain) - qos_offset;
    if (nn_plist_init_frommsg (&qos, NULL, PP_STATUSINFO | PP_KEYHASH, 0, &src) < 0)
    {
      nn_log (LC_WARNING, "data(application, vendor %d.%d): %x:%x:%x:%x #%lld: invalid inline qos\n",
              src.vendorid.id[0], src.vendorid.id[1], PGUID (pwr->c.guid), sampleinfo->seq);
      return 0;
    }
    /* Complex qos bit also gets set when statusinfo bits other than
       dispose/unregister are set.  They are not currently defined,
       but this may save us if they do get defined one day. */
    statusinfo = (qos.present & PP_STATUSINFO) ? qos.statusinfo : 0;
  }

  if (avl_empty (&pwr->readers))
  {
    /* wasn't empty when enqueued, but needn't still be */
    goto done;
  }
  topic = pwr->readers.root->reader->topic;
  assert (topic != NULL);

  /* Note: deserializing done potentially many times for a historical
     data sample (once per reader that cares about that data).  For
     now, this is accepted as sufficiently abnormal behaviour to not
     worry about it.  */
  if (statusinfo == 0)
  {
    /* normal write */
    char *datap;
    int needs_free;
    if (!(data_smhdr_flags & DATA_FLAG_DATAFLAG) || sampleinfo->size == 0)
    {
      nn_log (LC_WARNING, "data(application, vendor %d.%d): %x:%x:%x:%x #%lld: write without payload\n",
              sampleinfo->rst->vendor.id[0], sampleinfo->rst->vendor.id[1],
              PGUID (pwr->c.guid), sampleinfo->seq);
      goto done;
    }
    failmsg = "data";
    needs_free = defragment (&datap, fragchain, sampleinfo->size);
    vmsg = deserialize (topic, datap, sampleinfo->size);
    if (needs_free) os_free (datap);
  }
  else if (sampleinfo->size)
  {
    /* dispose or unregister with included serialized key or data
       (data is a PrismTech extension), as expected  */
    char *datap;
    int needs_free;
    needs_free = defragment (&datap, fragchain, sampleinfo->size);
    if (data_smhdr_flags & DATA_FLAG_KEYFLAG)
    {
      failmsg = "key";
      vmsg = deserialize_from_key (topic, datap, sampleinfo->size);
    }
    else
    {
      failmsg = "data";
      vmsg = deserialize (topic, datap, sampleinfo->size);
    }
    if (needs_free) os_free (datap);
  }
  else if (data_smhdr_flags & DATA_FLAG_INLINE_QOS)
  {
    /* RTI always tries to make us survive on the keyhash. RTI must
       mend its ways. */
    if (NN_STRICT_P)
      failmsg = "no content";
    else if (!(qos.present & PP_KEYHASH))
      failmsg = "qos present but without keyhash";
    else
    {
      failmsg = "keyhash";
      vmsg = deserialize_from_keyhash (topic, qos.keyhash.value, sizeof (qos.keyhash));
    }
  }
  else
  {
    failmsg = "no content whatsoever";
  }
  if (vmsg == NULL)
  {
    /* No message => error out */
    goto done;
  }

  /* If tracing, print the full contents */
  if (config.enabled_logcats & LC_TRACE)
  {
    char tmp[1024];
    int tmpsize = sizeof (tmp), res;
    if (data_smhdr_flags & DATA_FLAG_DATAFLAG)
    {
      serdata_t qq = serialize (serpool, topic, vmsg);
      res = prettyprint_serdata (tmp, tmpsize, qq);
      serdata_unref (qq);
    }
    else
    {
      serdata_t qq = serialize_key (serpool, topic, vmsg);
      res = prettyprint_serdata (tmp, tmpsize, qq);
      serdata_unref (qq);
    }
    assert (res >= 0);
    nn_log (LC_TRACE, "data(application, vendor %d.%d): %x:%x:%x:%x #%lld: %s/%s:%s%s\n",
            sampleinfo->rst->vendor.id[0], sampleinfo->rst->vendor.id[1],
            PGUID (pwr->c.guid), sampleinfo->seq, topic_name (topic), topic_typename (topic),
            tmp, res < tmpsize ? "" : " (trunc)");
  }

  /* Fill in the non-payload part of the v_message */
  {
    const struct receiver_state *rst = sampleinfo->rst;
    nn_prismtech_writer_info_t wri;
    os_int64 tstamp;
    unsigned stinfo[2];
    int i, nstinfo;
    if (NN_SAMPLEINFO_HAS_WRINFO (sampleinfo))
      nn_plist_extract_wrinfo (&wri, sampleinfo, fragchain);
    else
    {
      nn_guid_t src;
      assert (!vendor_is_prismtech (rst->vendor));
      memset (&wri, 0, sizeof (&wri));
      src.prefix = rst->source_guid_prefix;
      src.entityid = msg->writerId;
      nn_guid_to_ospl_gid (&wri.writerGID, &src);
    }
    tstamp = valid_ddsi_timestamp (rst->timestamp) ? nn_from_ddsi_time (rst->timestamp) : 0;

    /* Let's try to handle a combined dispose/unregister message
       properly for the first time in the history of DDSI2 */
    if (statusinfo == (NN_STATUSINFO_DISPOSE | NN_STATUSINFO_UNREGISTER))
    {
#if 0 /* once I figure out how to copy a v_message */
      nstinfo = 2;
      stinfo[0] = NN_STATUSINFO_DISPOSE;
      stinfo[1] = NN_STATUSINFO_UNREGISTER;
#else
      nstinfo = 1;
      stinfo[0] = NN_STATUSINFO_DISPOSE;
#endif
    }
    else
    {
      nstinfo = 1;
      stinfo[0] = statusinfo;
    }
    for (i = 0; i < nstinfo; i++)
    {
      if (!fill_v_message_qos (pwr, vmsg, &wri, tstamp, sampleinfo->seq, stinfo[i], (data_smhdr_flags & DATA_FLAG_DATAFLAG) != 0))
      {
        nn_log (LC_ERROR, "data(application, vendor %d.%d): %x:%x:%x:%x #%lld: fill_v_message_qos failed\n",
                sampleinfo->rst->vendor.id[0], sampleinfo->rst->vendor.id[1],
                PGUID (pwr->c.guid), sampleinfo->seq);
        c_free (vmsg);
        goto done;
      }
      nn_log (LC_TRACE, " %lld(%p)=>", sampleinfo->seq, (void *) fragchain);
      nn_groupset_foreach (pwr->groups, do_groupwrite, vmsg);
      nn_log (LC_TRACE, "\n");
    }
  }

  c_free (vmsg);

 done:
  nn_plist_fini (&qos);
  if (vmsg == NULL && topic != NULL)
  {
    nn_log (LC_WARNING, "data(application, vendor %d.%d): %x:%x:%x:%x #%lld: deserialization %s/%s failed (%s)\n",
            sampleinfo->rst->vendor.id[0], sampleinfo->rst->vendor.id[1],
            PGUID (pwr->c.guid), sampleinfo->seq,
            topic_name (topic), topic_typename (topic),
            failmsg ? failmsg : "for reasons unknown");
  }
  return 0;
}

static int user_dqueue_handler (const struct nn_rsample_info *sampleinfo, const struct nn_rdata *fragchain, UNUSED_ARG (void *vqarg))
{
  int res;
  os_mutexLock (&lock);
  res = user_dqueue_handler_unlocked (sampleinfo, fragchain);
  os_mutexUnlock (&lock);
  return res;
}

static void deliver_regular_unlocked (struct nn_rsample_chain *sc)
{
  while (sc->first)
  {
    struct nn_rsample_chain_elem *e = sc->first;
    sc->first = e->next;
    user_dqueue_handler_unlocked (e->sampleinfo, e->fragchain);
    nn_fragchain_unref (e->fragchain);
  }
}

static void handle_regular (struct receiver_state *rst, const Data_DataFrag_common_t *msg, UNUSED_ARG (int size), const struct nn_rsample_info *sampleinfo, UNUSED_ARG (char *datap), struct nn_rmsg *rmsg, struct nn_rdata *rdata)
{
  struct proxy_writer *pwr;
  struct nn_rsample *rsample;
  nn_guid_t src, dst;

  src.prefix = rst->source_guid_prefix;
  src.entityid = msg->writerId;
  dst.prefix = rst->dest_guid_prefix;
  dst.entityid = msg->readerId;

  /* We only process it if we know the source; check this early for
     two reasons: for performance (not that we care about that at this
     stage, really) and for looking up the topic definition from a
     local reader associated with this endpoint. */
  if (!is_writer_entityid (msg->writerId))
  {
    nn_log (LC_TRACE, "%x:%x:%x:%x -> %x:%x:%x:%x src not writer", PGUID (src), PGUID (dst));
    return;
  }
  if ((pwr = avl_lookup (&proxywrtree, &src, NULL)) == NULL)
  {
    nn_log (LC_TRACE, "%x:%x:%x:%x? -> %x:%x:%x:%x", PGUID (src), PGUID (dst));
    return;
  }
  if (avl_empty (&pwr->readers))
  {
    nn_log (LC_TRACE, "%x:%x:%x:%x -> %x:%x:%x:%x: no readers", PGUID (src), PGUID (dst));
    return;
  }

  if (config.arrival_of_data_asserts_pp_and_ep_liveliness)
  {
    struct proxy_participant *pp = pwr->c.pp;
    os_int64 tnow = now ();
    pp->tlease_end = add_duration_to_time (tnow, pp->tlease_dur);
    avl_augment_update (&proxypptree, pp);
    assert_pwr_liveliness_based_on_pp (pwr, &tnow);
  }

  /* Track highest sequence number we know of */
  if (sampleinfo->seq > pwr->last_seq)
    pwr->last_seq = sampleinfo->seq;

  if ((rsample = nn_defrag_rsample (pwr->defrag, rdata, sampleinfo)) != NULL)
  {
    int refc_adjust = 0;
    struct nn_rsample_chain sc;
    struct nn_rdata *fragchain = nn_rsample_fragchain (rsample);
    struct nn_dqueue *dqueue = dqueue_for_proxy_writer (pwr);
    switch (nn_reorder_rsample (&sc, pwr->reorder, rsample, &refc_adjust))
    {
      case NN_REORDER_ACCEPT:
        if (have_reliable_subs_proxy (pwr))
          break;
        else
        {
          /* Gap [1, sampleinfo->seq) will force delivery of this
             sample, and not cause the gap to be added to the reorder
             admin. */
          enum nn_reorder_result res;
          int gap_refc_adjust = 0;
          res = nn_reorder_gap (&sc, pwr->reorder, rdata, 1, sampleinfo->seq, &gap_refc_adjust);
          assert (res == NN_REORDER_DELIVER);
          assert (gap_refc_adjust == 0);
          /* FALLS THROUGH */
        }
      case NN_REORDER_DELIVER:
        if (pwr->deliver_synchronously)
          deliver_regular_unlocked (&sc);
        else
          nn_dqueue_enqueue (dqueue, &sc);
        break;
      case NN_REORDER_TOO_OLD: /* try out-of-sync readers */
        {
          struct rhc_writers_node *wn;
          struct nn_rsample *rsample_dup = NULL;
          int reuse_rsample_dup = 0;
          for (wn = avl_findmin (&pwr->readers); wn != NULL; wn = avl_findsucc (&pwr->readers, wn))
          {
            if (wn->in_sync || sampleinfo->seq > wn->u.not_in_sync.end_of_tl_seq)
              continue;
            if (!reuse_rsample_dup)
              rsample_dup = nn_reorder_rsample_dup (rmsg, rsample);
            switch (nn_reorder_rsample (&sc, wn->u.not_in_sync.reorder, rsample_dup, &refc_adjust))
            {
              case NN_REORDER_DELIVER:
                /* note: can't deliver to a reader, only to a group */
                nn_dqueue_enqueue (dqueue, &sc);
                maybe_set_reader_in_sync (wn);
                reuse_rsample_dup = 0;
                break;
              case NN_REORDER_TOO_OLD:
              case NN_REORDER_REJECT:
                reuse_rsample_dup = 1;
                break;
              case NN_REORDER_ACCEPT:
                reuse_rsample_dup = 0;
                break;
            }
          }
        }
        break;
      case NN_REORDER_REJECT:
        break;
    }
    nn_fragchain_adjust_refcount (fragchain, refc_adjust);
  }
}

static int nn_handle_spdp (UNUSED_ARG (struct receiver_state *rst), UNUSED_ARG (const Data_DataFrag_common_t *msg), UNUSED_ARG (int size), const struct nn_rsample_info *sampleinfo, UNUSED_ARG (char *datap), UNUSED_ARG (struct nn_rmsg *rmsg), struct nn_rdata *rdata)
{
  struct nn_rsample *rsample;
  struct nn_rsample_chain sc;
  struct nn_rdata *fragchain;
  int refc_adjust = 0;
  rsample = nn_defrag_rsample (spdp_defrag, rdata, sampleinfo);
  fragchain = nn_rsample_fragchain (rsample);
  if (nn_reorder_rsample (&sc, spdp_reorder, rsample, &refc_adjust) == NN_REORDER_DELIVER)
    nn_dqueue_enqueue (spdp_dqueue, &sc);
  nn_fragchain_adjust_refcount (fragchain, refc_adjust);
  return 0;
}

static int handle_Data (struct receiver_state *rst, const Data_t *msg, int size, struct nn_rsample_info *sampleinfo, char *datap, struct nn_rmsg *rmsg)
{
  struct nn_rdata *rdata;
  int submsg_offset, payload_offset;

  nn_log (LC_TRACE, "DATA(%x:%x:%x:%x -> %x:%x:%x:%x #%lld",
          PGUIDPREFIX (rst->source_guid_prefix), msg->x.writerId.u,
          PGUIDPREFIX (rst->dest_guid_prefix), msg->x.readerId.u,
          fromSN (msg->x.writerSN));

  submsg_offset = (char *) msg - NN_RMSG_PAYLOAD (rmsg);
  if (datap)
    payload_offset = (char *) datap - NN_RMSG_PAYLOAD (rmsg);
  else
    payload_offset = submsg_offset + size;
  rdata = nn_rdata_new (rmsg, 0, sampleinfo->size, submsg_offset, payload_offset);

  os_mutexLock (&lock);
  switch (msg->x.writerId.u)
  {
    case NN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER:
      nn_handle_spdp (rst, &msg->x, size, sampleinfo, datap, rmsg, rdata);
      break;

    default:
      handle_regular (rst, &msg->x, size, sampleinfo, datap, rmsg, rdata);
      break;
  }
  os_mutexUnlock (&lock);

  nn_log (LC_TRACE, ")");
  return 1;
}

static int handle_DataFrag (struct receiver_state *rst, const DataFrag_t *msg, int size, struct nn_rsample_info *sampleinfo, char *datap, struct nn_rmsg *rmsg)
{
  struct nn_rdata *rdata;
  int submsg_offset, payload_offset;
  os_uint32 begin, endp1;

  nn_log (LC_TRACE, "DATAFRAG(%x:%x:%x:%x -> %x:%x:%x:%x #%lld/[%u..%u]",
          PGUIDPREFIX (rst->source_guid_prefix), msg->x.writerId.u,
          PGUIDPREFIX (rst->dest_guid_prefix), msg->x.readerId.u,
          fromSN (msg->x.writerSN),
          msg->fragmentStartingNum, msg->fragmentStartingNum + msg->fragmentsInSubmessage - 1);

  if (is_builtin_entityid (msg->x.writerId))
  {
    nn_log (LC_WARNING, "DATAFRAG(%x:%x:%x:%x #%lld -> %x:%x:%x:%x) - fragmented builtin data not yet supported\n", PGUIDPREFIX (rst->source_guid_prefix), msg->x.writerId.u, fromSN (msg->x.writerSN), PGUIDPREFIX (rst->dest_guid_prefix), msg->x.readerId.u);
    return 1;
  }

  submsg_offset = (char *) msg - NN_RMSG_PAYLOAD (rmsg);
  if (datap)
    payload_offset = (char *) datap - NN_RMSG_PAYLOAD (rmsg);
  else
    payload_offset = submsg_offset + size;

  begin = (unsigned) (msg->fragmentStartingNum - 1) * msg->fragmentSize;
  endp1 = begin + ((char *) msg + size - datap);
  if (endp1 > msg->sampleSize)
    endp1 = msg->sampleSize;

  nn_log (LC_TRACE, "/[%u..%u) of %u", begin, endp1, msg->sampleSize);

  rdata = nn_rdata_new (rmsg, begin, endp1, submsg_offset, payload_offset);
  os_mutexLock (&lock);
  handle_regular (rst, &msg->x, size, sampleinfo, datap, rmsg, rdata);
  os_mutexUnlock (&lock);
  nn_log (LC_TRACE, ")");
  return 1;
}

static void malformed_packet_received (const char *msg, const char *submsg, int len, const char *state, nn_vendorid_t vendorid)
{
  int i;
  LOGBUF_DECLNEW (lb);
  nn_logb (lb, LC_WARNING, "malformed packet received from vendor %d.%d state %s <", vendorid.id[0], vendorid.id[1], state);
  if (submsg)
  {
    for (i = 0; i < 32 && i < len && msg + i < submsg; i++)
    {
      if (isprint ((unsigned char) msg[i]))
        nn_logb (lb, LC_WARNING, "%c", msg[i]);
      else
        nn_logb (lb, LC_WARNING, "\\x%02x", (unsigned char) msg[i]);
    }
    nn_logb (lb, LC_WARNING, " @0x%x ", (int) (submsg - msg));
    for (i = 0; i < 32 && i < len - (int) (submsg - msg); i++)
    {
      if (isprint ((unsigned char) submsg[i]))
        nn_logb (lb, LC_WARNING, "%c", submsg[i]);
      else
        nn_logb (lb, LC_WARNING, "\\x%02x", (unsigned char) submsg[i]);
    }
  }
  else
  {
    for (i = 0; i < 32 && i < len; i++)
    {
      if (isprint ((unsigned char) msg[i]))
        nn_logb (lb, LC_WARNING, "%c", msg[i]);
      else
        nn_logb (lb, LC_WARNING, "\\x%02x", (unsigned char) msg[i]);
    }
  }
  nn_logb (lb, LC_WARNING, "> (note: maybe partially bswap'd)\n");
  nn_logb_flush (lb);
  LOGBUF_FREE (lb);
}

static struct receiver_state *rst_cow_if_needed (int *rst_live, struct nn_rmsg *rmsg, struct receiver_state *rst)
{
  if (! *rst_live)
    return rst;
  else
  {
    struct receiver_state *nrst = nn_rmsg_alloc (rmsg, sizeof (*nrst));
    *nrst = *rst;
    return nrst;
  }
}

static void handle_message (const struct sockaddr_in *src, const nn_guid_prefix_t *dst_prefix, char *msg /* NOT const - we may byteswap it */, int len, struct nn_rmsg *rmsg)
{
  const char *state;
  char *submsg;
  Header_t *hdr = (Header_t *) msg;
  struct receiver_state *rst;
  int rst_live;
  int tsec, tusec;

  if (config.enabled_logcats & LC_TRACE)
  {
    /*nn_log (LC_TRACE, "handle_message\n");*/
    time_to_sec_usec (&tsec, &tusec, now ());
    nn_log (LC_TRACE, "%d.%06d %s:%d => ", tsec, tusec, inet_ntoa (src->sin_addr), ntohs (src->sin_port));
  }
  submsg = NULL;
  state = "header";
  if (len < (int) RTPS_MESSAGE_HEADER_SIZE)
    goto malformed;
  if (hdr->protocol.id[0] != 'R' || hdr->protocol.id[1] != 'T' ||
      hdr->protocol.id[2] != 'P' || hdr->protocol.id[3] != 'S')
    goto malformed;
  if (hdr->version.major != RTPS_MAJOR || hdr->version.minor != RTPS_MINOR)
    goto malformed;

  hdr->guid_prefix = nn_ntoh_guid_prefix (hdr->guid_prefix);

  /* Receiver state is dynamically allocated with lifetime bound to
     the message.  Updates cause a new copy to be created if the
     current one is "live", i.e., possibly referenced by a
     submessage (for now, only Data(Frag)). */
  rst = nn_rmsg_alloc (rmsg, sizeof (*rst));
  memset (rst, 0, sizeof (rst));
  rst->source_guid_prefix = hdr->guid_prefix;
  rst->dest_guid_prefix = *dst_prefix;
  rst->timestamp = invalid_ddsi_timestamp;
  rst->vendor = hdr->vendorid;
  rst->protocol_version = hdr->version;
  rst_live = 0;

  nn_log (LC_TRACE, "HDR(%x:%x:%x vendor %d.%d) len %d\n", hdr->guid_prefix.u[0], hdr->guid_prefix.u[1], hdr->guid_prefix.u[2], rst->vendor.id[0], rst->vendor.id[1], len);

  if (config.coexistWithNativeNetworking && is_own_vendor (rst->vendor))
    return;

  submsg = msg + RTPS_MESSAGE_HEADER_SIZE;
  while (submsg <= msg + len - sizeof (SubmessageHeader_t))
  {
    Submessage_t *sm = (Submessage_t *) submsg;
    int byteswap;
    int octetsToNextHeader;
    int submsg_size;

    state = "parse";

    if (sm->smhdr.flags & SMFLAG_ENDIANNESS)
      byteswap = ! PLATFORM_IS_LITTLE_ENDIAN;
    else
      byteswap = PLATFORM_IS_LITTLE_ENDIAN;
    if (byteswap)
    {
      sm->smhdr.octetsToNextHeader = bswap2u (sm->smhdr.octetsToNextHeader);
    }

    octetsToNextHeader = sm->smhdr.octetsToNextHeader;
    if (octetsToNextHeader != 0)
      submsg_size = RTPS_SUBMESSAGE_HEADER_SIZE + octetsToNextHeader;
    else if (sm->smhdr.submessageId == SMID_PAD ||
             sm->smhdr.submessageId == SMID_INFO_TS)
      submsg_size = RTPS_SUBMESSAGE_HEADER_SIZE;
    else
      submsg_size = msg + len - submsg;
    /*nn_log (LC_TRACE, "submsg_size %d\n", submsg_size);*/

    if (submsg + submsg_size > msg + len)
      break;

    nn_log (LC_TRACE, "  ");
    switch (sm->smhdr.submessageId)
    {
      case SMID_PAD:
        nn_log (LC_TRACE, "PAD");
        break;
      case SMID_ACKNACK:
        state = "parse:acknack";
        if (!valid_AckNack (&sm->acknack, submsg_size, byteswap))
          goto malformed;
        handle_AckNack (rst, &sm->acknack, submsg_size);
        break;
      case SMID_HEARTBEAT:
        state = "parse:heartbeat";
        if (!valid_Heartbeat (&sm->heartbeat, submsg_size, byteswap))
          goto malformed;
        handle_Heartbeat (rst, &sm->heartbeat, submsg_size, rmsg);
        break;
      case SMID_GAP:
        state = "parse:gap";
        /* Gap is handled synchronously in principle, but may
           sometimes have to record a gap in the reorder admin.  The
           first case by definition doesn't need to set "rst_live",
           the second one avoids that because it doesn't require the
           rst after inserting the gap in the admin. */
        if (!valid_Gap (&sm->gap, submsg_size, byteswap))
          goto malformed;
        handle_Gap (rst, &sm->gap, submsg_size, rmsg);
        break;
      case SMID_INFO_TS:
        state = "parse:info_ts";
        if (!valid_InfoTS (&sm->infots, submsg_size, byteswap))
          goto malformed;
        rst = rst_cow_if_needed (&rst_live, rmsg, rst);
        handle_InfoTS (rst, &sm->infots, submsg_size);
        break;
      case SMID_INFO_SRC:
        state = "parse:info_src";
        if (!valid_InfoSRC (&sm->infosrc, submsg_size, byteswap))
          goto malformed;
        rst = rst_cow_if_needed (&rst_live, rmsg, rst);
        handle_InfoSRC (rst, &sm->infosrc, submsg_size);
        break;
      case SMID_INFO_REPLY_IP4:
        state = "parse:info_reply_ip4";
        nn_log (LC_TRACE, "INFO_REPLY_IP4");
        break;
      case SMID_INFO_DST:
        state = "parse:info_dst";
        if (!valid_InfoDST (&sm->infodst, submsg_size, byteswap))
          goto malformed;
        rst = rst_cow_if_needed (&rst_live, rmsg, rst);
        handle_InfoDST (rst, &sm->infodst, submsg_size);
        break;
      case SMID_INFO_REPLY:
        state = "parse:info_reply";
        nn_log (LC_TRACE, "INFO_REPLY");
        break;
      case SMID_NACK_FRAG:
        state = "parse:nackfrag";
        if (!valid_NackFrag (&sm->nackfrag, submsg_size, byteswap))
          goto malformed;
        handle_NackFrag (rst, &sm->nackfrag, submsg_size);
        break;
      case SMID_HEARTBEAT_FRAG:
        state = "parse:heartbeatfrag";
        if (!valid_HeartbeatFrag (&sm->heartbeatfrag, submsg_size, byteswap))
          goto malformed;
        handle_HeartbeatFrag (rst, &sm->heartbeatfrag, submsg_size, rmsg);
        break;
      case SMID_DATA_FRAG:
        state = "parse:datafrag";
        {
          struct nn_rsample_info sampleinfo;
          char *datap;
          /* valid_DataFrag does not validate the payload */
          if (!valid_DataFrag (rst, rmsg, &sm->datafrag, submsg_size, byteswap, &sampleinfo, &datap))
            goto malformed;
          handle_DataFrag (rst, &sm->datafrag, submsg_size, &sampleinfo, datap, rmsg);
          rst_live = 1;
        }
        break;
      case SMID_DATA:
        state = "parse:data";
        {
          struct nn_rsample_info sampleinfo;
          char *datap;
          /* valid_Data does not validate the payload */
          if (!valid_Data (rst, rmsg, &sm->data, submsg_size, byteswap, &sampleinfo, &datap))
            goto malformed;
          handle_Data (rst, &sm->data, submsg_size, &sampleinfo, datap, rmsg);
          rst_live = 1;
        }
        break;
      default:
        state = "parse:undefined";
        nn_log (LC_TRACE, "UNDEFINED(%x)", sm->smhdr.submessageId);
        if (sm->smhdr.submessageId <= 0x7f)
        {
          /* Other submessages in the 0 .. 0x7f range may be added in
             future version of the protocol -- so an undefined code
             for the implemented version of the protocol indicates a
             malformed message. */
          if (rst->protocol_version.major < RTPS_MAJOR ||
              (rst->protocol_version.major == RTPS_MAJOR &&
               rst->protocol_version.minor <= RTPS_MINOR))
            goto malformed;
        }
        else if (is_own_vendor (rst->vendor))
        {
          /* One wouldn't expect undefined stuff from ourselves,
             except that we need to be up- and backwards compatible
             with ourselves, too! */
#if 0
          goto malformed;
#endif
        }
        else
        {
          /* Ignore other vendors' private submessages */
        }
        break;
    }
    submsg += submsg_size;
    nn_log (LC_TRACE, "\n");
  }
  if (submsg != msg + len)
  {
    state = "parse:shortmsg";
    nn_log (LC_TRACE, "short");
    goto malformed;
  }
  return;

 malformed:
  malformed_packet_received (msg, submsg, len, state, hdr->vendorid);
}

static void do_packet (os_socket fd, struct participant *pp, struct nn_rbufpool *rbpool)
{
  const int maxsz = config.rmsg_chunk_size < 65536 ? config.rmsg_chunk_size : 65536;
  static nn_guid_prefix_t nullprefix;
  int sz;
  struct sockaddr_in src;
  os_uint32 srclen = sizeof (src);
  struct nn_rmsg *rmsg;
  struct msghdr msghdr;
  struct iovec msg_iov;

  rmsg = nn_rmsg_new (rbpool);

  msg_iov.iov_base = NN_RMSG_PAYLOAD (rmsg);
  msg_iov.iov_len = maxsz;
  msghdr.msg_name = &src;
  msghdr.msg_namelen = srclen;
  msghdr.msg_iov = &msg_iov;
  msghdr.msg_iovlen = 1;
#if SYSDEPS_MSGHDR_ACCRIGHTS
  msghdr.msg_accrights = NULL;
  msghdr.msg_accrightslen = 0;
#else
  msghdr.msg_control = NULL;
  msghdr.msg_controllen = 0;
#endif
  if ((sz = recvmsg (fd, &msghdr, 0)) < 0)
  {
    int err = os_sockError ();
    if (err != os_sockENOTSOCK && err != os_sockEINTR && err != os_sockEBADF && err != os_sockECONNRESET)
      nn_log (LC_ERROR, "recvfrom sock %d: %d errno %d\n", fd, sz, err);
  }
#if SYSDEPS_MSGHDR_FLAGS
  else if (msghdr.msg_flags & MSG_TRUNC)
  {
    nn_log (LC_WARNING, "%s:%d => truncated\n", inet_ntoa (src.sin_addr), ntohs (src.sin_port));
  }
#endif
  else
  {
    nn_rmsg_setsize (rmsg, sz);
    handle_message (&src, pp ? &pp->guid.prefix : &nullprefix, NN_RMSG_PAYLOAD (rmsg), sz, rmsg);
  }
  nn_rmsg_commit (rmsg);
}

static void *xmit_thread (UNUSED_ARG (void *varg))
{
  const double assumedrate = 10e6; /* 100 Mbit/s */
  double qlen = 0.0;
  os_int64 tlast = now();
  struct nn_xpack *xp;

  xp = nn_xpack_new ();
  os_mutexLock(&lock);
  while (keepgoing)
  {
    os_int64 minwait;
    os_int64 twakeup, tnow;
    os_size_t nbytes;
    double dt;

#if DO_VGLOBALS
    vglobals();
#endif
    nbytes = handle_xevents (xp);

    /* Very simplistic flow control: assume nbytes have been added to
       the send buffer instantaneously while it is being drained at
       assumedrate bytes/s. If qlen > 0, wait at least long enough for
       it to drain, if < 0 clamp it to 0. */
    tnow = now();
    dt = (tnow - tlast) * 1e-9;
    qlen += nbytes - dt * assumedrate;
    tlast = tnow;
    if (qlen >= 0) {
      minwait = (os_int64) (1e9 * qlen / assumedrate);
#if 1
      minwait = 0;
#endif
      nn_log (LC_TRACE, "xmit_thread: qlen %g minwait %gus\n", qlen, minwait * 1e-3);
    } else {
      qlen = 0;
      minwait = 0;
    }
    twakeup = earliest_in_xeventq (xevents);
    if (twakeup == T_NEVER)
      os_condWait(&evcond, &lock);
    else if (twakeup <= tnow)
    {
      os_time to;
      os_mutexUnlock(&lock);
      if (minwait > 0)
      {
        to.tv_sec = (int) (minwait / 1000000000);
        to.tv_nsec = (unsigned) (minwait % 1000000000);
        os_nanoSleep (to);
      }
      os_mutexLock(&lock);
    }
    else if (twakeup > tnow)
    {
      os_time to;
      twakeup -= tnow; /* os_condTimedWait: relative timeout */
      if (minwait < 1 * T_MILLISECOND)
        minwait = 1 * T_MILLISECOND;
      if (twakeup < minwait)
        twakeup = minwait;
      to.tv_sec = (int) (twakeup / 1000000000);
      to.tv_nsec = (unsigned) (twakeup % 1000000000);
      os_condTimedWait (&evcond, &lock, &to);
    }
  }
  os_mutexUnlock(&lock);
  nn_xpack_free (xp);
  return NULL;
}

static void *recv_thread (UNUSED_ARG (void *varg))
{
#define NUM_FIXED 4
  os_socket fixed[NUM_FIXED];
  struct nn_rbufpool *rbpool;
  fixed[0] = discsock_uc;
  fixed[1] = discsock_mc;
  fixed[2] = datasock_uc;
  fixed[3] = datasock_mc;
  rbpool = nn_rbufpool_new (1048576, config.rmsg_chunk_size);
  os_mutexLock (&lock);
  while (keepgoing)
  {
    int select_res, i;
    fd_set fdset, fdset_err;
    os_socket maxsock;
    os_time timeout;
    cleanup_dead_proxies ();

    FD_ZERO (&fdset);
    FD_ZERO (&fdset_err);
    maxsock = 0;
    for (i = 0; i < NUM_FIXED; i++)
    {
      FD_SET (fixed[i], &fdset);
      FD_SET (fixed[i], &fdset_err);
      if (fixed[i] > maxsock)
        maxsock = fixed[i];
    }
    if (config.many_sockets_mode)
    {
      for (i = 0; i < nparticipants; i++)
      {
        os_socket sock = participant_set[i]->sock;
        FD_SET (sock, &fdset);
        if (sock > maxsock)
          maxsock = sock;
      }
      participant_set_changed = 0;
    }

    os_mutexUnlock (&lock);
    timeout.tv_sec = 0;
    timeout.tv_nsec = 100000000; /* 100ms */
    if ((select_res = os_sockSelect (maxsock+1, &fdset, NULL, &fdset_err, &timeout)) == -1)
    {
      int err = os_sockError ();
      if (err != os_sockENOTSOCK && err != os_sockEINTR && err != os_sockEBADF)
      {
        nn_log (LC_ERROR, "ddsi2: select: %d\n", err);
        abort ();
      }
    }

    if (!config.many_sockets_mode)
      assert (!participant_set_changed);
    if (!participant_set_changed && select_res > 0)
    {
      for (i = 0; i < NUM_FIXED; i++)
      {
        if (FD_ISSET (fixed[i], &fdset))
          do_packet (fixed[i], NULL, rbpool);
      }
      if (config.many_sockets_mode)
      {
        for (i = 0; i < nparticipants; i++)
          if (FD_ISSET (participant_set[i]->sock, &fdset))
            do_packet (participant_set[i]->sock, participant_set[i], rbpool);
      }
    }

    os_mutexLock (&lock);
  }
  os_mutexUnlock (&lock);
  return (void *) rbpool;
#undef NUM_FIXED
}

static void init_locpair (nn_locator_t *loc, nn_locator_udpv4_t *udpv4, struct in_addr addr, unsigned short port)
{
  /* addr in proper network format; we keep the address part of the
     locators in proper network format, but not the kind and port fields */
  memset (loc, 0, sizeof (*loc));
  loc->kind = NN_LOCATOR_KIND_UDPv4;
  memcpy (loc->address + 12, &addr, 4);
  loc->port = port;
  nn_loc_to_loc_udpv4 (udpv4, loc);
}

static int find_own_ip (struct in_addr *ownip)
{
  os_ifAttributes ifs[MAX_INTERFACES];
  const char *sep = " ";
  char last_if_name[80] = "";
  int quality = -1;
  os_result res;
  unsigned i;
  unsigned nif;

  nn_log (LC_CONFIG, "interfaces:");
  if ((res = os_sockQueryInterfaces (&ifs[0], (os_uint32) MAX_INTERFACES, &nif)) != os_resultSuccess)
  {
    nn_log (LC_ERROR, "os_sockQueryInterfaces: %d\n", (int) res);
    return 0;
  }
  n_interfaces = 0;
  for (i = 0; i < nif; i++, sep = ", ")
  {
    struct in_addr tmpip, tmpmask;
    char if_name[sizeof (last_if_name)];
    int q;

    os_strncpy (if_name, ifs[i].name, sizeof (if_name) - 1);
    if_name[sizeof (if_name) - 1] = 0;

    /* filter out those interfaces that do not support IP
       addressing, that are down, that are loopback interfaces, or
       that don't support broadcasting */
    if ((ifs[i].flags & IFF_UP) == 0 || (ifs[i].flags & IFF_LOOPBACK))
    {
      if (strcmp (if_name, last_if_name))
        nn_log (LC_CONFIG, "%s%s", sep, if_name);
      os_strcpy (last_if_name, if_name);
      continue;
    }
    /* Get IP address, netmask */
    if (((struct sockaddr_in *) &ifs[i].address)->sin_family != AF_INET)
    {
      if (strcmp (if_name, last_if_name))
        nn_log (LC_CONFIG, "%s%s", sep, if_name);
      os_strcpy (last_if_name, if_name);
      continue;
    }
    else
    {
      tmpip = ((struct sockaddr_in *) &ifs[i].address)->sin_addr;
    }
    if (((struct sockaddr_in *) &ifs[i].network_mask)->sin_family != AF_INET)
    {
      if (strcmp (if_name, last_if_name))
        nn_log (LC_CONFIG, "%s%s", sep, if_name);
      os_strcpy (last_if_name, if_name);
      continue;
    }
    else
    {
      tmpmask = ((struct sockaddr_in *) &ifs[i].network_mask)->sin_addr;
    }
    /* In order of decreasing preference: multicast capable, not
       point-to-point, anything else (it doesn't really matter much
       which one we pick as IP routing will take of most issues) */
    if (ifs[i].flags & IFF_MULTICAST)
      q = 2;
    else if (!(ifs[i].flags & IFF_POINTOPOINT))
      q = 1;
    else
      q = 0;
    if (strcmp (if_name, last_if_name))
      nn_log (LC_CONFIG, "%s%s@q%d:%s", sep, if_name, q, inet_ntoa (tmpip));
    else
      nn_log (LC_CONFIG, "@q%d:%s", q, inet_ntoa (tmpip));
    if (q > quality)
    {
      quality = q;
      *ownip = tmpip;
    }
    os_strcpy (last_if_name, if_name);

    interfaces[n_interfaces].addr = tmpip;
    interfaces[n_interfaces].netmask = tmpmask;
    interfaces[n_interfaces].mc_capable = (q >= 2);
    interfaces[n_interfaces].point_to_point = (q < 1);
    n_interfaces++;
  }
  nn_log (LC_CONFIG, "\n");
  return (quality >= 0);
}

static void add_peer_addresses_1 (struct addrset *as, struct in_addr ip, int port)
{
  nn_locator_t loc;
  nn_locator_udpv4_t loc4;
  assert (port == 0 || (port > 0 && port < 65536));
  memset (&loc, 0, sizeof (loc));
  loc.kind = NN_LOCATOR_KIND_UDPv4;
  memcpy (loc.address + 12, &ip, 4);
  loc.port = port;
  nn_loc_to_loc_udpv4 (&loc4, &loc);
  if (port != 0)
  {
    nn_log (LC_CONFIG, "add_peer_addresses: add %s:%d\n", inet_ntoa (ip), loc4.port);
    add_to_addrset (as, &loc4);
  }
  else if (is_mcaddr (&loc4))
  {
    loc4.port = config.port_base + config.port_dg * domainid + config.port_d0;
    nn_log (LC_CONFIG, "add_peer_addresses: add %s:%d\n", inet_ntoa (ip), loc4.port);
    add_to_addrset (as, &loc4);
  }
  else
  {
    int i;
    nn_log (LC_CONFIG, "add_peer_addresses: add %s", inet_ntoa (ip));
    for (i = 0; i < 10; i++)
    {
      loc4.port = config.port_base + config.port_dg * domainid + i * config.port_pg + config.port_d1;
      nn_log (LC_CONFIG, "%s:%d", (i == 0) ? "" : ", ", loc4.port);
      add_to_addrset (as, &loc4);
    }
    nn_log (LC_CONFIG, "\n");
  }
}

static int inet_from_string (const char *str, struct in_addr *addr)
{
  struct addrinfo template;
  struct addrinfo *result_list;
  memset (&template, 0, sizeof (template));
  template.ai_family = AF_INET;
  template.ai_socktype = SOCK_DGRAM;
   if (getaddrinfo (str, NULL, &template, &result_list) != 0)
    return 0;
  else if (result_list == NULL)
    return 0;
  else
  {
    *addr = ((struct sockaddr_in *) result_list->ai_addr)->sin_addr;
    /* Ignore other entries, just take first */
    freeaddrinfo (result_list);
    return 1;
  }
}

static void add_peer_addresses (struct addrset *as, const char *addrs)
{
  char *addrs_copy = os_strdup (addrs);
  char *pos = addrs_copy, *a;
  while ((a = ddsi2_strsep (&pos, ",")) != NULL)
  {
    struct in_addr ip;
    char *colon = strchr (a, ':');
    int port = 0;
    if (colon)
    {
      *colon++ = 0;
      port = atoi (colon);
      if (port <= 0 || port >= 65536)
      {
        nn_log (LC_CONFIG, "add_peer_addresses: %d: invalid port\n", port);
        exit (1);
      }
    }
    if (inet_from_string (a, &ip))
      add_peer_addresses_1 (as, ip, port);
    else
    {
      nn_log (LC_WARNING | LC_CONFIG, "add_peer_addresses: %s not a valid address\n", a);
    }
  }
  os_free (addrs_copy);
}

static int make_uc_sockets (int ppid)
{
  int r;
  if ((r = make_socket (&discsock_uc, config.port_base + config.port_dg * domainid + ppid * config.port_pg + config.port_d1, NULL)) < 0)
    return r;
  if ((r = make_socket (&datasock_uc, config.port_base + config.port_dg * domainid + ppid * config.port_pg + config.port_d3, NULL)) < 0)
  {
    os_sockFree (discsock_uc);
    return r;
  }
  return 0;
}

static int make_builtin_endpoint_xqos (nn_xqos_t *q, const nn_xqos_t *template)
{
  int res;
  if ((res = nn_xqos_copy (q, template)) < 0)
    return res;
  q->reliability.kind = NN_RELIABLE_RELIABILITY_QOS;
  q->reliability.max_blocking_time = nn_to_ddsi_duration (100 * T_MILLISECOND);
  q->durability.kind = NN_TRANSIENT_LOCAL_DURABILITY_QOS;
  return 0;
}

void rtps_init (void *vbase, v_kernel kernel, int pdomainid, int ppid, UNUSED_ARG (unsigned flags), const char *lcl_ownip, const char *lcl_peer_addrs)
{
  tstart = now ();

  /* Print start time for referencing relative times in the remainder
     of the nn_log. */
  {
    int sec = (int) (tstart / 1000000000);
    unsigned usec = (unsigned) (tstart % 1000000000) / 1000;
    os_time tv;
    char str[26]; /* 26 per ctime_r() manpage */
    char *pnl;
    tv.tv_sec = sec;
    tv.tv_nsec = usec * 1000;
    os_ctime_r (&tv, str);
    if ((pnl = strchr (str, '\n')) != NULL)
      *pnl = 0;
    nn_log (LC_INFO | LC_CONFIG, "started at %d.06%u -- %s\n", sec, usec, str);
  }

  domainid = pdomainid;
  ospl_base = vbase;
  ospl_kernel = kernel;

  inet_from_string ("239.255.0.1", &mcip);
  /* Must enumerate interfaces nowadays */
  if (!find_own_ip (&ownip))
  {
    nn_log (LC_ERROR, "Failed to determine default own IP address\n");
    exit (1);
  }
  if (config.allowMulticast)
  {
    int i;
    for (i = 0; i < n_interfaces; i++)
    {
      if (interfaces[i].mc_capable)
        break;
    }
    if (i == n_interfaces)
    {
      nn_log (LC_WARNING, "No multicast capable interfaces: disabling multicast\n");
      config.allowMulticast = 0;
    }
  }
  if (lcl_ownip)
  {
    int i;
    if (!inet_from_string (lcl_ownip, &ownip))
    {
      nn_log (LC_ERROR, "%s: not a valid IP address\n", lcl_ownip);
      exit (1);
    }
    for (i = 0; i < n_interfaces; i++)
    {
      if (interfaces[i].addr.s_addr == ownip.s_addr)
        break;
    }
    if (i == n_interfaces)
    {
      nn_log (LC_ERROR, "No interface bound to requested preferred address\n");
      exit (1);
    }
  }
  nn_log (LC_CONFIG, "ownip: %s\n", inet_ntoa (ownip));
  nn_log (LC_CONFIG, "networkid: 0x%lx\n", (unsigned long) myNetworkId);

  if (config.startup_mode_duration > 0)
    startup_mode = 1;
  else
    startup_mode = 0;
  nn_log (LC_CONFIG, "startup-mode: %s\n", startup_mode ? "enabled" : "disabled");

  if (!config.many_sockets_mode)
    participant_set = NULL;
  else
  {
    if ((participant_set = os_malloc (config.max_participants * sizeof (*participant_set))) == NULL)
    {
      nn_log (LC_ERROR, "failed to allocate memory for %d participants\n", config.max_participants);
      exit (1);
    }
  }

  ospl_qostype = (c_collectionType) c_metaArrayTypeNew (
          c_metaObject (ospl_base), "C_ARRAY<c_octet>", c_octet_t (ospl_base), 0);
  osplser_init (ospl_base);

  xmsgpool = nn_xmsgpool_new ();
  serpool = serstatepool_new ();

  nn_xqos_init_default_reader (&default_xqos_rd);
  nn_xqos_init_default_writer (&default_xqos_wr);
  nn_xqos_copy (&spdp_endpoint_xqos, &default_xqos_rd);
  spdp_endpoint_xqos.durability.kind = NN_TRANSIENT_LOCAL_DURABILITY_QOS;
  make_builtin_endpoint_xqos (&builtin_endpoint_xqos_rd, &default_xqos_rd);
  make_builtin_endpoint_xqos (&builtin_endpoint_xqos_wr, &default_xqos_wr);

  avl_init (&pptree, offsetof (struct participant, avlnode), offsetof (struct participant, guid), compare_guid, 0, free_participant);
  avl_init (&proxypptree, offsetof (struct proxy_participant, avlnode), offsetof (struct proxy_participant, guid), compare_guid, augment_proxy_participant, free_proxy_participant);
  avl_init (&proxyrdtree, offsetof (struct proxy_reader, c.avlnode), offsetof (struct proxy_reader, c.guid), compare_guid, augment_proxy_reader, free_proxy_reader);
  avl_init (&proxywrtree, offsetof (struct proxy_writer, c.avlnode), offsetof (struct proxy_writer, c.guid), compare_guid, augment_proxy_writer, free_proxy_writer);
  xevents = new_xeventq ();

  spdp_defrag = nn_defrag_new (NN_DEFRAG_DROP_OLDEST, config.defrag_unreliable_maxsamples);
  spdp_reorder = nn_reorder_new (NN_REORDER_MODE_ALWAYS_DELIVER, config.primary_reorder_maxsamples);

  if (ppid >= 0)
  {
    if (make_uc_sockets (ppid) < 0)
    {
      nn_log (LC_ERROR, "rtps_init: failed to create unicast sockets for domain %d participant %d\n", domainid, ppid);
      exit (1);
    }
  }
  else
  {
    const int max_attempts = 10;
    nn_log (LC_CONFIG, "rtps_init: trying to find a free participant index\n");
    for (ppid = 0; ppid < max_attempts; ppid++)
    {
      int r = make_uc_sockets (ppid);
      if (r == 0) /* Success! */
        break;
      else if (r == -1) /* Try next one */
        continue;
      else /* Oops! */
      {
        nn_log (LC_ERROR, "rtps_init: failed to create unicast sockets for domain %d participant %d\n", domainid, ppid);
        exit (1);
      }
    }
    if (ppid == max_attempts)
    {
      nn_log (LC_ERROR, "rtps_init: failed to find a free participant index for domain %d\n", domainid);
      exit (1);
    }
  }
  nn_log (LC_CONFIG, "rtps_init: domainid %d participantid %d\n", domainid, ppid);
  participantid = ppid;

  if (make_socket (&discsock_mc, config.port_base + config.port_dg * domainid + config.port_d0, &mcip) < 0 ||
      make_socket (&datasock_mc, config.port_base + config.port_dg * domainid + config.port_d2, &mcip) < 0)
  {
    nn_log (LC_ERROR, "rtps_init: failed to create multicast sockets for domain %d participant %d\n", domainid, ppid);
    exit (1);
  }

  init_locpair (&loc_meta_uc, &udpv4_meta_uc, ownip, config.port_base + config.port_dg * domainid + ppid * config.port_pg + config.port_d1);
  init_locpair (&loc_meta_mc, &udpv4_meta_mc, mcip, config.port_base + config.port_dg * domainid + config.port_d0);
  init_locpair (&loc_default_uc, &udpv4_default_uc, ownip, config.port_base + config.port_dg * domainid + ppid * config.port_pg + config.port_d3);
  init_locpair (&loc_default_mc, &udpv4_default_mc, mcip, config.port_base + config.port_dg * domainid + config.port_d2);

  as_disc_init = new_addrset ();
  add_to_addrset (as_disc_init, &udpv4_meta_mc);
  if (lcl_peer_addrs)
    add_peer_addresses (as_disc_init, lcl_peer_addrs);
  as_disc = new_addrset ();
  copy_addrset_into_addrset (as_disc, as_disc_init);
  nn_log (LC_TRACE, "sockets: disc_uc %d disc_mc %d uc %d mc %d\n", discsock_uc, discsock_mc, datasock_uc, datasock_mc);

  {
    os_mutexAttr mattr;
    os_threadAttr tattr;
    os_condAttr cattr;
    keepgoing = 1;

    os_mutexAttrInit(&mattr);
    os_threadAttrInit(&tattr);
    os_condAttrInit(&cattr);
    mattr.scopeAttr = OS_SCOPE_PRIVATE;
    cattr.scopeAttr = OS_SCOPE_PRIVATE;

    os_mutexInit (&lock, &mattr);
    os_condInit (&evcond, &lock, &cattr);
    os_condInit (&throttle_cond, &lock, &cattr);

    os_threadCreate(&xmit_tid, "xmit", &tattr, xmit_thread, NULL);
    os_threadCreate(&recv_tid, "recv", &tattr, recv_thread, NULL);
  }

  spdp_dqueue = nn_dqueue_new ("spdp", nn_spdp_dqueue_handler, NULL);
  builtins_dqueue = nn_dqueue_new ("builtins", builtins_dqueue_handler, NULL);
  user_dqueue = nn_dqueue_new ("user", user_dqueue_handler, NULL);

  qxev_info (xevents, 0);
  if (startup_mode)
    qxev_end_startup_mode (xevents, now () + config.startup_mode_duration * T_MILLISECOND);
}

void rtps_term (void)
{
  void *tmp;
  struct nn_rbufpool *recv_rbpool; /* needed for delayed freeing */

  /* Stop all I/O */
  os_mutexLock (&lock);
  keepgoing = 0; /* so threads will stop once they get round to checking */
  os_condBroadcast (&throttle_cond);
  os_condSignal (&evcond); /* should cause xmit_thread to check */
  if (config.many_sockets_mode)
    participant_set_changed = 1;
  os_sockFree (discsock_uc); /* any one of these'll cause recv_thread to check */
  os_sockFree (discsock_mc);
  os_sockFree (datasock_uc);
  os_sockFree (datasock_mc);
  os_mutexUnlock (&lock);
  os_threadWaitExit (xmit_tid, NULL);
  os_threadWaitExit (recv_tid, &tmp);
  recv_rbpool = tmp;

  /* Once the receive threads have stopped, defragmentation and
     reorder state can't change anymore, and can be freed safely. */
  nn_reorder_free (spdp_reorder);
  nn_defrag_free (spdp_defrag);

  /* No new data gets added to any admin, all synchronous processing
     has ended, so now we can drain the delivery queues to end up with
     the expected reference counts all over the radmin thingummies. */
  nn_dqueue_free (user_dqueue);
  nn_dqueue_free (builtins_dqueue);
  nn_dqueue_free (spdp_dqueue);

  /* deleting (proxy) participants will take down all (proxy)
     readers/writers as well */
  while (!avl_empty (&proxypptree))
    avl_delete (&proxypptree, proxypptree.root);
  assert (avl_empty (&proxyrdtree));
  assert (avl_empty (&proxywrtree));
  while (!avl_empty (&pptree))
    delete_participant (pptree.root);
  free_xeventq (xevents);
  unref_addrset (as_disc);
  unref_addrset (as_disc_init);
  os_condDestroy (&evcond);
  os_condDestroy (&throttle_cond);
  os_mutexDestroy (&lock);

  /* Must delay freeing of rbufpools until after *all* references have
     been dropped, which only happens once all receive threads have
     stopped, defrags and reorders have been freed, and all delivery
     queues been drained.  I.e., until very late in the game. */
  nn_rbufpool_free (recv_rbpool);

  nn_xqos_fini (&builtin_endpoint_xqos_wr);
  nn_xqos_fini (&builtin_endpoint_xqos_rd);
  nn_xqos_fini (&spdp_endpoint_xqos);
  nn_xqos_fini (&default_xqos_wr);
  nn_xqos_fini (&default_xqos_rd);

  serstatepool_free (serpool);
  nn_xmsgpool_free (xmsgpool);
  osplser_fini ();
  if (participant_set)
    os_free (participant_set);
  c_free (ospl_qostype);
}
