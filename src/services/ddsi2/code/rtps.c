/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2010 PrismTech
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

/* FIXME: The handling of out-of-sequence message (though much
   improved to before, when it was non-existent) is still quite
   primitive: it is per (remote-writer)-reader combination, and it
   does not faithfully represent all combinations of DATA and GAPs;
   still it works somewhat. */

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
#include "avl.h"
#include "osplser.h"
#include "protocol.h"
#include "rtps.h"
#include "misc.h"

#ifndef OS_WIN32_IF_H
#include <netdb.h>
#endif

#include "mlv.h"

#ifndef UNUSED
#if defined (__GNUC__)
#define UNUSED __attribute__ ((unused))
#else
#define UNUSED
#endif /* def GNUC */
#endif /* ndef UNUSED */

#ifndef NDEBUG
#define UNUSED_NDEBUG
#else
#define UNUSED_NDEBUG UNUSED
#endif

/* vglobals() gets done if explicitly enabled, or if !NDEBUG && not
   explicitly disabled */
#ifndef DO_VGLOBALS
#ifndef NDEBUG
#define DO_VGLOBALS 1
#endif
#endif

#define MANY_SOCKETS 1
#if ! MANY_SOCKETS
#define MAX_PARTICIPANTS 0
#else
#define MAX_PARTICIPANTS 100
#endif

/* How to encode an AckNack without NAK'ing any messages, that is, how
   to represent an empty NAK set in an AckNack submessage. The
   specification requires the set to contain at least one entry,
   i.e. ACKNACK_NUMBITS_EMPTYSET=1, but that causes RTI's
   implementation to immediately respond with a HeartBeat without the
   Final set, triggering an AckNack ... */
#define ACKNACK_NUMBITS_EMPTYSET 0

/* A bit of a hack - like most of the liveliness implementation ... */
#define ARRIVAL_OF_DATA_ASSERTS_PP_AND_EP_LIVELINESS 1

/* A reliable writer in this DDSI implementation seems to not
   cooperate very well with a reliable reader from RTI if the writer
   is at a high sequence number at the time the reader is
   discovered. We attempt two things: one is to respond to an invalid
   AckNack we get from RTI (seemingly) upon discovery of the writer,
   the second is to more aggressively schedule the retransmission of
   missing messages until a pure Ack (that is, without also being a
   Nack) is received.

   The first part seems to work. If the WHC is not empty we can simply
   respond with a Heartbeat; but for an empty, no valid Heartbeat can
   be generated. If RESPOND_... is true, we just transmit an
   out-of-spec Heartbeat (like RTI does ...) anyway, but if it is
   false, we can delay the heartbeat until the WHC is no longer empty,
   but just before the data is sent out. Both techniques seem to work.

   The second part is mostly untested, for the simple reason that the
   problem hasn't really surfaced yet. */
#define RESPOND_TO_RTI_INIT_ZERO_ACK_WITH_INVALID_HEARTBEAT 0
#define ACCELERATE_REXMIT_BLOCKSIZE 32

/* If XMIT_DATA_OUT_OF_ORDER is true, the time for which the
   transmission of a data message will be scheduled in
   rtps_write_msg() (as well as a possible "piggybacked" heartbeat to
   request an ack) will be randomized to a small integer instead of
   fixed at zero */
#define XMIT_DATA_OUT_OF_ORDER 0

#if 1
#define WHC_HIGHWATER_MARK 50
#define WHC_LOWWATER_MARK 10
#define XEVQ_HIGHWATER_MARK 30
#define XEVQ_LOWWATER_MARK 5
#else
#define WHC_HIGHWATER_MARK 10
#define WHC_LOWWATER_MARK 6
#define XEVQ_HIGHWATER_MARK 8
#define XEVQ_LOWWATER_MARK 2
#endif

#define MAX_XEVENTS_BATCH_SIZE 8

#define MAX_OSA_SIZE 32

#define MAX_SEQ_NUMBER 0x7fffffffffffffffll

#define T_NEVER 0x7fffffffffffffffll
#define T_MILLISECOND 1000000ll
#define T_SECOND (1000 * T_MILLISECOND)

#define ALIGN4(x) (((x) + 3) & -4)

#define PGUID(g) (g).prefix.u[0], (g).prefix.u[1], (g).prefix.u[2], (g).entityid.u

struct addrset;
struct xevent;
struct xeventq;
struct proxy_endpoint;

#include "rtps_private.h"

static serstatepool_t serpool;
static c_base ospl_base;

static STRUCT_AVLTREE (proxypptree, struct proxy_participant *) proxypptree;
static STRUCT_AVLTREE (proxyeptree, struct proxy_endpoint *) proxyeptree;
static STRUCT_AVLTREE (participant_avltree, struct participant *) pptree;
static struct xeventq *xevents;
static os_socket discsock_uc, discsock_mc, datasock_uc, datasock_mc;

static struct in_addr ownip, mcip;
static locator_t loc_meta_mc, loc_meta_uc, loc_default_mc, loc_default_uc;
static locator_udpv4_t udpv4_meta_mc, udpv4_meta_uc, udpv4_default_mc, udpv4_default_uc;

static struct addrset *as_disc_init;
static struct addrset *as_disc;

#if MANY_SOCKETS
static int participant_set_changed;
static int nparticipants = 0;
static struct participant *participant_set[MAX_PARTICIPANTS];
#endif

static const ddsi_time_t ddsi_time_infinite = { 0x7fffffff, 0xffffffff };

static os_threadId xmit_tid;
static os_threadId recv_tid;
static os_mutex lock;
static os_cond evcond;
static os_cond throttle_cond;
static int keepgoing = 1;

static os_int64 tstart;

static int domainid = 0;
static int participantid = -1;

static int debugflags = DBGFLAG_TROUBLE;
static int (*trace_function) (const char *fmt, va_list ap) = vprintf;

#if DO_VGLOBALS
static void vglobals (void);
#endif

static presentation_qospolicy_t default_presentation_qospolicy;

static struct msg *make_SPDP_message (struct participant *pp);
static void write_pmd_message (struct participant *pp);
static void sedp_write_writer (struct writer *wr, const struct writer *info);
static void sedp_write_reader (struct writer *wr, const struct reader *info);
static int remove_acked_messages (struct writer *wr);
static struct msg *new_data_msg (ptrdiff_t *doff, struct writer *wr, unsigned flags);

static void unref_addrset (struct addrset *as);
static struct addrset *ref_addrset (struct addrset *as);
static void add_proxy_writer_to_reader (struct reader *rd, struct proxy_endpoint *pwr);
static void add_proxy_reader_to_writer (struct writer *wr, struct proxy_endpoint *prd);
static void remove_proxy_writer_from_reader (struct reader *rd, struct proxy_endpoint *pwr);
static void remove_proxy_reader_from_writer (struct writer *wr, struct proxy_endpoint *prd);

static struct writer *new_writer_unl (struct participant *pp, entityid_t id, unsigned mode, C_STRUCT (v_topic) const * const ospl_topic, const char *keystr, const char *partition);
static struct reader *new_reader_unl (struct participant *pp, entityid_t id, unsigned mode, C_STRUCT (v_topic) const * const ospl_topic, const char *keystr, const char *partition, void (*data_recv_cb) (), void *cb_arg);

static os_int64 earliest_in_xeventq (struct xeventq *evq);
static void xeventq_adjust_throttle (struct xeventq *evq);

static void handle_SEDP (const struct datainfo *di, const void *vdata, int len);
static void handle_PMD (const struct datainfo *di, const void *vdata, int len);

static int make_heartbeat_ack_required (struct writer *wr);
static void rtps_write_msg (struct writer *wr, struct msg *msg);
#ifndef NDEBUG
static int chk_outgoing_rtps_data_msg (const struct msg *msg);
#endif

static const int global_treat_transient_peers_as_transient_local = 0;
static const int strict_validation = 0;
static int use_mcast_flag;
static int use_mcast_loopback;
static int ignore_own_vendor;
static int aggressive_keep_last1_whc_flag;
static int conservative_builtin_reader_startup_flag;
static int noqueue_heartbeat_messages_flag;

static os_int64 now (void)
{
  os_time tv;
  tv = os_timeGet();
  return ((os_int64) tv.tv_sec * 1000000000ll + tv.tv_nsec);
}

static void time_to_sec_usec (int *sec, int *usec, os_int64 t)
{
  *sec = (int) (t / 1000000000);
  *usec = (int) (t % 1000000000) / 1000;
}

int rtps_vtrace (unsigned cat, const char *fmt, va_list ap)
{
  if (!(debugflags & cat))
  {
    return 0;
  }
  else
  {
    return trace_function (fmt, ap);
  }
}

int rtps_trace (unsigned cat, const char *fmt, ...)
{
  va_list ap;
  int n;
  va_start (ap, fmt);
  n = rtps_vtrace (cat, fmt, ap);
  va_end (ap);
  return n;
}

static int trace (unsigned cat, const char *fmt, ...)
{
  va_list ap;
  int n;
  va_start (ap, fmt);
  n = rtps_vtrace (cat, fmt, ap);
  va_end (ap);
  return n;
}

static int vtraceb (tracebuf_t trb, unsigned cat, const char *fmt, va_list ap)
{
  int n, trunc = 0, nrem;
  trb->cat |= cat;
  nrem = trb->bufsz - trb->pos;
  if (nrem == 0)
    return 0;
  if (!(debugflags & cat))
    return 0;
  n = os_vsnprintf (trb->buf + trb->pos, nrem, fmt, ap);
  if (n < nrem)
    trb->pos += n;
  else
  {
    trb->pos += nrem;
    trunc = 1;
  }
  if (trunc)
  {
    static const char msg[] = "(trunc)\n";
    assert (trb->pos <= trb->bufsz);
    assert (trb->pos >= (int) sizeof (msg));
    memcpy (trb->buf + trb->pos - sizeof (msg), msg, sizeof (msg));
  }
  return n;
}

static int traceb (tracebuf_t trb, unsigned cat, const char *fmt, ...)
{
  va_list ap;
  int n;
  va_start (ap, fmt);
  n = vtraceb (trb, cat, fmt, ap);
  va_end (ap);
  return n;
}

static void tracebuf_output (tracebuf_t trb)
{
  trace (trb->cat, "%s", trb->buf);
  trb->pos = 0;
  trb->cat = 0;
}

static void print_sockerror (const char *msg)
{
  int err = os_sockError ();
  trace (DBGFLAG_TROUBLE, "SOCKET ERROR %s %d\n", msg, err);
}

static os_int64 add_duration_to_time (os_int64 t, os_int64 d)
{
  os_int64 sum = t + d;
  assert (t >= 0 && d >= 0);
  return sum < t ? T_NEVER : sum;
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

static int vendor_is_rti (vendorid_t vendor)
{
  const vendorid_t twinoaks = VENDORID_RTI;
  return vendor.id[0] == twinoaks.id[0] && vendor.id[1] == twinoaks.id[1];
}

static int vendor_is_twinoaks (vendorid_t vendor)
{
  const vendorid_t twinoaks = VENDORID_TWINOAKS;
  return vendor.id[0] == twinoaks.id[0] && vendor.id[1] == twinoaks.id[1];
}

static int vendor_is_prismtech (vendorid_t vendor)
{
  const vendorid_t prismtech = VENDORID_PRISMTECH;
  return vendor.id[0] == prismtech.id[0] && vendor.id[1] == prismtech.id[1];
}

static int is_own_vendor (vendorid_t vendor)
{
  const vendorid_t ownid = MY_VENDOR_ID;
  return vendor.id[0] == ownid.id[0] && vendor.id[1] == ownid.id[1];
}

static entityid_t entityid_from_u (unsigned u)
{
  entityid_t id;
  id.u = u;
  return id;
}

static int compare_time (const void *va, const void *vb)
{
  const os_int64 *a = va;
  const os_int64 *b = vb;
  return (*a == *b) ? 0 : (*a < *b) ? -1 : 1;
}

static int compare_guid (const void *va, const void *vb)
{
  return memcmp (va, vb, sizeof (guid_t));
}

static int compare_seq (const void *va, const void *vb)
{
  const os_int64 *a = va;
  const os_int64 *b = vb;
  return (*a == *b) ? 0 : (*a < *b) ? -1 : 1;
}

static int compare_addr (const void *va, const void *vb)
{
  return memcmp (va, vb, sizeof (locator_udpv4_t));
}

static int is_writer_entityid (entityid_t id)
{
  switch (id.u & ENTITYID_KIND_MASK)
  {
    case ENTITYID_KIND_READER_WITH_KEY:
    case ENTITYID_KIND_READER_NO_KEY:
      return 0;
    case ENTITYID_KIND_WRITER_WITH_KEY:
    case ENTITYID_KIND_WRITER_NO_KEY:
      return 1;
    default:
      return 0;
  }
}

static int is_builtin_entityid (entityid_t id)
{
  return (id.u & ENTITYID_SOURCE_MASK) == ENTITYID_SOURCE_BUILTIN;
}

static os_int64 fromSN (const sequence_number_t sn)
{
  return ((os_int64) sn.high << 32) | sn.low;
}

static sequence_number_t toSN (os_int64 n)
{
  sequence_number_t x;
  x.high = (int) (n >> 32);
  x.low = (unsigned) n;
  return x;
}

static double time_to_double (os_int64 t)
{
  return t / 1e9;
}

static void debug_print_rawdata (const char *msg, const void *data, int len)
{
  const unsigned char *c = data;
  int i;
  trace (DBGFLAG_TRACING, "%s<", msg);
  for (i = 0; i < len; i++)
    trace (DBGFLAG_TRACING, "%s%02x", (i > 0 && (i%4) == 0) ? " " : "", c[i]);
  trace (DBGFLAG_TRACING, ">");
}

static int make_socket (os_socket *socket, unsigned short port, const struct in_addr *mcip)
{
  struct sockaddr_in socketname;

  *socket = os_sockNew (AF_INET, SOCK_DGRAM);

  /*trace (DBGFLAG_TRACING, "make_socket\n");*/
  if (!*socket)
  {
    print_sockerror ("socket");
    return -2;
  }
#if 0 /* really #if unix */
  if (*socket >= FD_SETSIZE)
  {
    trace (DBGFLAG_TROUBLE, "ddsi2: fatal: numerical value of file descriptor too large for select\n");
    abort ();
  }
#endif
  socketname.sin_family = AF_INET;
  socketname.sin_port = htons (port);
  socketname.sin_addr.s_addr = htonl (INADDR_ANY);

  if (mcip)
  {
    int one = 1;
    if (os_sockSetsockopt (*socket, SOL_SOCKET, SO_REUSEADDR, (char *) &one, sizeof (one)) != os_resultSuccess)
    {
      print_sockerror ("SO_REUSEADDR");
      return -2;
    }
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
    mreq.imr_multiaddr = *mcip;
#if 0
    mreq.imr_interface.s_addr = htonl (INADDR_ANY);
#else
    mreq.imr_interface = ownip;
#endif
    if (os_sockSetsockopt (*socket, IPPROTO_IP, IP_MULTICAST_IF, (char *) &ownip, sizeof (ownip)) != os_resultSuccess)
    {
      print_sockerror ("IP_MULTICAST_IF");
      return -2;
    }
    if (os_sockSetsockopt (*socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *) &mreq, sizeof (mreq)) != os_resultSuccess)
    {
      print_sockerror ("IP_ADD_MEMBERSHIP");
      return -2;
    }
    if (os_sockSetsockopt (*socket, IPPROTO_IP, IP_MULTICAST_TTL, (char *) &ttl, sizeof (ttl)) != os_resultSuccess)
    {
      print_sockerror ("IP_MULICAST_TTL");
      return -2;
    }
    loop = use_mcast_loopback;
    if (os_sockSetsockopt (*socket, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof (loop)) != os_resultSuccess)
    {
      print_sockerror ("IP_MULTICAST_LOOP");
    }
  }
  return 0;
}

/*///
///*/

static int partitions_match_p (const char *a, const char *b)
{
  return a && b && strcmp (a, b) == 0;
}

static int dds_attach_proxy_endpoint_walkpprd (void *vnode, void *arg)
{
  struct reader *rd = vnode;
  struct proxy_endpoint *ep = arg;
  assert (is_writer_entityid (ep->guid.entityid));
  /*trace (DBGFLAG_TRACING, "DAPEW R %p %p %p %p\n", rd->topic, ep->topic, rd->typename, ep->typename);*/
  if ((!rd->reliable || ep->reliable) &&
      rd->durability <= ep->durability &&
      rd->access_scope <= ep->presentation_qospolicy.access_scope &&
      (rd->exclusive_ownership ? EXCLUSIVE_OWNERSHIP_QOS : SHARED_OWNERSHIP_QOS) == ep->ownership_kind &&
      rd->topic && ep->topic && ep->typename &&
      strcmp (topic_name (rd->topic), ep->topic) == 0 &&
      strcmp (topic_typename (rd->topic), ep->typename) == 0 &&
      partitions_match_p (rd->partition, ep->partition))
    add_proxy_writer_to_reader (rd, ep);
  return AVLWALK_CONTINUE;
}

static int dds_attach_proxy_endpoint_walkppwr (void *vnode, void *arg)
{
  struct writer *wr = vnode;
  struct proxy_endpoint *ep = arg;
  assert (!is_writer_entityid (ep->guid.entityid));
  /*trace (DBGFLAG_TRACING, "DAPEW W %p %p %p %p\n", wr->topic, ep->topic, wr->typename, ep->typename);*/
  if ((!ep->reliable || wr->reliable) &&
      ep->durability <= wr->durability &&
      ep->presentation_qospolicy.access_scope <= wr->access_scope &&
      (wr->exclusive_ownership ? EXCLUSIVE_OWNERSHIP_QOS : SHARED_OWNERSHIP_QOS) == ep->ownership_kind &&
      wr->topic && ep->topic && ep->typename &&
      strcmp (topic_name (wr->topic), ep->topic) == 0 &&
      strcmp (topic_typename (wr->topic), ep->typename) == 0 &&
      partitions_match_p (wr->partition, ep->partition))
    add_proxy_reader_to_writer (wr, ep);
  return AVLWALK_CONTINUE;
}

static int dds_attach_proxy_endpoint_walkpp (void *vnode, void *arg)
{
  struct participant *pp = vnode;
  struct proxy_endpoint *ep = arg;
  /*trace (DBGFLAG_TRACING, "DAPEW %x:%x:%x ; %d ; %p\n", pp->guid.prefix.u[0], pp->guid.prefix.u[1], pp->guid.prefix.u[2], is_writer_entityid (ep->guid.entityid), pp->readers.root);*/
  if (is_writer_entityid (ep->guid.entityid))
    avl_walk (&pp->readers, dds_attach_proxy_endpoint_walkpprd, ep);
  else
    avl_walk (&pp->writers, dds_attach_proxy_endpoint_walkppwr, ep);
  return AVLWALK_CONTINUE;
}

static void dds_attach_proxy_endpoint (struct proxy_endpoint *ep)
{
  avl_walk (&pptree, dds_attach_proxy_endpoint_walkpp, ep);
}

static int dds_attach_reader_to_proxies_helper (void *vnode, void *varg)
{
  struct proxy_endpoint *ep = vnode;
  struct reader *rd = varg;
  if (is_writer_entityid (ep->guid.entityid))
    dds_attach_proxy_endpoint_walkpprd (rd, ep);
  return AVLWALK_CONTINUE;
}

static void dds_attach_reader_to_proxies (struct reader *rd)
{
  avl_walk (&proxyeptree, dds_attach_reader_to_proxies_helper, rd);
}

static int dds_attach_writer_to_proxies_helper (void *vnode, void *varg)
{
  struct proxy_endpoint *ep = vnode;
  struct writer *wr = varg;
  if (!is_writer_entityid (ep->guid.entityid))
    dds_attach_proxy_endpoint_walkppwr (wr, ep);
  return AVLWALK_CONTINUE;
}

static void dds_attach_writer_to_proxies (struct writer *wr)
{
  avl_walk (&proxyeptree, dds_attach_writer_to_proxies_helper, wr);
}
/*///
///*/


/** MESSAGES *****************************************************************/

static struct msg *new_msg (void)
{
  struct msg *m = os_malloc (offsetof (struct msg, msg));
  m->refc = 1;
  m->maxlen = 0;
  m->len = 0;
  m->serdata = NULL;
  m->tx_event = NULL;
  return m;
}

static struct msg *ref_msg (struct msg *msg)
{
  if (msg)
    msg->refc++;
  return msg;
}

static void unref_msg (struct msg *msg)
{
  if (msg && --msg->refc == 0)
  {
    assert (msg->tx_event == NULL);
    if (msg->serdata)
      serdata_release (msg->serdata);
    os_free (msg);
  }
}

/** MESSAGE QUEUE ************************************************************/

static void augment_xevent (void *vnode)
{
  struct xevent *e = vnode;
  int size;
  if (e->avlnode.left)
    e->min_tsched = e->avlnode.left->min_tsched;
  else
    e->min_tsched = e->tsched;
  /* approximation of immediate transmission */
  size = (e->tsched <= 1000) ? 1 : 0;
  if (e->avlnode.left)
    size += e->avlnode.left->size;
  if (e->avlnode.right)
    size += e->avlnode.right->size;
  e->size = size;
}

static void delete_xevent (struct xevent *ev)
{
  struct xeventq *evq = ev->evq;
  avl_delete (&evq->xevents, ev);
  switch (ev->kind)
  {
    case XEVK_HEARTBEAT:
    case XEVK_ACKNACK:
      break;
    case XEVK_MSG:
      unref_addrset (ev->u.msg.dest);
      unref_msg (ev->u.msg.msg);
      break;
    case XEVK_DATA:
      if (ev->u.data.msg->tx_event == ev)
	ev->u.data.msg->tx_event = NULL;
      unref_addrset (ev->u.data.dest_all);
      unref_msg (ev->u.data.msg);
      break;
    case XEVK_DATA_RESEND:
      if (ev->u.data_resend.msg->tx_event == ev)
	ev->u.data_resend.msg->tx_event = NULL;
      if (ev->u.data_resend.dest_all)
	unref_addrset (ev->u.data_resend.dest_all);
      unref_msg (ev->u.data_resend.msg);
      break;
    case XEVK_SPDP:
      unref_msg (ev->u.spdp.msg);
      break;
    case XEVK_GAP:
      unref_msg (ev->u.gap.msg);
      break;
    case XEVK_PMD_UPDATE:
      break;
    case XEVK_INFO:
      break;
    default:
      abort ();
  }
  os_free (ev);
  xeventq_adjust_throttle (evq);
}

static void resched_xevent (struct xevent *ev, os_int64 tsched)
{
  struct xeventq *evq = ev->evq;
  avlparent_t parent;
  avl_delete (&evq->xevents, ev);
  avl_lookup (&evq->xevents, &tsched, &parent);
  if (tsched < ev->tsched)
    os_condSignal (&evcond);
  ev->tsched = tsched;
  avl_init_node (&ev->avlnode, parent);
  avl_insert (&evq->xevents, ev);
  xeventq_adjust_throttle (evq);
}

static void resched_xevent_if_earlier (struct xevent *ev, os_int64 tsched)
{
  if (tsched < ev->tsched)
    resched_xevent (ev, tsched);
}

static struct xevent *qxev_common (struct xeventq *evq, os_int64 tsched, enum xeventkind kind)
{
  struct xevent *ev = os_malloc (sizeof (*ev));
  avlparent_t parent;
  avl_lookup (&evq->xevents, &tsched, &parent);
  avl_init_node (ev, parent);
  ev->evq = evq;
  ev->tsched = tsched;
  ev->kind = kind;
  ev->size = 0;
  return ev;
}

static void qxev_insert (struct xevent *xev)
{
  struct xeventq *evq = xev->evq;
  os_int64 tbefore = earliest_in_xeventq (evq);
  avl_insert (&evq->xevents, xev);
  xeventq_adjust_throttle (evq);
  if (xev->tsched < tbefore)
    os_condSignal (&evcond);
}

static struct xevent *qxev_heartbeat (struct xeventq *evq, os_int64 tsched, struct writer *wr)
{
  struct xevent *ev;
  ev = qxev_common (evq, tsched, XEVK_HEARTBEAT);
  ev->u.heartbeat.wr = wr;
  qxev_insert (ev);
  return ev;
}

static struct xevent *qxev_acknack (struct xeventq *evq, os_int64 tsched, struct rhc_writers_node *rwn)
{
  struct xevent *ev;
  ev = qxev_common (evq, tsched, XEVK_ACKNACK);
  ev->u.acknack.rwn = rwn;
  qxev_insert (ev);
  return ev;
}

static struct xevent *qxev_msg (struct xeventq *evq, os_int64 tsched, struct addrset *dest, struct msg *msg)
{
  struct xevent *ev;
  assert (msg->tx_event == NULL);
  ev = qxev_common (evq, tsched, XEVK_MSG);
  ev->u.msg.dest = ref_addrset (dest);
  ev->u.msg.msg = ref_msg (msg);
  qxev_insert (ev);
  return ev;
}

static struct xevent *qxev_data (struct xeventq *evq, os_int64 tsched, struct addrset *dest_all, struct msg *msg)
{
  struct xevent *ev;
  assert (msg->tx_event == NULL);
  ev = qxev_common (evq, tsched, XEVK_DATA);
  ev->u.data.dest_all = ref_addrset (dest_all);
  ev->u.data.msg = ref_msg (msg);
  msg->tx_event = ev;
  qxev_insert (ev);
  return ev;
}

static struct xevent *qxev_data_resend (struct xeventq *evq, os_int64 tsched, struct proxy_endpoint *prd, struct addrset *dest_all, struct msg *msg)
{
  struct xevent *ev;
  if (msg->tx_event == NULL)
  {
    ev = qxev_common (evq, tsched, XEVK_DATA_RESEND);
    ev->u.data_resend.prd = prd;
    ev->u.data_resend.dest_all = NULL;
    ev->u.data_resend.msg = ref_msg (msg);
    msg->tx_event = ev;
    qxev_insert (ev);
  }
  else if (msg->tx_event->kind == XEVK_DATA)
  {
    trace (DBGFLAG_TRACING, " (nack-before-xmit)");
  }
  else
  {
    assert (msg->tx_event->kind == XEVK_DATA_RESEND);
    if (prd == msg->tx_event->u.data_resend.prd)
    {
      /* exactly the same */
      ev = msg->tx_event;
    }
    else
    {
      /* resend to two different peers -> just resend to everybody and
	 forget to which peers */
      trace (DBGFLAG_TRACING, " (merged)", msg);
      ev = msg->tx_event;
      ev->u.data_resend.dest_all = ref_addrset (dest_all);
      ev->u.data_resend.prd = NULL;
    }
  }
  return ev;
}

static struct xevent *qxev_spdp (struct xeventq *evq, os_int64 tsched, struct msg *msg)
{
  struct xevent *ev;
  assert (msg->tx_event == NULL);
  ev = qxev_common (evq, tsched, XEVK_SPDP);
  ev->u.spdp.msg = ref_msg (msg);
  qxev_insert (ev);
  return ev;
}

static struct xevent *qxev_gap (struct xeventq *evq, os_int64 tsched, struct proxy_endpoint *prd, struct msg *msg)
{
  struct xevent *ev;
  assert (msg->tx_event == NULL);
  ev = qxev_common (evq, tsched, XEVK_GAP);
  ev->u.gap.prd = prd;
  ev->u.gap.msg = ref_msg (msg);
  qxev_insert (ev);
  return ev;
}

static struct xevent *qxev_pmd_update (struct xeventq *evq, os_int64 tsched, struct participant *pp)
{
  struct xevent *ev;
  ev = qxev_common (evq, tsched, XEVK_PMD_UPDATE);
  ev->u.pmd_update.pp = pp;
  qxev_insert (ev);
  return ev;
}

static struct xevent *qxev_info (struct xeventq *evq, os_int64 tsched)
{
  struct xevent *ev;
  ev = qxev_common (evq, tsched, XEVK_INFO);
  qxev_insert (ev);
  return ev;
}

static struct xeventq *new_xeventq (void)
{
  struct xeventq *evq = os_malloc (sizeof (*evq));
  avl_init (&evq->xevents, offsetof (struct xevent, avlnode), offsetof (struct xevent, tsched), compare_time, augment_xevent, 0);
  evq->oldsize = 0;
  return evq;
}

static void free_xeventq (struct xeventq *evq)
{
  while (!avl_empty (&evq->xevents))
    delete_xevent (evq->xevents.root);
  os_free (evq);
}

static int xeventq_size (const struct xeventq *evq)
{
  if (avl_empty (&evq->xevents))
    return 0;
  else
    return evq->xevents.root->size;
}

static int xeventq_must_throttle (struct xeventq *evq)
{
  return xeventq_size (xevents) > XEVQ_HIGHWATER_MARK;
}

static int xeventq_may_continue (struct xeventq *evq)
{
  return xeventq_size (xevents) < XEVQ_LOWWATER_MARK;
}

static void xeventq_adjust_throttle (struct xeventq *evq)
{
  /* A *very* simplistic bang-bang regulator */
  int oldsize = evq->oldsize;
  int newsize = xeventq_size (evq);
  evq->oldsize = newsize;
  if (newsize < XEVQ_LOWWATER_MARK && oldsize >= XEVQ_LOWWATER_MARK)
    os_condBroadcast (&throttle_cond);
}

static os_int64 earliest_in_xeventq (struct xeventq *evq)
{
  if (avl_empty (&evq->xevents))
    return T_NEVER;
  else
    return evq->xevents.root->min_tsched;
}

static struct xevent *next_from_xeventq (struct xeventq *evq)
{
  return avl_findmin (&evq->xevents);
}

/** ADDRSET ******************************************************************/

static struct addrset *new_addrset (void)
{
  struct addrset *as = os_malloc (sizeof (*as));
  as->refc = 1;
  avl_init (&as->ucaddrs, offsetof (struct addrset_node, avlnode), offsetof (struct addrset_node, addr), compare_addr, 0, os_free);
  avl_init (&as->mcaddrs, offsetof (struct addrset_node, avlnode), offsetof (struct addrset_node, addr), compare_addr, 0, os_free);
  return as;
}

static struct addrset *ref_addrset (struct addrset *as)
{
  if (as != NULL)
    as->refc++;
  return as;
}

static void unref_addrset (struct addrset *as)
{
  if (--as->refc == 0)
  {
    avl_free (&as->ucaddrs);
    avl_free (&as->mcaddrs);
    os_free (as);
  }
}

static int is_mcaddr (const locator_udpv4_t *addr)
{
  return IN_MULTICAST (ntohl (addr->address));
}

