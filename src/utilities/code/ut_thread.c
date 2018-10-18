/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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

#include "os_heap.h"
#include "os_stdlib.h"
#include "os_thread.h"
#include "os_report.h"
#include "os_time.h"
#include "os_mutex.h"
#include "os_atomics.h"

#include "ut_thread.h"

/* Things don't go wrong if CACHE_LINE_SIZE is defined incorrectly,
 * they just run slower because of false cache-line sharing. It can be
 * discovered at run-time, but in practice it's 64 for most CPUs and
 * 128 for some.
 */
#define UT_CACHE_LINE_SIZE 64


/* GCC has a nifty feature allowing the specification of the required
 * alignment: __attribute__ ((aligned (CACHE_LINE_SIZE))) in this
 * case. Many other compilers implement it as well, but it is by no
 * means a standard feature.  So we do it the old-fashioned way.
 */

enum ut_threadStateEnum {
    UT_THREAD_STATE_ZERO,
    UT_THREAD_STATE_REAP,
    UT_THREAD_STATE_ALIVE
};

struct ut_threadLiveliness {
    os_boolean alive;
    os_uint32 prevDrowsyCnt;

    os_uint32 sleepCnt;
    os_duration sleepCum;
    os_duration awakeCum;
};


/* drowsyCnt even : thread awake
 * drowsyCnt odd  : thread asleep
 */
#define UT_THREAD_BASE                          \
    volatile os_uint32 drowsyCnt;               \
    volatile os_uint32 sleepSec;                \
    os_threadId tid;                            \
    struct ut_threadLiveliness liveliness;      \
    volatile enum ut_threadStateEnum state;     \
    void *threads;                              \
    char *name;                                 \
    os_cond cond;                               \
    void * (*f) (void *arg);                    \
    void *f_arg /* note: no semicolon! */

struct ut_threadStateBase {
    UT_THREAD_BASE;
};


OS_STRUCT(ut_thread) {
    UT_THREAD_BASE;
    char pad[UT_CACHE_LINE_SIZE
             * ((sizeof (struct ut_threadStateBase) + UT_CACHE_LINE_SIZE - 1)
                / UT_CACHE_LINE_SIZE)
             - sizeof (struct ut_threadStateBase)];
};
#undef UT_THREAD_BASE


OS_STRUCT(ut_threads) {
    os_mutex lock;
    os_threadId main;
    void *userdata;
    os_int32 size;
    os_duration interval;
    os_timeM lastCheck;
    OS_STRUCT(ut_thread) *pool;
};



#if OS_HAS_TSD_USING_THREAD_KEYWORD
__thread ut_thread ut_tsdThreadState;
#endif


/* Sleep is managed in seconds. Non-null nanoseconds adds one second lag. */
#define UT_SLEEPSEC_FROM_DURATION(d) ((os_uint32)(OS_DURATION_GET_SECONDS(d) + (OS_DURATION_GET_NANOSECONDS(d) != 0)))

/* snprintf can return negative values. Only update position with non-negative values. */
#define UT_RET_POSITIVE(n) (((n) > 0) ? (unsigned)n : 0)

static void*
ut_mallocAligned(os_size_t size)
{
    /* This wastes some space, but we use it only once and it isn't a
     * huge amount of memory, just a little over a cache line.
     * Alternatively, we good use valloc() and have it aligned to a
     * page boundary, but that one isn't part of the O/S abstraction
     * layer ...
     */
    const os_address clm1 = UT_CACHE_LINE_SIZE - 1;
    os_address ptrA;
    void **pptr;
    void *ptr;
    ptr = os_malloc(size + UT_CACHE_LINE_SIZE + sizeof(void *));
    if (ptr == NULL) {
        return NULL;
    }
    ptrA = ((os_address) ptr + sizeof (void *) + clm1) & ~clm1;
    pptr = (void **) ptrA;
    pptr[-1] = ptr;
    return (void *) ptrA;
}


static void
ut_freeAligned(void *ptr)
{
    void **pptr = ptr;
    os_free (pptr[-1]);
}


static os_boolean
ut_drowsyAwake(os_uint32 cnt)
{
    return (cnt % 2) == 0;
}


static os_boolean
ut_drowsyAsleep(os_uint32 cnt)
{
    return (cnt % 2) == 1;
}


