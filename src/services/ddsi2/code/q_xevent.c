#include <math.h>
#include <stdlib.h>

#include "os_heap.h"
#include "os_cond.h"
#include "os_mutex.h"
#include "os_defs.h"
#include "os_stdlib.h"

#include "ut_avl.h"
#include "q_time.h"
#include "q_log.h"
#include "q_addrset.h"
#include "q_xmsg.h"
#include "q_whc.h"
#include "q_xevent.h"
#include "q_thread.h"
#include "q_config.h"
#include "q_unused.h"
#include "q_globals.h"
#include "q_ephash.h"
#include "q_transmit.h"
#include "q_error.h"
#include "q_bswap.h"
#include "q_entity.h"
#include "q_misc.h"
#include "q_radmin.h"
#include "q_bitset.h"
#include "q_osplser.h"
#include "q_lease.h"
#include "q_xmsg.h"
#include "q_fibheap.h"

#include "sysdeps.h"

/* This is absolute bottom for signed integers, where -x = x and yet x
   != 0 -- and note that it had better be 2's complement machine! */
#define TSCHED_DELETE ((os_int64) ((os_uint64) 1 << 63))

#if __STDC_VERSION__ >= 199901L
#define POS_INFINITY_DOUBLE INFINITY
#else
/* Hope for the best -- the only consequence of getting this wrong is
   that T_NEVER may be printed as a fugly value instead of as +inf. */
#define POS_INFINITY_DOUBLE (HUGE_VAL + HUGE_VAL)
#endif

enum xeventkind {
  XEVK_HEARTBEAT,
  XEVK_ACKNACK,
  XEVK_SPDP,
  XEVK_PMD_UPDATE,
  XEVK_END_STARTUP_MODE,
  XEVK_DELETE_WRITER,
  XEVK_CALLBACK
};

struct xevent {
  struct fibheap_node heapnode;
  struct xeventq *evq;
  os_int64 tsched;
  enum xeventkind kind;
  union {
    struct {
      nn_guid_t wr_guid;
    } heartbeat;
    struct {
      nn_guid_t pwr_guid;
      nn_guid_t rd_guid;
    } acknack;
    struct {
      nn_guid_t pp_guid;
      nn_guid_prefix_t dest_proxypp_guid_prefix; /* only if "directed" */
      int directed;
    } spdp;
    struct {
      nn_guid_t pp_guid;
    } pmd_update;
#if 0
    struct {
    } info;
#endif
#if 0
    struct {
    } end_startup_mode;
#endif
    struct {
      nn_guid_t guid;
    } delete_writer;
    struct {
      void (*cb) (struct xevent *ev, void *arg, os_int64 tnow);
      void *arg;
    } callback;
  } u;
};

enum xeventkind_nt {
  XEVK_MSG,
  XEVK_MSG_REXMIT
};

struct untimed_listelem {
  struct xevent_nt *next;
};

struct xevent_nt {
  struct untimed_listelem listnode;
  struct xeventq *evq;
  enum xeventkind_nt kind;
  union {
    struct {
      /* xmsg is self-contained / relies on reference counts */
      struct nn_xmsg *msg;
    } msg;
    struct {
      /* xmsg is self-contained / relies on reference counts */
      struct nn_xmsg *msg;
      unsigned queued_rexmit_bytes;
      ut_avlNode_t msg_avlnode;
    } msg_rexmit;
  } u;
};

struct xeventq {
  struct fibheap xevents;
  ut_avlTree_t msg_xevents;
  struct xevent_nt *non_timed_xmit_list_oldest;
  struct xevent_nt *non_timed_xmit_list_newest; /* undefined if ..._oldest == NULL */
  unsigned queued_rexmit_bytes;
  unsigned queued_rexmit_msgs;
  unsigned max_queued_rexmit_bytes;
  unsigned max_queued_rexmit_msgs;
  int terminate;
  struct thread_state1 *ts;
  os_mutex lock;
  os_cond cond;
  ddsi_tran_conn_t tev_conn;
};

static void *xevent_thread (struct xeventq *xevq);
static os_int64 earliest_in_xeventq (struct xeventq *evq);
static int msg_xevents_cmp (const void *a, const void *b);

static const ut_avlTreedef_t msg_xevents_treedef = UT_AVL_TREEDEF_INITIALIZER_INDKEY (offsetof (struct xevent_nt, u.msg_rexmit.msg_avlnode), offsetof (struct xevent_nt, u.msg_rexmit.msg), msg_xevents_cmp, 0);

static int compare_xevent_tsched (const void *va, const void *vb)
{
  const struct xevent *a = va;
  const struct xevent *b = vb;
  return (a->tsched == b->tsched) ? 0 : (a->tsched < b->tsched) ? -1 : 1;
}

static void update_rexmit_counts (struct xeventq *evq, struct xevent_nt *ev)
{
#if 0
  TRACE (("ZZZ(%p,%u)", (void *) ev, ev->u.msg_rexmit.queued_rexmit_bytes));
#endif
  assert (ev->kind == XEVK_MSG_REXMIT);
  assert (ev->u.msg_rexmit.queued_rexmit_bytes <= evq->queued_rexmit_bytes);
  assert (evq->queued_rexmit_msgs > 0);
  evq->queued_rexmit_bytes -= ev->u.msg_rexmit.queued_rexmit_bytes;
  evq->queued_rexmit_msgs--;
}

#if 0
static void trace_msg (const char *func, const struct nn_xmsg *m)
{
  if (config.enabled_logcats & LC_TRACE)
  {
    nn_guid_t wrguid;
    os_int64 wrseq;
    nn_fragment_number_t wrfragid;
    nn_xmsg_guid_seq_fragid (m, &wrguid, &wrseq, &wrfragid);
    TRACE ((" %s(%x:%x:%x:%x/%lld/%u)", func, PGUID (wrguid), wrseq, wrfragid));
  }
}
#else
static void trace_msg (UNUSED_ARG (const char *func), UNUSED_ARG (const struct nn_xmsg *m))
{
}
#endif

static struct xevent_nt *lookup_msg (struct xeventq *evq, struct nn_xmsg *msg)
{
  assert (nn_xmsg_kind (msg) == NN_XMSG_KIND_DATA_REXMIT);
  trace_msg ("lookup-msg", msg);
  return ut_avlLookup (&msg_xevents_treedef, &evq->msg_xevents, msg);
}

static void remember_msg (struct xeventq *evq, struct xevent_nt *ev)
{
  assert (ev->kind == XEVK_MSG_REXMIT);
  trace_msg ("remember-msg", ev->u.msg_rexmit.msg);
  ut_avlInsert (&msg_xevents_treedef, &evq->msg_xevents, ev);
}

static void forget_msg (struct xeventq *evq, struct xevent_nt *ev)
{
  assert (ev->kind == XEVK_MSG_REXMIT);
  trace_msg ("forget-msg", ev->u.msg_rexmit.msg);
  ut_avlDelete (&msg_xevents_treedef, &evq->msg_xevents, ev);
}

