#include <stdlib.h>

#include "os_heap.h"
#include "os_cond.h"

#include "nn_avl.h"
#include "nn_time.h"
#include "nn_log.h"
#include "nn_addrset.h"
#include "nn_xmsg.h"
#include "nn_rtps_private.h"
#include "nn_xevent.h"
#include "nn_config.h"

#include "sysdeps.h"

static int compare_time (const void *va, const void *vb)
{
  const os_int64 *a = va;
  const os_int64 *b = vb;
  return (*a == *b) ? 0 : (*a < *b) ? -1 : 1;
}

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

void delete_xevent (struct xevent *ev)
{
  struct xeventq *evq = ev->evq;
  avl_delete (&evq->xevents, ev);
  switch (ev->kind)
  {
    case XEVK_MSG:
    case XEVK_HEARTBEAT:
    case XEVK_ACKNACK:
    case XEVK_SPDP:
    case XEVK_PMD_UPDATE:
    case XEVK_INFO:
    case XEVK_END_STARTUP_MODE:
      break;
  }
  os_free (ev);
  xeventq_adjust_throttle (evq);
}

void delete_xevent_free_deps (struct xevent *ev)
{
  struct xeventq *evq = ev->evq;
  avl_delete (&evq->xevents, ev);
  switch (ev->kind)
  {
    case XEVK_MSG:
      nn_xmsg_free (ev->u.msg.msg);
      break;
    case XEVK_HEARTBEAT:
    case XEVK_ACKNACK:
    case XEVK_SPDP:
    case XEVK_PMD_UPDATE:
    case XEVK_INFO:
    case XEVK_END_STARTUP_MODE:
      break;
  }
  os_free (ev);
  xeventq_adjust_throttle (evq);
}

void resched_xevent (struct xevent *ev, os_int64 tsched)
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

int resched_xevent_if_earlier (struct xevent *ev, os_int64 tsched)
{
  if (tsched < ev->tsched)
  {
    resched_xevent (ev, tsched);
    return 1;
  }
  else
  {
    return 0;
  }
}

static struct xevent *qxev_common (struct xeventq *evq, os_int64 tsched, enum xeventkind kind)
{
  struct xevent *ev = os_malloc (sizeof (*ev));
  avlparent_t parent;
  if (config.xmit_out_of_order && config.xmit_out_of_order > 1)
  {
    /* We're actually making it even worse than just transmitting out
       of order: we add jitter to everything ... but a few nanoseconds
       won't be noticeable and therefore the effect will be mostly
       confined to the events scheduled at near t = 0. */
    tsched += random () % config.xmit_out_of_order;
  }
  avl_lookup (&evq->xevents, &tsched, &parent);
  avl_init_node (&ev->avlnode, parent);
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

struct xevent *qxev_msg (struct xeventq *evq, os_int64 tsched, struct nn_xmsg *msg)
{
  struct xevent *ev;
  ev = qxev_common (evq, tsched, XEVK_MSG);
  ev->u.msg.msg = msg;
  qxev_insert (ev);
  return ev;
}

struct xevent *qxev_heartbeat (struct xeventq *evq, os_int64 tsched, struct writer *wr)
{
  struct xevent *ev;
  ev = qxev_common (evq, tsched, XEVK_HEARTBEAT);
  ev->u.heartbeat.wr = wr;
  qxev_insert (ev);
  return ev;
}

struct xevent *qxev_acknack (struct xeventq *evq, os_int64 tsched, struct rhc_writers_node *rwn)
{
  struct xevent *ev;
  ev = qxev_common (evq, tsched, XEVK_ACKNACK);
  ev->u.acknack.rwn = rwn;
  qxev_insert (ev);
  return ev;
}

struct xevent *qxev_spdp (struct xeventq *evq, os_int64 tsched, struct participant *pp, struct proxy_participant *proxypp)
{
  struct xevent *ev;
  ev = qxev_common (evq, tsched, XEVK_SPDP);
  ev->u.spdp.pp = pp;
  ev->u.spdp.dest = proxypp;
  qxev_insert (ev);
  return ev;
}

struct xevent *qxev_pmd_update (struct xeventq *evq, os_int64 tsched, struct participant *pp)
{
  struct xevent *ev;
  ev = qxev_common (evq, tsched, XEVK_PMD_UPDATE);
  ev->u.pmd_update.pp = pp;
  qxev_insert (ev);
  return ev;
}

struct xevent *qxev_info (struct xeventq *evq, os_int64 tsched)
{
  struct xevent *ev;
  ev = qxev_common (evq, tsched, XEVK_INFO);
  qxev_insert (ev);
  return ev;
}

struct xevent *qxev_end_startup_mode (struct xeventq *evq, os_int64 tsched)
{
  struct xevent *ev;
  ev = qxev_common (evq, tsched, XEVK_END_STARTUP_MODE);
  qxev_insert (ev);
  return ev;
}

struct xeventq *new_xeventq (void)
{
  struct xeventq *evq = os_malloc (sizeof (*evq));
  avl_init (&evq->xevents, offsetof (struct xevent, avlnode), offsetof (struct xevent, tsched), compare_time, augment_xevent, 0);
  evq->oldsize = 0;
  return evq;
}

void free_xeventq (struct xeventq *evq)
{
  while (!avl_empty (&evq->xevents))
    delete_xevent_free_deps (evq->xevents.root);
  os_free (evq);
}

int xeventq_size (const struct xeventq *evq)
{
  if (avl_empty (&evq->xevents))
    return 0;
  else
    return evq->xevents.root->size;
}

int xeventq_must_throttle (struct xeventq *evq)
{
  return xeventq_size (evq) > config.xevq_highwater_mark;
}

int xeventq_may_continue (struct xeventq *evq)
{
  return xeventq_size (evq) < config.xevq_lowwater_mark;
}

void xeventq_adjust_throttle (struct xeventq *evq)
{
  /* A *very* simplistic bang-bang regulator */
  int oldsize = evq->oldsize;
  int newsize = xeventq_size (evq);
  evq->oldsize = newsize;
  if (newsize < config.xevq_lowwater_mark && oldsize >= config.xevq_lowwater_mark)
    os_condBroadcast (&throttle_cond);
}

os_int64 earliest_in_xeventq (struct xeventq *evq)
{
  if (avl_empty (&evq->xevents))
    return T_NEVER;
  else
    return evq->xevents.root->min_tsched;
}

struct xevent *next_from_xeventq (struct xeventq *evq)
{
  return avl_findmin (&evq->xevents);
}

struct xevent *eventq_peeksucc (struct xevent *ev)
{
  return avl_findsucc (&ev->evq->xevents, ev);
}

