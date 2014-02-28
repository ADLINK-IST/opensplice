#include <assert.h>

#include "os_heap.h"
#include "os_stdlib.h"
#include "os_thread.h"

#include "q_thread.h"
#include "q_servicelease.h"
#include "q_error.h"
#include "q_log.h"
#include "q_config.h"
#include "q_globals.h"
#include "sysdeps.h"

static char main_thread_name[] = "main";

struct thread_states thread_states;
#if OS_HAS_TSD_USING_THREAD_KEYWORD
__thread struct thread_state1 *tsd_thread_state;
#endif

static void *os_malloc_aligned_cacheline (os_size_t size)
{
  /* This wastes some space, but we use it only once and it isn't a
     huge amount of memory, just a little over a cache line.
     Alternatively, we good use valloc() and have it aligned to a page
     boundary, but that one isn't part of the O/S abstraction layer
     ... */
  const os_address clm1 = CACHE_LINE_SIZE - 1;
  os_address ptrA;
  void **pptr;
  void *ptr;
  if ((ptr = os_malloc (size + CACHE_LINE_SIZE + sizeof (void *))) == NULL)
    return NULL;
  ptrA = ((os_address) ptr + sizeof (void *) + clm1) & ~clm1;
  pptr = (void **) ptrA;
  pptr[-1] = ptr;
  return (void *) ptrA;
}

static void os_free_aligned (void *ptr)
{
  void **pptr = ptr;
  os_free (pptr[-1]);
}

int thread_states_init (int maxthreads)
{
  int i;

  if (os_mutexInit (&thread_states.lock, &gv.mattr) != os_resultSuccess)
    goto err_lock;
  thread_states.nthreads = maxthreads;
  if ((thread_states.ts =
       os_malloc_aligned_cacheline (maxthreads * sizeof (*thread_states.ts))) == NULL)
    goto err_tstates;
  memset (thread_states.ts, 0, maxthreads * sizeof (*thread_states.ts));
  for (i = 0; i < thread_states.nthreads; i++)
  {
    thread_states.ts[i].state = THREAD_STATE_ZERO;
    thread_states.ts[i].vtime = 1;
    thread_states.ts[i].watchdog = 1;
    thread_states.ts[i].lb = NULL;
    thread_states.ts[i].name = NULL;
  }
  return 0;
 err_tstates:
  os_mutexDestroy (&thread_states.lock);
 err_lock:
  NN_FATAL0 ("thread_states_init: failed to initialize thread state table\n");
  return ERR_UNSPECIFIED;
}

void thread_states_fini (void)
{
  int i;
  for (i = 0; i < thread_states.nthreads; i++)
    assert (thread_states.ts[i].state != THREAD_STATE_ALIVE);
  os_mutexDestroy (&thread_states.lock);
  os_free_aligned (thread_states.ts);

  /* All spawned threads are gone, but the main thread is still alive,
     downgraded to an ordinary thread (we're on it right now). We
     don't want to lose the ability to log messages, so set ts to a
     NULL pointer and rely on lookup_thread_state()'s checks
     thread_states.ts. */
  thread_states.ts = NULL;
}

struct thread_state1 *lookup_thread_state_real (void)
{
  if (thread_states.ts)
  {
    os_threadId self = os_threadIdSelf ();
    int i;
    for (i = 0; i < thread_states.nthreads; i++)
      if (os_threadEqual (thread_states.ts[i].tid, self))
        return &thread_states.ts[i];
  }
  return NULL;
}

struct thread_context {
  struct thread_state1 *self;
  void * (*f) (void *arg);
  void *arg;
};

static void *create_thread_wrapper (struct thread_context *ctxt)
{
  void *ret;
  ctxt->self->tid = os_threadIdSelf ();
  ret = ctxt->f (ctxt->arg);
  logbuf_free (ctxt->self->lb);
  os_free (ctxt);
  return ret;
}

static int find_free_slot (const char *name)
{
  int i, cand;
  for (i = 0, cand = -1; i < thread_states.nthreads; i++)
  {
    if (thread_states.ts[i].state != THREAD_STATE_ALIVE)
      cand = i;
    if (thread_states.ts[i].state == THREAD_STATE_ZERO)
      break;
  }
  if (cand == -1)
    NN_FATAL1 ("create_thread: %s: no free slot\n", name ? name : "(anon)");
  return cand;
}

