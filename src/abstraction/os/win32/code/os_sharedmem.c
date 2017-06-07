/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
/** \file os/win32/code/os_sharedmem.c
 *  \brief WIN32 shared memory management
 *
 * Implements shared memory management for WIN32
 */

#include "os_sharedmem.h"

#include "os_win32incs.h"
#include <Sddl.h>

#include "os_stdlib.h"
#include "os_abstract.h"
#include "os_report.h"
#include "os_process.h"
#include "os__process.h"
#include "os_heap.h"
#include "os_thread.h"
#include "os_mutex.h"
#include "os_cond.h"
#include "os_init.h"
#include "os__debug.h"
#include "os_iterator.h"    /*2008 */
#include "os_signal.h"  /* 2008 */
#include "os_socket.h"
#include "os_errno.h"
#define OSPL_SHM_PROCMON
#include <assert.h>
#include <stdio.h>
#if 0
  #define TRACE_PKILL printf
#else
  #define TRACE_PKILL(...)
#endif

#ifdef OSPL_SHM_PROCMON

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#define USE_PIPE

static os_result
os_sharedMemoryProcessMonitorInit(
    os_sharedHandle sharedHandle);

static os_result
os_sharedMemoryProcessMonitorDeinit(
    os_sharedHandle sharedHandle);

static os_result
os_sharedMemoryRegisterProcess(
    os_sharedHandle);

static os_result
os_sharedMemoryDeregisterProcess(
    os_sharedHandle);

static void
os_sharedMemoryTerminateMonitor(
    os_sharedHandle);

static char *
os_sharedMemoryHandleGetKeyfile(
    os_sharedHandle);

typedef enum {
    CLIENT_PIPE_STATE_CONNECTING = 0,
    CLIENT_PIPE_STATE_READING,
    CLIENT_PIPE_STATE_WRITING
} ClientPipeState_t;


#define _PIPE_DEFAULT_TIMEOUT  5000 /* milliseconds */


#define BUFSIZE 10
#define MAX_NUM_CLIENT_CONNECTIONS 8

enum shmMessage
{
    PROCESS_CONNECT = 'C',
    PROCESS_DISCONNECT,
    MONITOR_EXIT
};

typedef enum {
    OS_SHM_MESSAGE_CLIENT_CONNECT_IND,
    OS_SHM_MESSAGE_CLIENT_CONNECT_RSP,
    OS_SHM_MESSAGE_CLIENT_DISCONNECT,
    OS_SHM_MESSAGE_MONITOR_EXIT,
    OS_SHM_MESSAGE_OK,
    OS_SHM_MESSAGE_ERROR
} os_shmMessageKind_t;

struct os_shmMessage {
    os_shmMessageKind_t kind;
    union {
        os_procId procId;
    } _u;
};

#endif

OS_CLASS(os__shmClient);
OS_STRUCT(os__shmClient){
    os_procId procId;
    os_shmProcState state;
};

OS_CLASS(os__shmClients);
OS_STRUCT(os__shmClients) {
    os_iter runningClients;
    os_iter diff;
    os_iter temp;
    fd_set rset;
};

OS_CLASS(os__shmDomain);
OS_STRUCT(os__shmDomain){
    os_sharedHandle handle;
    os_threadId procMonTid;
    int terminate;
    os__shmClients clnts;
};

OS_CLASS(os__clientShmDomain);
OS_STRUCT(os__clientShmDomain){
    os_sharedHandle shmhandle;
    HANDLE hWaitHandle;
    os_procId serverId;
    os_onSharedMemoryManagerDiedCallback onServerDied;
    void *args;
};


typedef struct
{
    HANDLE procHandle;
    os__shmDomain shmDomain;
} os__threadInfo;

/* Shared memory segments I am attached to */
static os_iter os__shmAttached = NULL;
/* Shared memory segments I am monitoring */
static os_iter os__shmMonitoring = NULL;
/** Mutex for locking the shared memory data */
static os_mutex os__shmAttachedLock;

/** condition variable for synchronisation
 *  twofold:
 *  1. to delay return of shared memory create call until the shared memory monitor thread is running.
 *  2. to signal the spliced that the shared memory monitor thread has detected a client side change.
 */
static os_cond os__shmAttachCond;

#define OS_LOG_AUTH     LOG_AUTH
#define OS_LOG_USER     LOG_USER
#define OS_LOG_WARNING  LOG_WARNING
#define OS_KEYFILE_LINE_SIZE_MAX    (256)
#define OS_KEYFILE_DOMAIN_NAME_SIZE (OS_KEYFILE_LINE_SIZE_MAX)
#define OS_KEYFILE_IMPL_ID_SIZE     (OS_KEYFILE_LINE_SIZE_MAX)
#define OS_KEYFILE_INT_ARRAY_SIZE   (3)

/** Defines the file prefix for the key file */
const char *key_file_path = NULL;
const char key_file_prefix[] = "osp";


typedef struct os_win32_pid_info {
    os_int32 pid;
    FILETIME time;
} os_win32_pid_info;

/** \brief Structure providing keyfile data
 *
 */
typedef struct os_win32_keyfile_data {
    os_int32   domain_id;
    os_char    domain_name[OS_KEYFILE_DOMAIN_NAME_SIZE];
    os_address address;
    os_address size;
    os_char    implementation_id[OS_KEYFILE_IMPL_ID_SIZE];
    os_win32_pid_info creator_pid;
} os_win32_keyfile_data;

struct os_sharedHandle_s {
    os_sharedAttr attr;
    void *mapped_address;
    char *name;
    char *key;
    HANDLE dataFile;
    HANDLE mapObject;
    os_address size;
    os_int32 shm_created;
    os_int32 id;
    char *keyfile;
};

struct os_shmInfo {
    os_sharedHandle sharedHandle;
    struct os_shmInfo *next;
};

struct os_sharedMemoryClientConnection {
    OVERLAPPED           overlap;
    HANDLE               pipeInstance;
    struct os_shmMessage request;
    DWORD                cbRead;
    struct os_shmMessage reply;
    ClientPipeState_t    state;
    BOOL                 ioPending;
};


/*
 * Return  os_resultSuccess - Complete action succeeded, no more calls expected
 *         os_resultBusy    - Action succeded, but expects more (or none) calls
 *         other            - Failure
 */
typedef os_result
(*os_win32_keyfile_loop_action)(const char *key_file_name, os_win32_keyfile_data *data, void *action_arg);


os_result
os_win32_keyfile_getAddress(
    FILE *key_file,
    os_address *data)
{
    char line[OS_KEYFILE_LINE_SIZE_MAX];
    os_result rv = os_resultFail;
    if (fgets(line, OS_KEYFILE_LINE_SIZE_MAX, key_file) != NULL) {
        if (sscanf(line, PA_ADDRFMT, (PA_ADDRCAST *)data) == 1) {
            rv = os_resultSuccess;
        }
    }
    return rv;
}

os_result
os_win32_keyfile_getInt(
    FILE *key_file,
    int *data)
{
    char line[OS_KEYFILE_LINE_SIZE_MAX];
    os_result rv = os_resultFail;
    if (fgets(line, OS_KEYFILE_LINE_SIZE_MAX, key_file) != NULL) {
        if (sscanf(line, "%d", data) == 1) {
            rv = os_resultSuccess;
        }
    }
    return rv;
}

os_result
os_win32_keyfile_getString(
    FILE *key_file,
    os_char *data,
    int data_size)
{
    os_result rv = os_resultFail;
    char *nl;
    if (fgets(data, data_size, key_file) != NULL) {
        /* Remove trailing newline char */
        if (nl = strchr(data, '\n')) {
            *nl = 0;
        }
        rv = os_resultSuccess;
    }
    return rv;
}

int
os_win32_keyfile_getIntArray(
    FILE *key_file,
    int *intArray,
    int size)
{
    char line[OS_KEYFILE_LINE_SIZE_MAX];
    char *safePtr;
    char *intStr;
    int cnt = 0;
    if (fgets(line, OS_KEYFILE_LINE_SIZE_MAX, key_file) != NULL) {
        for (intStr = os_strtok_r(line, " ", &safePtr);
                (intStr != NULL) && (cnt < size);
                   intStr = os_strtok_r(NULL, " ", &safePtr)) {
            if (sscanf(intStr, "%d", &(intArray[cnt])) == 1) {
                cnt++;
            }
        }
    }
    return cnt;
}

os_result
os_win32_keyfile_getState(
    FILE *key_file,
    os_state *data)
{
    os_result rv = os_resultFail;
    int pids[OS_KEYFILE_INT_ARRAY_SIZE];
    int cnt;
    /* Try getting next state. */
    while ((cnt = os_win32_keyfile_getIntArray(key_file, pids, OS_KEYFILE_INT_ARRAY_SIZE)) != 0) {
        /* Valid state has two ints to distinguish it from a pid info. */
        if (cnt == 2) {
            *data = pids[0];
            /* We've found the next state: stop searching. */
            rv = os_resultSuccess;
            break;
        }
    }
    return rv;
}

os_result
os_win32_keyfile_getPidInfo(
    FILE *key_file,
    os_win32_pid_info *data)
{
    os_result rv = os_resultFail;
    int pids[OS_KEYFILE_INT_ARRAY_SIZE];
    int cnt;
    /* Try getting next pid info. */
    while ((cnt = os_win32_keyfile_getIntArray(key_file, pids, OS_KEYFILE_INT_ARRAY_SIZE)) != 0) {
        /* Valid pid info has one or three ints. */
        if (cnt == 1) {
            data->pid = pids[0];
            data->time.dwLowDateTime = 0;
            data->time.dwHighDateTime = 0;
            /* We've found the next pid info: stop searching. */
            rv = os_resultSuccess;
            break;
        } else if (cnt == 3) {
            data->pid = pids[0];
            data->time.dwLowDateTime = pids[1];
            data->time.dwHighDateTime = pids[2];
            /* We've found the next pid info: stop searching. */
            rv = os_resultSuccess;
            break;
        }
    }
    return rv;
}




typedef struct os_win32_keyfile_dataArg {
    os_int32 id;
    const char *name;
    os_win32_keyfile_data data;
} os_win32_keyfile_dataArg;


static os_result
os_win32_getDataByIdAction(
    const char *key_file_name,
    os_win32_keyfile_data *data,
    void *action_arg)
{
    os_result rv = os_resultBusy; /* busy: keep searching */
    os_win32_keyfile_dataArg *arg = (os_win32_keyfile_dataArg*)action_arg;

    assert(arg);
    assert(data);
    assert(key_file_name);

    if (data->domain_id == arg->id) {
        memcpy(&(arg->data), data, sizeof(os_win32_keyfile_data));
        rv = os_resultSuccess; /* success: stop searching */
    }

    return rv;
}

static os_result
os_win32_getDataByNameAction(
    const char *key_file_name,
    os_win32_keyfile_data *data,
    void *action_arg)
{
    os_result rv = os_resultBusy; /* busy: keep searching */
    os_win32_keyfile_dataArg *arg = (os_win32_keyfile_dataArg*)action_arg;

    assert(arg);
    assert(data);
    assert(arg->name);
    assert(key_file_name);

    if (strcmp(data->domain_name, arg->name) == 0) {
        memcpy(&(arg->data), data, sizeof(os_win32_keyfile_data));
        rv = os_resultSuccess; /* success: stop searching */
    }

    return rv;
}



/** \brief Parse the keyfile into an os_win32_keyfile_data struct.
 *
 */
