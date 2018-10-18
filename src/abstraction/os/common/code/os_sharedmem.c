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
/** \file os/common/code/os_sharedmem.c
 *  \brief common shared memory implementation
 *
 * Implements shared memory for UNIX like platyforms and knows
 * three types of implementation:
 * - POSIX shared objects
 * - SVR4 shared memory segments (IPC)
 * - Heap
 */

#include "os_heap.h"
#include "os_defs.h"
#include <assert.h>
#include <signal.h>
#include "../common/code/os_sharedmem_handle.c"

#if !defined( __vxworks ) || defined( __RTP__)
#include "../common/code/os_keyfile.c"
#endif

#include "code/os__sharedmem.h"
#include "os_thread.h"
#include "os_socket.h"
#include "os_mutex.h"
#include "os_cond.h"

#if 0
  #define TRACE_PKILL printf
#else
  #define TRACE_PKILL(...)
#endif

#ifdef OSPL_SHM_PROCMON
#include <sys/un.h>

static os_result
os_sharedMemoryProcessMonitorInit(
    os_sharedHandle sharedHandle);

static os_result
os_sharedMemoryProcessMonitorDeinit(
    os_sharedHandle sharedHandle);

static os_result
os_sharedMemoryRegisterProcess(
    os_sharedHandle sharedHandle);

static os_result
os_sharedMemoryDeregisterProcess(
    os_sharedHandle sharedHandle);

#endif

OS_CLASS(os__shmClient);
OS_STRUCT(os__shmClient){
    os_int procId;
    int fd;
    os_shmProcState state;
};

OS_CLASS(os__shmClients);
OS_STRUCT(os__shmClients) {
    os_iter runningClients;
    os_iter diff;
    os_iter temp;
    fd_set rset;
    int maxSock;
};

OS_CLASS(os__shmDomain);
OS_STRUCT(os__shmDomain){
    os_sharedHandle handle;
    int osplProcMonSock;
    os_threadId procMonTid;
    int terminate;
    os__shmClients clnts;
};

/* Shared memory segments I am monitoring */
static os_iter os__shmMonitoring = NULL;

OS_CLASS(os__clientShmDomain);
OS_STRUCT(os__clientShmDomain){
    os_sharedHandle handle;
    int mySock;
    os_mutex mutex;
    os_boolean monitorRunning;
    os_threadId monitorThread;
    os_boolean detaching;
    os_onSharedMemoryManagerDiedCallback onServerDied;
    void *args;
};

OS_STRUCT(os_implData) {
    char *lockFileName;
};

static char *
getKeyfileFromHandle(os_sharedHandle sharedHandle);


/* Shared memory segments I am attached to */
static os_iter os__shmAttached = NULL;
/** Mutex for locking the shared memory data */
static os_mutex os__shmAttachedLock;
/** condition variable for synchronisation
 *  twofold:
 *  1. to delay return of shared memory create call until the shared memory monitor thread is running.
 *  2. to signal the spliced that the shared memory monitor thread has detected a client side change.
 */
static os_cond os__shmAttachCond;

/** Defines the file name for the shared mem creation lock file */
static const char *os_key_file_creation_lock = "spddscreationLock";


void
os_sharedMemoryInit(void)
{
    os_result result;

    os_heap_sharedMemoryInit();

    os__shmMonitoring = os_iterNew(NULL);
    os__shmAttached = os_iterNew(NULL);

    result = os_mutexInit(&os__shmAttachedLock, NULL);
    if (result == os_resultSuccess) {
        result = os_condInit(&os__shmAttachCond, &os__shmAttachedLock, NULL);
    }
    if (result != os_resultSuccess) {
        OS_REPORT(OS_ERROR, "os_sharedMemoryInit", 0, "Initialization failed (Fatal)");
        abort();
    }
    return;
}

void
os_sharedMemoryExit(void)
{
    os__shmDomain shmDomain;
    os_iter copyIter;
    os_heap_sharedMemoryExit();

    copyIter = os_iterCopy(os__shmMonitoring);
    shmDomain = (os__shmDomain)os_iterTakeFirst(copyIter);

    while(shmDomain){
#ifdef OSPL_SHM_PROCMON
        os_sharedMemoryProcessMonitorDeinit(shmDomain->handle);
#endif
        shmDomain = (os__shmDomain)os_iterTakeFirst(copyIter);
    }
    os_iterFree(copyIter);
    os_iterFree(os__shmMonitoring);
    os_iterFree(os__shmAttached);
    os_mutexDestroy(&os__shmAttachedLock);
    os_condDestroy(&os__shmAttachCond);

    return;
}

os_result
os_sharedMemoryCreate(
    os_sharedHandle sharedHandle,
    os_address size)
{
    os_result result = os_resultFail;

    assert(sharedHandle != NULL);
    assert(sharedHandle->name != NULL);

    switch (sharedHandle->attr.sharedImpl) {
    case OS_MAP_ON_FILE:
        result = os_posix_sharedMemoryCreate(sharedHandle->name, &sharedHandle->attr, size, sharedHandle->id);
    break;
    case OS_MAP_ON_SEG:
        result = os_svr4_sharedMemoryCreate(sharedHandle->name, &sharedHandle->attr, size, sharedHandle->id);
#ifdef OSPL_SHM_PROCMON
        if(result == os_resultSuccess){
            result = os_sharedMemoryProcessMonitorInit(sharedHandle);
            if(result != os_resultSuccess){
                (void)os_svr4_sharedMemoryDestroy(sharedHandle->name);
            }
        }
#endif
    break;
    case OS_MAP_ON_HEAP:
        result = os_heap_sharedMemoryCreate(sharedHandle->name, &sharedHandle->attr, size, sharedHandle->id);
    break;
    }
    return result;
}

os_result
os_sharedMemoryGetNameFromId(
    os_sharedHandle sharedHandle,
    char **name)
{
    os_result result = os_resultFail;

    assert(sharedHandle != NULL);
    switch (sharedHandle->attr.sharedImpl) {
    case OS_MAP_ON_FILE:
        result = os_posix_sharedMemoryGetNameFromId(sharedHandle->id, name);
    break;
    case OS_MAP_ON_SEG:
        result = os_svr4_sharedMemoryGetNameFromId(sharedHandle->id, name);
    break;
    case OS_MAP_ON_HEAP:
        result = os_heap_sharedMemoryGetNameFromId(sharedHandle->id, name);
    break;
    }
    return result;
}

