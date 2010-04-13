/* Originally based on the Linuxthreads[1] mutex implementation, but
   very heavily modified and extended to provide something like
   pthreads mutexes & condition variables across processes in shared
   memory.

   Copyright (C) 2009 Erik Boasson (eb@ilities.com)

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License
   as published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.
                                                                      
   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
   
   [1] Linuxthreads - a simple clone()-based implementation of Posix
       threads for Linux. Copyright (C) 1998 Xavier Leroy
       (Xavier.Leroy@inria.fr) */
#include <errno.h>
#include <sched.h>
#include <time.h>
#include <stdlib.h>
#include <limits.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/time.h>

#include <libkern/OSAtomic.h>

#include "include/msem.h"

/* The status field of a spinlock is a pointer whose least significant
   bit is a locked flag.

   Thus the field values have the following meanings:

   status == 0:       spinlock is free
   status == 1:       spinlock is taken; no thread is waiting on it

   (status & 1) == 1: spinlock is taken and (status & ~1L) is a
                      pointer to the first waiting thread; other
		      waiting threads are linked via the p_nextlock
		      field.
   (status & 1) == 0: same as above, but spinlock is not taken.

   The waiting list is not sorted by priority order.  Actually, we
   always insert at top of list (sole insertion mode that can be
   performed without locking). For __pthread_unlock, we perform a
   linear search in the list to find the highest-priority, oldest
   waiting thread. This is safe because there are no concurrent
   __pthread_unlock operations -- only the thread that locked the
   mutex can unlock it. */

/* Memory barriers -- not needed on x86 & ARMs, I hope */
#define MEMORY_BARRIER()
#define READ_MEMORY_BARRIER()
#define WRITE_MEMORY_BARRIER()

typedef struct __msem_thrdescr
{
  struct __msem_thrdescr *p_nextlock;	/* can be on a queue and waiting on a lock */
  int p_pid;				/* PID of Unix process */
  pthread_t p_pthread;			/* pthread within process */
  __msem_t cvsem;			/* for cond_wait & cond_timedwait */
  struct __msem_thrdescr *wnext;	/* for cond_wait & cond_timedwait */
  struct __msem_thrdescr *enext;	/* for cond_timedwait */
  struct __msem_thrdescr *eprev;	/* for cond_timedwait */
  int on_toq;				/* for cond_timedwait */
  struct timespec to;			/* for cond_timedwait */
} *thrdescr_t;

#define MAX_THREADS 1024
static const int max_threads = MAX_THREADS;
static const int shm_size = MAX_THREADS * sizeof (struct __msem_thrdescr);
static thrdescr_t thrdescrs;
static pthread_key_t thread_info_key;
static int shmid;
static int semid;

/* Semaphore number for suspending/waking up thread THR; thread
   descriptors are protected by semaphore number 0. */
#define WKSEM(thr) (1 + (int) ((thr) - thrdescrs))

/* Timeout queue plus thread used for waking up threads that timed out
   in cond_timedwait(); by using a separate thread with standard
   pthreads operations we can get away with a very simple
   implementation without worrying about timers. */
static thrdescr_t toq = NULL;
static pthread_mutex_t toq_lock;
static pthread_cond_t toq_cond;
static pthread_t timeout_tid;
static volatile int terminate;

static void *timeout_thread (void *varg);

#define DIE0(fmt) do {					\
    fprintf (stderr, "%s:%d:" fmt, __FILE__, __LINE__);	\
    abort ();						\
  } while (0)
#define DIE1(fmt, a) do {					\
    fprintf (stderr, "%s:%d:" fmt, __FILE__, __LINE__, (a));	\
    abort ();							\
  } while (0)
#define DIE3(fmt, a, b, c) do {						\
    fprintf (stderr, "%s:%d:" fmt, __FILE__, __LINE__, (a), (b), (c));	\
    abort ();								\
  } while (0)

