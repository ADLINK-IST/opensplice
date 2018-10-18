/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#include "nb__thread.h"
#include "nb__util.h"
#include "nb__log.h"
#include "nb__configuration.h"

#include "vortex_os.h"
#include "os_report.h"
#include "os_thread.h"
#include "os_mutex.h"
#include "u_service.h"

struct nb_threadContext {
    nb_thread self;
    nb_threadRoutine func;
    void *arg;
};

/* Things don't go wrong if CACHE_LINE_SIZE is defined incorrectly,
   they just run slower because of false cache-line sharing. It can be
   discovered at run-time, but in practice it's 64 for most CPUs and
   128 for some. */
#define CACHE_LINE_SIZE 64

typedef os_uint32 nb_vtime;
typedef os_int32  nb_svtime; /* signed version */

enum nb_threadState {
    NB_THREAD_STATE_ZERO,
    NB_THREAD_STATE_ALIVE
};

/*
 * watchdog indicates progress for the service lease liveliness mechsanism, while vtime
 * indicates progress for the Garbage collection purposes.
 *  vtime even : thread awake
 *  vtime odd  : thread asleep
 */
#define THREAD_BASE                     \
    nb_threadStates ts;                 \
    volatile nb_vtime vtime;            \
    volatile nb_vtime watchdog;         \
    os_threadId tid;                    \
    os_threadId extTid;                 \
    enum nb_threadState state;          \
    nb_logbuf logbuf;                   \
    char *name /* note: no semicolon! */

struct nb_threadStateBase {
    THREAD_BASE;
};

C_STRUCT(nb_thread) {
    C_EXTENDS(nb_object);
    THREAD_BASE;
    char pad[CACHE_LINE_SIZE
             * ((sizeof (struct nb_threadStateBase) + CACHE_LINE_SIZE - 1)
                 / CACHE_LINE_SIZE)
             - sizeof (struct nb_threadStateBase)];
};

#undef THREAD_BASE

C_STRUCT(nb_threadStates) {
    os_mutex lock;
    nb_logConfig gc;
    unsigned nthreads;
    nb_thread threads; /* [nthreads] */
};

#if OS_HAS_TSD_USING_THREAD_KEYWORD
__thread nb_thread nb_threadSelf;
#endif

static void             nb__threadReapState(nb_thread thread,
                                            c_bool sync_lease) __nonnull_all__;

static void*            nb__mallocCachelineAligned(os_size_t size)
                                            __attribute_malloc__
                                            __attribute_returns_nonnull__
                                            __attribute_assume_aligned__((CACHE_LINE_SIZE));

static void             nb__freeCachelineAligned(void *ptr) __nonnull_all__;

static void             nb_threadInit(nb_thread thread,
                                      nb_threadStates ts) __nonnull_all__;

/* Helper function with nb_objectDeinitFunc signature */
static void             nb__threadDeinitFunc(nb_object _this) __nonnull_all__;

static void             nb_threadDeinit(nb_thread _this) __nonnull_all__;

static int              nb_threadStatesFindFreeSlot(nb_threadStates ts,
                                                    const char *name) __nonnull_all__;

static nb_threadStates  nb_threadStatesLookup(void) __attribute_returns_nonnull__;

static void*
nb__mallocCachelineAligned(
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

    ptr = os_malloc (size + CACHE_LINE_SIZE + sizeof (void *));
    ptrA = ((os_address) ptr + sizeof (void *) + clm1) & ~clm1;
    pptr = (void **) ptrA;
    pptr[-1] = ptr;
    return (void *) ptrA;
}

static void
nb__freeCachelineAligned(
    void *ptr)
{
    void **pptr = ptr;
    os_free (pptr[-1]);
}

#if 0
static int
nb__vtime_asleep_p(
    nb_vtime vtime)
{
    return (vtime % 2) == 1;
}
#endif

static void
nb__threadDeinitFunc(
        nb_object _this)
{
    nb_objectIsValidKind(_this, NB_OBJECT_THREAD);

    nb_threadDeinit(nb_thread(_this));
}


static void
nb_threadInit(
        nb_thread thread,
        nb_threadStates ts)
{
    assert(thread);
    assert(ts);

    /* Note: memory for all threads is allocated by nb_threadStatesInit in one alloc, aligned to cache-line size, re-usable */
    nb__objectInit(nb_object(thread), NB_OBJECT_THREAD, nb__threadDeinitFunc);
    thread->ts = ts;
    thread->state = NB_THREAD_STATE_ZERO;
    thread->vtime = 1;
    thread->watchdog = 1;
    thread->logbuf = NULL;
    thread->name = NULL;
}