static void add_to_addrset (struct addrset *as, const locator_udpv4_t *addr)
{
  struct addrset_node *n;
  avlparent_t parent;
  struct addrset_avltree *tree = is_mcaddr (addr) ? &as->mcaddrs : &as->ucaddrs;
  if ((n = avl_lookup (tree, addr, &parent)) == NULL)
  {
    n = os_malloc (sizeof (*n));
    avl_init_node (&n->avlnode, parent);
    n->addr = *addr;
    avl_insert (tree, n);
  }
}

static int copy_addrset_into_addrset_helper (void *vnode, void *varg)
{
  const struct addrset_node *n = vnode;
  add_to_addrset (varg, &n->addr);
  return AVLWALK_CONTINUE;
}

static void copy_addrset_into_addrset_uc (struct addrset *as, const struct addrset *asadd)
{
  avl_walk ((struct addrset_avltree *) &asadd->ucaddrs, copy_addrset_into_addrset_helper, as);
}


static void copy_addrset_into_addrset_mc (struct addrset *as, const struct addrset *asadd)
{
  avl_walk ((struct addrset_avltree *) &asadd->mcaddrs, copy_addrset_into_addrset_helper, as);
}

static void copy_addrset_into_addrset (struct addrset *as, const struct addrset *asadd)
{
  copy_addrset_into_addrset_uc (as, asadd);
  copy_addrset_into_addrset_mc (as, asadd);
}

static int addrset_empty_uc (struct addrset *as)
{
  return avl_empty (&as->ucaddrs);
}

static int addrset_empty_mc (struct addrset *as)
{
  return avl_empty (&as->mcaddrs);
}

#if 0
static int addrset_empty (struct addrset *as)
{
  return addrset_empty_uc (as) && addrset_empty_mc (as);
}
#endif

static int addrset_any_uc (struct addrset *as, locator_udpv4_t *dst)
{
  if (addrset_empty_uc (as))
    return 0;
  else
  {
    *dst = as->ucaddrs.root->addr;
    return 1;
  }
}

static int addrset_any_mc (struct addrset *as, locator_udpv4_t *dst)
{
  if (addrset_empty_mc (as))
    return 0;
  else
  {
    *dst = as->mcaddrs.root->addr;
    return 1;
  }
}

typedef size_t (*addrset_forall_fun_t) (const locator_udpv4_t *addr, void *arg);

struct addrset_forall_addresses_helper_arg {
  addrset_forall_fun_t f;
  void *arg;
  size_t ret;
};

static int addrset_forall_helper (void *vnode, void *varg)
{
  const struct addrset_node *n = vnode;
  struct addrset_forall_addresses_helper_arg *arg = varg;
  arg->ret += arg->f (&n->addr, arg->arg);
  return AVLWALK_CONTINUE;
}

static size_t addrset_forall_addresses (struct addrset *as, addrset_forall_fun_t f, void *arg)
{
  struct addrset_forall_addresses_helper_arg arg1;
  arg1.f = f;
  arg1.arg = arg;
  arg1.ret = 0;
  avl_walk (&as->mcaddrs, addrset_forall_helper, &arg1);
  avl_walk (&as->ucaddrs, addrset_forall_helper, &arg1);
  return arg1.ret;
}

static size_t trace_addrset_helper (const locator_udpv4_t *n, void *vtf)
{
  unsigned *tf = vtf;
  struct in_addr x;
  x.s_addr = n->address;
  trace (*tf, " %s:%d", inet_ntoa (x), n->port);
  return 0;
}

static void trace_addrset (unsigned tf, const char *prefix, struct addrset *as)
{
  trace (tf, prefix);
  addrset_forall_addresses (as, trace_addrset_helper, &tf);
}

/** OUT-OF-SEQUENCE MESSAGES *************************************************/

static void osa_remember (out_of_seq_admin_t *osa, const struct handle_regular_helper_arg *info)
{
  struct out_of_seq_msg *m;
  avlparent_t parent;
  if (osa->size >= MAX_OSA_SIZE)
    return;
  if (avl_lookup (&osa->msgs, &info->seq, &parent) != NULL)
    return;
  /*trace (DBGFLAG_TRACING, "REMEMBER %lld\n", info->seq);*/
  m = os_malloc (sizeof (*m));
  avl_init_node (&m->avlnode, parent);
  m->gaplength = 0;
  m->info = *info;
  switch (m->info.payload_kind)
  {
    case PK_NONE:
    case PK_RAW_MALLOCED:
      break;
    case PK_V_MESSAGE:
      c_keep (m->info.payload.v_message);
      break;
    case PK_RAW_ALIASED:
      m->info.payload_kind = PK_RAW_MALLOCED;
      m->info.payload.raw.ptr = os_malloc (m->info.payload.raw.len);
      memcpy (m->info.payload.raw.ptr, info->payload.raw.ptr, m->info.payload.raw.len);
      break;
  }
  avl_insert (&osa->msgs, m);
  ++osa->size;
}

static void osa_remember_gap (out_of_seq_admin_t *osa, os_int64 seq, os_int64 length)
{
  struct out_of_seq_msg *m;
  avlparent_t parent;
  if (osa->size >= MAX_OSA_SIZE)
    return;
  if ((m = avl_lookup (&osa->msgs, &seq, &parent)) != NULL)
    return;
  m = os_malloc (sizeof (*m));
  avl_init_node (&m->avlnode, parent);
  m->gaplength = length;
  memset (&m->info, 0, sizeof (m->info));
  m->info.seq = seq;
  avl_insert (&osa->msgs, m);
  ++osa->size;
}

static struct out_of_seq_msg *osa_havemsg (out_of_seq_admin_t *osa, os_int64 seq)
{
  struct out_of_seq_msg *m;
  m = avl_lookup (&osa->msgs, &seq, NULL);
  /*trace (DBGFLAG_TRACING, "HAVEMSG %lld %s\n", seq, m ? "yes" : "no");*/
  return m;
}

static void osa_dropmsg (out_of_seq_admin_t *osa, os_int64 seq)
{
  /* anything <= seq may be dropped */
  struct out_of_seq_msg *m = avl_findmin (&osa->msgs);
  while (m && m->info.seq <= seq)
  {
    struct out_of_seq_msg *next = avl_findsucc (&osa->msgs, m);
    avl_delete (&osa->msgs, m);
    --osa->size;
    /*trace (DBGFLAG_TRACING, "DROPMSG %lld\n", m->info.seq);*/
    m = next;
  }
}

static void free_out_of_seq_msg (void *vnode)
{
  struct out_of_seq_msg *m = vnode;
  switch (m->info.payload_kind)
  {
    case PK_NONE:
      break;
    case PK_RAW_ALIASED:
      /* May not be an alias anymore by this time */
      abort ();
      break;
    case PK_RAW_MALLOCED:
      os_free (m->info.payload.raw.ptr);
      break;
    case PK_V_MESSAGE:
      c_free (m->info.payload.v_message);
      break;
  }
  os_free (m);
}

static unsigned durability_to_mask (durability_kind_t k)
{
  switch (k)
  {
    case VOLATILE_DURABILITY_QOS: return MF_DURABILITY_VOLATILE;
    case TRANSIENT_LOCAL_DURABILITY_QOS: return MF_DURABILITY_TRANSIENT_LOCAL;
    case TRANSIENT_DURABILITY_QOS: return MF_DURABILITY_TRANSIENT;
    case PERSISTENT_DURABILITY_QOS: return MF_DURABILITY_PERSISTENT;
  }
  abort (); return 0;
}

static durability_kind_t durability_from_mask (unsigned m)
{
  switch (m & MF_DURABILITY_MASK)
  {
    case MF_DURABILITY_VOLATILE: return VOLATILE_DURABILITY_QOS;
    case MF_DURABILITY_TRANSIENT_LOCAL: return TRANSIENT_LOCAL_DURABILITY_QOS;
    case MF_DURABILITY_TRANSIENT: return TRANSIENT_DURABILITY_QOS;
    case MF_DURABILITY_PERSISTENT: return PERSISTENT_DURABILITY_QOS;
  }
  abort (); return (durability_kind_t) 0;
}

static presentation_access_scope_kind_t access_scope_from_mask (unsigned m)
{
  switch (m & MF_ACCESS_SCOPE_MASK)
  {
    case MF_ACCESS_SCOPE_INSTANCE: return INSTANCE_PRESENTATION_QOS;
    case MF_ACCESS_SCOPE_TOPIC: return TOPIC_PRESENTATION_QOS;
    case MF_ACCESS_SCOPE_GROUP: return GROUP_PRESENTATION_QOS;
  }
  abort (); return (presentation_access_scope_kind_t) 0;
}

static unsigned access_scope_to_mask (presentation_access_scope_kind_t k)
{
  switch (k)
  {
    case INSTANCE_PRESENTATION_QOS: return MF_ACCESS_SCOPE_INSTANCE;
    case TOPIC_PRESENTATION_QOS: return MF_ACCESS_SCOPE_TOPIC;
    case GROUP_PRESENTATION_QOS: return MF_ACCESS_SCOPE_GROUP;
  }
  abort (); return 0;
}

static destination_order_kind_t destination_order_from_mask (unsigned m)
{
  switch (m & MF_DESTINATION_ORDER_MASK)
  {
    case MF_DESTINATION_ORDER_RECEPTION: return BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS;
    case MF_DESTINATION_ORDER_SOURCE: return BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;
  }
  abort (); return (destination_order_kind_t) 0;
}

static unsigned destination_order_to_mask (destination_order_kind_t k)
{
  switch (k)
  {
    case BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS: return MF_DESTINATION_ORDER_RECEPTION;
    case BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS: return MF_DESTINATION_ORDER_SOURCE;
  }
  abort (); return 0;
}

static const char *durability_to_string (durability_kind_t k)
{
  switch (k)
  {
    case VOLATILE_DURABILITY_QOS: return "volatile";
    case TRANSIENT_LOCAL_DURABILITY_QOS: return "transient-local";
    case TRANSIENT_DURABILITY_QOS: return "transient";
    case PERSISTENT_DURABILITY_QOS: return "persistent";
  }
  abort (); return 0;
}

static void fill_datainfo (struct datainfo *di, const struct rhc_writers_node *wn, const struct handle_regular_helper_arg *arg)
{
  const struct proxy_endpoint *ep = wn->proxy_writer;
  di->dst = wn->reader_guid;
  di->src = wn->writer_guid;
  di->statusinfo = arg->statusinfo;
  di->qos =
    (ep->reliable ? MF_RELIABLE : 0) |
    durability_to_mask (ep->durability);
  di->tstamp = arg->tstamp;
  di->seqnr = arg->seq;
}

static void deliver_one_message (struct rhc_writers_node *wn, struct handle_regular_helper_arg *arg)
{
  wn->reader->ndelivered++;
  if (wn->reader->data_recv_cb.generic)
  {
    struct datainfo di;
    trace (DBGFLAG_TRACING, " %x:%x:%x:%x", PGUID (wn->reader->guid));
    fill_datainfo (&di, wn, arg);
    switch (arg->payload_kind)
    {
      case PK_NONE:
	abort ();
	break;
      case PK_RAW_ALIASED:
      case PK_RAW_MALLOCED:
	wn->reader->data_recv_cb.raw (&di, arg->payload.raw.ptr, arg->payload.raw.len);
	break;
      case PK_V_MESSAGE:
	wn->reader->data_recv_cb.cooked.f (&di, arg->payload.v_message, wn->reader->data_recv_cb.cooked.arg);
	break;
    }
  }
  else
  {
    trace (DBGFLAG_TRACING, " (%x:%x:%x:%x)", PGUID (wn->reader->guid));
  }
}

static void osa_handoff (struct rhc_writers_node *wn)
{
  struct out_of_seq_msg *m;
  while ((m = osa_havemsg (&wn->osa, wn->seq + 1)) != NULL)
  {
    struct handle_regular_helper_arg *arg = &m->info;
    if (m->gaplength)
    {
      wn->seq = arg->seq + m->gaplength - 1;
      trace (DBGFLAG_TRACING, " [%lld]", m->gaplength);
    }
    else
    {
      trace (DBGFLAG_TRACING, " ()");
      wn->seq = arg->seq;
      deliver_one_message (wn, arg);
    }
  }
  osa_dropmsg (&wn->osa, wn->seq);
}

static void osa_init (out_of_seq_admin_t *osa)
{
  avl_init (&osa->msgs, offsetof (struct out_of_seq_msg, avlnode), offsetof (struct out_of_seq_msg, info.seq), compare_seq, 0, free_out_of_seq_msg);
  osa->size = 0;
}

static void osa_destroy (out_of_seq_admin_t *osa)
{
  avl_free (&osa->msgs);
}

/** PROXIES ******************************************************************/

static void maybe_add_pp_as_meta_to_as_disc (const struct proxy_participant *pp)
{
  if (addrset_empty_mc (pp->as_meta))
  {
    locator_udpv4_t loc;
    if (addrset_any_uc (pp->as_meta, &loc))
      add_to_addrset (as_disc, &loc);
  }
}

static int rebuild_as_disc_helper (void *vnode, void *varg UNUSED)
{
  const struct proxy_participant *pp = vnode;
  maybe_add_pp_as_meta_to_as_disc (pp);
  return AVLWALK_CONTINUE;
}

static void rebuild_as_disc (void)
{
  trace (DBGFLAG_TRACING, "rebuilding discovery address set\n");
  unref_addrset (as_disc);
  as_disc = new_addrset ();
  copy_addrset_into_addrset (as_disc, as_disc_init);
  avl_walk (&proxypptree, rebuild_as_disc_helper, NULL);
}

static struct proxy_participant *new_proxy_participant (guid_t guid, builtin_endpoint_set_t bes, struct addrset *as_default, struct addrset *as_meta, os_int64 tlease_dur, vendorid_t vendor)
{
  struct proxy_participant *pp;
  avlparent_t parent;
  if (avl_lookup (&proxypptree, &guid, &parent) != NULL)
    abort ();
  pp = os_malloc (sizeof (*pp));
  avl_init_node (&pp->avlnode, parent);
  pp->guid = guid;
  pp->vendor = vendor;
  pp->refc = 1;
  pp->bes = bes;
  pp->as_default = ref_addrset (as_default);
  pp->as_meta = ref_addrset (as_meta);
  maybe_add_pp_as_meta_to_as_disc (pp);
  pp->tlease_dur = tlease_dur;
  pp->tlease_end = add_duration_to_time (now (), tlease_dur);
  avl_insert (&proxypptree, pp);
  return pp;
}

static void augment_proxy_participant (void *vnode)
{
  struct proxy_participant *pp = vnode;
  os_int64 x = pp->tlease_end;
  guid_t xid = pp->guid;
  if (pp->avlnode.left && pp->avlnode.left->min_tlease_end < x)
  {
    x = pp->avlnode.left->min_tlease_end;
    xid = pp->avlnode.left->guid_min_tlease_end;
  }
  if (pp->avlnode.right && pp->avlnode.right->min_tlease_end < x)
  {
    x = pp->avlnode.right->min_tlease_end;
    xid = pp->avlnode.right->guid_min_tlease_end;
  }
  pp->min_tlease_end = x;
  pp->guid_min_tlease_end = xid;
}

static struct proxy_participant *ref_proxy_participant (struct proxy_participant *pp)
{
  if (pp)
    pp->refc++;
  return pp;
}

static int free_proxy_participant_helper (void *vnode UNUSED_NDEBUG, void *varg UNUSED_NDEBUG)
{
#ifndef NDEBUG
  struct proxy_endpoint *ep = vnode;
  const guid_t *ppid = varg;
  assert (memcmp (&ppid->prefix, &ep->guid.prefix, sizeof (ep->guid.prefix)) == 0);
#endif
  return AVLWALK_DELETE | AVLWALK_CONTINUE;
}

static void free_proxy_participant (void *vpp)
{
  struct proxy_participant *pp = vpp;
  guid_t min, max;
  int tsec, tusec;
  time_to_sec_usec (&tsec, &tusec, now ());
  /* participant dead => endpoints dead - do this the inefficient way */
  trace (DBGFLAG_TRACING, "%d.%06d free_proxy_participant(%x:%x:%x:%x)\n", tsec, tusec, PGUID (pp->guid));
  min = max = pp->guid;
  min.entityid.u = 0;
  max.entityid.u = ~0;
  avl_walkrange (&proxyeptree, &min, &max, free_proxy_participant_helper, &pp->guid);
  if (addrset_empty_mc (pp->as_meta))
    rebuild_as_disc ();
  unref_addrset (pp->as_default);
  unref_addrset (pp->as_meta);
  assert (pp->refc == 1);
  os_free (pp);
}

static void unref_proxy_participant (struct proxy_participant *pp)
{
  assert (pp->refc > 1);
  --pp->refc;
}

static struct proxy_endpoint *new_proxy_endpoint (guid_t guid, struct proxy_participant *pp, unsigned qos, struct addrset *as, os_int64 tlease_dur, const char *topic, const char *typename, const char *partition, liveliness_kind_t liveliness_kind, ownership_kind_t ownership_kind, destination_order_kind_t destination_order_kind, presentation_qospolicy_t presentation_qospolicy)
{
  struct proxy_endpoint *ep;
  avlparent_t parent;
  assert (!(qos & MF_HANDLE_AS_TRANSIENT_LOCAL));
  assert ((topic == NULL && typename == NULL && is_builtin_entityid (guid.entityid)) ||
	  (topic != NULL && typename != NULL));
  if (avl_lookup (&proxyeptree, &guid, &parent) != NULL)
    abort ();
  ep = os_malloc (sizeof (*ep));
  avl_init_node (&ep->avlnode, parent);
  ep->guid = guid;
  ep->reliable = (qos & MF_RELIABLE) ? 1 : 0;
  ep->durability = durability_from_mask (qos);
  ep->as = ref_addrset (as);
  ep->pp = ref_proxy_participant (pp);
  ep->tlease_dur = tlease_dur;
  ep->tlease_end = add_duration_to_time (now (), tlease_dur);
  ep->topic = topic ? os_strdup (topic) : NULL;
  ep->typename = typename ? os_strdup (typename) : NULL;
  ep->partition = partition ? os_strdup (partition) : NULL;
  ep->liveliness_kind = liveliness_kind;
  ep->ownership_kind = ownership_kind;
  ep->destination_order_kind = destination_order_kind;
  ep->presentation_qospolicy = presentation_qospolicy;
  if (is_writer_entityid (guid.entityid))
  {
    ep->u.wr.last_seq = 0;
    avl_init (&ep->matched_locals.readers, offsetof (struct rhc_writers_node, proxyavlnode), offsetof (struct rhc_writers_node, reader_guid), compare_guid, 0, 0);
  }
  else
  {
    avl_init (&ep->matched_locals.writers, offsetof (struct whc_readers_node, proxyavlnode), offsetof (struct whc_readers_node, writer_guid), compare_guid, 0, 0);
  }
  avl_insert (&proxyeptree, ep);
  return ep;
}

static void augment_proxy_endpoint (void *vnode)
{
  struct proxy_endpoint *ep = vnode;
  os_int64 x = ep->tlease_end;
  guid_t xid = ep->guid;
  if (ep->avlnode.left)
  {
    if (ep->avlnode.left->min_tlease_end < x)
    {
      x = ep->avlnode.left->min_tlease_end;
      xid = ep->avlnode.left->guid_min_tlease_end;
    }
  }
  if (ep->avlnode.right)
  {
    if (ep->avlnode.right->min_tlease_end < x)
    {
      x = ep->avlnode.right->min_tlease_end;
      xid = ep->avlnode.right->guid_min_tlease_end;
    }
  }
  ep->min_tlease_end = x;
  ep->guid_min_tlease_end = xid;
}

static void remove_xevents_for_proxy_reader (struct proxy_endpoint *prd)
{
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
}

static void free_proxy_endpoint (void *vep)
{
  struct proxy_endpoint *ep = vep;
  int tsec, tusec;
  time_to_sec_usec (&tsec, &tusec, now ());
  trace (DBGFLAG_TRACING, "%d.%06d free_proxy_endpoint(%x:%x:%x:%x)\n", tsec, tusec, PGUID (ep->guid));
  if (is_writer_entityid (ep->guid.entityid))
  {
    while (!avl_empty (&ep->matched_locals.readers))
      remove_proxy_writer_from_reader (ep->matched_locals.readers.root->reader, ep);
  }
  else
  {
    remove_xevents_for_proxy_reader (ep);
    while (!avl_empty (&ep->matched_locals.writers))
      remove_proxy_reader_from_writer (ep->matched_locals.writers.root->writer, ep);
  }
  if (ep->topic) os_free (ep->topic);
  if (ep->typename) os_free (ep->typename);
  if (ep->partition) os_free (ep->partition);
  unref_addrset (ep->as);
  unref_proxy_participant (ep->pp);
  os_free (ep);
}