os_result
os_sharedMemoryDestroy(
    os_sharedHandle sharedHandle)
{
    os_result result = os_resultFail;

    assert(sharedHandle != NULL);
    assert(sharedHandle->name != NULL);
    switch (sharedHandle->attr.sharedImpl) {
    case OS_MAP_ON_FILE:
        result = os_posix_sharedMemoryDestroy(sharedHandle->name);
    break;
    case OS_MAP_ON_SEG:
        result = os_svr4_sharedMemoryDestroy(sharedHandle->name);
#ifdef OSPL_SHM_PROCMON
        if(result == os_resultSuccess){
            os_sharedMemoryProcessMonitorDeinit(sharedHandle);
        }
#endif
    break;
    case OS_MAP_ON_HEAP:
        result = os_heap_sharedMemoryDestroy(sharedHandle->name);
    break;
    }
    return result;
}

os_result
os_sharedMemoryAttach(
    os_sharedHandle sharedHandle)
{
    os_result result = os_resultFail;

    assert(sharedHandle != NULL);
    assert(sharedHandle->name != NULL);
    assert(sharedHandle->mapped_address == NULL);
    switch (sharedHandle->attr.sharedImpl) {
    case OS_MAP_ON_FILE:
        result = os_posix_sharedMemoryAttach (sharedHandle->name, &sharedHandle->attr, &sharedHandle->mapped_address, sharedHandle->id);
    break;
    case OS_MAP_ON_SEG:
        result = os_svr4_sharedMemoryAttach (sharedHandle->name, &sharedHandle->attr, &sharedHandle->mapped_address, sharedHandle->id);
#ifdef OSPL_SHM_PROCMON
        if(result == os_resultSuccess){
            if (os_sharedMemoryRegisterProcess(sharedHandle) != os_resultSuccess) {
                OS_REPORT_WID(OS_WARNING, "os_sharedMemoryAttach",0, sharedHandle->id,
                        "Could not register process with Spliced::SHM Process Monitor");
            }
        }
#endif
    break;
    case OS_MAP_ON_HEAP:
        result = os_heap_sharedMemoryAttach (sharedHandle->name, &sharedHandle->mapped_address);
    break;
    }
    return result;
}

static os_result
os__sharedMemoryDetach(
    os_sharedHandle sharedHandle,
    os_boolean clean)
{
    os_result result = os_resultFail;

    assert(sharedHandle != NULL);
    assert(sharedHandle->name != NULL);

    switch (sharedHandle->attr.sharedImpl) {
    case OS_MAP_ON_FILE:
        assert(sharedHandle->mapped_address != NULL);
        result = os_posix_sharedMemoryDetach (sharedHandle->name, sharedHandle->mapped_address, sharedHandle->id);
    break;
    case OS_MAP_ON_SEG:
        assert(sharedHandle->mapped_address != NULL);
        result = os_svr4_sharedMemoryDetach (sharedHandle->name, sharedHandle->mapped_address, sharedHandle->id);
#ifdef OSPL_SHM_PROCMON
        if((result == os_resultSuccess) && clean){
            if (os_sharedMemoryDeregisterProcess(sharedHandle) != os_resultSuccess) {

            }
        }
#endif
    break;
    case OS_MAP_ON_HEAP:
        /* sharedHandle->mapped_address may be 0 in heap configuration */
        result = os_heap_sharedMemoryDetach (sharedHandle->name, sharedHandle->mapped_address, sharedHandle->id);
    break;
    }
    if (result == os_resultSuccess) {
        sharedHandle->mapped_address = NULL;
    }
    return result;
}

os_result
os_sharedMemoryDetach(
    os_sharedHandle sharedHandle)
{
    return os__sharedMemoryDetach(sharedHandle, OS_TRUE);
}


os_result
os_sharedMemoryDetachUnclean(
    os_sharedHandle sharedHandle)
{
    return os__sharedMemoryDetach(sharedHandle, OS_FALSE);
}


os_result
os_sharedSize(
    os_sharedHandle sharedHandle,
    os_address *size)
{
    os_result result = os_resultFail;

    assert(sharedHandle != NULL);
    assert(sharedHandle->name != NULL);

    switch (sharedHandle->attr.sharedImpl) {
    case OS_MAP_ON_FILE:
        assert(sharedHandle->mapped_address != NULL);
        result = os_posix_sharedSize(sharedHandle->name, size);
    break;
    case OS_MAP_ON_SEG:
        assert(sharedHandle->mapped_address != NULL);
        result = os_svr4_sharedSize(sharedHandle->name, size);
    break;
    case OS_MAP_ON_HEAP:
        /* sharedHandle->mapped_address may be 0 in heap configuration */
        result = os_heap_sharedSize(sharedHandle->name, size);
    break;
    default:
    break;
    }
    return result;
}

char *
os_findKeyFile(
    const char * name)
{
     char* result;
     os_sharedAttr shmAttr;
     os_sharedAttrInit(&shmAttr);
     result = NULL;

     switch (shmAttr.sharedImpl) {
     case OS_MAP_ON_FILE:
         result = os_posix_findKeyFile(name);
     break;
     case OS_MAP_ON_SEG:
         result = os_svr4_findKeyFile(name);
     break;
     case OS_MAP_ON_HEAP:
         result = NULL;
     break;
     }
     return result;
}

char *
os_findKeyFileById(
    os_int32 domainId)
{
     char* result;
     os_sharedAttr shmAttr;
     os_sharedAttrInit(&shmAttr);
     result = NULL;

     switch (shmAttr.sharedImpl) {
     case OS_MAP_ON_FILE:
         result = os_posix_findKeyFileById(domainId);
     break;
     case OS_MAP_ON_SEG:
         result = os_svr4_findKeyFileById(domainId);
     break;
     case OS_MAP_ON_HEAP:
         result = NULL;
     break;
     }
     return result;
}

char *
os_findKeyFileByNameAndId(
    const char * name,
    const os_int32 id)
{
     char* result;
     os_sharedAttr shmAttr;
     os_sharedAttrInit(&shmAttr);
     result = NULL;

     switch (shmAttr.sharedImpl) {
     case OS_MAP_ON_FILE:
         result = os_posix_findKeyFileByIdAndName(id, name);
     break;
     case OS_MAP_ON_SEG:
         result = os_svr4_findKeyFileByIdAndName(id, name);
     break;
     case OS_MAP_ON_HEAP:
         result = NULL;
     break;
     }
     return result;
}

os_int32
os_destroyKeyFile(
    const char * name)
{
     os_int32 result;
     os_sharedAttr shmAttr;
     os_sharedAttrInit(&shmAttr);
     result = 0;
     (void)result;

     switch (shmAttr.sharedImpl) {
     case OS_MAP_ON_FILE:
         result = os_posix_destroyKeyFile(name);
     break;
     case OS_MAP_ON_SEG:
         result = os_svr4_destroyKeyFile(name);
     break;
     case OS_MAP_ON_HEAP:
         result = 0;
     break;
     }
     return result;
}

