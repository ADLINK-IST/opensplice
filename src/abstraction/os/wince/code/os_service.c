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
#include "code/os__service.h"
#include "os_time.h"
#include <time.h>
#include "os_heap.h"
#include "os_stdlib.h"
#include "os_mutex.h"
#include "os_cond.h"
#include "os_errno.h"

#include <stdio.h>
#include <assert.h>

#include "os__debug.h"

#define _MSGQ_MAX_INSTANCES      8

#define _POOL_BLOCKSIZE 128
#define OS_SERVICE_DEFAULT_NAME "osplOSService"
#define OS_SERVICE_DEFAULT_PIPE_NAME OS_SERVICE_DEFAULT_NAME
#define OS_SERVICE_MSGQUEUE_PREFIX "osplMessageQueue"

/* Setting to 0 directly iso using OS_TIMEM_ZERO as the compiler returns
 * 'error C2099: initializer is not a constant' when using OS_TIMEM_ZERO
 */
static os_timeM _ospl_clock_starttime = {OS_TIME_ZERO};
static LONGLONG _ospl_clock_freq = 0; /* frequency of high performance counter */
static LONGLONG _ospl_clock_offset = 0;
static HANDLE hReadMsgQueue;

struct _msgq_instance {
    HANDLE hReadMsgQInst;
    struct os_servicemsg request;
    DWORD reqRead;
    struct os_servicemsg reply;
    BOOL fPendingIO;
};

struct pool_entity {
    long id; /* when negative, the mutex is already in use! */
    HANDLE h;
    long lifecycleId;
    char name[OS_SERVICE_ENTITY_NAME_MAX];
};

/* the blocks are linked in reverse order for efficiency */
struct pool_block {
    struct pool_entity entity[_POOL_BLOCKSIZE];
    struct pool_block *prev;
};

struct pool {
    long blockCount;
    long inuse; /* total number of events in use */
    struct pool_block *tail;
};

struct eventService {
    struct pool eventPool;
    struct pool semPool;
};

static HANDLE _ospl_serviceThreadId = 0;
static char *_ospl_servicePipeName = OS_SERVICE_DEFAULT_PIPE_NAME;
static char *_ospl_serviceName = OS_SERVICE_DEFAULT_NAME;
static os_boolean _ospl_singleProcess = OS_FALSE;
static os_mutex messageQueueMutex;
static long privateMessageQueueId;
static os_mutex privateProcessMutex;

/* Generic pool functions */
static void
poolInit(
    struct pool *pool)
{
    pool->blockCount = 0;
    pool->tail = NULL;
    pool->inuse = 0;
}

static void
poolDeinit(
    struct pool *pool)
{
    int i;
    struct pool_block *block;
    struct pool_block *freeBlock;

    block = pool->tail;
    while (block != NULL) {
        for (i = 0; i < _POOL_BLOCKSIZE; i++) {
            CloseHandle(block->entity[i].h);
        }
        freeBlock = block;
        block = block->prev;
        free(freeBlock);
    }
    pool->blockCount = 0;
    pool->inuse = 0;
    pool->tail = NULL;
}

static HANDLE
createSem(
    long id,
    char *name)
{
    wchar_t* wStringName;
    HANDLE handle;

    _snprintf(name, OS_SERVICE_ENTITY_NAME_MAX,
              "%s%d%d", OS_SERVICE_SEM_NAME_PREFIX, id, os_getShmBaseAddressFromPointer(NULL));
    wStringName = wce_mbtowc(name);
    handle = CreateSemaphore(NULL, 0, 0x7fffffff, wStringName);
    os_free (wStringName);

    return handle;
}

static HANDLE
createEv(
    long id,
    char *name)
{
    wchar_t* wStringName;
    HANDLE handle;

    _snprintf(name, OS_SERVICE_ENTITY_NAME_MAX,
              "%s%d%d", OS_SERVICE_EVENT_NAME_PREFIX, id, os_getShmBaseAddressFromPointer(NULL));
    wStringName = wce_mbtowc(name);
    handle = CreateEvent(NULL, FALSE, FALSE, wStringName);
    os_free (wStringName);

    return handle;
}

