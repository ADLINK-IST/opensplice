#include <assert.h>

#include "os_if.h"
#include "os_defs.h"
#include "os_heap.h"
#include "os_mutex.h"
#include "os_cond.h"
#include "os_thread.h"

#include "u_user.h"
#include "u_participant.h"

#include "nn_servicelease.h"
#include "nn_config.h"
#include "nn_log.h"

static void nn_retrieve_lease_settings (v_duration *leaseExpiryTime, os_time *sleepTime)
{
  const c_float leaseSec = config.servicelease_expiry_time;
  c_float sleepSec = leaseSec * config.servicelease_update_factor;
  sleepTime->tv_sec = (os_int32) sleepSec;
  sleepTime->tv_nsec = (os_int32) ((sleepSec - (float) sleepTime->tv_sec) * 1000000000.0f);
  leaseExpiryTime->seconds = (os_int32) leaseSec;
  leaseExpiryTime->nanoseconds = (os_int32) ((leaseSec - (float) leaseExpiryTime->seconds) * 1000000000.0f);
}

/* FIXME: lease updating should not be completely independent thread,
   rather it should require liveliness assertions from each individual
   thread, so that any thread getting blocked for too long causes the
   process' lease to expire.

   But there more important matters exist still, so we go the easy way
   first. */

struct nn_servicelease {
  v_duration leasePeriod;
  os_time sleepTime;
  int keepgoing;
  u_participant participant;

  os_mutex lock;
  os_cond cond;
  os_threadId tid;
};

static void *lease_renewal_thread (struct nn_servicelease *sl)
{
  os_result waitResult;
  os_mutexLock (&sl->lock);
  while (sl->keepgoing)
  {
    u_serviceRenewLease (u_service(sl->participant), sl->leasePeriod);
    waitResult = os_condTimedWait (&sl->cond, &sl->lock, &sl->sleepTime);
    if (waitResult == os_resultFail)
    {
        nn_log (LC_FATAL, "lease_renewal_thread - os_condTimedWait failed - thread will terminate");
        break;
    }
  }
  os_mutexUnlock (&sl->lock);
  return NULL;
}

struct nn_servicelease *nn_servicelease_new (u_participant participant)
{
  struct nn_servicelease *sl;
  os_mutexAttr mattr;
  os_condAttr cattr;

  if ((sl = os_malloc (sizeof (*sl))) == NULL)
    goto fail_0;
  nn_retrieve_lease_settings (&sl->leasePeriod, &sl->sleepTime);
  sl->keepgoing = -1;
  sl->participant = participant;

  os_mutexAttrInit (&mattr);
  mattr.scopeAttr = OS_SCOPE_PRIVATE;
  if (os_mutexInit (&sl->lock, &mattr) != os_resultSuccess)
    goto fail_lock;

  os_condAttrInit (&cattr);
  cattr.scopeAttr = OS_SCOPE_PRIVATE;
  if (os_condInit (&sl->cond, &sl->lock, &cattr) != os_resultSuccess)
    goto fail_cond;
  return sl;

 fail_cond:
  os_mutexDestroy (&sl->lock);
 fail_lock:
  os_free (sl);
 fail_0:
  return NULL;
}

int nn_servicelease_start_renewing (struct nn_servicelease *sl)
{
  os_threadAttr tattr;

  os_mutexLock (&sl->lock);
  assert (sl->keepgoing == -1);
  sl->keepgoing = 1;
  os_mutexUnlock (&sl->lock);

  os_threadAttrInit (&tattr);
  if (os_threadCreate (&sl->tid, "servicelease", &tattr,
                       (void * (*) (void *)) lease_renewal_thread, sl) !=
      os_resultSuccess)
    goto fail_thread;
  return 0;

 fail_thread:
  sl->keepgoing = -1;
  return -1;
}

void nn_servicelease_free (struct nn_servicelease *sl)
{
  if (sl->keepgoing != -1)
  {
    os_mutexLock (&sl->lock);
    sl->keepgoing = 0;
    os_condSignal (&sl->cond);
    os_mutexUnlock (&sl->lock);
    os_threadWaitExit (sl->tid, (void **) 0);
  }
  os_condDestroy (&sl->cond);
  os_mutexDestroy (&sl->lock);
  os_free (sl);
}