static os_result
os_win32_keyFileParser(
    FILE *key_file,
    os_win32_keyfile_data *data)
{
    os_result rv;

    assert(key_file);
    assert(data);

    /*
     * The win32 keyfile contains the following lines:
     *      - Domain name
     *      - Address
     *      - Size
     *      - Implementation id
     *      - Creator pid
     *      - Domain id
     *      - Attached pid
     *      - State
     *      - Attached pid
     *      - State
     *      - State
     *      - Attached pid
     *      - Attached pid
     *            etc
     */
    rv = os_win32_keyfile_getString(key_file, data->domain_name, OS_KEYFILE_DOMAIN_NAME_SIZE);
    if (rv == os_resultSuccess) {
        rv = os_win32_keyfile_getAddress(key_file, &(data->address));
    }
    if (rv == os_resultSuccess) {
        rv = os_win32_keyfile_getAddress(key_file, &(data->size));
    }
    if (rv == os_resultSuccess) {
        rv = os_win32_keyfile_getString(key_file, data->implementation_id, OS_KEYFILE_IMPL_ID_SIZE);
    }
    if (rv == os_resultSuccess) {
        rv = os_win32_keyfile_getPidInfo(key_file, &(data->creator_pid));
    }
    if (rv == os_resultSuccess) {
        rv = os_win32_keyfile_getInt(key_file, &(data->domain_id));
    }
    /* We ignore the attached PIDs and states during this parsing.
     * The function that is interrested in them, should call
     * os_win32_keyfile_getPidInfo or os_win32_keyfile_getState
     * itself, after this parsing. */

    return rv;
}

/** \brief Check if the contents of the identified key file
 *         matches the identified id and and set the name
 *
 * \b os_win32_loopKeyFiles tries to compare the contents of the identified
 * key file in \b key_file_name with the identified \b id.
 * On a match 1 will be returned and the domain name will be set,
 * on a mismatch 0 will be returned and name will be NULL.
 */
/*
 * Return  os_resultSuccess - The action function was called at least once and succeeded.
 *         other            - Failure or no calls to action.
 */
static os_result
os_win32_loopKeyFiles(
    os_win32_keyfile_loop_action action,
    void *action_arg)
{
    os_result rv = os_resultBusy;
    HANDLE fileHandle;
    WIN32_FIND_DATA fileData;
    char key_file_name[MAX_PATH];
    FILE *key_file;
    os_win32_keyfile_data data;
    int last = 0;

    key_file = NULL;

    if (key_file_path == NULL) {
        key_file_path = os_getTempDir();
    }

    if (key_file_path == NULL) {
        OS_REPORT(OS_ERROR, "os_win32_loopKeyFiles", 0, "Failed to determine temporary directory");
        return os_resultFail;
    }

    os_strcpy(key_file_name, key_file_path);
    os_strcat(key_file_name, "\\");
    os_strcat(key_file_name, key_file_prefix);
    os_strcat(key_file_name, "*.tmp");

    fileHandle = FindFirstFile(key_file_name, &fileData);

    if (fileHandle == INVALID_HANDLE_VALUE) {
        /* Try to communicate error message via thread-specific memory */
#define ERR_MESSAGE "os_win32_loopKeyFiles: Could not find any key file: %s"
        char *message;
        os_size_t messageSize;

        messageSize = sizeof(ERR_MESSAGE) + strlen(key_file_name);
        /* Free any existing thread specific warnings */
        os_threadMemFree(OS_THREAD_WARNING);
        message = (char *)os_threadMemMalloc(OS_THREAD_WARNING, (os_int32)messageSize, NULL, NULL);
        if (message) {
            snprintf(message, messageSize, ERR_MESSAGE, key_file_name);
        } else {
            /* Allocation failed, use report mechanism */
            OS_REPORT(OS_WARNING, "os_win32_loopKeyFiles", 0, "Could not find any key file: %s", key_file_name);
        }
#undef ERR_MESSAGE
        return os_resultFail;
    }

    /* Busy means continue searching. */
    while ((rv == os_resultBusy) && (last == 0)) {
        /* Get, open, parse and perform action on current KeyFile. */
        os_strcpy(key_file_name, key_file_path);
        os_strcat(key_file_name, "\\");
        os_strcat(key_file_name, fileData.cFileName);
        key_file = fopen(key_file_name, "r");
        if (key_file != NULL) {
            rv = os_win32_keyFileParser(key_file, &data);
            fclose(key_file);
            if (rv == os_resultSuccess) {
                rv = action(key_file_name, &data, action_arg);
            } else {
                /* The parsing failed. This can have a number of valid
                 * reasons:
                 *  - No rights; keyfile was created by another user.
                 *  - Not enough data; keyfile is being created right now.
                 *  - etc.
                 * Because we don't know the reason it failed, just asume
                 * that it is a valid reason and just continue. */
                rv = os_resultBusy;
                OS_REPORT(OS_INFO, "os_win32_loopKeyFiles", 0, "Failed to parse key file: %s", key_file_name);
            }
        } else {
            OS_REPORT(OS_INFO, "os_win32_loopKeyFiles", 0, "Failed to open key file: %s", key_file_name);
        }

        /* Get next file when we need to continue searching. */
        if (rv == os_resultBusy) {
            if (FindNextFile(fileHandle, &fileData) == 0) {
                /* We handled the last key file. */
                last = 1;
            }
        }
    }

    FindClose(fileHandle);

    return rv;
}



struct os_shmInfo *shmInfo = NULL;

static int os_isSharedMemGlobal = 1;

int
os_sharedMemIsGlobal(void)
{
    return os_isSharedMemGlobal;
}

void os_sharedMemoryInit(void)
{
    os_result result;

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

}

void os_sharedMemoryExit(void)
{
    os__shmDomain shmDomain;
    os_iter copyIter;

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

const char *
os_getDomainNameforMutex(
    os_mutex *mutex)
{
    /* get shm and look if pointer is in memory range */
    os_address base,size,mtx;
    struct os_shmInfo *shmInf;
    char *result = NULL;
    os_int32 foundName = 0;

    assert(mutex);

    for (shmInf = shmInfo;shmInf != NULL && foundName==0;shmInf = shmInf->next) {
        base = (os_address)shmInf->sharedHandle->mapped_address;
        size = shmInf->sharedHandle->size;
        mtx = (os_address)mutex;
        /* check if the mutex is in the shm range */
        if((base < mtx) && (mtx < (base+size) )) {
            result = shmInf->sharedHandle->name;
            foundName = 1;
        }
    }
    /* is no shm present name = null */
    return result;
}

const char*
os_getDomainNameforCond (
        os_cond* cond)
{
   /* get shm and look if pointer is in memory range */
   os_address base, size, cnd;
   struct os_shmInfo* shmInf;
   char* result = NULL;
   os_int32 foundName = 0;

   assert(cond);

   for (shmInf = shmInfo;shmInf != NULL && foundName==0;shmInf = shmInf->next)
   {
       base = (os_address)shmInf->sharedHandle->mapped_address;
       size = shmInf->sharedHandle->size;
       cnd = (os_address)cond;

       /* check if the cond is in the shm range */
       if((base < cnd) && (cnd < (base+size) )) {
           result = shmInf->sharedHandle->name;
           foundName =1;
       }
   }
   /* is no shm present name = null */
   return result;
}

/** \brief Get an unique key identifying a domain, used for naming semaphores and mutexes
 *
 * A multi-domain user application will map a SHM file per domain so OpenSplice needs to compare the pointer
 * address to the SHM mapping range to find out to which domain it belongs.
 * The splice daemon is responsible for semaphore/mutex creation, so it may call this function with NULL (ptr is not available yet).
 * The splice daemon and services are started separately so will always have at most one SHM mapping per process.
 * If multiple domains are running separately (no multi-domain applications) they may use the same SHM mapping range.
 * But Windows needs semaphores/mutexes to be created with unique names or they will be mixed up between domains.
 *
 * The key is identical to the unique filename created by GetTempFile during shared memory creation/attachment.
 * Some applications use the abstraction layer but don't use shared memory (like idlpp, odlpp that use the memory manager for heap memory),
 * so there is no key. In those cases the os_serviceName will be used as key. There is no sharing between processes thus the serviceName cannot change
 * between semaphore/mutex create and open calls.
 */
const os_char*
os_getShmDomainKeyForPointer(void *ptr) {
    struct os_shmInfo *shmPtr;
    os_address min, max, pointer;
    const os_char *result;

    result = NULL;

    if (shmInfo == NULL) {
        result = os_serviceName();
    } else {
        if (ptr == NULL) {
            assert(shmInfo->sharedHandle);
            result = shmInfo->sharedHandle->key;
        } else {
            for (shmPtr = shmInfo; shmPtr != NULL && result == NULL; shmPtr = shmPtr->next) {
                assert(shmPtr->sharedHandle);
                min = (os_address)shmPtr->sharedHandle->mapped_address;
                max = (os_address)shmPtr->sharedHandle->size + min;
                pointer = (os_address)ptr;
                if ((pointer >= min) && (pointer <= max)) {
                    result = shmPtr->sharedHandle->key;
                }
            }
        }

    }
    assert(result);
    return result;
}

/** \brief Create a handle for shared memory operations
 *
 * The identified \b name and \b sharedAttr values
 * are applied during creation of the shared memory.
 * The requested map address in \b sharedAttr is applied
 * during the attach function.
 */
os_sharedHandle
os_sharedCreateHandle(
    const char *name,
    const os_sharedAttr *sharedAttr,
    const int id)
{
    os_sharedHandle sh;

    assert(name != NULL);
    assert(sharedAttr != NULL);
    sh = (os_sharedHandle)(os_malloc (sizeof (struct os_sharedHandle_s)));

    if (sh != NULL) {
        sh->name = (char*)os_malloc(strlen(name) + 1);
        if (sh->name != NULL) {
            os_strcpy(sh->name, name);
            sh->attr = *sharedAttr;
            sh->key = NULL;
            sh->mapped_address = (void *)0;
            sh->dataFile = 0;
            sh->mapObject = 0;
            sh->size = 0;
            sh->shm_created = 0;
            sh->id = id;
            sh->keyfile = NULL;
        } else {
            os_free(sh);
            sh = NULL;
        }
    } else {
       OS_REPORT(OS_ERROR, "OS Abstraction", 0, "Could not create os_sharedHandle");
    }

    return sh;
}

/** \brief Destroy a handle for shared memory operations
 */
void
os_sharedDestroyHandle(
    os_sharedHandle sharedHandle)
{
    assert(sharedHandle != NULL);
    assert(sharedHandle->name != NULL);

    os_free(sharedHandle->name);
    os_free(sharedHandle->key);
    os_free(sharedHandle->keyfile);
    os_free(sharedHandle);
}

/** \brief Return the address of the attached shared memory
 *         related to the handle
 */
void *
os_sharedAddress(
    os_sharedHandle sharedHandle)
{
    assert(sharedHandle != NULL);
    return sharedHandle->mapped_address;
}


/** \brief Return the file-path of the key file related
 *         to the identified shared memory
 *
 * \b os_findKeyFile tries to find the key file related to \b name
 * in the \b temporary directory. The key files are prefixed with \b
 * /<temporary directory>/sp2v3key_.
 *
 * \b os_findKeyFile first opens the temporary directory by calling
 * \b opendir. Then it reads all entries in search for  any entry
 * that starts with the name \b sp2v3key_ by reading the entry with
 * \b readdir. If the a matching entry is found, it opens the file
 * and compares the contents with the required \b name. If the
 * \b name matches the contents, the entry is found, and the path
 * is returned to the caller. The memory for the path is allocated
 * from heap and is expected to be freed by the caller.
 *
 * If no matching entry is found, NULL is returned to the caller.
 */
typedef struct os_win32_findArg {
    os_int32 id;
    const char *name;
    char *key_file;
} os_win32_findArg;


static os_result
os_findKeyFileAction(
    const char *key_file_name,
    os_win32_keyfile_data *data,
    void *action_arg)
{
    os_result rv = os_resultBusy; /* busy: keep searching */
    os_win32_findArg *arg = (os_win32_findArg*)action_arg;

    assert(arg);
    assert(data);
    assert(arg->name);
    assert(key_file_name);

    if (strcmp(data->domain_name, arg->name) == 0) {
        arg->key_file = os_strdup(key_file_name);
        if (arg->key_file != NULL) {
            rv = os_resultSuccess; /* success: stop searching */
        } else {
            rv = os_resultFail; /* failure: stop searching */
        }
    }

    return rv;
}

char *
os_findKeyFile(
    const char *name)
{
    os_win32_findArg arg;
    arg.id = 0;
    arg.name = name;
    arg.key_file = NULL;
    (void)os_win32_loopKeyFiles(os_findKeyFileAction, (void*)&arg);
    return arg.key_file;
}



static os_result
os_findKeyFileByIdAction(
    const char *key_file_name,
    os_win32_keyfile_data *data,
    void *action_arg)
{
    os_result rv = os_resultBusy; /* busy: keep searching */
    os_win32_findArg *arg = (os_win32_findArg*)action_arg;

    assert(arg);
    assert(data);
    assert(key_file_name);

    if (data->domain_id == arg->id) {
        arg->key_file = os_strdup(key_file_name);
        if (arg->key_file != NULL) {
            rv = os_resultSuccess; /* success: stop searching */
        } else {
            rv = os_resultFail; /* failure: stop searching */
        }
    }

    return rv;
}

char *
os_findKeyFileById(
    os_int32 id)
{
    os_win32_findArg arg;
    arg.id = id;
    arg.name = NULL;
    arg.key_file = NULL;
    (void)os_win32_loopKeyFiles(os_findKeyFileByIdAction, (void*)&arg);
    return arg.key_file;
}


static os_result
os_findKeyFileByNameAndIdAction(
    const char *key_file_name,
    os_win32_keyfile_data *data,
    void *action_arg)
{
    os_result rv = os_resultBusy; /* busy: keep searching */
    os_win32_findArg *arg = (os_win32_findArg*)action_arg;

    assert(arg);
    assert(data);
    assert(arg->name);
    assert(key_file_name);

    if (data->domain_id == arg->id) {
        if (strcmp(data->domain_name, arg->name) == 0) {
            arg->key_file = os_strdup(key_file_name);
            if (arg->key_file != NULL) {
                rv = os_resultSuccess; /* success: stop searching */
            } else {
                rv = os_resultFail; /* failure: stop searching */
            }
        }
    }

    return rv;
}

char *
os_findKeyFileByNameAndId(
    const char *name,
    const os_int32 id)
{
    os_win32_findArg arg;
    arg.id = id;
    arg.name = name;
    arg.key_file = NULL;
    (void)os_win32_loopKeyFiles(os_findKeyFileByNameAndIdAction, (void*)&arg);
    return arg.key_file;
}






/** \brief Return list of processes defined in key file \b fileName
 *         as an iterator contained in \b pidList
 *
 * \b returns 0 on success and 1 if key file not found or unreadable
 */
os_int32
os_sharedMemoryListUserProcesses(
    os_iter pidList,
    const char * fileName)
{
    FILE *key_file;
    BOOL result;
    HANDLE hProcess;
    FILETIME creationTime;
    FILETIME exitTime;
    FILETIME kernelTime;
    FILETIME userTime;
    os_int32 retVal = 0;
    os_win32_keyfile_data data;
    os_win32_pid_info attachedPid;

    char pidstr[16];
    char *listpidstr;

    /* get creator pid and associated pids from fileName */

    if (fileName != NULL)
    {
        key_file = fopen(fileName, "r");
        if (key_file != NULL)
        {
            /* Parse keyFile until (but not including) the first attached pid or domain state. */
            if (os_win32_keyFileParser(key_file, &data) == os_resultSuccess) {
                /* Start list with the creator pid. */
                memcpy(&attachedPid, &(data.creator_pid), sizeof(os_win32_pid_info));
                do {
                    /* if creation times present in file then verify with os */
                    if (attachedPid.time.dwLowDateTime != 0 || attachedPid.time.dwHighDateTime != 0)
                    {
                        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, attachedPid.pid);
                        result = GetProcessTimes(hProcess, &creationTime, &exitTime, &kernelTime, &userTime);
                        if(result)
                        {
                            if(attachedPid.time.dwLowDateTime  == creationTime.dwLowDateTime &&
                               attachedPid.time.dwHighDateTime == creationTime.dwHighDateTime)
                            {
                                /* change pid to string to match iterator model */
                                sprintf(pidstr, "%d", attachedPid.pid);
                                listpidstr =  os_strdup(pidstr);
                                os_iterAppend(pidList, listpidstr);

                            }
                        }
                        CloseHandle(hProcess);
                    }
                /* Next attached pid. */
                } while (os_win32_keyfile_getPidInfo(key_file, &attachedPid) == os_resultSuccess);
            }
            fclose(key_file);
        }
        else
        {
            retVal = 1;    /* can't open file */
        }
    }
    else
    {
        retVal = 1;    /* no file */
    }

    return retVal;
}