static void clean_dirty_state (void)
{
  union semun arg;
  int i;
  arg.val = 1;
  if (semctl (semid, 0, SETVAL, arg) == -1)
    DIE1 ("semctl set master sysv-semaphore to 1 failed, errno = %d\n", errno);
  arg.val = 0;
  for (i = 1; i <= max_threads; i++)
  {
    if (semctl (semid, i, SETVAL, arg) == -1)
      DIE1 ("semctl set thread sysv-semaphore to 0 failed, errno = %d\n", errno);
  }
}

void __msem_init_global (void)
{
  key_t key;
  void *addr;
  int do_clean;
  int size;

  pthread_key_create (&thread_info_key, NULL);

  if (getenv ("__MSEM_KEY"))
    key = strtoul (getenv ("__MSEM_KEY"), NULL, 0);
  else
    key = 0x22223333;
  if (getenv ("__MSEM_ADDR"))
    addr = (void *) strtoul (getenv ("__MSEM_ADDR"), NULL, 0);
  else
    /*addr = (void *) 0x200000000;*/
    addr = (void *) 0x50000000;
  /*fprintf (stderr, "__msem_init_global: sysv ipc key %x addr %p\n", (unsigned) key, addr);*/

  do_clean = 0;

  size = shm_size;
  if ((size % getpagesize()) != 0) {
    size += getpagesize() - (size % getpagesize());
  }

  if ((shmid = shmget(key, size, IPC_CREAT | IPC_EXCL | 0600)) > 0) {
    do_clean = 1;
  } else {
    if (errno == EEXIST) {
      /* we'll be the client process since the shm seg already exists */
      if ((shmid = shmget(key, 0, 0)) == -1) {
        DIE1 ("__msem_init_global: shmget errno %d\n", errno);
      } else {
        /* do nothing */
      }
    } else {
      DIE1 ("__msem_init_global: shmget (creat|excl) errno %d\n", errno);
    }
  }

  /*fprintf (stderr, "__msem_init_global: shmid = %d size = %d\n", shmid, size);*/

  if ((thrdescrs = shmat(shmid, addr, 0)) == (thrdescr_t)-1) {
  /*if ((thrdescrs = shmat(shmid, 0, 0)) == (thrdescr_t)-1) {*/
    DIE1 ("__msem_init_global: shmat errno %d\n", errno);
  } else {
    /*fprintf (stderr, "__msem_init_global: thrdescrs = %p\n", (void *)thrdescrs);*/
  }

  if ((semid = semget(key, 1 + max_threads, IPC_CREAT | IPC_EXCL | 0600)) > 0) {
    do_clean = 1;
  } else {
    if (errno == EEXIST) {
      /* we'll be the client process here */
      if ((semid = semget(key, 0, 0)) == -1) {
        DIE1 ("__msem_init_global: semget errno %d\n", errno);
      } else {
        /* do nothing */
      }
    } else {
      DIE1 ("__msem_init_global: semget (creat|excl) errno %d\n", errno);
    }
  }
   
  /*fprintf (stderr, "__msem_init_global: semid = %d\n", semid);*/
  
  if (do_clean)
  {
    fprintf (stderr, "__msem_init_global: resetting semaphores\n");
    clean_dirty_state ();
    memset (addr, 0, size);
  }

  /* cond_timedwait() support */
  {
    sigset_t allsigs, oldsigs;
    toq = NULL;
    pthread_mutex_init (&toq_lock, NULL);
    pthread_cond_init (&toq_cond, NULL);
    sigfillset (&allsigs);
    pthread_sigmask (SIG_BLOCK, &allsigs, &oldsigs);
    pthread_create (&timeout_tid, NULL, timeout_thread, NULL);
    pthread_sigmask (SIG_SETMASK, &oldsigs, NULL);
  }
  
  thrdescrs = addr;
  terminate = 0;
}

