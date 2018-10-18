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
#include "cma__thread.h"
#include "u_service.h"

struct cma_threadContext {
    cma_thread self;
    cma_threadRoutine func;
    void *arg;
};

/* Things don't go wrong if CACHE_LINE_SIZE is defined incorrectly,
   they just run slower because of false cache-line sharing. It can be
   discovered at run-time, but in practice it's 64 for most CPUs and
   128 for some. */
#define CACHE_LINE_SIZE 64

typedef os_uint32 cma_vtime;
typedef os_int32 cma_svtime; /* signed version */

enum cma_threadState {
    CMA_THREAD_STATE_ZERO,
    CMA_THREAD_STATE_ALIVE
};

/*
 * watchdog indicates progress for the service lease liveliness mechanism, while vtime
 * indicates progress for the Garbage collection purposes.
 *  vtime even : thread awake
 *  vtime odd  : thread asleep
 */
#define THREAD_BASE                     \
    cma_threadStates ts;                \
    volatile cma_vtime vtime;           \
    volatile cma_vtime watchdog;        \
    os_threadId tid;                    \
    os_threadId extTid;                 \
    enum cma_threadState state;         \
    cma_logbuf logbuf;                  \
    char *name /* note: no semicolon! */

struct cma_threadStateBase {
        THREAD_BASE;
};

C_STRUCT(cma_thread) {
    C_EXTENDS(cma_object);
    THREAD_BASE;
    char pad[CACHE_LINE_SIZE
             * ((sizeof (struct cma_threadStateBase) + CACHE_LINE_SIZE - 1)
                 / CACHE_LINE_SIZE)
             - sizeof (struct cma_threadStateBase)];
};

#undef THREAD_BASE

C_STRUCT(cma_threadStates) {
    os_mutex lock;
    cma_logConfig gc;
    unsigned nthreads;
    cma_thread threads; /* [nthreads] */
};

#if OS_HAS_TSD_USING_THREAD_KEYWORD
__thread cma_thread cma_threadSelf;
#endif

static void
cma__threadReapState(
    cma_thread _this,
    c_bool sync_lease) __nonnull_all__;

static void*
cma__mallocCachelineAligned(
    os_size_t size) __attribute_malloc__ __attribute_returns_nonnull__
                    __attribute_assume_aligned__((CACHE_LINE_SIZE));

static void
cma__freeCachelineAligned(
    void *ptr) __nonnull_all__;

static void
cma__threadInit(
    cma_thread _this,
    cma_threadStates ts) __nonnull_all__;

static void
cma__threadObjDeinitFunc(
    cma_object _this) __nonnull_all__;

static void
cma__threadDeinit(
    cma_thread _this) __nonnull_all__;

static int
cma__threadStatesFindFreeSlot(
    cma_threadStates ts,
    const char *name) __nonnull_all__;

static cma_threadStates
cma__threadStatesLookup(void) __attribute_returns_nonnull__;

static void*
cma__mallocCachelineAligned(
    os_size_t size)
{
    /* This wastes some space, but we use it only once and it isn't a
     huge amount of memory, just a little over a cache line.
     Alternatively, we good use valloc() and have it aligned to a page
     boundary, but that one isn't part of the O/S abstraction layer ... */
    const os_address clm1 = CACHE_LINE_SIZE - 1;
    os_address ptrA;
    void **pptr;
    void *ptr;

    ptr = os_malloc(size + CACHE_LINE_SIZE + sizeof(void*));
    ptrA = ((os_address)ptr + sizeof(void*) + clm1) & ~clm1;
    pptr = (void**)ptrA;
    pptr[-1] = ptr;
    return (void*)ptrA;
}

static void
cma__freeCachelineAligned(
    void *ptr)
{
    void **pptr = ptr;
    os_free(pptr[-1]);
}

static void
cma__threadObjDeinitFunc(
    cma_object _this)
{
    cma_objectIsValidKind(_this, CMA_OBJECT_THREAD);

    cma__threadDeinit(cma_thread(_this));
}

static void
cma__threadInit(
    cma_thread _this,
    cma_threadStates ts)
{
    assert(_this);
    assert(ts);

    /* Note: memory for all threads is allocated by nb_threadStatesInit in one alloc,
     * aligned to cache-line size, re-usable */
    cma__objectInit(cma_object(_this), CMA_OBJECT_THREAD, cma__threadObjDeinitFunc);
    _this->ts = ts;
    _this->state = CMA_THREAD_STATE_ZERO;
    _this->vtime = 1;
    _this->watchdog = 1;
    _this->logbuf = NULL;
    _this->name = NULL;
}