/** \brief frees memory used by iterator created by prior call to
 *  \b listUserProcesses creating list in \b pidList
 */
os_int32
os_sharedMemoryListUserProcessesFree(
    os_iter pidList)
{
    char *pidstr;

    pidstr = (char *) os_iterTakeFirst(pidList);
    while (pidstr)
    {
        os_free(pidstr);
        pidstr = (char *) os_iterTakeFirst(pidList);
    }
    return 0;

}

/** \brief Return list in \b nameList of running opensplice domains defined
 * by presence of associated key files in relevant temporary directory
 *
 * \b returns 0 on success and 1 if key file not found or unreadable
 */
static os_result
os_sharedMemoryListDomainNamesAction(
    const char *key_file_name,
    os_win32_keyfile_data *data,
    void *action_arg)
{
    os_iter nameList = (os_iter)action_arg;
    OS_UNUSED_ARG(key_file_name);
    os_iterAppend(nameList, os_strdup(data->domain_name));
    return os_resultBusy; /* busy: keep searching */
}

os_int32
os_sharedMemoryListDomainNames(
    os_iter nameList)
{
    os_int32 retVal = 1;
    if (os_win32_loopKeyFiles(os_sharedMemoryListDomainNamesAction,
                              (void*)nameList) == os_resultBusy) {
        retVal = 0;
    }
    return retVal;
}


/** \brief frees memory used by iterator created by prior call to
 *  \b listDomainNames creating list in \b nameList
 */
void
os_sharedMemoryListDomainNamesFree(
    os_iter nameList)
{
    char *name;

    name = (char *) os_iterTakeFirst(nameList);
    while (name)
    {
        os_free(name);
        name = (char *) os_iterTakeFirst(nameList);
    }
}

/** \brief Return list in \b idList of running opensplice domains defined
 * by presence of associated key files in relevant temporary directory
 *
 * \b returns 0 on success and 1 if key file not found or unreadable
 */

typedef struct os_listDomainIdsActionArg {
    os_int32 **idList;
    os_int32  *listSize;
} os_listDomainIdsActionArg;

static os_result
os_listDomainIdsAction(
    const char *key_file_name,
    os_win32_keyfile_data *data,
    void *action_arg)
{
    os_listDomainIdsActionArg *arg = (os_listDomainIdsActionArg *)action_arg;
    os_result rv = os_resultFail;
    os_int32 size = *(arg->listSize);

    OS_UNUSED_ARG(key_file_name);

    *(arg->idList) = (os_int32*)os_realloc(*(arg->idList), (os_size_t)(sizeof(os_int32*) * (size + 1)));
    if (*(arg->idList) != NULL) {
        (*(arg->idList))[size] = data->domain_id;
        *(arg->listSize) = size + 1;
        rv = os_resultBusy; /* busy: keep searching */;
    }

    return rv;
}

os_int32
os_sharedMemoryListDomainIds(
    os_int32 **idList,
    os_int32  *listSize)
{
    os_listDomainIdsActionArg arg;
    os_int32 retVal = 1;

    *idList   = NULL;
    *listSize = 0;

    arg.idList = idList;
    arg.listSize = listSize;
    if (os_win32_loopKeyFiles(os_listDomainIdsAction,
                              (void*)&arg) == os_resultBusy) {
        retVal = 0;
    }

    if (retVal != 0) {
        if (*idList != NULL) {
            os_free(*idList);
        }
        *idList = NULL;
        *listSize = 0;
    }

    return retVal;
}


/** \brief Removes tmp and dbf files from asscociated domain defined by
 *  \b name
 */
os_int32
os_destroyKeyFile(
    const char *name)
{
    os_char dbf_file_name [MAX_PATH];
    os_char *ptr;
    HANDLE fileHandle;
    WIN32_FIND_DATA fileData;

    if (name ==  NULL)
    {
        return 1;
    }

    fileHandle = FindFirstFile(name, &fileData);

    if (fileHandle != INVALID_HANDLE_VALUE)
    {
        /* file exists so delete */
        CloseHandle(fileHandle);

        if (DeleteFile(name) == 0)
        {
            OS_REPORT(OS_ERROR, "os_destroyKeyFile", 0,
                      "Can not delete the key-file '%s': %s",
                      name, os_strError (os_getErrno()));
        }
    }

    os_strcpy(dbf_file_name, name);
    ptr = strchr(dbf_file_name, '.');
    if (ptr != NULL)
    {
        dbf_file_name[ptr - dbf_file_name + 1] = 'D';
        dbf_file_name[ptr - dbf_file_name + 2] = 'B';
        dbf_file_name[ptr - dbf_file_name + 3] = 'F';

        fileHandle = FindFirstFile(dbf_file_name, &fileData);

        if (fileHandle != INVALID_HANDLE_VALUE)
        {
            CloseHandle(fileHandle);
            if (DeleteFile(dbf_file_name) == 0)
            {
                OS_REPORT(OS_ERROR, "os_destroyKeyFile", 0,
                          "Can not delete the database file '%s': %s",
                          dbf_file_name, os_strError (os_getErrno()));
                return 1;
            }
        }
    }

    return 0;
}

int
os_sharedMemorySegmentFree(
    const char * name)
{
    return 0;
}

/** \brief Windows specific temporary tmp and dbf file clean up
 *  following any spliced termination other than through ospl tool
 */
static os_result
os_cleanSharedMemAndOrKeyFilesAction(
    const char *key_file_name,
    os_win32_keyfile_data *data,
    void *action_arg)
{
    HANDLE hProcess;
    BOOL doClean = FALSE;
    FILETIME creationTime;
    FILETIME exitTime;
    FILETIME kernelTime;
    FILETIME userTime;
    os_int32 procResult;
    BOOL result;
    os_iter pidList = NULL;
    char *pidstr = NULL;
    os_int32 userProc;
    os_char dbf_file_name [MAX_PATH];
    os_char *ptr;

    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, data->creator_pid.pid);
    if (hProcess == NULL)
    {
        doClean = TRUE;
    }
    else
    {
        /* Only perform clean up if we opened a process that matches the spliced process we  expected (i.e.
         * if the creation time also matches) AND THE SPLICED PROCESS IS NO LONGER RUNNING
         */
        if (os_procCheckStatus(os_handleToProcId(hProcess), &procResult) != os_resultBusy)
        {
            creationTime.dwLowDateTime = 0;
            creationTime.dwHighDateTime = 0;
            result = TRUE;
            if (data->creator_pid.time.dwLowDateTime != 0 || data->creator_pid.time.dwHighDateTime != 0)
            {
                result = GetProcessTimes(hProcess, &creationTime, &exitTime, &kernelTime, &userTime);
            }
            if(result)
            {
                if(data->creator_pid.time.dwLowDateTime  == creationTime.dwLowDateTime &&
                   data->creator_pid.time.dwHighDateTime == creationTime.dwHighDateTime)
                {
                    doClean = TRUE;
                }
            }
        }
        CloseHandle(hProcess);
    }
    if(doClean)
    {
        /* remove any associated processes */
        pidList = os_iterNew(NULL);
        if (os_sharedMemoryListUserProcesses(pidList, key_file_name) == 0)
        {
            while ((pidstr = (char *)os_iterTakeFirst(pidList)) != NULL)
            {
                userProc = atoi(pidstr);
                os_procServiceDestroy(userProc, FALSE, 100);
            }
            os_sharedMemoryListUserProcessesFree(pidList);
        }

        os_remove (key_file_name);

        os_strcpy(dbf_file_name, key_file_name);
        ptr = strchr(dbf_file_name, '.');
        if (ptr != NULL)
        {
            dbf_file_name[ptr - dbf_file_name + 1] = 'D';
            dbf_file_name[ptr - dbf_file_name + 2] = 'B';
            dbf_file_name[ptr - dbf_file_name + 3] = 'F';
            os_remove(dbf_file_name);
        }
    }

    return os_resultBusy; /* busy: keep searching */;
}