static int
poolClaim(
    struct pool *pool,
    HANDLE (*create)(long id, char *name),
    long *id,
    long *lifecycleId)
{
    struct pool_block *block;
    struct pool_block *newBlock;
    int i;
    long max;
    int result;

    block = pool->tail;
    max = pool->blockCount*_POOL_BLOCKSIZE;
    if (max == pool->inuse) {
        /* first allocated new block */
        newBlock = (struct pool_block*)malloc(sizeof(struct pool_block));
        if (!newBlock) {
            return 1; /* failed to allocate new block */
        }
        newBlock->prev = NULL;
        for (i = 0; i < _POOL_BLOCKSIZE; i++) {
            newBlock->entity[i].id = max + i + 1; /* at least one */
            newBlock->entity[i].lifecycleId = 0;
            newBlock->entity[i].h = create(newBlock->entity[i].id,
                                           newBlock->entity[i].name);
        }
        newBlock->prev = pool->tail;
        pool->tail = newBlock;
        pool->blockCount++;
    }
    /* find a free entity */
    result = 1;
    block = pool->tail;
    while (block) {
        i = 0;
        while (i < _POOL_BLOCKSIZE) {
            if (block->entity[i].id > 0) {
                *id = block->entity[i].id;
                *lifecycleId = block->entity[i].lifecycleId;
                block->entity[i].id = -block->entity[i].id;
                i = _POOL_BLOCKSIZE;
                block = NULL;
                pool->inuse++;
                result = 0; /* success */
            }
            i++;
        }
        if (block) {
            block = block->prev;
        }
    }
    return result;
}

static int
poolRelease(
    struct pool *pool,
    long id)
{
    struct pool_block *block;
    long blockNr;
    long idxInBlock;
    long i;
    int result;

    block = pool->tail;
    blockNr = (pool->blockCount - 1) - ((id - 1) / _POOL_BLOCKSIZE); /* reverse order */
    idxInBlock = (id - 1) % _POOL_BLOCKSIZE;
    for (i = 0; i < blockNr; i++) {
        block = block->prev;
    }
    if (block->entity[idxInBlock].id < 0) {
        OS_DEBUG_1("poolRelease", "Releasing event %d", id);
        block->entity[idxInBlock].id = -block->entity[idxInBlock].id;
        /* increment the lifecycleId to indicate that this entity represents a new cond/mutex */
        block->entity[idxInBlock].lifecycleId = block->entity[idxInBlock].lifecycleId + 1;
        pool->inuse--;
        result = 0; /* success */
    } else {
        OS_DEBUG_1("poolRelease", "Trying to destroy incorrect mutex %d", id);
        result = 1;
    }

    return result;
}

static void
handleRequest(
    struct eventService *es,
    struct _msgq_instance *msgq,
    int *terminate)
{
    struct pool *pool;

    /* read request */
    msgq->reply.kind = msgq->request.kind;
    switch (msgq->request.kind) {
    case OS_SRVMSG_CREATE_EVENT:
        pool = &es->eventPool;
        if (poolClaim(pool, createEv, &msgq->reply._u.id, &msgq->reply.lifecycleId) == 0) {
            msgq->reply.result = os_resultSuccess;
        } else {
            msgq->reply.result = os_resultFail;
        }
    break;
    case OS_SRVMSG_DESTROY_EVENT:
        pool = &es->eventPool;
        if (poolRelease(pool, msgq->request._u.id) == 0) {
            msgq->reply.result = os_resultSuccess;
        } else {
            msgq->reply.result = os_resultFail;
        }
    break;
    case OS_SRVMSG_CREATE_SEMAPHORE:
        pool = &es->semPool;
        if (poolClaim(pool, createSem, &msgq->reply._u.id, &msgq->reply.lifecycleId) == 0) {
            msgq->reply.result = os_resultSuccess;
        } else {
            msgq->reply.result = os_resultFail;
        }
    break;
    case OS_SRVMSG_DESTROY_SEMAPHORE:
        pool = &es->semPool;
        if (poolRelease(pool, msgq->request._u.id) == 0) {
            msgq->reply.result = os_resultSuccess;
        } else {
            msgq->reply.result = os_resultFail;
        }
    break;
    case OS_SRVMSG_GET_TIME:
        msgq->reply._u.time.start_time = _ospl_clock_starttime;
        msgq->reply._u.time.time_offset = _ospl_clock_offset;
        msgq->reply.result = os_resultSuccess;
    break;
    case OS_SRVMSG_TERMINATE:
        msgq->reply.result = os_resultSuccess;
        *terminate = 1;
    break;
    case OS_SRVMSG_UNDEFINED:
    case OS_SRVMSG_COUNT:
    default:
        msgq->reply.result = os_resultFail;
//        OS_DEBUG_1("handleRequest", "Incorrect msg kind in request %d", pipe->request.kind);
    }
}

