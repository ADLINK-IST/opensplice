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
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include "os__service.h"
#include <sys/timeb.h>
#include <time.h>
#include "os_heap.h"
#include "os_mutex.h"
#include "os_stdlib.h"

#include <stdio.h>
#include <assert.h>
#include "os_time.h"
#include "os_thread.h"

#include "os__debug.h"
#include "os__sharedmem.h"

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#include <Sddl.h>

#define _PIPE_STATE_CONNECTING 0
#define _PIPE_STATE_READING    1
#define _PIPE_STATE_WRITING    2
#define _PIPE_MAX_INSTANCES    8

#define _PIPE_DEFAULT_TIMEOUT  200 /* milliseconds */

#define _POOL_BLOCKSIZE 128
#define OS_SERVICE_PIPE_PREFIX "\\\\.\\pipe\\"
#define OS_SERVICE_DEFAULT_NAME "osplOSService"
#define OS_SERVICE_DEFAULT_PIPE_NAME OS_SERVICE_PIPE_PREFIX OS_SERVICE_DEFAULT_NAME

static os_time _ospl_clock_starttime = {0, 0};
static LONGLONG _ospl_clock_freq = 0; /* frequency of high performance counter */
static LONGLONG _ospl_clock_offset = 0;

struct _pipe_instance
{
   OVERLAPPED oOverlap;
   HANDLE hPipeInst;
   struct os_servicemsg request;
   DWORD reqRead;
   struct os_servicemsg reply;
   DWORD dwState;
   BOOL fPendingIO;
};

struct pool_entity {
    long id; /* when negative, the mutex is already in use! */
    HANDLE h;
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
    SECURITY_ATTRIBUTES security_attributes;
    BOOL sec_descriptor_ok;
    HANDLE namedSem;

    /* Vista and on have tightened security WRT shared memory
    we need to grant rights to interactive users et al via a discretionary
    access control list. NULL atributes did not allow interactive
    users other than process starter to access */

    ZeroMemory(&security_attributes, sizeof(security_attributes));
    security_attributes.nLength = sizeof(security_attributes);
    sec_descriptor_ok = ConvertStringSecurityDescriptorToSecurityDescriptor
                            ("D:P(A;OICI;GA;;;WD)", /* grant all acess to world (everyone) */
                            SDDL_REVISION_1,
                            &security_attributes.lpSecurityDescriptor,
                            NULL);

    if (_snprintf(name, OS_SERVICE_ENTITY_NAME_MAX, "%s%s%d%s",
        (os_sharedMemIsGlobal() ? OS_SERVICE_GLOBAL_NAME_PREFIX : ""),
        OS_SERVICE_SEM_NAME_PREFIX,
        id,
        os_getShmDomainKeyForPointer(NULL)) <= 0) {
        OS_REPORT_1(OS_ERROR, "createSem", 0, "Semaphore name exceeds maximum allowed length (%d)", OS_SERVICE_ENTITY_NAME_MAX);
        return NULL;
    }

    /* Note that NULL sec attributes would be default access - believed != 'everyone' */
    namedSem = CreateSemaphore((os_sharedMemIsGlobal() && sec_descriptor_ok ? &security_attributes : NULL),
                                0,
                                0x7fffffff,
                                name);

    if (sec_descriptor_ok)
    {
        /* Free the heap allocated descriptor */
        LocalFree(security_attributes.lpSecurityDescriptor);
    }

    return namedSem;
}