void __msem_fini_global (void)
{
  int result;
  int shmid;
  struct shmid_ds shmid_ds;

  pthread_mutex_lock (&toq_lock);
  terminate = 1;
  pthread_mutex_unlock (&toq_lock);
  pthread_cond_signal (&toq_cond);
  pthread_join (timeout_tid, NULL);
  pthread_mutex_destroy (&toq_lock);
  pthread_cond_destroy (&toq_cond);

  /* detach shared memory first */
  if ((result = shmdt(thrdescrs)) == -1) {
    fprintf (stderr, "__msem_fini_global: shmdt errno %d\n", errno);
    fprintf (stderr, "__msem_fini_global: detachment from shared mem unsuccessful\n");
  } else {
    /*fprintf (stderr, "__msem_fini_global: detachment from shared mem successful\n");*/
    /* then need to destroy the shared memory, if there's no
       one else attached to it */
    if ((shmid = shmget(0x22223333, 0, 0)) == -1) {
      fprintf (stderr, "__msem_fini_global: shmget errno %d\n", errno);
      fprintf (stderr, "__msem_fini_global: unable to get shared memory to attempt deallocation\n");
    } else {
      if ((result = shmctl(shmid, IPC_STAT, &shmid_ds)) == -1) {
        fprintf (stderr, "__msem_fini_global: shmctl (IPC_STAT) errno %d\n", errno);
        fprintf (stderr, "__msem_fini_global: unable to obtain shared memory status\n");
      } else {
        if (shmid_ds.shm_nattch) {
          fprintf (stderr, "__msem_fini_global: unable to destroy shared mem since %d users still attached\n", shmid_ds.shm_nattch);
        } else {
          if ((result = shmctl(shmid, IPC_RMID, NULL)) == -1) {
            fprintf (stderr, "__msem_fini_global: shmctl (IPC_RMID) errno %d\n", errno);
            fprintf (stderr, "__msem_fini_global: unable to mark shared memory for deallocation");
          } else {
            if ((semid = semget(0x22223333, 0, 0)) == -1) {
              fprintf (stderr, "__msem_fini_global: shmget errno %d\n", errno);
              fprintf (stderr, "__msem_fini_global: unable to get semaphore set to attempt deallocation\n");
            } else {
              if ((result = semctl(semid, 0, IPC_RMID)) == -1) {
                fprintf (stderr, "__msem_fini_global: semctl (IPC_STAT) errno %d\n", errno);
                fprintf (stderr, "__msem_fini_global: unable to mark semaphore set for deallocation\n");
              } else {
                fprintf (stderr, "__msem_fini_global: detachment and deallocation of shared mem and semaphore set successful\n");
              }
            }
          }
        }
      }
    }
  }
}

/* HELPERS ----------------------------------------------------------------- */

static void oper_on_sem (int semid, int semnum, int op)
{
  struct sembuf sop;
  int r;
  sop.sem_num = semnum;
  sop.sem_op  = op;
  sop.sem_flg = 0;
  while ((r = semop (semid, &sop, 1)) == -1 && errno == EINTR);
  if (r == -1) DIE0 ("oper_on_master: semop failed\n");
}

static void restart (thrdescr_t th)
{
  oper_on_sem (semid, WKSEM (th), 1);
}

static void suspend (thrdescr_t self)
{
  oper_on_sem (semid, WKSEM (self), -1);
}

static void oper_on_master (int op)
{
  oper_on_sem (semid, 0, op);
}