static void *
osServiceThread(
    void *arg)
{
    struct _msgq_instance msgq[_MSGQ_MAX_INSTANCES];
    MSGQUEUEOPTIONS msgqOptionsWrite, msgqOptionsRead;
    HANDLE hEvents[_MSGQ_MAX_INSTANCES];
    BOOL fSuccess;
    DWORD i;
    DWORD dwWait;
    DWORD cbRet;
    struct eventService es;
    int terminate;
    HANDLE namedMessageQueue;
    char messageQueueName[256];
    wchar_t* wStringName;

    /* only use 'ready' to indicate initialisation is done. Then
     * never use this variable again as it is a stack variable of
     *  the os_serviceStart() routine
     */
    HANDLE *initializedEvent = (HANDLE *)arg;

    terminate = 0;
    poolInit(&es.eventPool);
    poolInit(&es.semPool);

    // write message queue options
    msgqOptionsWrite.dwFlags = MSGQUEUE_ALLOW_BROKEN;
    msgqOptionsWrite.dwMaxMessages = 0;
    msgqOptionsWrite.cbMaxMessage = sizeof(struct os_servicemsg);
    msgqOptionsWrite.bReadAccess = FALSE;
    msgqOptionsWrite.dwSize = sizeof(msgqOptionsWrite);
    // Read message queue options
    msgqOptionsRead.dwFlags = MSGQUEUE_ALLOW_BROKEN;
    msgqOptionsRead.dwMaxMessages = 0;
    msgqOptionsRead.cbMaxMessage = sizeof(struct os_servicemsg);
    msgqOptionsRead.bReadAccess = TRUE;
    msgqOptionsRead.dwSize = sizeof(msgqOptionsRead);

    wStringName = wce_mbtowc(_ospl_servicePipeName);

    for (i = 0; i < _MSGQ_MAX_INSTANCES; i++)
    {
        hEvents[i] = CreateEvent(
           NULL,    // default security attribute
           FALSE,   // auto-reset event
           FALSE,   // initial state = unsignaled
           NULL);   // unnamed event object

        // Create Read message queue
        msgq[i].hReadMsgQInst = CreateMsgQueue(wStringName, &msgqOptionsRead);
        // Write message queue is created dynamically below

        if (msgq[i].hReadMsgQInst == INVALID_HANDLE_VALUE)
        {
            OS_DEBUG_2("osServiceThread", "Failed to create message queue %s %d",
                       _ospl_servicePipeName, os_getErrno());
        }
        // Add handles to event array
        hEvents[i] = msgq[i].hReadMsgQInst;

        msgq[i].reply.result = os_resultFail;
        msgq[i].reply.kind = OS_SRVMSG_UNDEFINED;
        msgq[i].reply._u.id = -1;

        msgq[i].reqRead = 0;
        msgq[i].request.result = os_resultFail;
        msgq[i].request.kind = OS_SRVMSG_UNDEFINED;
        msgq[i].request._u.id = -1;
    }

    os_free(wStringName);

    /* We are done initialising, notify thread that started this service and then never use this
     * handle again!
     */
    SetEvent(*initializedEvent);
    initializedEvent = NULL;
    while (!terminate)
    {
        /* Wait for the event object to be signaled, indicating
         * completion of an overlapped read, write, or connect
         * operation.
         */

        dwWait = WaitForMultipleObjects(
                     _MSGQ_MAX_INSTANCES,    // number of event objects
                     hEvents,                // array of event objects
                     FALSE,                  // do not wait for all
                     INFINITE);              // wait for ever

        if(dwWait == WAIT_FAILED)
        {
            // TODO: error handling
            continue;
        }

        /* dwWait shows which event completed the operation */
        i = dwWait - WAIT_OBJECT_0;

        assert(i >= 0 || i < _MSGQ_MAX_INSTANCES);

        /* lock the centralised message queue at the point a message is seen */
        os_mutexLock(&messageQueueMutex);

        fSuccess = ReadMsgQueue(msgq[i].hReadMsgQInst,
                                &msgq[i].request,
                                sizeof(msgq[i].request),
                                &msgq[i].reqRead,
                                INFINITE,
                                &cbRet);
        /* Check for success or failure */
        if (!fSuccess || (msgq[i].reqRead == 0))
        {
            ResetEvent(hEvents[i]);
            os_mutexUnlock(&messageQueueMutex);
            continue;
        }

        /* We extract the privateMessageQueueId from the centralised message queue.  This
         * id is the unique queue that the response needs to be written to because that
         * is where the response is expected.  This separation of message queues prevents
         * race conditions of a mutex call being done from two processes at the same time
         */

        /* next handle the request */
        handleRequest(&es, &msgq[i], &terminate);

        if (msgq[i].request.privateMessageQueueId != 0)
        {
            sprintf (messageQueueName, OS_SERVICE_MSGQUEUE_PREFIX"_%d", msgq[i].request.privateMessageQueueId);
        }
        else
        {
            OS_DEBUG_1("osServiceThread", "Failed to get privateMessageQueueId : %d", os_getErrno());
        }

        /* Create (in fact attach to) the pre-existing message queue  */
        wStringName = wce_mbtowc(messageQueueName);
        namedMessageQueue = CreateMsgQueue(wStringName, &msgqOptionsWrite);
        if (namedMessageQueue == INVALID_HANDLE_VALUE)
        {
            OS_DEBUG_2("osServiceThread", "Failed to create %s %d", messageQueueName, os_getErrno());
        }

        os_free(wStringName);

        /* And send the response */
        fSuccess = WriteMsgQueue(namedMessageQueue,
                                 &msgq[i].reply,
                                 sizeof(msgq[i].reply),
                                 INFINITE,
                                 0);

        CloseHandle (namedMessageQueue);

        /* unlock the centralised message queue, now the message is handled */
        os_mutexUnlock(&messageQueueMutex);

        /* Check for success or failure */
        if (!fSuccess)
        {
            OS_REPORT(OS_ERROR, "os_serviceStart", 0, "failed to write to %s : %d", msgq[i].request.privateMessageQueueId, os_getErrno());
            // TODO: error handling
            continue;
        }
    }

    for (i = 0; i < _MSGQ_MAX_INSTANCES; i++)
    {
        CloseHandle(hEvents[i]);
        CloseHandle(msgq[i].hReadMsgQInst);
    }

    poolDeinit(&es.eventPool);
    poolDeinit(&es.semPool);

    return NULL;
}