static void*
ut_threadWrapper (void *vself)
{
    ut_thread const self = vself;
    void *ret;
    /* Lock to ensure d_threadCreate has updated the treadState */
    os_mutexLock(&(((ut_threads)(self->threads))->lock));
    os_mutexUnlock (&(((ut_threads)(self->threads))->lock));
    ret = self->f(self->f_arg);
    ut_threadAsleep(self, UT_SLEEP_INDEFINITELY);
    pa_fence();
    self->state = UT_THREAD_STATE_REAP;
    os_mutexLock(&(((ut_threads)(self->threads))->lock));
    os_condBroadcast(&self->cond);
    os_mutexUnlock (&(((ut_threads)(self->threads))->lock));

    return ret;
}

static ut_thread
ut_threadFindFreeSlot(ut_threads threads, const char *name)
{
    os_int32 i;
    assert(name);
    assert(threads);
    for (i = 0; i < threads->size; i++) {
        ut_thread thr = &(threads->pool[i]);
        if (thr->state == UT_THREAD_STATE_ZERO) {
            pa_fence();
            assert(thr->name == NULL);
            return thr;
        }
    }
    OS_REPORT(OS_ERROR, OS_FUNCTION, 0, "%s: no free slot", name ? name : "(anon)");
    return NULL;
}

static void
ut_threadStateSet(ut_thread thr, enum ut_threadStateEnum state, os_threadId tid, const char *name)
{
    assert(thr);
    pa_fence_acq ();
    if (thr->name != NULL) {
        os_free(thr->name);
        thr->name = NULL;
    }
    if (name != NULL) {
        thr->name = os_strdup(name);
    }
    thr->tid = tid;
    thr->state = state;
    pa_fence_rel ();
}

os_threadId
ut_threadGetId(ut_thread thr)
{
    assert(thr);
    return thr->tid;
}

const char*
ut_threadGetName(ut_thread thr)
{
    assert(thr);
    return thr->name;
}

os_uint32
ut_threadToString(ut_thread thr, os_boolean alive, const char *info, char *buf, os_uint32 size)
{
    int ret;
    assert(thr);
    assert(buf);
    if (info != NULL) {
        ret = snprintf(buf, size,
                       " \"%s\"(0x%" PA_PRIxADDR "):%c:%s",
                       thr->name,
                       (os_address)os_threadIdToInteger(thr->tid),
                       alive ? 'a' : 'd',
                       info);
    } else {
        ret = snprintf(buf, size,
                       " \"%s\"(0x%" PA_PRIxADDR "):%c",
                       thr->name,
                       (os_address)os_threadIdToInteger(thr->tid),
                       alive ? 'a' : 'd');
    }
    return (ret > 0) ? (os_uint32)ret : 0;
}

void
ut_threadCreate(ut_threads threads, ut_thread *thr, const char *name, const os_threadAttr *threadAttr, os_threadRoutine startRoutine, void *arg)
{
    os_result rc = os_resultFail;

    assert(thr);
    assert(threads);
    assert(threads->pool);

    os_mutexLock(&(threads->lock));
    *thr = ut_threadFindFreeSlot(threads, name);
    if (*thr != NULL) {
        (*thr)->name = os_strdup(name);
        if ((*thr)->name != NULL) {
            (*thr)->f = startRoutine;
            (*thr)->f_arg = arg;
            rc = os_threadCreate(&((*thr)->tid), name, threadAttr, ut_threadWrapper, *thr);
            if (rc == os_resultSuccess) {
                pa_fence ();
                ut_threadAwake(*thr);
                (*thr)->state = UT_THREAD_STATE_ALIVE;
            } else {
                OS_REPORT(OS_ERROR, OS_FUNCTION, 0, "%s: failed to create thread: %s", name, os_resultImage(rc));
                os_free((*thr)->name);
                (*thr)->name = NULL;
                (*thr)->f = NULL;
                (*thr)->f_arg = NULL;
                (*thr) = NULL;
            }
        } else {
            OS_REPORT(OS_ERROR, OS_FUNCTION, 0, "%s: out of memory", name);
            (*thr) = NULL;
        }
    } else {
        OS_REPORT(OS_ERROR, OS_FUNCTION, 0, "%s: out of thread slots (max %d)", name, (int)threads->size);
    }
    os_mutexUnlock (&(threads->lock));
}