static thrdescr_t thrdescr_self (void)
{
  thrdescr_t self = pthread_getspecific (thread_info_key);
  if (self == NULL)
  {
    /* Hunt it down */
    int mypid = getpid ();
    int i, avail = -1;
    thrdescr_t new;
    pthread_t pt = pthread_self ();

    /*fprintf (stderr, "%d:%x looking for self...\n", mypid, (unsigned) pt);*/
    oper_on_master (-1);
    /*fprintf (stderr, "%d:%x got master...\n", mypid, (unsigned) pt);*/
    for (i = 0; i < max_threads; i++)
    {
      if (thrdescrs[i].p_pid == 0)
      {
	/* If the stored pid is 0, it is available by definition */
	if (avail == -1) avail = i;
      }
      else if (mypid == thrdescrs[i].p_pid)
      {
	/* My process ID, check for the thread id */
	if (pthread_equal (pt, thrdescrs[i].p_pthread))
	{
	  /* pid & thread match => it's got be me */
	  self = &thrdescrs[i];
	  goto done;
	}
	else if (pthread_kill (thrdescrs[i].p_pthread, 0) != 0)
	{
	  /* same pid, different thread, but can't send a signal, so
	     the entry is (supposedly) available */ 
	  if (avail == -1) avail = i;
	}
      }
      else if (kill (thrdescrs[i].p_pid, 0) != 0)
      {
	/* Can't send a signal => assume the process is gone */
	if (avail == -1) avail = i;
      }
      else
      {
	/* Different process, alive & well. Can't check if the thread
	   has terminated, so we can't know if the entry is
	   available. */
      }
    }

    /*fprintf (stderr, "avail = %d\n", avail);*/

    if (i < max_threads)
      new = &thrdescrs[i];
    else
      new = &thrdescrs[avail];

    new->p_nextlock = NULL;
    new->p_pid = mypid;
    new->p_pthread = pt;
    __msem_init (&new->cvsem, 1);
    new->wnext = NULL; /* not strictly necessary */
    new->enext = NULL; /* ditto */
    new->eprev = NULL; /* ditto */
    new->on_toq = 0;
    /*fprintf (stderr, "resetting semaphore\n");*/
    {
      union semun arg;
      arg.val = 0;
      if (semctl (semid, WKSEM (new), SETVAL, arg) == -1)
	DIE3 ("semctl set threads sysv-semaphore (%d:%d) to 0 failed, errno = %d\n", (int) semid, WKSEM (new), errno);
    }
    self = new;
  done:
    pthread_setspecific (thread_info_key, self);
    /*fprintf (stderr, "done\n");*/
    oper_on_master (1);
  }
  return self;
}

void __msem_init (__msem_t *lock, int locked)
{
  lock->status = (locked ? 1 : 0);
}

void __msem_remove (__msem_t *lock __attribute__ ((unused)))
{
}

int __msem_trylock (__msem_t *lock)
{
  long oldstatus;
  do {
    oldstatus = lock->status;
    if (oldstatus != 0) return EAGAIN;
  } while (!OSAtomicCompareAndSwapLong (0, 1, &lock->status));
  return 0;
}

void __msem_lock (__msem_t *lock)
{
  thrdescr_t self = 0;

  long oldstatus, newstatus;
  int successful_seizure;

again:

  /* Try once or suspend. */
  do {
    oldstatus = lock->status;
    successful_seizure = 0;

    if ((oldstatus & 1) == 0) {
      newstatus = oldstatus | 1;
      successful_seizure = 1;
    } else {
      self = thrdescr_self ();
      newstatus = (long) self | 1;
    }

    if (self != NULL) {
      self->p_nextlock = (thrdescr_t) (oldstatus & ~1L);
      /* Make sure the store in p_nextlock completes before performing
         the compare-and-swap */
      MEMORY_BARRIER ();
    }
  } while (!OSAtomicCompareAndSwapLong (oldstatus, newstatus, &lock->status));

  /* Suspend with guard against spurious wakeup. Spurious wakeups
     happen when a call to cond_timedwait times out just when it is
     being triggered by cond_signal() or cond_broadcast(). SysV
     semaphores are counting semaphores so the first suspend following
     such an event will fall through, but the second will be
     successful. */
  if (!successful_seizure)
  {
    do {
      suspend (self);
    } while (self->p_nextlock != NULL);
    goto again;
  }
}