os_int32
os_destroyKey(
    const char * name)
{
     os_int32 result;
     os_sharedAttr shmAttr;
     os_sharedAttrInit(&shmAttr);
     result = 0;
     (void)result;

     switch (shmAttr.sharedImpl) {
     case OS_MAP_ON_FILE:
         result = os_posix_destroyKey(name);
     break;
     case OS_MAP_ON_SEG:
         result = os_svr4_destroyKey(name);
     break;
     case OS_MAP_ON_HEAP:
         result = 0;
     break;
     }
     return result;
}

os_int32
os_sharedMemoryListDomainNames(
    os_iter nameList)
{
     os_int32 result;
     os_sharedAttr shmAttr;
     os_sharedAttrInit(&shmAttr);
     result = 0;
     (void)result;

     switch (shmAttr.sharedImpl) {
     case OS_MAP_ON_FILE:
         result = os_posix_listDomainNames(nameList);
     break;
     case OS_MAP_ON_SEG:
         result = os_svr4_listDomainNames(nameList);
     break;
     case OS_MAP_ON_HEAP:
         result = 0;
     break;
     }
     return result;
}

void
os_sharedMemoryListDomainNamesFree(
    os_iter nameList)
{
     os_sharedAttr shmAttr;
     os_sharedAttrInit(&shmAttr);

     switch (shmAttr.sharedImpl) {
     case OS_MAP_ON_FILE:
         os_posix_listDomainNamesFree(nameList);
     break;
     case OS_MAP_ON_SEG:
         os_svr4_listDomainNamesFree(nameList);
     break;
     case OS_MAP_ON_HEAP:
     break;
     }
}

os_int32
os_sharedMemoryListDomainIds(
     os_int32 **idList,
     os_int32  *listSize)
{
     os_int32 result;
     os_sharedAttr shmAttr;
     os_sharedAttrInit(&shmAttr);
     result = 0;
     (void)result;

     switch (shmAttr.sharedImpl) {
     case OS_MAP_ON_FILE:
         result = os_posix_listDomainIds(idList, listSize);
     break;
     case OS_MAP_ON_SEG:
         result = os_svr4_listDomainIds(idList, listSize);
     break;
     case OS_MAP_ON_HEAP:
         result = 0;
     break;
     }
     return result;
}


os_int32
os_sharedMemoryListUserProcesses(
    os_iter pidList,
    const char * fileName)
{
     os_int32 result;
     os_sharedAttr shmAttr;
     os_sharedAttrInit(&shmAttr);
     result = 0;
     (void)result;

     switch (shmAttr.sharedImpl) {
     case OS_MAP_ON_FILE:
         result = os_posix_listUserProcesses(pidList, fileName);
     break;
     case OS_MAP_ON_SEG:
         result = os_svr4_listUserProcesses(pidList, fileName);
     break;
     case OS_MAP_ON_HEAP:
         result = 0;
     break;
     }
     return result;
}

os_int32
os_sharedMemoryListUserProcessesFree(
    os_iter pidList)
{
     os_int32 result;
     os_sharedAttr shmAttr;
     os_sharedAttrInit(&shmAttr);
     result = 0;
     (void)result;

     switch (shmAttr.sharedImpl) {
     case OS_MAP_ON_FILE:
         result = os_posix_listUserProcessesFree(pidList);
     break;
     case OS_MAP_ON_SEG:
         result = os_svr4_listUserProcessesFree(pidList);
     break;
     case OS_MAP_ON_HEAP:
         result = 0;
     break;
     }
     return result;
}

os_int32
os_sharedMemorySegmentFree(
    const char * fname)
{
     os_int32 result;
     os_sharedAttr shmAttr;
     os_sharedAttrInit(&shmAttr);
     result = 0;
     (void)result;

     switch (shmAttr.sharedImpl) {
     case OS_MAP_ON_FILE:
         result = os_posix_sharedMemorySegmentFree(fname);
     break;
     case OS_MAP_ON_SEG:
         result = os_svr4_sharedMemorySegmentFree(fname);
     break;
     case OS_MAP_ON_HEAP:
         result = 0;
     break;
     }
     return result;
}

void
os_cleanSharedMemAndOrKeyFiles(
    void)
{
     os_sharedAttr shmAttr;
     os_sharedAttrInit(&shmAttr);

     switch (shmAttr.sharedImpl) {
     case OS_MAP_ON_FILE:
         os_posix_cleanSharedMemAndOrKeyFiles();
     break;
     case OS_MAP_ON_SEG:
         os_svr4_cleanSharedMemAndOrKeyFiles();
     break;
     case OS_MAP_ON_HEAP:
     break;
     }
}

os_state
os_sharedMemoryGetState(
    os_sharedHandle sharedHandle)
{
     os_state state = OS_STATE_NONE;
     char *keyfile;

     assert(sharedHandle != NULL);

     keyfile = getKeyfileFromHandle(sharedHandle);
     if ((keyfile != NULL) || (sharedHandle->attr.sharedImpl == OS_MAP_ON_HEAP)) {
         switch (sharedHandle->attr.sharedImpl) {
         case OS_MAP_ON_FILE:
             state = os_posix_sharedMemoryGetState(keyfile);
         break;
         case OS_MAP_ON_SEG:
             state = os_svr4_sharedMemoryGetState(keyfile);
         break;
         case OS_MAP_ON_HEAP:
             state = os_heap_sharedMemoryGetState(sharedHandle->id);
         break;
         }
     }

     return state;
}

os_result
os_sharedMemorySetState(
    os_sharedHandle sharedHandle,
    os_state state)
{
     os_result result = os_resultUnavailable;
     char *keyfile;

     assert(sharedHandle != NULL);

     keyfile = getKeyfileFromHandle(sharedHandle);
     if ((keyfile != NULL) || (sharedHandle->attr.sharedImpl == OS_MAP_ON_HEAP)) {
         switch (sharedHandle->attr.sharedImpl) {
         case OS_MAP_ON_FILE:
             result = os_posix_sharedMemorySetState(keyfile, state);
         break;
         case OS_MAP_ON_SEG:
             result = os_svr4_sharedMemorySetState(keyfile, state);
         break;
         case OS_MAP_ON_HEAP:
             result = os_heap_sharedMemorySetState(sharedHandle->id, state);
         break;
         }
     }

     return result;
}

static char *
getKeyfileFromHandle(os_sharedHandle sharedHandle) {
    if (sharedHandle->keyfile == NULL) {
        switch (sharedHandle->attr.sharedImpl) {
        case OS_MAP_ON_FILE:
            sharedHandle->keyfile = os_posix_findKeyFileById(sharedHandle->id);
        break;
        case OS_MAP_ON_SEG:
            sharedHandle->keyfile = os_svr4_findKeyFileById(sharedHandle->id);
        break;
        case OS_MAP_ON_HEAP:
        break;
        }
    }
    return sharedHandle->keyfile;
}