static void
cma__threadDeinit(
    cma_thread _this)
{
    cma_objectIsValidKind(_this, CMA_OBJECT_THREAD);

    /* Thread objects are reused during service lifespan and all threads must be joined at service exit.
     * Memory for thread objects is allocated/deallocated once for all threads (see cma__threadStatesInit/Deinit).
     * Pointers in thread objects are freed when a thread is joined, so the object can be reused for new threads.
     * cma_objectFree must be used (as that frees memory of individual objects)
     */
    cma__objectDeinit(cma_object(_this));
}

cma_threadStates
cma_threadStatesAlloc(void)
{
    cma_threadStates ts = os_malloc(sizeof *ts);
    return ts;
}

os_result
cma_threadStatesInit(
    cma_threadStates ts,
    unsigned maxThreads,
    cma_logConfig gc)
{
    unsigned i;

    assert(ts);

    if (os_mutexInit(&(ts->lock), NULL) != os_resultSuccess) {
        goto err_mutexinit;
    }

    ts->gc = gc;
    ts->nthreads = maxThreads;
    ts->threads = cma__mallocCachelineAligned(ts->nthreads * sizeof(*ts->threads));
    memset(ts->threads, 0, ts->nthreads * sizeof(*ts->threads));

    for (i = 0; i < ts->nthreads; i++) {
        cma__threadInit(&ts->threads[i], ts);
    }

    return os_resultSuccess;

err_mutexinit:
    return os_resultFail;
}

void
cma_threadStatesDeinit(
    cma_threadStates ts)
{
    unsigned i;

    assert(ts);

    for (i = 0; i < ts->nthreads; i++) {
        assert(ts->threads[i].state != CMA_THREAD_STATE_ALIVE);
        cma__threadDeinit(&(ts->threads[i]));
    }

    os_mutexDestroy(&ts->lock);
    cma__freeCachelineAligned(ts->threads);

    ts->threads = NULL;
}

void
cma_threadStatesDealloc(
    cma_threadStates ts)
{
    assert(ts);

    os_free(ts);
}

static int
cma__threadStatesFindFreeSlot(
    cma_threadStates ts,
    const char *name)
{
    unsigned i;
    int idx;

    assert(ts);
    assert(name);
    assert(ts->nthreads < OS_MAX_INTEGER(int));

    for (i = 0, idx = -1; i < ts->nthreads; i++) {
        if (ts->threads[i].state != CMA_THREAD_STATE_ALIVE) {
            idx = (int)i;
            break;
        }
    }

    if (idx == -1) {
        OS_REPORT(OS_FATAL, "cma__threadStatesFindFreeSlot", idx,
            "Unable to find free slot for thread '%s'",
            name);
    }

    return idx;
}

cma_thread
cma_threadLookup(void)
{
    cma_thread *self;
#if OS_HAS_TSD_USING_THREAD_KEYWORD
    self = &cma_threadSelf;
#else
    self = os_threadMemGet(OS_THREAD_STATE);
#endif /* OS_HAS_TSD_USING_THREAD_KEYWORD */

    return self ? *self : NULL;
}

cma_logConfig
cma_threadLogConfig(
    cma_thread _this)
{
    cma_objectIsValidKind(_this, CMA_OBJECT_THREAD);

    return _this->ts->gc;
}

const char*
cma_threadName(
    cma_thread _this)
{
    cma_objectIsValidKind(_this, CMA_OBJECT_THREAD);

    return _this->name;
}

cma_logbuf
cma_threadLogbuf(
    cma_thread _this)
{
    cma_objectIsValidKind(_this, CMA_OBJECT_THREAD);

    return _this->logbuf;
}

static cma_threadStates
cma__threadStatesLookup(void)
{
    cma_thread self = cma_threadLookup();

    /* This should only be invoked from threads started as cma_thread */
    cma_objectIsValidKind(self, CMA_OBJECT_THREAD);
    if (!self) abort();

    return self->ts;
}

static void
cma__threadReapState(
    cma_thread _this,
    c_bool sync_lease)
{
    cma_objectIsValidKind(_this, CMA_OBJECT_THREAD);

    OS_UNUSED_ARG(sync_lease);

    os_mutexLock(&_this->ts->lock);
    _this->state = CMA_THREAD_STATE_ZERO;

    os_free(_this->name);
    _this->name = NULL;

    cma_logbufFree(_this->logbuf, _this);
    _this->logbuf = NULL;

    os_mutexUnlock(&_this->ts->lock);
}