os_result
ut_threadWaitExit(ut_thread thr, void **result)
{
    os_result r;

    assert(thr);

    r = os_threadWaitExit(thr->tid, result);
    if (r == os_resultSuccess) {
        ut_threadStateSet(thr, UT_THREAD_STATE_ZERO, OS_THREAD_ID_NONE, NULL);
    }

    return r;
}

os_result
ut_threadTimedWaitExit(ut_thread thr, os_duration timeout, void **result)
{
    os_result r = os_resultSuccess;
    ut_threads threads = (ut_threads)thr->threads;
    ut_thread self;
    assert(thr);

    self = ut_threadLookupSelf(threads);
    os_mutexLock(&threads->lock);
    if (thr->state != UT_THREAD_STATE_REAP) {
        r = ut_condTimedWait(self, &thr->cond, &threads->lock, timeout);
    }
    os_mutexUnlock(&threads->lock);
    if (r == os_resultSuccess) {
        r = os_threadWaitExit(thr->tid, result);
    }
    if (r == os_resultSuccess) {
        ut_threadStateSet(thr, UT_THREAD_STATE_ZERO, OS_THREAD_ID_NONE, NULL);
    }

    return r;
}


os_result
ut_sleep(ut_thread thr, os_duration delay)
{
    os_result r;
    assert(thr);
    assert(ut_drowsyAwake(thr->drowsyCnt));
    ut_threadAsleep(thr, UT_SLEEPSEC_FROM_DURATION(delay));
    r = ospl_os_sleep(delay);
    ut_threadAwake(thr);
    return r;
}

os_result
ut_condTimedWait(ut_thread thr, os_cond *cv, os_mutex *mtx, os_duration timeout)
{
    os_result r;
    assert(thr);
    assert(ut_drowsyAwake(thr->drowsyCnt));
    ut_threadAsleep(thr, UT_SLEEPSEC_FROM_DURATION(timeout));
    r = os_condTimedWait(cv, mtx, timeout);
    ut_threadAwake(thr);
    return r;
}

void
ut_condWait(ut_thread thr, os_cond *cv, os_mutex *mtx)
{
    assert(thr);
    assert(ut_drowsyAwake(thr->drowsyCnt));
    ut_threadAsleep(thr, UT_SLEEP_INDEFINITELY);
    os_condWait(cv, mtx);
    ut_threadAwake(thr);
}

ut_thread
ut_threadLookupId(ut_threads threads, os_threadId tid)
{
    os_int32 i;
    assert(threads);
    if (threads->pool) {
        for (i = 0; i < threads->size; i++) {
            if (os_threadIdToInteger(threads->pool[i].tid) == os_threadIdToInteger(tid)) {
                return &(threads->pool[i]);
            }
        }
    }
    return NULL;
}

ut_thread
ut_threadLookupSelf(ut_threads threads)
{
#if OS_HAS_TSD_USING_THREAD_KEYWORD
    if (ut_tsdThreadState == NULL) {
      ut_tsdThreadState = ut_threadLookupId(threads, os_threadIdSelf());
    }
    return ut_tsdThreadState;
#else
  return ut_threadLookupId(threads, os_threadIdSelf());
#endif
}


void
ut_threadAsleep(ut_thread thr, os_uint32 sec)
{
    os_uint32 dCnt;
    assert(thr);
    dCnt = thr->drowsyCnt;
    thr->sleepSec = sec;
    if (ut_drowsyAwake(dCnt)) {
        pa_fence_acq ();
        thr->drowsyCnt = dCnt + 1;
    } else {
        pa_fence_acq ();
        thr->drowsyCnt = dCnt + 2;
    }
    pa_fence_rel();
}

void
ut_threadAwake(ut_thread thr)
{
    os_uint32 dCnt;
    assert(thr);
    dCnt = thr->drowsyCnt;
    if (ut_drowsyAsleep(dCnt)) {
        pa_fence_acq ();
        thr->drowsyCnt = dCnt + 1;
    } else {
        pa_fence_acq ();
        thr->drowsyCnt = dCnt + 2;
    }
    pa_fence_rel ();
}