static HANDLE
createEv(
    long id,
    char *name)
{
    SECURITY_ATTRIBUTES security_attributes;
    BOOL sec_descriptor_ok;
    HANDLE namedEv;

    /* Vista and on have tightened security WRT shared memory
    we need to grant rights to interactive users et al via a discretionary
    access control list. NULL atributes did not allow interactive
    users other than process starter to access */

    ZeroMemory(&security_attributes, sizeof(security_attributes));
    security_attributes.nLength = sizeof(security_attributes);
    sec_descriptor_ok = ConvertStringSecurityDescriptorToSecurityDescriptor
                            ("D:P(A;OICI;GA;;;WD)", /* grant all acess to 'world' (everyone) */
                            SDDL_REVISION_1,
                            &security_attributes.lpSecurityDescriptor,
                            NULL);

    if (_snprintf(name, OS_SERVICE_ENTITY_NAME_MAX, "%s%s%d%s",
        (os_sharedMemIsGlobal() ? OS_SERVICE_GLOBAL_NAME_PREFIX : ""),
        OS_SERVICE_EVENT_NAME_PREFIX,
        id,
        os_getShmDomainKeyForPointer(NULL)) <= 0) {
        OS_REPORT_1(OS_ERROR, "createEv", 0, "Event name exceeds maximum allowed length (%d)", OS_SERVICE_ENTITY_NAME_MAX);
        return NULL;
    }

    /* Note that NULL sec attributes would be default access - believed != 'everyone' */
    namedEv = CreateEvent((os_sharedMemIsGlobal() && sec_descriptor_ok ? &security_attributes : NULL),
                           FALSE,
                           FALSE,
                           name);
    if (sec_descriptor_ok)
    {
        /* Free the heap allocated descriptor */
        LocalFree(security_attributes.lpSecurityDescriptor);
    }

    return namedEv;
}

static int
poolClaim(
    struct pool *pool,
    HANDLE (*create)(long id, char *name),
    long *id)
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
        newBlock = os_malloc(sizeof(struct pool_block));
        if (!newBlock) {
            return 1; /* failed to allocate new block */
        }
        newBlock->prev = NULL;
        for (i = 0; i < _POOL_BLOCKSIZE; i++) {
            newBlock->entity[i].id = max + i + 1; /* at least one */
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

    if (id == 0) { /* 0 is uninitialized mutex */
       return 0;
    }

    block = pool->tail;
    blockNr = (pool->blockCount - 1) - ((id - 1) / _POOL_BLOCKSIZE); /* reverse order */
    idxInBlock = (id - 1) % _POOL_BLOCKSIZE;
    for (i = 0; i < blockNr; i++) {
       block = block->prev;
    }
    if (block->entity[idxInBlock].id < 0) {
       block->entity[idxInBlock].id = -block->entity[idxInBlock].id;
       pool->inuse--;
       result = 0; /* success */
    } else {
       OS_DEBUG_1("poolRelease", "Trying to destroy incorrect mutex %d", id);
       result = 1;
    }

    return result;
}

/* Event Service functions */

/* ConnectToNewClient(HANDLE, LPOVERLAPPED)
 *
 * This function is called to start an overlapped connect operation.
 * It returns TRUE if an operation is pending or FALSE if the
 * connection has been completed.
 */
static BOOL
ConnectToNewClient(
    HANDLE hPipe,
    LPOVERLAPPED lpo)
{
    BOOL fConnected;
    BOOL fPendingIO = FALSE;

    /* Start an overlapped connection for this pipe instance. */
    fConnected = ConnectNamedPipe(hPipe, lpo);

    /* Overlapped ConnectNamedPipe should return zero.*/
    if (fConnected) {
        OS_DEBUG_1("ConnectToNewClient", "ConnectNamedPipe failed with %d.\n", GetLastError());
        return 0;
    }

    switch (GetLastError()) {
    /* The overlapped connection in progress. */
    case ERROR_IO_PENDING:
        fPendingIO = TRUE;
    break;
    case ERROR_PIPE_CONNECTED: /* Client is already connected, so signal an event. */
        if (!SetEvent(lpo->hEvent)) {
            OS_DEBUG_1("ConnectToNewClient", "ConnectNamedPipe failed with %d.\n", GetLastError());
            fPendingIO = FALSE;
        }
    break;
    default: /* If an error occurs during the connect operation... */
        OS_DEBUG_1("ConnectToNewClient", "ConnectNamedPipe failed with %d.\n", GetLastError());
        fPendingIO = FALSE;
    break;
    }

   return fPendingIO;
}