static void add_to_non_timed_xmit_list (struct xeventq *evq, struct xevent_nt *ev)
{
  ev->listnode.next = NULL;
  if (evq->non_timed_xmit_list_oldest == NULL) {
    /* list is currently empty so add the first item (at the front) */
    evq->non_timed_xmit_list_oldest = ev;
  } else {
    evq->non_timed_xmit_list_newest->listnode.next = ev;
  }
  evq->non_timed_xmit_list_newest = ev;

  if (ev->kind == XEVK_MSG_REXMIT)
    remember_msg (evq, ev);

  os_condSignal (&evq->cond);
}

static struct xevent_nt *getnext_from_non_timed_xmit_list  (struct xeventq *evq)
{
  /* function removes and returns the first item in the list
     (from the front) and frees the container */
  struct xevent_nt *ev = evq->non_timed_xmit_list_oldest;
  if (ev != NULL)
  {
    evq->non_timed_xmit_list_oldest = ev->listnode.next;

    if (ev->kind == XEVK_MSG_REXMIT)
    {
      assert (lookup_msg (evq, ev->u.msg_rexmit.msg) == ev);
      forget_msg (evq, ev);
    }
  }
  return ev;
}

static int non_timed_xmit_list_is_empty (struct xeventq *evq)
{
  /* check whether the "non-timed" xevent list is empty */
  return (evq->non_timed_xmit_list_oldest == NULL);
}

static int compute_non_timed_xmit_list_size (struct xeventq *evq)
{
  /* returns how many "non-timed" xevents are pending by counting the
     number of events in the list -- it'd be easy to compute the
     length incrementally in the add_... and next_... functions, but
     it isn't really being used anywhere, so why bother? */
  struct xevent_nt *current = evq->non_timed_xmit_list_oldest;
  int i = 0;
  while (current)
  {
    current = current->listnode.next;
    i++;
  }
  return i;
}

#ifndef NDEBUG
static int nontimed_xevent_in_queue (struct xeventq *evq, struct xevent_nt *ev)
  {
  struct xevent_nt *x;
  for (x = evq->non_timed_xmit_list_oldest; x; x = x->listnode.next)
    {
    if (x == ev)
      return 1;
  }
  return 0;
}
#endif

static void free_xevent (struct xeventq *evq, struct xevent *ev)
{
  (void) evq;
  if (ev->tsched != TSCHED_DELETE)
  {
    switch (ev->kind)
    {
      case XEVK_HEARTBEAT:
      case XEVK_ACKNACK:
      case XEVK_SPDP:
      case XEVK_PMD_UPDATE:
      case XEVK_END_STARTUP_MODE:
      case XEVK_DELETE_WRITER:
      case XEVK_CALLBACK:
        break;
    }
  }
  os_free (ev);
}

static void free_xevent_nt (struct xeventq *evq, struct xevent_nt *ev)
{
  assert (!nontimed_xevent_in_queue (evq, ev));
  switch (ev->kind)
  {
    case XEVK_MSG:
      nn_xmsg_free (ev->u.msg.msg);
      break;
    case XEVK_MSG_REXMIT:
      assert (ut_avlLookup (&msg_xevents_treedef, &evq->msg_xevents, ev->u.msg_rexmit.msg) == NULL);
      update_rexmit_counts (evq, ev);
      nn_xmsg_free (ev->u.msg_rexmit.msg);
      break;
  }
  os_free (ev);
}

void delete_xevent (struct xevent *ev)
{
  struct xeventq *evq = ev->evq;
  os_mutexLock (&evq->lock);
  /* Can delete it only once, no matter how we implement it internally */
  assert (ev->tsched != TSCHED_DELETE);
  assert (TSCHED_DELETE < ev->tsched);
  if (ev->tsched != T_NEVER)
  {
    ev->tsched = TSCHED_DELETE;
    fh_decreasekey (&evq->xevents, ev);
  }
  else
  {
    ev->tsched = TSCHED_DELETE;
    fh_insert (&evq->xevents, ev);
  }
  /* TSCHED_DELETE is absolute minimum time, so chances are we need to
     wake up the thread.  The superfluous signal is harmless. */
  os_condSignal (&evq->cond);
  os_mutexUnlock (&evq->lock);
}

int resched_xevent_if_earlier (struct xevent *ev, os_int64 tsched)
{
  struct xeventq *evq = ev->evq;
  int is_resched;
  os_mutexLock (&evq->lock);
  assert (tsched != TSCHED_DELETE);
  /* If you want to delete it, you to say so by calling the right
     function. Don't want to reschedule an event marked for deletion,
     but with TSCHED_DELETE = MIN_INT64, tsched >= ev->tsched is
     guaranteed to be false. */
  assert (tsched > TSCHED_DELETE);
  if (tsched >= ev->tsched)
    is_resched = 0;
  else
  {
    os_int64 tbefore = earliest_in_xeventq (evq);
    assert (tsched != T_NEVER);
    if (ev->tsched != T_NEVER)
    {
      ev->tsched = tsched;
      fh_decreasekey (&evq->xevents, ev);
    }
    else
    {
      ev->tsched = tsched;
      fh_insert (&evq->xevents, ev);
    }
    is_resched = 1;
    if (tsched < tbefore)
      os_condSignal (&evq->cond);
  }
  os_mutexUnlock (&evq->lock);
  return is_resched;
}

static struct xevent *qxev_common (struct xeventq *evq, os_int64 tsched, enum xeventkind kind)
{
  /* qxev_common is the route by which all timed xevents are
     created. */
  struct xevent *ev = os_malloc (sizeof (*ev));

  assert (tsched != TSCHED_DELETE);
  ASSERT_MUTEX_HELD (&evq->lock);

  /* round up the scheduled time if required */
  if (tsched != T_NEVER && config.schedule_time_rounding != 0)
  {
    os_int64 tsched_rounded = time_round_up (tsched, config.schedule_time_rounding);
    TRACE (("rounded event scheduled for %lld to %lld\n", tsched, tsched_rounded));
    tsched = tsched_rounded;
  }

  ev->evq = evq;
  ev->tsched = tsched;
  ev->kind = kind;
  return ev;
}

static struct xevent_nt *qxev_common_nt (struct xeventq *evq, enum xeventkind_nt kind)
{
  /* qxev_common_nt is the route by which all non-timed xevents are created. */
  struct xevent_nt *ev = os_malloc (sizeof (*ev));
  ev->evq = evq;
  ev->kind = kind;
  return ev;
}

static os_int64 earliest_in_xeventq (struct xeventq *evq)
{
  struct xevent *min;
  ASSERT_MUTEX_HELD (&evq->lock);
  if ((min = fh_min (&evq->xevents)) == NULL)
    return T_NEVER;
  else
    return min->tsched;
}

static void qxev_insert (struct xevent *ev)
{
  /* qxev_insert is how all timed xevents are registered into the
     event administration. */
  struct xeventq *evq = ev->evq;
  ASSERT_MUTEX_HELD (&evq->lock);
  if (ev->tsched != T_NEVER)
  {
    os_int64 tbefore = earliest_in_xeventq (evq);
    fh_insert (&evq->xevents, ev);
    if (ev->tsched < tbefore)
      os_condSignal (&evq->cond);
  }
}