static void
nb_threadDeinit(
        nb_thread _this)
{
    nb_objectIsValidKind(_this, NB_OBJECT_THREAD);

    /* nb_thread objects are reused during service lifespan and all threads must be joined at service exit
     * Memory for nb_thread objects is allocated/deallocated once for all threads (see nb_threadStatesInit/Deinit)
     * Pointers in nb_thread objects are freed when a thread is joined, so the object can be reused for new threads
     * nb_objectFree must be used (as that frees memory of individual objects)
     */
    nb__objectDeinit(nb_object(_this));
}

nb_threadStates
nb_threadStatesAlloc(void)
{
    nb_threadStates _this = os_malloc(sizeof *_this);

    return _this;
}

os_result
nb_threadStatesInit(
        nb_threadStates _this,
        unsigned maxthreads,
        nb_logConfig gc)
{
    unsigned i;

    assert(_this);

    if (os_mutexInit(&_this->lock, NULL) != os_resultSuccess) {
        goto err_mutexinit;
    }

    _this->gc = gc;
    _this->nthreads = maxthreads;
    _this->threads = nb__mallocCachelineAligned(maxthreads * sizeof(*_this->threads));

    memset(_this->threads, 0, maxthreads * sizeof(*_this->threads));
    for (i = 0; i < _this->nthreads; i++) {
        nb_threadInit(&_this->threads[i], _this);
    }

    return os_resultSuccess;

/* Error handling */
err_mutexinit:
    return os_resultFail;
}

void
nb_threadStatesDeinit(
        nb_threadStates _this)
{
    unsigned i;

    assert(_this);

    for (i = 0; i < _this->nthreads; i++) {
        assert(_this->threads[i].state != NB_THREAD_STATE_ALIVE);
        nb_threadDeinit(&(_this->threads[i]));
    }

    os_mutexDestroy(&_this->lock);
    nb__freeCachelineAligned(_this->threads);

    _this->threads = NULL;
}

void
nb_threadStatesDealloc(
        nb_threadStates ts)
{
    assert(ts);

    os_free(ts);
}

#if 0
static int
nb_threadEqual(
    os_threadId a,
    os_threadId b)
{
    os_ulong_int ai = os_threadIdToInteger(a);
    os_ulong_int bi = os_threadIdToInteger(b);
    return ai == bi;
}
#endif

static int
nb_threadStatesFindFreeSlot(
        nb_threadStates _this,
        const char *name)
{
    unsigned i;
    int idx;

    assert(_this);
    assert(name);
    assert(_this->nthreads < OS_MAX_INTEGER(int));

    for (i = 0, idx = -1; i < _this->nthreads; i++) {
        if (_this->threads[i].state != NB_THREAD_STATE_ALIVE) {
            idx = (int) i;
        }
        if (_this->threads[i].state == NB_THREAD_STATE_ZERO) {
            break;
        }
    }

    if (idx == -1) {
        OS_REPORT(OS_FATAL, "nb__find_free_slot", idx,
            "Unable to find free slot for thread '%s'",
            name);
    }

    return idx;
}


nb_thread
nb_threadLookup (void)
{
    nb_thread *self;
#if OS_HAS_TSD_USING_THREAD_KEYWORD
    self = &nb_threadSelf;
#else
    self = os_threadMemGet(OS_THREAD_STATE);
#endif

    return self ? *self : NULL; /* May be NULL for non-NB thread (e.g., listeners) */
}

nb_logConfig
nb_threadLogConfig(
        nb_thread _this)
{
    nb_objectIsValidKind(_this, NB_OBJECT_THREAD);

    return _this->ts->gc;
}

const char *
nb_threadName(
        nb_thread _this)
{
    nb_objectIsValidKind(_this, NB_OBJECT_THREAD);

    return _this->name;
}

nb_logbuf
nb_threadLogbuf(
        nb_thread _this)
{
    nb_objectIsValidKind(_this, NB_OBJECT_THREAD);

    return _this->logbuf;
}

static nb_threadStates
nb_threadStatesLookup (void)
{
    nb_thread self = nb_threadLookup();

    /* This call should only be invoked from threads started as nb_thread.
     * Listener-threads should never touch code that invokes this. */
    nb_objectIsValidKind(self, NB_OBJECT_THREAD);

    if(!self){
        abort();
    }

    return self->ts;
}

static void
nb__threadReapState(
        nb_thread thread,
        c_bool sync_lease)
{
    nb_objectIsValidKind(thread, NB_OBJECT_THREAD);

    OS_UNUSED_ARG(sync_lease);

    os_mutexLock(&thread->ts->lock);
    thread->state = NB_THREAD_STATE_ZERO;

    os_free(thread->name);
    thread->name = NULL;

    nb_logbufFree(thread, thread->logbuf);
    thread->logbuf = NULL;

    os_mutexUnlock(&thread->ts->lock);
}