static void cleanup_dead_proxies (void)
{
  os_int64 t = now ();
  while (!avl_empty (&proxyeptree) && proxyeptree.root->min_tlease_end < t)
  {
    struct proxy_endpoint *ep =
      avl_lookup (&proxyeptree, &proxyeptree.root->guid_min_tlease_end, NULL);
    assert (ep != NULL);
    /*trace (DBGFLAG_TRACING, "cleanup_dead_proxies: endpoint %x:%x:%x:%x\n", ep->guid.prefix.u[0], ep->guid.prefix.u[1], ep->guid.prefix.u[2], ep->guid.entityid.u);*/
    avl_delete (&proxyeptree, ep);
  }
  while (!avl_empty (&proxypptree) && proxypptree.root->min_tlease_end < t)
  {
    struct proxy_participant *pp =
      avl_lookup (&proxypptree, &proxypptree.root->guid_min_tlease_end, NULL);
    assert (pp != NULL);
    /*trace (DBGFLAG_TRACING, "cleanup_dead_proxies: participant %x:%x:%x:%x\n", pp->guid.prefix.u[0], pp->guid.prefix.u[1], pp->guid.prefix.u[2], pp->guid.entityid.u);*/
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
  if (wr->partition)
    os_free (wr->partition);
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
  if (rd->partition)
    os_free (rd->partition);
  if (!is_builtin_entityid (rd->guid.entityid))
  {
    assert (rd->data_recv_cb.cooked.f);
    rd->data_recv_cb.cooked.f (NULL, NULL, rd->data_recv_cb.cooked.arg);
  }
  os_free (rd);
}

static int attach_new_pp_to_proxypp_helper (void *vnode, void *varg)
{
  struct proxy_participant *proxypp = vnode;
  struct participant *pp = varg;
  struct proxy_endpoint *ep;
  guid_t guid1 = proxypp->guid;
  if (pp->sedp_writer_reader)
  {
    guid1.entityid.u = ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER;
    if ((ep = avl_lookup (&proxyeptree, &guid1, NULL)) != NULL)
      add_proxy_writer_to_reader (pp->sedp_writer_reader, ep);
  }
  guid1.entityid.u = ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER;
  if ((ep = avl_lookup (&proxyeptree, &guid1, NULL)) != NULL)
    add_proxy_reader_to_writer (pp->sedp_writer_writer, ep);
  if (pp->sedp_reader_reader)
  {    
    guid1.entityid.u = ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER;
    if ((ep = avl_lookup (&proxyeptree, &guid1, NULL)) != NULL)
      add_proxy_writer_to_reader (pp->sedp_reader_reader, ep);
  }
  guid1.entityid.u = ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_READER;
  if ((ep = avl_lookup (&proxyeptree, &guid1, NULL)) != NULL)
    add_proxy_reader_to_writer (pp->sedp_reader_writer, ep);
  if (pp->participant_message_reader)
  {
    guid1.entityid.u = ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_WRITER;
    if ((ep = avl_lookup (&proxyeptree, &guid1, NULL)) != NULL)
      add_proxy_writer_to_reader (pp->participant_message_reader, ep);
  }
  guid1.entityid.u = ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_READER;
  if ((ep = avl_lookup (&proxyeptree, &guid1, NULL)) != NULL)
    add_proxy_reader_to_writer (pp->participant_message_writer, ep);
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

static struct participant *new_participant_unl (guid_prefix_t idprefix, unsigned flags)
{
  struct participant *pp;
  avlparent_t parent;
  guid_t guid;
  int tsec, tusec;
  time_to_sec_usec (&tsec, &tusec, now ());
  assert ((flags & ~RTPS_PF_NO_BUILTIN_READERS) == 0);
#if MAX_PARTICIPANTS > 0
  assert (nparticipants < MAX_PARTICIPANTS);
#endif
  guid.prefix = idprefix;
  guid.entityid.u = ENTITYID_PARTICIPANT;
  trace (DBGFLAG_TRACING, "%d.%06d new_participant(%x:%x:%x:%x, %x)\n", tsec, tusec, PGUID (guid), flags);
  if (avl_lookup (&pptree, &guid, &parent) != NULL)
    abort ();
  pp = os_malloc (sizeof (*pp));
  avl_init_node (&pp->avlnode, parent);
  pp->guid = guid;
  avl_init (&pp->writers, offsetof (struct writer, avlnode), offsetof (struct writer, guid), compare_guid, augment_writer, free_writer);
  avl_init (&pp->readers, offsetof (struct reader, avlnode), offsetof (struct reader, guid), compare_guid, augment_reader, free_reader);

  pp->dying = 0;
  pp->lease_duration = 11 * T_SECOND; /* FIXME: fixed duration is a hack */

#if MANY_SOCKETS
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
#endif
  nparticipants++;
  avl_insert (&pptree, pp);

  pp->pmd_update_xevent = qxev_pmd_update (xevents, (pp->lease_duration == T_NEVER) ? T_NEVER : 0, pp);

  pp->bes = 0;

  pp->spdp_pp_writer = new_writer_unl (pp, entityid_from_u (ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER), 0, NULL, NULL, NULL);
  pp->bes |= DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER;
  
  pp->sedp_reader_writer = new_writer_unl (pp, entityid_from_u (ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER), MF_RELIABLE | MF_HANDLE_AS_TRANSIENT_LOCAL | MF_DURABILITY_TRANSIENT_LOCAL, NULL, NULL, NULL);
  pp->bes |= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER;
  pp->sedp_writer_writer = new_writer_unl (pp, entityid_from_u (ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER), MF_RELIABLE | MF_HANDLE_AS_TRANSIENT_LOCAL | MF_DURABILITY_TRANSIENT_LOCAL, NULL, NULL, NULL);
  pp->bes |= DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER;
  
  pp->participant_message_writer = new_writer_unl (pp, entityid_from_u (ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_WRITER), MF_RELIABLE | MF_HANDLE_AS_TRANSIENT_LOCAL | MF_DURABILITY_TRANSIENT_LOCAL, NULL, NULL, NULL);
  pp->bes |= BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_WRITER;

  if (flags & RTPS_PF_NO_BUILTIN_READERS)
  {
    pp->spdp_pp_reader = NULL;
    pp->sedp_reader_reader = NULL;
    pp->sedp_writer_reader = NULL;
    pp->participant_message_reader = NULL;
  }
  else
  {
    pp->spdp_pp_reader = new_reader_unl (pp, entityid_from_u (ENTITYID_SPDP_BUILTIN_PARTICIPANT_READER), 0, NULL, NULL, NULL, 0, NULL);
    pp->bes |= DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR;
    pp->sedp_reader_reader = new_reader_unl (pp, entityid_from_u (ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_READER), MF_RELIABLE | MF_HANDLE_AS_TRANSIENT_LOCAL | MF_DURABILITY_TRANSIENT_LOCAL, NULL, NULL, NULL, handle_SEDP, NULL);
    pp->bes |=DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR;
    pp->sedp_writer_reader = new_reader_unl (pp, entityid_from_u (ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER), MF_RELIABLE | MF_HANDLE_AS_TRANSIENT_LOCAL | MF_DURABILITY_TRANSIENT_LOCAL, NULL, NULL, NULL, handle_SEDP, NULL);
    pp->bes |= DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR;

    pp->participant_message_reader = new_reader_unl (pp, entityid_from_u (ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_READER), MF_RELIABLE | MF_HANDLE_AS_TRANSIENT_LOCAL | MF_DURABILITY_TRANSIENT_LOCAL, NULL, NULL, NULL, handle_PMD, NULL);
    pp->bes |= BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_READER;
  }  
  
  attach_new_pp_to_proxypp (pp);

  {
    struct msg *msg = make_SPDP_message (pp);
    pp->spdp_xevent = qxev_spdp (xevents, 0, msg);
    unref_msg (msg);
  }
  return pp;
}

struct participant *new_participant (guid_prefix_t idprefix, unsigned flags)
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
  struct msg *m;
  pp->dying = 1;
  m = make_SPDP_message (pp);
  qxev_data (xevents, 0, as_disc, m);
  unref_msg (m);
  assert (nparticipants > 0);
  --nparticipants;
  avl_free (&pp->readers);
  avl_free (&pp->writers);
  delete_xevent (pp->spdp_xevent);
  delete_xevent (pp->pmd_update_xevent);
#if MANY_SOCKETS
  os_sockFree (pp->sock);
  if (pp->participant_set_index < nparticipants)
  {
    struct participant *pp1 = participant_set[nparticipants];
    pp1->participant_set_index = pp->participant_set_index;
    participant_set[pp1->participant_set_index] = pp1;
  }
  participant_set_changed = 1;
#endif
  os_free (pp);
}

void delete_participant (struct participant *pp)
{
  int tsec, tusec;
  time_to_sec_usec (&tsec, &tusec, now ());
  os_mutexLock (&lock);
  assert (avl_lookup (&pptree, &pp->guid, NULL) == (void *) pp);
  trace (DBGFLAG_TRACING, "%d.%06d DELETE_PARTICIPANT %x:%x:%x:%x\n", tsec, tusec, PGUID (pp->guid));
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
  trace (DBGFLAG_TRACING, "%d.%06d DELETE_READER %x:%x:%x:%x\n", tsec, tusec, PGUID (rd->guid));
  age = time_to_double (now () - rd->tcreate);
  trace (DBGFLAG_PERFINFO, "reader %x:%x:%x:%x age %g ndelivered %lld\n", PGUID (rd->guid), age, rd->ndelivered);
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
  trace (DBGFLAG_TRACING, "%d.%06d DELETE_WRITER %x:%x:%x:%x\n", tsec, tusec, PGUID (wr->guid));
  age = time_to_double (now () - wr->tcreate);
  trace (DBGFLAG_PERFINFO, "writer %x:%x:%x:%x age %g seq %lld\n", PGUID (wr->guid), age, wr->seq);
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
  unref_msg (n->msg);
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
  locator_udpv4_t addr;
  if (addrset_any_mc (n->proxy_reader->as, &addr))
    add_to_addrset (dst, &addr);
  else if (addrset_any_uc (n->proxy_reader->as, &addr))
    add_to_addrset (dst, &addr);
  return AVLWALK_CONTINUE;
}

static void rebuild_writer_addrset (struct writer *wr)
{
  unref_addrset (wr->as);
  wr->as = new_addrset ();
  if (!avl_empty (&wr->readers))
  {
    locator_udpv4_t addr;
    if (wr->readers.root->avlnode.height == 1 &&
	addrset_any_uc (wr->readers.root->proxy_reader->as, &addr))
      add_to_addrset (wr->as, &addr);
    else
      avl_walk (&wr->readers, rebuild_writer_addrset_walkrd, wr->as);
  }
}

static void free_whc_readers_node (void *vnode)
{
  struct whc_readers_node *n = vnode;
  avl_delete (&n->proxy_reader->matched_locals.writers, n);
  rebuild_writer_addrset (n->writer);
  remove_acked_messages (n->writer);
  os_free (n);
}

static int new_reader_writer_common (guid_t *pguid, topic_t *ptopic, const struct participant *pp, entityid_t id, C_STRUCT (v_topic) const * const ospl_topic, const char *partition, const char *keystr)
{
  guid_t guid;
  topic_t topic;
  int tsec, tusec;
  time_to_sec_usec (&tsec, &tusec, now ());
  assert (is_builtin_entityid (id) ? (ospl_topic == NULL) : (ospl_topic != NULL));
  if (ospl_topic == NULL)
    topic = NULL;
  else if ((topic = deftopic (ospl_topic, keystr)) == NULL)
    return -1;
  guid = pp->guid;
  guid.entityid = id;
  trace (DBGFLAG_TRACING, "%d.%06d new_%s(%x:%x:%x:%x, %s.%s/%s)\n", tsec, tusec, is_writer_entityid (id) ? "writer" : "reader", PGUID (guid), partition == NULL ? "(null)" : *partition == 0 ? "(default)" : partition, topic ? topic_name (topic) : "(null)", topic ? topic_typename (topic) : "(null)");
  *pguid = guid;
  *ptopic = topic;
  return 0;
}

static struct writer *new_writer_unl (struct participant *pp, entityid_t id, unsigned mode, C_STRUCT (v_topic) const * const ospl_topic, const char *keystr, const char *partition)
{
  struct writer *wr;
  avlparent_t pp_parent;
  guid_t guid;
  topic_t topic;
  assert (is_writer_entityid (id));
  assert ((mode & ~MF_MASK) == 0);
  assert (!((mode & MF_HANDLE_AS_TRANSIENT_LOCAL) && durability_from_mask (mode) == MF_DURABILITY_VOLATILE));
  if (new_reader_writer_common (&guid, &topic, pp, id, ospl_topic, partition, keystr) != 0)
    return NULL;
  if (avl_lookup (&pp->writers, &guid, &pp_parent) != NULL)
    abort ();
  wr = os_malloc (sizeof (*wr));
  avl_init_node (&wr->avlnode, pp_parent);
  wr->participant = pp;
  wr->guid = guid;
  wr->seq = 0;
  wr->hbcount = 0;
  wr->tcreate = now ();
  wr->throttling = 0;
  wr->deleting = 0;
  wr->dying = 0;
#if ! RESPOND_TO_RTI_INIT_ZERO_ACK_WITH_INVALID_HEARTBEAT
  wr->hb_before_next_msg = 0;
#endif
  wr->partition = partition ? os_strdup (partition) : NULL;
  wr->reliable = (mode & MF_RELIABLE) ? 1 : 0;
  wr->exclusive_ownership = (mode & MF_EXCLUSIVE_OWNERSHIP) ? 1 : 0;
  wr->handle_as_transient_local = (mode & MF_HANDLE_AS_TRANSIENT_LOCAL) ? 1 : 0;
  wr->durability = durability_from_mask (mode);
  wr->access_scope = access_scope_from_mask (mode);
  wr->destination_order = destination_order_from_mask (mode);
  wr->with_key = ((id.u & ENTITYID_KIND_MASK) == ENTITYID_KIND_WRITER_WITH_KEY);
  wr->topic = topic;
  wr->as = new_addrset ();
  wr->heartbeat_xevent = wr->reliable ? qxev_heartbeat (xevents, T_NEVER, wr) : NULL;
  wr->t_of_last_heartbeat = 0;
  wr->whc_seq_size = 0;
  wr->whc_tlidx_size = 0;
  wr->lease_duration = T_NEVER;/* 11 * T_SECOND;*/ /* FIXME: fixed duration is a hack*/
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

struct writer *new_writer (struct participant *pp, entityid_t id, unsigned mode, C_STRUCT (v_topic) const * const ospl_topic, const char *partition)
{
  struct writer *wr;
  os_mutexLock (&lock);
  wr = new_writer_unl (pp, id, mode, ospl_topic, NULL, partition);
  os_mutexUnlock (&lock);
  return wr;
}

static void add_proxy_reader_to_writer (struct writer *wr, struct proxy_endpoint *prd)
{
  struct whc_readers_node *n;
  avlparent_t parent, parent_pe;
  os_int64 tnow = now ();
  trace (DBGFLAG_TRACING, " [wr %x:%x:%x:%x -> prd %x:%x:%x:%x]",
	  PGUID (wr->guid), PGUID (prd->guid));
  if (avl_lookup (&wr->readers, &prd->guid, &parent) != NULL)
    /* attempting to add a proxy reader multiple times has no effect */
    return;
  if (avl_lookup (&prd->matched_locals.writers, &wr->guid, &parent_pe) != NULL)
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
  n->seq = prd->reliable ? 0 : MAX_SEQ_NUMBER;
  n->pure_ack_received = 0;
  n->reader_guid = prd->guid;
  n->writer_guid = wr->guid;
  n->proxy_reader = prd;
  n->last_acknack = 0;
  avl_insert (&wr->readers, n);
  avl_insert (&prd->matched_locals.writers, n);
  rebuild_writer_addrset (wr);
  if (!avl_empty (&wr->whc_seq) && wr->heartbeat_xevent)
  {
    /* new reader & data available -> make sure the next heartbeat is
       sent very soon */
    int delta = 10;
    if (vendor_is_twinoaks (prd->pp->vendor))
      /* needed at some point, so still here */
      delta = 100;
    resched_xevent_if_earlier (wr->heartbeat_xevent, tnow + delta * T_MILLISECOND);
  }
}

static void remove_proxy_reader_from_writer (struct writer *wr, struct proxy_endpoint *prd)
{
  struct whc_readers_node *n;
  n = avl_lookup (&wr->readers, &prd->guid, NULL);
  assert (n != NULL);
  assert (avl_lookup (&prd->matched_locals.writers, &wr->guid, NULL) == n);
  avl_delete (&wr->readers, n);
}

#if 0
static int prettyprint_whc_helper (void *vnode, void *varg UNUSED)
{
  static char tmp[1024];
  struct whc_node *node = vnode;
  int tmpsize = sizeof (tmp), res;
  res = prettyprint_serdata (tmp, tmpsize, node->serdata);
  assert (res >= 0);
  trace (DBGFLAG_TRACING, "  seq %lld [%s] %s%s\n", node->seq, node->in_tlidx ? "tl" : "", tmp, res >= tmpsize ? " (trunc)" : "");
  return AVLWALK_CONTINUE;
}

static void prettyprint_whc (struct writer *wr)
{
  trace (DBGFLAG_TRACING, "WHC writer %x:%x:%x:%x seq#%d tlidx#%d:\n", PGUID (wr->guid), wr->whc_seq_size, wr->whc_tlidx_size);
  avl_walk (&wr->whc_seq, prettyprint_whc_helper, NULL);
  assert (wr->whc_seq_size >= wr->whc_tlidx_size);
}
#else
static void prettyprint_whc (struct writer *wr UNUSED_NDEBUG)
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

static void add_msg_to_whc (struct writer *wr, struct msg *msg)
{
  const int data_offset =
    (int) (sizeof (Header_t) + sizeof (InfoDST_t) + sizeof (InfoTS_t));
  struct whc_node *newn = NULL;
  os_int64 seq = fromSN (((Data_t *) (msg->msg + data_offset))->writerSN);

#if 0
  trace (DBGFLAG_TRACING, "add_msg_to_whc: seq %lld key", seq);
  debug_print_rawdata ("", msg->serdata->key, sizeof (msg->serdata->key));
  trace (DBGFLAG_TRACING, "\n");
#endif

  /* An unreliable writer may not have any reliable subs */
  assert (wr->reliable || have_reliable_subs (wr) == 0);
  if ((wr->reliable && have_reliable_subs (wr)) || wr->handle_as_transient_local)
  {
    avlparent_t parent;
    if (avl_lookup (&wr->whc_seq, &seq, &parent) != NULL)
      abort ();
    newn = os_malloc (sizeof (*newn));
    avl_init_node (&newn->avlnode_seq, parent);
    newn->seq = seq;
    newn->in_tlidx = 0; /* if transient_local it will always become 1 in this call */
    newn->serdata = msg->serdata; /* so avl.c can handle the tlidx key */
    newn->msg = ref_msg (msg);
    avl_insert (&wr->whc_seq, newn);
    wr->whc_seq_size++;
    /*trace (DBGFLAG_TRACING, "add_msg_to_whc: insert %p in seq\n", newn);*/
  }

  if (wr->handle_as_transient_local)
  {
    avlparent_t parent;
    struct whc_node *oldtln = avl_lookup (&wr->whc_tlidx, msg->serdata, &parent);
    /*XXX if(!unregister) { XXX unregister should not enter tlidx I think */
    avl_init_node (&newn->avlnode_tlidx, parent);
    newn->in_tlidx = 1;
    avl_insert (&wr->whc_tlidx, newn);
    wr->whc_tlidx_size++;
    /*XXX } XXX*/
    /*trace (DBGFLAG_TRACING, "add_msg_to_whc: insert %p in tlidx\n", newn);*/
    if (oldtln != NULL)
    {
      /*trace (DBGFLAG_TRACING, "add_msg_to_whc: delete old %p from tlidx\n", oldtln);*/
      avl_delete (&wr->whc_tlidx, oldtln);
      wr->whc_tlidx_size--;
      /* taking oldtlnn out of tlidx means it may no longer be needed;
	 if so, remove it from whc_seq */
      if (aggressive_keep_last1_whc_flag ||
	  (!have_reliable_subs (wr) || oldtln->seq < wr->readers.root->min_seq))
      {
	/*trace (DBGFLAG_TRACING, "add_msg_to_whc: delete old %p from seq\n", oldtln);*/
	avl_delete (&wr->whc_seq, oldtln);
	wr->whc_seq_size--;
      }
    }
  }

  if (wr->reliable || wr->handle_as_transient_local)
    prettyprint_whc (wr);

  /* new data -> make sure heartbeat is scheduled for soonish transmission */
  if (wr->heartbeat_xevent)
    resched_xevent_if_earlier (wr->heartbeat_xevent, now () + 50 * T_MILLISECOND);
}

#ifndef NDEBUG
static int chk_outgoing_rtps_data_msg (const struct msg *msg)
{
  /* Very simple sanity check on format of outgoing data messages, to
     prevent nasty surprises in the adding/skipping/inserting headers
     during transmission and in extracting the serial number from the
     data part for the WHC */
  int pos;
  if (msg->serdata == NULL)
    return 0;
  if (msg->len < (int) (sizeof (Header_t) + sizeof (InfoDST_t) + sizeof (InfoTimestamp_t) + sizeof (Data_t)))
    return 0;
  pos = 0;
  if (memcmp (&((Header_t *) (msg->msg + pos))->protocol, "RTPS", 4) != 0)
    return 0;
  pos += sizeof (Header_t);
  if (((InfoDST_t *) (msg->msg + pos))->smhdr.submessageId != SMID_INFO_DST)
    return 0;
  pos += sizeof (InfoDST_t);
  if (((InfoTS_t *) (msg->msg + pos))->smhdr.submessageId != SMID_INFO_TS)
    return 0;
  pos += sizeof (InfoTS_t);
  if (((Data_t *) (msg->msg + pos))->smhdr.submessageId != SMID_DATA)
    return 0;
  return 1;
}
#endif

static void rtps_write_msg (struct writer *wr, struct msg *msg)
{
  int hb_sched = 0;
  int txmit = 0;
  assert (chk_outgoing_rtps_data_msg (msg));
  add_msg_to_whc (wr, msg);
#if ! RESPOND_TO_RTI_INIT_ZERO_ACK_WITH_INVALID_HEARTBEAT
  if (wr->hb_before_next_msg && !avl_empty (&wr->whc_seq))
  {
    resched_xevent_if_earlier (wr->heartbeat_xevent, 0);
    wr->hb_before_next_msg = 0;
    hb_sched = 1;
  }
#endif
#if XMIT_DATA_OUT_OF_ORDER
  txmit = random () % 4;
#endif
  qxev_data (xevents, txmit, wr->as, msg);
  if (!hb_sched && wr->heartbeat_xevent && make_heartbeat_ack_required (wr))
    resched_xevent_if_earlier (wr->heartbeat_xevent, txmit);
}

static void free_rhc_writers_node (void *vnode)
{
  struct rhc_writers_node *n = vnode;
  if (n->acknack_xevent)
    delete_xevent (n->acknack_xevent);
  avl_delete (&n->proxy_writer->matched_locals.readers, n);
  osa_destroy (&n->osa);
  os_free (n);
}

static struct reader *new_reader_unl (struct participant *pp, entityid_t id, unsigned mode, C_STRUCT (v_topic) const * const ospl_topic, const char *keystr, const char *partition, void (*data_recv_cb) (), void *cb_arg)
{
  struct reader *rd;
  avlparent_t pp_parent;
  guid_t guid;
  topic_t topic;
  assert (!is_writer_entityid (id));
  assert ((mode & ~MF_MASK) == 0);
  assert (!((mode & MF_HANDLE_AS_TRANSIENT_LOCAL) && durability_from_mask (mode) == MF_DURABILITY_VOLATILE));
  if (new_reader_writer_common (&guid, &topic, pp, id, ospl_topic, partition, keystr) != 0)
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
  rd->partition = partition ? os_strdup (partition) : NULL;
  rd->reliable = (mode & MF_RELIABLE) ? 1 : 0;
  rd->exclusive_ownership = (mode & MF_EXCLUSIVE_OWNERSHIP) ? 1 : 0;
  rd->handle_as_transient_local = (mode & MF_HANDLE_AS_TRANSIENT_LOCAL) ? 1 : 0;
  rd->durability = durability_from_mask (mode);
  rd->access_scope = access_scope_from_mask (mode);
  rd->destination_order = destination_order_from_mask (mode);
  rd->topic = topic;
  if (is_builtin_entityid (rd->guid.entityid))
  {
    rd->data_recv_cb.raw = data_recv_cb;
  }
  else
  {
    rd->data_recv_cb.cooked.f = data_recv_cb;
    rd->data_recv_cb.cooked.arg = cb_arg;
  }
  rd->lease_duration = T_NEVER;/*10 * T_SECOND;*/ /* FIXME: fixed 10s is a hack */
  avl_init (&rd->writers, offsetof (struct rhc_writers_node, avlnode), offsetof (struct rhc_writers_node, writer_guid), compare_guid, 0, free_rhc_writers_node);
  avl_insert (&pp->readers, rd);
  add_reader_for_sedp (rd);
  dds_attach_reader_to_proxies (rd);
  return rd;
}

struct reader *new_reader (struct participant *pp, entityid_t id, unsigned mode, C_STRUCT (v_topic) const * const ospl_topic, const char *partition, data_recv_cb_fun_t data_recv_cb, void *cb_arg)
{
  struct reader *rd;
  os_mutexLock (&lock);
  rd = new_reader_unl (pp, id, mode, ospl_topic, NULL, partition, data_recv_cb, cb_arg);
  os_mutexUnlock (&lock);
  return rd;
}

static void add_proxy_writer_to_reader (struct reader *rd, struct proxy_endpoint *pwr)
{
  struct rhc_writers_node *n;
  avlparent_t parent, parent_pe;
  os_int64 tnow = now ();
  int delta = 10;
  if (vendor_is_twinoaks (pwr->pp->vendor))
    delta = 100;
  trace (DBGFLAG_TRACING, " [pwr %x:%x:%x:%x -> rd %x:%x:%x:%x]",
	  PGUID (pwr->guid), PGUID (rd->guid));
  /* can't match a reliable reader with a best-effort writer */
  assert (!rd->reliable || pwr->reliable);
  if (avl_lookup (&rd->writers, &pwr->guid, &parent) != NULL)
    /* attempting to add a proxy reader multiple times has no effect */
    return;
  if (avl_lookup (&pwr->matched_locals.writers, &rd->guid, &parent_pe) != NULL)
    /* if it isn't in one tree, it mustn't be in the other one */
    abort ();
  n = os_malloc (sizeof (*n));
  avl_init_node (&n->avlnode, parent);
  avl_init_node (&n->proxyavlnode, parent_pe);
  n->reader = rd;
  n->proxy_writer = pwr;
  n->writer_guid = pwr->guid;
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
  if (!rd->handle_as_transient_local)
    /* not treated as transient-local -> ack all old data */
    n->seq = pwr->u.wr.last_seq;
  if (!conservative_builtin_reader_startup_flag &&
      is_builtin_entityid (rd->guid.entityid) &&
      !avl_empty (&pwr->matched_locals.writers))
    /* discovery data is processed exactly once, the 2nd & later
       builtin readers don't need copies */
    n->seq = pwr->u.wr.last_seq;
  else
    /* normal transient-local */
    n->seq = 0;
  n->count = 0;
  n->acknack_xevent = rd->reliable ? qxev_acknack (xevents, tnow + delta * T_MILLISECOND, n) : NULL;
  osa_init (&n->osa);
  avl_insert (&rd->writers, n);
  avl_insert (&pwr->matched_locals.writers, n);
}

static void remove_proxy_writer_from_reader (struct reader *rd, struct proxy_endpoint *pwr)
{
  struct rhc_writers_node *n;
  n = avl_lookup (&rd->writers, &pwr->guid, NULL);
  assert (n != NULL);
  assert (avl_lookup (&pwr->matched_locals.readers, &rd->guid, NULL) == n);
  avl_delete (&rd->writers, n);
}

/*/
/*/

static void chk_no_such_writer (guid_t guid)
{
  struct participant *pp;
  struct writer *wr;
  guid_t ppguid;
  ppguid = guid;
  ppguid.entityid.u = ENTITYID_PARTICIPANT;
  if ((pp = avl_lookup (&pptree, &ppguid, NULL)) != NULL)
  {
    trace (DBGFLAG_TRACING, "WEIRD: local participant does exist\n");
    if ((wr = avl_lookup (&pp->writers, &guid, NULL)) != NULL)
      trace (DBGFLAG_TRACING, "WEIRDER: local writer does exist as well\n");
    /*abort ();*/
  }
}

static void chk_no_such_reader (guid_t guid)
{
  struct participant *pp;
  struct reader *rd;
  guid_t ppguid;
  ppguid = guid;
  ppguid.entityid.u = ENTITYID_PARTICIPANT;
  if ((pp = avl_lookup (&pptree, &ppguid, NULL)) != NULL)
  {
    trace (DBGFLAG_TRACING, "WEIRD: local participant does exist\n");
    if ((rd = avl_lookup (&pp->readers, &guid, NULL)) != NULL)
      trace (DBGFLAG_TRACING, "WEIRDER: local reader does exist as well\n");
    /*abort ();*/
  }
}

static unsigned short bswap2u (unsigned short x)
{
  return (x >> 8) | (x << 8);
}

static unsigned bswap4u (unsigned x)
{
  return (x >> 24) | ((x >> 8) & 0xff00) | ((x << 8) & 0xff0000) | (x << 24);
}

#if 0
static short bswap2 (short x)
{
  return (short) bswap2u ((unsigned short) x);
}
#endif

static int bswap4 (int x)
{
  return (int) bswap4u ((unsigned) x);
}

static void bswapSN (sequence_number_t *sn)
{
  sn->high = bswap4 (sn->high);
  sn->low = bswap4u (sn->low);
}

static void bswap_sequence_number_set_hdr (sequence_number_set_t *snset)
{
  bswapSN (&snset->bitmap_base);
  snset->numbits = bswap4u (snset->numbits);
}

static void bswap_sequence_number_set_bitmap (sequence_number_set_t *snset)
{
  int i, n = (snset->numbits + 31) / 32;
  for (i = 0; i < n; i++)
    snset->bits[i] = bswap4u (snset->bits[i]);
}

static int pl_needs_bswap (const struct CDRHeader *hdr)
{
#if PLATFORM_IS_LITTLE_ENDIAN
  return (hdr->identifier == PL_CDR_BE);
#else
  return (hdr->identifier == PL_CDR_LE);
#endif
}

static struct msg *msg_make_room (struct msg *m, char **p, int n)
{
  if (m->len + n > m->maxlen)
  {
    m->maxlen = (m->len + n + 1023) & -1024;
    m = os_realloc (m, offsetof (struct msg, msg) + m->maxlen);
  }
  *p = m->msg + m->len;
  m->len += n;
  return m;
}

static guid_prefix_t hton_guid_prefix (guid_prefix_t p)
{
#if PLATFORM_IS_LITTLE_ENDIAN
  int i;
  for (i = 0; i < 3; i++)
    p.u[i] = bswap4u (p.u[i]);
#endif
  return p;
}

static guid_prefix_t ntoh_guid_prefix (guid_prefix_t p)
{
  return hton_guid_prefix (p);
}

static entityid_t hton_entityid (entityid_t e)
{
#if PLATFORM_IS_LITTLE_ENDIAN
  e.u = bswap4u (e.u);
#endif
  return e;
}

static entityid_t ntoh_entityid (entityid_t e)
{
  return hton_entityid (e);
}

static guid_t hton_guid (guid_t g)
{
  g.prefix = hton_guid_prefix (g.prefix);
  g.entityid = hton_entityid (g.entityid);
  return g;
}

static guid_t ntoh_guid (guid_t g)
{
  g.prefix = ntoh_guid_prefix (g.prefix);
  g.entityid = ntoh_entityid (g.entityid);
  return g;
}

static struct msg *msg_add_Header (struct msg *m, const guid_prefix_t *guid_prefix)
{
  static const vendorid_t myvendorid = MY_VENDOR_ID;
  Header_t *hdr;
  char *p;
  m = msg_make_room (m, &p, sizeof (Header_t));
  hdr = (Header_t *) p;
  hdr->protocol.id[0] = 'R';
  hdr->protocol.id[1] = 'T';
  hdr->protocol.id[2] = 'P';
  hdr->protocol.id[3] = 'S';
  hdr->version.major = RTPS_MAJOR;
  hdr->version.minor = RTPS_MINOR;
  hdr->vendorid = myvendorid;
  hdr->guid_prefix = hton_guid_prefix (*guid_prefix);
  return m;
}

static ddsi_time_t to_ddsi_time (os_int64 t)
{
  ddsi_time_t x;
  double td = (double) (t % 1000000000);
  x.seconds = (int) (t / 1000000000);
  x.fraction = (unsigned) (4.294967296 * td);
  /* not sure if rounding errors can cause nastiness - that depends a
     bit on the rounding modes and conversion to unsigned behaviour,
     and whatnot ... (actually, one lsb of t translates to multiple
     lsbs of fraction and multiple ulps in td, so I guess it won't
     happen) */
  assert (!(td > 0 && x.fraction == 0));
  return x;
}

static os_int64 from_ddsi_time (ddsi_time_t x)
{
  os_int64 t;
  t = x.seconds * 1000000000ll + (os_int64) (x.fraction / 4.294967296);
  return t;
}

static struct msg *msg_add_InfoTimestamp (struct msg *m, os_int64 t)
{
  InfoTimestamp_t *ts;
  char *tmp;
  m = msg_make_room (m, &tmp, sizeof (InfoTimestamp_t));
  ts = (InfoTimestamp_t *) tmp;
  ts->smhdr.submessageId = SMID_INFO_TS;
  ts->smhdr.flags = (PLATFORM_IS_LITTLE_ENDIAN ? SMFLAG_ENDIANNESS : 0);
  ts->smhdr.octetsToNextHeader = sizeof (ddsi_time_t);
  ts->time = to_ddsi_time (t);
  return m;
}

static struct msg *msg_add_InfoDST (struct msg *m, const guid_prefix_t *gp)
{
  InfoDST_t *dst;
  char *tmp;
  m = msg_make_room (m, &tmp, sizeof (InfoDST_t));
  dst = (InfoDST_t *) tmp;
  dst->smhdr.submessageId = SMID_INFO_DST;
  dst->smhdr.flags = (PLATFORM_IS_LITTLE_ENDIAN ? SMFLAG_ENDIANNESS : 0);
  dst->smhdr.octetsToNextHeader = sizeof (dst->guid_prefix);
  dst->guid_prefix = hton_guid_prefix (*gp);
  return m;
}

static entityid_t to_entityid (unsigned u)
{
  entityid_t e;
  e.u = u;
  return e;
}

static struct msg *msg_add_parameter (struct msg *m, char **p, int pid, int len)
{
  int len4 = (len + 3) & -4; /* must alloc a multiple of 4 */
  parameter_t *phdr;
  char *tmp;
  m = msg_make_room (m, &tmp, sizeof (parameter_t) + len4);
  phdr = (parameter_t *) tmp;
  *p = tmp + sizeof (parameter_t);
  phdr->parameterid = pid;
  phdr->length = len4;
  if (len4 > len)
  {
    /* zero out padding bytes added to satisfy parameter alignment --
       alternative: zero out, but this way valgrind/purify can tell us
       where we forgot to initialize something */
    memset (*p + len, 0, len4 - len);
  }
  return m;
}

static struct msg *msg_add_string_parameter (struct msg *m, int pid, const char *str)
{
  struct cdrstring *p;
  char *tmp;
  int len = strlen (str) + 1;
  m = msg_add_parameter (m, &tmp, pid, 4 + len);
  p = (struct cdrstring *) tmp;
  p->length = len;
  memcpy (p->contents, str, len);
  return m;
}

static struct msg *msg_add_stringseq_parameter (struct msg *m, int pid, int n, const char **str)
{
  char *tmp;
  int i, len = 0;
  for (i = 0; i < n; i++)
  {
    int len1 = strlen (str[i]) + 1;
    len += 4 + ALIGN4 (len1);
  }
  m = msg_add_parameter (m, &tmp, pid, 4 + len);
  *((int *) tmp) = n;
  tmp += sizeof (int);
  for (i = 0; i < n; i++)
  {
    struct cdrstring *p = (struct cdrstring *) tmp;
    int len1 = strlen (str[i]) + 1;
    p->length = len1;
    memcpy (p->contents, str[i], len1);
    if (len1 < ALIGN4 (len1))
      memset (p->contents + len1, 0, ALIGN4 (len1) - len1);
    tmp += 4 + ALIGN4 (len1);
  }
  return m;
}

static struct msg *msg_add_Gap (struct msg *m, struct whc_readers_node *wrn, os_int64 start, os_int64 base, int numbits, const unsigned *bits)
{
  int pos = m->len;
  Gap_t *gap;
  char *tmp;
  assert (numbits > 0);
  m = msg_make_room (m, &tmp, GAP_SIZE (numbits));
  gap = (Gap_t *) tmp;
  gap->smhdr.submessageId = SMID_GAP;
  gap->smhdr.flags =
    (PLATFORM_IS_LITTLE_ENDIAN ? SMFLAG_ENDIANNESS : 0);
  gap->smhdr.octetsToNextHeader = m->len - pos - RTPS_SUBMESSAGE_HEADER_SIZE;
  gap->readerId = hton_entityid (wrn->reader_guid.entityid);
  gap->writerId = hton_entityid (wrn->writer_guid.entityid);
  gap->gapStart = toSN (start);
  gap->gapList.bitmap_base = toSN (base);
  gap->gapList.numbits = numbits;
  memcpy (gap->gapList.bits, bits, SEQUENCE_NUMBER_SET_BITS_SIZE (numbits));
  return m;
}

static int acknack_numbits_needed (struct rhc_writers_node *rwn, int *isnack)
{
  os_int64 n;
  /* rwn->seq must have already been updated in the case of lost
     packets; no msgs are kept in storage and so the range of seqs to
     be NACKed is straightforward */
  n = rwn->proxy_writer->u.wr.last_seq - rwn->seq;
  if (isnack)
    *isnack = (n > 0);
  if (n > 256)
    return 256;
  else if (n == 0) /* = 0 is disallowed */
    return ACKNACK_NUMBITS_EMPTYSET;
  else if (n < 0) /* shouldn't happen anymore */
  {
    trace (DBGFLAG_TRACING, "ACKNACK_NUMBITS_NEEDED: n < 0\n");
    return ACKNACK_NUMBITS_EMPTYSET;
  }
  else
  {
    return (int) n;
  }
}

static struct msg *msg_add_AckNack (struct msg *m, struct rhc_writers_node *rwn, int *isnack)
{
  AckNack_t *an;
  int pos = m->len;
  int numbits = acknack_numbits_needed (rwn, isnack);
  count_t *countp;
  os_int64 i;
  char *tmp;
  m = msg_make_room (m, &tmp, ACKNACK_SIZE (numbits));
  an = (AckNack_t *) tmp;
  an->smhdr.submessageId = SMID_ACKNACK;
  an->smhdr.flags =
    (PLATFORM_IS_LITTLE_ENDIAN ? SMFLAG_ENDIANNESS : 0)
    | (*isnack ? 0 : ACKNACK_FLAG_FINAL);
  an->smhdr.octetsToNextHeader = m->len - pos - RTPS_SUBMESSAGE_HEADER_SIZE;
  an->readerId = hton_entityid (rwn->reader_guid.entityid);
  an->writerId = hton_entityid (rwn->writer_guid.entityid);
  an->readerSNState.bitmap_base = toSN (rwn->seq+1); /* ack all delivered */
  an->readerSNState.numbits = numbits;
  memset (an->readerSNState.bits, 0, SEQUENCE_NUMBER_SET_BITS_SIZE (numbits));
  i = 0;
  while (i < numbits && (rwn->seq+1) + i <= rwn->proxy_writer->u.wr.last_seq)
  {
    struct out_of_seq_msg *m;
    if ((m = osa_havemsg (&rwn->osa, (rwn->seq+1) + i)) != NULL)
    {
      i += m->gaplength ? m->gaplength : 1;
    }
    else
    {
      an->readerSNState.bits[i/32] |= 1u << (31 - (i % 32));
      i++;
    }
  }
  countp = (count_t *) ((char *) an + offsetof (AckNack_t, readerSNState) + SEQUENCE_NUMBER_SET_SIZE (numbits));
  *countp = ++rwn->count;
  trace (DBGFLAG_TRACING, "acknack %x:%x:%x:%x -> %x:%x:%x:%x: #%d:%lld/%d:", PGUID (rwn->reader_guid), PGUID (rwn->writer_guid), rwn->count, fromSN (an->readerSNState.bitmap_base), an->readerSNState.numbits);
#if ACKNACK_NUMBITS_EMPTYSET > 0 /* numbits is unsigned, so always >= 0 */
  assert (an->readerSNState.numbits >= ACKNACK_NUMBITS_EMPTYSET);
#endif
  assert (an->readerSNState.numbits <= 256);
  for (i = 0; i < an->readerSNState.numbits; i++)
    trace (DBGFLAG_TRACING, "%c", (an->readerSNState.bits[i/32] & (1u << (31 - (i % 32)))) ? '1' : '0');
  trace (DBGFLAG_TRACING, "\n");
  return m;
}

static int make_heartbeat_ack_required (struct writer *wr)
{
  os_int64 tnow = now ();
  if (tnow >= wr->t_of_last_heartbeat + 100 * T_MILLISECOND)
    return 1;
  else if (wr->whc_seq_size - wr->whc_tlidx_size < WHC_HIGHWATER_MARK/4)
    return 0;
  else if (wr->whc_seq_size - wr->whc_tlidx_size < WHC_HIGHWATER_MARK/2)
    return tnow >= wr->t_of_last_heartbeat + 20 * T_MILLISECOND;
  else if (wr->whc_seq_size - wr->whc_tlidx_size < 2*WHC_HIGHWATER_MARK/3)
    return tnow >= wr->t_of_last_heartbeat + 2 * T_MILLISECOND;
  else
    return 1;
}

static struct msg *msg_add_Heartbeat (struct msg *m, struct writer *wr, int *hbansreq, entityid_t dst)
{
  Heartbeat_t *hb;
  int pos = m->len;
  char *tmp;
  assert (wr->reliable);
#if ! RESPOND_TO_RTI_INIT_ZERO_ACK_WITH_INVALID_HEARTBEAT
  /* We're not really allowed to generate heartbeats when the WHC is
     empty, but it appears RTI sort-of needs them ... */
  assert (!avl_empty (&wr->whc_seq));
#endif
  m = msg_make_room (m, &tmp, sizeof (Heartbeat_t));
  hb = (Heartbeat_t *) tmp;
  hb->smhdr.submessageId = SMID_HEARTBEAT;
  hb->smhdr.flags = (PLATFORM_IS_LITTLE_ENDIAN ? SMFLAG_ENDIANNESS : 0);
  if (*hbansreq < 0)
    *hbansreq = make_heartbeat_ack_required (wr);
  if (! *hbansreq)
    hb->smhdr.flags |= HEARTBEAT_FLAG_FINAL;
  hb->smhdr.octetsToNextHeader = m->len - pos - RTPS_SUBMESSAGE_HEADER_SIZE;
#if 1
  hb->readerId = hton_entityid (dst);
#else
  if (!is_builtin_entityid (wr->guid.entityid))
    hb->readerId = hton_entityid (dst);
  else
  {
    entityid_t x = wr->guid.entityid;
    x.u = (x.u & ~0xff) | 0xc7;
    hb->readerId = hton_entityid (x);
  }
#endif
  hb->writerId = hton_entityid (wr->guid.entityid);
#if RESPOND_TO_RTI_INIT_ZERO_ACK_WITH_INVALID_HEARTBEAT
  if (!avl_empty (&wr->whc_seq))
  {
#endif
    hb->firstSN = toSN (wr->whc_seq.root->minseq);
    hb->lastSN = toSN (wr->whc_seq.root->maxseq);
#if RESPOND_TO_RTI_INIT_ZERO_ACK_WITH_INVALID_HEARTBEAT
  }
  else /* Thank you, RTI */
  {
    hb->firstSN = toSN (wr->seq + 1);
    hb->lastSN = toSN (wr->seq);
  }
#endif
  hb->count = ++wr->hbcount;
  return m;
}

static serdata_t make_spdp_serdata_for_keyhash (guid_t guid)
{
  /* See make_pmd_serdata_for_keyhash. */
  return serialize_raw (serpool, osplser_topic4u, &guid);
}

static struct msg *make_SPDP_message (struct participant *pp)
{
  static const vendorid_t myvendorid = MY_VENDOR_ID;
  struct writer *wr = pp->spdp_pp_writer;
  struct msg *m;
  ptrdiff_t doff;
  Data_t *d;
  struct CDRHeader *plhdr;
  char *tmp;
  char *p;
  unsigned statusinfo = pp->dying ? (STATUSINFO_DISPOSE | STATUSINFO_UNREGISTER) : 0;
  unsigned dataflag = statusinfo ? 0 : DATA_FLAG_DATAFLAG;
  m = new_data_msg (&doff, wr, DATA_FLAG_INLINE_QOS | dataflag);
  m->serdata = make_spdp_serdata_for_keyhash (pp->guid);
  m = msg_add_parameter (m, &p, PID_KEY_HASH, 16);
  serdata_keyhash (m->serdata, p);
  if (pp->dying)
  {
    m = msg_add_parameter (m, &p, PID_STATUS_INFO, 4);
    /* For some odd reason, StatusInfo is an array of octets; we treat
       it as a 32-bit unsigned int, so on little-endian platforms, we
       need to byteswap it */
    *((unsigned *) p) =
      PLATFORM_IS_LITTLE_ENDIAN ? bswap4u (statusinfo) : statusinfo;
  }
  m = msg_add_parameter (m, &p, PID_SENTINEL, 0);
  if (!pp->dying)
  {
    m = msg_make_room (m, &tmp, sizeof (struct CDRHeader));
    plhdr = (struct CDRHeader *) tmp;
    plhdr->identifier = PLATFORM_IS_LITTLE_ENDIAN ? PL_CDR_LE : PL_CDR_BE;
    plhdr->options = 0;
    m = msg_add_parameter (m, &p, PID_PARTICIPANT_GUID, sizeof (guid_t));
    *((guid_t *) p) = hton_guid (pp->guid);
    m = msg_add_parameter (m, &p, PID_BUILTIN_ENDPOINT_SET, sizeof (builtin_endpoint_set_t));
    *((builtin_endpoint_set_t *) p) = pp->bes;
    m = msg_add_parameter (m, &p, PID_PROTOCOL_VERSION, sizeof (protocol_version_t));
    ((protocol_version_t *) p)->major = RTPS_MAJOR;
    ((protocol_version_t *) p)->minor = RTPS_MINOR;
    m = msg_add_parameter (m, &p, PID_VENDORID, sizeof (vendorid_t));
    *((vendorid_t *) p) = myvendorid;
    m = msg_add_parameter (m, &p, PID_DEFAULT_UNICAST_LOCATOR, sizeof (locator_t));
#if ! MANY_SOCKETS
    memcpy (p, &loc_default_uc, sizeof (loc_default_uc));
#else
    memcpy (p, &pp->sockloc, sizeof (pp->sockloc));
#endif
    m = msg_add_parameter (m, &p, PID_METATRAFFIC_UNICAST_LOCATOR, sizeof (locator_t));
#if ! MANY_SOCKETS
    memcpy (p, &loc_meta_uc, sizeof (loc_meta_uc));
#else
    memcpy (p, &pp->sockloc, sizeof (pp->sockloc));
#endif
    if (use_mcast_flag)
    {
      m = msg_add_parameter (m, &p, PID_DEFAULT_MULTICAST_LOCATOR, sizeof (locator_t));
      memcpy (p, &loc_default_mc, sizeof (loc_default_mc));
      m = msg_add_parameter (m, &p, PID_METATRAFFIC_MULTICAST_LOCATOR, sizeof (locator_t));
      memcpy (p, &loc_meta_mc, sizeof (loc_meta_mc));
    }
    m = msg_add_parameter (m, &p, PID_PARTICIPANT_LEASE_DURATION, sizeof (duration_t));
    if (pp->lease_duration != T_NEVER)
      *((duration_t *) p) = to_ddsi_time (pp->lease_duration);
    else
      *((duration_t *) p) = ddsi_time_infinite;
    m = msg_add_parameter (m, &p, PID_SENTINEL, 0);
  }
  d = (Data_t *) ((char *) m + doff);
  d->smhdr.octetsToNextHeader =
    m->len - (((char *) &d->smhdr.octetsToNextHeader + 2) - m->msg);
  return m;
}

static struct msg *make_AckNack_msg (struct rhc_writers_node *rwn)
{
  struct msg *m = new_msg ();
  int isnack;
  m = msg_add_Header (m, &rwn->reader_guid.prefix);
  m = msg_add_InfoDST (m, &rwn->writer_guid.prefix);
  m = msg_add_AckNack (m, rwn, &isnack);
  return m;
}

static struct msg *make_Heartbeat_msg (struct writer *wr, int *hbansreq)
{
  /*static guid_prefix_t null_guid_prefix;*/
  assert (wr->reliable);
  if (avl_empty (&wr->whc_seq))
  {
    /* do nothing cos we can't actually transmit valid heartbeats if
       there is no data */
    return NULL;
  }
  else
  {
    struct msg *m = new_msg ();
    m = msg_add_Header (m, &wr->guid.prefix);
    /*m = msg_add_InfoDST (m, &null_guid_prefix);*/
    m = msg_add_Heartbeat (m, wr, hbansreq, to_entityid (ENTITYID_UNKNOWN));
    return m;
  }
}

static int is_metaport (const locator_udpv4_t *addr)
{
  /* I really don't think anyone should care, and I'm not quite
     certain that it holds universally, but it does seem likely that
     an odd port is user traffic and an even one meta traffic. We use
     this to choose the socket over which to transmit a packet ... */
  return ((addr->port % 2) == 0);  
}

static size_t tx_do_send (tracebuf_t trb, const locator_udpv4_t *addr, const struct msg *m, int offset)
{
  struct sockaddr_in s;
  os_socket sock;
  os_int32 nbytes;
  const char *fn = NULL;
  assert (offset >= 0);
  assert (offset < m->len);
  assert ((unsigned) offset >= sizeof (Header_t));
  assert ((unsigned) offset == sizeof (Header_t) || (unsigned) offset == sizeof (Header_t) + sizeof (InfoDST_t));
  memset (&s, 0, sizeof (s));
  s.sin_family = AF_INET;
  s.sin_addr.s_addr = addr->address;
  s.sin_port = htons (addr->port);
  traceb (trb, DBGFLAG_TRACING, " %s:%d", inet_ntoa (s.sin_addr), addr->port);
  if (is_mcaddr (addr))
  {
    if (is_metaport (addr))
      sock = discsock_mc;
    else
      sock = datasock_mc;
  }
  else
  {
    if (is_metaport (addr))
      sock = discsock_uc;
    else
      sock = datasock_uc;
  }  

  if ((unsigned) offset == sizeof (Header_t))
  {
    fn = "sendto";
    nbytes = os_sockSendto (sock, m->msg, m->len, (struct sockaddr *) &s, sizeof (s));
  }
  else
  {
#if HAVE_OS_SOCKSENDMSG
    struct iovec iov[2];
    struct msghdr msghdr;
    iov[0].iov_base = (void *) m->msg;
    iov[0].iov_len = sizeof (Header_t);
    iov[1].iov_base = (void *) (m->msg + offset);
    iov[1].iov_len = m->len - offset;
    msghdr.msg_name = &s;
    msghdr.msg_namelen = sizeof (s);
    msghdr.msg_iov = iov;
    msghdr.msg_iovlen = 2;
#if __sun
    msghdr.msg_accrights = NULL;
    msghdr.msg_accrightslen = 0;
#else
    msghdr.msg_control = NULL;
    msghdr.msg_controllen = 0;
    msghdr.msg_flags = 0;
#endif
    fn = "sendmsg";
    nbytes = os_sockSendmsg (sock, &msghdr, 0);
#else /* HAVE_OS_SOCKSENDMSG */
    /* This is horrible! */
    int sz = sizeof (Header_t) + m->len - offset;
    char *m1 = os_malloc (sz);
    memcpy (m1, m->msg, sizeof (Header_t));
    memcpy (m1 + sizeof (Header_t), m->msg + offset, m->len - offset);
    fn = "sendto";
    nbytes = os_sockSendto (sock, m1, sz, (struct sockaddr *) &s, sizeof (s));
    os_free (m1);
#endif /* HAVE_OS_SOCKSENDMSG */
  }
  if (nbytes == -1)
  {
    print_sockerror (fn);
    return 0;
  }
  else
  {
    return (size_t) nbytes;
  }
}

struct txhelper_arg {
  tracebuf_t trb;
  const struct msg *msg;
  int offset;
};

static os_size_t txhelper (const locator_udpv4_t *addr, void *varg)
{
  const struct txhelper_arg *arg = varg;
  return tx_do_send (arg->trb, addr, arg->msg, arg->offset);
}

static os_size_t handle_xevk_heartbeat (tracebuf_t trb, struct xevent *ev)
{
  int hbansreq = -1; /* auto decide */
  struct msg *m = make_Heartbeat_msg (ev->u.heartbeat.wr, &hbansreq);
  struct writer *wr = ev->u.heartbeat.wr;
  os_size_t nbytes;
  int tsec, tusec;
  time_to_sec_usec (&tsec, &tusec, now ());

  if (m)
  {
    if (noqueue_heartbeat_messages_flag)
    {
      /* Fastest way to get a heartbeat out, but sometimes the
	 heartbeat will describe the future by mentioning the
	 existence of sequence numbers of which the initial
	 transmission is queued behind the heartbeat event. */
      struct txhelper_arg arg;
      assert (m->serdata == NULL);
      arg.trb = trb;
      arg.msg = m;
      arg.offset = sizeof (Header_t);
      traceb (trb, DBGFLAG_TRACING, "%d.%06d xmit heartbeat(wr %x:%x:%x:%x) [", tsec, tusec, PGUID (wr->guid));
      nbytes = addrset_forall_addresses (wr->as, txhelper, &arg);
      traceb (trb, DBGFLAG_TRACING, " ] ");
    }
    else
    {
      /* This should solve that by queueing the heartbeat */
      traceb (trb, DBGFLAG_TRACING, "%d.%06d queue heartbeat(wr %x:%x:%x:%x) msg %p\n", tsec, tusec, PGUID (wr->guid), m);
      qxev_msg (xevents, 0, wr->as, m);
      nbytes = 0;
    }
    if (hbansreq)
    {
      wr->t_of_last_heartbeat = now ();
    }
  }
  else
  {
    traceb (trb, DBGFLAG_TRACING, "%d.%06d skip heartbeat(wr %x:%x:%x:%x) ", tsec, tusec, PGUID (wr->guid));
    nbytes = 0;
  }
  unref_msg (m);
  if (avl_empty (&wr->whc_seq) || avl_empty (&wr->readers))
  {
    /* Can't transmit a valid heartbeat if there is no data; and it
       wouldn't actually be sent anywhere if there are no readers, so
       there is little point in processing the xevent all the time.

       Note that add_msg_to_whc and add_proxy_reader_to_writer will
       perform a reschedule. 8.4.2.2.3: need not (can't, really!) send
       a heartbeat if no data is available */
    traceb (trb, DBGFLAG_TRACING, "resched: never (whc empty or no readers)\n");
    resched_xevent (ev, T_NEVER);
  }
  else if (wr->readers.root->min_seq >= wr->whc_seq.root->maxseq)
  {
    /* Slightly different from requiring an empty whc_seq: if it is
       transient_local, whc_seq usually won't be empty even when all
       msgs have been ack'd. 8.4.2.2.3: need not send heartbeats
       when all messages have been acknowledged. */
    traceb (trb, DBGFLAG_TRACING, "resched: never (all acked)\n");
    resched_xevent (ev, T_NEVER);
  }
  else
  {
    /* 8.4.2.2.3: must send _periodic_ heartbeats */
    os_int64 tnow = now ();
    os_int64 mindelta = 10 * T_MILLISECOND;
    os_int64 maxdelta = (hbansreq ? 20 : 50) * T_MILLISECOND;
    os_int64 tnext = ev->tsched + maxdelta;
    if (tnext < tnow + mindelta)
      tnext = tnow + mindelta;
    traceb (trb, DBGFLAG_TRACING, "resched: in %gs (%lld < %lld)\n", time_to_double (tnext - tnow), wr->readers.root->min_seq, wr->whc_seq.root->maxseq);
    resched_xevent (ev, tnext);
  }
  return nbytes;
}

static os_size_t handle_xevk_acknack (tracebuf_t trb, struct xevent *ev)
{
  struct msg *m = make_AckNack_msg (ev->u.acknack.rwn);
  locator_udpv4_t addr;
  os_size_t nbytes;
  int tsec, tusec;
  time_to_sec_usec (&tsec, &tusec, now ());
  assert (m->serdata == NULL);
  if (addrset_any_uc (ev->u.acknack.rwn->proxy_writer->as, &addr) ||
      addrset_any_mc (ev->u.acknack.rwn->proxy_writer->as, &addr))
  {
    traceb (trb, DBGFLAG_TRACING, "%d.%06d xmit acknack(rd %x:%x:%x:%x -> pwr %x:%x:%x:%x) [", tsec, tusec, PGUID (ev->u.acknack.rwn->reader->guid), PGUID (ev->u.acknack.rwn->proxy_writer->guid));
    nbytes = tx_do_send (trb, &addr, m, sizeof (Header_t));
    traceb (trb, DBGFLAG_TRACING, " ]\n");
  }
  else
  {
    traceb (trb, DBGFLAG_TRACING, "%d.%06d skip acknack(rd %x:%x:%x:%x -> pwr %x:%x:%x:%x): no address\n", tsec, tusec, PGUID (ev->u.acknack.rwn->reader->guid), PGUID (ev->u.acknack.rwn->proxy_writer->guid));
    nbytes = 0;
  }
  unref_msg (m);
  /* not allowed to spontaneously send AckNacks, but we want to keep
     the event structure for eventual rescheduling upon receipt of a
     Heartbeat */
  resched_xevent (ev, T_NEVER);
  return nbytes;
}

static void extract_writer_guid_seq_from_msg (guid_t *guid, os_int64 *seq, const struct msg *m)
{
  guid_t g;
  sequence_number_t sn;
  const Data_t *data =
    (const Data_t *) (m->msg + sizeof (Header_t) + sizeof (InfoDST_t) + sizeof (InfoTS_t));
  assert (chk_outgoing_rtps_data_msg (m));
  memcpy (&g.prefix, &((const Header_t *) m->msg)->guid_prefix, sizeof (g.prefix));
  memcpy (&g.entityid, &data->writerId, sizeof (g.entityid));
  *guid = ntoh_guid (g);
  if (seq)
  {
    memcpy (&sn, &data->writerSN, sizeof (sn));
    *seq = fromSN (sn);
  }
}

static os_size_t handle_xevk_msg (tracebuf_t trb, struct xevent *ev)
{
  struct txhelper_arg arg;
  os_size_t nbytes;
  int tsec, tusec;
  time_to_sec_usec (&tsec, &tusec, now ());
  arg.trb = trb;
  arg.msg = ev->u.msg.msg;
  arg.offset = sizeof (Header_t);
  traceb (trb, DBGFLAG_TRACING, "%d.%06d xmit msg %p [", tsec, tusec, ev->u.msg.msg);
  nbytes = addrset_forall_addresses (ev->u.msg.dest, txhelper, &arg);
  traceb (trb, DBGFLAG_TRACING, " ]\n");
  delete_xevent (ev);

  return nbytes;
}

static os_size_t handle_xevk_data (tracebuf_t trb, struct xevent *ev)
{
  struct txhelper_arg arg;
  os_size_t nbytes;
  int tsec, tusec;
  time_to_sec_usec (&tsec, &tusec, now ());

  assert (ev->u.data.msg->serdata != NULL);
  arg.trb = trb;
  arg.msg = ev->u.data.msg;
  arg.offset = sizeof (Header_t) + sizeof (InfoDST_t);

  if(debugflags & DBGFLAG_TRACING){
    guid_t wrguid;
    os_int64 wrseq;
    char str[4096];
    const char *tail;
    int strsize = sizeof (str), res;
    const struct topic *topic = serdata_topic (arg.msg->serdata);
    extract_writer_guid_seq_from_msg (&wrguid, &wrseq, ev->u.data.msg);
    res = prettyprint_serdata (str, strsize, arg.msg->serdata);
    if (res >= 0) {
      if (res < strsize) tail = "";
      else tail = " (trunc)";
    } else {
      if (-res < strsize) tail = " (fail)";
      else tail = " (fail,trunc)";
    }
    traceb (trb, DBGFLAG_TRACING, "%d.%06d xmit data %x:%x:%x:%x #%lld %s/%s:%s%s [", tsec, tusec, PGUID (wrguid), wrseq, topic_name (topic), topic_typename (topic), str, tail);
  }

  nbytes = addrset_forall_addresses (ev->u.data.dest_all, txhelper, &arg);
  traceb (trb, DBGFLAG_TRACING, " ]\n");
  delete_xevent (ev);
  return nbytes;
}

static os_size_t handle_xevk_data_resend (tracebuf_t trb, struct xevent *ev)
{
  locator_udpv4_t addr;
  guid_t wrguid;
  os_size_t nbytes;
  int tsec, tusec;
  time_to_sec_usec (&tsec, &tusec, now ());
  assert (ev->u.data_resend.msg->serdata != NULL);
  extract_writer_guid_seq_from_msg (&wrguid, NULL, ev->u.data_resend.msg);
  if (ev->u.data_resend.prd == NULL)
  {
    struct txhelper_arg arg;
    assert (ev->u.data.msg->serdata != NULL);
    arg.trb = trb;
    arg.msg = ev->u.data_resend.msg;
    arg.offset = sizeof (Header_t) + sizeof (InfoDST_t);
    traceb (trb, DBGFLAG_TRACING, "%d.%06d rexmit data %x:%x:%x:%x [", tsec, tusec, PGUID (wrguid));
    nbytes = addrset_forall_addresses (ev->u.data_resend.dest_all, txhelper, &arg);
    traceb (trb, DBGFLAG_TRACING, " ]\n");
  }
  else if (addrset_any_uc (ev->u.data_resend.prd->as, &addr) ||
	   addrset_any_mc (ev->u.data_resend.prd->as, &addr))
  {
    /* rewrite destination */
    char *base = ev->u.data_resend.msg->msg;
    InfoDST_t *dst = (InfoDST_t *) (base + sizeof (Header_t));
    Data_t *data = (Data_t *) (base + sizeof (Header_t) + sizeof (InfoDST_t) + sizeof (InfoTS_t));
    assert (chk_outgoing_rtps_data_msg (ev->u.data_resend.msg));
    dst->guid_prefix = hton_guid_prefix (ev->u.data_resend.prd->guid.prefix);
    data->readerId = hton_entityid (ev->u.data_resend.prd->guid.entityid);
    traceb (trb, DBGFLAG_TRACING, "%d.%06d rexmit data %x:%x:%x:%x -> %x:%x:%x:%x [", tsec, tusec, PGUID (wrguid), PGUID (ev->u.data_resend.prd->guid));
    nbytes = tx_do_send (trb, &addr, ev->u.data_resend.msg, sizeof (Header_t));
    traceb (trb, DBGFLAG_TRACING, " ]\n");
  }
  else
  {
    traceb (trb, DBGFLAG_TRACING, "%d.%06d rexmit data %x:%x:%x:%x -> %x:%x:%x:%x skipped: no dst address\n", tsec, tusec, PGUID (wrguid), PGUID (ev->u.data_resend.prd->guid));
    nbytes = 0;
  }
  delete_xevent (ev);
  return nbytes;
}

static size_t handle_xevk_spdp (tracebuf_t trb, struct xevent *ev)
{
  char *base = ev->u.spdp.msg->msg;
  InfoTS_t *ts = (InfoTS_t *) (base + sizeof (Header_t) + sizeof (InfoDST_t));
  os_int64 tnow = now (), mindelta = 10 * T_MILLISECOND;
  os_int64 tnext = ev->tsched + 10 * T_SECOND;
  struct txhelper_arg arg;
  size_t nbytes;
  int tsec, tusec;
  time_to_sec_usec (&tsec, &tusec, now ());
  assert (chk_outgoing_rtps_data_msg (ev->u.spdp.msg));
  assert (ts->smhdr.submessageId == SMID_INFO_TS);
  /* Set time stamp just before transmitting it */
  ts->time = to_ddsi_time (tnow);
  arg.trb = trb;
  arg.msg = ev->u.spdp.msg;
  arg.offset = sizeof (Header_t);
  if (tnext < tnow + mindelta)
    tnext = tnow + mindelta;
  traceb (trb, DBGFLAG_TRACING, "%d.%06d xmit spdp (resched %gs) [", tsec, tusec, time_to_double (tnext - tnow));
  nbytes = addrset_forall_addresses (as_disc, txhelper, &arg);
  traceb (trb, DBGFLAG_TRACING, " ]\n");
  resched_xevent (ev, tnext);
  return nbytes;
}

static size_t handle_xevk_gap (tracebuf_t trb, struct xevent *ev)
{
  locator_udpv4_t addr;
  size_t nbytes;
  int tsec, tusec;
  time_to_sec_usec (&tsec, &tusec, now ());
  assert (ev->u.gap.msg->serdata == NULL);
  if (addrset_any_uc (ev->u.gap.prd->as, &addr) ||
      addrset_any_mc (ev->u.gap.prd->as, &addr))
  {
    traceb (trb, DBGFLAG_TRACING, "%d.%06d xmit gap to %x:%x:%x:%x [", tsec, tusec, PGUID (ev->u.gap.prd->guid));
    nbytes = tx_do_send (trb, &addr, ev->u.gap.msg, sizeof (Header_t));
    traceb (trb, DBGFLAG_TRACING, " ]\n");
  }
  else
  {
    traceb (trb, DBGFLAG_TRACING, "%d.%06d xmit gap skipped: no address for %x:%x:%x:%x\n", tsec, tusec, PGUID (ev->u.gap.prd->guid));
    nbytes = 0;
  }
  delete_xevent (ev);
  return nbytes;
}

static size_t handle_xevk_pmd_update (tracebuf_t trb, struct xevent *ev)
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
    traceb (trb, DBGFLAG_TRACING, "%d.%06d resched pmd(%x:%x:%x:%x): never\n", tsec, tusec, PGUID (pp->guid));
  }
  else
  {
    os_int64 tnow = now (), mindelta = 100 * T_MILLISECOND;
    tnext = ev->tsched + intv - 100 * T_MILLISECOND;
    if (tnext < tnow + mindelta)
      tnext = tnow + mindelta;
    traceb (trb, DBGFLAG_TRACING, "%d.%06d resched pmd(%x:%x:%x:%x): %gs\n", tsec, tusec, PGUID (pp->guid), time_to_double (tnext - tnow));
  }
  resched_xevent (ev, tnext);
  return 0;
}