static void qxev_insert_nt (struct xevent_nt *ev)
{
  /* qxev_insert is how all non-timed xevents are queued. */
  struct xeventq *evq = ev->evq;
  ASSERT_MUTEX_HELD (&evq->lock);
  add_to_non_timed_xmit_list (evq, ev);
  TRACE (("non-timed queue now has %d items\n", compute_non_timed_xmit_list_size (evq)));
}

static int msg_xevents_cmp (const void *a, const void *b)
{
  return nn_xmsg_compare_fragid (a, b);
}

struct xeventq * xeventq_new
(
  ddsi_tran_conn_t conn,
  unsigned max_queued_rexmit_bytes,
  unsigned max_queued_rexmit_msgs
)
{
  struct xeventq *evq = os_malloc (sizeof (*evq));
  /* limit to 2GB to prevent overflow (4GB - 64kB should be ok, too) */
  if (max_queued_rexmit_bytes > 2147483648u)
    max_queued_rexmit_bytes = 2147483648u;
  fh_init (&evq->xevents, offsetof (struct xevent, heapnode), compare_xevent_tsched);
  ut_avlInit (&msg_xevents_treedef, &evq->msg_xevents);
  evq->non_timed_xmit_list_oldest = NULL;
  evq->non_timed_xmit_list_newest = NULL;
  evq->terminate = 0;
  evq->ts = NULL;
  evq->max_queued_rexmit_bytes = max_queued_rexmit_bytes;
  evq->max_queued_rexmit_msgs = max_queued_rexmit_msgs;
  evq->queued_rexmit_bytes = 0;
  evq->queued_rexmit_msgs = 0;
  evq->tev_conn = conn;
  os_mutexInit (&evq->lock, &gv.mattr);
  os_condInit (&evq->cond, &evq->lock, &gv.cattr);
  return evq;
}

int xeventq_start (struct xeventq *evq, const char *name)
{
  char *evqname;
  assert (evq->ts == NULL);

  if (name == NULL)
    evqname = "tev";
  else if ((evqname = os_malloc (strlen (name) + 5)) == NULL)
    return ERR_OUT_OF_MEMORY;
  else
    sprintf (evqname, "tev.%s", name);

  evq->terminate = 0;
  evq->ts = create_thread (evqname, (void * (*) (void *)) xevent_thread, evq);

  if (name != NULL)
    os_free (evqname);
  return (evq->ts == NULL) ? ERR_UNSPECIFIED : 0;
}

void xeventq_stop (struct xeventq *evq)
{
  assert (evq->ts != NULL);
  os_mutexLock (&evq->lock);
  evq->terminate = 1;
  os_condSignal (&evq->cond);
  os_mutexUnlock (&evq->lock);
  join_thread (evq->ts, NULL);
  evq->ts = NULL;
}

void xeventq_free (struct xeventq *evq)
{
  struct xevent *ev;
  assert (evq->ts == NULL);
  while ((ev = fh_extractmin (&evq->xevents)) != NULL)
    free_xevent (evq, ev);
  while (!non_timed_xmit_list_is_empty(evq))
    free_xevent_nt (evq, getnext_from_non_timed_xmit_list (evq));
  assert (ut_avlIsEmpty (&evq->msg_xevents));
  os_condDestroy (&evq->cond);
  os_mutexDestroy (&evq->lock);
  os_free (evq);
}

/* EVENT QUEUE EVENT HANDLERS ******************************************************/

static os_size_t handle_xevk_msg (struct nn_xpack *xp, struct xevent_nt *ev, UNUSED_ARG (os_int64 tnow))
{
  assert (!nontimed_xevent_in_queue (ev->evq, ev));

  nn_xpack_addmsg (xp, ev->u.msg.msg);
  os_free (ev);
  return 0;
}

static os_size_t handle_xevk_msg_rexmit (struct nn_xpack *xp, struct xevent_nt *ev, UNUSED_ARG (os_int64 tnow))
{
  struct xeventq *evq = ev->evq;

  assert (!nontimed_xevent_in_queue (ev->evq, ev));

  nn_xpack_addmsg (xp, ev->u.msg_rexmit.msg);

  /* FIXME: less than happy about having to relock the queue for a
     little while here */
  os_mutexLock (&evq->lock);
  update_rexmit_counts (evq, ev);
  os_mutexUnlock (&evq->lock);

  os_free (ev);
  return 0;
}

static os_size_t handle_xevk_heartbeat (struct nn_xpack *xp, struct xevent *ev, os_int64 tnow)
{
  struct nn_xmsg *msg;
  struct writer *wr;
  os_int64 t_next;
  int hbansreq = 0;

  if ((wr = ephash_lookup_writer_guid (&ev->u.heartbeat.wr_guid)) == NULL)
  {
    TRACE (("heartbeat(wr %x:%x:%x:%x) writer gone\n",
            PGUID (ev->u.heartbeat.wr_guid)));
    return 0;
  }

  assert (wr->reliable);
  os_mutexLock (&wr->e.lock);
  if (!writer_must_have_hb_scheduled (wr))
  {
    hbansreq = 1; /* just for trace */
    msg = NULL; /* Need not send it now, and no need to schedule it for the future */
    t_next = T_NEVER;
  }
  else if (!writer_hbcontrol_must_send (wr, tnow))
  {
    hbansreq = 1; /* just for trace */
    msg = NULL;
    t_next = tnow + writer_hbcontrol_intv (wr, tnow);
  }
  else
  {
    hbansreq = writer_hbcontrol_ack_required (wr, tnow);
    msg = writer_hbcontrol_create_heartbeat (wr, tnow, hbansreq, 0);
    t_next = tnow + writer_hbcontrol_intv (wr, tnow);
  }

  TRACE (("heartbeat(wr %x:%x:%x:%x%s) %s, resched in %g s (min-ack %lld%s, avail-seq %lld, xmit %lld)\n",
          PGUID (wr->e.guid),
          hbansreq ? "" : " final",
          msg ? "sent" : "suppressed",
          (t_next == T_NEVER) ? POS_INFINITY_DOUBLE : (t_next - tnow) / 1e9,
          ut_avlIsEmpty (&wr->readers) ? (os_int64) -1 : ((struct wr_prd_match *) ut_avlRoot (&wr_readers_treedef, &wr->readers))->min_seq,
          ut_avlIsEmpty (&wr->readers) || ((struct wr_prd_match *) ut_avlRoot (&wr_readers_treedef, &wr->readers))->all_have_replied_to_hb ? "" : "!",
          whc_empty (wr->whc) ? (os_int64) -1 : whc_max_seq (wr->whc), wr->seq_xmit));
  resched_xevent_if_earlier (ev, t_next);
  wr->hbcontrol.tsched = t_next;
  os_mutexUnlock (&wr->e.lock);

  /* Can't transmit synchronously with writer lock held: trying to add
     the heartbeat to the xp may cause xp to be sent out, which may
     require updating wr->seq_xmit for other messages already in xp.
     Besides, nn_xpack_addmsg may sleep for bandwidth-limited channels
     and we certainly don't want to hold the lock during that time. */
  if (msg)
    nn_xpack_addmsg (xp, msg);
  return 0;
}