/* result must be free'd */
os_char *
os_createPipeNameFromMutex(
    os_mutex *mutex)
{
    const char *name;

    assert(mutex);

    name = os_getDomainNameforMutex(mutex);
    if (name == NULL) {
        name = _ospl_serviceName;
    }
    return os_constructPipeName(name);
}

/* result must be free'd */
os_char *
os_createPipeNameFromCond(os_cond *cond)
{
    const char *name;

    assert(cond);

    name = os_getDomainNameforCond(cond);
    if (name == NULL) {
        name = _ospl_serviceName;
    }
    return os_constructPipeName(name);
}

/* result must be free'ed */
os_char *
os_constructPipeName(
    const os_char * name)
{
    /* Windows CE uses message queues rather than pipes, so there is no need
     * need for a "/" prefix
     *
     * However it does need to remove all spaces in the name:
     */

    os_char *n;
    os_uint32 i, len;

    assert(name);

    n = os_malloc(strlen(name) + 1);
    if (n) {
        strcpy(n, name);
        /* replace all ' ' occurrences with '_', since spaces are not
         * allowed in the message queue name
         */
        len = strlen(n);
        for (i = 0; i < len; i++) {
            if (n[i] == ' ') {
                n[i] = '_';
            }
        }
    }
    return n;
}