static int info_trace (const char *fmt, ...)
{
  va_list ap;
  int n;
  va_start (ap, fmt);
  n = rtps_vtrace (DBGFLAG_INFO, fmt, ap);
  va_end (ap);
  return n;
}

static size_t handle_xevk_info (tracebuf_t trb, struct xevent *ev)
{
  static os_uint64 seq0, seq1;
  struct mlv_stats st;
  int tsec, tusec;
  time_to_sec_usec (&tsec, &tusec, now ());
  mlv_stats (&st);
  traceb (trb, DBGFLAG_INFO, "%d.%06d MLV %d %d %d %d #%llu\n", tsec, tusec, st.current, st.nblocks, st.nzeroblocks, st.nwhiledisabled, st.seq);
  tracebuf_output (trb);
  mlv_printlive (seq0, seq1, info_trace);
  seq0 = seq1;
  seq1 = st.seq + 1;
  resched_xevent (ev, now () + 60 * T_SECOND);
  return 0;
}

static size_t handle_xevents (void)
{
  os_int64 t = now ();
  int nhandled = 0;
  size_t nbytes = 0;
  TRACEBUF_DECLNEW (trb);
  while (nhandled++ < MAX_XEVENTS_BATCH_SIZE && earliest_in_xeventq (xevents) <= t)
  {
    struct xevent *ev = next_from_xeventq (xevents);
    switch (ev->kind)
    {
      case XEVK_HEARTBEAT:
	nbytes += handle_xevk_heartbeat (trb, ev);
	break;
      case XEVK_ACKNACK:
	nbytes += handle_xevk_acknack (trb, ev);
	break;
      case XEVK_MSG:
	nbytes += handle_xevk_msg (trb, ev);
	break;
      case XEVK_DATA:
	nbytes += handle_xevk_data (trb, ev);
	break;
      case XEVK_DATA_RESEND:
	nbytes += handle_xevk_data_resend (trb, ev);
	break;
      case XEVK_GAP:
	nbytes += handle_xevk_gap (trb, ev);
	break;
      case XEVK_SPDP:
	nbytes += handle_xevk_spdp (trb, ev);
	break;
      case XEVK_PMD_UPDATE:
	nbytes += handle_xevk_pmd_update (trb, ev);
	break;
      case XEVK_INFO:
	nbytes += handle_xevk_info (trb, ev);
	break;
    }
    tracebuf_output (trb);
  }
  TRACEBUF_FREE (trb);
  return nbytes;
}

static struct msg *new_data_msg (ptrdiff_t *doff, struct writer *wr, unsigned flags)
{
  static const guid_prefix_t nullprefix;
  struct msg *m = new_msg ();
  os_int64 tnow = now ();
  Data_t *d;
  char *tmp;
  m = msg_add_Header (m, &wr->guid.prefix);
  m = msg_add_InfoDST (m, &nullprefix);
  m = msg_add_InfoTimestamp (m, tnow);
  m = msg_make_room (m, &tmp, sizeof (Data_t));
  d = (Data_t *) tmp;
  d->smhdr.submessageId = SMID_DATA;
  d->smhdr.flags = (PLATFORM_IS_LITTLE_ENDIAN ? SMFLAG_ENDIANNESS : 0) | flags;
  /*d->smhdr.octetsToNextHeader;*/
  d->extraFlags = 0;
  /*d->octetsToInlineQos;*/
  d->readerId = hton_entityid (to_entityid (ENTITYID_UNKNOWN));
  d->writerId = hton_entityid (wr->guid.entityid);
  d->writerSN = toSN (++wr->seq);
  d->octetsToInlineQos =
    m->len - (((char *) &d->octetsToInlineQos + 2) - m->msg);
  *doff = (char *) d - (char *) m;
  return m;
}

static serdata_t make_pmd_serdata_for_keyhash (guid_prefix_t prefix, unsigned kind)
{
  /* Serialized form will have guid fields in LE form on LE machines,
     which is *wrong*, and therefore we can't really use the result
     from the serializer. There is one exception: the keyhash, which
     is always in BE form. */
  struct { guid_prefix_t pre; unsigned kind; } key;
  key.pre = prefix;
  key.kind = kind;
  return serialize_raw (serpool, osplser_topic4u, &key);
}