void
os_cleanSharedMemAndOrKeyFiles(
    void)
{
    HANDLE hFile;
    WIN32_FIND_DATA fileData;
    os_char key_file_name [MAX_PATH];
    os_char dbf_file_name [MAX_PATH];
    os_char *ptr;
    os_uint32 last = 0;
    FILE *key_file;

    /* Loop through keyfiles to find the proper one and clean it. */
    (void)os_win32_loopKeyFiles(os_cleanSharedMemAndOrKeyFilesAction, NULL);
    if (key_file_path == NULL) {
        return;
    }

    /* We now have to check for abandoned DBF files where there is no corresponding
    *  tmp file.If there is a corresponding tmp file then it will be a valid pair
    *  as we have already removed the invalid tmp files.
    */

    last = 0;
    os_strcpy(dbf_file_name, key_file_path);
    os_strcat(dbf_file_name, "\\");
    os_strcat(dbf_file_name, key_file_prefix);
    os_strcat(dbf_file_name, "*.DBF");

    hFile = FindFirstFile(dbf_file_name, &fileData);

    if (hFile != INVALID_HANDLE_VALUE)
    {
        os_strcpy(dbf_file_name, key_file_path);
        os_strcat(dbf_file_name, "\\");
        os_strcat(dbf_file_name, fileData.cFileName);

        while (!last)
        {
            /* check that it does not have a matching tmp file, any matching tmp file will be valid */

            os_strcpy(key_file_name, dbf_file_name);
            ptr = strchr(key_file_name, '.');  /* assumes keeping existing file format */
            if (ptr != NULL)
            {
                key_file_name[ptr - key_file_name + 1] = 't';
                key_file_name[ptr - key_file_name + 2] = 'm';
                key_file_name[ptr - key_file_name + 3] = 'p';
            }

            key_file = fopen(key_file_name, "r");
            if (key_file == NULL)
            {
                /* there is no matching tmp file, delete the orphan DBF file */

                os_remove(dbf_file_name);
            }
            else
            {
                fclose(key_file);
            }

            if (FindNextFile(hFile, &fileData) == 0)
            {
                last = 1;
            }
            else
            {
                os_strcpy(dbf_file_name, key_file_path);
                os_strcat(dbf_file_name, "\\");
                os_strcat(dbf_file_name, fileData.cFileName);
            }
        }
    }
    FindClose(hFile);
}


/** \brief Get the SHM map address from shm key file
 *
 * \b os_getShmMapAddress returns the map address of a domain identified by a name
 *
 * Equivalent of os_svr4_getMapAddress.
 * Find the key file related to the named shared memory area.
 * When found, read the map address from the 2nd line.
 */
static void *
os_getShmMapAddress(
    const char *name)
{
    os_win32_keyfile_dataArg arg;
    os_result rv;
    os_address map_address = 0;

    arg.id = 0;
    arg.name = name;

    rv = os_win32_loopKeyFiles(os_win32_getDataByNameAction, &arg);
    if (rv == os_resultSuccess) {
        map_address = arg.data.address;
    }

    return (void*)map_address;
}

/** \brief Returns the file-path of the SHM data file
 *
 * \b os_getShmFile tries to find the key file for a
 * Windows shared memory file by calling \b os_findKeyFile.
 *
 * If the key file is found, the datafile can be found with
 * help of the key filename and contents.
 *
 * If the key file could not be found, -1 is returned to the caller
 * if \b create = 0. If \b create is != 0 however, it creates a new
 * key file by calling \b GetTempFileName and \b CreateFile,
 * which creates and opens a new unique file based upon the provided path.
 * The \b domain name, \b map_address, \b size and PID are then
 * written in the key file after which it is closed.
 * The key filename is then translated to a shared data filename by changing
 * the file extension to DBF, and returned after the \b key_file_name is freed.
 */
static char *
os_getShmFile(
    os_sharedHandle sharedHandle,
    os_address size,
    int create)
{
    char *key_file_name;
    char *shm_file_name;
    UINT unique;
    HANDLE fileHandle;
    DWORD written;
    DWORD pid;
    char buf[512];
    size_t key_file_name_len;
    FILETIME creationTime;
    FILETIME exitTime;
    FILETIME kernelTime;
    FILETIME userTime;
    HANDLE hProcess;
    BOOL result;

    key_file_name = os_findKeyFile(sharedHandle->name);

    if (key_file_name == NULL) {
        if (create == 0) {
            return NULL;
        }
        key_file_name = (char*)os_malloc(MAX_PATH);
        snprintf(key_file_name, MAX_PATH, "%s%s", key_file_path, key_file_prefix);
        unique = GetTempFileName(key_file_path, key_file_prefix, 0, key_file_name);

        if (unique == 0) {
            OS_REPORT(OS_ERROR, "os_getShmFile", 0,
                      "GetTempFileName failed for (%s, %s, 0, %s): %s",
                      key_file_path, key_file_prefix, key_file_name, os_strError (os_getErrno()));
            return NULL;
        }
        fileHandle = CreateFile(key_file_name,
                                GENERIC_READ | GENERIC_WRITE,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL,
                                CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);

        if (fileHandle == INVALID_HANDLE_VALUE) {
            OS_REPORT(OS_ERROR, "os_getShmFile", 0,
                      "CreateFile failed for key-file '%s': %s",
                      key_file_name, os_strError (os_getErrno()));
            return NULL;
        }

        pid = GetCurrentProcessId();
        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
        if(hProcess != INVALID_HANDLE_VALUE)
        {
            result = GetProcessTimes(hProcess, &creationTime, &exitTime, &kernelTime, &userTime);
            if(!result)
            {
                creationTime.dwLowDateTime = 0;
                creationTime.dwHighDateTime = 0;
            }
            CloseHandle(hProcess);
        } else
        {
            creationTime.dwLowDateTime = 0;
            creationTime.dwHighDateTime = 0;
        }
        snprintf(buf, sizeof(buf), "%s\n"PA_ADDRFMT"\n"PA_ADDRFMT"\nWIN-SHM\n%d %d %d\n%d\n",
            sharedHandle->name, (PA_ADDRCAST)sharedHandle->attr.map_address, size, pid,
            creationTime.dwLowDateTime, creationTime.dwHighDateTime, sharedHandle->id);

        if (WriteFile((HANDLE)fileHandle, buf, (DWORD)strlen(buf), &written, NULL) == 0) {
            OS_REPORT(OS_ERROR, "os_getShmFile", 0,
                      "WriteFile failed for key-file '%s': %s",
                      key_file_name, os_strError(os_getErrno()));
            return NULL;
        }
        CloseHandle((HANDLE)fileHandle);

    }
    shm_file_name = (char*)os_malloc(MAX_PATH);
    os_strcpy(shm_file_name, key_file_name);
    // os_strcat(shm_file_name, "_DBF");
    key_file_name_len = strlen(shm_file_name);
    os_strcpy (&shm_file_name [key_file_name_len - 3], "DBF");
    os_free(key_file_name);
    return shm_file_name;
}

/** \brief Get a name for a file mappin object based on a domain
 *  name.
 *
 * \b Finds the key file for the given domain name with \b os_findKeyFile.
 *
 * If the key file is found, uses it's unique 'temp' file name
 * as the basis for the file mapping object name.
 *
 * If the key file could not be found, NULL is returned to the caller
 *
 * If the global_map arg is not zero then a name for a shared / global
 * object is returned. The name will be local otherwise.
 */
static char *
os_getMapName(
    const char *name,
    int global_map)
{
    char *key_file_name;
    char *map_name;
    char *key;

    key_file_name = os_findKeyFile(name);
    if (key_file_name == NULL) {
        return NULL;
    }
    key = key_file_name + strlen(key_file_name);
    while (*key != '\\' && *key != '/') {
        key--;
    }
    key++;
    map_name = (char*)os_malloc(MAX_PATH);
    if (global_map)
    {
        os_strcpy(map_name, "Global\\");
        os_strcat(map_name, key);
    }
    else
    {
        os_strcpy(map_name, key);
    }
    os_strcat(map_name, "_MAP");
    os_free(key_file_name);
    return map_name;
}

/** \brief Destroy the key related to the name
 *
 * The key file related to name is destroyed.
 * First \b os_destroyKey finds the path of the key
 * file by calling \b os_findKeyFile. If the key file
 * is not found, -1 is returned. If the key file is
 * found, the file is destroyed by calling \b unlink.
 * Depending on the result of \b unlink, 0 or -1
 * is returned after \b key_file_name is freed.
 */
static int
os_destroyKey(
    const char *name)
{
    char *key_file_name;

    key_file_name = os_findKeyFile(name);
    if (key_file_name ==  NULL) {
        return -1;
    }
    if (DeleteFile(key_file_name) == 0) {
        OS_REPORT(OS_ERROR, "OS Abstraction", 0,
                  "Can not delete the key-file '%s': %s",
                  key_file_name, os_strError(os_getErrno()));
        os_free(key_file_name);
        return -1;
    }
    os_free(key_file_name);
    return 0;
}

