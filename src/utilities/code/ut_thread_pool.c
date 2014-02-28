/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#include "ut_thread_pool.h"
#include "os_heap.h"
#include "os_mutex.h"
#include "os_semaphore.h"
#include "os_time.h"

typedef struct ddsi_work_queue_job
{
  struct ddsi_work_queue_job * m_next_job; /* Jobs list pointer */
  os_threadRoutine m_fn;                   /* Thread function */
  void * m_arg;                            /* Thread function argument */
}
* ddsi_work_queue_job_t;

OS_STRUCT(ut_thread_pool)
{
  ddsi_work_queue_job_t m_jobs;      /* Job queue */
  ddsi_work_queue_job_t m_jobs_tail; /* Tail of job queue */
  ddsi_work_queue_job_t m_free;      /* Job free list */
  os_uint32 m_thread_max;            /* Maximum number of threads */
  os_uint32 m_thread_min;            /* Minimum number of threads */
  os_uint32 m_threads;               /* Current number of threads */
  os_uint32 m_waiting;               /* Number of threads waiting for a job */
  os_uint32 m_job_count;             /* Number of queued jobs */
  os_uint32 m_job_max;               /* Maximum number of jobs to queue */
  unsigned short m_count;            /* Counter for thread name */
  os_threadAttr m_attr;              /* Thread creation attribute */
  os_sem_t m_sem;                    /* Thread wait semaphore */
  os_mutex m_mutex;                  /* Pool guard mutex */
};

static void * ut_thread_start_fn (void * arg)
{
  ddsi_work_queue_job_t job;
  ut_thread_pool pool = (ut_thread_pool) arg;

  /* Thread loops, pulling jobs from queue */

  while (TRUE)
  {
    /* Wait for job */

    os_sem_wait (&pool->m_sem);
    os_mutexLock (&pool->m_mutex);

    /* Check if pool deleted or being purged */

    if (pool->m_jobs == NULL)
    {
      pool->m_threads--;
      os_mutexUnlock (&pool->m_mutex);
      break;
    }

    /* Take job from queue head */

    pool->m_waiting--;
    job = pool->m_jobs;
    pool->m_jobs = job->m_next_job;
    pool->m_job_count--;

    os_mutexUnlock (&pool->m_mutex);

    /* Do job */

    (job->m_fn) (job->m_arg);

    /* Put job back on free list */

    os_mutexLock (&pool->m_mutex);
    pool->m_waiting++;
    job->m_next_job = pool->m_free;
    pool->m_free = job;
    os_mutexUnlock (&pool->m_mutex);
  }

  return NULL;
}

static os_result ut_thread_pool_new_thread (ut_thread_pool pool)
{
  static unsigned char pools = 0; /* Pool counter - TODO make atomic */

  char name [16];
  os_threadId id;
  os_result res;

  sprintf (name, "OSPL-%u-%u", pools++, pool->m_count++);
  res = os_threadCreate (&id, name, &pool->m_attr, ut_thread_start_fn, pool);

  if (res == os_resultSuccess)
  {
    pool->m_threads++;
    pool->m_waiting++;
  }

  return res;
}

ut_thread_pool ut_thread_pool_new 
(
  os_uint32 threads,
  os_uint32 max_threads,
  os_uint32 max_queue,
  os_threadAttr * attr
)
{
  ut_thread_pool pool;
  ddsi_work_queue_job_t job;
  os_mutexAttr mattr;

  /* Sanity check QoS */

  if (max_threads && (max_threads < threads))
  {
    max_threads = threads;
  }
  if (max_queue && (max_queue < threads))
  {
    max_queue = threads;
  }

  pool = os_malloc (sizeof (*pool));
  memset (pool, 0, sizeof (*pool));
  pool->m_thread_min = threads;
  pool->m_thread_max = max_threads;
  pool->m_job_max = max_queue;
  os_sem_init (&pool->m_sem, 0);
  os_threadAttrInit (&pool->m_attr);
  os_mutexAttrInit (&mattr);
  os_mutexInit (&pool->m_mutex, &mattr);

  if (attr)
  {
    pool->m_attr = *attr;
  }

  /* Create initial threads and jobs */

  while (threads--)
  {
    if (ut_thread_pool_new_thread (pool) != os_resultSuccess)
    {
      ut_thread_pool_free (pool);
      pool = NULL;
      break;
    }
    job = os_malloc (sizeof (*job));
    job->m_next_job = pool->m_free;
    pool->m_free = job;
  }

  return pool;
}