void __msem_unlock (__msem_t *lock)
{
  thrdescr_t thr, *ptr, *maxptr;
  long oldstatus;

again:

  while ((oldstatus = lock->status) == 1) {
    if (OSAtomicCompareAndSwapLong (oldstatus, 0, &lock->status)) {
      return;
    }
  }

  /* Find last thread in waiting queue */
  ptr = (thrdescr_t *) &lock->status;
  thr = (thrdescr_t) (oldstatus & ~1L);
  maxptr = ptr;
  while (thr != 0) {
    maxptr = ptr;
    ptr = &thr->p_nextlock;
    /* Prevent reordering of the load of lock->status above and the
       load of *ptr below, as well as reordering of *ptr between
       several iterations of the while loop.  Some processors (e.g.
       multiprocessor Alphas) could perform such reordering even though
       the loads are dependent. */
    READ_MEMORY_BARRIER();
    thr = *ptr;
  }
  /* Prevent reordering of the load of lock->status above and
     thr->p_nextlock below */
  READ_MEMORY_BARRIER();
  /* Remove thread from waiting list. */
  if (maxptr == (thrdescr_t *) &lock->status) {
    /* If thread is at head, remove it with compare-and-swap to guard
       against concurrent lock operation. This removal also has the
       side effect of marking the lock as released because the new
       status comes from thr->p_nextlock whose least significant bit
       is clear. */
    thr = (thrdescr_t) (oldstatus & ~1L);
    if (!OSAtomicCompareAndSwapLong (oldstatus, (long) thr->p_nextlock, &lock->status))
      goto again;
  } else {
    /* No risk of concurrent access, remove thread normally.  But in
       this case we must also flip the least significant bit of the
       status to mark the lock as released. */
    thr = *maxptr;
    *maxptr = thr->p_nextlock;
    do {
      oldstatus = lock->status;
    } while (!OSAtomicCompareAndSwapLong (oldstatus, oldstatus & ~1L, &lock->status));
  }
  /* Prevent reordering of store to *maxptr above and store to thr->p_nextlock
     below */
  WRITE_MEMORY_BARRIER();
  /* Wake up the selected waiting thread */
  thr->p_nextlock = NULL;
  restart (thr);
}

/****/

void __msem_mutex_init (__msem_mutex_t *mtx)
{
  __msem_init (mtx, 0);
}

void __msem_mutex_destroy (__msem_mutex_t *mtx)
{
  __msem_remove (mtx);
}

void __msem_mutex_lock (__msem_mutex_t *mtx)
{
  __msem_lock (mtx);
}

int __msem_mutex_trylock (__msem_mutex_t *mtx)
{
  return __msem_trylock (mtx);
}

void __msem_mutex_unlock (__msem_mutex_t *mtx)
{
  __msem_unlock (mtx);
}

/****/

void __msem_cond_init (__msem_cond_t *cv)
{
  __msem_init (&cv->lock, 0);
  cv->waiters = NULL;
}

void __msem_cond_destroy (__msem_cond_t *cv)
{
  __msem_remove (&cv->lock);
}

void __msem_cond_signal (__msem_cond_t *cv)
{
  __msem_lock (&cv->lock);
  if (cv->waiters)
  {
    thrdescr_t t = cv->waiters;
    cv->waiters = cv->waiters->wnext;
    restart (t);
  }
  __msem_unlock (&cv->lock);
}

void __msem_cond_broadcast (__msem_cond_t *cv)
{
  __msem_lock (&cv->lock);
  while (cv->waiters)
  { 
    thrdescr_t t = cv->waiters;
    cv->waiters = cv->waiters->wnext;
    restart (t);
  }
  __msem_unlock (&cv->lock);
}

void __msem_cond_wait (__msem_cond_t *cv, __msem_mutex_t *mtx)
{
  thrdescr_t self = thrdescr_self ();
  __msem_lock (&cv->lock);
  self->wnext = cv->waiters;
  cv->waiters = self;
  __msem_unlock (&cv->lock);
  __msem_unlock (mtx);
  /* Note that the spurious wakeups in __msem_lock() can occur here,
     too, but cond_wait/cond_timedwait is allowed the occasional
     spurious wakeup */
  suspend (self);
  __msem_lock (mtx);
}