static os_int64 next_deliv_seq (const struct proxy_writer *pwr, const os_int64 next_seq)
{
  /* We want to determine next_deliv_seq, the next sequence number to
     be delivered to all in-sync readers, so that we can acknowledge
     what we have actually delivered.  This is different from next_seq
     tracks, which tracks the sequence number up to which all samples
     have been received.  The difference is the delivery queue.

     There is always but a single delivery queue, and hence delivery
     thread, associated with a single proxy writer; but the ACKs are
     always generated by another thread.  Therefore, updates to
     next_deliv_seq need only be atomic with respect to these reads.
     On all supported platforms we can atomically load and store 32
     bits without issue, and so we store just the low word of the
     sequence number.

     We know 1 <= next_deliv_seq AND next_seq - N <= next_deliv_seq <=
     next_seq for N << 2**32.  With n = next_seq, nd = next_deliv_seq,
     H the upper half and L the lower half:

       - H(nd) <= H(n) <= H(nd)+1         { n >= nd AND N << 2*32}
       - H(n) = H(nd)   => L(n) >= L(nd)  { n >= nd }
       - H(n) = H(nd)+1 => L(n) < L(nd)   { N << 2*32 }

     Therefore:

       L(n) < L(nd) <=> H(n) = H(nd+1)

     a.k.a.:

       nd = nd' - if nd' > nd then 2**32 else 0
         where nd' = 2**32 * H(n) + L(nd)

     By not locking next_deliv_seq, we may have nd a bit lower than it
     could be, but that only means we are acknowledging slightly less
     than we could; but that is perfectly acceptible.

     FIXME: next_seq - #dqueue could probably be used instead,
     provided #dqueue is decremented after delivery, rather than
     before delivery. */
  const os_uint32 lw = atomic_read_u32 (&pwr->next_deliv_seq_lowword);
  os_int64 next_deliv_seq;
  next_deliv_seq = (next_seq & ~(os_int64) 0xffffffff) | lw;
  if (next_deliv_seq > next_seq)
    next_deliv_seq -= ((os_int64) 1) << 32;
  assert (0 < next_deliv_seq && next_deliv_seq <= next_seq);
  return next_deliv_seq;
}

static int add_AckNack (struct nn_xmsg *msg, struct proxy_writer *pwr, struct pwr_rd_match *rwn, int *isnack)
{
  /* If pwr->have_seen_heartbeat == 0, no heartbeat has been received
     by this proxy writer yet, so we'll be sending a pre-emptive
     AckNack.  NACKing data now will most likely cause another NACK
     upon reception of the first heartbeat, and so cause the data to
     be resent twice. */
  const int max_numbits = 256; /* as spec'd */
  int notail = 0; /* all known missing ones are nack'd */
  struct nn_reorder *reorder;
  AckNack_t *an;
  struct nn_xmsg_marker sm_marker;
  int i, numbits;
  os_int64 base;
  unsigned ui;

  union {
    struct nn_fragment_number_set set;
    char pad[NN_FRAGMENT_NUMBER_SET_SIZE (256)];
  } nackfrag;
  int nackfrag_numbits;
  os_int64 nackfrag_seq;
  os_int64 bitmap_base;

  ASSERT_MUTEX_HELD (pwr->e.lock);

  /* if in sync, look at proxy writer status, else look at
     proxy-writer--reader match status */
  if (rwn->in_sync)
  {
    reorder = pwr->reorder;
    if (!config.late_ack_mode)
      bitmap_base = nn_reorder_next_seq (reorder);
    else
    {
      bitmap_base = next_deliv_seq (pwr, nn_reorder_next_seq (reorder));
      if (nn_dqueue_is_full (pwr->dqueue))
        notail = 1;
    }
  }
  else
  {
    reorder = rwn->u.not_in_sync.reorder;
    bitmap_base = nn_reorder_next_seq (reorder);
  }

  an = nn_xmsg_append (msg, &sm_marker, ACKNACK_SIZE_MAX);
  nn_xmsg_submsg_init (msg, sm_marker, SMID_ACKNACK);
  an->readerId = nn_hton_entityid (rwn->rd_guid.entityid);
  an->writerId = nn_hton_entityid (pwr->e.guid.entityid);

  /* Make bitmap; note that we've made sure to have room for the
     maximum bitmap size. */
  numbits = nn_reorder_nackmap (reorder, bitmap_base, pwr->last_seq, &an->readerSNState, max_numbits, notail);
  base = fromSN (an->readerSNState.bitmap_base);

  /* Scan through bitmap, cutting it off at the first missing sample
     that the defragmenter knows about. Then note the sequence number
     & add a NACKFRAG for that sample */
  nackfrag_numbits = 0;
  for (i = 0; i < numbits && nackfrag_numbits <= 0; i++)
  {
    os_uint32 fragnum;
    nackfrag_seq = base + i;
    if (!nn_bitset_isset (numbits, an->readerSNState.bits, i))
      continue;
    if (nackfrag_seq == pwr->last_seq)
      fragnum = pwr->last_fragnum;
    else
      fragnum = 0xffffffff;
    nackfrag_numbits = nn_defrag_nackmap (pwr->defrag, nackfrag_seq, fragnum, &nackfrag.set, max_numbits);
  }
  if (nackfrag_numbits > 0) {
    /* Cut the NACK short, NACKFRAG will be added after the NACK's is
       properly formatted */
    an->readerSNState.numbits = numbits = i - 1;
  }

  /* Let caller know whether it is a nack, and, in steady state, set
     final to prevent a response if it isn't.  The initial
     (pre-emptive) acknack is different: it'd be nice to get a
     heartbeat in response.

     Who cares about an answer to an acknowledgment!? -- actually,
     that'd a very useful feature in combination with directed
     heartbeats, or somesuch, to get reliability guarantees. */
  *isnack = (numbits > 0);
  if (!pwr->have_seen_heartbeat) {
    /* We must have seen a heartbeat for us to consider setting FINAL */
  } else if (*isnack && base + numbits <= pwr->last_seq) {
    /* If it's a NACK and it doesn't cover samples all the way up to
       the highest known sequence number, there's some reason to expect
       we may to do another round.  For which we need a Heartbeat.

       Note: last_seq exists, base is first in bitmap, numbits is
       length of bitmap, hence less-than-or-equal. */
  } else {
    /* An ACK or we think we'll get everything now. */
    an->smhdr.flags |= ACKNACK_FLAG_FINAL;
  }

  /* If we refuse to send invalid AckNacks, grow a length-0 bitmap and
     zero-fill it. Cleared bits are meaningless (DDSI 2.1, table 8.33,
     although RTI seems to think otherwise). */
  if (numbits == 0 && config.acknack_numbits_emptyset > 0)
  {
    an->readerSNState.numbits = config.acknack_numbits_emptyset;
    nn_bitset_zero (an->readerSNState.numbits, an->readerSNState.bits);
  }

  {
    /* Count field is at a variable offset ... silly DDSI spec. */
    nn_count_t *countp =
      (nn_count_t *) ((char *) an + offsetof (AckNack_t, readerSNState) +
                      NN_SEQUENCE_NUMBER_SET_SIZE (an->readerSNState.numbits));
    if (rwn->count == DDSI_COUNT_MAX)
      NN_FATAL0 ("reader reached maximum acknack sequence number");
    *countp = ++rwn->count;

    /* Reset submessage size, now that we know the real size, and update
       the offset to the next submessage. */
    nn_xmsg_shrink (msg, sm_marker, ACKNACK_SIZE (an->readerSNState.numbits));
    nn_xmsg_submsg_setnext (msg, sm_marker);

    TRACE (("acknack %x:%x:%x:%x -> %x:%x:%x:%x: #%d:%lld/%d:",
            PGUID (rwn->rd_guid), PGUID (pwr->e.guid), rwn->count,
            base, an->readerSNState.numbits));
    for (ui = 0; ui != an->readerSNState.numbits; ui++)
      TRACE (("%c", nn_bitset_isset (numbits, an->readerSNState.bits, ui) ? '1' : '0'));
  }

  if (nackfrag_numbits > 0)
  {
    NackFrag_t *nf;

    /* We use 0-based fragment numbers, but externally have to provide
       1-based fragment numbers */
    assert (nackfrag_numbits == (int) nackfrag.set.numbits);

    nf = nn_xmsg_append (msg, &sm_marker, NACKFRAG_SIZE (nackfrag_numbits));

    nn_xmsg_submsg_init (msg, sm_marker, SMID_NACK_FRAG);
    nf->readerId = nn_hton_entityid (rwn->rd_guid.entityid);
    nf->writerId = nn_hton_entityid (pwr->e.guid.entityid);
    nf->writerSN = toSN (nackfrag_seq);
    nf->fragmentNumberState.bitmap_base = nackfrag.set.bitmap_base + 1;
    nf->fragmentNumberState.numbits = nackfrag.set.numbits;
    memcpy (nf->fragmentNumberState.bits, nackfrag.set.bits, NN_FRAGMENT_NUMBER_SET_BITS_SIZE (nackfrag_numbits));

    {
      nn_count_t *countp =
        (nn_count_t *) ((char *) nf + offsetof (NackFrag_t, fragmentNumberState) + NN_FRAGMENT_NUMBER_SET_SIZE (nf->fragmentNumberState.numbits));
      if (pwr->nackfragcount == DDSI_COUNT_MAX)
        NN_FATAL0 ("proxy writer reached maximum nackfrag sequence number");
      *countp = ++pwr->nackfragcount;
      nn_xmsg_submsg_setnext (msg, sm_marker);

      TRACE ((" + nackfrag #%d:%lld/%u/%d:", *countp, fromSN (nf->writerSN), nf->fragmentNumberState.bitmap_base, nf->fragmentNumberState.numbits));
      for (ui = 0; ui != nf->fragmentNumberState.numbits; ui++)
        TRACE (("%c", nn_bitset_isset (nf->fragmentNumberState.numbits, nf->fragmentNumberState.bits, ui) ? '1' : '0'));
    }
  }

  TRACE (("\n"));
  return 0;
}