void
os_createPipeNameFromDomainName(
    const os_char *name)
{
    assert(name);
    if (name == NULL) {
       name = _ospl_serviceName;
    }

    _ospl_servicePipeName = os_constructPipeName(name);
}

#define UNIQUE_PREFIX "ospl"
os_result
os_serviceStart(
    const char *name)
{
    os_result r;
    DWORD threadIdent;
    LARGE_INTEGER frequency;
    LARGE_INTEGER hpt;
    SYSTEMTIME    systemTime;
    FILETIME    systemTimeSince1601;
    DWORD64 dw64HighDateTime, dw64LowDateTime;
    DWORD64 dw64CurrentTime;
    DWORD64 dw64MAXDWORD;
    HANDLE initializedEvent;
    DWORD result;
    char uniqueName[16 + sizeof(UNIQUE_PREFIX)];

    r = os_resultSuccess;
    initializedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (QueryPerformanceFrequency(&frequency) != 0)
    {
        _ospl_clock_freq = frequency.QuadPart;

        GetLocalTime(&systemTime);
        SystemTimeToFileTime(&systemTime, &systemTimeSince1601);

        // Convert to 64 bit for calculations
        dw64HighDateTime = systemTimeSince1601.dwHighDateTime;
        dw64LowDateTime = systemTimeSince1601.dwLowDateTime;
        dw64MAXDWORD = MAXDWORD;
        dw64CurrentTime = (dw64HighDateTime * (dw64MAXDWORD+1)) + dw64LowDateTime;

        QueryPerformanceCounter(&hpt);
        _ospl_clock_offset = hpt.QuadPart;
        _ospl_clock_starttime = OS_TIMEM_INIT((dw64CurrentTime - EPOCH_DIFF)/UNITY_DIFF,
                                              (dw64CurrentTime - EPOCH_DIFF)%UNITY_DIFF * 100);
    }
    else
    {
        /* no high performance timer available!
         * so we fall back to a millisecond clock.
         */
        OS_REPORT(OS_WARNING, "os_serviceStart", 0,
                "No high-resolution timer found (reason: %s), "\
                "switching to millisecond resolution.", os_getErrno());
    }

    if (name == NULL)
    {
        _snprintf(uniqueName, sizeof(uniqueName), "%s%d", UNIQUE_PREFIX, GetCurrentProcessId());
        _ospl_serviceName = (char *)os_malloc(strlen(uniqueName) + 1);
        strcpy(_ospl_serviceName, uniqueName);
    }
    else
    {
        _ospl_serviceName = (char*)os_malloc(strlen(name) + 1);
        if (_ospl_serviceName)
        {
            strcpy(_ospl_serviceName, name);
        }
    }

    /* Create private mutex that locks the centralised message queue here */
    os_mutexInit(&messageQueueMutex, NULL);

    if (_ospl_serviceName)
    {
        _ospl_servicePipeName = os_constructPipeName(_ospl_serviceName);
        _ospl_serviceThreadId = CreateThread(NULL,
            (SIZE_T)128*1024,
            (LPTHREAD_START_ROUTINE)osServiceThread,
            (LPVOID)&initializedEvent,
            (DWORD)0, &threadIdent);
        if (_ospl_serviceThreadId == 0)
        {
            r = os_resultFail;
            os_free(_ospl_servicePipeName);
            _ospl_servicePipeName = OS_SERVICE_DEFAULT_PIPE_NAME;
        }
        else
        {
            /* Wait for thread to be done with intialisation */
            result = WaitForSingleObject(initializedEvent, INFINITE);
            assert(result == WAIT_OBJECT_0);
            if (result != WAIT_OBJECT_0)
            {
                r = os_resultFail;
            }
        }
    }
    else
    {
        r = os_resultFail;
    }

    return r;
}
#undef UNIQUE_PREFIX