/* DisconnectAndReconnect(DWORD)
 * This function is called when an error occurs or when the client
 * closes its handle to the pipe. Disconnect from this client, then
 * call ConnectNamedPipe to wait for another client to connect.
 */
static VOID
DisconnectAndReconnect(
    struct _pipe_instance *pipe)
{

    /* Disconnect the pipe instance. */
    if (!DisconnectNamedPipe(pipe->hPipeInst) ) {
        OS_DEBUG_1("DisconnectAndReconnect", "DisconnectNamedPipe failed with %d.\n", GetLastError());
    }
    /* connect to new client */
    pipe->fPendingIO = ConnectToNewClient(
                           pipe->hPipeInst,
                           &pipe->oOverlap);

    pipe->dwState = (pipe->fPendingIO?_PIPE_STATE_CONNECTING:_PIPE_STATE_READING);
}

static void
handleRequest(
    struct eventService *es,
    struct _pipe_instance *pipe,
    int *terminate)
{
    struct pool *pool;

    /* read request */
    pipe->reply.kind = pipe->request.kind;
    switch (pipe->request.kind) {
    case OS_SRVMSG_CREATE_EVENT:
        pool = &es->eventPool;
        if (poolClaim(pool, createEv, &pipe->reply._u.id) == 0) {
            pipe->reply.result = os_resultSuccess;
        } else {
            pipe->reply.result = os_resultFail;
        }
    break;
    case OS_SRVMSG_DESTROY_EVENT:
        pool = &es->eventPool;
        if (poolRelease(pool, pipe->request._u.id) == 0) {
           pipe->reply.result = os_resultSuccess;
        } else {
           pipe->reply.result = os_resultFail;
        }
    break;
    case OS_SRVMSG_CREATE_SEMAPHORE:
        pool = &es->semPool;
        if (poolClaim(pool, createSem, &pipe->reply._u.id) == 0) {
            pipe->reply.result = os_resultSuccess;
        } else {
            pipe->reply.result = os_resultFail;
        }
    break;
    case OS_SRVMSG_DESTROY_SEMAPHORE:
        pool = &es->semPool;
        if (poolRelease(pool, pipe->request._u.id) == 0) {
            pipe->reply.result = os_resultSuccess;
        } else {
            pipe->reply.result = os_resultFail;
        }
    break;
    case OS_SRVMSG_GET_TIME:
        pipe->reply._u.time.start_time = _ospl_clock_starttime;
        pipe->reply._u.time.time_offset = _ospl_clock_offset;
        pipe->reply.result = os_resultSuccess;
    break;
    case OS_SRVMSG_TERMINATE:
        pipe->reply.result = os_resultSuccess;
        *terminate = 1;
    break;
    case OS_SRVMSG_UNDEFINED:
    case OS_SRVMSG_COUNT:
    default:
        pipe->reply.result = os_resultFail;
        OS_DEBUG_1("handleRequest", "Incorrect msg kind in request %d", pipe->request.kind);
    }
}