void
cma_threadUpgrade(
    cma_threadStates ts)
{
    int idx;
    cma_thread _this;

    assert(ts);

    os_mutexLock(&ts->lock);

    idx = cma__threadStatesFindFreeSlot(ts, "main");
    /* There should always be a free slot, since new threads should only be started after main-thread is upgraded */
    assert(idx >= 0);

    _this = &ts->threads[idx];

#if OS_HAS_TSD_USING_THREAD_KEYWORD
    assert(cma_threadSelf == NULL);
    cma_threadSelf = _this;
#else
    {
        cma_thread *self;
        assert(os_threadMemGet(OS_THREAD_STATE) == NULL);
        self = os_threadMemMalloc(OS_THREAD_STATE, sizeof(*self), NULL, NULL);
        *self = _this;
    }
#endif /* OS_HAS_TSD_USING_THREAD_KEYWORD */

    _this->state = CMA_THREAD_STATE_ALIVE;
    _this->tid = os_threadIdSelf();
    _this->logbuf = cma_logbufNew();
    _this->name = os_strdup("main");

    os_mutexUnlock(&ts->lock);
}

void
cma_threadDowngrade(
    void)
{
    cma_thread _this = cma_threadLookup();

    cma__threadReapState(_this, FALSE);

#if OS_HAS_TSD_USING_THREAD_KEYWORD
    cma_threadSelf = NULL;
#else
    os_threadMemFree(OS_THREAD_STATE);
#endif /* OS_HAS_TSD_USING_THREAD_KEYWORD */
}

os_result
cma_threadJoin(
    cma_thread _this,
    void **retval)
{
    os_result result;

    cma_objectIsValidKind(_this, CMA_OBJECT_THREAD);
    assert(_this->state == CMA_THREAD_STATE_ALIVE);

    CMA_TRACE(("Joining thread '%s'...", _this->name));
    result = os_threadWaitExit(_this->extTid, retval);
    CMA_TRACE((" %s\n", os_resultImage(result)));
    cma__threadReapState(_this, FALSE);

    return result;
}

static void*
cma__threadStartRoutine(
    struct cma_threadContext *context)
{
    void *rc;
    cma_thread *self;

    assert(context);

#if OS_HAS_TSD_USING_THREAD_KEYWORD
    self = &cma_threadSelf;
#else
    assert(os_threadMemGet(OS_THREAD_STATE) == NULL);
    self = os_threadMemMalloc(OS_THREAD_STATE, sizeof(*self), NULL, NULL);
#endif /* OS_HAS_TSD_USING_THREAD_KEYWORD */

    *self = context->self;
    context->self->tid = os_threadIdSelf();
    rc = context->func(context->arg);
    os_free(context);
    return rc;
}

os_result
cma_threadCreate(
    const char *name,
    cma_thread *thread,
    const cma_threadRoutine func,
    void *arg)
{
    int idx;
    os_threadAttr tattr;
    os_threadId tid;
    os_result result;
    struct cma_threadContext *context;
    cma_threadStates ts = cma__threadStatesLookup();

    assert(ts);
    assert(name);
    assert(thread);
    assert(func);

    context = os_malloc(sizeof(*context));

    os_mutexLock(&ts->lock);

    if ((idx = cma__threadStatesFindFreeSlot(ts, name)) < 0) {
        /* error reported by cma__threadStatesFindFreeSlot */
        goto err_nofreeslot;
    }

    *thread = &(ts->threads[idx]);

    context->self = *thread;
    context->func = func;
    context->arg = arg;

    if (((*thread)->name = os_strdup(name)) == NULL) {
        CMA_ERROR("cma_threadCreate", "Out of memory while creating thread '%s'\n",
            name);
        goto err_strdup;
    }

    if (((*thread)->logbuf = cma_logbufNew()) == NULL) {
        /* error reported by cma_logbufNew */
        goto err_logbufnew;
    }

    os_threadAttrInit(&tattr);
    (*thread)->state = CMA_THREAD_STATE_ALIVE;

    result = u_serviceThreadCreate(&tid, name, &tattr, (os_threadRoutine)cma__threadStartRoutine, context);
    if (result != os_resultSuccess) {
        (*thread)->state = CMA_THREAD_STATE_ZERO;
        CMA_ERROR("cma_threadCreate", "Failed to create thread '%s': os_threadCreate result %s\n",
            name, os_resultImage(result));
        goto err_thrcreate;
    }

    CMA_TRACE(("Created new thread '%s' (%lu, at index %d)\n",
        name, os_threadIdToInteger(tid), idx));
    (*thread)->extTid = tid;
    os_mutexUnlock(&ts->lock);
    return os_resultSuccess;

err_thrcreate:
    cma_logbufFree((*thread)->logbuf, *thread);
err_logbufnew:
    os_free((*thread)->name);
err_strdup:
err_nofreeslot:
    os_mutexUnlock(&ts->lock);
    os_free(context);
    *thread = NULL;
    return os_resultFail;
}