void
ut_threadsSetInterval(ut_threads threads, os_duration interval)
{
    assert(threads);
    threads->interval = interval;
}

os_boolean
ut_threadsAllIsWell(ut_threads threads, ut_threadReport reportCb, void *reportData)
{
    ut_thread self;
    os_timeM tnow;
    os_duration tdelta;
    const char *caller = "<!ut_thread>";
    os_boolean ok = OS_FALSE;
    os_uint32 changeCnt = 0;
    os_int32 aliveCnt = 0;
    os_int32 i;

    assert(threads);
    assert(threads->pool);

    os_mutexLock(&(threads->lock));

    if (OS_TIMEM_GET_VALUE(threads->lastCheck) == 0) {
        /* We need to initialize the liveliness. */
        threads->lastCheck = os_timeMGet();
    }

    self = ut_threadLookupSelf(threads);
    tnow = os_timeMGet();
    tdelta = os_timeMDiff(tnow, threads->lastCheck);

    if (self != NULL) {
        caller = self->name;
    }

    /* Check whether all threads have made progress (or have declared themselves to be asleep). */
    for (i = 0; i < threads->size; i++) {
        ut_thread thr = &(threads->pool[i]);
        switch (thr->state) {
            case UT_THREAD_STATE_ALIVE:
                pa_fence ();
                {
                    os_uint32 sleepSec;
                    os_uint32 dCnt0, dCnt;
                    os_boolean alive;

                    /* Bracket reading of sleepSec with reading drowsyCnt, so that we can
                     * detect races with the other thread.
                     */
                    dCnt0 = thr->drowsyCnt;
                    pa_fence_acq ();
                    sleepSec = thr->sleepSec;
                    pa_fence_acq ();
                    dCnt = thr->drowsyCnt;

                    if (ut_drowsyAwake(dCnt)) {
                        /* thread has declared itself awake */
                        if (dCnt != thr->liveliness.prevDrowsyCnt) {
                            /* drowsyCnt has changed, so thread has declared
                             * itself awake within the interval: restart
                             */
                            thr->liveliness.awakeCum = 0;
                            alive = OS_TRUE;
                        } else {
                            /* accumulate awake time and check if the interval
                             * hasn't passed yet
                             */
                            thr->liveliness.awakeCum += tdelta;
                            alive = (thr->liveliness.awakeCum < threads->interval);
                        }
                    } else {
                        /* thread has declared itself asleep */
                        if (dCnt == thr->liveliness.sleepCnt) {
                            /* accumulate sleep time and check it is
                             * within bounds + interval period
                             */
                            thr->liveliness.sleepCum += tdelta;
                            alive = ((sleepSec == 0) || (thr->liveliness.sleepCum < (OS_DURATION_INIT(sleepSec, 0) + threads->interval)));
                        } else if (dCnt == dCnt0) {
                            /* sleepSec we read is consistent with drowsyCnt (since the drowsyCnt
                             * read before and after we read sleepSec are equal)
                             */
                            thr->liveliness.sleepCnt = dCnt;
                            thr->liveliness.sleepCum = 0;
                            alive = OS_TRUE;
                        } else {
                            /* inconsistent sleep state which means the thread must have been
                             * awake at that point; we update sleepCnt to avoid (with
                             * certainty) a race with the other thread where we eventually
                             * erroneously conclude that its drowsyCnt equals our sleepCnt
                             */
                            thr->liveliness.sleepCnt = dCnt - 2;
                            alive = OS_TRUE;
                        }
                    }

                    if (alive) {
                        aliveCnt++;
                    }

                    if (thr->liveliness.alive != alive) {
                        changeCnt++;
                    }

                    if (reportCb != NULL) {
                        char info[128] = "";
                        snprintf(info, sizeof(info), "%u->%u", thr->liveliness.prevDrowsyCnt, dCnt);
                        reportCb(thr,
                                 alive,
                                 thr->liveliness.alive != alive,
                                 info,
                                 caller,
                                 reportData,
                                 threads->userdata);
                    }

                    thr->liveliness.prevDrowsyCnt = dCnt;
                    thr->liveliness.alive = alive;
                }
                break;
            case UT_THREAD_STATE_REAP:
                /* FALLSTHROUGH */
            case UT_THREAD_STATE_ZERO:
                /* Simulate alive. */
                aliveCnt++;
                break;
        }
    }

    ok = (aliveCnt == threads->size);

    if (reportCb != NULL) {
        reportCb(NULL,
                 ok,
                 changeCnt > 0,
                 NULL,
                 caller,
                 reportData,
                 threads->userdata);
    }

    threads->lastCheck = tnow;

    os_mutexUnlock (&(threads->lock));

    return ok;
}