void ut_thread_pool_free (ut_thread_pool pool)
{
  static const os_time delay = { 0, 250000000 }; /* 250 ms */

  os_uint32 retry = 8;
  os_uint32 threads;
  ddsi_work_queue_job_t job;

  if (pool == NULL)
  {
    return;
  }

  os_mutexLock (&pool->m_mutex);

  /* Delete all pending jobs from queue */

  while (pool->m_jobs)
  {
    job = pool->m_jobs;
    pool->m_jobs = job->m_next_job;
    os_free (job);
  }

  /* Wake all waiting threads */

  threads = pool->m_threads;
  while (threads)
  {
    threads--;
    os_sem_post (&pool->m_sem);
  }

  os_mutexUnlock (&pool->m_mutex);

  /* Wait for threads to complete */

  while (retry--)
  {
    os_mutexLock (&pool->m_mutex);
    threads = pool->m_threads;
    os_mutexUnlock (&pool->m_mutex);

    if (threads == 0)
    {
      break;
    }
    os_nanoSleep (delay);
  }

  /* Delete all free jobs from queue */

  while (pool->m_free)
  {
    job = pool->m_free;
    pool->m_free = job->m_next_job;
    os_free (job);
  }

  os_sem_destroy (&pool->m_sem);
  os_mutexDestroy (&pool->m_mutex);
  os_free (pool);
}

os_result ut_thread_pool_submit
  (ut_thread_pool pool, os_threadRoutine fn, void * arg)
{
  os_result res = os_resultSuccess;
  ddsi_work_queue_job_t job;

  os_mutexLock (&pool->m_mutex);

  if (pool->m_job_max && pool->m_job_count >= pool->m_job_max)
  {
     /* Maximum number of jobs reached */

     res = os_resultBusy;
  }
  else
  {
    /* Get or create new job */

    if (pool->m_free)
    {
      job = pool->m_free;
      pool->m_free = job->m_next_job;
    }
    else
    {
      job = os_malloc (sizeof (*job));
    }
    job->m_next_job = NULL;
    job->m_fn = fn;
    job->m_arg = arg;

    /* Add new job to end of queue */

    if (pool->m_jobs)
    {
      pool->m_jobs_tail->m_next_job = job;
    }
    else
    {
      pool->m_jobs = job;
    }
    pool->m_jobs_tail = job;
    pool->m_job_count++;

    /* Allocate thread if more jobs than waiting threads and within maximum */

    if (pool->m_waiting < pool->m_job_count)
    {
      if ((pool->m_thread_max == 0) || (pool->m_threads < pool->m_thread_max))
      {
        /* OK if fails as have queued job */

        ut_thread_pool_new_thread (pool);
      }
    }

    /* Wakeup processing thread */

    os_sem_post (&pool->m_sem);
  }

  os_mutexUnlock (&pool->m_mutex);

  return res;
}

void ut_thread_pool_purge (ut_thread_pool pool)
{
  os_uint32 total;

  os_mutexLock (&pool->m_mutex);
  total = pool->m_threads;
  while (pool->m_waiting && (total > pool->m_thread_min))
  {
    pool->m_waiting--;
    total--;
    os_sem_post (&pool->m_sem);
  }
  os_mutexUnlock (&pool->m_mutex);
}