static void write_pmd_message (struct participant *pp)
{
  struct msg *m;
  struct writer *wr = pp->participant_message_writer;
  Data_t *d;
  ParticipantMessageData_t *pmd;
  ptrdiff_t doff;
  struct CDRHeader *cdr;
  const int pmd_kind = PARTICIPANT_MESSAGE_DATA_KIND_AUTOMATIC_LIVELINESS_UPDATE;
  char *p, *tmp;
  int tsec, tusec;
  time_to_sec_usec (&tsec, &tusec, now ());
  /* Even if others (Twinoaks, at least) don't do it, it should be ok
     to add a timestamp to PMDs */
  m = new_data_msg (&doff, wr, DATA_FLAG_INLINE_QOS | DATA_FLAG_DATAFLAG);
  m->serdata = make_pmd_serdata_for_keyhash (pp->guid.prefix, pmd_kind);
  m = msg_add_parameter (m, &p, PID_KEY_HASH, 16);
  serdata_keyhash (m->serdata, p);
  m = msg_add_parameter (m, &p, PID_SENTINEL, 0);
  m = msg_make_room (m, &tmp, sizeof (struct CDRHeader));
  cdr = (struct CDRHeader *) tmp;
  cdr->identifier = CDR_BE;
  cdr->options = 0;
  m = msg_make_room (m, &tmp, offsetof (ParticipantMessageData_t, value) + 4);
  pmd = (ParticipantMessageData_t *) tmp;
  pmd->participantGuidPrefix = hton_guid_prefix (pp->guid.prefix);
  pmd->kind = htonl (pmd_kind); /* octets, not integer */
  pmd->length = htonl (1); /* CDR length */
  memset (pmd->value, 0, 4);
  /*pmd->value[0] = 1;*/
  d = (Data_t *) ((char *) m + doff);
  d->smhdr.octetsToNextHeader =
    m->len - (((char *) &d->smhdr.octetsToNextHeader + 2) - m->msg);
  trace (DBGFLAG_TRACING, "%d.%06d pmd: write for %x:%x:%x:%x via %x:%x:%x:%x\n", tsec, tusec, PGUID (pp->guid), PGUID (wr->guid));
  rtps_write_msg (wr, m);
  unref_msg (m);
}

static serdata_t make_sedp_serdata_for_keyhash (guid_t epguid)
{
  /* See make_pmd_serdata_for_keyhash. */
  return serialize_raw (serpool, osplser_topic4u, &epguid);
}

static void sedp_write_endpoint_endoflife (struct writer *wr, guid_t epguid)
{
  struct msg *m;
  ptrdiff_t doff;
  Data_t *d;
  char *p;
  unsigned statusinfo = STATUSINFO_DISPOSE | STATUSINFO_UNREGISTER;
  /* RTI sends just the keyhash, so we might as well do the same, even
     though it is out-of-spec */
  m = new_data_msg (&doff, wr, DATA_FLAG_INLINE_QOS);
  m->serdata = make_sedp_serdata_for_keyhash (epguid);
  m = msg_add_parameter (m, &p, PID_KEY_HASH, 16);
  serdata_keyhash (m->serdata, p);
  m = msg_add_parameter (m, &p, PID_STATUS_INFO, 4);
  /* For some odd reason, StatusInfo is an array of octets; we treat
     it as a 32-bit unsigned int, so on little-endian platforms, we
     need to byteswap it */
  *((unsigned *) p) =
    PLATFORM_IS_LITTLE_ENDIAN ? bswap4u (statusinfo) : statusinfo;
  m = msg_add_parameter (m, &p, PID_SENTINEL, 0);
  d = (Data_t *) ((char *) m + doff);
  d->smhdr.octetsToNextHeader =
    m->len - (((char *) &d->smhdr.octetsToNextHeader + 2) - m->msg);
  trace (DBGFLAG_TRACING, "sedp: write end-of-life for %x:%x:%x:%x via %x:%x:%x:%x\n", PGUID (epguid), PGUID (wr->guid));
  rtps_write_msg (wr, m);
  unref_msg (m);
}

static void sedp_write_endpoint (struct writer *wr, guid_t epguid, unsigned epmode, const topic_t eptopic, const char *eppartition, os_int64 lease_duration)
{
  static const vendorid_t myvendorid = MY_VENDOR_ID;
  struct msg *m;
  ptrdiff_t doff;
  Data_t *d;
  struct CDRHeader *plhdr;
  char *p, *tmp;
  assert (eptopic);
  m = new_data_msg (&doff, wr, DATA_FLAG_INLINE_QOS | DATA_FLAG_DATAFLAG);
  m->serdata = make_sedp_serdata_for_keyhash (epguid);
  m = msg_add_parameter (m, &p, PID_KEY_HASH, 16);
  serdata_keyhash (m->serdata, p);
  m = msg_add_parameter (m, &p, PID_SENTINEL, 0);
  m = msg_make_room (m, &tmp, sizeof (struct CDRHeader));
  plhdr = (struct CDRHeader *) tmp;
  plhdr->identifier = PLATFORM_IS_LITTLE_ENDIAN ? PL_CDR_LE : PL_CDR_BE;
  plhdr->options = 0;
  m = msg_add_parameter (m, &p, PID_ENDPOINT_GUID, sizeof (guid_t));
  *((guid_t *) p) = hton_guid (epguid);
  m = msg_add_parameter (m, &p, PID_VENDORID, sizeof (vendorid_t));
  *((vendorid_t *) p) = myvendorid;
  m = msg_add_string_parameter (m, PID_TOPIC_NAME, topic_name (eptopic));
  m = msg_add_string_parameter (m, PID_TYPE_NAME, topic_typename (eptopic));
  if (eppartition && eppartition[0] != 0) /* partition defaults to "" */
    m = msg_add_stringseq_parameter (m, PID_PARTITION, 1, &eppartition);
  m = msg_add_parameter (m, &p, PID_RELIABILITY, sizeof (reliability_qos_t));
  ((reliability_qos_t *) p)->kind =
    (epmode & MF_RELIABLE) ? RELIABLE_RELIABILITY_QOS : BEST_EFFORT_RELIABILITY_QOS;
  ((reliability_qos_t *) p)->max_blocking_time = to_ddsi_time (T_SECOND);
  m = msg_add_parameter (m, &p, PID_OWNERSHIP, sizeof (ownership_qospolicy_t));
  ((ownership_qospolicy_t *) p)->kind =
    (epmode & MF_EXCLUSIVE_OWNERSHIP) ? EXCLUSIVE_OWNERSHIP_QOS : SHARED_OWNERSHIP_QOS;
  ((reliability_qos_t *) p)->max_blocking_time = to_ddsi_time (T_SECOND);
  m = msg_add_parameter (m, &p, PID_DURABILITY, sizeof (durability_kind_t));
  *((durability_kind_t *) p) = durability_from_mask (epmode);
  m = msg_add_parameter (m, &p, PID_PRESENTATION, sizeof (presentation_qospolicy_t));
  /* contains padding */
  memset (p, 0, sizeof (presentation_qospolicy_t));
  ((presentation_qospolicy_t *) p)->access_scope = access_scope_from_mask (epmode);
  ((presentation_qospolicy_t *) p)->ordered_access = 0;
  ((presentation_qospolicy_t *) p)->coherent_access = 0;
  m = msg_add_parameter (m, &p, PID_DESTINATION_ORDER, sizeof (destination_order_qospolicy_t));
  ((destination_order_qospolicy_t *) p)->kind = destination_order_from_mask (epmode);
  m = msg_add_parameter (m, &p, PID_PROTOCOL_VERSION, sizeof (protocol_version_t));
  ((protocol_version_t *) p)->major = RTPS_MAJOR;
  ((protocol_version_t *) p)->minor = RTPS_MINOR;
  m = msg_add_parameter (m, &p, PID_LIVELINESS, sizeof (liveliness_qospolicy_t));
  ((liveliness_qospolicy_t *) p)->kind = AUTOMATIC_LIVELINESS_QOS;
  if (lease_duration != T_NEVER)
    ((liveliness_qospolicy_t *) p)->lease_duration = to_ddsi_time (lease_duration);
  else
    ((liveliness_qospolicy_t *) p)->lease_duration = ddsi_time_infinite;
  /*DEADLINE = inf is default, but some version of Twinoaks'
    implementation used to crash on that */
  m = msg_add_parameter (m, &p, PID_DEADLINE, sizeof (deadline_qospolicy_t));
  ((deadline_qospolicy_t *) p)->deadline = ddsi_time_infinite;
  m = msg_add_parameter (m, &p, PID_SENTINEL, 0);
  d = (Data_t *) ((char *) m + doff);
  d->smhdr.octetsToNextHeader =
    m->len - (((char *) &d->smhdr.octetsToNextHeader + 2) - m->msg);
  trace (DBGFLAG_TRACING, "sedp: write for %x:%x:%x:%x via %x:%x:%x:%x\n", PGUID (epguid), PGUID (wr->guid));
  rtps_write_msg (wr, m);
  unref_msg (m);
}

static void sedp_write_writer (struct writer *wr, const struct writer *info)
{
  assert (!is_builtin_entityid (info->guid.entityid));
  assert (info->topic);
  if (!info->dying)
    sedp_write_endpoint (wr, info->guid, (info->reliable ? MF_RELIABLE : 0) | durability_to_mask (info->durability) | access_scope_to_mask (info->access_scope) | (info->exclusive_ownership ? MF_EXCLUSIVE_OWNERSHIP : 0) | destination_order_to_mask (info->destination_order), info->topic, info->partition, info->lease_duration);
  else
    sedp_write_endpoint_endoflife (wr, info->guid);
}

static void sedp_write_reader (struct writer *wr, const struct reader *info)
{
  assert (!is_builtin_entityid (info->guid.entityid));
  assert (info->topic);
  if (!info->dying)
    sedp_write_endpoint (wr, info->guid, (info->reliable ? MF_RELIABLE : 0) | durability_to_mask (info->durability) | access_scope_to_mask (info->access_scope) | (info->exclusive_ownership ? MF_EXCLUSIVE_OWNERSHIP : 0) | destination_order_to_mask (info->destination_order), info->topic, info->partition, info->lease_duration);
  else
    sedp_write_endpoint_endoflife (wr, info->guid);
}

static struct msg *append_writer_info (struct msg *m, C_STRUCT (v_message) const *srcmsg)
{
  char *p;
  struct rtps_prismtech_writer_info *wri;
  m = msg_add_parameter (m, &p, PID_PRISMTECH_WRITER_INFO, sizeof (*wri));
  wri = (struct rtps_prismtech_writer_info *) p;
  wri->transactionId = srcmsg->transactionId;
  wri->writerGID = srcmsg->writerGID;
  wri->writerInstanceGID = srcmsg->writerInstanceGID;
  return m;
}

static int rtps_write_unl (struct writer *wr, C_STRUCT (v_message) const *msg)
{
  struct msg *m;
  ptrdiff_t doff;
  Data_t *d;
  int sersize, sersize_a;
  char *p, *tmp;
  assert (wr->topic);
  m = new_data_msg (&doff, wr, DATA_FLAG_INLINE_QOS | DATA_FLAG_DATAFLAG);
  m->serdata = serialize (serpool, wr->topic, msg);
  if (debugflags & DBGFLAG_TRACING)
  {
    if (!serdata_verify (m->serdata, msg))
      trace (DBGFLAG_TRACING, "serdata_verify: fail\n");
  }
  sersize = serdata_size (m->serdata);
  sersize_a = ALIGN4 (sersize);
  if (topic_haskey (wr->topic))
  {
    m = msg_add_parameter (m, &p, PID_KEY_HASH, 16);
    serdata_keyhash (m->serdata, p);
  }
  m = append_writer_info (m, msg);
  m = msg_add_parameter (m, &p, PID_SENTINEL, 0);
  m = msg_make_room (m, &tmp, sersize_a);
  memcpy (tmp, &m->serdata->hdr, sersize);
  memset (tmp + sersize, 0, sersize_a - sersize);
  d = (Data_t *) ((char *) m + doff);
  d->smhdr.octetsToNextHeader =
    m->len - (((char *) &d->smhdr.octetsToNextHeader + 2) - m->msg);
#if 0
  trace (DBGFLAG_TRACING, "write via %x:%x:%x:%x\n", PGUID (wr->guid));
#endif
  rtps_write_msg (wr, m);
  unref_msg (m);
  return 0;
}

static int writer_must_throttle (const struct writer *wr)
{
  return wr->whc_seq_size - wr->whc_tlidx_size > WHC_HIGHWATER_MARK;
}

static int writer_may_continue (const struct writer *wr)
{
  return wr->whc_seq_size - wr->whc_tlidx_size < WHC_LOWWATER_MARK;
}

static void throttle_writer (struct writer *wr)
{
  int xt = xeventq_must_throttle (xevents);
  int wt = writer_must_throttle (wr);
  while (keepgoing && (xt || wt))
  {
    int tsec, tusec;
    
    time_to_sec_usec (&tsec, &tusec, now ());
    trace (DBGFLAG_TRACING, "%d.%06d writer %x:%x:%x:%x waiting for whc|evq to shrink below low-water mark (whc %d xevq %d)\n", tsec, tusec, PGUID (wr->guid), wr->whc_seq_size - wr->whc_tlidx_size, xeventq_size (xevents));
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
    trace (DBGFLAG_TRACING, "%d.%06d writer %x:%x:%x:%x done waiting for whc|evq to shrink below low-water mark (whc %d xevq %d)\n", tsec, tusec, PGUID (wr->guid), wr->whc_seq_size - wr->whc_tlidx_size, xeventq_size (xevents));
    
    xt = xeventq_must_throttle (xevents);
    wt = writer_must_throttle (wr);
  }
}

int rtps_write (struct writer *wr, C_STRUCT (v_message) const *msg)
{
  int r;
  os_mutexLock (&lock);
  throttle_writer (wr);
  r = rtps_write_unl (wr, msg);
  os_mutexUnlock (&lock);
  return r;
}

static int rtps_write_statusinfo (struct writer *wr, C_STRUCT (v_message) const *keymsg, unsigned statusinfo)
{
  struct msg *m;
  ptrdiff_t doff;
  Data_t *d;
  char *p;
  serdata_t serdata;
  int sersize, sersize_a;
  unsigned keyflag;
  assert (wr->topic);
  serdata = serialize_key (serpool, wr->topic, keymsg);
  sersize = serdata_size (serdata);
  sersize_a = ALIGN4 (sersize);
  keyflag = serdata_keyhash_exact_p (serdata) ? 0 : DATA_FLAG_KEYFLAG;
  m = new_data_msg (&doff, wr, DATA_FLAG_INLINE_QOS | keyflag);
  m->serdata = serdata;
  if (topic_haskey (wr->topic))
  {
    m = msg_add_parameter (m, &p, PID_KEY_HASH, 16);
    serdata_keyhash (m->serdata, p);
  }
  m = msg_add_parameter (m, &p, PID_STATUS_INFO, 4);
  /* For some odd reason, StatusInfo is an array of octets; we treat
     it as a 32-bit unsigned int, so on little-endian platforms, we
     need to byteswap it */
  *((unsigned *) p) =
    PLATFORM_IS_LITTLE_ENDIAN ? bswap4u (statusinfo) : statusinfo;
  m = append_writer_info (m, keymsg);
  m = msg_add_parameter (m, &p, PID_SENTINEL, 0);
  if (keyflag)
  {
    char *tmp;
    m = msg_make_room (m, &tmp, sersize_a);
    memcpy (tmp, &m->serdata->hdr, sersize);
    memset (tmp + sersize, 0, sersize_a - sersize);
  }
  d = (Data_t *) ((char *) m + doff);
  d->smhdr.octetsToNextHeader =
    m->len - (((char *) &d->smhdr.octetsToNextHeader + 2) - m->msg);
#if 0
  trace (DBGFLAG_TRACING, "write ST%x via %x:%x:%x:%x\n", statusinfo, PGUID (wr->guid));
#endif
  rtps_write_msg (wr, m);
  unref_msg (m);
  return 0;
}

int rtps_dispose (struct writer *wr, C_STRUCT (v_message) const *keymsg)
{
  int r;
  os_mutexLock (&lock);
  throttle_writer (wr);
  r = rtps_write_statusinfo (wr, keymsg, STATUSINFO_DISPOSE);
  os_mutexUnlock (&lock);
  return r;
}

int rtps_unregister (struct writer *wr, C_STRUCT (v_message) const *keymsg)
{
  int r;
  os_mutexLock (&lock);
  throttle_writer (wr);
  r = rtps_write_statusinfo (wr, keymsg, STATUSINFO_UNREGISTER);
  os_mutexUnlock (&lock);
  return r;
}

static int valid_sequence_number_set (const sequence_number_set_t *snset)
{
  if (fromSN (snset->bitmap_base) <= 0)
    return 0;
  if (snset->numbits <= 0 || snset->numbits > 256)
    return 0;
  return 1;
}