os_result
os_serviceStop(void)
{
    struct os_servicemsg request;
    struct os_servicemsg reply;
    DWORD nRead;
    os_result osr;

    osr = os_resultSuccess;
    request.kind = OS_SRVMSG_TERMINATE;
    osr = requestResponseFromServiceQueue (&request, &reply, _ospl_servicePipeName);

    if (osr == os_resultSuccess)
    {
        if (GetExitCodeThread(_ospl_serviceThreadId, &nRead) == 0)
        {
            OS_DEBUG_1("serviceStop", "GetExitCodeThread Failed %d", os_getErrno());
            osr = os_resultFail;
        }
        else
        {
            while (nRead == STILL_ACTIVE)
            {
                Sleep(100);
                if (GetExitCodeThread(_ospl_serviceThreadId, &nRead) == 0)
                {
                    OS_DEBUG_1("serviceStop", "GetExitCodeThread Failed %d", os_getErrno());
                    osr = os_resultFail;
                    nRead = 0; /* break loop */
                }
            }
        }
    }
    else
    {
        osr = os_resultFail;
    }

    os_mutexDestroy (&messageQueueMutex);

    os_free(_ospl_servicePipeName);
   _ospl_servicePipeName = OS_SERVICE_DEFAULT_PIPE_NAME;

    return osr;
}

const char *
os_serviceName(void)
{
    return _ospl_serviceName;
}

void
os_serviceInitialiseMsgQueueData (void)
{
    MSGQUEUEOPTIONS msgqOptionsRead;
    wchar_t* wStringName;
    DWORD bufferSize = sizeof(struct os_servicemsg);
    char privateMessageQueueName[256];

    /* Initialise the private mutex used for locking access to the message queues
     * that are owned by this process */
    os_mutexInit(&privateProcessMutex, NULL);

    /* Set the unique identifier of this process's message queue */
    privateMessageQueueId = os_procIdSelf();
    snprintf(privateMessageQueueName, 256, OS_SERVICE_MSGQUEUE_PREFIX"_%d", privateMessageQueueId);

    /* Read message queue options */
    msgqOptionsRead.dwFlags = MSGQUEUE_ALLOW_BROKEN;
    msgqOptionsRead.dwMaxMessages = 0;
    msgqOptionsRead.cbMaxMessage = bufferSize;
    msgqOptionsRead.bReadAccess = TRUE;
    msgqOptionsRead.dwSize = sizeof(msgqOptionsRead);

    /* Create the process's private message queue.  This can be done here, in advance of its
     * use, because there is only one private read message queue for each process and that is
     * based on the process id
     */
    wStringName = wce_mbtowc(privateMessageQueueName);
    hReadMsgQueue = CreateMsgQueue(wStringName, &msgqOptionsRead);
    if (hReadMsgQueue == INVALID_HANDLE_VALUE)
    {
        OS_DEBUG_2("requestResponseFromServiceQueue", "Failed to create %s %d",
                   privateMessageQueueName, os_getErrno());
        os_free(wStringName);
        return;
    }
    if (os_getErrno() == ERROR_ALREADY_EXISTS)
    {
        OS_REPORT(OS_WARNING, "os_serviceInitialiseMsgQueueData", 0, "already existed : %s", privateMessageQueueName);
    }
    os_free(wStringName);
}

void
os_serviceFinaliseMsgQueueData (void)
{
    /* Release the resources that were created during os_serviceInitialiseMsgQueueData */

    CloseHandle(hReadMsgQueue);
    os_mutexDestroy(&privateProcessMutex);
}

long
os_serviceGetProcessId(void)
{
    return privateMessageQueueId;
}

os_mutex *
os_serviceGetPrivateMutex(void)
{
    return &privateProcessMutex;
}

void
os_serviceSetSingleProcess (void)
{
    _ospl_singleProcess = OS_TRUE;
    /* It is possible (depending on environment variables for instance) that the
     * os_reportInit() has to do slightly different things when it is initialized
     * by a single process setup. So, do it again in that case. */
    os_reportInit(OS_TRUE);
}