static void *
osServiceThread(
    void *arg)
{
    struct _pipe_instance pipe[_PIPE_MAX_INSTANCES];
    HANDLE hEvents[_PIPE_MAX_INSTANCES];
    BOOL fSuccess;
    DWORD i;
    DWORD dwWait;
    DWORD cbRet;
    struct eventService es;
    SECURITY_ATTRIBUTES security_attributes;

	int terminate;
    /* only use 'ready' to indicate initialisation is done. Then
     * never use this variable again as it is a stack variable of
     *  the os_serviceStart() routine
     */
    HANDLE *initializedEvent = (HANDLE *)arg;
    BOOL sec_descriptor_ok;

    terminate = 0;

	ZeroMemory(&security_attributes, sizeof(security_attributes));
    security_attributes.nLength = sizeof(security_attributes);
    sec_descriptor_ok = ConvertStringSecurityDescriptorToSecurityDescriptor
                            ("D:P(A;OICI;GA;;;WD)", /* grant all acess to 'world' (everyone) */
                            SDDL_REVISION_1,
                            &security_attributes.lpSecurityDescriptor,
                            NULL);

    os_threadSetThreadName(-1, "ospService OSPL Service Thread");
    poolInit(&es.eventPool);
    poolInit(&es.semPool);
    for (i = 0; i < _PIPE_MAX_INSTANCES; i++) {
        hEvents[i] = CreateEvent(
                         NULL,    // default security attribute
                         TRUE,    // manual-reset event
                         TRUE,    // initial state = signaled
                         NULL);   // unnamed event object

        pipe[i].hPipeInst = CreateNamedPipe(
                                _ospl_servicePipeName,     // pipe name
                                PIPE_ACCESS_DUPLEX |     // read/write access
                                FILE_FLAG_OVERLAPPED,    // overlapped mode
                                PIPE_TYPE_MESSAGE |      // message-type pipe
                                PIPE_READMODE_MESSAGE |  // message-read mode
                                PIPE_WAIT,               // blocking mode
                                _PIPE_MAX_INSTANCES,     // number of instances
                                sizeof(pipe[i].reply),   // output buffer size
                                sizeof(pipe[i].request), // input buffer size
                                _PIPE_DEFAULT_TIMEOUT,   // client time-out
								(os_sharedMemIsGlobal() && sec_descriptor_ok ? &security_attributes : NULL)); // Set security attributes

        if (pipe[i].hPipeInst == INVALID_HANDLE_VALUE) {
            OS_DEBUG_2("osServiceThread", "Failed to create named pipe %s %d",
                       _ospl_servicePipeName, GetLastError());
        }
        // Call the subroutine to connect to the new client
        memset(&pipe[i].oOverlap, 0, sizeof(OVERLAPPED));
        pipe[i].oOverlap.hEvent = hEvents[i];
        pipe[i].fPendingIO = ConnectToNewClient(
            pipe[i].hPipeInst,
            &pipe[i].oOverlap);

        pipe[i].dwState = (pipe[i].fPendingIO?_PIPE_STATE_CONNECTING:_PIPE_STATE_READING);

        pipe[i].reply.result = os_resultFail;
        pipe[i].reply.kind = OS_SRVMSG_UNDEFINED;
        pipe[i].reply._u.id = -1;

        pipe[i].reqRead = 0;
        pipe[i].request.result = os_resultFail;
        pipe[i].request.kind = OS_SRVMSG_UNDEFINED;
        pipe[i].request._u.id = -1;
    }

    /* We are done initialising, notify thread that started this service and then never use this
     * handle again!
     */
    SetEvent(*initializedEvent);
    initializedEvent = NULL;
    while (!terminate) {
        /* Wait for the event object to be signaled, indicating
         * completion of an overlapped read, write, or connect
         * operation.
         */
        dwWait = WaitForMultipleObjects(
                     _PIPE_MAX_INSTANCES,    // number of event objects
                     hEvents,                // array of event objects
                     FALSE,                  // do not wait for all
                     INFINITE);              // wait for ever

        /* dwWait shows which pipe completed the operation. */
        i = dwWait - WAIT_OBJECT_0;  // determines which pipe
        assert(i >= 0 || i < _PIPE_MAX_INSTANCES);
        /* Get the result if the operation was pending. */

        if (pipe[i].fPendingIO) {
            fSuccess = GetOverlappedResult(
                           pipe[i].hPipeInst, // handle to pipe
                           &pipe[i].oOverlap, // OVERLAPPED structure
                           &cbRet,            // bytes transferred
                           FALSE);            // do not wait
            switch (pipe[i].dwState) {
                /* Pending connect operation */
            case _PIPE_STATE_CONNECTING:
                if (fSuccess) {
                    pipe[i].dwState = _PIPE_STATE_READING; /* next state */
                } else {
                    DisconnectAndReconnect(&pipe[i]);
                }
            break;
                /* Pending read operation */
            case _PIPE_STATE_READING:
                if (!fSuccess && (GetLastError() == ERROR_IO_PENDING)) {
                    pipe[i].fPendingIO = TRUE;
                    continue;
                }
                if (!fSuccess || (cbRet == 0)) {
                    OS_DEBUG_4("osServiceThread", "[%d] pending READ: failure %d %d %d", i, fSuccess, cbRet, GetLastError());
                    assert(0);
                    DisconnectAndReconnect(&pipe[i]);
                    continue;
                }
                pipe[i].dwState = _PIPE_STATE_WRITING;
            break;
            /* Pending write operation */
            case _PIPE_STATE_WRITING:
                if (!fSuccess || (cbRet != sizeof(pipe[i].reply))) {
                    OS_DEBUG_2("osServiceThread", "[%d] pending WRITE: failure %d", i, GetLastError());
                    DisconnectAndReconnect(&pipe[i]);
                    continue;
                }
                pipe[i].dwState = _PIPE_STATE_READING;
            break;
            default:
                OS_DEBUG("osServiceThread", "Invalid pipe state.");
            }
        }

        /* The pipe state determines which operation to do next. */
        switch (pipe[i].dwState) {
        /* _PIPE_STATE_READING:
         * The pipe instance is connected to the client
         * and is ready to read a request from the client.
         */
        case _PIPE_STATE_READING:
            //memset(&pipe[i].oOverlap, 0, sizeof(OVERLAPPED));
            //pipe[i].oOverlap.hEvent = hEvents[i];
            fSuccess = ReadFile(
                           pipe[i].hPipeInst,
                           &pipe[i].request,
                           sizeof(pipe[i].request),
                           &pipe[i].reqRead,
                           &pipe[i].oOverlap);
            /* The read operation completed successfully. */
            if (fSuccess && (pipe[i].reqRead != 0)) {
                pipe[i].fPendingIO = FALSE;
                pipe[i].dwState = _PIPE_STATE_WRITING;
                continue;
            }
            /* The read operation is still pending. */
            if (!fSuccess && (GetLastError() == ERROR_IO_PENDING)) {
                pipe[i].fPendingIO = TRUE;
                continue;
            }
            OS_DEBUG_2("osServiceThread", "[%d] READ: failure %d", i, GetLastError());
            /* An error occurred; disconnect from the client. */
            assert(0);
            DisconnectAndReconnect(&pipe[i]);
        break;
        /* _PIPE_STATE_WRITING:
         * The request was successfully read from the client.
         * Get the reply data and write it to the client.
         */
        case _PIPE_STATE_WRITING:
            handleRequest(&es, &pipe[i], &terminate);
            fSuccess = WriteFile(
                           pipe[i].hPipeInst,
                           &pipe[i].reply,
                           sizeof(pipe[i].reply),
                           &cbRet,
                           &pipe[i].oOverlap);
            if (fSuccess && (cbRet == sizeof(pipe[i].reply))) {
                pipe[i].fPendingIO = FALSE;
                pipe[i].dwState = _PIPE_STATE_READING;
                DisconnectAndReconnect(&pipe[i]);
                continue;
            }
            /* The write operation is still pending. */
            if (!fSuccess && (GetLastError() == ERROR_IO_PENDING)) {
                pipe[i].fPendingIO = TRUE;
                continue;
            }
            /* Whether an error occurred on not, just disconnect from the client */
            OS_DEBUG_2("osServiceThread", "[%d] WRITE: failure %d", i, GetLastError());
            assert(0);
            DisconnectAndReconnect(&pipe[i]);
        break;
        default:
        break;
        }
    }

    for (i = 0; i < _PIPE_MAX_INSTANCES; i++) {
        CloseHandle(hEvents[i]);
        CloseHandle(pipe[i].hPipeInst);
    }
    poolDeinit(&es.eventPool);
    poolDeinit(&es.semPool);
    free(_ospl_servicePipeName); /* allocated by os_serviceStart! */
	if (sec_descriptor_ok)
    {
        /* Free the heap allocated descriptor */
        LocalFree(security_attributes.lpSecurityDescriptor);
    }

    _ospl_servicePipeName = OS_SERVICE_DEFAULT_PIPE_NAME;


    return NULL;
}