static os_size_t handle_xevk_acknack (UNUSED_ARG (struct nn_xpack *xp), struct xevent *ev, os_int64 tnow)
{
  /* FIXME: ought to keep track of which NACKs are being generated in
     response to a Heartbeat.  There is no point in having multiple
     readers NACK the data.

     FIXME: ought to determine the set of missing samples (as it does
     now), and then check which for of those fragments are available already.
     A little snag is that the defragmenter can throw out partial samples in
     favour of others, so MUST ensure that the defragmenter won't start
     threshing and fail to make progress! */
  struct proxy_writer *pwr;
  os_sockaddr_storage addr;
  struct nn_xmsg *msg;
  struct pwr_rd_match *rwn;

  if ((pwr = ephash_lookup_proxy_writer_guid (&ev->u.acknack.pwr_guid)) == NULL)
    return 0;

  os_mutexLock (&pwr->e.lock);
  if ((rwn = ut_avlLookup (&pwr_readers_treedef, &pwr->readers, &ev->u.acknack.rd_guid)) == NULL)
  {
    os_mutexUnlock (&pwr->e.lock);
    return 0;
  }

  if (addrset_any_uc (pwr->c.as, &addr) || addrset_any_mc (pwr->c.as, &addr))
  {
    int isnack;
    if ((msg = nn_xmsg_new (gv.xmsgpool, &ev->u.acknack.rd_guid.prefix, ACKNACK_SIZE_MAX, NN_XMSG_KIND_CONTROL)) == NULL)
      goto outofmem;
    nn_xmsg_setdst1 (msg, &ev->u.acknack.pwr_guid.prefix, &addr);
    if (config.meas_hb_to_ack_latency && rwn->hb_timestamp)
    {
      /* If HB->ACK latency measurement is enabled, and we have a
         timestamp available, add it and clear the time stamp.  There
         is no real guarantee that the two match, but I haven't got a
         solution for that yet ...  If adding the time stamp fails,
         too bad, but no reason to get worried. */
      nn_xmsg_add_timestamp (msg, rwn->hb_timestamp);
      rwn->hb_timestamp = 0;
    }
    if (add_AckNack (msg, pwr, rwn, &isnack) < 0)
      goto outofmem;
    TRACE (("send acknack(rd %x:%x:%x:%x -> pwr %x:%x:%x:%x)\n",
            PGUID (ev->u.acknack.rd_guid), PGUID (ev->u.acknack.pwr_guid)));
  }
  else
  {
    TRACE (("skip acknack(rd %x:%x:%x:%x -> pwr %x:%x:%x:%x): no address\n",
            PGUID (ev->u.acknack.rd_guid), PGUID (ev->u.acknack.pwr_guid)));
    msg = NULL;
  }
  if (!pwr->have_seen_heartbeat && tnow - rwn->tcreate <= 300 * T_SECOND)
  {
     /* Force pre-emptive AckNacks out until we receive a heartbeat,
        but let the frequency drop over time and stop after a couple
        of minutes. */
    int intv, age = (int) ((tnow - rwn->tcreate) / T_SECOND + 1);
    if (age <= 10)
      intv = 1;
    else if (age <= 60)
      intv = 2;
    else if (age <= 120)
      intv = 5;
    else
      intv = 10;
    resched_xevent_if_earlier (ev, tnow + intv * T_SECOND);
  }
  os_mutexUnlock (&pwr->e.lock);

  /* nn_xpack_addmsg may sleep (for bandwidth-limited channels), so
     must be outside the lock */
  if (msg)
    nn_xpack_addmsg (xp, msg);
  return 0;

 outofmem:
  /* What to do if out of memory?  Crash or burn? */
  os_mutexUnlock (&pwr->e.lock);
  if (msg)
    nn_xmsg_free (msg);
  resched_xevent_if_earlier (ev, tnow + 100 * T_MILLISECOND);
  return 0;
}

