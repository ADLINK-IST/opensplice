#include <assert.h>
#include <stdlib.h>
#include <stddef.h>

#include "os_heap.h"
#include "os_cond.h"
#include "os_mutex.h"
#include "os_thread.h"

#include "q_gc.h"
#include "q_log.h"
#include "q_config.h"
#include "q_time.h"
#include "q_thread.h"
#include "q_ephash.h"
#include "q_unused.h"
#include "q_lease.h"
#include "q_globals.h" /* for mattr, cattr */

#include "q_rtps.h" /* for guid_hash */

struct gcreq_queue {
  struct gcreq *first;
  struct gcreq *last;
  os_mutex lock;
  os_cond cond;
  int terminate;
  os_int32 count;
  struct thread_state1 *ts;
};

static void threads_vtime_gather_for_wait (int *nivs, struct idx_vtime *ivs)
{
  /* copy vtimes of threads, skipping those that are sleeping */
  int i, j;
  for (i = j = 0; i < thread_states.nthreads; i++)
  {
    vtime_t vtime = thread_states.ts[i].vtime;
    if (vtime_awake_p (vtime))
    {
      ivs[j].idx = i;
      ivs[j].vtime = vtime;
      ++j;
    }
  }
  *nivs = j;
}

static int threads_vtime_check (int *nivs, struct idx_vtime *ivs)
{
  /* check all threads in ts have made progress those that have are
     removed from the set */
  int i = 0;
  while (i < *nivs)
  {
    int thridx = ivs[i].idx;
    vtime_t vtime = thread_states.ts[thridx].vtime;
    assert (vtime_awake_p (ivs[i].vtime));
    if (!vtime_gt (vtime, ivs[i].vtime))
      ++i;
    else
    {
      if (i + 1 < *nivs)
        ivs[i] = ivs[*nivs - 1];
      --(*nivs);
    }
  }
  return *nivs == 0;
}

static void *gcreq_queue_thread (struct gcreq_queue *q)
{
  struct thread_state1 *self = lookup_thread_state ();
  struct os_time to = { 0, 100 * T_MILLISECOND };
  struct os_time shortsleep = { 0, 1 * T_MILLISECOND };
  struct gcreq *gcreq = NULL;
  os_mutexLock (&q->lock);
  while (!(q->terminate && q->count == 0))
  {
    /* If we are waiting for a gcreq to become ready, don't bother
       looking at the queue; if we aren't, wait for a request to come
       in.  We can't really wait until something came in because we're
       also checking lease expirations. */
    if (gcreq == NULL)
    {
      if (q->first == NULL)
        os_condTimedWait (&q->cond, &q->lock, &to);
      if (q->first)
      {
        gcreq = q->first;
        q->first = q->first->next;
      }
    }
    os_mutexUnlock (&q->lock);

    /* Cleanup dead proxy entities. One can argue this should be an
       independent thread, but one can also easily argue that an
       expired lease is just another form of a request for
       deletion. In any event, letting this thread do this should have
       very little impact on its primary purpose and be less of a
       burden on the system than having a separate thread or adding it
       to the workload of the data handling threads. */
    thread_state_awake (self);
    check_and_handle_lease_expiration (self, now ());
    thread_state_asleep (self);

    if (gcreq)
    {
      if (!threads_vtime_check (&gcreq->nvtimes, gcreq->vtimes))
      {
        /* Not all threads made enough progress => gcreq is not ready
           yet => sleep for a bit and rety.  Note that we can't even
           terminate while this gcreq is waiting and that there is no
           condition on which to wait, so a plain sleep is quite
           reasonable. */
        TRACE (("gc %p: not yet, shortsleep\n", gcreq));
        os_nanoSleep (shortsleep);
      }
      else
      {
        /* Sufficent progress has been made: may now continue deleting
           it; the callback is responsible for requeueing (if complex
           multi-phase delete) or freeing the delete request.  Reset
           the current gcreq as this one obviously is no more.  */
        TRACE (("gc %p: deleting\n", gcreq));
        thread_state_awake (self);
        gcreq->cb (gcreq);
        thread_state_asleep (self);
        gcreq = NULL;
      }
    }

    os_mutexLock (&q->lock);
  }
  os_mutexUnlock (&q->lock);
  return NULL;
}