static void *timeout_thread (void *varg __attribute__ ((unused)))
{
  pthread_mutex_lock (&toq_lock);
  while (!terminate)
  {
    struct timespec to;
    struct timeval tv;

    if (toq)
    {
      to = toq->to;
      pthread_cond_timedwait (&toq_cond, &toq_lock, &to);
    }
    else
    {
      pthread_cond_wait (&toq_cond, &toq_lock);
    }
    
    gettimeofday (&tv, NULL);
    to.tv_sec = tv.tv_sec;
    to.tv_nsec = 1000 * tv.tv_usec;
    
    while (toq &&
	   (toq->to.tv_sec < to.tv_sec ||
	    (toq->to.tv_sec == to.tv_sec && toq->to.tv_nsec <= to.tv_nsec)))
    {
      thrdescr_t thr = toq;
      toq = thr->enext;
      if (toq) toq->eprev = NULL;
      OSMemoryBarrier ();
      thr->on_toq = 0;
      restart (thr);
    }
  }
  pthread_mutex_unlock (&toq_lock);
  return NULL;
}

static void add_to_toq (thrdescr_t self, const struct timespec *abstime)
{
  thrdescr_t pt, t;
  pthread_mutex_lock (&toq_lock);
  self->on_toq = 1;
  self->to = *abstime;
  pt = NULL;
  t = toq;
  while (t && (t->to.tv_sec < abstime->tv_sec ||
	       (t->to.tv_sec == abstime->tv_sec && t->to.tv_nsec < abstime->tv_nsec)))
  {
    pt = t;
    t = t->enext;
  }
  if (pt == NULL)
  {
    /* new first entry */
    self->enext = t;
    if (t) t->eprev = self;
    self->eprev = NULL;
    toq = self;
    pthread_cond_signal (&toq_cond);
  }
  else
  {
    /* put it between pt and t */
    self->enext = t;
    if (t) t->eprev = self;
    self->eprev = pt;
    pt->enext = self;
  }
  pthread_mutex_unlock (&toq_lock);
}

static int rm_from_toq (thrdescr_t self)
{
  if (!self->on_toq)
  {
    /* Guaranteed no longer on toq */
    return 0;
  }
  else
  {
    /* Probably still on toq */
    pthread_mutex_lock (&toq_lock);
    if (self->on_toq)
    {
      if (self->eprev)
	self->eprev->enext = self->enext;
      else
	toq = self->enext;
      if (self->enext)
	self->enext->eprev = self->eprev;
      self->on_toq = 0;
    }
    pthread_mutex_unlock (&toq_lock);
    return 1;
  }
}

int __msem_cond_timedwait (__msem_cond_t *cv, __msem_mutex_t *mtx, const struct timespec *abstime)
{
  thrdescr_t self = thrdescr_self ();
  thrdescr_t *pt, t;
  int ret = 0;

  add_to_toq (self, abstime);
  
  __msem_lock (&cv->lock);
  self->wnext = cv->waiters;
  cv->waiters = self;
  __msem_unlock (&cv->lock);
  __msem_unlock (mtx);

  /* Note that the spurious wakeups in __msem_lock() can occur here,
     too, but cond_wait/cond_timedwait is allowed the occasional
     spurious wakeup */
  suspend (self);

  if (rm_from_toq (self))
  {
    /* If we had to take ourselves off the sleep queue, we must have
       been triggered by signal or broadcast and therefore are no
       longer on the list of waiters. */
    ret = 0;
  }
  else
  {
    __msem_lock (&cv->lock);
    /* Already removed from toq -- we did timeout, and are probably,
       though not necessarily, still listed as a waiter, as a regular
       wakeup may have taken/take place in the short window between
       time out and reaching this point. */
    pt = &cv->waiters;
    t = cv->waiters;
    while (t && t != self)
      pt = &t->wnext, t = t->wnext;
    if (t != NULL)
      *pt = t->wnext;
    __msem_unlock (&cv->lock);
    ret = t ? ETIMEDOUT : 0;
  }

  __msem_lock (mtx);
  return ret;
}