void
nb_threadUpgrade(
        nb_threadStates ts)
{
    int idx;
    nb_thread thread;

    assert(ts);

    os_mutexLock(&ts->lock);

    idx = nb_threadStatesFindFreeSlot(ts, "main");

    /* There should be free indices, since threads should only be started
     * after the main-thread is upgraded. */
    assert(idx >= 0);

    thread = &ts->threads[idx];

#if OS_HAS_TSD_USING_THREAD_KEYWORD
    assert(nb_threadSelf == NULL);
    nb_threadSelf = thread;
#else
    {
        nb_thread *self;
        assert(os_threadMemGet(OS_THREAD_STATE) == NULL);
        self = os_threadMemMalloc(OS_THREAD_STATE, sizeof(*self), NULL, NULL);
        *self = thread;
    }
#endif

    thread->state = NB_THREAD_STATE_ALIVE;
    thread->tid = os_threadIdSelf();
    thread->logbuf = nb_logbufNew();
    thread->name = os_strdup("main");

    os_mutexUnlock(&ts->lock);
}

void
nb_threadDowngrade(
        void)
{
    nb_thread self = nb_threadLookup();

    nb__threadReapState(self, FALSE);

#if !OS_HAS_TSD_USING_THREAD_KEYWORD
    os_threadMemFree(OS_THREAD_STATE);
#else
    nb_threadSelf = NULL;
#endif
}

os_result
nb_threadJoin(
    nb_thread thread,
    void **retval)
{
    os_result result;

    nb_objectIsValidKind(thread, NB_OBJECT_THREAD);

    assert(thread->state == NB_THREAD_STATE_ALIVE);
    NB_TRACE(("Joining thread '%s'...", thread->name));
    result = os_threadWaitExit(thread->extTid, retval);
    NB_TRACE((" %s\n", os_resultImage(result)));
    nb__threadReapState(thread, FALSE);
    return result;
}

static void*
nb__threadStartRoutine(
    struct nb_threadContext *context)
{
    void *rc;
    nb_thread *self;

    assert(context);

#if OS_HAS_TSD_USING_THREAD_KEYWORD
    self = &nb_threadSelf;
#else
    assert(os_threadMemGet(OS_THREAD_STATE) == NULL);
    self = os_threadMemMalloc(OS_THREAD_STATE, sizeof(*self), NULL, NULL);
#endif

    *self = context->self;

    context->self->tid = os_threadIdSelf();
    rc = context->func(context->arg);
    os_free(context);
    return rc;
}

os_result
nb_threadCreate(
        const char *name,
        nb_thread *thread,
        const nb_threadRoutine func,
        void *arg)
{
    int idx;
    os_threadAttr tattr;
    os_threadId tid;
    os_result result;
    struct nb_threadContext *context;
    nb_threadStates ts = nb_threadStatesLookup();

    assert(ts);
    assert(name);
    assert(thread);
    assert(func);

    context = os_malloc(sizeof *context);

    os_mutexLock(&ts->lock);

    if ((idx = nb_threadStatesFindFreeSlot(ts, name)) < 0) {
        /* Error reported by nb__find_free_slot */
        goto err_nofreeslot;
    }

    *thread = &(ts->threads[idx]);

    context->self = *thread;
    context->func = func;
    context->arg = arg;

    if (((*thread)->name = os_strdup(name)) == NULL) {
        NB_ERROR_1("nb_threadCreate", "Out of memory while creating thread '%s'\n", name);
        goto err_strdup;
    }

    if (((*thread)->logbuf = nb_logbufNew()) == NULL) {
        /* Error reported by nb_logbufNew */
        goto err_logbufcreate;
    }

    os_threadAttrInit(&tattr);
    (*thread)->state = NB_THREAD_STATE_ALIVE;

    result = u_serviceThreadCreate(&tid, name, &tattr, (nb_threadRoutine)nb__threadStartRoutine, context);
    if (result != os_resultSuccess) {
        (*thread)->state = NB_THREAD_STATE_ZERO;
        NB_ERROR_2("nb_threadCreate", "Failed to create thread '%s': os_threadCreate returned %s\n",
            name, os_resultImage(result));
        goto err_thrcreate;
    }

    NB_TRACE(("Created new thread '%s' (%lu, at index %d)\n", name, os_threadIdToInteger(tid), idx));
    (*thread)->extTid = tid;
    os_mutexUnlock(&ts->lock);
    return os_resultSuccess;

/* Error handling */
err_thrcreate:
    nb_logbufFree(*thread, (*thread)->logbuf);
err_logbufcreate:
    os_free((*thread)->name);
err_strdup:
err_nofreeslot:
    os_mutexUnlock(&ts->lock);
    os_free(context);
    *thread = NULL;
    return os_resultFail;
}