static os_size_t handle_xevk_spdp (UNUSED_ARG (struct nn_xpack *xp), struct xevent *ev, os_int64 tnow)
{
  /* Like the writer pointer in the heartbeat event, the participant pointer in the spdp event is assumed valid. */
  const os_int64 mindelta = 10 * T_MILLISECOND;
  struct participant *pp;
  struct proxy_reader *prd;
  struct writer *spdp_wr;
  struct whc_node *whcn;
  serstate_t st;
  serdata_t sd;
  nn_guid_t kh;

  if ((pp = ephash_lookup_participant_guid (&ev->u.spdp.pp_guid)) == NULL)
  {
    TRACE (("handle_xevk_spdp %x:%x:%x:%x - unknown guid\n",
            PGUID (ev->u.spdp.pp_guid)));
    return 0;
  }

  if ((spdp_wr = get_builtin_writer (pp, NN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER)) == NULL)
  {
    TRACE (("handle_xevk_spdp %x:%x:%x:%x - spdp writer of participant not found\n",
            PGUID (ev->u.spdp.pp_guid)));
    goto skip;
  }

  if (!ev->u.spdp.directed)
  {
    /* memset is for tracing output */
    memset (&ev->u.spdp.dest_proxypp_guid_prefix, 0, sizeof (ev->u.spdp.dest_proxypp_guid_prefix));
    prd = NULL;
  }
  else
  {
    nn_guid_t guid;
    guid.prefix = ev->u.spdp.dest_proxypp_guid_prefix;
    guid.entityid.u = NN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_READER;
    if ((prd = ephash_lookup_proxy_reader_guid (&guid)) == NULL)
    {
      TRACE (("xmit spdp: no proxy reader %x:%x:%x:%x\n", PGUID (guid)));
      goto skip;
    }
  }

  /* Look up data in (transient-local) WHC by key value */
  if ((st = serstate_new (gv.serpool, NULL)) == NULL)
  {
    TRACE (("xmit spdp: skip %x:%x:%x:%x: out of memory\n", PGUID (ev->u.spdp.pp_guid)));
    goto skip;
  }
  kh = nn_hton_guid (ev->u.spdp.pp_guid);
  serstate_set_key (st, 1, 16, &kh);
  sd = serstate_fix (st);

  os_mutexLock (&spdp_wr->e.lock);
  if ((whcn = whc_findkey (spdp_wr->whc, sd)) != NULL)
  {
    /* Claiming it is new rather than a retransmit so that the rexmit
       limiting won't kick in.  It is best-effort and therefore the
       updating of the last transmitted sequence number won't take
       place anyway.  Nor is it necessary to fiddle with heartbeat
       control stuff. */
    enqueue_sample_wrlock_held (spdp_wr, whcn->seq, whcn->serdata, prd, 1);
  }
  os_mutexUnlock (&spdp_wr->e.lock);

  serdata_unref (sd);

#ifndef NDEBUG
  if (whcn == NULL)
  {
    /* If undirected, it is pp->spdp_xevent, and that one must never
       run into an empty WHC unless it is already marked for deletion.

       If directed, it may happen in response to an SPDP packet during
       creation of the participant.  This is because pp is inserted in
       the hash table quite early on, which, in turn, is because it
       needs to be visible for creating its builtin endpoints.  But in
       this case, the initial broadcast of the SPDP packet of pp will
       happen shortly. */
    if (!ev->u.spdp.directed)
    {
      os_mutexLock (&pp->e.lock);
      os_mutexLock (&ev->evq->lock);
      assert (ev->tsched == TSCHED_DELETE);
      os_mutexUnlock (&ev->evq->lock);
      os_mutexUnlock (&pp->e.lock);
    }
    else
    {
      TRACE (("xmit spdp: suppressing early spdp response from %x:%x:%x:%x to %x:%x:%x:%x\n",
              PGUID (pp->e.guid), PGUIDPREFIX (ev->u.spdp.dest_proxypp_guid_prefix), NN_ENTITYID_PARTICIPANT));
    }
  }
#endif

 skip:
  if (ev->u.spdp.directed)
  {
    /* Directed events are used to send SPDP packets to newly
       discovered peers, and used just once. */
    delete_xevent (ev);
  }
  else
  {
    /* NB lease duration can't change */
    os_int64 t_next = add_duration_to_time (tnow, config.spdp_interval);
    if (t_next < tnow + mindelta)
      t_next = tnow + mindelta;

    TRACE (("xmit spdp %x:%x:%x:%x to %x:%x:%x:%x (resched %gs)\n",
            PGUID (pp->e.guid),
            PGUIDPREFIX (ev->u.spdp.dest_proxypp_guid_prefix), NN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_READER,
            (t_next - tnow) / 1e9));
    resched_xevent_if_earlier (ev, t_next);
  }
  return 0;
}

static void write_pmd_message (struct nn_xpack *xp, struct participant *pp, unsigned pmd_kind)
{
#define PMD_DATA_LENGTH 1
  struct writer *wr;
  union {
    ParticipantMessageData_t pmd;
    char pad[offsetof (ParticipantMessageData_t, value) + PMD_DATA_LENGTH];
  } u;
  serdata_t serdata;
  serstate_t serstate;

  if ((wr = get_builtin_writer (pp, NN_ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_WRITER)) == NULL)
  {
    TRACE (("write_pmd_message(%x:%x:%x:%x) - builtin pmd writer not found\n", PGUID (pp->e.guid)));
    return;
  }

  u.pmd.participantGuidPrefix = nn_hton_guid_prefix (pp->e.guid.prefix);
  u.pmd.kind = toBE4u (pmd_kind);
  u.pmd.length = PMD_DATA_LENGTH;
  memset (u.pmd.value, 0, u.pmd.length);

  serstate = serstate_new (gv.serpool, NULL);
  serstate_append_blob (serstate, 4, sizeof (u.pad), &u.pmd);
  serstate_set_key (serstate, 0, 16, &u.pmd);
  serstate_set_msginfo (serstate, 0, now (), 1, NULL);
  serdata = serstate_fix (serstate);

  /* HORRIBLE HACK ALERT -- serstate/serdata looks at whether topic is
     a null pointer to choose PL_CDR_x encoding or regular CDR_x
     encoding. */
  serdata->hdr.identifier = PLATFORM_IS_LITTLE_ENDIAN ? CDR_LE : CDR_BE;

  write_sample (xp, wr, serdata);
#undef PMD_DATA_LENGTH
}

static os_size_t handle_xevk_pmd_update (struct nn_xpack *xp, struct xevent *ev, os_int64 tnow)
{
  struct participant *pp;
  os_int64 intv, tnext;

  if ((pp = ephash_lookup_participant_guid (&ev->u.pmd_update.pp_guid)) == NULL)
    return 0;

  write_pmd_message (xp, pp, PARTICIPANT_MESSAGE_DATA_KIND_AUTOMATIC_LIVELINESS_UPDATE);

  /* QoS changes can't change lease durations. So the only thing that
     could cause trouble here is that the addition or removal of a
     writer cause the interval to change for this participant.  If we
     lock pp for reading out the lease duration we are guaranteed a
     consistent value (can't assume 64-bit atomic reads on all support
     platforms!) */
  os_mutexLock (&pp->e.lock);
  intv = pp->lease_duration;

  /* FIXME: need to use smallest liveliness duration of all automatic-liveliness writers */
  if (intv == T_NEVER)
  {
    tnext = T_NEVER;
    TRACE (("resched pmd(%x:%x:%x:%x): never\n", PGUID (pp->e.guid)));
  }
  else
  {
    tnext = tnow + intv - 2 * T_SECOND;
    TRACE (("resched pmd(%x:%x:%x:%x): %gs\n", PGUID (pp->e.guid), (tnext - tnow) / 1e9));
  }

  resched_xevent_if_earlier (ev, tnext);
  os_mutexUnlock (&pp->e.lock);
  return 0;
}