os_boolean
os_serviceGetSingleProcess (void)
{
    return _ospl_singleProcess;
}

os_result
requestResponseFromServiceQueue
(
    struct os_servicemsg * request,
    struct os_servicemsg * reply,
    char * writeMsgQueueName
)
{
    MSGQUEUEOPTIONS msgqOptionsWrite;
    HANDLE hWriteMsgQueue;
    BOOL result;
    DWORD lastError;
    DWORD cbRet;
    DWORD nRead;
    DWORD bufferSize = sizeof(struct os_servicemsg);
    os_result osr = os_resultSuccess;
    DWORD wait;
    wchar_t* wStringName;

    /* The algorithm is as follows :
     * The write message queue is the centralised message queue.  We write a
     * message to this containing the process id of the caller.  We then wait
     * on the read message queue which is named after this, the calling process.
     * The osServiceThread function reads from the centralised message queue and
     * writes its response (i.e. the mutex, or cond, etc response) to the message
     * queue named in the message.  The read message queue will then receive
     * that message and act appropriately.
     *
     * Note that the read message queue is based on our process id so the queue
     * can be created ahead of time in os_serviceInitialiseMsgQueueData.
     * The writer message queue cannot however, since its name is based on the
     * DDS Domain in which the mutex/cond is requested
     */

    /* write message queue options */
    msgqOptionsWrite.dwFlags = MSGQUEUE_ALLOW_BROKEN;
    msgqOptionsWrite.dwMaxMessages = 0;
    msgqOptionsWrite.cbMaxMessage = bufferSize;
    msgqOptionsWrite.bReadAccess = FALSE;
    msgqOptionsWrite.dwSize = sizeof(msgqOptionsWrite);

    /* the message that is written to the centralised message queue */
    request->privateMessageQueueId = os_serviceGetProcessId();

    /* initialise the reply message */
    reply->result = os_resultFail;
    reply->kind = OS_SRVMSG_UNDEFINED;

    /* Create a handle to the central queue which we plan to write to */
    wStringName = wce_mbtowc(writeMsgQueueName);
    hWriteMsgQueue = CreateMsgQueue(wStringName, &msgqOptionsWrite);
    if (hWriteMsgQueue == INVALID_HANDLE_VALUE)
    {
        OS_DEBUG_2("requestResponseFromServiceQueue", "Failed to create %s %d",
                   writeMsgQueueName, os_getErrno());
        os_free(wStringName);
        return os_resultFail;
    }
    os_free(wStringName);

    /* lock this process's mutex */
    os_mutexLock (&privateProcessMutex);

    result = WriteMsgQueue(hWriteMsgQueue, request, bufferSize, INFINITE, 0);

    /* If successful wait for the read message queue to receive an update */
    if(result)
    {
        /* Note that the 'WaitForSingleObject' call is required because it blocks
         * but yeilds control (allowing other threads in).  This is not what happens
         * when ReadMsgQueue blocks.  see scarab 2462
         */

        wait = WaitForSingleObject (hReadMsgQueue, INFINITE);
        if (wait != WAIT_OBJECT_0)
        {
            OS_DEBUG_2("requestResponseFromServiceQueue", "WaitForSingleObject failed %s %d",
                       _serviceReadMsgqName, os_getErrno());
        }

        do
        {
            result = ReadMsgQueue(hReadMsgQueue,
                                  reply,
                                  bufferSize,
                                  &nRead,
                                  INFINITE,
                                  &cbRet);
            if(!result)
            {
                lastError = os_getErrno();
            }
            else
            {
                lastError = ERROR_SUCCESS;
            }
        } while((!result) && (lastError == ERROR_PIPE_NOT_CONNECTED));
    }

    /* check the data read is valid */
    if (!result || (nRead != bufferSize))
    {
        OS_DEBUG_4("requestResponseFromServiceQueue", "Failure %d %d %d %d\n", result, os_getErrno(), nRead, reply->kind);
        osr = os_resultFail;
    }

    CloseMsgQueue(hWriteMsgQueue);

    /* unlock this process's mutex */
    os_mutexUnlock (&privateProcessMutex);

    return osr;
}
