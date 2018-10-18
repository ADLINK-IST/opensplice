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
/** \file os/wince/code/os_sharedmem.c
 *  \brief WinCE shared memory management
 *
 * Implements shared memory management for WinCE
 */

#include "os_sharedmem.h"
#include "os__sharedmem.h"
#include "os__time.h"
#include "os_stdlib.h"
#include "os_report.h"
#include "os_signal.h"

#include "os_heap.h"
#include "os_thread.h"
#include "code/os__debug.h"
#include "os_errno.h"

#include <assert.h>
#include <stdio.h>


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


static char *
os_sharedMemoryHandleGetKeyfile(
    os_sharedHandle);


/** \brief Structure providing keyfile data
 *
 */
typedef struct os_wince_keyfile_data {
    os_int32   domain_id;
    os_char    domain_name[OS_KEYFILE_DOMAIN_NAME_SIZE];
    os_address address;
    os_address size;
    os_char    implementation_id[OS_KEYFILE_IMPL_ID_SIZE];
    os_int32   creator_pid;
} os_wince_keyfile_data;

struct os_sharedHandle_s {
    os_sharedAttr attr;
    void *mapped_address;
    char *name;
    HANDLE mapObject;
    os_address size;
    os_int32 shm_created;
    int id;
    char* keyfile;
};

struct os_shmInfo {
    os_sharedHandle sharedHandle;
    struct os_shmInfo *next;
};

/*
 * Return  os_resultSuccess - Complete action succeeded, no more calls expected
 *         os_resultBusy    - Action succeded, but expects more (or none) calls
 *         other            - Failure
 */
typedef os_result
(*os_wince_keyfile_loop_action)(const char *key_file_name, os_wince_keyfile_data *data, void *action_arg);


os_result
os_wince_keyfile_getAddress(
    FILE *key_file,
    os_address *data)
{
    char line[OS_KEYFILE_LINE_SIZE_MAX];
    os_result rv = os_resultFail;
    if (fgets(line, OS_KEYFILE_LINE_SIZE_MAX, key_file) != NULL) {
        if (sscanf(line, "%lx", data) == 1) {
            rv = os_resultSuccess;
        }
    }
    return rv;
}