static os_result
os_sharedMemoryCreateFile(
    os_sharedHandle sharedHandle,
    os_address size)
{
    char *shm_file_name;
    char *map_object_name;
    char key_name[MAX_PATH];
    HANDLE dd;
    HANDLE md;
    SECURITY_ATTRIBUTES security_attributes;
    BOOL sec_descriptor_ok;

    shm_file_name = os_getShmFile(sharedHandle, size, 1);
    _splitpath(shm_file_name, NULL, NULL, key_name, NULL);
    if (strlen(key_name) == 0) {
        OS_REPORT(OS_ERROR, "OS Abstraction", 0,
                  "Failed to create name for database file '%s'", shm_file_name);
        os_free(shm_file_name);
        return os_resultFail;
    }

    dd = CreateFile(shm_file_name,
                    GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (dd == INVALID_HANDLE_VALUE) {
        OS_REPORT(OS_ERROR, "OS Abstraction", 0,
                  "Can not create the database-file '%s': %s",
                  shm_file_name, os_strError(os_getErrno()));
        os_free(shm_file_name);
        return os_resultFail;
    }
    os_free(shm_file_name);

    if (SetFilePointer(dd, (LONG)size, 0, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
        OS_REPORT(OS_ERROR, "OS Abstraction", 0,
                  "SetFilePointer to end of database failed: %s", os_strError(os_getErrno()));
        CloseHandle((HANDLE)dd);
        return os_resultFail;
    }
    if (SetEndOfFile(dd) == 0) {
        OS_REPORT(OS_ERROR, "OS Abstraction", 0,
                  "Not enought diskspace for database: %s", os_strError(os_getErrno()));
        CloseHandle((HANDLE)dd);
        return os_resultFail;
    }
    /* Try and create a global file mapping object first */
    map_object_name = os_getMapName(sharedHandle->name, 1);

    /* Vista and on have tightened security WRT shared memory
    we need to grant rights to interactive users et al via a discretionary
    access control list. NULL attrs does not allow users other than process
    starter to access */
    md = NULL;
    ZeroMemory(&security_attributes, sizeof(security_attributes));
    security_attributes.nLength = sizeof(security_attributes);
    sec_descriptor_ok = ConvertStringSecurityDescriptorToSecurityDescriptor
                            ("D:P(A;OICI;GA;;;WD)", /* grant all access to world */
                            SDDL_REVISION_1,
                            &security_attributes.lpSecurityDescriptor,
                            NULL);
    if (sec_descriptor_ok)
    {
        md = CreateFileMapping(dd, &security_attributes, PAGE_READWRITE, 0, (DWORD)size, map_object_name);
        LocalFree(security_attributes.lpSecurityDescriptor);
    }

    if (md == NULL)
    {
        /* Couldn't create a global mapping - we're probably not admin or local system */
        os_free(map_object_name);
        os_isSharedMemGlobal = 0;
        /* get a local file mapping name instead  and create that */
        map_object_name = os_getMapName(sharedHandle->name, 0);
        md = CreateFileMapping(dd, NULL, PAGE_READWRITE, 0, (DWORD)size, map_object_name);
        if (md == NULL) {
            OS_REPORT(OS_ERROR, "OS Abstraction", 0,
                      "Can not create mapping object '%s' for database: %s",
                      map_object_name, os_strError(os_getErrno()));
            os_free(map_object_name);
            CloseHandle((HANDLE)dd);
            return os_resultFail;
        }
    }
    os_free(map_object_name);
    sharedHandle->key = (char*)os_malloc((strlen(key_name) * sizeof(char)) + 1);
    os_strcpy(sharedHandle->key, key_name);
    sharedHandle->dataFile = dd;
    sharedHandle->mapObject = md;
    sharedHandle->size = size;
    sharedHandle->shm_created = 1;
    return os_resultSuccess;
}

static os_result
os_sharedMemoryAttachFile(
    os_sharedHandle sharedHandle)
{
    char *shm_file_name;
    char *map_object_name;
    char key_name[MAX_PATH];
    HANDLE dd;
    HANDLE md;
    void *address;
    void *request_address;
    DWORD size;
    if (!sharedHandle->shm_created) {
        shm_file_name = os_getShmFile(sharedHandle, 0, 0);
        if (shm_file_name == NULL) {
            return os_resultFail;
        }
        _splitpath(shm_file_name, NULL, NULL, key_name, NULL);
        if (strlen(key_name) == 0) {
            OS_REPORT(OS_ERROR, "OS Abstraction", 0,
                      "Failed to create key name for SHM file %s", shm_file_name);
            os_free(shm_file_name);
            return os_resultFail;
        }

        dd = CreateFile(shm_file_name,
                        GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (dd == INVALID_HANDLE_VALUE) {
            OS_REPORT_WID(OS_ERROR, "OS Abstraction", 0, sharedHandle->id,
                      "Can not open the database file '%s': %s",
                      shm_file_name, os_strError(os_getErrno()));
            os_free(shm_file_name);
            return os_resultFail;
        }
        os_free(shm_file_name);
        sharedHandle->dataFile = dd;
        size = SetFilePointer(dd, 0, 0, FILE_END);
        if (size == INVALID_SET_FILE_POINTER) {
            OS_REPORT_WID(OS_ERROR, "OS Abstraction", 0, sharedHandle->id,
                      "Can not determine database size: %s",
                      os_strError(os_getErrno()));
            return os_resultFail;
        }
        sharedHandle->size = size;
        /* Try first to open a global mapping. If that fails just open a local mapping
        Doesn't matter which order we do this - the temp file naming means the handle name'll
        have been unique */
        map_object_name = os_getMapName(sharedHandle->name, 1);

        if (map_object_name == NULL) {
            return os_resultFail;
        }
        md = OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, map_object_name);

        if (md == NULL) {
            /* Couldn't open a global mapping - must be local. Just free the name
            get a local one instead and try again */
            os_free(map_object_name);
            os_isSharedMemGlobal = 0;
            map_object_name = os_getMapName(sharedHandle->name, 0);
            md = OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, map_object_name);
            if (md == NULL) {
                OS_REPORT_WID(OS_ERROR, "OS Abstraction", 0, sharedHandle->id,
                          "Can not open mapping object '%s': %s",
                          map_object_name, os_strError(os_getErrno()));
                os_free(map_object_name);
                return os_resultFail;
            }
        }
        sharedHandle->key = (char*)os_malloc((strlen(key_name) * sizeof(char)) + 1);
        os_strcpy(sharedHandle->key, key_name);
        os_free(map_object_name);

        sharedHandle->mapObject = md;
    }

    request_address = os_getShmMapAddress(sharedHandle->name);
    if (request_address == NULL) {
        OS_REPORT_WID(OS_ERROR, "OS Abstraction", 0, sharedHandle->id,
                  "Failed to get map_address from key file");
        return os_resultFail;
    }
    address = MapViewOfFileEx(sharedHandle->mapObject,
                              FILE_MAP_ALL_ACCESS, 0, 0,
                              sharedHandle->size, request_address);

    if (address == NULL) {
        OS_REPORT_WID(OS_ERROR, "OS Abstraction", 0, sharedHandle->id,
                  "Can not Map View Of file: %s",
                  os_strError(os_getErrno()));
        return os_resultFail;
    }

    sharedHandle->mapped_address = address;
    return os_resultSuccess;
}

static os_result
os_sharedMemoryDetachFile(
    os_sharedHandle sharedHandle)
{
    if (UnmapViewOfFile(sharedHandle->mapped_address) == 0) {
        OS_REPORT(OS_ERROR, "OS Abstraction", 0,
                  "Can not unmap view of file: %s", os_strError(os_getErrno()));
        return os_resultFail;
    }
    sharedHandle->mapped_address = NULL;

    if (!sharedHandle->shm_created) {
        if (CloseHandle(sharedHandle->mapObject) == 0) {
            OS_REPORT(OS_ERROR, "OS Abstraction", 0,
                      "Can not close handle of map: %s", os_strError(os_getErrno()));
            return os_resultFail;
        }
        sharedHandle->mapObject = 0;

        if (CloseHandle(sharedHandle->dataFile) == 0) {
            OS_REPORT(OS_ERROR, "OS Abstraction", 0,
                      "Can not close database file: %s", os_strError(os_getErrno()));
            return os_resultFail;
        }
        sharedHandle->dataFile = 0;
    }
    return os_resultSuccess;
}

static os_result
os_sharedMemoryDestroyFile(
    os_sharedHandle sharedHandle)
{
    char *shm_file_name;

    if (CloseHandle(sharedHandle->mapObject) == 0) {
        OS_REPORT(OS_ERROR, "OS Abstraction", 0,
                  "Can not close handle of map: %s", os_strError(os_getErrno()));
        return os_resultFail;
    }
    sharedHandle->mapObject = 0;
    if (CloseHandle(sharedHandle->dataFile) == 0) {
        OS_REPORT(OS_ERROR, "OS Abstraction", 0,
                  "Can not close database-file: %s", os_strError(os_getErrno()));
        return os_resultFail;
    }
    sharedHandle->dataFile = 0;
    shm_file_name = os_getShmFile(sharedHandle, 0, 0);
    if (DeleteFile(shm_file_name) == 0) {
        OS_REPORT(OS_ERROR, "OS Abstraction", 0,
                  "Can not delete database-file '%s' for domain '%s': %s",
                  shm_file_name, sharedHandle->name, os_strError(os_getErrno()));
        os_free(shm_file_name);
        return os_resultFail;
    }
    os_destroyKey(sharedHandle->name);
    return os_resultSuccess;
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
        result = os_sharedMemoryCreateFile (sharedHandle, size);
#ifdef OSPL_SHM_PROCMON
        if (result == os_resultSuccess)
        {
            result = os_sharedMemoryProcessMonitorInit(sharedHandle);
        }
#endif
    break;
    case OS_MAP_ON_SEG:
        result = os_resultUnavailable;
    break;
    case OS_MAP_ON_HEAP:
        result = os_resultUnavailable;
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
        result = os_sharedMemoryDestroyFile (sharedHandle);
#ifdef OSPL_SHM_PROCMON
        if(result == os_resultSuccess){
            os_sharedMemoryProcessMonitorDeinit(sharedHandle);
        }
#endif
    break;
    case OS_MAP_ON_SEG:
        result = os_resultUnavailable;
    break;
    case OS_MAP_ON_HEAP:
        result = os_resultUnavailable;
    break;
    }
    return result;
}