void upgrade_main_thread (void)
{
  int cand;
  struct thread_state1 *ts1;
  os_mutexLock (&thread_states.lock);
  if ((cand = find_free_slot ("name")) < 0)
    abort ();
  ts1 = &thread_states.ts[cand];
  if (ts1->state == THREAD_STATE_ZERO)
    assert (vtime_asleep_p (ts1->vtime));
  ts1->state = THREAD_STATE_ALIVE;
  ts1->tid = os_threadIdSelf ();
  ts1->lb = logbuf_new ();
  ts1->name = main_thread_name;
  os_mutexUnlock (&thread_states.lock);
}

const struct config_thread_properties_listelem *lookup_thread_properties (const char *name)
{
  const struct config_thread_properties_listelem *e;
  for (e = config.thread_properties; e != NULL; e = e->next)
    if (strcmp (e->name, name) == 0)
      break;
  return e;
}

struct thread_state1 *create_thread (const char *name, void * (*f) (void *arg), void *arg)
{
  struct config_thread_properties_listelem const * const tprops = lookup_thread_properties (name);
  int cand;
  os_threadAttr tattr;
  struct thread_state1 *ts1;
  os_threadId tid;
  struct thread_context *ctxt;
  if ((ctxt = os_malloc (sizeof (*ctxt))) == NULL)
    return NULL;
  os_mutexLock (&thread_states.lock);
  if ((cand = find_free_slot (name)) < 0)
    goto fatal;
  ts1 = &thread_states.ts[cand];
  if (ts1->state == THREAD_STATE_ZERO)
    assert (vtime_asleep_p (ts1->vtime));
  if ((ts1->name = os_strdup (name)) == NULL)
  {
    NN_FATAL1 ("create_thread: %s: out of memory\n", name);
    goto fatal;
  }
  ts1->lb = logbuf_new ();
  ts1->state = THREAD_STATE_ALIVE;
  ctxt->self = ts1;
  ctxt->f = f;
  ctxt->arg = arg;
  os_threadAttrInit (&tattr);
  if (tprops != NULL)
  {
    if (!tprops->sched_priority.isdefault)
      tattr.schedPriority = tprops->sched_priority.value;
    tattr.schedClass = tprops->sched_class; /* explicit default value in the enum */
    if (!tprops->stack_size.isdefault)
      tattr.stackSize = (os_uint32) tprops->stack_size.value;
  }
  TRACE (("create_thread: %s: class %d priority %d stack %u\n", name, (int) tattr.schedClass, tattr.schedPriority, tattr.stackSize));

  if (os_threadCreate (&tid, name, &tattr, (void * (*) (void *)) create_thread_wrapper, ctxt) != os_resultSuccess)
  {
    ts1->state = THREAD_STATE_ZERO;
    NN_FATAL1 ("create_thread: %s: os_threadCreate failed\n", name);
    goto fatal;
  }
  nn_log (LC_INFO, "started new thread 0x%llx : %s\n", (unsigned long long) os_threadIdToInteger (tid), name);
  ts1->extTid = tid; /* overwrite the temporary value with the correct external one */
  os_mutexUnlock (&thread_states.lock);
  return ts1;
 fatal:
  os_mutexUnlock (&thread_states.lock);
  os_free (ctxt);
  abort ();
  return NULL;
}

static void reap_thread_state (struct thread_state1 *ts1, int sync_with_servicelease)
{
  os_mutexLock (&thread_states.lock);
  ts1->state = THREAD_STATE_ZERO;
  if (sync_with_servicelease)
    nn_servicelease_statechange_barrier (gv.servicelease);
  if (ts1->name != main_thread_name)
    os_free (ts1->name);
  os_mutexUnlock (&thread_states.lock);
}

int join_thread (struct thread_state1 *ts1, void **retval)
{
  int ret;
  assert (ts1->state == THREAD_STATE_ALIVE);
  if (os_threadWaitExit (ts1->extTid, retval) == 0)
    ret = 0;
  else
    ret = ERR_UNSPECIFIED;
  assert (vtime_asleep_p (ts1->vtime));
  reap_thread_state (ts1, 1);
  return ret;
}

void downgrade_main_thread (void)
{
  struct thread_state1 *ts1 = lookup_thread_state ();
  thread_state_asleep (ts1);
  logbuf_free (ts1->lb);
  /* no need to sync with service lease: already stopped */
  reap_thread_state (ts1, 0);
}

/* SHA1 not available (unoffical build.) */