static os_size_t handle_xevk_end_startup_mode (UNUSED_ARG (struct nn_xpack *xp), struct xevent *ev, UNUSED_ARG (os_int64 tnow))
{
  struct ephash_enum_writer est;
  struct writer *wr;
  assert (gv.startup_mode);
  TRACE (("end startup mode\n"));
  gv.startup_mode = 0;
  /* FIXME: MEMBAR needed for startup mode (or use a lock) */
  ephash_enum_writer_init (&est);
  while ((wr = ephash_enum_writer_next (&est)) != NULL)
    writer_exit_startup_mode (wr);
  ephash_enum_writer_fini (&est);
  delete_xevent (ev);
  return 0;
}

static os_size_t handle_xevk_delete_writer (UNUSED_ARG (struct nn_xpack *xp), struct xevent *ev, UNUSED_ARG (os_int64 tnow))
{
  /* don't worry if the writer is already gone by the time we get here. */
  TRACE (("handle_xevk_delete_writer: %x:%x:%x:%x\n", PGUID (ev->u.delete_writer.guid)));
  delete_writer_nolinger (&ev->u.delete_writer.guid);
  delete_xevent (ev);
  return 0;
}

static os_size_t handle_individual_xevent (struct xevent *xev, struct nn_xpack *xp, os_int64 tnow)
{
  os_size_t nbytes = 0;
  switch (xev->kind)
  {
    case XEVK_HEARTBEAT:
      nbytes = handle_xevk_heartbeat (xp, xev, tnow);
      break;
    case XEVK_ACKNACK:
      nbytes = handle_xevk_acknack (xp, xev, tnow);
      break;
    case XEVK_SPDP:
      nbytes = handle_xevk_spdp (xp, xev, tnow);
      break;
    case XEVK_PMD_UPDATE:
      nbytes = handle_xevk_pmd_update (xp, xev, tnow);
      break;
    case XEVK_END_STARTUP_MODE:
      nbytes = handle_xevk_end_startup_mode (xp, xev, tnow);
      break;
    case XEVK_DELETE_WRITER:
      nbytes = handle_xevk_delete_writer (xp, xev, tnow);
      break;
    case XEVK_CALLBACK:
      xev->u.callback.cb (xev, xev->u.callback.arg,tnow);
      break;
  }
  return nbytes;
}

static os_size_t handle_individual_xevent_nt (struct xevent_nt *xev, struct nn_xpack *xp, os_int64 tnow)
{
  os_size_t nbytes = 0;
  switch (xev->kind)
  {
    case XEVK_MSG:
      nbytes = handle_xevk_msg (xp, xev, tnow);
      break;
    case XEVK_MSG_REXMIT:
      nbytes = handle_xevk_msg_rexmit (xp, xev, tnow);
      break;
  }
  return nbytes;
}

static os_size_t handle_timed_xevent (struct thread_state1 *self, struct xevent *xev, struct nn_xpack *xp, os_int64 tnow)
{
   /* This function handles the individual xevent irrespective of
      whether it is a "timed" or "non-timed" xevent */
  os_size_t nbytes;
  struct xeventq *xevq = xev->evq;

  /* We relinquish the lock while processing the event, but require it
     held for administrative work. */
  ASSERT_MUTEX_HELD (&xevq->lock);

  assert (xev->evq == xevq);
  assert (xev->tsched != TSCHED_DELETE);

  os_mutexUnlock (&xevq->lock);
  thread_state_awake (self);
  nbytes = handle_individual_xevent (xev, xp, tnow);
  os_mutexLock (&xevq->lock);

  ASSERT_MUTEX_HELD (&xevq->lock);
  return nbytes;
}

static os_size_t handle_nontimed_xevent (struct thread_state1 *self, struct xevent_nt *xev, struct nn_xpack *xp, os_int64 tnow)
{
   /* This function handles the individual xevent irrespective of
      whether it is a "timed" or "non-timed" xevent */
  os_size_t nbytes;
  struct xeventq *xevq = xev->evq;

  /* We relinquish the lock while processing the event, but require it
     held for administrative work. */
  ASSERT_MUTEX_HELD (&xevq->lock);

  assert (xev->evq == xevq);

  os_mutexUnlock (&xevq->lock);
  thread_state_awake (self);
  nbytes = handle_individual_xevent_nt (xev, xp, tnow);
  /* non-timed xevents are freed by the handlers */
  os_mutexLock (&xevq->lock);

  ASSERT_MUTEX_HELD (&xevq->lock);
  return nbytes;
}

static os_size_t handle_xevents (struct thread_state1 *self, struct xeventq *xevq, struct nn_xpack *xp, os_int64 tnow)
{
  os_size_t nbytes = 0;
  int xeventsToProcess = 1;

  ASSERT_MUTEX_HELD (&xevq->lock);

  /* The following loops give priority to the "timed" events (heartbeats,
     acknacks etc) if there are any.  The algorithm is that we handle all
     "timed" events that are scheduled now and then handle one "non-timed"
     event.  If there weren't any "non-timed" events then the loop
     terminates.  If there was one, then after handling it, re-read the
     clock and continue the loop, i.e. test again to see whether any
     "timed" events are now due. */

  while (xeventsToProcess)
  {
    while (earliest_in_xeventq (xevq) <= tnow)
    {
      struct xevent *xev = fh_extractmin (&xevq->xevents);
      if (xev->tsched == TSCHED_DELETE)
      {
        free_xevent (xevq, xev);
        nbytes = 0;
      }
      else
      {
        /* event rescheduling functions look at xev->tsched to
           determine whether it is currently on the heap or not (i.e.,
           scheduled or not), so set to TSCHED_NEVER to indicate it
           currently isn't. */
        xev->tsched = T_NEVER;
        nbytes += handle_timed_xevent (self, xev, xp, tnow);
      }

      /* Limited-bandwidth channels means events can take a LONG time
         to process.  So read the clock more often. */
      tnow = now ();
    }

    if (!non_timed_xmit_list_is_empty(xevq))
    {
      struct xevent_nt *xev = getnext_from_non_timed_xmit_list (xevq);
      nbytes += handle_nontimed_xevent (self, xev, xp, tnow);
      tnow = now ();
    }
    else
    {
      xeventsToProcess = 0;
    }
  }

  ASSERT_MUTEX_HELD (&xevq->lock);
  return nbytes;
}