os_result
os_sharedMemoryAttach(
    os_sharedHandle sharedHandle)
{
    os_result result = os_resultFail;
    struct os_shmInfo *shmInf;

    assert(sharedHandle != NULL);
    assert(sharedHandle->name != NULL);
    assert(sharedHandle->mapped_address == NULL);
    switch (sharedHandle->attr.sharedImpl) {
    case OS_MAP_ON_FILE:
        result = os_sharedMemoryAttachFile (sharedHandle);
        if (result == os_resultSuccess) {
            /* add shm to shmInfo object */
            shmInf = os_malloc(sizeof(struct os_shmInfo));
            shmInf->sharedHandle = sharedHandle;
            shmInf->next = shmInfo;
            shmInfo = shmInf;
#ifdef OSPL_SHM_PROCMON
            os_sharedMemoryRegisterProcess(sharedHandle);
#endif
        }
    break;
    case OS_MAP_ON_SEG:
        result = os_resultUnavailable;
    break;
    case OS_MAP_ON_HEAP:
        result = os_resultUnavailable;
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
    os_boolean shmInfRemoved = OS_FALSE;
    struct os_shmInfo *shmInfoPrev;
    struct os_shmInfo *shmInfoCur;

    assert(sharedHandle != NULL);
    assert(sharedHandle->name != NULL);
    switch (sharedHandle->attr.sharedImpl) {
    case OS_MAP_ON_FILE:
        /* Update the shmInfo list to indicate that the shared memory segment
         * identified by the sharedHandle is no longer valid. We will thus remove
         * a shmInfo object from the list
         */
        shmInfoPrev = NULL;
        shmInfoCur = shmInfo;
        while(shmInfRemoved == OS_FALSE && shmInfoCur != NULL)
        {
            /* If the shared handle provided for this detach call is the same
             * as the sharedHandle of the shmInfoCur object then we have found
             * the shmInfo object describing this particular memory segment.
             * So we need to proceed to remove it from the list as it will no
             * longer be valid.
             */
            if(shmInfoCur->sharedHandle == sharedHandle)
            {
                /* If the shmInfoCur was not the first shmInfo object we
                 * encountered, then we need to remove a shmInfo object from
                 * the middle or tail of the list. So we accomplish this by
                 * updating the 'next' pointer of the shmInfoPrev object (if any)
                 * to the 'next' pointer of the shmInfoCur object.
                 * If the shmInfoPrev is still 'NULL' then we are removing the
                 * shmInfo object at the head of the list, so update the head
                 * pointer 'shmInfo' accordingly.
                 */
                if(shmInfoPrev)
                {
                    shmInfoPrev->next = shmInfoCur->next;
                } else
                {
                    shmInfo = shmInfoCur->next;
                }
                /* Now free the memory and indicated that we removed a shmInfo
                 * object from the list
                 */
                os_free(shmInfoCur);
                shmInfoCur = NULL;
                shmInfRemoved = OS_TRUE;
            } else
            {
                shmInfoPrev = shmInfoCur;
                shmInfoCur = shmInfoCur->next;
            }
        }
        result = os_sharedMemoryDetachFile (sharedHandle);
#ifdef OSPL_SHM_PROCMON
        if((result == os_resultSuccess) && clean){
            os_sharedMemoryDeregisterProcess(sharedHandle);
        }
#endif
    break;
    case OS_MAP_ON_SEG:
        result = os_resultUnavailable;
    break;
    case OS_MAP_ON_HEAP:
        result = os_resultUnavailable;
    break;
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

void
os_sharedAttrInit(
    os_sharedAttr *sharedAttr)
{
    assert(sharedAttr != NULL);
    sharedAttr->lockPolicy = OS_LOCK_DEFAULT;
    sharedAttr->sharedImpl = OS_MAP_ON_FILE;
    sharedAttr->userCred.uid = 0;
    sharedAttr->userCred.gid = 0;
#if _WIN64
    sharedAttr->map_address = (void *)0x100000000;
#else
    sharedAttr->map_address = (void *)0x40000000;
#endif
}


/** \brief Check if the contents of the identified key file
 *         matches the identified id and and set the name
 *
 * \b sharedMemoryGetNameFromId tries to compare the contents of the
 * identified key file in \b key_file_name with the identified \b id.
 * On a match, os_resultSuccess will be returned and the domain name
 * will be set.
 * On a mismatch, os_resultFail will be returned and name will be NULL.
 */
static os_result
sharedMemoryGetNameFromId(
    int id,
    char **name)
{
    os_win32_keyfile_dataArg arg;
    os_result rv;

    *name = NULL;
    arg.id = id;
    arg.name = NULL;

    rv = os_win32_loopKeyFiles(os_win32_getDataByIdAction, &arg);
    if (rv == os_resultSuccess) {
        *name = os_strdup(arg.data.domain_name);
        if (*name == NULL) {
            rv = os_resultFail;
        }
    }

    return rv;
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
        if (sharedHandle->size == 0) {
            result = os_resultFail;
        } else {
            *size = sharedHandle->size;
            result = os_resultSuccess;
        }
    break;
    case OS_MAP_ON_SEG:
        result = os_resultUnavailable;
    break;
    case OS_MAP_ON_HEAP:
        *size = 0xFFFFFFFF; /* maximal address on 32bit systems */
        result = os_resultSuccess;
    break;
    default:
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
        result = sharedMemoryGetNameFromId(sharedHandle->id, name);
    break;
    case OS_MAP_ON_SEG:
        result = os_resultUnavailable;
    break;
    case OS_MAP_ON_HEAP:
        result = os_resultUnavailable;
    break;
    }
    return result;
}

void
os_sharedMemoryRegisterUserProcess(
    const os_char* domainName,
    os_procId pid)
{
    char* keyFileName;
    HANDLE fileHandle;
    char buffer[512];
    DWORD written;
    BOOL writeResult;
    FILETIME creationTime;
    FILETIME exitTime;
    FILETIME kernelTime;
    FILETIME userTime;
    HANDLE hProcess;
    BOOL result;

    keyFileName = os_findKeyFile(domainName);
    if (keyFileName == NULL)
    {
        OS_REPORT(OS_ERROR, "os_sharedMemoryRegisterUserProcess", 0, "Unable to register process with PID '%d' as a user of shared "
                  "memory. This only affects clean up procedures in case of later failure.", pid);
    } else
    {
        /* Open the file for an atomic append */
        fileHandle = CreateFile(keyFileName,
                                FILE_APPEND_DATA,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);
        if(fileHandle == INVALID_HANDLE_VALUE)
        {
            OS_REPORT(OS_ERROR, "os_sharedMemoryRegisterUserProcess", 0,
                      "CreateFile failed while trying to append to key-file '%s': %s",
                      keyFileName, os_strError (os_getErrno ()));
        } else
        {
            /* Fill the buffer with the pid info to be written */
            hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
            if(hProcess != INVALID_HANDLE_VALUE)
            {
                result = GetProcessTimes(hProcess, &creationTime, &exitTime, &kernelTime, &userTime);
                if(!result)
                {
                    creationTime.dwLowDateTime = 0;
                    creationTime.dwHighDateTime = 0;
                }
                CloseHandle(hProcess);
            } else
            {
                creationTime.dwLowDateTime = 0;
                creationTime.dwHighDateTime = 0;
            }
            snprintf(buffer, sizeof(buffer), "%d %d %d\n", pid, creationTime.dwLowDateTime, creationTime.dwHighDateTime);
            /* Now write the buffer to the key file, appending it at the end
             * as we opened the file with the FILE_APPEND_DATA flag
             */
            writeResult = WriteFile(fileHandle, buffer, (DWORD)strlen(buffer), &written, NULL);
            if(writeResult == 0)
            {
                OS_REPORT(OS_ERROR, "os_sharedMemoryRegisterUserProcess", 0,
                          "WriteFile failed while trying to append to key-file '%s': %s",
                          keyFileName, os_strError (os_getErrno ()));
            }
            CloseHandle(fileHandle);
        }
    }
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

static os_int32
disconnectClient (
    void *obj,
    os_iterActionArg arg)
{
    os__shmClient client = (os__shmClient)obj;
    os__shmClient removedClient = (os__shmClient)arg;

    if(client->procId == removedClient->procId){
        return 1;
    }
    return 0;
}


VOID CALLBACK pipeHandler(
  PVOID lpParameter,
  BOOLEAN TimerOrWaitFired
)
{
    /* A process has left the system */
    os__shmClient removedClient;
    os__shmClient foundClient;
    os__threadInfo *threadInfo = (os__threadInfo*)lpParameter;

    removedClient = (os__shmClient)(os_malloc(OS_SIZEOF(os__shmClient)));
    removedClient->procId = os_handleToProcId(threadInfo->procHandle);

    /* Search running process list if the terminated process should still be running */
    foundClient = os_iterTakeAction(threadInfo->shmDomain->clnts->runningClients, disconnectClient, removedClient);
    if ( foundClient != NULL)
    {   /* It is still in the suppose-to-be-running list (crashed/killed), so possible unsafe detach*/
        os_free(foundClient);
        removedClient->state = OS_SHM_PROC_TERMINATED;
    }
    else
    {   /* It is not in the running, so the process detached itself from shared-memory (normal shutdown) */
        removedClient->state = OS_SHM_PROC_DETACHED;
    }

    os_mutexLock(&os__shmAttachedLock);
    /* Add to changed list */
    os_iterAppend(threadInfo->shmDomain->clnts->diff, removedClient);

    /* Signal WaitForClientChanges something has changed */
    os_condSignal(&os__shmAttachCond);
    os_mutexUnlock(&os__shmAttachedLock);
    os_free(threadInfo);
}
char* getShmPipeName(os_sharedHandle shmHandle)
{
    char pipeName[] = "\\\\.\\pipe\\shmMonitor_";
    size_t shmPineNameSize = sizeof(pipeName) + strlen(shmHandle->key) + 1;
    char * shmPipeName = os_malloc(shmPineNameSize);
    snprintf(shmPipeName, shmPineNameSize, "%s%s", pipeName, shmHandle->key);
    return (shmPipeName);
}


// ConnectToNewClient(HANDLE, LPOVERLAPPED)
// This function is called to start an overlapped connect operation.
// It returns TRUE if an operation is pending or FALSE if the
// connection has been completed.

static BOOL
clientConnectionConnectToNewClient(
    HANDLE hPipe,
    LPOVERLAPPED lpo)
{
    BOOL fConnected, fPendingIO = FALSE;

    // Start an overlapped connection for this pipe instance.
    fConnected = ConnectNamedPipe(hPipe, lpo);

    // Overlapped ConnectNamedPipe should return zero.
    if (!fConnected) {
        switch (os_getErrno ()) {
        // The overlapped connection in progress.
        case ERROR_IO_PENDING:
            fPendingIO = TRUE;
            break;
        // Client is already connected, so signal an event.
        case ERROR_PIPE_CONNECTED:
            if (!SetEvent(lpo->hEvent)) {
                OS_REPORT(OS_ERROR, "clientConnectionConnectToNewClient", 0,
                        "Failed to connect to named pipe, System Error Code: %d",
                        (int)os_getErrno ());
            }
            break;
        // If an error occurs during the connect operation...
        default:
            OS_REPORT(OS_ERROR, "clientConnectionConnectToNewClient", 0,
                    "Failed to connect to named pipe, System Error Code: %d",
                    (int)os_getErrno ());
            break;
        }
    } else {
        OS_REPORT(OS_ERROR, "clientConnectionConnectToNewClient", 0,
                "Failed to connect to named pipe, System Error Code: %d",
                (int)os_getErrno ());
    }

    return fPendingIO;
}

static void
clientConnectionDisconnectAndReconnect(
    struct os_sharedMemoryClientConnection *connection)
{
    // Disconnect the pipe instance.
    if (!DisconnectNamedPipe(connection->pipeInstance)) {
        OS_REPORT(OS_ERROR, "clientConnectionDisconnectAndReconnect", 0,
                  "Failed to disconnect named pipe, System Error Code:  %d",
                  (int)os_getErrno ());
    }

    // Call a subroutine to connect to the new client.
    connection->ioPending = clientConnectionConnectToNewClient(
            connection->pipeInstance,
            &connection->overlap);

    connection->state = connection->ioPending ?
            CLIENT_PIPE_STATE_CONNECTING : // still connecting
            CLIENT_PIPE_STATE_READING;     // ready to read
}

static void
clientConnectionStartWrite(
    struct os_sharedMemoryClientConnection *connection,
    os__shmDomain shmDomain);


static void
clientConnectionFinishRead(
    struct os_sharedMemoryClientConnection *connection,
    os__shmDomain shmDomain)
{
    HANDLE procHandle;
    HANDLE hWaitHandle;
    os__shmClient client, newClient;
    os_procId procId;

    switch (connection->request.kind) {
        case OS_SHM_MESSAGE_CLIENT_CONNECT_IND:
        {
            /* New process connected */
            os__threadInfo *threadInfo = (os__threadInfo*)malloc(sizeof(os__threadInfo));

            /* Process ID is in the second part of the message, after the command character */
            procId = connection->request._u.procId;
            procHandle = os_procIdToHandle(procId); /* We need a handle to this process to wait for it */
            client = (os__shmClient)(os_malloc(OS_SIZEOF(os__shmClient)));
            newClient = (os__shmClient)(os_malloc(OS_SIZEOF(os__shmClient)));

            client->procId = procId;
            client->state = OS_SHM_PROC_ATTACHED;

            newClient->procId = client->procId;
            newClient->state = client->state;

            os_mutexLock(&os__shmAttachedLock);
            /* Insert newly reported process to the list of running clients */
            os_iterInsert(shmDomain->clnts->runningClients, client);
            /* And add it to the changed list */
            os_iterAppend(shmDomain->clnts->diff, newClient);
            /* Signal WaitForClientChanges something has changed */
            os_condSignal(&os__shmAttachCond);
            os_mutexUnlock(&os__shmAttachedLock);

            threadInfo->procHandle = procHandle;
            threadInfo->shmDomain = shmDomain;

            /* Kindly ask Windows to call pipeHandle in case the process with procHandle terminates, and do this only once */
            if (!RegisterWaitForSingleObject(&hWaitHandle, procHandle, pipeHandler, threadInfo, INFINITE, WT_EXECUTEONLYONCE)) {
                OS_REPORT(OS_ERROR, "sharedMemoryProcessMonitor", 0,
                          "Failed to allocate a wait handler on process handle, "
                          "System Error Code:  %d", (int)os_getErrno ());
            }

            connection->reply.kind = OS_SHM_MESSAGE_CLIENT_CONNECT_RSP;
            connection->reply._u.procId = os_procIdSelf();
            connection->ioPending = FALSE;
            connection->state = CLIENT_PIPE_STATE_WRITING;
            clientConnectionStartWrite(connection, shmDomain);
            break;
        }
        case OS_SHM_MESSAGE_CLIENT_DISCONNECT:
        {
            /* Process reported disconnection from shared-memory, normal shutdown */
            /* Process ID is in the second part of the message, after the command character */
            procId = connection->request._u.procId;
            procHandle = os_procIdToHandle(procId);
            newClient = (os__shmClient)(os_malloc(OS_SIZEOF(os__shmClient)));

            newClient->procId = procId;
            newClient->state = OS_SHM_PROC_DETACHED;

            os_mutexLock(&os__shmAttachedLock);
            /* Removed process that indicated termination from running list */
            os_iterTakeAction(shmDomain->clnts->runningClients, disconnectClient, newClient);
            /* And add to list of changed processes */
            os_iterAppend(shmDomain->clnts->diff, newClient);

            os_mutexUnlock(&os__shmAttachedLock);

            clientConnectionDisconnectAndReconnect(connection);
#if 0
            connection->reply.kind = OS_SHM_MESSAGE_OK;
            connection->ioPending = FALSE;
            connection->state = CLIENT_PIPE_STATE_WRITING;
            clientConnectionStartWrite(connection, shmDomain);
#endif
            break;
        }
        case OS_SHM_MESSAGE_MONITOR_EXIT:
        {
            /* System is shutting down, shmDomain->terminate is set to TRUE, but I still need to be wakened */
            clientConnectionDisconnectAndReconnect(connection);
#if 0
            connection->reply.kind = OS_SHM_MESSAGE_OK;
            connection->ioPending = FALSE;
            connection->state = CLIENT_PIPE_STATE_WRITING;
            clientConnectionStartWrite(connection, shmDomain);
#endif
            break;
        }
        default:
        {
            OS_REPORT(OS_WARNING, "sharedMemoryProcessMonitor", 0,
                      "Unexpected response received from pipe");
            break;
        }
    }
}

static void
clientConnectionStartRead(
    struct os_sharedMemoryClientConnection *connection,
    os__shmDomain shmDomain)
{
    BOOL fSuccess;
    DWORD dwErr;

    fSuccess = ReadFile(
            connection->pipeInstance,
            &connection->request,
            sizeof(struct os_shmMessage),
            &connection->cbRead,
            &connection->overlap);

    // The read operation completed successfully.
    if (fSuccess && connection->cbRead != 0) {
        clientConnectionFinishRead(connection, shmDomain);
    } else {
        // The read operation is still pending.
        dwErr = os_getErrno ();
        if (!fSuccess && (dwErr == ERROR_IO_PENDING)) {
            connection->ioPending = TRUE;
        } else {
            // An error occurred; disconnect from the client.
            clientConnectionDisconnectAndReconnect(connection);
        }
    }
}

static void
clientConnectionFinishWrite(
    struct os_sharedMemoryClientConnection *connection,
    os__shmDomain shmDomain)
{
    connection->ioPending = FALSE;
    connection->state = CLIENT_PIPE_STATE_CONNECTING;
    clientConnectionStartRead(connection, shmDomain);
}

static void
clientConnectionStartWrite(
    struct os_sharedMemoryClientConnection *connection,
    os__shmDomain shmDomain)
{
    BOOL fSuccess;
    DWORD cbRet, dwErr;

    fSuccess = WriteFile(
            connection->pipeInstance,
            &connection->reply,
            sizeof(struct os_shmMessage),
            &cbRet,
            &connection->overlap);

    // The write operation completed successfully.
    if (fSuccess && (cbRet == sizeof(struct os_shmMessage))) {
        clientConnectionFinishWrite(connection, shmDomain);
    } else {
        // The write operation is still pending.
        dwErr = os_getErrno();
        if (!fSuccess && (dwErr == ERROR_IO_PENDING)) {
            connection->ioPending = TRUE;
        } else {
            // An error occurred; disconnect from the client.
            clientConnectionDisconnectAndReconnect(connection);
        }
    }
}

static void *
sharedMemoryProcessMonitorNew(
    void* data)
{
    os__shmDomain shmDomain;
    BOOL fSuccess = TRUE;
    char *shmPipeName;
    DWORD dwWait, cbRet, i;
    SECURITY_ATTRIBUTES securityAttributes;
    BOOL secDescriptorOk;

    struct os_sharedMemoryClientConnection connections[MAX_NUM_CLIENT_CONNECTIONS];
    HANDLE hEvents[MAX_NUM_CLIENT_CONNECTIONS];

    memset(hEvents, 0, sizeof(HANDLE) * MAX_NUM_CLIENT_CONNECTIONS);
    memset(connections, 0, sizeof(struct os_sharedMemoryClientConnection) * MAX_NUM_CLIENT_CONNECTIONS);

    shmDomain = (os__shmDomain)data;
    shmPipeName = getShmPipeName(shmDomain->handle);

    ZeroMemory(&securityAttributes, sizeof(securityAttributes));
    securityAttributes.nLength = sizeof(securityAttributes);
    secDescriptorOk = ConvertStringSecurityDescriptorToSecurityDescriptor
            ("D:P(A;OICI;GA;;;WD)", /* grant all acess to 'world' (everyone) */
                    SDDL_REVISION_1,
                    &securityAttributes.lpSecurityDescriptor,
                    NULL);

    for (i = 0; i < MAX_NUM_CLIENT_CONNECTIONS; i++) {
        connections[i].pipeInstance = INVALID_HANDLE_VALUE;
    }

    for (i = 0; fSuccess && (i < MAX_NUM_CLIENT_CONNECTIONS); i++) {

        // Create an event object for this instance.

        hEvents[i] = CreateEvent(
            NULL,    // default security attribute
            TRUE,    // manual reset event
            TRUE,    // initial state = signaled
            NULL);   // unnamed event object

        if (hEvents[i] != NULL) {
            connections[i].overlap.hEvent = hEvents[i];

            connections[i].pipeInstance = CreateNamedPipe(
                    shmPipeName,                   // pipe name
                    PIPE_ACCESS_DUPLEX |           // read/write access
                    FILE_FLAG_OVERLAPPED,          // overlapped mode
                    PIPE_TYPE_MESSAGE |            // message-type pipe
                    PIPE_READMODE_MESSAGE |        // message-read mode
                    PIPE_WAIT,                     // blocking mode
                    MAX_NUM_CLIENT_CONNECTIONS,    // number of instances
                    sizeof(struct os_shmMessage),  // output buffer size
                    sizeof(struct os_shmMessage),  // input buffer size
                    _PIPE_DEFAULT_TIMEOUT,         // client time-out
                    (os_sharedMemIsGlobal() && secDescriptorOk ? &securityAttributes : NULL)); // Set security attributes

            if (connections[i].pipeInstance == INVALID_HANDLE_VALUE) {
                OS_REPORT(OS_ERROR, "sharedMemoryProcessMonitor", 0,
                          "Failed to create named pipe name %s, System Error Code:  %d",
                          shmPipeName, (int)os_getErrno());
                fSuccess = FALSE;
            }

            // Call the subroutine to connect to the new client

            connections[i].ioPending = clientConnectionConnectToNewClient(
                    connections[i].pipeInstance,
                    &connections[i].overlap);

            connections[i].state = connections[i].ioPending ?
                    CLIENT_PIPE_STATE_CONNECTING : // still connecting
                    CLIENT_PIPE_STATE_READING;     // ready to read

        } else {
            OS_REPORT(OS_ERROR, "sharedMemoryProcessMonitor", 0,
                      "Failed to create event, System Error Code:  %d",
                      (int)os_getErrno());
            fSuccess = FALSE;
        }
    }

    if (fSuccess) {

        /* Initialization success so signal the os_sharedMemoryProcessMonitorIniti
         * operation that this thread has started.
         */
        os_mutexLock(&os__shmAttachedLock);
        os_condSignal(&os__shmAttachCond);
        os_mutexUnlock(&os__shmAttachedLock);

        while (fSuccess && !shmDomain->terminate) {

            // Wait for the event object to be signaled, indicating
            // completion of an overlapped read, write, or
            // connect operation.

            dwWait = WaitForMultipleObjects(
                    MAX_NUM_CLIENT_CONNECTIONS,    // number of event objects
                    hEvents,                       // array of event objects
                    FALSE,                         // does not wait for all
                    INFINITE);                     // waits indefinitely

            // dwWait shows which pipe completed the operation.

            i = dwWait - WAIT_OBJECT_0;  // determines which pipe
            if (i < 0 || i > (MAX_NUM_CLIENT_CONNECTIONS - 1)) {
                OS_REPORT(OS_WARNING, "sharedMemoryProcessMonitor", 0,
                          "WaitForMultipleObjects returned wrong index"
                          "System Error Code:  %d", (int)os_getErrno());
                fSuccess = FALSE;
            }

            // Get the result if the operation was pending.

            if (fSuccess && connections[i].ioPending) {
                BOOL ioPending;

                ioPending = GetOverlappedResult(
                        connections[i].pipeInstance, // handle to pipe
                        &connections[i].overlap,     // OVERLAPPED structure
                        &cbRet,                      // bytes transferred
                        FALSE);                      // do not wait

                switch (connections[i].state) {
                // Pending connect operation
                case CLIENT_PIPE_STATE_CONNECTING:
                    if (ioPending) {
                        connections[i].state = CLIENT_PIPE_STATE_READING;
                        clientConnectionStartRead(&connections[i], shmDomain);
                    } else {
                        clientConnectionDisconnectAndReconnect(&connections[i]);
                    }
                    break;

                // Pending read operation
                case CLIENT_PIPE_STATE_READING:
                    if (!ioPending || cbRet == 0) {
                        clientConnectionDisconnectAndReconnect(&connections[i]);
                        continue;
                    }
                    connections[i].cbRead = cbRet;
                    clientConnectionFinishRead(&connections[i], shmDomain);
                    break;

                // Pending write operation
                case CLIENT_PIPE_STATE_WRITING:
                    if (!ioPending || cbRet != sizeof(struct os_shmMessage)) {
                        clientConnectionDisconnectAndReconnect(&connections[i]);
                        continue;
                    }
                    clientConnectionFinishWrite(&connections[i], shmDomain);
                    break;
                }
            }
        }

    } else {
        /* Initialization failed so signal the os_sharedMemoryProcessMonitorIniti
          * operation that this thread has failed to start.
          */
         OS_REPORT(OS_WARNING, "sharedMemoryProcessMonitor", 0,
                   "Could not create pipe, monitor already running?"
                   "System Error Code:  %d", (int)os_getErrno());
         os_mutexLock(&os__shmAttachedLock);
         (void)os_condSignal(&os__shmAttachCond);
         os_mutexUnlock(&os__shmAttachedLock);
    }

    for (i = 0; i < MAX_NUM_CLIENT_CONNECTIONS; i++) {
        if (connections[i].pipeInstance != INVALID_HANDLE_VALUE) {
            CloseHandle(connections[i].pipeInstance);
        }
        if (hEvents[i] != NULL) {
            CloseHandle(hEvents[i]);
        }
    }

    if (secDescriptorOk) {
        /* Free the heap allocated descriptor */
        LocalFree(securityAttributes.lpSecurityDescriptor);
    }

    return NULL;
}

/* Signal the os_sharedMemoryWaitForClientChanges operation about
 * the detected clients side change.
 */

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
                            sharedMemoryProcessMonitorNew, shmDomain);
                    if(result == os_resultSuccess){
                        os_condWait(&os__shmAttachCond, &os__shmAttachedLock);
                    }
                    os_mutexUnlock(&os__shmAttachedLock);
                }
            }
        }

        if(result == os_resultFail){
            /* Stop sharedMemoryProcessMonitor */
            shmDomain->terminate = 1;
            os_sharedMemoryTerminateMonitor(shmDomain->handle);
            TRACE_PKILL("Waiting for thread to exit...\n");
            os_threadWaitExit(shmDomain->procMonTid, NULL);
            TRACE_PKILL("Thread exited...\n");

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

    if(shmDomain){
        shmDomain->terminate = 1;
        /* Send signal to sharedMemoryProcessMonitor to terminate */
        os_sharedMemoryTerminateMonitor(shmDomain->handle);
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

static os_result
writeMessageToPipe(
    struct os_shmMessage *message,
    os_sharedHandle shmHandle)
{
    os_result result = os_resultSuccess;
    HANDLE hPipe;
    BOOL   fSuccess = FALSE;
    char   cMessage[BUFSIZE];
    long   lWritten, lToWrite = BUFSIZE;
    // Try to open a named pipe; wait for it, if necessary.
    char *shmPipeName = getShmPipeName(shmHandle);
    memset(cMessage, 0, BUFSIZE);

    while (1) {
        /* Open pipe */
        hPipe = CreateFile(
                shmPipeName,   // pipe name
                GENERIC_READ |  // read and write access
                GENERIC_WRITE,
                0,              // no sharing
                NULL,           // default security attributes
                OPEN_EXISTING,  // opens existing pipe
                0,              // default attributes
                NULL);          // no template file
        /* Break if the pipe handle is valid. */
        if (hPipe != INVALID_HANDLE_VALUE)
            break;

        /* Exit if an error other than ERROR_PIPE_BUSY occurs.*/
        if (os_getErrno() != ERROR_PIPE_BUSY) {
            OS_REPORT(OS_INFO, "sharedMemoryProcessMonitor", 0,
                      "Failed to open shared memory message pipe"
                      "System Error Code:  %d", (int)os_getErrno());
            return os_resultFail;
        }

        /* All pipe instances are busy, so wait for 20 seconds.*/
        if ( ! WaitNamedPipe(shmPipeName, 20000)) {
            OS_REPORT(OS_INFO, "sharedMemoryProcessMonitor", 0,
                      "Failed to open shared memory message pipe after 20 seconds"
                      "System Error Code:  %d (%s)", (int)os_getErrno(), shmPipeName);
            os_free(shmPipeName);
            return os_resultFail;
        }
    }
    /* Write message to monitor process. First part is command, second part is our process ID */
    fSuccess = WriteFile(
            hPipe,                 // pipe handle
            message,               // message
            sizeof(*message),      // message length
            &lWritten,             // bytes written
            NULL);                 // not overlapped
    if (!fSuccess) {
        result = os_resultFail;
        OS_REPORT(OS_INFO, "sharedMemoryProcessMonitor", 0,
                  "Failed to write message to pipe"
                  "System Error Code:  %d", (int)os_getErrno());
    }
    FlushFileBuffers(hPipe);
    DisconnectNamedPipe(hPipe);
    /* Close our side of pipe */
    CloseHandle(hPipe);
    os_free(shmPipeName);
    return (result);
}



static os_result
writeMessageToPipeAndWaitReply(
    struct os_shmMessage *request,
    struct os_shmMessage *reply,
    os_sharedHandle shmHandle)
{
    BOOL   fSuccess = FALSE;
    DWORD nRead, lastError;
    // Try to open a named pipe; wait for it, if necessary.
    char *shmPipeName = getShmPipeName(shmHandle);

    do {
        fSuccess = CallNamedPipe(
                shmPipeName,
                request, sizeof(*request),
                reply, sizeof(*reply),
                &nRead,
                NMPWAIT_USE_DEFAULT_WAIT);

        if(!fSuccess){
            lastError = os_getErrno();
        } else {
            lastError = ERROR_SUCCESS;
        }
    } while((!fSuccess) && (lastError == ERROR_PIPE_BUSY));

    os_free(shmPipeName);

    if (!fSuccess) {
        OS_REPORT(OS_ERROR, "writeMessageToPipeAndWaitReply", 0,
                "Failed to call shared memory message pipe"
                "System Error Code:  %d", (int)os_getErrno());
        goto pipe_error;
    }

    if (nRead != sizeof(*reply)) {
        OS_REPORT(OS_ERROR, "writeMessageToPipeAndWaitReply", 0,
                  "Reply message is has incorrect size");
        goto pipe_error;
    }

    return os_resultSuccess;

pipe_error:
    return os_resultFail;
}


static void
os__sharedMemoryClientShmDomainFree(
    os__clientShmDomain clientShmDomain)
{
    os_result result = os_resultSuccess;
    if (clientShmDomain != NULL) {
        clientShmDomain->onServerDied = NULL;
        if (clientShmDomain->hWaitHandle != NULL) {
            if (!UnregisterWait(clientShmDomain->hWaitHandle)) {
                OS_REPORT(OS_ERROR, "os__sharedMemoryClientShmDomainFree", 0,
                                  "Failed to unregister wait handle"
                                  "System Error Code:  %d", (int)os_getErrno());
            }
        }
        os_free(clientShmDomain);
    }
}

static os__clientShmDomain
os__sharedMemoryClientShmDomainNew(
    os_sharedHandle sharedHandle)
{
    os__clientShmDomain clientShmDomain;

    clientShmDomain = (os__clientShmDomain)(os_malloc(OS_SIZEOF(os__clientShmDomain)));

    if (clientShmDomain != NULL) {
        clientShmDomain->shmhandle = sharedHandle;
        clientShmDomain->hWaitHandle = NULL;
        clientShmDomain->onServerDied = NULL;
        clientShmDomain->args = NULL;
    } else {
        OS_REPORT(OS_ERROR,"os__sharedMemoryClientShmDomainNew",0,
                      "os_malloc(%d) failed", OS_SIZEOF(os__clientShmDomain));
    }

    return clientShmDomain;
}


static os_result
os_sharedMemoryRegisterProcess(
    os_sharedHandle shmHandle)
{
    os_result result = os_resultFail;
    os__clientShmDomain clientShmDomain;
    struct os_shmMessage request;
    struct os_shmMessage reply;

    clientShmDomain = os__sharedMemoryClientShmDomainNew(shmHandle);
    if(clientShmDomain == NULL){
        OS_REPORT_WID(OS_WARNING,"os_sharedMemoryRegisterProcess",0, shmHandle->id,
                     "Failed to create shared memory client");
        goto err_alloc;
    }

    request.kind = OS_SHM_MESSAGE_CLIENT_CONNECT_IND;
    request._u.procId = os_procIdSelf();

    result = writeMessageToPipeAndWaitReply(&request, &reply, shmHandle);
    if (result != os_resultSuccess) {
        goto err_write;
    }

    if (reply.kind != OS_SHM_MESSAGE_CLIENT_CONNECT_RSP) {
        OS_REPORT_WID(OS_WARNING,"os_sharedMemoryRegisterProcess",0, shmHandle->id,
                     "Register process id with server failed");
        goto err_reply;
    }

    clientShmDomain->serverId = reply._u.procId;

    os_mutexLock(&os__shmAttachedLock);
    os__shmAttached = os_iterAppend(os__shmAttached, clientShmDomain);
    os_mutexUnlock(&os__shmAttachedLock);

    return os_resultSuccess;

err_reply:
err_write:
err_alloc:
    return result;
}

static os_int32
findClientByHandle (
    void *obj,
    os_iterActionArg arg)
{
    os_sharedHandle handle = (os_sharedHandle)arg;
    os__clientShmDomain clientShmDomain = (os__clientShmDomain)obj;

    if(clientShmDomain->shmhandle == handle){
        return 1;
    }
    return 0;
}


static os_result
os_sharedMemoryDeregisterProcess(
    os_sharedHandle shmHandle)
{
    os_result result = os_resultFail;
    os__clientShmDomain clientShmDomain;
    struct os_shmMessage request;

    os_mutexLock(&os__shmAttachedLock);
    clientShmDomain = (os__clientShmDomain)(os_iterTakeAction(os__shmAttached, findClientByHandle, shmHandle));
    if (clientShmDomain) {
        clientShmDomain->onServerDied = NULL;
    }
    os_mutexUnlock(&os__shmAttachedLock);

    os__sharedMemoryClientShmDomainFree(clientShmDomain);

    request.kind = OS_SHM_MESSAGE_CLIENT_DISCONNECT;
    request._u.procId = os_procIdSelf();

    result = writeMessageToPipe(&request, shmHandle);
    if (result != os_resultSuccess) {
        OS_REPORT(OS_ERROR, "os_sharedMemoryDeregisterProcess", 0,
                  "Failed to send notification to server");
    }

    return os_resultSuccess;
}

VOID CALLBACK serverDiedHandler(
  PVOID lpParameter,
  BOOLEAN TimerOrWaitFired)
{
    os_sharedHandle shmHandle = (os_sharedHandle)lpParameter;
    os__clientShmDomain clientShmDomain;
    os_onSharedMemoryManagerDiedCallback onServerDied = NULL;
    void *args;

    os_mutexLock(&os__shmAttachedLock);
    clientShmDomain = (os__clientShmDomain)(os_iterReadAction(os__shmAttached, findClientByHandle, shmHandle));
    if (clientShmDomain) {
        onServerDied = clientShmDomain->onServerDied;
        args = clientShmDomain->args;
    }
    os_mutexUnlock(&os__shmAttachedLock);

    if (onServerDied != NULL) {
        onServerDied(shmHandle, args);
    }
}

os_result
os_sharedMemoryRegisterServerDiedCallback(
    os_sharedHandle sharedHandle,
    os_onSharedMemoryManagerDiedCallback onServerDied,
    void *args)
{
    os_result result = os_resultUnavailable;
    os__clientShmDomain clientShmDomain;
    HANDLE hWaitHandle;
    HANDLE procHandle;

    os_mutexLock(&os__shmAttachedLock);
    clientShmDomain = (os__clientShmDomain)(os_iterReadAction(os__shmAttached, findClientByHandle, sharedHandle));
    os_mutexUnlock(&os__shmAttachedLock);

    if (clientShmDomain && clientShmDomain->serverId != OS_INVALID_PID) {
        procHandle = os_procIdToHandle(clientShmDomain->serverId);
        clientShmDomain->onServerDied = onServerDied;
        clientShmDomain->args = args;

        if (RegisterWaitForSingleObject(&hWaitHandle, procHandle, serverDiedHandler, sharedHandle, INFINITE, WT_EXECUTEONLYONCE)) {
            clientShmDomain->hWaitHandle = hWaitHandle;
            result = os_resultSuccess;
        } else {
            result = os_resultFail;
            OS_REPORT(OS_API_INFO, "os_sharedMemoryRegisterServerDiedCallback", 0,
                    "Failed to allocate a wait handler on process handle, "
                    "System Error Code:  %d", (int)os_getErrno());
        }
    } else {
        OS_REPORT(OS_API_INFO, "os_sharedMemoryRegisterServerDiedCallback", 0, "os_sharedMemoryRegisterServerDiedCallback server unknown");
    }

    return result;
}

static void
os_sharedMemoryTerminateMonitor(
    os_sharedHandle shmHandle)
{
    os_result result;
    struct os_shmMessage request;

    request.kind = OS_SHM_MESSAGE_MONITOR_EXIT;
    request._u.procId = os_procIdSelf();

    result = writeMessageToPipe(&request, shmHandle);
    if (result != os_resultSuccess) {
        OS_REPORT(OS_INFO, "os_sharedMemoryTerminateMonitor", 0,
                  "Failed to send exit notification to shared memory monitor");    }
}

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
                    OS_REPORT(OS_ERROR, "os_sharedMemoryWaitForClientChanges", 0,
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


os_state
os_sharedMemoryGetState(
    os_sharedHandle sharedHandle)
{
    os_state state = OS_STATE_NONE;
    os_win32_keyfile_data dummy;
    os_state tmp;
    os_result rv;
    char *fileName;
    FILE *fileDesc;

    assert(sharedHandle != NULL);

    fileName = os_sharedMemoryHandleGetKeyfile(sharedHandle);
    if (fileName) {
        fileDesc = fopen(fileName, "r");
        if (fileDesc != NULL) {
            /* Skip keyFile until (but not including) the first attached pid or domain state. */
            rv = os_win32_keyFileParser(fileDesc, &dummy);
            if (rv == os_resultSuccess) {
                /* Get last state, which is the current one. */
                while (os_win32_keyfile_getState(fileDesc, &tmp) == os_resultSuccess) {
                    state = tmp;
                }
            }
            fclose(fileDesc);
        }
    }

    return state;
}


os_result
os_sharedMemorySetState(
    os_sharedHandle sharedHandle,
    os_state state)
{
    os_result rv = os_resultUnavailable;
    DWORD written;
    BOOL writeResult;
    HANDLE fileHandle;
    char buffer[256];
    char *fileName;

    assert(sharedHandle != NULL);

    fileName = os_sharedMemoryHandleGetKeyfile(sharedHandle);
    if (fileName) {
        /* Have a pessimistic view on the outcome. */
        rv = os_resultFail;
        /* Open the file for an atomic append */
        fileHandle = CreateFile(fileName,
                                FILE_APPEND_DATA,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);
        if(fileHandle == INVALID_HANDLE_VALUE) {
            OS_REPORT(OS_ERROR, "os_sharedMemorySetState", 0,
                      "CreateFile failed while trying to append to key-file '%s': %s",
                      fileName, os_strError (os_getErrno()));
        } else {
            /* Fill the buffer with the state to be written.
             * Add an extra int(0) to be able to distinguish between the
             * state and pid info. */
            snprintf(buffer, sizeof(buffer), "%d 0\n", (int)state);
            /* Now write the buffer to the key file, appending it at the end
             * as we opened the file with the FILE_APPEND_DATA flag. */
            writeResult = WriteFile(fileHandle, buffer, (DWORD)strlen(buffer), &written, NULL);
            if(writeResult == 0) {
                OS_REPORT(OS_ERROR, "os_sharedMemorySetState", 0,
                          "WriteFile failed while trying to append to key-file '%s': %s",
                          fileName, os_strError (os_getErrno()));
            } else {
                rv = os_resultSuccess;
            }
            CloseHandle(fileHandle);
        }
    }

    return rv;
}


static char *
os_sharedMemoryHandleGetKeyfile(
    os_sharedHandle sharedHandle)
{
    if (sharedHandle->keyfile == NULL) {
        sharedHandle->keyfile = os_findKeyFileById(sharedHandle->id);
    }
    return sharedHandle->keyfile;
}


os_result os_sharedMemoryLock(os_sharedHandle sharedHandle) {
    OS_UNUSED_ARG(sharedHandle);
    return os_resultUnavailable;
}

void os_sharedMemoryUnlock(os_sharedHandle sharedHandle) {
    OS_UNUSED_ARG(sharedHandle);
    return;
}