static int valid_AckNack (AckNack_t *msg, int size, int byteswap)
{
  count_t *count; /* this should've preceded the bitmap */
  if (size < (int) ACKNACK_SIZE (0))
    /* note: sizeof(*msg) is not sufficient verification, but it does
       suffice for verifying all fixed header fields exist */
    return 0;
  if (byteswap)
  {
    bswap_sequence_number_set_hdr (&msg->readerSNState);
    /* bits[], count deferred until validation of fixed part */
  }
  msg->readerId = ntoh_entityid (msg->readerId);
  msg->writerId = ntoh_entityid (msg->writerId);
  /* Validation following 8.3.7.1.3 + 8.3.5.5 */
  if (!valid_sequence_number_set (&msg->readerSNState))
  {
    if (strict_validation)
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
  count = (count_t *) ((char *) &msg->readerSNState +
		       SEQUENCE_NUMBER_SET_SIZE (msg->readerSNState.numbits));
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
  msg->readerId = ntoh_entityid (msg->readerId);
  msg->writerId = ntoh_entityid (msg->writerId);
  if (fromSN (msg->gapStart) <= 0)
    return 0;
  if (!valid_sequence_number_set (&msg->gapList))
  {
    if (strict_validation || msg->gapList.numbits != 0)
      return 0;
  }
  if (size < (int) GAP_SIZE (msg->gapList.numbits))
    return 0;
  if (byteswap)
    bswap_sequence_number_set_bitmap (&msg->gapList);
  return 1;
}

static int valid_InfoDST (InfoDST_t *msg, int size, int byteswap UNUSED)
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
  msg->readerId = ntoh_entityid (msg->readerId);
  msg->writerId = ntoh_entityid (msg->writerId);
  /* Validation following 8.3.7.5.3 */
  if (fromSN (msg->firstSN) <= 0 ||
      /* fromSN (msg->lastSN) <= 0 || -- implicit in last < first */
      fromSN (msg->lastSN) < fromSN (msg->firstSN))
  {
    if (strict_validation)
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

static char *nextafter_parameter_list (struct CDRHeader *hdr, char *pl, char *plend)
{
  /* note: last byte of pl must be at (plend-1) */
  int qosbswap;
  if (hdr->identifier != PL_CDR_LE && hdr->identifier != PL_CDR_BE)
  {
    /* ParameterLists are supposed to be encapsulated as
       PL_CDR_{LE,BE} */
    return NULL;
  }
  qosbswap = pl_needs_bswap (hdr);
  /*trace (DBGFLAG_TRACING, "pl_needs_bswap = %d\n", qosbswap);*/
  while (pl + sizeof (parameter_t) <= plend)
  {
    parameter_t *par = (parameter_t *) pl;
    unsigned short pid, length;
    /* swapping header partially based on wireshark dissector
       output, partially on intuition, and in a small part based on
       the spec */
    pid = qosbswap ? bswap2u (par->parameterid) : par->parameterid;
    length = qosbswap ? bswap2u (par->length) : par->length;
    if (pid == PID_SENTINEL)
    {
      /* Sentinel terminates list, hen lenth is ignored, DDSI 9.4.2.11 */
      /*trace (DBGFLAG_TRACING, "%4x PID %x\n", (unsigned) (pl - (char *) hdr), pid);*/
      return pl + sizeof (*par);
    }
    else
    {
      if ((length % 4) != 0) /* DDSI 9.4.2.11 */
	return NULL;
      /*trace (DBGFLAG_TRACING, "%4x PID %x len %d\n", (unsigned) (pl - (char *) hdr), pid, length);*/
      pl += sizeof (*par) + length;
    }
  }
  /* If we get here, that means we reached the end of the message
     without encountering a sentinel. That is an error */
  trace (DBGFLAG_TRACING, "PL structure not good - sentinel missing (pl %p plend %p)\n", pl, plend);
  return NULL;
}

static int valid_Data (Data_t *msg, int size, int byteswap, parameter_t **qosp, struct CDRHeader **payloadp)
{
  char *ptr;
  *qosp = NULL;
  *payloadp = NULL;
  if (size < (int) sizeof (*msg))
    return 0; /* too small even for fixed fields */
  /* D=1 && K=1 is invalid in this version of the protocol */
  if ((msg->smhdr.flags & (DATA_FLAG_DATAFLAG | DATA_FLAG_KEYFLAG)) == (DATA_FLAG_DATAFLAG | DATA_FLAG_KEYFLAG))
    return 0;
  if (byteswap)
  {
    msg->extraFlags = bswap2u (msg->extraFlags);
    msg->octetsToInlineQos = bswap2u (msg->octetsToInlineQos);
    bswapSN (&msg->writerSN);
  }
  msg->readerId = ntoh_entityid (msg->readerId);
  msg->writerId = ntoh_entityid (msg->writerId);
  if ((msg->smhdr.flags & (DATA_FLAG_INLINE_QOS | DATA_FLAG_DATAFLAG | DATA_FLAG_KEYFLAG)) == 0)
  {
    /* no QoS, no payload, so octetsToInlineQos will never be used
       though one would expect octetsToInlineQos and size to be in
       agreement or octetsToInlineQos to be 0 or so */
    return 1;
  }

  /* QoS and/or payload, so octetsToInlineQos must be within the
     msg; since the serialized data and serialized parameter lists
     have a 4 byte header, that one, too must fit */
  if ((int) (offsetof (Data_t, octetsToInlineQos) + sizeof (msg->octetsToInlineQos) + 4) >= size)
    return 0;

  ptr = (char *) msg + offsetof (Data_t, octetsToInlineQos) + sizeof (msg->octetsToInlineQos) + msg->octetsToInlineQos;
  if (!(msg->smhdr.flags & DATA_FLAG_INLINE_QOS))
  {
    /*trace (DBGFLAG_TRACING, "no inline QoS\n");*/
    *qosp = NULL;
  }
  else
  {
    struct CDRHeader qoshdr;
    *qosp = (parameter_t *) ptr;
    qoshdr.identifier = (msg->smhdr.flags & SMFLAG_ENDIANNESS) ? PL_CDR_LE : PL_CDR_BE;
    qoshdr.options = 0;
    /*trace (DBGFLAG_TRACING, "inline QoS\n");*/
    if ((ptr = nextafter_parameter_list (&qoshdr, ptr, (char *) msg + size)) == NULL)
      /* invalid parameter list structure */
      return 0;
  }

  if (!(msg->smhdr.flags & (DATA_FLAG_DATAFLAG | DATA_FLAG_KEYFLAG)))
  {
    /*trace (DBGFLAG_TRACING, "no payload\n");*/
    *payloadp = NULL;
  }
  else /*if (msg->writerId.u != ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_WRITER)*/
  {
    *payloadp = (struct CDRHeader *) ptr;
    if ((*payloadp)->identifier == PL_CDR_BE || (*payloadp)->identifier == PL_CDR_LE)
    {
      /*trace (DBGFLAG_TRACING, "payload is PL (%p, %p - %p)\n", (char *) msg, ptr, (char *) msg + size);*/
      ptr = nextafter_parameter_list (*payloadp, ptr + 4, (char *) msg + size);
    }
    else if ((*payloadp)->identifier == CDR_BE || (*payloadp)->identifier == CDR_LE)
    {
      /* assume properly serialized data */
      /*trace (DBGFLAG_TRACING, "payload is data\n");*/
      ptr = (char *) msg + size;
    }
    else
    {
      /*trace (DBGFLAG_TRACING, "payload has unrecognized CDR header\n");*/
      return 0;
    }
  }

  /*trace (DBGFLAG_TRACING, "%p %p\n", ptr, (char *) msg + size);*/
  if (ptr != (char *) msg + size)
    return 0;
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
    if (newsize < WHC_LOWWATER_MARK && oldsize >= WHC_LOWWATER_MARK)
      os_condBroadcast (&throttle_cond);
  }
  return arg.count;
}

static struct msg *find_old_msg (const struct writer *wr, os_int64 seq)
{
  struct whc_node *n;
  if ((n = avl_lookup (&wr->whc_seq, &seq, NULL)) == NULL)
    return NULL;
  else
    return n->msg;
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


static struct msg *make_Gap_msg (struct whc_readers_node *wrn, os_int64 start, os_int64 base, int numbits, const unsigned *bits)
{
  struct msg *m = new_msg ();
  m = msg_add_Header (m, &wrn->writer_guid.prefix);
  m = msg_add_InfoDST (m, &wrn->reader_guid.prefix);
  m = msg_add_Gap (m, wrn, start, base, numbits, bits);
  return m;
}

static int acknack_is_nack (const AckNack_t *msg)
{
  unsigned x = 0, mask;
  int i;
  if (msg->readerSNState.numbits == 0)
    /* Disallowed by the spec, but RTI appears to require them (and so
       even we generate them) */
    return 0;
  for (i = 0; i < (int) SEQUENCE_NUMBER_SET_BITS_SIZE (msg->readerSNState.numbits) / 4 - 1; i++)
    x |= msg->readerSNState.bits[i];
  if ((msg->readerSNState.numbits % 32) == 0)
    mask = ~0u;
  else
    mask = ~(~0u >> (msg->readerSNState.numbits % 32));
  x |= msg->readerSNState.bits[i] & mask;
  return x != 0;
}

static void force_heartbeat_to_peer (struct whc_readers_node *rn, os_int64 tsched, int hbansreq)
{
  struct writer *wr = rn->writer;
  struct msg *m;
  assert (wr->reliable);
  /* If WHC is empty, we can't generate a valid heartbeat, so we
     postpone it until just before transmitting the next data message
     from this writer (and just after adding that message to the
     WHC). */
#if ! RESPOND_TO_RTI_INIT_ZERO_ACK_WITH_INVALID_HEARTBEAT
  if (avl_empty (&wr->whc_seq))
  {
    wr->hb_before_next_msg = 1;
    return;
  }
#endif
  /* Send a Heartbeat just to this peer */
  m = new_msg ();
  m = msg_add_Header (m, &wr->guid.prefix);
  m = msg_add_InfoDST (m, &rn->proxy_reader->guid.prefix);
  m = msg_add_Heartbeat (m, wr, &hbansreq, rn->proxy_reader->guid.entityid);
  qxev_msg (xevents, tsched, rn->proxy_reader->as, m);
  unref_msg (m);
}

static int handle_AckNack (struct receiver_state *rst, const AckNack_t *msg, int size UNUSED)
{
  struct proxy_endpoint *ep;
  struct whc_readers_node *rn;
  guid_t src, dst;
  os_int64 seqbase;
  count_t *countp;
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
  memset (gapbits, 0, sizeof (gapbits));
  countp = (count_t *) ((char *) msg + offsetof (AckNack_t, readerSNState) + SEQUENCE_NUMBER_SET_SIZE (msg->readerSNState.numbits));
  src.prefix = rst->source_guid_prefix;
  src.entityid = msg->readerId;
  dst.prefix = rst->dest_guid_prefix;
  dst.entityid = msg->writerId;
  trace (DBGFLAG_TRACING, "ACKNACK(#%d:%lld/%d:", *countp, fromSN (msg->readerSNState.bitmap_base), msg->readerSNState.numbits);
  for (i = 0; i < (int) msg->readerSNState.numbits; i++)
    trace (DBGFLAG_TRACING, "%c", msg->readerSNState.bits[i/32] & (1u << (31 - (i%32))) ? '1' : '0');
  if ((ep = avl_lookup (&proxyeptree, &src, NULL)) == NULL)
  {
    trace (DBGFLAG_TRACING, " %x:%x:%x:%x? -> %x:%x:%x:%x)", PGUID (src), PGUID (dst));
    return 1;
  }
  if ((rn = avl_lookup (&ep->matched_locals.writers, &dst, NULL)) == NULL)
  {
    trace (DBGFLAG_TRACING, " %x:%x:%x:%x -> %x:%x:%x:%x?)", PGUID (src), PGUID (dst));
    chk_no_such_writer (dst);
    return 1;
  }
  if (!rn->writer->reliable)
  {
    trace (DBGFLAG_TRACING, " %x:%x:%x:%x -> %x:%x:%x:%x not a reliable writer!)", PGUID (src), PGUID (dst));
    return 1;
  }
  whc_was_not_empty = !avl_empty (&rn->writer->whc_seq);
  if (*countp <= rn->last_acknack)
  {
    trace (DBGFLAG_TRACING, " [%x:%x:%x:%x -> %x:%x:%x:%x])", PGUID (src), PGUID (dst));
    return 1;
  }
  rn->last_acknack = *countp;
  seqbase = fromSN (msg->readerSNState.bitmap_base);
  trace (DBGFLAG_TRACING, " %x:%x:%x:%x -> %x:%x:%x:%x", PGUID (src), PGUID (dst));
  if (seqbase - 1 > rn->seq)
  {
    os_int64 n_ack = (seqbase - 1) - rn->seq;
    int n;
    rn->seq = seqbase - 1;
    avl_augment_update (&rn->writer->readers, rn);
    n = remove_acked_messages (rn->writer);
    trace (DBGFLAG_TRACING, " ACK%lld RM%d", n_ack, n);
  }
  numbits = (int) msg->readerSNState.numbits;
  is_pure_ack = !acknack_is_nack (msg);
  msgs_sent = 0;
  max_seq_in_reply = 0;
  if (seqbase == 0 && numbits == 0)
  {
    trace (DBGFLAG_TRACING, " RTI-zero-ack");
    if (avl_empty (&rn->writer->whc_seq))
    {
      trace (DBGFLAG_TRACING, " whc empty");
      force_heartbeat_to_peer (rn, -2, 0);
    }
    else
    {
      trace (DBGFLAG_TRACING, " rebase");
      force_heartbeat_to_peer (rn, -2, 0);
      numbits = ACCELERATE_REXMIT_BLOCKSIZE;
      seqbase = rn->writer->whc_seq.root->minseq;
    }
  }
  else if (!rn->pure_ack_received)
  {
    if (is_pure_ack)
    {
      trace (DBGFLAG_TRACING, " happy now");
      rn->pure_ack_received = 1;
    }
    else if ((int) msg->readerSNState.numbits < ACCELERATE_REXMIT_BLOCKSIZE)
    {
      trace (DBGFLAG_TRACING, " accelerating");
      accelerate_rexmit = 1;
      if (accelerate_rexmit && numbits < ACCELERATE_REXMIT_BLOCKSIZE)
	numbits = ACCELERATE_REXMIT_BLOCKSIZE;
    }
    else
    {
      trace (DBGFLAG_TRACING, " complying");
    }
  }
  for (i = 0; i < numbits && seqbase + i <= rn->writer->seq; i++)
  {
    /* Accelerated schedule may run ahead of sequence number set
       contained in the acknack, and assumes all messages beyond the
       set are NAK'd -- don't feel like tracking where exactly we left
       off ... */
    if (i >= (int) msg->readerSNState.numbits ||
	msg->readerSNState.bits[i/32] & (1u << (31 - (i%32))))
    {
      struct msg *m;
      if ((m = find_old_msg (rn->writer, seqbase + i)) != NULL)
      {
	trace (DBGFLAG_TRACING, " RX%lld", seqbase + i);
	qxev_data_resend (xevents, 0, rn->proxy_reader, rn->writer->as, m);
	max_seq_in_reply = seqbase + i;
	msgs_sent++;
      }
      else if (gapstart == -1)
      {
	trace (DBGFLAG_TRACING, " M%lld", seqbase + i);
	gapstart = seqbase + i;
	gapend = gapstart + 1;
      }
      else if (seqbase + i == gapend)
      {
	trace (DBGFLAG_TRACING, " M%lld", seqbase + i);
	gapend = seqbase + i + 1;
      }
      else if (seqbase + i - gapend < 256)
      {
	int idx = (int) (seqbase + i - gapend);
	trace (DBGFLAG_TRACING, " M%lld", seqbase + i);
	gapnumbits = idx + 1;
	gapbits[idx/32] |= 1u << (31 - (idx%32));
      }
    }
  }
  if (gapstart > 0)
  {
    struct msg *m;
    if (gapend == seqbase + msg->readerSNState.numbits)
    {
      gapend = grow_gap_to_next_seq (rn->writer, gapend);
    }
    if (gapnumbits == 0)
    {
      gapnumbits = 1;
      gapbits[0] = 1u << 31;
      gapend--;
    }
    /* The non-bitmap part of a gap message says everything <=
       gapend-1 is no more (so the maximum sequence number it informs
       the peer of is gapend-1); each bit adds one sequence number to
       that. */
    if (gapend-1 + gapnumbits > max_seq_in_reply)
      max_seq_in_reply = gapend-1 + gapnumbits;
    trace (DBGFLAG_TRACING, " XGAP%lld..%lld/%d:", gapstart, gapend, gapnumbits);
    for (i = 0; i < gapnumbits; i++)
      trace (DBGFLAG_TRACING, "%c", gapbits[i/32] & (1u << (31 - (i%32))) ? '1' : '0');
    m = make_Gap_msg (rn, gapstart, gapend, gapnumbits, gapbits);
    qxev_gap (xevents, -1, ep, m);
    unref_msg (m);
    msgs_sent++;
  }
  /* If rexmits and/or a gap message were sent, and if the last
     sequence number that we're informing the Nack'ing peer about is
     less than the last sequence number transmitted by the writer,
     tell the peer to acknowledge quickly. Not sure if that helps, but
     it might ... [NB writer->seq is the last msg sent so far] */
  if (msgs_sent && max_seq_in_reply < rn->writer->seq)
  {
    trace (DBGFLAG_TRACING, " HB(#sent:%d maxseq:%lld<%lld)", msgs_sent, max_seq_in_reply, rn->writer->seq);
    force_heartbeat_to_peer (rn, 0, 1);
  }
  trace (DBGFLAG_TRACING, ")");
  return 1;
}

struct handle_Heartbeat_helper_arg {
  struct receiver_state *rst;
  const Heartbeat_t *msg;
  guid_t src, dst;
  struct proxy_endpoint *ep;
  os_int64 firstseq;
};

static int handle_Heartbeat_helper (void *vnode, void *varg)
{
  struct rhc_writers_node *wn = vnode;
  struct handle_Heartbeat_helper_arg *arg = varg;
  if (arg->msg->count <= wn->last_heartbeat)
  {
    trace (DBGFLAG_TRACING, " (%x:%x:%x:%x)", PGUID (wn->reader->guid));
    return 1;
  }
  wn->last_heartbeat = arg->msg->count;
  trace (DBGFLAG_TRACING, " %x:%x:%x:%x@%lld", PGUID (wn->reader->guid), wn->seq);
  if (arg->firstseq > wn->seq + 1)
  {
    trace (DBGFLAG_TRACING, "/LOST%lld", (arg->firstseq - 1) - wn->seq);
    wn->seq = arg->firstseq - 1;
    /* maybe we can now handoff data */
    osa_handoff (wn);
  }
  /* Reschedule AckNack transmit if deemed appropriate; unreliable
     readers have acknack_xevent == NULL and can't do this. */
  if (wn->acknack_xevent)
  {
    if (wn->proxy_writer->u.wr.last_seq > wn->seq)
    {
      trace (DBGFLAG_TRACING, "/NAK");
      resched_xevent_if_earlier (wn->acknack_xevent, 0);
    }
    else if (!(arg->msg->smhdr.flags & HEARTBEAT_FLAG_FINAL))
    {
      resched_xevent_if_earlier (wn->acknack_xevent, 0);/* now () + 0 * T_MILLISECOND); */
    }
    else
    {
      resched_xevent_if_earlier (wn->acknack_xevent, now () + 7 * T_SECOND);
    }
  }
  return AVLWALK_CONTINUE;
}

static void handle_forall_destinations (const guid_t *src, const guid_t *dst, struct proxy_endpoint *ep, avlwalk_fun_t fun, void *arg, int print_addrs)
{
  /* prefix:  id:   to:
     0        0     all matched readers
     0        !=0   all matched readers with entityid id
     !=0      0     to all matched readers in addressed participant
     !=0      !=0   to the one addressed reader
  */
  const int haveprefix =
    !(dst->prefix.u[0] == 0 && dst->prefix.u[1] == 0 && dst->prefix.u[2] == 0);
  const int haveid = !(dst->entityid.u == ENTITYID_UNKNOWN);
  switch ((haveprefix << 1) | haveid)
  {
    case (0 << 1) | 0: /* all: full treewalk */
      if (print_addrs) trace (DBGFLAG_TRACING, "%x:%x:%x:%x -> %x:%x:%x:%x:", PGUID (*src), PGUID (*dst));
      avl_walk (&ep->matched_locals.readers, fun, arg);
      break;
    case (0 << 1) | 1: /* all with correct entityid: special filtering treewalk */
      {
	struct rhc_writers_node *wn;
	if (print_addrs) trace (DBGFLAG_TRACING, "%x:%x:%x:%x -> %x:%x:%x:%x:", PGUID (*src), PGUID (*dst));
	wn = avl_findmin (&ep->matched_locals.readers);
	while (wn)
	{
	  if (wn->reader_guid.entityid.u == dst->entityid.u)
	    fun (wn, arg);
	  wn = avl_findsucc (&ep->matched_locals.readers, wn);
	}
      }
      break;
    case (1 << 1) | 0: /* all within one participant: walk a range of keyvalues */
      {
	guid_t a, b;
	if (print_addrs) trace (DBGFLAG_TRACING, "%x:%x:%x:%x -> %x:%x:%x:%x:", PGUID (*src), PGUID (*dst));
	a = *dst; a.entityid.u = 0;
	b = *dst; b.entityid.u = ~0;
	avl_walkrange (&ep->matched_locals.readers, &a, &b, fun, arg);
      }
      break;
    case (1 << 1) | 1: /* fully addressed: dst should exist (but for removal) */
      {
	struct rhc_writers_node *wn;
	if ((wn = avl_lookup (&ep->matched_locals.readers, dst, NULL)) != NULL)
	{
	  if (print_addrs) trace (DBGFLAG_TRACING, "%x:%x:%x:%x -> %x:%x:%x:%x:", PGUID (*src), PGUID (*dst));
	  fun (wn, arg);
	}
	else
	{
	  if (print_addrs) trace (DBGFLAG_TRACING, "%x:%x:%x:%x -> %x:%x:%x:%x?", PGUID (*src), PGUID (*dst));
	  chk_no_such_reader (*dst);
	}
      }
      break;
    default:
      abort ();
  }
}

static int handle_Heartbeat (struct receiver_state *rst, const Heartbeat_t *msg, int size UNUSED)
{
  struct handle_Heartbeat_helper_arg arg;
  struct proxy_endpoint *ep;
  arg.rst = rst;
  arg.msg = msg;
  arg.src.prefix = rst->source_guid_prefix;
  arg.src.entityid = msg->writerId;
  arg.dst.prefix = rst->dest_guid_prefix;
  arg.dst.entityid = msg->readerId;
  arg.firstseq = fromSN (msg->firstSN);
  trace (DBGFLAG_TRACING, "HB(%s#%d:%lld..%lld ", msg->smhdr.flags & HEARTBEAT_FLAG_FINAL ? "F" : "", msg->count, arg.firstseq, fromSN (msg->lastSN));
  if ((ep = avl_lookup (&proxyeptree, &arg.src, NULL)) == NULL)
  {
    trace (DBGFLAG_TRACING, "%x:%x:%x:%x? -> %x:%x:%x:%x)", PGUID (arg.src), PGUID (arg.dst));
    return 1;
  }
  arg.ep = ep;
  if (fromSN (msg->lastSN) > ep->u.wr.last_seq)
    ep->u.wr.last_seq = fromSN (msg->lastSN);
  handle_forall_destinations (&arg.src, &arg.dst, ep, handle_Heartbeat_helper, &arg, 1);
  trace (DBGFLAG_TRACING, ")");
  return 1;
}

static int handle_InfoDST (struct receiver_state *rst, const InfoDST_t *msg, int size UNUSED)
{
  rst->dest_guid_prefix = ntoh_guid_prefix (msg->guid_prefix);
  trace (DBGFLAG_TRACING, "DST(%x:%x:%x)", rst->dest_guid_prefix.u[0], rst->dest_guid_prefix.u[1], rst->dest_guid_prefix.u[2]);
  return 1;
}

static int handle_InfoTS (struct receiver_state *rst, const InfoTS_t *msg, int size UNUSED)
{
  trace (DBGFLAG_TRACING, "TS(");
  if (msg->smhdr.flags & INFOTS_INVALIDATE_FLAG)
  {
    rst->have_timestamp = 0;
    trace (DBGFLAG_TRACING, "invalidate");
  }
  else
  {
    rst->timestamp = msg->time;
    rst->have_timestamp = 1;
    trace (DBGFLAG_TRACING, "%d.%08x", rst->timestamp.seconds, rst->timestamp.fraction);
  }
  trace (DBGFLAG_TRACING, ")");
  return 1;
}

static int handle_Gap (struct receiver_state *rst, const Gap_t *msg, int size UNUSED)
{
  struct proxy_endpoint *ep;
  struct rhc_writers_node *wn;
  guid_t src, dst;
  os_int64 gapstart, listbase, end;
  int i;
  src.prefix = rst->source_guid_prefix;
  src.entityid = msg->writerId;
  dst.prefix = rst->dest_guid_prefix;
  dst.entityid = msg->readerId;
  gapstart = fromSN (msg->gapStart);
  listbase = fromSN (msg->gapList.bitmap_base);
  trace (DBGFLAG_TRACING, "GAP(%lld..%lld/%d ", gapstart, listbase, msg->gapList.numbits);
  if ((ep = avl_lookup (&proxyeptree, &src, NULL)) == NULL)
  {
    trace (DBGFLAG_TRACING, "%x:%x:%x:%x? -> %x:%x:%x:%x)", PGUID (src), PGUID (dst));
    return 1;
  }
  if ((wn = avl_lookup (&ep->matched_locals.readers, &dst, NULL)) == NULL)
  {
    trace (DBGFLAG_TRACING, "%x:%x:%x:%x -> %x:%x:%x:%x?)", PGUID (src), PGUID (dst));
    chk_no_such_reader (dst);
    return 1;
  }
  trace (DBGFLAG_TRACING, "%x:%x:%x:%x -> %x:%x:%x:%x", PGUID (src), PGUID (dst));
  for (i = 0; i < (int) msg->gapList.numbits; i++)
    if (!(msg->gapList.bits[i/32] & (1u << (31 - (i%32)))))
      break;
  end = listbase + i - 1;
  if (gapstart <= wn->seq + 1 && wn->seq < end)
  {
    trace (DBGFLAG_TRACING, " LOST%lld", end - wn->seq);
    wn->seq = end;
  }
  if (wn->seq < end)
  {
    osa_remember_gap (&wn->osa, gapstart, end - gapstart + 1);
  }
  if (end > ep->u.wr.last_seq)
    ep->u.wr.last_seq = end;
  trace (DBGFLAG_TRACING, ")");
  return 1;
}

static void *pl_findnextnaked (const parameter_t *fstpar, int bswap, const void *cursor, parameterid_t soughtpid)
{
  unsigned short pid, length;
  char *pl;
  if (fstpar == NULL)
    return NULL;
  if (cursor == NULL)
    pl = (char *) fstpar;
  else
    pl = (char *) cursor - sizeof (parameter_t);
  do {
    parameter_t *par = (parameter_t *) pl;
    pid = bswap ? bswap2u (par->parameterid) : par->parameterid;
    length = bswap ? bswap2u (par->length) : par->length;
    if (pid == soughtpid && (void *) (par + 1) != cursor)
      return (void *) (par + 1);
    pl += sizeof (*par) + length;
  } while (pid != PID_SENTINEL);
  return NULL;
}

static int pl_getlength (const void *parcontents, int bswap)
{
  const parameter_t *par;
  if (parcontents == NULL)
    return 0;
  par = (const parameter_t *) parcontents - 1;
  return bswap ? (int) bswap2u (par->length) : (int) par->length;
}

static void *pl_findnaked (const parameter_t *fstpar, int bswap, parameterid_t soughtpid)
{
  return pl_findnextnaked (fstpar, bswap, NULL, soughtpid);
}

static void *pl_find (const struct CDRHeader *hdr, parameterid_t soughtpid)
{
  const int bswap = pl_needs_bswap (hdr);
  parameter_t *fstpar = (parameter_t *) (hdr + 1);
  return pl_findnaked (fstpar, bswap, soughtpid);
}

static void *pl_findnext (const struct CDRHeader *hdr, const void *cursor, parameterid_t soughtpid)
{
  const int bswap = pl_needs_bswap (hdr);
  parameter_t *fstpar = (parameter_t *) (hdr + 1);
  return pl_findnextnaked (fstpar, bswap, cursor, soughtpid);
}

static int add_disc_sedp_writer_writer (void *vnode, void *varg)
{
  struct participant *pp = vnode;
  struct proxy_endpoint *ep = varg;
  if (pp->sedp_writer_reader)
    add_proxy_writer_to_reader (pp->sedp_writer_reader, ep);
  return AVLWALK_CONTINUE;
}

static int add_disc_sedp_writer_reader (void *vnode, void *varg)
{
  struct participant *pp = vnode;
  struct proxy_endpoint *ep = varg;
  add_proxy_reader_to_writer (pp->sedp_writer_writer, ep);
  return AVLWALK_CONTINUE;
}

static int add_disc_sedp_reader_writer (void *vnode, void *varg)
{
  struct participant *pp = vnode;
  struct proxy_endpoint *ep = varg;
  if (pp->sedp_reader_reader)
    add_proxy_writer_to_reader (pp->sedp_reader_reader, ep);
  return AVLWALK_CONTINUE;
}

static int add_disc_sedp_reader_reader (void *vnode, void *varg)
{
  struct participant *pp = vnode;
  struct proxy_endpoint *ep = varg;
  add_proxy_reader_to_writer (pp->sedp_reader_writer, ep);
  return AVLWALK_CONTINUE;
}

static int add_p2p_ppmsg_writer (void *vnode, void *varg)
{
  struct participant *pp = vnode;
  struct proxy_endpoint *ep = varg;
  if (pp->participant_message_reader)
    add_proxy_writer_to_reader (pp->participant_message_reader, ep);
  return AVLWALK_CONTINUE;
}

static int add_p2p_ppmsg_reader (void *vnode, void *varg)
{
  struct participant *pp = vnode;
  struct proxy_endpoint *ep = varg;
  add_proxy_reader_to_writer (pp->participant_message_writer, ep);
  return AVLWALK_CONTINUE;
}

static int pl_get_guid (const struct CDRHeader *pl, parameterid_t pid, guid_t *guid)
{
  guid_t *gp;
  if ((gp = pl_find (pl, pid)) == NULL)
    return 0;
  *guid = ntoh_guid (*gp);
  return 1;
}

static int pl_get_builtin_endpoint_set (const struct CDRHeader *pl, builtin_endpoint_set_t *set)
{
  builtin_endpoint_set_t *sp;
  if ((sp = pl_find (pl, PID_BUILTIN_ENDPOINT_SET)) == NULL)
    return 0;
  *set = pl_needs_bswap (pl) ? bswap4u (*sp) : *sp;
  return 1;
}

static char *cdr_extract_string (const struct cdrstring *x, int byteswap)
{
  unsigned len = byteswap ? (unsigned) bswap4 (x->length) : (unsigned) x->length;
  char *n = os_malloc (len);
  if (n == NULL)
    return NULL;
  memcpy (n, x->contents, len);
  if (n[len-1] == 0)
    return n;
  else
  {
    os_free (n);
    return NULL;
  }
}

static int pl_get_string (const struct CDRHeader *pl, int pid, char **str)
{
  const struct cdrstring *n;
  if ((n = pl_find (pl, pid)) == NULL)
    return 0;
  if ((*str = cdr_extract_string (n, pl_needs_bswap (pl))) != NULL)
    return 1;
  else
    return 0;
}

static int pl_get_topicname (const struct CDRHeader *pl, char **name)
{
  return pl_get_string (pl, PID_TOPIC_NAME, name);
}

static int pl_get_typename (const struct CDRHeader *pl, char **name)
{
  return pl_get_string (pl, PID_TYPE_NAME, name);
}

static int pl_get_partitions (const struct CDRHeader *pl, int *n, char ***pnames)
{
  const char *seq;
  const char *seqend;
  int bswap = pl_needs_bswap (pl);
  char **names;
  int i, seqsz;
  if ((seq = pl_find (pl, PID_PARTITION)) == NULL)
    return 0;
  if ((seqsz = pl_getlength (seq, bswap)) < (int) sizeof (int))
    return 0;
  seqend = seq + seqsz;
  *n = *((const int *) seq);
  seq += sizeof (int);
  if (bswap)
    *n = bswap4 (*n);
  if (*n < 0)
    return 0;
  if (*n == 0)
  {
    *pnames = NULL;
    return 1;
  }
  if ((names = os_malloc (*n * sizeof (*names))) == NULL)
    return 0;
  for (i = 0; i < *n; i++)
    names[i] = NULL;
  for (i = 0; i < *n; i++)
  {
    int len1;
    assert (seq <= seqend);
    if (seqend - seq < (int) sizeof (int))
      goto fail;
    len1 = *((const int *) seq);
    seq += sizeof (int);
    if (bswap)
      len1 = bswap4 (len1);
    if (len1 < 1 || seqend - seq < len1 || seq[len1-1] != 0)
      goto fail;
    if ((names[i] = os_malloc (len1)) == NULL)
      goto fail;
    memcpy (names[i], seq, len1);
    seq += ALIGN4 (len1);
  }
  *pnames = names;
  return 1;
 fail:
  for (i = 0; i < *n && names[i]; i++)
    os_free (names[i]);
  os_free (names);
  return 0;
}

static int pl_get_expects_inline_qos (const struct CDRHeader *pl, int *expects_inline_qos)
{
  char *x;
  if ((x = pl_find (pl, PID_EXPECTS_INLINE_QOS)) == NULL)
    return 0;
  else
  {
    *expects_inline_qos = (x[0] != 0);
    return 1;
  }
}

static int pl_get_reliability (const struct CDRHeader *pl, int *reliable)
{
  /* ignoring max_blocking time; kind is first anyway */
  reliability_qos_t *p;
  reliability_kind_t raw;
  if ((p = pl_find (pl, PID_RELIABILITY)) == NULL)
    return 0;
  raw = pl_needs_bswap (pl) ? (reliability_kind_t) bswap4 (p->kind) : p->kind;
  *reliable = (raw == RELIABLE_RELIABILITY_QOS);
  return 1;
}

static int pl_get_durability (const struct CDRHeader *pl, durability_kind_t *durability)
{
  /* ignoring max_blocking time; kind is first anyway */
  durability_kind_t *p;
  if ((p = pl_find (pl, PID_DURABILITY)) == NULL)
    return VOLATILE_DURABILITY_QOS;
  *durability = pl_needs_bswap (pl) ? (durability_kind_t) bswap4 (*p) : *p;
  return 1;
}

static void bswap_duration_t (duration_t *d)
{
  d->seconds = bswap4 (d->seconds);
  d->fraction = bswap4u (d->fraction);
}

static int pl_get_duration (const struct CDRHeader *pl, int pid, os_int64 *dur)
{
  const duration_t *p;
  duration_t ddsi_dur_h;
  if ((p = pl_find (pl, pid)) == NULL)
    return 0;
  if (pl_getlength (p, pl_needs_bswap (pl)) < (int) sizeof (*p))
    return 0;
  memcpy (&ddsi_dur_h, p, sizeof (ddsi_dur_h));
  if (pl_needs_bswap (pl))
    bswap_duration_t (&ddsi_dur_h);
  *dur = from_ddsi_time (ddsi_dur_h);
  return 1;
}

static int pl_get_liveliness (const struct CDRHeader *pl, liveliness_qospolicy_t *pol)
{
  const liveliness_qospolicy_t *p;
  if ((p = pl_find (pl, PID_LIVELINESS)) == NULL)
    return 0;
  memcpy (pol, p, sizeof (*pol));
  if (pl_needs_bswap (pl))
  {
    pol->kind = (liveliness_kind_t) bswap4u (p->kind);
    bswap_duration_t (&pol->lease_duration);
  }
  return 1;
}

static int pl_get_ownership (const struct CDRHeader *pl, ownership_qospolicy_t *pol)
{
  const ownership_qospolicy_t *p;
  if ((p = pl_find (pl, PID_OWNERSHIP)) == NULL)
    return 0;
  if (pl_getlength (p, pl_needs_bswap (pl)) < (int) sizeof (*p))
    return 0;
  memcpy (pol, p, sizeof (*pol));
  if (pl_needs_bswap (pl))
    pol->kind = (ownership_kind_t) bswap4u (p->kind);
  return 1;
}

static int pl_get_destination_order (const struct CDRHeader *pl, destination_order_qospolicy_t *pol)
{
  const destination_order_qospolicy_t *p;
  if ((p = pl_find (pl, PID_DESTINATION_ORDER)) == NULL)
    return 0;
  if (pl_getlength (p, pl_needs_bswap (pl)) < (int) sizeof (*p))
    return 0;
  memcpy (pol, p, sizeof (*pol));
  if (pl_needs_bswap (pl))
    pol->kind = (destination_order_kind_t) bswap4u (p->kind);
  return 1;
}


static int pl_get_presentation_qospolicy (const struct CDRHeader *pl, presentation_qospolicy_t *pol)
{
  const presentation_qospolicy_t *p;
  if ((p = pl_find (pl, PID_PRESENTATION)) == NULL)
    return 0;
  if (pl_getlength (p, pl_needs_bswap (pl)) < (int) sizeof (*p))
    goto fail;
  memcpy (pol, p, sizeof (*pol));
  if (pl_needs_bswap (pl))
    pol->access_scope = (presentation_access_scope_kind_t) bswap4u (p->access_scope);
  switch (pol->access_scope)
  {
    case INSTANCE_PRESENTATION_QOS:
    case TOPIC_PRESENTATION_QOS:
    case GROUP_PRESENTATION_QOS:
      break;
    default:
      goto fail;
  }
  if (pol->coherent_access & ~1u)
    goto fail;
  if (pol->ordered_access & ~1u)
    goto fail;
  return 1;
 fail:
  trace (DBGFLAG_TRACING, "pl_get_presentation_qospolicy: present but malformed\n");
  return 0;
}

static int pl_getnaked_statusinfo (const parameter_t *pl, int needs_bswap, unsigned *statusinfo)
{
  unsigned *p;
  if ((p = pl_findnaked (pl, needs_bswap, PID_STATUS_INFO)) == NULL)
    return 0;
  if (pl_getlength (p, needs_bswap) != 4)
    return 0;
  /* DDSI spec is weird in its definition of status info bits: an
     array of 4 octets */
  *statusinfo = PLATFORM_IS_LITTLE_ENDIAN ? bswap4u (*p) : *p;
  return 1;
}

static int pl_getnaked_writer_info (const parameter_t *pl, int needs_bswap, struct rtps_prismtech_writer_info *wri)
{
  /* Note: message source *MUST* be PrismTech */
  unsigned *p;
  if ((p = pl_findnaked (pl, needs_bswap, PID_PRISMTECH_WRITER_INFO)) == NULL)
    return 0;
  if (pl_getlength (p, needs_bswap) != sizeof (*wri))
    return 0;
  memcpy (wri, p, sizeof (*wri));
  if (needs_bswap)
  {
    wri->transactionId = bswap4 (wri->transactionId);
    wri->writerGID.systemId = bswap4 (wri->writerGID.systemId);
    wri->writerGID.localId = bswap4 (wri->writerGID.localId);
    wri->writerGID.serial = bswap4 (wri->writerGID.serial);
    wri->writerInstanceGID.systemId = bswap4 (wri->writerInstanceGID.systemId);
    wri->writerInstanceGID.systemId = bswap4 (wri->writerInstanceGID.localId);
    wri->writerInstanceGID.systemId = bswap4 (wri->writerInstanceGID.serial);
  }
  return 1;
}

static locator_t bswap_locator (locator_t loc)
{
  loc.kind = bswap4 (loc.kind);
  loc.port = bswap4 (loc.port);
  return loc;
}

static locator_udpv4_t bswap_locator_udpv4 (locator_udpv4_t loc)
{
  loc.port = bswap4 (loc.port);
  return loc;
}

static int loc_to_loc_udpv4 (locator_udpv4_t *dst, locator_t src)
{
  if (src.kind != LOCATOR_KIND_UDPv4)
    return 0;
  else
  {
    memcpy (&dst->address, src.address + 12, 4);
    dst->port = src.port;
    return 1;
  }
}

static int pl_get_locator_udpv4 (const struct CDRHeader *pl, int pid, locator_udpv4_t *dst)
{
  int alt = -1, bswap = pl_needs_bswap (pl);
  locator_t *locp, loc;
  locator_udpv4_t *loc4p;
  switch (pid)
  {
    case PID_UNICAST_LOCATOR: break;
    case PID_MULTICAST_LOCATOR: alt = PID_MULTICAST_IPADDRESS; break;
    case PID_DEFAULT_UNICAST_LOCATOR: alt = PID_DEFAULT_UNICAST_IPADDRESS; break;
    case PID_DEFAULT_MULTICAST_LOCATOR: break;
    case PID_METATRAFFIC_UNICAST_LOCATOR: alt = PID_METATRAFFIC_UNICAST_IPADDRESS; break;
    case PID_METATRAFFIC_MULTICAST_LOCATOR: alt = PID_METATRAFFIC_MULTICAST_IPADDRESS; break;
    default:
      abort ();
  }
  locp = pl_find (pl, pid);
  while (locp)
  {
    loc = bswap ? bswap_locator (*locp) : *locp;
    if (loc_to_loc_udpv4 (dst, loc))
      return 1;
    locp = pl_findnext (pl, locp, pid);
  }
  if (alt != -1 && (loc4p = pl_find (pl, alt)) != NULL)
  {
    *dst = bswap ? bswap_locator_udpv4 (*loc4p) : *loc4p;
    return 1;
  }
  return 0;
}

static int assert_ep_liveliness_based_on_pp (void *vnode, void *varg)
{
  /* EP liveliness shouldn't be asserted when SPDP gets notified of
     the pp's existence, I think; calling avl_augment_update all the
     time is also not really smart ... */
  struct proxy_endpoint *ep = vnode;
  const os_int64 *tnow = varg;
  ep->tlease_end = add_duration_to_time (*tnow, ep->tlease_dur);
  avl_augment_update (&proxyeptree, ep);
  return AVLWALK_CONTINUE;
}

static void assert_pp_and_all_ep_liveliness (struct proxy_participant *pp)
{
  guid_t min, max;
  os_int64 tnow = now ();
  pp->tlease_end = add_duration_to_time (tnow, pp->tlease_dur);
  avl_augment_update (&proxypptree, pp);
  min = pp->guid; min.entityid.u = 0;
  max = pp->guid; max.entityid.u = ~0;
  avl_walkrange (&proxyeptree, &min, &max, assert_ep_liveliness_based_on_pp, &tnow);
}

static int force_spdp_xmit (void *vnode, void *varg UNUSED)
{
  struct participant *pp = vnode;
  /*struct proxy_participant *proxypp = varg;*/
  /* Ought to send it only to proxypp, but this is simpler */
  resched_xevent_if_earlier (pp->spdp_xevent, 0);
  return AVLWALK_CONTINUE;
}

static int handle_SPDP_dead (struct receiver_state *rst UNUSED, const Data_t *msg, int size, parameter_t *qosp, int qos_needs_bswap, struct CDRHeader *data, unsigned statusinfo)
{
  struct proxy_participant *pp;
  guid_t guid;

  trace (DBGFLAG_TRACING, "SPDP ");

  /* Payload & statusinfo != 0 => dispose or unregister with
     serialized key present; the key here is a GUID or a GUID prefix
     (not sure which, but the prefix is always at the lower offset),
     which does not depend on the LE/BE setting */
  if (data)
  {
    guid_prefix_t *gg = (guid_prefix_t *) (data + 1);
    if ((char *) msg + size < (char *) (gg + 1))
    {
      trace (DBGFLAG_TRACING, "ST%x/SHORT", statusinfo);
      return 0;
    }
    else
    {
      guid.prefix = ntoh_guid_prefix (*gg);
    }
  }
  else /* That would be RTI's mode of operation ... (which we now follow) */
  {
    char *p;
    trace (DBGFLAG_TRACING, "no payload? ");
    if ((p = pl_findnaked (qosp, qos_needs_bswap, PID_KEY_HASH)) != NULL)
    {
      trace (DBGFLAG_TRACING, "keyhash ");
      if (pl_getlength (p, qos_needs_bswap) != 16)
      {
	trace (DBGFLAG_TRACING, "of wrong length");
	return 0;
      }
      memcpy (&guid, p, 16);
      guid.prefix = ntoh_guid_prefix (guid.prefix);
    }
    else
    {
      trace (DBGFLAG_TRACING, "no keyhash?");
      return 0;
    }
  }

  guid.entityid.u = ENTITYID_PARTICIPANT;
  trace (DBGFLAG_TRACING, "ST%x/%x:%x:%x:%x", statusinfo, PGUID (guid));
  if ((pp = avl_lookup (&proxypptree, &guid, NULL)) == NULL)
    trace (DBGFLAG_TRACING, " unknown");
  else
  {
    trace (DBGFLAG_TRACING, " sched_delete");
    pp->tlease_end = 0;
    avl_augment_update (&proxypptree, pp);
  }
  return 0;
}

static int handle_SPDP (struct receiver_state *rst, const Data_t *msg, int size, int byteswap UNUSED, parameter_t *qosp, struct CDRHeader *data)
{
  guid_t guid;
  builtin_endpoint_set_t bes;
  os_int64 tlease_dur;
  struct addrset *as_meta, *as_default;
  struct proxy_participant *pp;
  struct proxy_endpoint *ep;
  locator_udpv4_t loc;
  const char *stat = "";
  int ignore = 0;
  unsigned statusinfo;
  int qos_needs_bswap;
  trace (DBGFLAG_TRACING, "SPDP ");

  if (msg->smhdr.flags & SMFLAG_ENDIANNESS)
    qos_needs_bswap = !PLATFORM_IS_LITTLE_ENDIAN;
  else
    qos_needs_bswap = PLATFORM_IS_LITTLE_ENDIAN;
  if (!pl_getnaked_statusinfo (qosp, qos_needs_bswap, &statusinfo))
    statusinfo = 0;
  if (statusinfo != 0)
  {
    return handle_SPDP_dead (rst, msg, size, qosp, qos_needs_bswap, data, statusinfo);
  }

  if (data == NULL)
  {
    trace (DBGFLAG_TRACING, "no payload?");
    return 0;
  }
  if (!(pl_get_guid (data, PID_PARTICIPANT_GUID, &guid) &&
	pl_get_builtin_endpoint_set (data, &bes)))
  {
    trace (DBGFLAG_TRACING, "parameters missing?");
    return 0;
  }
  /* At some point the RTI implementation didn't mention
     BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_READER & ...WRITER, or
     so it seemed; and yet they are necessary for correct operation,
     so add them. */
  if (vendor_is_rti (rst->vendor))
  {
    bes |=
      BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_READER |
      BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_WRITER;
  }
  /* Twinoaks generates (generated?) SPDP with invalid entity id (it
     must be ENTITYID_PARTICIPANT, otherwise what's the point of
     defining that value) */
  if (guid.entityid.u == 0)
    guid.entityid.u = ENTITYID_PARTICIPANT;
  if (avl_lookup (&pptree, &guid, NULL))
  {
    stat = " (local)";
    ignore = 1;
  }
  if ((pp = avl_lookup (&proxypptree, &guid, NULL)) != NULL)
  {
    stat = " (known - FIXME updating lease)";
    assert_pp_and_all_ep_liveliness (pp);
    ignore = 1;
  }
  trace (DBGFLAG_TRACING, "%x:%x:%x:%x bes %x%s", PGUID (guid), bes, stat);
  if (ignore)
    return 1;
  trace (DBGFLAG_TRACING, " NEW");
  if (!pl_get_duration (data, PID_PARTICIPANT_LEASE_DURATION, &tlease_dur))
  {
    trace (DBGFLAG_TRACING, " (PARTICIPANT_LEASE_DURATION defaulting to 11s)");
    tlease_dur = 11 * T_SECOND;
  }
  as_default = new_addrset ();
  as_meta = new_addrset ();
  if (pl_get_locator_udpv4 (data, PID_DEFAULT_UNICAST_LOCATOR, &loc))
    add_to_addrset (as_default, &loc);
  if (pl_get_locator_udpv4 (data, PID_METATRAFFIC_UNICAST_LOCATOR, &loc))
    add_to_addrset (as_meta, &loc);
  if (use_mcast_flag)
  {
    if (pl_get_locator_udpv4 (data, PID_DEFAULT_MULTICAST_LOCATOR, &loc))
      add_to_addrset (as_default, &loc);
    if (pl_get_locator_udpv4 (data, PID_METATRAFFIC_MULTICAST_LOCATOR, &loc))
      add_to_addrset (as_meta, &loc);
  }
  trace_addrset (DBGFLAG_TRACING, " (def", as_default);
  trace_addrset (DBGFLAG_TRACING, " meta", as_meta);
  trace (DBGFLAG_TRACING, ")");
  pp = new_proxy_participant (guid, bes, as_default, as_meta, tlease_dur, rst->vendor);
  if (pp->bes & DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER)
  {
    guid_t guid1 = pp->guid;
    guid1.entityid.u = ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER;
    ep = new_proxy_endpoint (guid1, pp, MF_RELIABLE|MF_DURABILITY_TRANSIENT_LOCAL, pp->as_meta, pp->tlease_dur, NULL, NULL, NULL, AUTOMATIC_LIVELINESS_QOS, SHARED_OWNERSHIP_QOS, BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS, default_presentation_qospolicy);
    avl_walk (&pptree, add_disc_sedp_writer_writer, ep);
  }
  if (pp->bes & DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR)
  {
    guid_t guid1 = pp->guid;
    guid1.entityid.u = ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER;
    ep = new_proxy_endpoint (guid1, pp, MF_RELIABLE|MF_DURABILITY_TRANSIENT_LOCAL, pp->as_meta, pp->tlease_dur, NULL, NULL, NULL, AUTOMATIC_LIVELINESS_QOS, SHARED_OWNERSHIP_QOS, BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS, default_presentation_qospolicy);
    avl_walk (&pptree, add_disc_sedp_writer_reader, ep);
  }
  if (pp->bes & DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER)
  {
    guid_t guid1 = pp->guid;
    guid1.entityid.u = ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER;
    ep = new_proxy_endpoint (guid1, pp, MF_RELIABLE|MF_DURABILITY_TRANSIENT_LOCAL, pp->as_meta, pp->tlease_dur, NULL, NULL, NULL, AUTOMATIC_LIVELINESS_QOS, SHARED_OWNERSHIP_QOS, BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS, default_presentation_qospolicy);
    avl_walk (&pptree, add_disc_sedp_reader_writer, ep);
  }
  if (pp->bes & DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR)
  {
    guid_t guid1 = pp->guid;
    guid1.entityid.u = ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_READER;
    ep = new_proxy_endpoint (guid1, pp, MF_RELIABLE|MF_DURABILITY_TRANSIENT_LOCAL, pp->as_meta, pp->tlease_dur, NULL, NULL, NULL, AUTOMATIC_LIVELINESS_QOS, SHARED_OWNERSHIP_QOS, BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS, default_presentation_qospolicy);
    avl_walk (&pptree, add_disc_sedp_reader_reader, ep);
  }
  if (pp->bes & BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_WRITER)
  {
    guid_t guid1 = pp->guid;
    guid1.entityid.u = ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_WRITER;
    ep = new_proxy_endpoint (guid1, pp, MF_RELIABLE|MF_DURABILITY_TRANSIENT_LOCAL, pp->as_meta, pp->tlease_dur, NULL, NULL, NULL, AUTOMATIC_LIVELINESS_QOS, SHARED_OWNERSHIP_QOS, BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS, default_presentation_qospolicy);
    avl_walk (&pptree, add_p2p_ppmsg_writer, ep);
  }
  if (pp->bes & BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_READER)
  {
    guid_t guid1 = pp->guid;
    guid1.entityid.u = ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_READER;
    ep = new_proxy_endpoint (guid1, pp, MF_RELIABLE|MF_DURABILITY_TRANSIENT_LOCAL, pp->as_meta, pp->tlease_dur, NULL, NULL, NULL, AUTOMATIC_LIVELINESS_QOS, SHARED_OWNERSHIP_QOS, BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS, default_presentation_qospolicy);
    avl_walk (&pptree, add_p2p_ppmsg_reader, ep);
  }
  unref_addrset (as_meta);
  unref_addrset (as_default);

  /* Force transmission of SPDP messages */
  avl_walk (&pptree, force_spdp_xmit, pp);
  return 1;
}

static void handle_PMD (const struct datainfo *di, const void *vdata, int len)
{
  const struct CDRHeader *data = vdata; /* built-ins not deserialized (yet) */
  const int bswap = (data->identifier == CDR_LE) ^ PLATFORM_IS_LITTLE_ENDIAN;
  struct proxy_participant *pp;
  guid_t ppguid;
  trace (DBGFLAG_TRACING, " PMD ST%x", di->statusinfo);
  if (data->identifier != CDR_LE && data->identifier != CDR_BE)
  {
    trace (DBGFLAG_TRACING, " PMD data->identifier %d !?", ntohs (data->identifier));
    return;
  }
  switch (di->statusinfo & (STATUSINFO_DISPOSE | STATUSINFO_UNREGISTER))
  {
    case 0:
      if (offsetof (ParticipantMessageData_t, value) > len - sizeof (struct CDRHeader))
	debug_print_rawdata (" SHORT1", data, len);
      else
      {
	const ParticipantMessageData_t *pmd = (ParticipantMessageData_t *) (data + 1);
	guid_prefix_t p = ntoh_guid_prefix (pmd->participantGuidPrefix);
	unsigned kind = ntohl (pmd->kind);
	unsigned length = bswap ? bswap4u (pmd->length) : pmd->length;
	trace (DBGFLAG_TRACING, " pp %x:%x:%x kind %u data %d", p.u[0], p.u[1], p.u[2], kind, length);
	if (len - sizeof (struct CDRHeader) - offsetof (ParticipantMessageData_t, value) < length)
	  debug_print_rawdata (" SHORT2", pmd->value, len - sizeof (struct CDRHeader) - offsetof (ParticipantMessageData_t, value));
	else
	  debug_print_rawdata ("", pmd->value, length);
	ppguid.prefix = p;
	ppguid.entityid.u = ENTITYID_PARTICIPANT;
	if ((pp = avl_lookup (&proxypptree, &ppguid, NULL)) == NULL)
	  trace (DBGFLAG_TRACING, " PPunknown");
	else
	  assert_pp_and_all_ep_liveliness (pp);
      }
      break;
    case STATUSINFO_DISPOSE:
    case STATUSINFO_UNREGISTER:
    case STATUSINFO_DISPOSE | STATUSINFO_UNREGISTER:
      /* Serialized key; BE of LE doesn't matter as both fields are
	 defined as octets.  */
      if (len < (int) (sizeof (struct CDRHeader) + sizeof (guid_prefix_t) + sizeof (liveliness_kind_t)))
	debug_print_rawdata (" SHORT3", data, len);
      else
      {
	ppguid.prefix = ntoh_guid_prefix (*((guid_prefix_t *) (data + 1)));
	ppguid.entityid.u = ENTITYID_PARTICIPANT;
	if ((pp = avl_lookup (&proxypptree, &ppguid, NULL)) != NULL)
	{
	  trace (DBGFLAG_TRACING, " sched_delete");
	  pp->tlease_end = 0;
	  avl_augment_update (&proxypptree, pp);
	}
      }
      break;
    default:
      assert (0);
      break;
  }
}

static int handle_SEDP_alive (const struct datainfo *di UNUSED_NDEBUG, const struct CDRHeader *data)
{
#define E(msg, lbl) do { trace (DBGFLAG_TRACING, (msg)); goto lbl; } while (0)
  guid_t guid;
  struct proxy_participant *pp;
  struct proxy_endpoint *ep;
  guid_t ppguid;
  char *topic = NULL;
  char *typename = NULL;
  char *partition = NULL;
  int reliable;
  durability_kind_t durability;
  avlparent_t parent;
  int inl_qos;
  locator_udpv4_t loc;
  liveliness_qospolicy_t liveliness;
  ownership_qospolicy_t ownership;
  destination_order_qospolicy_t destination_order;
  presentation_qospolicy_t presentation_qospolicy;
  os_int64 tlease_dur;
  unsigned mode;
  struct addrset *as;
  int result = 0;

  assert (data);
  assert ((di->statusinfo & (STATUSINFO_DISPOSE | STATUSINFO_UNREGISTER)) == 0);

  if (!pl_get_guid (data, PID_ENDPOINT_GUID, &guid))
    E (" no guid?", done);

  trace (DBGFLAG_TRACING, " %x:%x:%x:%x", PGUID (guid));
  ppguid = guid;
  ppguid.entityid.u = ENTITYID_PARTICIPANT;
  if (avl_lookup (&pptree, &ppguid, NULL))
    E (" local pp?", done);
  if ((pp = avl_lookup (&proxypptree, &ppguid, NULL)) == NULL)
    E (" unknown proxy pp?", done);
  if (is_builtin_entityid (guid.entityid))
    E (" built-in", done);
  if (!pl_get_topicname (data, &topic))
    E (" no topic?", done);
  if (!pl_get_typename (data, &typename))
    E (" no typename?", done_topic);
  if (!pl_get_reliability (data, &reliable))
    reliable = 0;
  if (!pl_get_durability (data, &durability))
    durability = VOLATILE_DURABILITY_QOS;
  if (pl_find (data, PID_PARTITION) == NULL)
    partition = os_strdup ("");
  else
  {
    int nps;
    char **ps;
    if (!pl_get_partitions (data, &nps, &ps))
      E (" failed to extract partitions", done_typename);
    if (nps == 0)
      partition = os_strdup ("");
    else if (nps == 1)
    {
      partition = ps[0];
      os_free (ps);
    }
    else
    {
      int i;
      for (i = 0; i < nps; i++)
	os_free (ps[i]);
      os_free (ps);
      E (" FIXME:multiple partitions not yet supported", done_typename);
    }
  }
  trace (DBGFLAG_TRACING, " %s %s %s: %s.%s/%s",
	 reliable ? "reliable" : "best-effort",
	 durability_to_string (durability),
	 is_writer_entityid (guid.entityid) ? "writer" : "reader",
	 *partition ? partition : "(default)", topic, typename);
  inl_qos = 0;
  if (!is_writer_entityid (guid.entityid))
    pl_get_expects_inline_qos (data, &inl_qos);
  if (inl_qos)
    E ("******* AARGH - it expects inline QoS ********\n", done_partition);
  if (avl_lookup (&proxyeptree, &guid, &parent))
  {
    trace (DBGFLAG_TRACING, " known");
    result = 1;
    goto done_partition;
  }

  trace (DBGFLAG_TRACING, " NEW");
  if (!pl_get_liveliness (data, &liveliness))
  {
    trace (DBGFLAG_TRACING, "(LIVELINESS defaulting to {AUTOMATIC,111s})");
    liveliness.kind = AUTOMATIC_LIVELINESS_QOS;
    liveliness.lease_duration.seconds = 111;
    liveliness.lease_duration.fraction = 0;
  }
  if (liveliness.kind != AUTOMATIC_LIVELINESS_QOS)
    trace (DBGFLAG_TRACING, "(FIXME: liveliness not AUTOMATIC)");
  if (!pl_get_ownership (data, &ownership))
    ownership.kind = SHARED_OWNERSHIP_QOS;
  if (!pl_get_destination_order (data, &destination_order))
    destination_order.kind = BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS;
  if (!pl_get_presentation_qospolicy (data, &presentation_qospolicy))
    presentation_qospolicy = default_presentation_qospolicy;
  tlease_dur = from_ddsi_time (liveliness.lease_duration);
  as = new_addrset ();
  if (pl_get_locator_udpv4 (data, PID_UNICAST_LOCATOR, &loc))
    add_to_addrset (as, &loc);
  else
    copy_addrset_into_addrset_uc (as, pp->as_default);
  if (use_mcast_flag)
  {
    if (pl_get_locator_udpv4 (data, PID_MULTICAST_LOCATOR, &loc))
      add_to_addrset (as, &loc);
    else
      copy_addrset_into_addrset_mc (as, pp->as_default);
  }
  mode = (reliable ? MF_RELIABLE : 0) | durability_to_mask (durability);
  trace_addrset (DBGFLAG_TRACING, " (as", as);
  trace (DBGFLAG_TRACING, ")");
  ep = new_proxy_endpoint (guid, pp, mode, as, tlease_dur, topic, typename, partition, liveliness.kind, ownership.kind, destination_order.kind, presentation_qospolicy);
  unref_addrset (as);
  /* cheating a bit: discover it once, match with all locals (and
     automatically bind it to any future new matching locals) */
  dds_attach_proxy_endpoint (ep);
  result = 1;

 done_partition:
  os_free (partition);
 done_typename:
  os_free (typename);
 done_topic:
  os_free (topic);
 done:
  return result;
#undef E
}

static void handle_SEDP_dead (const struct datainfo *di UNUSED, const struct CDRHeader *data, int len)
{
  struct proxy_endpoint *ep;
  guid_t guid;

  /* We ought to have received a PL_CDR_{BE,LE} encoded parameter
     list, but (RTI's implementation being as "efficient"
     [non-conforming, that is] as it is) we may have to make do
     with a keyhash, which will be presented to us, here, as PL_BE
     encoded data. */
  switch (data->identifier)
  {
    case PL_CDR_LE:
    case PL_CDR_BE:
      if (!pl_get_guid (data, PID_ENDPOINT_GUID, &guid))
      {
	trace (DBGFLAG_TRACING, " no guid?");
	return;
      }
      break;

    case CDR_BE:
      /* Assume a key hash, len = 16 + size of header */
      trace (DBGFLAG_TRACING, " no data?");
      if (len != sizeof (struct CDRHeader) + 16)
      {
	trace (DBGFLAG_TRACING, " not keyhash?");
	return;
      }
      trace (DBGFLAG_TRACING, " keyhash ");
      memcpy (&guid, data + 1, sizeof (guid));
      guid = ntoh_guid (guid);
      break;

    default:
      debug_print_rawdata (" GARBAGE", data, len);
      break;
  }

  trace (DBGFLAG_TRACING, "%x:%x:%x:%x", PGUID (guid));
  if ((ep = avl_lookup (&proxyeptree, &guid, NULL)) == NULL)
    trace (DBGFLAG_TRACING, " unknown");
  else
  {
    trace (DBGFLAG_TRACING, " sched_delete");
    ep->tlease_end = 0;
    avl_augment_update (&proxyeptree, ep);
  }
}

static void handle_SEDP (const struct datainfo *di, const void *vdata, int len)
{
  const struct CDRHeader *data = vdata; /* built-ins not deserialized (yet) */
  trace (DBGFLAG_TRACING, " SEDP ST%x", di->statusinfo);
  if (data == NULL)
  {
    trace (DBGFLAG_TRACING, " no payload?");
    return;
  }
  switch (di->statusinfo & (STATUSINFO_DISPOSE | STATUSINFO_UNREGISTER))
  {
    case 0:
      handle_SEDP_alive (di, data);
      break;

    case STATUSINFO_DISPOSE:
    case STATUSINFO_UNREGISTER:
    case (STATUSINFO_DISPOSE | STATUSINFO_UNREGISTER):
      handle_SEDP_dead (di, data, len);
      break;
  }
}

struct update_seqnr_admin_Data_helper_arg {
  struct receiver_state *rst;
  const Data_t *msg;
  guid_t src, dst;
  os_int64 seq;
};

static int update_seqnr_admin_Data_helper (void *vnode, void *varg)
{
  struct rhc_writers_node *wn = vnode;
  struct update_seqnr_admin_Data_helper_arg *arg = varg;
  if (arg->seq == wn->seq + 1)
    wn->seq = arg->seq;
#if 0
  else if (arg->seq > wn->seq + 1)
    send_AckNack (wn);
#endif
  return AVLWALK_CONTINUE;
}

static void update_seqnr_admin_Data (struct receiver_state *rst, const Data_t *msg)
{
  struct update_seqnr_admin_Data_helper_arg arg;
  struct proxy_endpoint *ep;
  arg.rst = rst;
  arg.msg = msg;
  arg.src.prefix = rst->source_guid_prefix;
  arg.src.entityid = msg->writerId;
  arg.dst.prefix = rst->dest_guid_prefix;
  arg.dst.entityid = msg->readerId;
  arg.seq = fromSN (msg->writerSN);
  /*trace (DBGFLAG_TRACING, "USAD %x:%x:%x:%x -> %x:%x:%x:%x\n", arg.src.prefix.u[0], arg.src.prefix.u[1], arg.src.prefix.u[2], arg.src.entityid.u, arg.dst.prefix.u[0], arg.dst.prefix.u[1], arg.dst.prefix.u[2], arg.dst.entityid.u);*/
  if ((ep = avl_lookup (&proxyeptree, &arg.src, NULL)) == NULL)
  {
    /*trace (DBGFLAG_TRACING, "USAD ignored: src unknown\n");*/
  }
  else
  {
    handle_forall_destinations (&arg.src, &arg.dst, ep, update_seqnr_admin_Data_helper, &arg, 0);
    if (arg.seq > ep->u.wr.last_seq)
    {
      ep->u.wr.last_seq = arg.seq;
    }
  }
}

static int handle_regular_helper (void *vnode, void *varg)
{
  struct rhc_writers_node *wn = vnode;
  struct handle_regular_helper_arg *arg = varg;
  int deliver = 0;
  if (!wn->reader->reliable)
  {
    if (arg->seq > wn->seq)
    {
      wn->seq = arg->seq;
      deliver = 1;
    }
  }
  else
  {
    if (arg->seq == wn->seq + 1)
    {
      wn->seq = arg->seq;
      deliver = 1;
    }
    else if (arg->seq > wn->seq)
    {
      arg->out_of_sequence++;
      if (!is_builtin_entityid (arg->src.entityid))
	osa_remember (&wn->osa, arg);
      else if (arg->out_of_sequence == 1)
	osa_remember (&wn->osa, arg);
    }
  }
  if (!is_builtin_entityid (arg->src.entityid))
  {
    if (deliver)
    {
      arg->deliver++;
      deliver_one_message (wn, arg);
    }
    else
    {
      trace (DBGFLAG_TRACING, " !%x:%x:%x:%x", PGUID (wn->reader->guid));
    }
  }
  else
  {
    /* Special-case builtins: participant messages and SEDP are the
       only ones handled through this path, and both need to processed
       only once: it isn't a terrible problem if they get processed
       multiple times, but since the actual processing of the data is
       global, there is no need for that, even though the reliable
       protocol state updates need to take place. And it saves a lot
       of output */
    assert (wn->reader->data_recv_cb.generic);
    if (deliver)
    {
      wn->reader->ndelivered++;
      if (++arg->deliver == 1)
	deliver_one_message (wn, arg);
      else
	trace (DBGFLAG_TRACING, " (%x:%x:%x:%x)", PGUID (wn->reader->guid));
    }
    else
    {
      trace (DBGFLAG_TRACING, " !%x:%x:%x:%x", PGUID (wn->reader->guid));
    }
  }
  /* Handoff as many stored out-of-seq messages as we can */
  osa_handoff (wn);
  return AVLWALK_CONTINUE;
}

static void handle_regular (struct receiver_state *rst, const Data_t *msg, int size, int byteswap UNUSED, parameter_t *qosp, struct CDRHeader *datap)
{
  struct handle_regular_helper_arg arg;
  struct proxy_endpoint *ep;
  struct { struct CDRHeader cdr; char kh[16]; } dispose_payload;
  int qos_needs_bswap;
  char *keyhash;
  topic_t topic;

  arg.src.prefix = rst->source_guid_prefix;
  arg.src.entityid = msg->writerId;
  arg.dst.prefix = rst->dest_guid_prefix;
  arg.dst.entityid = msg->readerId;
  memset (&arg.wri, 0, sizeof (&arg.wri));
  arg.wri.writerGID.systemId = arg.src.prefix.u[0];
  arg.wri.writerGID.localId = arg.src.prefix.u[1];
  arg.wri.writerGID.serial = arg.src.prefix.u[3];
  arg.tstamp = rst->have_timestamp ? from_ddsi_time (rst->timestamp) : 0;
  arg.seq = fromSN (msg->writerSN);
  arg.deliver = 0;
  arg.out_of_sequence = 0;
  arg.payload_kind = PK_NONE;
  if (msg->smhdr.flags & SMFLAG_ENDIANNESS)
    qos_needs_bswap = !PLATFORM_IS_LITTLE_ENDIAN;
  else
    qos_needs_bswap = PLATFORM_IS_LITTLE_ENDIAN;

  /* We only process it if we know the source; check this early for
     two reasons: for performance (not that we care about that at this
     stage, really) and for looking up the topic definition from a
     local reader associated with this endpoint. */
  if (!is_writer_entityid (msg->writerId))
  {
    trace (DBGFLAG_TRACING, "%x:%x:%x:%x -> %x:%x:%x:%x src not writer", PGUID (arg.src), PGUID (arg.dst));
    return;
  }
  if ((ep = avl_lookup (&proxyeptree, &arg.src, NULL)) == NULL)
  {
    trace (DBGFLAG_TRACING, "%x:%x:%x:%x? -> %x:%x:%x:%x", PGUID (arg.src), PGUID (arg.dst));
    return;
  }
  if (avl_empty (&ep->matched_locals.readers))
  {
    trace (DBGFLAG_TRACING, "%x:%x:%x:%x -> %x:%x:%x:%x: no readers", PGUID (arg.src), PGUID (arg.dst));
    return;
  }

#if ARRIVAL_OF_DATA_ASSERTS_PP_AND_EP_LIVELINESS
  {
    struct proxy_participant *pp = ep->pp;
    os_int64 tnow = now ();
    pp->tlease_end = add_duration_to_time (tnow, pp->tlease_dur);
    avl_augment_update (&proxypptree, pp);
    assert_ep_liveliness_based_on_pp (ep, &tnow);
  }
#endif

  /* Find status info and keyhash in inline qos set, needed for
     determining whether absence of payload is a problem or not (and
     if not, having an alternative). These involve scanning the inline
     qos, so they are potentially somewhat expensive. */
  if (!pl_getnaked_statusinfo (qosp, qos_needs_bswap, &arg.statusinfo))
    arg.statusinfo = 0;
  keyhash = pl_findnaked (qosp, qos_needs_bswap, PID_KEY_HASH);
  if (keyhash && pl_getlength (keyhash, qos_needs_bswap) != 16)
  {
    trace (DBGFLAG_TRACING, "supplied keyhash has wrong length");
    return;
  }
  trace (DBGFLAG_TRACING, "#%lld ST%x ", arg.seq, arg.statusinfo);

  /* If an OpenSplice writer gid is embedded, use it -- but only look
     for one if the message was sent by OpenSplice. We have a
     reasonablish default set already. */
  if (vendor_is_prismtech (rst->vendor))
    pl_getnaked_writer_info (qosp, qos_needs_bswap, &arg.wri);

  /* One would expect DATA for a regular write, and KEY for a dispose
     or unregister, based on the spec, but at least RTI relies on the
     keyhash for dispose & unregister, at least for the topics/types
     seen by me; we'll take the payload if available, and accept a
     keyhash for dipose/unregister if there is no payload. */
  if (arg.statusinfo == 0 && (datap == NULL || !(msg->smhdr.flags & DATA_FLAG_DATAFLAG)))
  {
    trace (DBGFLAG_TRACING, "regular but no data");
    return;
  }
  else if (arg.statusinfo != 0 && (datap == NULL || !(msg->smhdr.flags & DATA_FLAG_KEYFLAG)) && keyhash == NULL)
  {
    trace (DBGFLAG_TRACING, "dispose/unregister but no keys/keyhash");
    return;
  }

  /* Src is ok, it has matched readrs, and the payloads situation is
     acceptable, so now find out the topic for deserialization. Note
     that this is the topic definition of an arbitrarily selected
     matched local reader; other local readers SHOULD have the same,
     but it isn't checked. */
  topic = ep->matched_locals.readers.root->reader->topic;
  if (topic == NULL)
  {
    /* Built-ins don't (yet) have topic definitions associated, but
       they do their own deserialization. */
    assert (is_builtin_entityid (arg.src.entityid));
    arg.payload_kind = PK_RAW_ALIASED;
    if (datap)
    {
      arg.payload.raw.len = (int) (((char *) msg + size) - (char *) datap);
      arg.payload.raw.ptr = datap;
    }
    else
    {
      assert (keyhash);
      dispose_payload.cdr.identifier = CDR_BE;
      dispose_payload.cdr.options = 0;
      memcpy (dispose_payload.kh, keyhash, 16);
      arg.payload.raw.len = sizeof (struct CDRHeader) + 16;
      arg.payload.raw.ptr = &dispose_payload.cdr;
    }
  }
  else if (arg.statusinfo == 0)
  {
    /* datap != NULL && contains real data (assuming a cooperative
       remote writer), so deserialize */
    const int datalen = (int) (((char *) msg + size) - (char *) datap);
    arg.payload_kind = PK_V_MESSAGE;
    if ((arg.payload.v_message = deserialize (topic, datap, datalen)) == NULL)
    {
      char tmp[1024];
      int tmpsize = sizeof (tmp), res;
      res = prettyprint_raw (tmp, tmpsize, topic, datap, datalen);
      assert (res < 0);
      trace (DBGFLAG_TRACING, "deserialization %s/%s failed: %s%s", topic_name (topic), topic_typename (topic), tmp, res < tmpsize ? "" : " (trunc)");
      return;
    }
  }
  else if (datap)
  {
    /* key payload */
    const int datalen = (int) (((char *) msg + size) - (char *) datap);
    arg.payload_kind = PK_V_MESSAGE;
    if ((arg.payload.v_message = deserialize_from_key (topic, datap, datalen)) == NULL)
    {
      trace (DBGFLAG_TRACING, "deserialization %s/%s key failed", topic_name (topic), topic_typename (topic));
      return;
    }
  }
  else
  {
    assert (keyhash);
    arg.payload_kind = PK_V_MESSAGE;
    if ((arg.payload.v_message = deserialize_from_keyhash (topic, keyhash, 16)) == NULL)
    {
      trace (DBGFLAG_TRACING, "deserialization %s/%s keyhash failed", topic_name (topic), topic_typename (topic));
      return;
    }
  }

  if ((debugflags & DBGFLAG_TRACING) && arg.payload_kind == PK_V_MESSAGE)
  {
    char tmp[4096];
    int tmpsize = sizeof (tmp), res;
    if (arg.statusinfo == 0)
    {
#if 1
      const int datalen = (int) (((char *) msg + size) - (char *) datap);
      res = prettyprint_raw (tmp, tmpsize, topic, datap, datalen);
#else
      serdata_t qq = serialize (serpool, topic, arg.payload.v_message);
      res = prettyprint_serdata (tmp, tmpsize, qq);
      serdata_release (qq);
#endif
    }
    else
    {
      serdata_t qq = serialize_key (serpool, topic, arg.payload.v_message);
      res = prettyprint_serdata (tmp, tmpsize, qq);
      serdata_release (qq);
    }
    assert (res >= 0);
    trace (DBGFLAG_TRACING, "%s/%s:%s%s ", topic_name (topic), topic_typename (topic), tmp, res < tmpsize ? "" : " (trunc)");
  }

  if (arg.payload_kind == PK_V_MESSAGE)
  {
    if (arg.statusinfo == (STATUSINFO_DISPOSE|STATUSINFO_UNREGISTER))
    {
      trace (DBGFLAG_TROUBLE, "can't deal with combined dispose+unregister in v_message");
      arg.statusinfo = STATUSINFO_DISPOSE;
    }
    if (!fill_v_message_qos (ep, &arg, arg.statusinfo, ospl_base))
    {
      trace (DBGFLAG_TRACING, "fill_v_message_qos failed");
      c_free (arg.payload.v_message);
      return;
    }
  }

  assert (is_builtin_entityid (arg.src.entityid) || arg.payload_kind == PK_V_MESSAGE);
  handle_forall_destinations (&arg.src, &arg.dst, ep, handle_regular_helper, &arg, 1);
  switch (arg.payload_kind)
  {
    case PK_NONE:
    case PK_RAW_ALIASED:
      break;
    case PK_RAW_MALLOCED:
      abort ();
      break;
    case PK_V_MESSAGE:
      c_free (arg.payload.v_message);
      break;
  }
}

static int handle_Data (struct receiver_state *rst, const Data_t *msg, int size, int byteswap, parameter_t *qosp, struct CDRHeader *datap)
{
  /*trace (DBGFLAG_TRACING, "%lld %x %x\n", fromSN (msg->writerSN), msg->readerId.u, msg->writerId.u);*/
  trace (DBGFLAG_TRACING, "DATA(");
  switch (msg->writerId.u)
  {
    case ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER:
      /*trace (DBGFLAG_TRACING, "participant!\n");*/
      handle_SPDP (rst, msg, size, byteswap, qosp, datap);
      break;
    default:
      handle_regular (rst, msg, size, byteswap, qosp, datap);
      update_seqnr_admin_Data (rst, msg);
      break;
  }
  trace (DBGFLAG_TRACING, ")");
  return 1;
}

static void malformed_packet_received (const char *msg, const char *submsg, int len)
{
  int i;
  trace (DBGFLAG_TRACING, "MALFORMED<");
  if (submsg)
  {
    for (i = 0; i < 32 && i < len && msg + i < submsg; i++)
      trace (DBGFLAG_TRACING, "%02x", (unsigned char) msg[i]);
    trace (DBGFLAG_TRACING, " @0x%x ", (int) (submsg - msg));
    for (i = 0; i < 32 && i < len - (int) (submsg - msg); i++)
      trace (DBGFLAG_TRACING, "%02x", (unsigned char) submsg[i]);
  }
  else
  {
    for (i = 0; i < 32 && i < len; i++)
      trace (DBGFLAG_TRACING, "%02x", (unsigned char) msg[i]);
  }
  trace (DBGFLAG_TRACING, "> (note: maybe bswap'd)\n");
}

static void handle_message (const struct sockaddr_in *src, const guid_prefix_t *dst_prefix, char *msg /* NOT const - we may byteswap it */, int len)
{
  char *submsg;
  Header_t *hdr = (Header_t *) msg;
  struct receiver_state rst;
  int tsec, tusec;

  /*trace (DBGFLAG_TRACING, "handle_message\n");*/
  time_to_sec_usec (&tsec, &tusec, now ());
  trace (DBGFLAG_TRACING, "%d.%06d %s:%d => ", tsec, tusec, inet_ntoa (src->sin_addr), ntohs (src->sin_port));
  submsg = NULL;
  if (len < (int) RTPS_MESSAGE_HEADER_SIZE)
    goto malformed;
  if (hdr->protocol.id[0] != 'R' || hdr->protocol.id[1] != 'T' ||
      hdr->protocol.id[2] != 'P' || hdr->protocol.id[3] != 'S')
    goto malformed;
  if (hdr->version.major != RTPS_MAJOR || hdr->version.minor != RTPS_MINOR)
    goto malformed;

  hdr->guid_prefix = ntoh_guid_prefix (hdr->guid_prefix);
  memset (&rst, 0, sizeof (rst));
  rst.source_guid_prefix = hdr->guid_prefix;
  rst.dest_guid_prefix = *dst_prefix;
  rst.have_timestamp = 0;
  rst.vendor = hdr->vendorid;

  trace (DBGFLAG_TRACING, "HDR(%x:%x:%x vendor %d.%d) len %d\n", hdr->guid_prefix.u[0], hdr->guid_prefix.u[1], hdr->guid_prefix.u[2], rst.vendor.id[0], rst.vendor.id[1], len);

  if (ignore_own_vendor && is_own_vendor (rst.vendor))
    return;

  submsg = msg + RTPS_MESSAGE_HEADER_SIZE;
  while (submsg < msg + len)
  {
    Submessage_t *sm = (Submessage_t *) submsg;
    int byteswap;
    int octetsToNextHeader;
    int submsg_size;

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
    /*trace (DBGFLAG_TRACING, "submsg_size %d\n", submsg_size);*/

    trace (DBGFLAG_TRACING, "  ");
    switch (sm->smhdr.submessageId)
    {
      case SMID_PAD:
	trace (DBGFLAG_TRACING, "PAD");
	break;
      case SMID_ACKNACK:
	if (!valid_AckNack (&sm->acknack, submsg_size, byteswap))
	  goto malformed;
	handle_AckNack (&rst, &sm->acknack, submsg_size);
	break;
      case SMID_HEARTBEAT:
	if (!valid_Heartbeat (&sm->heartbeat, submsg_size, byteswap))
	  goto malformed;
	handle_Heartbeat (&rst, &sm->heartbeat, submsg_size);
	break;
      case SMID_GAP:
	if (!valid_Gap (&sm->gap, submsg_size, byteswap))
	  goto malformed;
	handle_Gap (&rst, &sm->gap, submsg_size);
	break;
      case SMID_INFO_TS:
	if (!valid_InfoTS (&sm->infots, submsg_size, byteswap))
	  goto malformed;
	handle_InfoTS (&rst, &sm->infots, submsg_size);
	break;
      case SMID_INFO_SRC:
	trace (DBGFLAG_TRACING, "INFO_SRC");
	break;
      case SMID_INFO_REPLY_IP4:
	trace (DBGFLAG_TRACING, "INFO_REPLY_IP4");
	break;
      case SMID_INFO_DST:
	if (!valid_InfoDST (&sm->infodst, submsg_size, byteswap))
	  goto malformed;
	handle_InfoDST (&rst, &sm->infodst, submsg_size);
	break;
      case SMID_INFO_REPLY:
	trace (DBGFLAG_TRACING, "INFO_REPLY");
	break;
      case SMID_NACK_FRAG:
	trace (DBGFLAG_TRACING, "NACK_FRAG");
	break;
      case SMID_HEARTBEAT_FRAG:
	trace (DBGFLAG_TRACING, "HEARTBEAT_FRAG");
	break;
      case SMID_DATA_FRAG:
	trace (DBGFLAG_TRACING, "DATA_FRAG");
	break;
      case SMID_DATA:
	{
	  struct CDRHeader *datap;
	  parameter_t *qosp;
	  if (!valid_Data (&sm->data, submsg_size, byteswap, &qosp, &datap))
	    goto malformed;
	  handle_Data (&rst, &sm->data, submsg_size, byteswap, qosp, datap);
	}
	break;
      default:
	trace (DBGFLAG_TRACING, "UNDEFINED");
	goto malformed;
    }
    submsg += submsg_size;
    trace (DBGFLAG_TRACING, "\n");
  }
  if (submsg != msg + len)
  {
    trace (DBGFLAG_TRACING, "short");
    goto malformed;
  }
  return;

 malformed:
  malformed_packet_received (msg, submsg, len);
}

static void do_packet (os_socket fd, struct participant *pp)
{
  static const guid_prefix_t nullprefix;
  int sz;
  char buf[8192];
  struct sockaddr_in src;
  os_uint32 srclen = sizeof (src);
  if ((sz = os_sockRecvfrom (fd, buf, sizeof (buf), (struct sockaddr *) &src, &srclen)) > 0 && sz <= 8192)
  {
#if ! MANY_SOCKETS
    handle_message (&src, &nullprefix, buf, sz);
#else
    handle_message (&src, pp ? &pp->guid.prefix : &nullprefix, buf, sz);
#endif
  }
  else if (sz > 0)
  {
    trace (DBGFLAG_TRACING, "%s:%d => truncated\n", inet_ntoa (src.sin_addr), ntohs (src.sin_port));
  }
  else
  {
    trace (DBGFLAG_TRACING, "sock %d: %d errno %d\n", fd, sz, os_sockError ());
  }
}

static void *xmit_thread(void *varg UNUSED)
{
  const double assumedrate = 10e6; /* 100 Mbit/s */
  double qlen = 0.0;
  os_int64 tlast = now();

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
    nbytes = handle_xevents();

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
      trace(DBGFLAG_TRACING, "xmit_thread: qlen %g minwait %gus\n", qlen, minwait * 1e-3);
    } else {
      qlen = 0;
      minwait = 0;
    }
    twakeup = earliest_in_xeventq(xevents);
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
	os_nanoSleep(to);
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
      os_condTimedWait(&evcond, &lock, &to);
    }
  }
  os_mutexUnlock(&lock);
  return NULL;
}