struct gcreq_queue *gcreq_queue_new (void)
{
  struct gcreq_queue *q;
  if ((q = os_malloc (sizeof (*q))) == NULL)
    goto fail_q;
  q->first = q->last = NULL;
  q->terminate = 0;
  q->count = 0;
  if (os_mutexInit (&q->lock, &gv.mattr) != os_resultSuccess)
    goto fail_lock;
  if (os_condInit (&q->cond, &q->lock, &gv.cattr) != os_resultSuccess)
    goto fail_cond;
  q->ts = create_thread ("gc", (void * (*) (void *)) gcreq_queue_thread, q);
  if (q->ts == NULL)
    goto fail_thread;
  return q;
 fail_thread:
  os_condDestroy (&q->cond);
 fail_cond:
  os_mutexDestroy (&q->lock);
 fail_lock:
  os_free (q);
 fail_q:
  return NULL;
}

void gcreq_queue_free (struct gcreq_queue *q)
{
  struct gcreq *gcreq;

  /* Create a no-op not dependent on any thread */
  gcreq = gcreq_new (q, gcreq_free);
  gcreq->nvtimes = 0;

  os_mutexLock (&q->lock);
  q->terminate = 1;
  /* Wait until there is only request in existence, the one we just
     allocated. Then we know the gc system is quiet. */
  while (q->count != 1)
    os_condWait (&q->cond, &q->lock);
  os_mutexUnlock (&q->lock);

  /* Force the gc thread to wake up by enqueueing our no-op. The
     callback, gcreq_free, will be called immediately, which causes
     q->count to 0 before the loop condition is evaluated again, at
     which point the thread terminates. */
  gcreq_enqueue (gcreq);

  join_thread (q->ts, (void **) 0);
  assert (q->first == NULL);
  os_condDestroy (&q->cond);
  os_mutexDestroy (&q->lock);
  os_free (q);
}

struct gcreq *gcreq_new (struct gcreq_queue *q, gcreq_cb_t cb)
{
  struct gcreq *gcreq;
  if ((gcreq = os_malloc (offsetof (struct gcreq, vtimes) + thread_states.nthreads * sizeof (*gcreq->vtimes))) == NULL)
  {
    /* Now what!? Might be an option to pre-allocate a few gcreqs for
       use when out of memory, and block gcreq_new requests until one
       becomes available. Whether that actually solves the problem or
       simply moves it, I'm not entirely certain. */
    abort ();
  }
  gcreq->cb = cb;
  gcreq->queue = q;
  threads_vtime_gather_for_wait (&gcreq->nvtimes, gcreq->vtimes);
  os_mutexLock (&q->lock);
  q->count++;
  os_mutexUnlock (&q->lock);
  return gcreq;
}

void gcreq_free (struct gcreq *gcreq)
{
  struct gcreq_queue *gcreq_queue = gcreq->queue;
  os_mutexLock (&gcreq_queue->lock);
  --gcreq_queue->count;
  if (gcreq_queue->terminate && gcreq_queue->count <= 1)
    os_condBroadcast (&gcreq_queue->cond);
  os_mutexUnlock (&gcreq_queue->lock);
  os_free (gcreq);
}

static int gcreq_enqueue_common (struct gcreq *gcreq)
{
  struct gcreq_queue *gcreq_queue = gcreq->queue;
  int isfirst;
  os_mutexLock (&gcreq_queue->lock);
  gcreq->next = NULL;
  if (gcreq_queue->first)
  {
    gcreq_queue->last->next = gcreq;
    isfirst = 0;
  }
  else
  {
    gcreq_queue->first = gcreq;
    isfirst = 1;
  }
  gcreq_queue->last = gcreq;
  if (isfirst)
    os_condBroadcast (&gcreq_queue->cond);
  os_mutexUnlock (&gcreq_queue->lock);
  return isfirst;
}

void gcreq_enqueue (struct gcreq *gcreq)
{
  gcreq_enqueue_common (gcreq);
}

int gcreq_requeue (struct gcreq *gcreq, gcreq_cb_t cb)
{
  gcreq->cb = cb;
  return gcreq_enqueue_common (gcreq);
}

/* SHA1 not available (unoffical build.) */