ut_threads
ut_threadsNew(const char *selfname, os_duration interval, os_int32 maxthreads, void *userdata)
{
    ut_threads threads = os_malloc(OS_SIZEOF(ut_threads));
    os_result osr;

    assert(selfname);
    assert(maxthreads > 0);
    if (threads) {
        os_int32 i;
        memset(threads, 0, OS_SIZEOF(ut_threads));
        osr = os_mutexInit(&(threads->lock), NULL);
        if (osr != os_resultSuccess) {
            OS_REPORT (OS_ERROR, OS_FUNCTION, 0, "failed to initialize mutex '%s'", os_resultImage(osr));
            os_free(threads);
            return NULL;
        }
        threads->pool = ut_mallocAligned((os_size_t)maxthreads * OS_SIZEOF(ut_thread));
        if (threads->pool == NULL) {
            os_mutexDestroy(&(threads->lock));
            OS_REPORT(OS_ERROR, OS_FUNCTION, 0, "failed to initialize threads table");
            os_free(threads);
            return NULL;
        }
        memset(threads->pool, 0, (os_size_t)maxthreads * OS_SIZEOF(ut_thread));
        for (i = 0; i < maxthreads && osr == os_resultSuccess; i++)
        {
            ut_thread thr = &(threads->pool[i]);
            thr->state = UT_THREAD_STATE_ZERO;
            thr->drowsyCnt = 1;
            thr->name  = NULL;
            thr->threads = (void*)threads;
            thr->liveliness.alive = OS_TRUE;
            osr = os_condInit(&thr->cond, &threads->lock, NULL);
        }
        if (osr != os_resultSuccess) {
            os_int32 j;
            OS_REPORT (OS_ERROR, OS_FUNCTION, 0, "failed to initialize cond[%d] '%s'", i, os_resultImage(osr));
            for (j = i; j >= i; j--) {
                ut_thread thr = &(threads->pool[i]);
                os_condDestroy(&thr->cond);
            }
            ut_freeAligned(threads->pool);
            os_mutexDestroy(&(threads->lock));
            os_free(threads);
            return NULL;
        }

        threads->size = maxthreads;
        threads->userdata = userdata;
        threads->interval = interval;

        /* Set the calling thread as main, alive and awake. */
        {
            ut_thread thr = ut_threadFindFreeSlot(threads, selfname);
            assert(thr);
            threads->main = os_threadIdSelf();
            ut_threadStateSet(thr, UT_THREAD_STATE_ALIVE, threads->main, selfname);
            ut_threadAwake(thr);
        }
    }
    return threads;
}

void
ut_threadsFree(ut_threads threads)
{
    os_int32 i;

    if (threads) {
        assert(threads->pool);

        /* Expect calling thread to be the main and simulate that it has stopped. */
        {
            ut_thread thr = ut_threadLookupSelf(threads);
            assert(thr == ut_threadLookupId(threads, threads->main));
            ut_threadAsleep(thr, UT_SLEEP_INDEFINITELY);
            ut_threadStateSet(thr, UT_THREAD_STATE_ZERO, OS_THREAD_ID_NONE, NULL);
        }

        for (i = 0; i < threads->size; i++) {
            ut_thread thr = &(threads->pool[i]);
            /* Just a sanity check. */
            assert(thr->state != UT_THREAD_STATE_ALIVE);
            assert(thr->state != UT_THREAD_STATE_REAP);
            if (thr->name != NULL) {
                os_free(thr->name);
                thr->name = NULL;
            }
            os_condDestroy(&thr->cond);
        }

        /* Now, cleanup the threads object. */
        os_mutexDestroy(&(threads->lock));
        ut_freeAligned(threads->pool);
        threads->pool = NULL;
        os_free(threads);
    }
}

void*
ut_threadsUserData(ut_threads threads)
{
    assert(threads);
    return threads->userdata;
}