static void *recv_thread (void *varg UNUSED)
{
#define NUM_FIXED 4
  os_socket fixed[NUM_FIXED];
  fixed[0] = discsock_uc;
  fixed[1] = discsock_mc;
  fixed[2] = datasock_uc;
  fixed[3] = datasock_mc;
  os_mutexLock (&lock);
  while (keepgoing)
  {
    int select_res, i;
    fd_set fdset, fdset_err;
    int maxsock;
    os_time timeout;
#if DO_VGLOBALS
    vglobals ();
#endif
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
    for (i = 0; i < nparticipants; i++)
    {
      os_socket sock = participant_set[i]->sock;
      FD_SET (sock, &fdset);
      if (sock > maxsock)
	maxsock = sock;
      participant_set_changed = 0;
    }

    os_mutexUnlock (&lock);
    timeout.tv_sec = 0;
    timeout.tv_nsec = 100000000; /* 100ms */
    if ((select_res = os_sockSelect (maxsock+1, &fdset, NULL, &fdset_err, &timeout)) == -1)
    {
      int err = os_sockError ();
      if (err != os_sockENOTSOCK && err != os_sockEINTR)
      {
	trace (DBGFLAG_TROUBLE, "ddsi2: select: %d\n", err);
	abort ();
      }
    }
    os_mutexLock (&lock);

#if ! MANY_SOCKETS
    assert (!participant_set_changed);
#endif
    if (!participant_set_changed)
    {
      for (i = 0; i < NUM_FIXED; i++)
      {
	if (FD_ISSET (fixed[i], &fdset))
	  do_packet (fixed[i], NULL);
      }
      for (i = 0; i < nparticipants; i++)
	if (FD_ISSET (participant_set[i]->sock, &fdset))
	  do_packet (participant_set[i]->sock, participant_set[i]);
    }
  }
  os_mutexUnlock (&lock);
  return NULL;
#undef NUM_FIXED
}