os_char *
createPipeName(void)
{
    return os_constructPipeName(_ospl_serviceName);
}

os_char *
os_createPipeNameFromMutex(
    os_mutex *mutex)
{
    os_char *name;

    assert(mutex);

    name = os_getDomainNameforMutex(mutex);
    if (name == NULL) {
        name = _ospl_serviceName;
    }
    return os_constructPipeName(name);
}

os_char *
os_createPipeNameFromCond(os_cond *cond)
{
    os_char *name;

    assert(cond);

    name = os_getDomainNameforCond(cond);
    if (name == NULL) {
        name = _ospl_serviceName;
    }
    return os_constructPipeName(name);
}

os_char *
os_constructPipeName(
    const os_char *name)
{
    os_char *n;
    size_t i, len;

    assert(name);

    n = os_malloc(strlen(name) + strlen(OS_SERVICE_PIPE_PREFIX) + 1);
    if (n) {
        strcpy(n, OS_SERVICE_PIPE_PREFIX);
        strcat(n, name);
        /* replace all ' ' occurrences with '/', since space is not
         * allowed in the pipename
         */
        len = strlen(n);
        for (i = strlen(OS_SERVICE_PIPE_PREFIX); i < len; i++) {
            if (n[i] == ' ') {
                n[i] = '/';
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
    HANDLE initializedEvent;
    DWORD result;
    LARGE_INTEGER p0, p1, p_best;
    LONGLONG limit, prev_diff, t_diff, limit_best, t_diff_best, t_diff_start;
    struct __timeb64 t0, t1, t_best, t_start;
    char uniqueName[16 + sizeof(UNIQUE_PREFIX)];

    r = os_resultSuccess;
    initializedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (QueryPerformanceFrequency(&frequency) != 0)
    {
        _ospl_clock_freq = frequency.QuadPart;
        /* Synchronization of the clock:
         * See: http://msdn.microsoft.com/en-us/magazine/cc163996.aspx
         */
        limit = 15 * 2; /* 15 usec accuracy requested. */
        limit_best = -1;
        t_diff_best = -1;
        _ftime64_s(&t_start);

        do
        {
            prev_diff = 0;
            QueryPerformanceCounter(&p0);
            _ftime64_s(&t0);

            do
            {
                _ftime64_s(&t1);
                QueryPerformanceCounter(&p1);

                if((t0.time == t1.time) && (t0.millitm == t1.millitm))
                {
                    prev_diff = p1.QuadPart - p0.QuadPart;
                    p0 = p1;
                }
            }
            while((t0.time == t1.time) && (t0.millitm == t1.millitm));

            if (t1.millitm >= t0.millitm)
            {
                t_diff = t1.millitm - t0.millitm;
                t_diff += (t1.time - t0.time) * 1000;
            }
            else
            {
                t_diff = t1.millitm - t0.millitm + 1000;
                t_diff += (t1.time * 1000) - (t0.time * 1000) - 1000;
            }

            if((t_diff_best == -1) || (t_diff <= t_diff_best))
            {
                if( (limit_best == -1) ||
                    ((p1.QuadPart - p0.QuadPart + prev_diff) < limit_best))
                {
                    t_diff_best = t_diff;
                    p_best = p1;
                    t_best = t1;
                    limit_best = p1.QuadPart - p0.QuadPart + prev_diff;
                }
            }
            if (t1.millitm >= t_start.millitm)
            {
                t_diff_start = t1.millitm - t_start.millitm;
                t_diff_start += (t1.time - t_start.time) * 1000;
            }
            else
            {
                t_diff_start = t1.millitm - t_start.millitm + 1000;
                t_diff_start += (t1.time * 1000) - (t_start.time * 1000) - 1000;
            }

        }
        while( ((t_diff > 15) ||
               ((p1.QuadPart - p0.QuadPart + prev_diff) >= limit)) &&
               (t_diff_start < 2000));

        _ospl_clock_starttime.tv_sec = (os_timeSec)t_best.time;
        _ospl_clock_starttime.tv_nsec = t_best.millitm * 1000000;
        _ospl_clock_offset = p_best.QuadPart;

        OS_REPORT_7(OS_DEBUG, "os_serviceStart", 0,
            "Time sync took: %"PA_PA_PRId64" ms; resolution= %"PA_PA_PRId64" ms, accuracy=%"PA_PA_PRId64" us, "
            "time=%"PA_PA_PRId64",%d, offset=%"PA_PA_PRId64" and frequency=%"PA_PA_PRId64"",
            t_diff_start, t_diff_best, limit_best/2,
            _ospl_clock_starttime.tv_sec, _ospl_clock_starttime.tv_nsec,
            _ospl_clock_offset, _ospl_clock_freq);
    }
    else
    {
        /* no high performance timer available!
         * so we fall back to a millisecond clock.
         */
        OS_REPORT_1(OS_WARNING, "os_serviceStart", 0,
                "No high-resolution timer found (reason: %s), "\
                "switching to millisecond resolution.", GetLastError());
    }

    if (name == NULL) {
        snprintf(uniqueName, sizeof(uniqueName), "%s%d",
                 UNIQUE_PREFIX, GetCurrentProcessId());
        _ospl_serviceName = (char *)os_malloc(strlen(uniqueName) + 1);
        os_strcpy(_ospl_serviceName, uniqueName);
    } else {
        _ospl_serviceName = os_malloc(strlen(name) + 1);
        if (_ospl_serviceName) {
            os_strcpy(_ospl_serviceName, name);
        }
    }
    if (_ospl_serviceName) {
        _ospl_servicePipeName = createPipeName();

        _ospl_serviceThreadId = CreateThread(NULL,
            (SIZE_T)128*1024,
            (LPTHREAD_START_ROUTINE)osServiceThread,
            (LPVOID)&initializedEvent,
            (DWORD)0, &threadIdent);
        if (_ospl_serviceThreadId == 0) {
            r = os_resultFail;
            free(_ospl_servicePipeName);

            _ospl_servicePipeName = OS_SERVICE_DEFAULT_PIPE_NAME;
        } else {
            /* Wait for thread to be done with intialisation */
            result = WaitForSingleObject(initializedEvent, INFINITE);
            assert(result == WAIT_OBJECT_0);
            if (result != WAIT_OBJECT_0) {
                r = os_resultFail;
            }
        }
    } else {
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
    BOOL result;
    DWORD nRead;
    os_result osr;

    osr = os_resultSuccess;
    request.kind = OS_SRVMSG_TERMINATE;
    reply.result = os_resultFail;
    result = CallNamedPipe(
                 _ospl_servicePipeName,
                 &request, sizeof(request),
                 &reply, sizeof(reply),
                 &nRead,
                 NMPWAIT_WAIT_FOREVER);
    if (!result || (nRead == 0)) {
        osr = os_resultFail;
    } else {
        if (GetExitCodeThread(_ospl_serviceThreadId, &nRead) == 0) {
            OS_DEBUG_1("serviceStop", "GetExitCodeThread Failed %d", (int)GetLastError());
           osr = os_resultFail;
        } else {
            while (nRead == STILL_ACTIVE) {
                Sleep(100);
                if (GetExitCodeThread(_ospl_serviceThreadId, &nRead) == 0) {
                    OS_DEBUG_1("serviceStop", "GetExitCodeThread Failed %d", (int)GetLastError());
                    osr = os_resultFail;
                    nRead = 0; /* break loop */
                }
            }
        }
    }
    return osr;
}

const char *
os_serviceName(void)
{
    return _ospl_serviceName;
}

os_char *
os_servicePipeName(void)
{
    return _ospl_servicePipeName;
}

void
os_serviceSetSingleProcess (void)
{
    _ospl_singleProcess = OS_TRUE;
}

os_boolean
os_serviceGetSingleProcess (void)
{
    return _ospl_singleProcess;
}