static os_int32
findDomainByHandle (
    void *obj,
    os_iterActionArg arg)
{
    os_sharedHandle handle = (os_sharedHandle)arg;
    os__shmDomain shmDomain = (os__shmDomain)obj;

    if(shmDomain->handle == handle){
        return 1;
    }
    return 0;
}

OSPL_DIAG_OFF(sign-conversion)
static void
os_fd_set(
    int fd,
    fd_set *fdsetp)
{
    /* FD_SET gives a sign-conversion warning, suppressing it */
    FD_SET(fd, fdsetp);
}
OSPL_DIAG_ON(sign-conversion)

OSPL_DIAG_OFF(sign-conversion)
static int
os_fd_isset(
    int fd,
    fd_set *fdsetp)
{
    /* FD_ISSET gives a sign-conversion warning, suppressing it */
    return FD_ISSET(fd, fdsetp);
}
OSPL_DIAG_ON(sign-conversion)

#ifdef OSPL_SHM_PROCMON

static os_int32
findClientByHandle (
    void *obj,
    os_iterActionArg arg)
{
    os_sharedHandle handle = (os_sharedHandle)arg;
    os__clientShmDomain clientShmDomain = (os__clientShmDomain)obj;

    if(clientShmDomain->handle == handle){
        return 1;
    }
    return 0;
}

static void
addSockToSet(
    void *obj,
    os_iterActionArg arg)
{
    os__shmClient client = (os__shmClient)obj;
    os__shmClients tc = (os__shmClients)arg;

    os_fd_set(client->fd, &tc->rset);

    if(client->fd > tc->maxSock){
        tc->maxSock = client->fd;
    }
    return;
}


static void
determineTerminatedClients(
    void *obj,
    os_iterActionArg arg)
{
    os__shmClient client = (os__shmClient)obj;
    os__shmClients tc = (os__shmClients)arg;
    char buffer[32];
    os_ssize_t n;

    if(os_fd_isset(client->fd, &tc->rset)){
        memset(buffer, 0, sizeof(buffer));
        n = read(client->fd, buffer, sizeof(buffer));
        if (n < 0) {
            OS_REPORT(OS_ERROR,"Spliced::SHM Process Monitor",0,
                        "read() from client socket failed = (%"PA_PRIuSIZE")\n",n);
        } else if(n == 0){
            client->state = OS_SHM_PROC_TERMINATED;

            if(!os_iterContains(tc->diff, client)){
                os_iterAppend(tc->diff, client);
            }
            TRACE_PKILL("Process crashed or got killed: '%d'\n", client->procId);
        } else {
            client->state = OS_SHM_PROC_DETACHED;

            if(!os_iterContains(tc->diff, client)){
                os_iterAppend(tc->diff, client);
            }
            TRACE_PKILL("Process detaching: '%s' (should match: '%d')\n",buffer, client->procId);
        }
        if (close(client->fd) < 0) {
            OS_REPORT(OS_ERROR,"Spliced::SHM Process Monitor",0,
                        "close(client socket) failed with errno (%d)", os_getErrno());
        }
    } else {
        /* Still alive */
        os_iterAppend(tc->temp, client);
    }
    return;
}

#define SHMSOCKNAME "/tmp/osplsock_%s"