static void init_locpair (locator_t *loc, locator_udpv4_t *udpv4, struct in_addr addr, unsigned short port)
{
  /* addr in proper network format; we keep the address part of the
     locators in proper network format, but not the kind and port fields */
  memset (loc, 0, sizeof (*loc));
  loc->kind = LOCATOR_KIND_UDPv4;
  memcpy (loc->address + 12, &addr, 4);
  loc->port = port;
  loc_to_loc_udpv4 (udpv4, *loc);
}

static int find_own_ip (struct in_addr *ownip)
{
#define MAX_INTERFACES 32
  os_ifAttributes ifs[MAX_INTERFACES];
  const char *sep = " ";
  char last_if_name[80] = "";
  int quality = -1;
  os_result res;
  unsigned i;
  unsigned nif;

  trace (DBGFLAG_TRACING, "interfaces:");
  if ((res = os_sockQueryInterfaces (&ifs[0], (os_uint32) MAX_INTERFACES, &nif)) != os_resultSuccess)
  {
    trace (DBGFLAG_TROUBLE, "os_sockQueryInterfaces: %d\n", (int) res);
    return 0;
  }
  for (i = 0; i < nif; i++, sep = ", ")
  {
    struct in_addr tmpip;
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
	trace (DBGFLAG_TRACING, "%s%s", sep, if_name);
      os_strcpy (last_if_name, if_name);
      continue;
    }
    /* Get IP address, netmask, broadcast address */
    if (((struct sockaddr_in *) &ifs[i].address)->sin_family != AF_INET)
    {
      if (strcmp (if_name, last_if_name))
	trace (DBGFLAG_TRACING, "%s%s", sep, if_name);
      os_strcpy (last_if_name, if_name);
      continue;
    }
    else
    {
      tmpip = ((struct sockaddr_in *) &ifs[i].address)->sin_addr;
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
      trace (DBGFLAG_TRACING, "%s%s@q%d:%s", sep, if_name, q, inet_ntoa (tmpip));
    else
      trace (DBGFLAG_TRACING, "@q%d:%s", q, inet_ntoa (tmpip));
    if (q > quality)
    {
      quality = q;
      *ownip = tmpip;
    }
    os_strcpy (last_if_name, if_name);
  }
  trace (DBGFLAG_TRACING, "\n");
  return (quality >= 0);
#undef MAX_INTERFACES
}

static void add_peer_addresses_1 (struct addrset *as, struct in_addr ip, int port)
{
  locator_t loc;
  locator_udpv4_t loc4;
  assert (port == 0 || (port > 0 && port < 65536));
  memset (&loc, 0, sizeof (loc));
  loc.kind = LOCATOR_KIND_UDPv4;
  memcpy (loc.address + 12, &ip, 4);
  loc.port = port;
  loc_to_loc_udpv4 (&loc4, loc);
  if (port != 0)
  {
    trace (DBGFLAG_TRACING, "add_peer_addresses: add %s:%d\n", inet_ntoa (ip), loc4.port);
    add_to_addrset (as, &loc4);
  }
  else if (is_mcaddr (&loc4))
  {
    loc4.port = PORT_BASE + PORT_DG * domainid + PORT_d0;
    trace (DBGFLAG_TRACING, "add_peer_addresses: add %s:%d\n", inet_ntoa (ip), loc4.port);
    add_to_addrset (as, &loc4);
  }
  else
  {
    int i;
    trace (DBGFLAG_TRACING, "add_peer_addresses: add %s", inet_ntoa (ip));
    for (i = 0; i < 10; i++)
    {
      loc4.port = PORT_BASE + PORT_DG * domainid + i * PORT_PG + PORT_d1;
      trace (DBGFLAG_TRACING, "%s:%d", (i == 0) ? "" : ", ", loc4.port);
      add_to_addrset (as, &loc4);
    }
    trace (DBGFLAG_TRACING, "\n");
  }
}

static int inet_from_string (const char *str, struct in_addr *addr)
{
#if 0
  /* Windows doesn't provide inet_aton, only inet_addr. Naturally,
     inet_addr is deprecated in Unix ... It'll have to do. */
  in_addr_t x = inet_addr (str);
  if (x == (in_addr_t) -1)
    return 0;
  else
  {
    addr->s_addr = x;
    return 1;
  }
#else
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
#endif
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
	trace (DBGFLAG_TRACING, "add_peer_addresses: %d: invalid port\n", port);
	exit (1);
      }
    }
    if (inet_from_string (a, &ip))
      add_peer_addresses_1 (as, ip, port);
    else
    {
      trace (DBGFLAG_TRACING, "add_peer_addresses: %s not a valid address\n", a);
      exit (1);
    }
  }
  os_free (addrs_copy);
}

static int make_uc_sockets (int ppid)
{
  int r;
  if ((r = make_socket (&discsock_uc, PORT_BASE + PORT_DG * domainid + ppid * PORT_PG + PORT_d1, NULL)) < 0)
    return r;
  if ((r = make_socket (&datasock_uc, PORT_BASE + PORT_DG * domainid + ppid * PORT_PG + PORT_d3, NULL)) < 0)
  {
    os_sockFree (discsock_uc);
    return r;
  }
  return 0;
}

void rtps_init (void *vbase, int pdomainid, int ppid, unsigned flags, const char *lcl_ownip, const char *lcl_peer_addrs)
{
  tstart = now ();

  /* Print start time for referencing relative times in the remainder
     of the log. */
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
    trace (DBGFLAG_TRACING, "TIME %d.06%u %s\n", sec, usec, str);
  }
  
  use_mcast_flag = (flags & RTPS_USE_MCAST) ? 1 : 0;
  use_mcast_loopback = (flags & RTPS_USE_MCAST_LOOPBACK) ? 1 : 0;
  ignore_own_vendor = (flags & RTPS_IGNORE_OWN_VENDOR) ? 1 : 0;
  aggressive_keep_last1_whc_flag = (flags & RTPS_AGGRESSIVE_KEEP_LAST1_WHC) ? 1 : 0;
  conservative_builtin_reader_startup_flag = (flags & RTPS_CONSERVATIVE_BUILTIN_READER_STARTUP) ? 1 : 0;
  noqueue_heartbeat_messages_flag = (flags & RTPS_NOQUEUE_HEARTBEAT_MESSAGES) ? 1 : 0;
  domainid = pdomainid;
  ospl_base = vbase;

  inet_from_string ("239.255.0.1", &mcip);
  if (lcl_ownip)
  {
    if (!inet_from_string (lcl_ownip, &ownip))
    {
      trace (DBGFLAG_TROUBLE, "rtps_init: %s: not a valid IP address\n", lcl_ownip);
      exit (1);
    }
  }
  else if (!find_own_ip (&ownip))
  {
    trace (DBGFLAG_TROUBLE, "rtps_init: failed to determine own IP address\n");
    exit (1);
  }
  trace (DBGFLAG_TRACING, "CONFIG own ip: %s\n", inet_ntoa (ownip));
  trace (DBGFLAG_TRACING, "CONFIG mcast: %s\n", use_mcast_flag ? "true" : "false");
  trace (DBGFLAG_TRACING, "CONFIG mcast-loopback: %s\n", use_mcast_loopback ? "true" : "false");
  trace (DBGFLAG_TRACING, "CONFIG coexist-with-native-networking: %s\n", ignore_own_vendor ? "true" : "false");
  trace (DBGFLAG_TRACING, "CONFIG aggressive-keep-last1-whc: %s\n", aggressive_keep_last1_whc_flag ? "true" : "false");
  trace (DBGFLAG_TRACING, "CONFIG conservative-builtin-reader-startup: %s\n", conservative_builtin_reader_startup_flag ? "true" : "false");
  trace (DBGFLAG_TRACING, "CONFIG noqueue-heartbeat-messages: %s\n", noqueue_heartbeat_messages_flag ? "true" : "false");

  osplser_init (ospl_base);
  serpool = serstatepool_new ();

  default_presentation_qospolicy.access_scope = INSTANCE_PRESENTATION_QOS;
  default_presentation_qospolicy.coherent_access = 0;
  default_presentation_qospolicy.ordered_access = 0;

  avl_init (&pptree, offsetof (struct participant, avlnode), offsetof (struct participant, guid), compare_guid, 0, free_participant);
  avl_init (&proxypptree, offsetof (struct proxy_participant, avlnode), offsetof (struct proxy_participant, guid), compare_guid, augment_proxy_participant, free_proxy_participant);
  avl_init (&proxyeptree, offsetof (struct proxy_endpoint, avlnode), offsetof (struct proxy_endpoint, guid), compare_guid, augment_proxy_endpoint, free_proxy_endpoint);
  xevents = new_xeventq ();

  if (ppid >= 0)
  {
    if (make_uc_sockets (ppid) < 0)
    {
      trace (DBGFLAG_TROUBLE, "rtps_init: failed to create unicast sockets for domain %d participant %d\n", domainid, ppid);
      exit (1);
    }
  }
  else
  {
    const int max_attempts = 10;
    trace (DBGFLAG_TRACING, "rtps_init: trying to find a free participant index\n");
    for (ppid = 0; ppid < max_attempts; ppid++)
    {
      int r = make_uc_sockets (ppid);
      if (r == 0) /* Success! */
	break;
      else if (r == -1) /* Try next one */
	continue;
      else /* Oops! */
      {
	trace (DBGFLAG_TROUBLE, "rtps_init: failed to create unicast sockets for domain %d participant %d\n", domainid, ppid);
	exit (1);
      }
    }
    if (ppid == max_attempts)
    {
      trace (DBGFLAG_TROUBLE, "rtps_init: failed to find a free participant index for domain %d\n", domainid);
      exit (1);
    }
  }
  trace (DBGFLAG_TRACING, "domainid %d participantid %d\n", domainid, ppid);
  participantid = ppid;

  if (make_socket (&discsock_mc, PORT_BASE + PORT_DG * domainid + PORT_d0, &mcip) < 0 ||
      make_socket (&datasock_mc, PORT_BASE + PORT_DG * domainid + PORT_d2, &mcip) < 0)
  {
    trace (DBGFLAG_TROUBLE, "rtps_init: failed to create multicast sockets for domain %d participant %d\n", domainid, ppid);
    exit (1);
  }

  init_locpair (&loc_meta_uc, &udpv4_meta_uc, ownip, PORT_BASE + PORT_DG * domainid + ppid * PORT_PG + PORT_d1);
  init_locpair (&loc_meta_mc, &udpv4_meta_mc, mcip, PORT_BASE + PORT_DG * domainid + PORT_d0);
  init_locpair (&loc_default_uc, &udpv4_default_uc, ownip, PORT_BASE + PORT_DG * domainid + ppid * PORT_PG + PORT_d3);
  init_locpair (&loc_default_mc, &udpv4_default_mc, mcip, PORT_BASE + PORT_DG * domainid + PORT_d2);

  as_disc_init = new_addrset ();
  add_to_addrset (as_disc_init, &udpv4_meta_mc);
  if (lcl_peer_addrs)
    add_peer_addresses (as_disc_init, lcl_peer_addrs);
  as_disc = new_addrset ();
  copy_addrset_into_addrset (as_disc, as_disc_init);
  trace (DBGFLAG_TRACING, "sockets: disc_uc %d disc_mc %d uc %d mc %d\n", discsock_uc, discsock_mc, datasock_uc, datasock_mc);

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

  qxev_info (xevents, 0);
}

void rtps_term (void)
{
  os_mutexLock (&lock);
  keepgoing = 0; /* so threads will stop once they get round to checking */
  os_condBroadcast (&throttle_cond);
  os_condSignal (&evcond); /* should cause xmit_thread to check */
  participant_set_changed = 1;
  os_sockFree (discsock_uc); /* any one of these'll cause recv_thread to check */
  os_sockFree (discsock_mc);
  os_sockFree (datasock_uc);
  os_sockFree (datasock_mc);
  os_mutexUnlock (&lock);
  os_threadWaitExit (xmit_tid, NULL);
  os_threadWaitExit (recv_tid, NULL);
  /* deleting (proxy) participants will take down all (proxy)
     readers/writers as well */
  while (!avl_empty (&proxypptree))
    avl_delete (&proxypptree, proxypptree.root);
  assert (avl_empty (&proxyeptree));
  while (!avl_empty (&pptree))
    delete_participant (pptree.root);
  free_xeventq (xevents);
  unref_addrset (as_disc);
  unref_addrset (as_disc_init);
  os_condDestroy (&evcond);
  os_condDestroy (&throttle_cond);
  os_mutexDestroy (&lock);

  serstatepool_free (serpool);
  osplser_fini ();
}

unsigned rtps_get_debug_flags (void)
{
  return debugflags;
}

unsigned rtps_set_debug_flags (unsigned fl)
{
  unsigned old = debugflags;
  debugflags = fl;
  return old;
}

void rtps_set_trace_function (int (*tf) (const char *fmt, va_list ap))
{
  trace_function = tf;
}

#if DO_VGLOBALS
struct vm {
  STRUCT_AVLNODE (vmwark_avlnode, struct vm *) avlnode;
  const void *p;
  int count;
};

static STRUCT_AVLTREE (vadm, struct vm *) vadm;

static int compare_ptr (const void *va, const void *vb)
{
  char const * const *a = va;
  char const * const *b = vb;
  return (*a == *b) ? 0 : (*a < *b) ? -1 : 1;
}

static int vm (const void *p)
{
  avlparent_t parent;
  struct vm *n;
  if ((n = avl_lookup (&vadm, &p, &parent)) == NULL)
  {
    n = os_malloc (sizeof (*n));
    avl_init_node (n, parent);
    n->p = p;
    n->count = 0;
    avl_insert (&vadm, n);
  }
  return n->count++;
}

typedef void (*vft) (const void *x);
struct wavl_helper_arg {
  vft f;
};
static int wavl_helper (void *node, void *arg)
{
  ((struct wavl_helper_arg *) arg)->f (node);
  return AVLWALK_CONTINUE;
}

static void wavl (const void *avltree, vft f)
{
  struct wavl_helper_arg arg;
  arg.f = f;
  avl_walk ((void *) avltree, wavl_helper, &arg);
}

static void vmsg (const struct msg *m);
static void vxevent (const struct xevent *ev);
static void vxeventq (const struct xeventq *q);
static void vaddrset (const struct addrset *as);
static void vparticipant (const struct participant *pp);
static void vwriter (const struct writer *w);
static void vreader (const struct reader *r);
static void vrhc_writers_node (const struct rhc_writers_node *n);
static void vwhc_readers_node (const struct whc_readers_node *n);
static void vwhc_tlidx_node (const struct whc_node *n);
static void vwhc_seq_node (const struct whc_node *n);
static void vproxy_endpoint (const struct proxy_endpoint *ep);
static void vproxy_participant (const struct proxy_participant *pp);

static void vmsg (const struct msg *m UNUSED_NDEBUG)
{
#ifndef NDEBUG
  int c = vm (m);
  assert (c < m->refc);
#endif
}

static void vxevent (const struct xevent *ev)
{
  if (vm (ev)) return;
  vxeventq (ev->evq);
  switch (ev->kind)
  {
    case XEVK_HEARTBEAT: vwriter (ev->u.heartbeat.wr); break;
    case XEVK_ACKNACK: vrhc_writers_node (ev->u.acknack.rwn); break;
    case XEVK_MSG: vaddrset (ev->u.data.dest_all); vmsg (ev->u.data.msg); break;
    case XEVK_DATA: vaddrset (ev->u.data.dest_all); vmsg (ev->u.data.msg); break;
    case XEVK_DATA_RESEND: vproxy_endpoint (ev->u.data_resend.prd); vaddrset (ev->u.data.dest_all); vmsg (ev->u.data_resend.msg); break;
    case XEVK_SPDP: vmsg (ev->u.spdp.msg); break;
    case XEVK_GAP: vproxy_endpoint (ev->u.gap.prd); vmsg (ev->u.gap.msg); break;
    case XEVK_PMD_UPDATE: vparticipant (ev->u.pmd_update.pp); break;
    case XEVK_INFO: break;
    default: abort ();
  }
}

static void vxeventq (const struct xeventq *q)
{
  if (vm (q)) return;
  wavl (&q->xevents, (vft) vxevent);
}

static void vaddrset (const struct addrset *as UNUSED_NDEBUG)
{
#ifndef NDEBUG
  int c = vm (as);
  assert (c < as->refc);
#endif
}

static void vproxy_participant (const struct proxy_participant *pp)
{
  int c = vm (pp);
  assert (c < pp->refc);
  if (c == 0)
  {
    vaddrset (pp->as_default);
    vaddrset (pp->as_meta);
  }
}

static void vproxy_endpoint (const struct proxy_endpoint *ep)
{
  if (vm (ep)) return;
  vproxy_participant (ep->pp);
  vaddrset (ep->as);
  if (is_writer_entityid (ep->guid.entityid))
    wavl (&ep->matched_locals.readers, (vft) vrhc_writers_node);
  else
    wavl (&ep->matched_locals.readers, (vft) vwhc_readers_node);
}

static void vwhc_seq_node (const struct whc_node *n)
{
  if (vm (n)) return;
  vmsg (n->msg);
}

static void vwhc_tlidx_node (const struct whc_node *n)
{
  if (vm (n)) return;
  assert (n->in_tlidx);
}

static void vwhc_readers_node (const struct whc_readers_node *n)
{
  if (vm (n)) return;
  vwriter (n->writer);
  vproxy_endpoint (n->proxy_reader);
}

static void vwriter (const struct writer *w)
{
  if (vm (w)) return;
  vparticipant (w->participant);
  vaddrset (w->as);
  if (w->reliable)
    vxevent (w->heartbeat_xevent);
  wavl (&w->whc_seq, (vft) vwhc_seq_node);
  wavl (&w->whc_tlidx, (vft) vwhc_tlidx_node);
  wavl (&w->readers, (vft) vwhc_readers_node);
}

static void vout_of_seq_msg (const struct out_of_seq_msg *m)
{
  if (vm (m)) return;
}

static void vrhc_writers_node (const struct rhc_writers_node *n)
{
  if (vm (n)) return;
  vreader (n->reader);
  vproxy_endpoint (n->proxy_writer);
  if (n->reader->reliable)
    vxevent (n->acknack_xevent);
  wavl (&n->osa.msgs, (vft) vout_of_seq_msg);
}

static void vreader (const struct reader *r)
{
  if (vm (r)) return;
  vparticipant (r->participant);
  wavl (&r->writers, (vft) vrhc_writers_node);
}

static void vparticipant (const struct participant *pp)
{
  if (vm (pp)) return;
  wavl (&pp->writers, (vft) vwriter);
  wavl (&pp->readers, (vft) vreader);
  vwriter (pp->spdp_pp_writer);
  vwriter (pp->sedp_reader_writer);
  vwriter (pp->sedp_writer_writer);
  vwriter (pp->participant_message_writer);
  if (pp->spdp_pp_reader)
    vreader (pp->spdp_pp_reader);
  if (pp->sedp_reader_reader)
    vreader (pp->sedp_reader_reader);
  if (pp->sedp_writer_reader)
    vreader (pp->sedp_writer_reader);
  if (pp->participant_message_reader)
    vreader (pp->participant_message_reader);
  vxevent (pp->spdp_xevent);
}

static void vglobals (void)
{
  if (debugflags & DBGFLAG_MEMCHECK)
  {
    avl_init (&vadm, offsetof (struct vm, avlnode), offsetof (struct vm, p), compare_ptr, 0, os_free);
    wavl (&proxypptree, (vft) vproxy_participant);
    wavl (&proxyeptree, (vft) vproxy_endpoint);
    wavl (&pptree, (vft) vparticipant);
    vxeventq (xevents);
    vaddrset (as_disc);
    avl_free (&vadm);
  }
}
#endif /* DO_VGLOBALS */