os_result
os_wince_keyfile_getInt(
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
os_wince_keyfile_getString(
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
os_wince_keyfile_getIntArray(
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
os_wince_keyfile_getState(
    FILE *key_file,
    os_state *data)
{
    os_result rv = os_resultFail;
    int pids[OS_KEYFILE_INT_ARRAY_SIZE];
    int cnt;
    /* Try getting next state. */
    while ((cnt = os_wince_keyfile_getIntArray(key_file, pids, OS_KEYFILE_INT_ARRAY_SIZE)) != 0) {
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
os_wince_keyfile_getPid(
    FILE *key_file,
    os_int32 *pid)
{
    os_result rv = os_resultFail;
    int pids[OS_KEYFILE_INT_ARRAY_SIZE];
    int cnt;
    /* Try getting next pid info. */
    while ((cnt = os_wince_keyfile_getIntArray(key_file, pids, OS_KEYFILE_INT_ARRAY_SIZE)) != 0) {
        /* Valid pid info has one or three ints.
         * Only the first int has real information in wince. */
        if (cnt == 1) {
            *pid = pids[0];
            /* We've found the next pid info: stop searching. */
            rv = os_resultSuccess;
            break;
        } else if (cnt == 3) {
            *pid = pids[0];
            /* We've found the next pid info: stop searching. */
            rv = os_resultSuccess;
            break;
        }
    }
    return rv;
}




typedef struct os_wince_keyfile_dataArg {
    os_int32 id;
    const char *name;
    os_wince_keyfile_data data;
} os_wince_keyfile_dataArg;


static os_result
os_wince_getDataByIdAction(
    const char *key_file_name,
    os_wince_keyfile_data *data,
    void *action_arg)
{
    os_result rv = os_resultBusy; /* busy: keep searching */
    os_wince_keyfile_dataArg *arg = (os_wince_keyfile_dataArg*)action_arg;

    assert(arg);
    assert(data);
    assert(key_file_name);

    if (data->domain_id == arg->id) {
        memcpy(&(arg->data), data, sizeof(os_wince_keyfile_data));
        rv = os_resultSuccess; /* success: stop searching */
    }

    return rv;
}

static os_result
os_wince_getDataByNameAction(
    const char *key_file_name,
    os_wince_keyfile_data *data,
    void *action_arg)
{
    os_result rv = os_resultBusy; /* busy: keep searching */
    os_wince_keyfile_dataArg *arg = (os_wince_keyfile_dataArg*)action_arg;

    assert(arg);
    assert(data);
    assert(arg->name);
    assert(key_file_name);

    if (strcmp(data->domain_name, arg->name) == 0) {
        memcpy(&(arg->data), data, sizeof(os_wince_keyfile_data));
        rv = os_resultSuccess; /* success: stop searching */
    }

    return rv;
}



/** \brief Parse the keyfile into an os_wince_keyfile_data struct.
 *
 */
static os_result
os_wince_keyFileParser(
    FILE *key_file,
    os_wince_keyfile_data *data)
{
    os_result rv;

    assert(key_file);
    assert(data);

/* NB WINCE temp file format differs from Linux/Windows after Implementation line
   WINCE                    WIN                            LINUX
   domain name              domain name                    domain name
   shmem address start      shmem address start            shmem address start
   shmem size               shmem size                     shmem size
   shmem implementation     shmem implementaion            shmem implementation
   domain id                creator process                creator process (spliced)
   creator process          domain id                      domain id
   child processes          child processes
   ..                       ..
*/

    /*
     * The wince keyfile contains the following lines:
     *      - Domain name
     *      - Address
     *      - Size
     *      - Implementation id
     *      - Domain id
     *      - Creator pid
     *      - Attached pid
     *      - State
     *      - Attached pid
     *      - State
     *      - State
     *      - Attached pid
     *      - Attached pid
     *            etc
     */
    rv = os_wince_keyfile_getString(key_file, data->domain_name, OS_KEYFILE_DOMAIN_NAME_SIZE);
    if (rv == os_resultSuccess) {
        rv = os_wince_keyfile_getAddress(key_file, &(data->address));
    }
    if (rv == os_resultSuccess) {
        rv = os_wince_keyfile_getAddress(key_file, &(data->size));
    }
    if (rv == os_resultSuccess) {
        rv = os_wince_keyfile_getString(key_file, data->implementation_id, OS_KEYFILE_IMPL_ID_SIZE);
    }
    if (rv == os_resultSuccess) {
        rv = os_wince_keyfile_getInt(key_file, &(data->domain_id));
    }
    if (rv == os_resultSuccess) {
        rv = os_wince_keyfile_getInt(key_file, &(data->creator_pid));
    }
    /* We ignore the attached PIDs and states during this parsing.
     * The function that is interrested in them, should call
     * os_wince_keyfile_getPidInfo or os_wince_keyfile_getState
     * itself, after this parsing. */

    return rv;
}

/** \brief Check if the contents of the identified key file
 *         matches the identified id and and set the name
 *
 * \b os_wince_loopKeyFiles tries to compare the contents of the identified
 * key file in \b key_file_name with the identified \b id.
 * On a match 1 will be returned and the domain name will be set,
 * on a mismatch 0 will be returned and name will be NULL.
 */
/*
 * Return  os_resultSuccess - The action function was called at least once and succeeded.
 *         other            - Failure or no calls to action.
 */
static os_result
os_wince_loopKeyFiles(
    os_wince_keyfile_loop_action action,
    void *action_arg)
{
    os_result rv = os_resultBusy;
    HANDLE fileHandle;
    WIN32_FIND_DATA fileData;
    char key_file_name[MAX_PATH];
    FILE *key_file;
    os_wince_keyfile_data data;
    int actionCalled = 0;
    wchar_t *wfname;
    os_char *ofname;

    key_file = NULL;

    if (key_file_path == NULL) {
        key_file_path = os_getTempDir();
    }

    if (key_file_path == NULL) {
        OS_REPORT(OS_ERROR, "os_wince_loopKeyFiles", 0, "Failed to determine temporary directory");
        return os_resultFail;
    }

    os_strcpy(key_file_name, key_file_path);
    /*os_strcat(key_file_name, "\\");*/
    os_strcat(key_file_name, key_file_prefix);
    os_strcat(key_file_name, "*.tmp");
    wfname = wce_mbtowc(key_file_name);
    fileHandle = FindFirstFile(wfname, &fileData);
    os_free(wfname);

    if (fileHandle == INVALID_HANDLE_VALUE) {
        /* Try to communicate error message via thread-specific memory */
#define ERR_MESSAGE "os_wince_loopKeyFiles: Could not find any key file: %s"
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
            OS_REPORT(OS_WARNING, "os_wince_loopKeyFiles", 0, "Could not find any key file: %s", key_file_name);
        }
#undef ERR_MESSAGE
        return os_resultFail;
    }

    /* Busy means continue searching. */
    while (rv == os_resultBusy) {
        /* Get, open, parse and perform action on current KeyFile. */
        ofname = wce_wctomb(fileData.cFileName);
        os_strcpy(key_file_name, key_file_path);
        /*os_strcat(key_file_name, "\\");*/
        os_strcat(key_file_name, ofname);
        os_free(ofname);
        key_file = fopen(key_file_name, "r");
        if (key_file != NULL) {
            rv = os_wince_keyFileParser(key_file, &data);
            fclose(key_file);
            if (rv == os_resultSuccess) {
                rv = action(key_file_name, &data, action_arg);
                actionCalled = 1;
            } else {
                /* The parsing failed. This can have a number of valid
                 * reasons:
                 *  - No rights; keyfile was created by another user.
                 *  - Not enough data; keyfile is being created right now.
                 *  - etc.
                 * Because we don't know the reason it failed, just asume
                 * that it is a valid reason and just continue. */
                rv = os_resultBusy;
                OS_REPORT(OS_INFO, "os_wince_loopKeyFiles", 0, "Failed to parse key file: %s", key_file_name);
            }
        } else {
            OS_REPORT(OS_INFO, "os_wince_loopKeyFiles", 0, "Failed to open key file: %s", key_file_name);
        }

        /* Get next file when we need to continue searching. */
        if (rv == os_resultBusy) {
            if (FindNextFile(fileHandle, &fileData) == 0) {
                /* We handled the last key file. */
                rv = os_resultSuccess;
            }
        }
    }

    FindClose(fileHandle);

    /* When action() isn't called, it's considered a failure. */
    if (rv == os_resultSuccess) {
        if (actionCalled == 0) {
            rv = os_resultFail;
        }
    }

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
}

void os_sharedMemoryExit(void)
{
}

const char *
os_getDomainNameforMutex(
    os_mutex *mutex)
{
    /* get shm and look if pointer is in memory range */
    os_address base,size,mtx;
    struct os_shmInfo *shmInf;
    const char *result = NULL;
    os_int32 foundName = 0;

    assert(mutex);

    for (shmInf = shmInfo;shmInf != NULL && foundName==0;shmInf = shmInf->next) {
        base = (os_address)shmInf->sharedHandle->mapped_address;
        size = (os_address)shmInf->sharedHandle->size;
        mtx = (os_address)mutex;
        /* check if the mutex is in the shm range */
        if((base < mtx) && (mtx < (base+size) )) {
            result = (char *)shmInf->sharedHandle->name;
            foundName =1;
        }
    }
    /* is no shm present name = null */
    return result;
}

const char* os_getDomainNameforCond (os_cond* cond)
{
    /* get shm and look if pointer is in memory range */
    os_address base, size, cnd;
    struct os_shmInfo* shmInf;
    const char* result = NULL;
    os_int32 foundName = 0;

    assert(cond);

    for (shmInf = shmInfo;shmInf != NULL && foundName==0;shmInf = shmInf->next)
    {
        base = (os_address)shmInf->sharedHandle->mapped_address;
        size = (os_address)shmInf->sharedHandle->size;
        cnd = (os_address)cond;

        /* check if the cond is in the shm range */
        if((base < cnd) && (cnd < (base+size) ))
        {
            result = (char*)shmInf->sharedHandle->name;
            foundName =1;
        }
    }
    /* is no shm present name = null */
    return result;
}

os_address
os_getShmBaseAddressFromPointer(
    void *vpointer)
{
     /* get shm and look if pointer is in memory range */
     struct os_shmInfo *shmInf;
     os_address base,size,pointer;
     os_address result;
     os_int32 foundName = 0;

     if (shmInfo != NULL) {
         result = (os_address)shmInfo->sharedHandle->mapped_address;
     } else {
         result = 0;
     }

     if (vpointer != NULL && shmInfo != NULL) {

         for (shmInf = shmInfo;shmInf != NULL && foundName==0;shmInf = shmInf->next) {
            base = (os_address)shmInf->sharedHandle->mapped_address;
            size = (os_address)shmInf->sharedHandle->size;
            pointer = (os_address)vpointer;
            /* check if the pointer is in the shm range */
            if((base < pointer) && (pointer < (base+size) )) {
                result = (os_address)shmInf->sharedHandle->mapped_address;
                foundName = 1;
            }
         }
     }
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
            strcpy(sh->name, name);
            sh->attr = *sharedAttr;
            sh->mapped_address = (void *)0;
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
typedef struct os_wince_findArg {
    os_int32 id;
    const char *name;
    char *key_file;
} os_wince_findArg;


static os_result
os_findKeyFileAction(
    const char *key_file_name,
    os_wince_keyfile_data *data,
    void *action_arg)
{
    os_result rv = os_resultBusy; /* busy: keep searching */
    os_wince_findArg *arg = (os_wince_findArg*)action_arg;

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
    os_wince_findArg arg;
    arg.id = 0;
    arg.name = name;
    arg.key_file = NULL;
    (void)os_wince_loopKeyFiles(os_findKeyFileAction, (void*)&arg);
    return arg.key_file;
}



static os_result
os_findKeyFileByIdAction(
    const char *key_file_name,
    os_wince_keyfile_data *data,
    void *action_arg)
{
    os_result rv = os_resultBusy; /* busy: keep searching */
    os_wince_findArg *arg = (os_wince_findArg*)action_arg;

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
    os_wince_findArg arg;
    arg.id = id;
    arg.name = NULL;
    arg.key_file = NULL;
    (void)os_wince_loopKeyFiles(os_findKeyFileByIdAction, (void*)&arg);
    return arg.key_file;
}


static os_result
os_findKeyFileByNameAndIdAction(
    const char *key_file_name,
    os_wince_keyfile_data *data,
    void *action_arg)
{
    os_result rv = os_resultBusy; /* busy: keep searching */
    os_wince_findArg *arg = (os_wince_findArg*)action_arg;

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
    os_wince_findArg arg;
    arg.id = id;
    arg.name = name;
    arg.key_file = NULL;
    (void)os_wince_loopKeyFiles(os_findKeyFileByNameAndIdAction, (void*)&arg);
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
    os_int32 pidtemp;
    os_int32 retVal = 0;
    os_wince_keyfile_data data;

    char pidstr[16];
    char *listpidstr;

    /* get creator pid and associated pids from fileName */

    if (fileName != NULL)
    {
        key_file = fopen(fileName, "r");
        if (key_file != NULL)
        {
            /* Parse keyFile until (but not including) the first attached pid. */
            if (os_wince_keyFileParser(key_file, &data) == os_resultSuccess) {
                /* Start list with the creator pid. */
                pidtemp = data.creator_pid;
                do {
                    /* change pid to string to match iterator model */
                    sprintf(pidstr,"%d",pidtemp);
                    listpidstr =  os_strdup(pidstr);
                    os_iterAppend(pidList, listpidstr);
                /* Next attached pid. */
                } while (os_wince_keyfile_getPid(key_file, &pidtemp) == os_resultSuccess);
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
    os_wince_keyfile_data *data,
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
    if (os_wince_loopKeyFiles(os_sharedMemoryListDomainNamesAction,
                              (void*)nameList) == os_resultSuccess) {
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
    os_wince_keyfile_data *data,
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
    if (os_wince_loopKeyFiles(os_listDomainIdsAction,
                              (void*)&arg) == os_resultSuccess) {
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
    wchar_t* tempWString;
    HANDLE fileHandle;
    WIN32_FIND_DATA fileData;

    if (name == NULL)
    {
        return 1;
    }

    tempWString = wce_mbtowc(name);
    fileHandle = FindFirstFile(tempWString, &fileData);

    if (fileHandle != INVALID_HANDLE_VALUE)
    {
        /* file exists so delete */
        CloseHandle(fileHandle);

        if (DeleteFile(tempWString) == 0)
        {
            OS_REPORT(OS_ERROR, "os_destroyKeyFile", 0, "Can not Delete the key file %d", os_getErrno());
        }
    }

    os_free(tempWString);
    return 0;
}

int
os_sharedMemorySegmentFree(
    const char * name)
{
    return 0;
}

void
os_cleanSharedMemAndOrKeyFiles(
    void)
{
    HANDLE hProcess;
    BOOL doClean = FALSE;
    os_iter pidList = NULL;
    os_iter nameList = NULL;
    char * fname;
    char * name;
    os_int32 userProc;
    os_int32 procResult;
    BOOL doTest = FALSE;
    char *pidstr = NULL;

    nameList = os_iterNew(NULL);
    pidList = os_iterNew(NULL);

    if (os_sharedMemoryListDomainNames(nameList) == 0)
    {

        while ((name = (char *)os_iterTakeFirst(nameList)) != NULL)
        {
            fname = os_findKeyFile(name);

            if (os_sharedMemoryListUserProcesses(pidList, fname) == 0)
            {
                doClean = FALSE;
                doTest = TRUE;

                while ((pidstr = (char *)os_iterTakeFirst(pidList)) != NULL)
                {
                    userProc = atoi(pidstr);
                    if (doTest)
                    {
                        /* test creator process  */

                        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, userProc);

                        if (hProcess == NULL)
                        {
                            doClean = TRUE;    /* no creator process to destroy so just del file */
                            break;
                        }
                        else
                        {
                            if (os_procCheckStatus((os_procId)hProcess, &procResult) != os_resultBusy)
                            {
                                doClean = TRUE; /* creator not busy so destroy files and any processes  */
                            }
                            CloseHandle(hProcess);
                        }
                        doTest = FALSE;

                        if(doClean)
                        {
                            /* remove creator process */
                            os_procServiceDestroy(userProc, FALSE, 100);
                        }
                    }
                    else
                    {
                        if (doClean)
                        {
                            /* remove remaining associated processes */
                            os_procServiceDestroy(userProc, FALSE, 100);
                        }
                    }
                }
                os_sharedMemoryListUserProcessesFree(pidList);

                if (doClean)
                {
                    /* remove tmp file */
                    os_destroyKeyFile(fname);
                }
            }
        }
        os_sharedMemoryListDomainNamesFree(nameList);
    }
}


static os_uint32
os_getSize(
    const char *name)
{
    os_wince_keyfile_dataArg arg;
    os_result rv;
    os_uint32 size = 0;

    arg.id = 0;
    arg.name = name;

    rv = os_wince_loopKeyFiles(os_wince_getDataByNameAction, &arg);
    if (rv == os_resultSuccess) {
        size = (os_uint32)arg.data.size;
    }

    return size;
}


/** \brief create a temporary file that contains the shared memory information
 *
 * \b os_findKeyFile locates the temporary file referencing the shared memory
 * if it already exists for the particular name.  In the case when it does not
 * already exist, the \b os_createShmTempFile function will create one containing
 * these details (name, size, pid).  When this operation is successful, 0 is
 * returned (-1 otherwise).
 */

static int
os_createShmTempFile(
    const char *name,
    int create,
    int size,
    int id
)
{
    char *key_file_name;
    UINT unique;
    HANDLE fileHandle;
    DWORD written;
    DWORD pid;
    char buf[512];
    LPWSTR retFileName;
    wchar_t* wStringPath;
    wchar_t* wStringPrefix;

    key_file_name = os_findKeyFile(name);
    fflush(stdout);

    if (key_file_name == NULL)
    {
        fflush(stdout);

        if (create == 0)
        {
            return -1;
        }

        retFileName = (LPWSTR) os_malloc(MAX_PATH);
        wStringPath = wce_mbtowc(key_file_path);
        wStringPrefix = wce_mbtowc(key_file_prefix);
        unique = GetTempFileName(wStringPath, wStringPrefix, 0, retFileName);

        if (unique == 0)
        {
            OS_REPORT_WID(OS_ERROR, "OS Abstraction", 0, id,
                      "GetTempFileName failed for (%s, %s, 0, <out>): System Error Code %d",
                      key_file_path, key_file_prefix,os_getErrno());
            os_free (wStringPath);
            os_free (wStringPrefix);
            return -1;
        }
        os_free (wStringPath);
        os_free (wStringPrefix);

        fileHandle = CreateFile(retFileName,
                                GENERIC_READ | GENERIC_WRITE,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL,
                                CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);

        if (fileHandle == INVALID_HANDLE_VALUE)
        {
            OS_REPORT_WID(OS_ERROR, "OS Abstraction", 0, id, "CreateFile failed, System Error Code: %d", os_getErrno());
            os_free(retFileName);
            return -1;
        }

        os_free(retFileName);

        pid = GetCurrentProcessId();
        /* use 0 for creation time as win ce does not support GetProcessTimes() */
        snprintf(buf,sizeof(buf),"%s\n<map_address>\n%x\n<implementation>\n%d\n%d %d %d\n", name, size, id, pid, 0, 0);
        if (WriteFile((HANDLE)fileHandle, buf, strlen(buf), &written, NULL) == 0)
        {
            OS_REPORT_WID(OS_ERROR, "os_createShmTempFile", 0, id,
                      "WriteFile failed, System Error Code:  %d", os_getErrno());
            return -1;
        }
        CloseHandle((HANDLE)fileHandle);
    }
    else
    {
       os_free(key_file_name);
    }

    return 0;
}

/** \brief Get a SVR4 IPC key for a shared meory segment by name
 *
 * \b os_getKey tries to find the SVR4 IPC key for a named
 * shared memory segment by calling \b os_findKeyFile.
 *
 * If the key file is found, the key is created by calling \b ftok
 * which translates a file path into a key. The key is then returned.
 *
 * If the key file could not be found, -1 is returned to the caller
 * if \b create = 0. If \b create is != 0 however, it creates a new
 * key file by calling \b mkstemp, which creates and opens a new
 * unique file based upon the provided path. The \b name is then
 * written in the key file after which it is closed. With the
 * call to \b ftok, the key file is then translated into a key
 * which is returned after the \b key_file_name is freed.
 */
static char *
os_getMapName(
    const char *name)
{
    char *key_file_name;
    char *map_name;
    char *key;

    key_file_name = os_findKeyFile(name);
    if (key_file_name == NULL)
    {
        return NULL;
    }
    key = key_file_name + strlen(key_file_name);
    while (*key != '\\' && *key != '/')
    {
        key--;
    }
    key++;
    map_name = (char*)os_malloc(MAX_PATH);
    strcpy(map_name, key);
    strcat(map_name, "_MAP");
    os_free(key_file_name);
    return map_name;
}

/** \brief Destory the key related to the name
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
    wchar_t* wStringName;

    key_file_name = os_findKeyFile(name);
    if (key_file_name == NULL)
    {
        return -1;
    }

    wStringName = wce_mbtowc(key_file_name);
    if (DeleteFile(wStringName) == 0)
    {
        OS_REPORT(OS_ERROR, "OS Abstraction", 0, "Can not Delete the key file %d", os_getErrno());
        os_free(key_file_name);
        os_free(wStringName);
        return -1;
    }
    os_free(key_file_name);
    os_free(wStringName);

    return 0;
}

static os_result
os_sharedMemoryCreateFile(
    os_sharedHandle sharedHandle,
    os_address size)
{
    HANDLE md;
    wchar_t* wStringName;

    if (os_createShmTempFile (sharedHandle->name, 1, size, sharedHandle->id) != 0)
    {
        OS_REPORT(OS_ERROR, "OS Abstraction", 0,
                  "Unable to create shared memory for %s : %d",
                  sharedHandle->name, os_getErrno());
        return os_resultFail;
    }

    OS_REPORT(OS_INFO, "OS Abstraction", 0,
              "Creating shared memeory with name %s and size %d",
              sharedHandle->name, size);

    wStringName = wce_mbtowc(sharedHandle->name);
    md = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
                           0, size, wStringName);
    if (md == NULL)
    {
        OS_REPORT(OS_ERROR, "OS Abstraction", 0,
                  "Can not Create mapping object (%s) %d",
                  sharedHandle->name, os_getErrno());
        os_free(wStringName);
        return os_resultFail;
    }

    os_free(wStringName);

    sharedHandle->mapObject = md;
    sharedHandle->size = size;
    sharedHandle->shm_created = 1;
    return os_resultSuccess;
}

static os_result
os_sharedMemoryAttachFile(
    os_sharedHandle sharedHandle)
{
    HANDLE md;
    void *address;
    wchar_t* wStringName;

    if (!sharedHandle->shm_created)
    {
        if (os_createShmTempFile(sharedHandle->name, 0, 0, sharedHandle->id) != 0)
        {
            OS_REPORT_WID(OS_ERROR, "OS Abstraction", 0, sharedHandle->id,
                      "Unable to create shared memory for %s : %d",
                      sharedHandle->name, os_getErrno());
            return os_resultFail;
        }

        sharedHandle->size = os_getSize(sharedHandle->name);

        OS_REPORT_WID(OS_INFO, "OS Abstraction", 0, sharedHandle->id,
                  "Attaching to shared memory with name %s, size is %d",
                  sharedHandle->name, sharedHandle->size);


        wStringName = wce_mbtowc(sharedHandle->name);
        md = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
                               0, sharedHandle->size, wStringName);
        if (md == NULL)
        {
            OS_REPORT_WID(OS_ERROR, "OS Abstraction", 0, sharedHandle->id,
                      "Can not Open mapping object (%s) %d",
                      sharedHandle->name, os_getErrno());
            os_free(wStringName);
            return os_resultFail;
        }

        os_free(wStringName);

        sharedHandle->mapObject = md;
        sharedHandle->shm_created = 1;
    }

    address = MapViewOfFile(sharedHandle->mapObject, FILE_MAP_ALL_ACCESS, 0, 0, sharedHandle->size);

    if (address == NULL)
    {
        OS_REPORT_WID(OS_ERROR, "OS Abstraction", 0, sharedHandle->id,
                  "Can not Map View Of file %d", os_getErrno());
        return os_resultFail;
    }

    sharedHandle->mapped_address = address;
    return os_resultSuccess;
}

static os_result
os_sharedMemoryDetachFile(
    os_sharedHandle sharedHandle)
{
    if (UnmapViewOfFile(sharedHandle->mapped_address) == 0)
    {
        OS_REPORT(OS_ERROR, "OS Abstraction", 0,
                  "Can not Unmap View Of file %d", os_getErrno());
        return os_resultFail;
    }
    sharedHandle->mapped_address = NULL;

    if (!sharedHandle->shm_created)
    {
        if (CloseHandle(sharedHandle->mapObject) == 0)
        {
            OS_REPORT(OS_ERROR, "OS Abstraction", 0,
                      "Can not close handle of map: System Error Code %d",
                      os_getErrno());
            return os_resultFail;
        }
        sharedHandle->mapObject = 0;
    }
   return os_resultSuccess;
}

static os_result
os_sharedMemoryDestroyFile(
    os_sharedHandle sharedHandle)
{
    if (CloseHandle(sharedHandle->mapObject) == 0)
    {
        OS_REPORT(OS_ERROR, "OS Abstraction", 0,
                  "Can not close handle of map %d", os_getErrno());
        return os_resultFail;
    }
    sharedHandle->mapObject = 0;

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
    assert(size > 0);
    switch (sharedHandle->attr.sharedImpl)
    {
        case OS_MAP_ON_FILE:
            result = os_sharedMemoryCreateFile (sharedHandle, size);
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
    switch (sharedHandle->attr.sharedImpl)
    {
        case OS_MAP_ON_FILE:
            result = os_sharedMemoryDestroyFile (sharedHandle);
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
    switch (sharedHandle->attr.sharedImpl)
    {
        case OS_MAP_ON_FILE:
           result = os_sharedMemoryAttachFile (sharedHandle);
           if (result == os_resultSuccess) {
               /* add shm to shmInfo object */
               shmInf = os_malloc(sizeof(struct os_shmInfo));
               shmInf->sharedHandle = sharedHandle;
               shmInf->next = shmInfo;
               shmInfo = shmInf;
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

os_result
os_sharedMemoryDetach(
    os_sharedHandle sharedHandle)
{
    os_result result = os_resultFail;

    assert(sharedHandle != NULL);
    assert(sharedHandle->name != NULL);
    switch (sharedHandle->attr.sharedImpl)
    {
        case OS_MAP_ON_FILE:
            result = os_sharedMemoryDetachFile (sharedHandle);
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
os_sharedMemoryDetachUnclean(
    os_sharedHandle sharedHandle)
{
    return os_sharedMemoryDetach(sharedHandle);
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
    sharedAttr->map_address = (void *)0x00220000;
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
    os_wince_keyfile_dataArg arg;
    os_result rv;

    *name = NULL;
    arg.id = id;
    arg.name = NULL;

    rv = os_wince_loopKeyFiles(os_wince_getDataByIdAction, &arg);
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
    switch (sharedHandle->attr.sharedImpl)
    {
        case OS_MAP_ON_FILE:
            if (sharedHandle->size == 0)
            {
                result = os_resultFail;
            }
            else
            {
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
    LPWSTR LPW_keyFileName;

    keyFileName = os_findKeyFile(domainName);
    LPW_keyFileName = (LPWSTR) wce_mbtowc(keyFileName);

    if (keyFileName == NULL)
    {
        OS_REPORT(OS_ERROR, "os_sharedMemoryRegisterUserProcess", 0, "Unable to register process with PID '%d' as a user of shared "
                  "memory. This only affects clean up procedures in case of later failure.", pid);
    } else
    {
        /* Open the file for an atomic append */
        fileHandle = CreateFile(LPW_keyFileName,
                                GENERIC_WRITE,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);
        SetFilePointer (fileHandle, 0, NULL, FILE_END);

        if(fileHandle == INVALID_HANDLE_VALUE)
        {
            OS_REPORT(OS_ERROR, "os_sharedMemoryRegisterUserProcess", 0,
                      "CreateFile failed while trying to append to key file %s, "
                      "System Error Code:  %d", keyFileName, os_getErrno());
        } else
        {
            /* Fill the buffer with the pid info to be written */
            /* use 0 for creation time as win ce does not support GetProcessTimes() */
            snprintf(buffer, sizeof(buffer), "%d %d %d\n", pid, 0, 0);
            /* Now write the buffer to the key file, appending it at the end
             * as we opened the file with the FILE_APPEND_DATA flag
             */
            writeResult = WriteFile(fileHandle, buffer, strlen(buffer), &written, NULL);
            if(writeResult == 0)
            {
                OS_REPORT(OS_ERROR, "os_sharedMemoryRegisterUserProcess", 0,
                          "WriteFile failed, System Error Code:  %d", os_getErrno());
            }
            CloseHandle(fileHandle);
        }
        os_free(LPW_keyFileName);
    }
}

void
os_shmClientFree(
    os_shmClient client)
{
    return;
}

os_result
os_sharedMemoryWaitForClientChanges(
    os_sharedHandle sharedHandle,
    os_duration maxBlockingTime,
    os_shmClient* changedClients)
{
    ospl_os_sleep(maxBlockingTime);
    return os_resultTimeout;
}


os_state
os_sharedMemoryGetState(
    os_sharedHandle sharedHandle)
{
    os_state state = OS_STATE_NONE;
    os_wince_keyfile_data dummy;
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
            rv = os_wince_keyFileParser(fileDesc, &dummy);
            if (rv == os_resultSuccess) {
                /* Get last state, which is the current one. */
                while (os_wince_keyfile_getState(fileDesc, &tmp) == os_resultSuccess) {
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
    LPWSTR LPW_fileName;
    HANDLE fileHandle;
    BOOL writeResult;
    char buffer[256];
    char *fileName;
    DWORD written;
    char *errMsg;

    assert(sharedHandle != NULL);

    fileName = os_sharedMemoryHandleGetKeyfile(sharedHandle);
    if (fileName) {
        /* Have a pessimistic view on the outcome. */
        rv = os_resultFail;
        LPW_fileName = (LPWSTR)wce_mbtowc(fileName);
        if (LPW_fileName) {
            /* Open the file for an atomic append */
            fileHandle = CreateFile(LPW_fileName,
                                    GENERIC_WRITE,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL);
            if(fileHandle == INVALID_HANDLE_VALUE) {
                errMsg = os_strError(os_getErrno());
                OS_REPORT(OS_ERROR, "os_sharedMemorySetState", 0,
                          "CreateFile failed while trying to append to key-file '%s': %s",
                          fileName, errMsg);
            } else {
                SetFilePointer (fileHandle, 0, NULL, FILE_END);
                /* Fill the buffer with the state to be written.
                 * Add an extra int(0) to be able to distinguish between the
                 * state and pid info. */
                snprintf(buffer, sizeof(buffer), "%d 0\n", (int)state);
                /* Now write the buffer to the key file, appending it at the end
                 * as we opened the file with the FILE_APPEND_DATA flag. */
                writeResult = WriteFile(fileHandle, buffer, strlen(buffer), &written, NULL);
                if(writeResult == 0) {
                    errMsg = os_strError(os_getErrno());
                    OS_REPORT(OS_ERROR, "os_sharedMemorySetState", 0,
                              "WriteFile failed while trying to append to key-file '%s': %s",
                              fileName, errMsg);
                } else {
                    rv = os_resultSuccess;
                }
                CloseHandle(fileHandle);
            }
            os_free(LPW_fileName);
        } else {
            OS_REPORT(OS_ERROR, "os_sharedMemorySetState", 0, "No more resources");
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

os_result
os_sharedMemoryRegisterServerDiedCallback(
    os_sharedHandle sharedHandle,
    os_onSharedMemoryManagerDiedCallback onServerDied,
    void *args)
{
    return os_resultUnavailable;
}

os_result os_sharedMemoryLock(os_sharedHandle sharedHandle) {
    OS_UNUSED_ARG(sharedHandle);
    return os_resultUnavailable;
}

void os_sharedMemoryUnlock(os_sharedHandle sharedHandle) {
    OS_UNUSED_ARG(sharedHandle);
    return;
}

void
os_sharedMemoryImplDataCreate(
    os_sharedHandle sharedHandle)
{
    OS_UNUSED_ARG(sharedHandle);
}

void
os_sharedMemoryImplDataDestroy(
    os_sharedHandle sharedHandle)
{
    OS_UNUSED_ARG(sharedHandle);
}