static void * xevent_thread (struct xeventq * xevq)
{
  struct thread_state1 *self = lookup_thread_state ();
  struct nn_xpack *xp;

  xp = nn_xpack_new (xevq->tev_conn);

  os_mutexLock (&xevq->lock);
  while (!xevq->terminate)
  {
    os_int64 tnow = now ();
    os_int64 twakeup;

    handle_xevents (self, xevq, xp, tnow);

    /* Send to the network unlocked, as it may sleep due to bandwidth
       limitation */
    os_mutexUnlock (&xevq->lock);
    nn_xpack_send (xp);
    os_mutexLock (&xevq->lock);

    thread_state_asleep (self);

    if (!non_timed_xmit_list_is_empty (xevq) || xevq->terminate)
    {
      /* continue immediately */
    }
    else if ((twakeup = earliest_in_xeventq (xevq)) == T_NEVER)
    {
      /* no scheduled events nor any non-timed events */
      os_condWait (&xevq->cond, &xevq->lock);
    }
    else
    {
      /* Although we assumed instantaneous handling of events, we
         don't want to sleep much longer than we have to. With
         os_condTimedWait requiring a relative time, we don't have
         much choice but to read the clock now */
      tnow = now ();
      if (twakeup > tnow)
      {
        os_time to;
        twakeup -= tnow; /* os_condTimedWait: relative timeout */
        to.tv_sec = (int) (twakeup / 1000000000);
        to.tv_nsec = (unsigned) (twakeup % 1000000000);
        os_condTimedWait (&xevq->cond, &xevq->lock, &to);
      }
    }
  }
  os_mutexUnlock (&xevq->lock);
  nn_xpack_send (xp);
  nn_xpack_free (xp);
  return NULL;
}

int qxev_msg (struct xeventq *evq, struct nn_xmsg *msg)
{
  struct xevent_nt *ev;
  assert (evq);
  assert (nn_xmsg_kind (msg) != NN_XMSG_KIND_DATA_REXMIT);
  os_mutexLock (&evq->lock);
  ev = qxev_common_nt (evq, XEVK_MSG);
  ev->u.msg.msg = msg;
  qxev_insert_nt (ev);
  os_mutexUnlock (&evq->lock);
  return 1;
}

int qxev_msg_rexmit_wrlock_held (struct xeventq *evq, struct nn_xmsg *msg, int force)
{
  unsigned msg_size = nn_xmsg_size (msg);
  struct xevent_nt *ev;

  assert (evq);
  assert (nn_xmsg_kind (msg) == NN_XMSG_KIND_DATA_REXMIT);
  os_mutexLock (&evq->lock);
  if ((ev = lookup_msg (evq, msg)) != NULL && nn_xmsg_merge_rexmit_destinations_wrlock_held (ev->u.msg_rexmit.msg, msg))
  {
    /* MSG got merged with a pending retransmit, so it has effectively been queued */
    os_mutexUnlock (&evq->lock);
    nn_xmsg_free (msg);
    return 1;
  }
  else if ((evq->queued_rexmit_bytes > evq->max_queued_rexmit_bytes ||
            evq->queued_rexmit_msgs == evq->max_queued_rexmit_msgs) &&
           !force)
  {
    /* drop it if insufficient resources available */
    os_mutexUnlock (&evq->lock);
    nn_xmsg_free (msg);
#if 0
    TRACE ((" qxev_msg_rexmit%s drop (sz %u qb %u qm %u)", force ? "!" : "",
            msg_size, evq->queued_rexmit_bytes, evq->queued_rexmit_msgs));
#endif
    return 0;
  }
  else
  {
    ev = qxev_common_nt (evq, XEVK_MSG_REXMIT);
    ev->u.msg_rexmit.msg = msg;
    ev->u.msg_rexmit.queued_rexmit_bytes = msg_size;
    evq->queued_rexmit_bytes += msg_size;
    evq->queued_rexmit_msgs++;
    qxev_insert_nt (ev);
#if 0
    TRACE (("AAA(%p,%u)", (void *) ev, msg_size));
#endif
    os_mutexUnlock (&evq->lock);
    return 1;
  }
}

struct xevent *qxev_heartbeat (struct xeventq *evq, os_int64 tsched, const nn_guid_t *wr_guid)
{
  /* Event _must_ be deleted before enough of the writer is freed to
     cause trouble.  Currently used exclusively for
     wr->heartbeat_xevent.  */
  struct xevent *ev;
  assert(evq);
  os_mutexLock (&evq->lock);
  ev = qxev_common (evq, tsched, XEVK_HEARTBEAT);
  ev->u.heartbeat.wr_guid = *wr_guid;
  qxev_insert (ev);
  os_mutexUnlock (&evq->lock);
  return ev;
}

struct xevent *qxev_acknack (struct xeventq *evq, os_int64 tsched, const nn_guid_t *pwr_guid, const nn_guid_t *rd_guid)
{
  struct xevent *ev;
  assert(evq);
  os_mutexLock (&evq->lock);
  ev = qxev_common (evq, tsched, XEVK_ACKNACK);
  ev->u.acknack.pwr_guid = *pwr_guid;
  ev->u.acknack.rd_guid = *rd_guid;
  qxev_insert (ev);
  os_mutexUnlock (&evq->lock);
  return ev;
}

struct xevent *qxev_spdp (os_int64 tsched, const nn_guid_t *pp_guid, const nn_guid_t *dest_proxypp_guid)
{
  struct xevent *ev;
  os_mutexLock (&gv.xevents->lock);
  ev = qxev_common (gv.xevents, tsched, XEVK_SPDP);
  ev->u.spdp.pp_guid = *pp_guid;
  if (dest_proxypp_guid == NULL)
    ev->u.spdp.directed = 0;
  else
  {
    ev->u.spdp.dest_proxypp_guid_prefix = dest_proxypp_guid->prefix;
    ev->u.spdp.directed = 1;
  }
  qxev_insert (ev);
  os_mutexUnlock (&gv.xevents->lock);
  return ev;
}

struct xevent *qxev_pmd_update (os_int64 tsched, const nn_guid_t *pp_guid)
{
  struct xevent *ev;
  os_mutexLock (&gv.xevents->lock);
  ev = qxev_common (gv.xevents, tsched, XEVK_PMD_UPDATE);
  ev->u.pmd_update.pp_guid = *pp_guid;
  qxev_insert (ev);
  os_mutexUnlock (&gv.xevents->lock);
  return ev;
}

struct xevent *qxev_end_startup_mode (os_int64 tsched)
{
  struct xevent *ev;
  os_mutexLock (&gv.xevents->lock);
  ev = qxev_common (gv.xevents, tsched, XEVK_END_STARTUP_MODE);
  qxev_insert (ev);
  os_mutexUnlock (&gv.xevents->lock);
  return ev;
}

struct xevent *qxev_delete_writer (os_int64 tsched, const nn_guid_t *guid)
{
  struct xevent *ev;
  os_mutexLock (&gv.xevents->lock);
  ev = qxev_common (gv.xevents, tsched, XEVK_DELETE_WRITER);
  ev->u.delete_writer.guid = *guid;
  qxev_insert (ev);
  os_mutexUnlock (&gv.xevents->lock);
  return ev;
}

struct xevent *qxev_callback (os_int64 tsched, void (*cb) (struct xevent *ev, void *arg, os_int64 tnow), void *arg)
{
  struct xevent *ev;
  os_mutexLock (&gv.xevents->lock);
  ev = qxev_common (gv.xevents, tsched, XEVK_CALLBACK);
  ev->u.callback.cb = cb;
  ev->u.callback.arg = arg;
  qxev_insert (ev);
  os_mutexUnlock (&gv.xevents->lock);
  return ev;
}

/* SHA1 not available (unoffical build.) */
