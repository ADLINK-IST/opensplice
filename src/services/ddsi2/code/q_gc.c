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

static void threads_vtime_gather_for_wait (unsigned *nivs, struct idx_vtime *ivs)
{
  /* copy vtimes of threads, skipping those that are sleeping */
  unsigned i, j;
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

static int threads_vtime_check (unsigned *nivs, struct idx_vtime *ivs)
{
  /* check all threads in ts have made progress those that have are
     removed from the set */
  unsigned i = 0;
  while (i < *nivs)
  {
    unsigned thridx = ivs[i].idx;
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
  struct gcreq *gcreq = NULL;
  int trace_shortsleep = 1;
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
        os_condTimedWait (&q->cond, &q->lock, 100*OS_DURATION_MILLISECOND);
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
    check_and_handle_lease_expiration (self, now_et ());
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
        if (trace_shortsleep)
        {
          TRACE (("gc %p: not yet, shortsleep\n", (void*)gcreq));
          trace_shortsleep = 0;
        }
        os_sleep(1*OS_DURATION_MILLISECOND);
      }
      else
      {
        /* Sufficent progress has been made: may now continue deleting
           it; the callback is responsible for requeueing (if complex
           multi-phase delete) or freeing the delete request.  Reset
           the current gcreq as this one obviously is no more.  */
        TRACE (("gc %p: deleting\n", (void*)gcreq));
        thread_state_awake (self);
        gcreq->cb (gcreq);
        thread_state_asleep (self);
        gcreq = NULL;
        trace_shortsleep = 1;
      }
    }

    os_mutexLock (&q->lock);
  }
  os_mutexUnlock (&q->lock);
  return NULL;
}

struct gcreq_queue *gcreq_queue_new (void)
{
  struct gcreq_queue *q = os_malloc (sizeof (*q));

  q->first = q->last = NULL;
  q->terminate = 0;
  q->count = 0;
  os_mutexInit (&q->lock, NULL);
  os_condInit (&q->cond, &q->lock, NULL);
  q->ts = create_thread ("gc", (void * (*) (void *)) gcreq_queue_thread, q);
  assert (q->ts);
  return q;
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
  gcreq = os_malloc (offsetof (struct gcreq, vtimes) + thread_states.nthreads * sizeof (*gcreq->vtimes));
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