static void*
sharedMemoryProcessMonitor(
    void* data)
{
    os__shmDomain shmDomain;
    int rc, clientSocket;
    os_ssize_t n;
    os_size_t len;
    struct sockaddr_un serveraddr;
    char *path;
    char buffer[32];
    os__shmClient client, newClient;
    struct timeval timeOut;
    int selectResult;
    os_iter temp;
    char* keyFileNameIdentifier = NULL;
    shmDomain = (os__shmDomain)data;

    keyFileNameIdentifier = os_svr4_findKeyFileIdentifierByIdAndName(shmDomain->handle->id, shmDomain->handle->name);
    if (keyFileNameIdentifier == NULL) {
        OS_REPORT_WID(OS_ERROR,"Spliced::SHM Process Monitor",0,shmDomain->handle->id,
                    "os_svr4_findKeyFileIdentifierByIdAndName failed/n");
        rc = -1;
        goto err_path_alloc;
    }

    len = strlen(keyFileNameIdentifier)+strlen(SHMSOCKNAME);
    if (len > sizeof(serveraddr.sun_path)) {
        OS_REPORT_WID(OS_ERROR,"Spliced::SHM Process Monitor",0,shmDomain->handle->id,
                    "name length (%"PA_PRIuSIZE") exceeds buffer size (%"PA_PRIuSIZE")",
                    len, sizeof(serveraddr.sun_path));
        rc = -1;
        goto err_path_alloc;
    }

    path = os_malloc(len);
    if (path == NULL) {
        OS_REPORT_WID(OS_ERROR,"Spliced::SHM Process Monitor",0,shmDomain->handle->id,
                    "os_malloc(%"PA_PRIuSIZE") failed",
                    len);
        rc = -1;
        goto err_path_alloc;
    }

    sprintf(path, SHMSOCKNAME, keyFileNameIdentifier);
    os_free(keyFileNameIdentifier);
    keyFileNameIdentifier = NULL;

    shmDomain->osplProcMonSock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (shmDomain->osplProcMonSock < 0){
        OS_REPORT_WID(OS_ERROR,"Spliced::SHM Process Monitor",0,shmDomain->handle->id,
                    "socket(AF_UNIX, SOCK_STREAM, 0) failed = (%d)",
                    shmDomain->osplProcMonSock);
        rc = -1;
        goto err_socket;
    }
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sun_family = AF_UNIX;
    strcpy(serveraddr.sun_path, path);

    rc = bind(shmDomain->osplProcMonSock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

    if (rc < 0){
        TRACE_PKILL("Bind failed try to close and reopen socket...\n");
        close(shmDomain->osplProcMonSock);
        rc = unlink(path);
        if (rc < 0) {
            OS_REPORT_WID(OS_ERROR,"Spliced::SHM Process Monitor",0,shmDomain->handle->id,
                        "unlink failed = (%d), probably inuse by other service, "
                        "service failed to startup.", rc);
            goto err_socket;
        }

        shmDomain->osplProcMonSock = socket(AF_UNIX, SOCK_STREAM, 0);
        if (shmDomain->osplProcMonSock < 0){
            OS_REPORT_WID(OS_ERROR,"Spliced::SHM Process Monitor",0,shmDomain->handle->id,
                        "socket(AF_UNIX, SOCK_STREAM, 0) failed = (%d)/n",
                        shmDomain->osplProcMonSock);
            rc = -1;
            goto err_socket;
        }
        rc = bind(shmDomain->osplProcMonSock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    }

    if (rc < 0){
        OS_REPORT_WID(OS_ERROR,"Spliced::SHM Process Monitor",0,shmDomain->handle->id,
                    "bind(server-socket) failed = (%d)/n", rc);
        goto err_bind;
    }
    rc = listen(shmDomain->osplProcMonSock, 100);

    if (rc< 0){
        OS_REPORT_WID(OS_ERROR,"Spliced::SHM Process Monitor",0,shmDomain->handle->id,
                    "listen(server-socket) failed = (%d)/n", rc);
        goto err_bind;
    }

    /* Signal the os_sharedMemoryProcessMonitorInit operation that
     * this thread is up and running.
     */
    os_mutexLock(&os__shmAttachedLock);
    (void)os_condSignal(&os__shmAttachCond);
    os_mutexUnlock(&os__shmAttachedLock);

    while(!shmDomain->terminate) {
        FD_ZERO(&shmDomain->clnts->rset);
        shmDomain->clnts->maxSock = shmDomain->osplProcMonSock;

        os_fd_set(shmDomain->osplProcMonSock, &shmDomain->clnts->rset);
        os_iterWalk(shmDomain->clnts->runningClients, addSockToSet, shmDomain->clnts);

        timeOut.tv_sec = 0;
        timeOut.tv_usec = 10 * 1000; /* 10 ms */

        selectResult = select(shmDomain->clnts->maxSock+1, &shmDomain->clnts->rset, NULL, NULL, &timeOut);

        if(selectResult < 0){
            OS_REPORT_WID(OS_ERROR,"Spliced::SHM Process Monitor",0,shmDomain->handle->id,
                        "select(client-sockets) failed = (%d)/n", selectResult);
            break;
        }

        if(selectResult > 0){
            if(os_fd_isset(shmDomain->osplProcMonSock, &shmDomain->clnts->rset)){
                clientSocket = accept(shmDomain->osplProcMonSock, NULL, NULL);

                if (clientSocket < 0){
                    OS_REPORT_WID(OS_ERROR,"Spliced::SHM Process Monitor",0,shmDomain->handle->id,
                                "accept(server-socket) failed = (%d)/n", clientSocket);
                    break;
                }
                memset(buffer, 0, sizeof(buffer));
                n = read(clientSocket, buffer, sizeof(buffer));
                if (n < 0) {
                    OS_REPORT_WID(OS_ERROR,"Spliced::SHM Process Monitor",0,shmDomain->handle->id,
                                "read(client-socket) failed = (%"PA_PRIuSIZE")", n);
                    break;
                }
                TRACE_PKILL("Process attaching: '%s' with socket %d\n",buffer, clientSocket);

                client = (os__shmClient)(os_malloc(OS_SIZEOF(os__shmClient)));
                newClient = (os__shmClient)(os_malloc(OS_SIZEOF(os__shmClient)));

                if(!client || !newClient){
                    OS_REPORT_WID(OS_ERROR,"Spliced::SHM Process Monitor",0,shmDomain->handle->id,
                                "malloc(%"PA_PRIuSIZE") failed",
                                OS_SIZEOF(os__shmClient) + OS_SIZEOF(os__shmClient));
                    break;
                }

                if(sscanf(buffer, "%d", &(client->procId)) != 1){
                    OS_REPORT_WID(OS_ERROR,"Spliced::SHM Process Monitor",0,shmDomain->handle->id,
                              "sscanf(client-procId) failed, received data is ignored");
                    os_free(client);
                    os_free(newClient);
                } else {
                    client->fd = clientSocket;
                    client->state = OS_SHM_PROC_ATTACHED;

                    newClient->procId = client->procId;
                    newClient->fd = client->fd;
                    newClient->state = client->state;

                    os_mutexLock(&os__shmAttachedLock);
                    os_iterInsert(shmDomain->clnts->runningClients, client);
                    os_iterAppend(shmDomain->clnts->diff, newClient);
                    os_mutexUnlock(&os__shmAttachedLock);
                    selectResult--;
                    /* Signal the os_sharedMemoryWaitForClientChanges operation about
                     * the detected clients side change.
                     */
                    os_mutexLock(&os__shmAttachedLock);
                    os_condSignal(&os__shmAttachCond);
                    os_mutexUnlock(&os__shmAttachedLock);
                }
            }

            if(selectResult > 0){
                os_mutexLock(&os__shmAttachedLock);
                shmDomain->clnts->temp = os_iterNew(NULL);
                os_iterWalk(shmDomain->clnts->runningClients, determineTerminatedClients, shmDomain->clnts);
                temp = shmDomain->clnts->runningClients;
                shmDomain->clnts->runningClients = shmDomain->clnts->temp;
                shmDomain->clnts->temp = NULL;
                os_mutexUnlock(&os__shmAttachedLock);
                os_iterFree(temp);
            }
        }
    }

    TRACE_PKILL("Closing socket...\n");
err_bind:
    (void)close(shmDomain->osplProcMonSock);
    (void)unlink(path);
err_socket:
    os_free(path);
    shmDomain->osplProcMonSock = -1;
err_path_alloc:

    os_free(keyFileNameIdentifier);
    if (rc < 0) {
        /* Initialization failed so signal the os_sharedMemoryProcessMonitorIniti
         * operation that this thread has failed to start.
         */
        os_mutexLock(&os__shmAttachedLock);
        (void)os_condSignal(&os__shmAttachCond);
        os_mutexUnlock(&os__shmAttachedLock);
    }
    return NULL;
}


static os_result
os_sharedMemoryProcessMonitorInit(
    os_sharedHandle sharedHandle)
{
    os_result result = os_resultFail;
    os_threadAttr attr;
    os__shmDomain shmDomain;

    shmDomain = (os__shmDomain)(os_malloc(OS_SIZEOF(os__shmDomain)));

    if(shmDomain){
        shmDomain->handle = sharedHandle;
        shmDomain->osplProcMonSock = -1;
        shmDomain->terminate = 0;
        shmDomain->clnts = (os__shmClients)(os_malloc(OS_SIZEOF(os__shmClients)));

        if(shmDomain->clnts){
            shmDomain->clnts->temp = NULL;
            shmDomain->clnts->runningClients = os_iterNew(NULL);

            if(shmDomain->clnts->runningClients){
                shmDomain->clnts->diff = os_iterNew(NULL);

                if(shmDomain->clnts->diff){

                    os_mutexLock(&os__shmAttachedLock);
                    os_iterInsert(os__shmMonitoring, shmDomain);

                    os_threadAttrInit(&attr);
                    result = os_threadCreate(&shmDomain->procMonTid,
                            "OsShmMonitor", &attr,
                            sharedMemoryProcessMonitor, shmDomain);
                    if(result == os_resultSuccess){
                        os_condWait(&os__shmAttachCond, &os__shmAttachedLock);
                        if (shmDomain->osplProcMonSock == -1) {
                            result = os_resultFail;
                        }
                    }
                    os_mutexUnlock(&os__shmAttachedLock);
                }
            }
        }

        if(result == os_resultFail){
            if(shmDomain->clnts){
                if(shmDomain->clnts->runningClients){
                    os_iterFree(shmDomain->clnts->runningClients);
                }
                if(shmDomain->clnts->diff){
                    os_iterFree(shmDomain->clnts->diff);
                }
                os_free(shmDomain->clnts);
            }
            os_free(shmDomain);
        }
    }
    return result;
}

static os_result
os_sharedMemoryProcessMonitorDeinit(
    os_sharedHandle sharedHandle)
{
    os_result result;
    os__shmDomain shmDomain;

    os_mutexLock(&os__shmAttachedLock);
    shmDomain = (os__shmDomain)(os_iterTakeAction(os__shmMonitoring, findDomainByHandle, sharedHandle));
    os_mutexUnlock(&os__shmAttachedLock);

    if(shmDomain && shmDomain->osplProcMonSock != -1){
        shmDomain->terminate = 1;
        TRACE_PKILL("Waiting for thread to exit...\n");
        os_threadWaitExit(shmDomain->procMonTid, NULL);
        TRACE_PKILL("Thread exited...\n");

        if(os_iterLength(shmDomain->clnts->runningClients) != 0){
            TRACE_PKILL("Some clients are still connected.\n");
        }
        os_iterFree(shmDomain->clnts->runningClients);
        os_iterFree(shmDomain->clnts->diff);
        os_free(shmDomain->clnts);
        os_free(shmDomain);
        result = os_resultSuccess;
    } else {
        result = os_resultFail;
    }
    return result;
}

static ssize_t send_nosigpipe(int fd, const void *buffer, size_t size, int flags)
{
#ifdef MSG_NOSIGNAL
    return send(fd, buffer, size, flags | MSG_NOSIGNAL);
#else
    sigset_t sset_before, sset_omask, sset_pipe, sset_after;
    ssize_t res;
    sigemptyset(&sset_pipe);
    sigaddset(&sset_pipe, SIGPIPE);
    sigpending(&sset_before);
    pthread_sigmask(SIG_BLOCK, &sset_pipe, &sset_omask);
    res = send(fd, buffer, size, flags);
    sigpending(&sset_after);
    if (!sigismember(&sset_before, SIGPIPE) && sigismember(&sset_after, SIGPIPE)) {
        /* sigtimedwait appears to be fairly well supported, just not by Mac OS. If other platforms prove to be a problem, we can do a proper indication of platform support in the os defs. The advantage of sigtimedwait is that it protects against a deadlock when SIGPIPE is sent from outside the program and all threads have it blocked. In any case, when SIGPIPE is sent in this manner and we consume the signal here, the signal is lost. Nobody should be abusing system-generated signals in this manner. */
#ifndef __APPLE__
        struct timespec timeout = { 0, 0 };
        sigtimedwait(&sset_pipe, NULL, &timeout);
#else
        int sig;
        sigwait(&sset_pipe, &sig);
#endif
#ifndef NDEBUG
        sigpending(&sset_after);
        assert(!sigismember(&sset_after, SIGPIPE));
#endif
        os_setErrno(EPIPE);
        res = -1;
    }
    pthread_sigmask(SIG_SETMASK, &sset_omask, NULL);
    return res;
#endif
}


static void
os__sharedMemoryClientShmDomainFree(
    os__clientShmDomain clientShmDomain,
    os_boolean waitForExit)
{
    if (clientShmDomain != NULL) {
        os_mutexLock(&clientShmDomain->mutex);
        if (clientShmDomain->monitorRunning) {
            clientShmDomain->onServerDied = NULL;
            if (clientShmDomain->mySock >= 0) {
                if (shutdown(clientShmDomain->mySock, SHUT_WR) < 0) {
                    OS_REPORT_WID(OS_ERROR, "os__sharedMemoryClientShmDomainFree", 0, clientShmDomain->handle->id,
                            "close(client socket failed with errno (%d)", os_getErrno());
                }
            }
        }
        os_mutexUnlock(&clientShmDomain->mutex);

        if (waitForExit && clientShmDomain->monitorThread) {
            (void) os_threadWaitExit(clientShmDomain->monitorThread, NULL);
        }
        if (clientShmDomain->mySock != -1) {
            close(clientShmDomain->mySock);
        }
        os_mutexDestroy(&clientShmDomain->mutex);
        os_free(clientShmDomain);
    }
}

static os__clientShmDomain
os__sharedMemoryClientShmDomainNew(
    os_sharedHandle sharedHandle)
{
    os_result result;
    os__clientShmDomain clientShmDomain;

    clientShmDomain = (os__clientShmDomain)(os_malloc(sizeof *clientShmDomain));

    clientShmDomain->handle = sharedHandle;
    clientShmDomain->mySock = -1;
    clientShmDomain->monitorRunning = FALSE;
    clientShmDomain->detaching = FALSE;
    clientShmDomain->onServerDied = NULL;
    clientShmDomain->args = NULL;
    clientShmDomain->monitorThread = OS_THREAD_ID_NONE;

    result = os_mutexInit(&clientShmDomain->mutex, NULL);

    if (result == os_resultSuccess) {
        clientShmDomain->mySock = socket(AF_UNIX, SOCK_STREAM, 0);
        if (clientShmDomain->mySock < 0){
            OS_REPORT_WID(OS_WARNING,"os__sharedMemoryClientShmDomainNew",0,sharedHandle->id,
                        "socket(AF_UNIX, SOCK_STREAM, 0) failed = (%d)",
                        clientShmDomain->mySock);
            os__sharedMemoryClientShmDomainFree(clientShmDomain, FALSE);
            clientShmDomain = NULL;
        }
    } else {
        os_free(clientShmDomain);
        clientShmDomain = NULL;
    }

    return clientShmDomain;
}

static os_result
os_sharedMemoryRegisterProcess(
    os_sharedHandle sharedHandle)
{
    struct sockaddr_un serveraddr;
    char *path = NULL;
    char buffer[32];
    int rc;
    int retries = 0;
    os_size_t len = 0;
    os__clientShmDomain clientShmDomain;
    os_result result = os_resultSuccess;
    os_duration delay = 100*OS_DURATION_MILLISECOND;

    char* keyFileNameIdentifier = NULL;

    keyFileNameIdentifier = os_svr4_findKeyFileIdentifierByIdAndName(sharedHandle->id, sharedHandle->name);
    /* Following retry loop is required because the service is still in the startup phase.
     * Can be removed in case the creation of a participant will block until the service
     * is operational.
     */
    while ((keyFileNameIdentifier == NULL) && (retries < 100)) /* give 100*100ms=10s time for the service to startup */
    {
        (void) ospl_os_sleep(delay);
        keyFileNameIdentifier = os_svr4_findKeyFileIdentifierByIdAndName(sharedHandle->id, sharedHandle->name);
        retries++;
    }
    if (keyFileNameIdentifier == NULL){
        OS_REPORT_WID(OS_WARNING,"os_sharedMemoryRegisterProcess",0,sharedHandle->id,
                    "os_svr4_findKeyFileIdentifierByIdAndName failed");
        result = os_resultFail;
    } else {
        len = strlen(keyFileNameIdentifier)+strlen(SHMSOCKNAME);
        if ((len > sizeof(serveraddr.sun_path)) || (len == 0)) {
        OS_REPORT_WID(OS_ERROR,"os_sharedMemoryRegisterProcess",0,sharedHandle->id,
                        "name length (%lu) exceeds buffer size (%d)",
                        (unsigned long) len, (int) sizeof(serveraddr.sun_path));
            result = os_resultFail;
        }
    }
    if (result == os_resultSuccess) {
        path = os_malloc(len);
        if (path == NULL) {
            OS_REPORT_WID(OS_ERROR,"os_sharedMemoryRegisterProcess",0,sharedHandle->id,
                        "os_malloc(%"PA_PRIuSIZE") failed",
                        len);
            result = os_resultFail;
        }
    }

    if (result == os_resultSuccess) {
        sprintf(path, SHMSOCKNAME, keyFileNameIdentifier);
        os_free(keyFileNameIdentifier);
        keyFileNameIdentifier = NULL;

        clientShmDomain = os__sharedMemoryClientShmDomainNew(sharedHandle);
        if(clientShmDomain == NULL){
            OS_REPORT_WID(OS_WARNING,"os_sharedMemoryRegisterProcess",0,sharedHandle->id,
                         "Failed to create shared memory client");
            result = os_resultFail;
        }
    }

    if (result == os_resultSuccess) {
        memset(&serveraddr, 0, sizeof(serveraddr));
        serveraddr.sun_family = AF_UNIX;
        strcpy(serveraddr.sun_path, path);

        rc = connect(clientShmDomain->mySock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
        /* Following retry loop is required because the service is still in the startup phase.
         * Can be removed in case the creation of a participant will block until the service
         * is operational.
         */
        retries = 0;
        while ((rc < 0) && (retries < 100)) /* give 100*100ms=10s time for the service to startup */
        {
            (void) ospl_os_sleep(delay);
            rc = connect(clientShmDomain->mySock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
            retries++;
        }
        if (rc < 0){
            OS_REPORT_WID(OS_WARNING,"os_sharedMemoryRegisterProcess",0,sharedHandle->id,
                        "connect(client-socket) failed = (%d)", rc);
            os__sharedMemoryClientShmDomainFree(clientShmDomain, FALSE);
            result = os_resultFail;
        }
    }
    if (result == os_resultSuccess) {
        os_ssize_t n;
        memset(buffer, 0, sizeof(buffer));
        sprintf(buffer, "%d", os_procIdSelf());
        n = send_nosigpipe(clientShmDomain->mySock, buffer, sizeof(buffer), 0);

        if (n < 0){
            OS_REPORT_WID(OS_ERROR,"os_sharedMemoryRegisterProcess",0,sharedHandle->id,
                        "client-socket send(procId) to server failed = (%ld)", (long) n);
            os__sharedMemoryClientShmDomainFree(clientShmDomain, FALSE);
            result = os_resultFail;
        } else {
            os_iterInsert(os__shmAttached, clientShmDomain);
        }
    }
    os_free(path);
    os_free(keyFileNameIdentifier);
    return result;
}

static os_result
os_sharedMemoryDeregisterProcess(
    os_sharedHandle sharedHandle)
{
    char buffer[32];
    os_ssize_t rc;
    os_result result = os_resultSuccess;
    os__clientShmDomain clientShmDomain;
    int sock;

    os_mutexLock(&os__shmAttachedLock);
    clientShmDomain = (os__clientShmDomain)(os_iterTakeAction(os__shmAttached, findClientByHandle, sharedHandle));
    os_mutexUnlock(&os__shmAttachedLock);

    if (clientShmDomain) {
        os_mutexLock(&clientShmDomain->mutex);
        if (os_threadIdToInteger(clientShmDomain->monitorThread) != os_threadIdToInteger(os_threadIdSelf())) {
            clientShmDomain->onServerDied = NULL;
            sock = clientShmDomain->mySock;
            os_mutexUnlock(&clientShmDomain->mutex);
            if (sock != -1) {
                memset(buffer, 0, sizeof(buffer));
                sprintf(buffer, "%d", os_procIdSelf());
                rc = send_nosigpipe(sock, buffer, sizeof(buffer), 0);

                if (rc < 0) {
                    /* This can fail when the server is already gone. */
                    result = os_resultFail;
                }
            }
            os__sharedMemoryClientShmDomainFree(clientShmDomain, TRUE);
        } else {
            clientShmDomain->detaching = TRUE;
            os_mutexUnlock(&clientShmDomain->mutex);
        }
    }

    return result;
}

static void *
os_sharedMemoryWaitForServerChanges(
    void *arg)
{
    os_sharedHandle sharedHandle = *(os_sharedHandle *) arg;
    os__clientShmDomain clientShmDomain = NULL;
    int sock = -1;
    fd_set rset;
    int selectResult;
    os_onSharedMemoryManagerDiedCallback onServerDied = NULL;
    os_boolean detaching;
    void *callbackArgs = NULL;

    os_mutexLock(&os__shmAttachedLock);
    clientShmDomain = (os__clientShmDomain)(os_iterReadAction(os__shmAttached, findClientByHandle, sharedHandle));
    if (clientShmDomain != NULL) {
        os_mutexLock(&clientShmDomain->mutex);
        sock = clientShmDomain->mySock;
        os_mutexUnlock(&clientShmDomain->mutex);
    }
    os_mutexUnlock(&os__shmAttachedLock);

    if (sock >= 0) {
        FD_ZERO(&rset);
        os_fd_set(sock, &rset);

        do { /* Retry select if interrupted by a signal */
            selectResult = select(sock+1, &rset, NULL, NULL, NULL);
        } while (selectResult == -1 && os_getErrno() == EINTR);

        if (selectResult > 0) {
            os_mutexLock(&clientShmDomain->mutex);
            onServerDied = clientShmDomain->onServerDied;
            callbackArgs = clientShmDomain->args;
            clientShmDomain->onServerDied = NULL;
            os_mutexUnlock(&clientShmDomain->mutex);
            if (onServerDied) {
                onServerDied(sharedHandle, callbackArgs);
            }
        }
    }

    if (clientShmDomain != NULL) {
        os_mutexLock(&clientShmDomain->mutex);
        clientShmDomain->monitorRunning = FALSE;
        detaching = clientShmDomain->detaching;
        os_mutexUnlock(&clientShmDomain->mutex);
        if (detaching) {
            os__sharedMemoryClientShmDomainFree(clientShmDomain, FALSE);
        }
    }

    return NULL;
}

os_result
os_sharedMemoryRegisterServerDiedCallback(
    os_sharedHandle sharedHandle,
    os_onSharedMemoryManagerDiedCallback onServerDied,
    void *args)
{
    os_result result = os_resultSuccess;
    os__clientShmDomain clientShmDomain;
    os_threadAttr tAttr;

    os_mutexLock(&os__shmAttachedLock);
    clientShmDomain = (os__clientShmDomain)(os_iterReadAction(os__shmAttached, findClientByHandle, sharedHandle));

    if (clientShmDomain != NULL) {
        os_mutexLock(&clientShmDomain->mutex);
        if (!clientShmDomain->monitorRunning){
            clientShmDomain->onServerDied = onServerDied;
            clientShmDomain->args = args;

            os_threadAttrInit(&tAttr);
            result = os_threadCreate(
                    &clientShmDomain->monitorThread, "SharedMemoryServerMonitor",
                    &tAttr, os_sharedMemoryWaitForServerChanges, clientShmDomain);
            if (result != os_resultSuccess) {
                OS_REPORT_WID(OS_ERROR,"os_sharedMemoryRegisterServerDiedCallback",0,sharedHandle->id,
                          "Failed to start shared memory server monitor = (%ld)", (long) result);
            }
            else {
                clientShmDomain->monitorRunning = TRUE;
            }
        }
        os_mutexUnlock(&clientShmDomain->mutex);
    } else {
        OS_REPORT_WID(OS_ERROR,"os_sharedMemoryRegisterServerDiedCallback",0,sharedHandle->id,
                   "Failed to find client shared memory entry");
        result = os_resultUnavailable;
    }

    os_mutexUnlock(&os__shmAttachedLock);

    return result;
}

#else

os_result
os_sharedMemoryRegisterServerDiedCallback(
    os_sharedHandle sharedHandle,
    os_onSharedMemoryManagerDiedCallback onServerDied,
    void *args)
{
    return os_resultUnavailable;
}

#endif


void
os_shmClientFree(
    os_shmClient client)
{
    if(client){
        if(client->next){
            os_shmClientFree(client->next);
        }
        os_free(client);
    }
}

os_result
os_sharedMemoryWaitForClientChanges(
    os_sharedHandle sharedHandle,
    os_duration maxBlockingTime,
    os_shmClient* changedClients)
{
    os_result result;
    os__shmDomain domain;
    os__shmClient internalClient;
    os_shmClient last, temp;

    os_mutexLock(&os__shmAttachedLock);
    domain = (os__shmDomain)(os_iterReadAction(os__shmMonitoring, findDomainByHandle, sharedHandle));

    if(domain){
        assert(domain->clnts);

        if(os_iterLength(domain->clnts->diff) == 0){
            result = os_condTimedWait(&os__shmAttachCond, &os__shmAttachedLock, maxBlockingTime);
        } else {
            result = os_resultSuccess;
        }

        if((os_iterLength(domain->clnts->diff) > 0) && (result == os_resultSuccess)){
            last = NULL;
            internalClient = (os__shmClient)(os_iterTakeFirst(domain->clnts->diff));

            while(internalClient){
                temp = (os_shmClient)(os_malloc(OS_SIZEOF(os_shmClient)));
                if (temp) {
                    temp->procId = internalClient->procId;
                    temp->state = internalClient->state;
                    temp->next = NULL;

                    if(last){
                        last->next = temp;
                    } else if(changedClients){
                        *changedClients = temp;
                    }
                    last = temp;
                } else {
                    OS_REPORT_WID(OS_ERROR, "os_sharedMemoryWaitForClientChanges", 0, sharedHandle->id,
                              "Out of resources: os_malloc failed");
                }
                os_free(internalClient);
                internalClient = (os__shmClient)(os_iterTakeFirst(domain->clnts->diff));
            }
        } else if(changedClients){
            *changedClients = NULL;
        }
    } else {
        result = os_resultUnavailable;

        if(changedClients){
            *changedClients = NULL;
        }
    }
    os_mutexUnlock(&os__shmAttachedLock);

    return result;
}

/* Prevent race condition between creation and attach shared memory segment
 * create an exclusive lock file during creation.
 * Don block to long, a previous process could have created the lock file
 * and didn't clean it up properly due to an exception in creating shared memory segment
 */
os_result
os_sharedMemoryLock(
    os_sharedHandle sharedHandle)
{
    os_result result = os_resultUnavailable;
    const char *tmpDir;
    os_size_t len;
    int retry = 0;
    int fd;

    if (sharedHandle != NULL) {
        assert(sharedHandle->data);
        assert(sharedHandle->data->lockFileName == NULL);
        tmpDir = os_getTempDir();
        len = strlen(tmpDir) + strlen(os_key_file_creation_lock) + 2;
        sharedHandle->data->lockFileName = os_malloc(len);
        (void)snprintf (sharedHandle->data->lockFileName, len, "%s/%s", tmpDir, os_key_file_creation_lock);
        do {
            fd = open(sharedHandle->data->lockFileName, O_CREAT | O_EXCL, S_IRWXU | S_IRWXG | S_IRWXO);
            if (fd == -1) {
                ospl_os_sleep(OS_DURATION_SECOND/2);
            }
        } while ((fd == -1) && (retry++ < 8));
        if (fd == -1) {
            result = os_resultFail;
        } else {
            if (close(fd) == -1) {
                OS_REPORT(OS_ERROR, OS_FUNCTION, 0,
                          "Failed to close exclusive lock file: %s", os_strError(os_getErrno()));
            }
            result = os_resultSuccess;
        }
    }
    return result;
}

void
os_sharedMemoryUnlock(
    os_sharedHandle sharedHandle)
{
    if (sharedHandle &&
        sharedHandle->data &&
        sharedHandle->data->lockFileName) {
        (void)remove(sharedHandle->data->lockFileName);
        os_free(sharedHandle->data->lockFileName);
        sharedHandle->data->lockFileName = NULL;
    }
}

void
os_sharedMemoryImplDataCreate(
    os_sharedHandle sharedHandle)
{
    assert(sharedHandle);
    sharedHandle->data = os_malloc(OS_SIZEOF(os_implData));
    sharedHandle->data->lockFileName = NULL;
}

void
os_sharedMemoryImplDataDestroy(
    os_sharedHandle sharedHandle)
{
    assert(sharedHandle);
    if (sharedHandle->data &&
        sharedHandle->data->lockFileName) {
        os_free(sharedHandle->data->lockFileName);
    }
    os_free(sharedHandle->data);
}
