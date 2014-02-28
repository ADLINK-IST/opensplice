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
/** \file os/win32/code/os_sharedmem.c
 *  \brief WIN32 shared memory management
 *
 * Implements shared memory management for WIN32
 */

#include "os_sharedmem.h"
#include "os_stdlib.h"
#include "os_abstract.h"
#include "os_report.h"
#include "os_process.h"

#include "os_heap.h"
#include "os_thread.h"
#include "os_mutex.h"
#include "os_cond.h"
#include "os_init.h"
#include "os__debug.h"
#include "os_iterator.h"    /*2008 */
#include "os_signal.h"  /* 2008 */

#include <assert.h>
#include <stdio.h>

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#include <Sddl.h>

#define OS_LOG_AUTH     LOG_AUTH
#define OS_LOG_USER     LOG_USER
#define OS_LOG_WARNING  LOG_WARNING

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
};

struct os_shmInfo {
    os_sharedHandle sharedHandle;
    struct os_shmInfo *next;
};

/**
* @todo This is defined in os_time.c which now needs a header as this is used by this file.
*/
void
os_timeModuleReinit(const os_char*);

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

char *
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

char* os_getDomainNameforCond (os_cond* cond)
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

/** Defines the file prefix for the key file */

char *key_file_path = NULL;
const char key_file_prefix[] = "osp";

/** \brief Check if the contents of the identified key file
 *         matches the identified id and and set the name
 *
 * \b os_findNameById tries to compare the contents of the identified
 * key file in \b key_file_name with the identified \b id.
 * On a match 1 will be returned and the domain name will be set,
 * on a mismatch 0 will be returned and name will be NULL.
 */
static int
os_findNameById(
    int id,
    char **name)
{
    HANDLE fileHandle;
    WIN32_FIND_DATA fileData;
    char key_file_name[MAX_PATH], line[512];
    FILE *key_file;
    char *nl;
    int last = 0;
    int domainId;
    int retVal =0;

    key_file = NULL;
    nl = NULL;

    if (key_file_path == NULL) {
        key_file_path = os_getTempDir();
    }

    if (key_file_path == NULL) {
        OS_REPORT(OS_ERROR, "os_findNameById", 0, "Failed to determine temporary directory");
        return 0;
    }

    os_strcpy(key_file_name, key_file_path);
    os_strcat(key_file_name, "\\");
    os_strcat(key_file_name, key_file_prefix);
    os_strcat(key_file_name, "*.tmp");

    fileHandle = FindFirstFile(key_file_name, &fileData);

    if (fileHandle == INVALID_HANDLE_VALUE) {
        /* Try to communicate error message via thread-specific memory */
#define ERR_MESSAGE "os_findNameById: Could not find any key file: %s"
        char *message;
        os_size_t messageSize;

        messageSize = sizeof(ERR_MESSAGE) + strlen(key_file_name);
        /* Free any existing thread specific warnings */
        os_threadMemFree(OS_THREAD_WARNING);
        message = (char *)os_threadMemMalloc(OS_THREAD_WARNING, (os_int32)messageSize);
        if (message) {
            snprintf(message, messageSize, ERR_MESSAGE, key_file_name);
        } else {
            /* Allocation failed, use report mechanism */
            OS_REPORT_1(OS_WARNING, "os_findNameById", 0, "Could not find any key file: %s", key_file_name);
        }
        return 0;
    }

    while (!last && !retVal) {
        os_strcpy(key_file_name, key_file_path);
        os_strcat(key_file_name, "\\");
        os_strcat(key_file_name, fileData.cFileName);
        key_file = fopen(key_file_name, "r");
        if (key_file != NULL) {
            if (fgets(line, sizeof(line), key_file) != NULL) {
                /* Remove trailing newline char */
                if (nl = strchr(line, '\n')) {
                    *nl = 0;
                }
                *name = os_strdup(line);
            }
            fgets(line, sizeof(line), key_file);
            fgets(line, sizeof(line), key_file);
            fgets(line, sizeof(line), key_file);
            fgets(line, sizeof(line), key_file);
            if (fgets(line, sizeof(line), key_file) != NULL) {
                sscanf(line, "%d", &domainId);
            }
            fclose(key_file);
            if (id != domainId) {
                os_free(*name);
                *name = NULL;
            } else {
                retVal =1;
            }
        } else {
            OS_REPORT_1(OS_INFO, "os_findNameById", 0, "Failed to open key file: %s", key_file_name);
        }

        if (!retVal && FindNextFile(fileHandle, &fileData) == 0 ) {
            last = 1;
        }
    }

    FindClose(fileHandle);


    return retVal;
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
char *
os_findKeyFile(
    const char *name)
{
    HANDLE fileHandle;
    WIN32_FIND_DATA fileData;
    char key_file_name[MAX_PATH], domainId[512];
    FILE *key_file;
    char *kfn, *nl;
    int last = 0;

    key_file = NULL;
    kfn = NULL;
    nl = NULL;

    if (key_file_path == NULL) {
        key_file_path = os_getTempDir();
    }

    if (key_file_path == NULL) {
        OS_REPORT(OS_ERROR, "os_findKeyFile", 0, "Failed to determine temporary directory");
        return NULL;
    }

    os_strcpy(key_file_name, key_file_path);
    os_strcat(key_file_name, "\\");
    os_strcat(key_file_name, key_file_prefix);
    os_strcat(key_file_name, "*.tmp");

    fileHandle = FindFirstFile(key_file_name, &fileData);

    if (fileHandle == INVALID_HANDLE_VALUE) {
        /* Try to communicate error message via thread-specific memory */
        char *message;
        os_size_t messageSize;

        messageSize = sizeof(ERR_MESSAGE) + strlen(key_file_name);
        /* Free any existing thread specific warnings */
        os_threadMemFree(OS_THREAD_WARNING);
        message = (char *)os_threadMemMalloc(OS_THREAD_WARNING, (os_int32)messageSize);
        if (message) {
            snprintf(message, messageSize, ERR_MESSAGE, key_file_name);
        } else {
            /* Allocation failed, use report mechanism */
            OS_REPORT_1(OS_WARNING, "os_findKeyFile", 0, "Could not find any key file: %s", key_file_name);
        }
        return NULL;
    }

    while (!last) {
        os_strcpy(key_file_name, key_file_path);
        os_strcat(key_file_name, "\\");
        os_strcat(key_file_name, fileData.cFileName);
        key_file = fopen(key_file_name, "r");
        if (key_file != NULL) {
            if (fgets(domainId, 512, key_file) != NULL) {
                /* Remove trailing newline char */
                if (nl = strchr(domainId, '\n')) {
                    *nl = 0;
                }
                /* Compare domain name from key file with requested domain name */
                if(os_strncasecmp(name, domainId, (os_uint32)strlen(name)) == 0) {
                    kfn = (char*)os_malloc(strlen(key_file_name) + 1);
                    if (kfn != NULL) {
                        os_strcpy(kfn, key_file_name);
                        last = 1;
                    }
                }
            }
            fclose(key_file);
        } else {
            OS_REPORT_1(OS_INFO, "os_findKeyFile", 0, "Failed to open key file: %s", key_file_name);
        }

        if (FindNextFile(fileHandle, &fileData) == 0) {
            last = 1;
        }
    }

    FindClose(fileHandle);

    if (kfn == NULL) {
        OS_REPORT_1(OS_INFO, "os_findKeyFile", 0, "Could not find matching key file for domain: %s", name);
    }

    return kfn;
}

char *
os_findKeyFileByNameAndId(
    const char *name,
    const os_int32 id)
{
    HANDLE fileHandle;
    WIN32_FIND_DATA fileData;
    char key_file_name[MAX_PATH], domainName[512], line[512];
    FILE *key_file;
    char *kfn, *nl;
    int last = 0;
    int domainId;

    key_file = NULL;
    kfn = NULL;
    nl = NULL;

    if (key_file_path == NULL) {
        key_file_path = os_getTempDir();
    }

    if (key_file_path == NULL) {
        OS_REPORT(OS_ERROR, "os_findKeyFileByNameAndId", 0, "Failed to determine temporary directory");
        return NULL;
    }

    os_strcpy(key_file_name, key_file_path);
    os_strcat(key_file_name, "\\");
    os_strcat(key_file_name, key_file_prefix);
    os_strcat(key_file_name, "*.tmp");

    fileHandle = FindFirstFile(key_file_name, &fileData);

    if (fileHandle == INVALID_HANDLE_VALUE) {
        /* Try to communicate error message via thread-specific memory */
        char *message;
        os_size_t messageSize;

        messageSize = sizeof(ERR_MESSAGE) + strlen(key_file_name);
        /* Free any existing thread specific warnings */
        os_threadMemFree(OS_THREAD_WARNING);
        message = (char *)os_threadMemMalloc(OS_THREAD_WARNING, (os_int32)messageSize);
        if (message) {
            snprintf(message, messageSize, ERR_MESSAGE, key_file_name);
        } else {
            /* Allocation failed, use report mechanism */
            OS_REPORT_1(OS_WARNING, "os_findKeyFileByNameAndId", 0, "Could not find any key file: %s", key_file_name);
        }
        return NULL;
    }

    while (!last) {
        os_strcpy(key_file_name, key_file_path);
        os_strcat(key_file_name, "\\");
        os_strcat(key_file_name, fileData.cFileName);
        key_file = fopen(key_file_name, "r");
        if (key_file != NULL)
        {
            if (fgets(domainName, 512, key_file) != NULL)
            {
                /* Remove trailing newline char */
                if (nl = strchr(domainName, '\n'))
                {
                    *nl = 0;
                }
                /* Compare domain name from key file with requested domain name */
                if(os_strncasecmp(name, domainName, (os_uint32)strlen(name)) == 0)
                {
                    kfn = (char*)os_malloc(strlen(key_file_name) + 1);
                    if (kfn != NULL)
                    {
                        os_strcpy(kfn, key_file_name);

                        fgets(line, sizeof(line), key_file);
                        fgets(line, sizeof(line), key_file);
                        fgets(line, sizeof(line), key_file);
                        fgets(line, sizeof(line), key_file);
                        if (fgets(line, sizeof(line), key_file) != NULL)
                        {
                            sscanf(line, "%d", &domainId);
                        }

                        if (id == domainId)
                        {
                            last = 1;
                        }
                        else
                        {
                            kfn = NULL;
                        }
                    }

                }
            }
            fclose(key_file);
        }
        else
        {
            OS_REPORT_1(OS_INFO, "os_findKeyFileByNameAndId", 0, "Failed to open key file: %s", key_file_name);
        }

        if (FindNextFile(fileHandle, &fileData) == 0)
        {
            last = 1;
        }
    }

    FindClose(fileHandle);

    if (kfn == NULL) {
        OS_REPORT_1(OS_INFO, "os_findKeyFile", 0, "Could not find matching key file for domain: %s", name);
    }

    return kfn;
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
    os_char line[512];
    os_int32 pidcount = 0;
    os_int32 pidtemp;
    FILETIME timetmp;
    FILETIME creationTime;
    FILETIME exitTime;
    FILETIME kernelTime;
    FILETIME userTime;
    os_int32 retVal = 0;

    char pidstr[16];
    char *listpidstr;

    /* get creator pid and associated pids from fileName */

    if (fileName != NULL)
    {
        key_file = fopen(fileName, "r");
        if (key_file != NULL)
        {
            fgets(line, sizeof(line), key_file); /* domain name */
            fgets(line, sizeof(line), key_file); /* address */
            fgets(line, sizeof(line), key_file); /* size */
            fgets(line, sizeof(line), key_file); /* implementation */

            while (!feof(key_file))
            {
                timetmp.dwLowDateTime = 0;
                timetmp.dwHighDateTime = 0;

                if (fgets(line, sizeof(line), key_file) != NULL)
                {
                    if (strlen(line) > 8)
                    {
                        sscanf(line, "%d %d %d", &pidtemp, &timetmp.dwLowDateTime, &timetmp.dwHighDateTime);

                    }
                    else
                    {
                        sscanf(line, "%d", &pidtemp);
                    }

                    /* if creation times present in file then verify with os */
                    if (timetmp.dwLowDateTime != 0 || timetmp.dwHighDateTime != 0)
                    {
                        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pidtemp);
                        result = GetProcessTimes(hProcess, &creationTime, &exitTime, &kernelTime, &userTime);
                        if(result)
                        {
                            if(timetmp.dwLowDateTime== creationTime.dwLowDateTime &&
                               timetmp.dwHighDateTime == creationTime.dwHighDateTime)
                            {
                                /* change pid to string to match iterator model */
                                sprintf(pidstr,"%d",pidtemp);
                                listpidstr =  os_strdup(pidstr);
                                os_iterAppend(pidList, listpidstr);

                            }
                        }
                        CloseHandle(hProcess);
                    }
                }

                if (pidcount == 0)
                {
                    fgets(line, sizeof(line), key_file); /* skip domain id */
                }

                pidcount++;
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
os_int32
os_sharedMemoryListDomainNames(
    os_iter nameList)
{
    HANDLE fileHandle;
    WIN32_FIND_DATA fileData;
    char key_file_name[MAX_PATH], domainId[512];
    FILE *key_file;
    char *kfn, *nl;
    os_int32 last = 0;
    os_int32 count = 0;
    os_int32 retVal = 0;
    char *name;

    key_file = NULL;
    kfn = NULL;
    nl = NULL;

    if (key_file_path == NULL) {
        key_file_path = os_getTempDir();
    }

    if (key_file_path == NULL) {
        OS_REPORT(OS_ERROR, "os_findKeyFile", 0, "Failed to determine temporary directory");
        return 1;
    }

    os_strcpy(key_file_name, key_file_path);
    os_strcat(key_file_name, "\\");
    os_strcat(key_file_name, key_file_prefix);
    os_strcat(key_file_name, "*.tmp");

    fileHandle = FindFirstFile(key_file_name, &fileData);

    if (fileHandle == INVALID_HANDLE_VALUE) {
        /* Try to communicate error message via thread-specific memory */
        char *message;
        os_size_t messageSize;

        messageSize = sizeof(ERR_MESSAGE) + strlen(key_file_name);
        /* Free any existing thread specific warnings */
        os_threadMemFree(OS_THREAD_WARNING);
        message = (char *)os_threadMemMalloc(OS_THREAD_WARNING, (os_int32)messageSize);
        if (message) {
            snprintf(message, messageSize, ERR_MESSAGE, key_file_name);
        } else {
            /* Allocation failed, use report mechanism */
            OS_REPORT_1(OS_WARNING, "os_findKeyFile", 0, "Could not find any key file: %s", key_file_name);
        }
        return 1;
    }

    while (!last) {
        os_strcpy(key_file_name, key_file_path);
        os_strcat(key_file_name, "\\");
        os_strcat(key_file_name, fileData.cFileName);
        /* only files of ospXXXX.tmp name format */

        key_file = fopen(key_file_name, "r");
        if (key_file != NULL)
        {
            if (fgets(domainId, 512, key_file) != NULL)
            {
                /* Remove trailing newline char */
                if (nl = strchr(domainId, '\n'))
                {
                    *nl = 0;
                }

                name = os_strdup(domainId);
                os_iterAppend(nameList, name);

            }
            fclose(key_file);
        }
        else
        {
            last = 1;
        }


        if (FindNextFile(fileHandle, &fileData) == 0) {
            last = 1;
        }
    }

    FindClose(fileHandle);

    return retVal;
}

/** \brief frees memory used by iterator created by prior call to
 *  \b listDomainNames creating list in \b nameList
 */
os_int32
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
    return 0;
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
            OS_REPORT_1(OS_ERROR, "os_destroyKeyFile", 0, "Can not Delete the key file %d", (int)GetLastError());
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
                OS_REPORT_1(OS_ERROR, "os_destroyKeyFile", 0, "Can not Delete the DBF file %d", (int)GetLastError());
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
void
os_cleanSharedMemAndOrKeyFiles(
    void)
{
    HANDLE hFile;
    HANDLE hProcess;
    WIN32_FIND_DATA fileData;
    os_char key_file_name [MAX_PATH];
    os_char dbf_file_name [MAX_PATH];
    os_char line[512];
    os_char *ptr;
    os_uint32 last = 0;
    FILE *key_file;
    os_uint32 creatorPid;
    FILETIME creatorTime;
    FILETIME creationTime;
    FILETIME exitTime;
    FILETIME kernelTime;
    FILETIME userTime;
    BOOL doClean = FALSE;
    os_int32 procResult;
    BOOL result;
    os_iter pidList = NULL;
    os_int32 userProc;
    char *pidstr = NULL;

    pidList = os_iterNew(NULL);
    key_file_path = os_getTempDir();

    os_strcpy(key_file_name, key_file_path);
    os_strcat(key_file_name, "\\");
    os_strcat(key_file_name, key_file_prefix);
    os_strcat(key_file_name, "*.tmp");

    hFile = FindFirstFile(key_file_name, &fileData);

    if (hFile != INVALID_HANDLE_VALUE)
    {
        os_strcpy(key_file_name, key_file_path);
        os_strcat(key_file_name, "\\");
        os_strcat(key_file_name, fileData.cFileName);
        key_file = fopen(key_file_name, "r");

        while (!last)
        {
            doClean = FALSE;
            if (key_file != NULL)
            {
                if (fgets(line, sizeof(line), key_file) != NULL) /* domain */
                {
                    fgets(line, sizeof(line), key_file);    /* address */
                    fgets(line, sizeof(line), key_file);    /* size */
                    fgets(line, sizeof(line), key_file);    /* implementation */

                    creatorTime.dwLowDateTime = 0;
                    creatorTime.dwHighDateTime = 0;

                    if (fgets(line, sizeof(line), key_file) != NULL)     /* creator pid */
                    {
                        if (strlen(line) > 8)
                        {
                            sscanf(line, "%d %d %d", &creatorPid, &creatorTime.dwLowDateTime, &creatorTime.dwHighDateTime);
                        }
                        else
                        {
                            sscanf(line, "%d", &creatorPid);
                        }
                        fclose(key_file);

                        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, creatorPid);
                        if (hProcess == NULL)
                        {
                            doClean = TRUE;
                        }
                        else
                        {
                            /* Only perform clean up if we opened a process that matches the spliced process we  expected (i.e.
                             * if the creation time also matches) AND THE SPLICED PROCESS IS NO LONGER RUNNING
                             */
                            if (os_procCheckStatus((os_procId)hProcess, &procResult) != os_resultBusy)
                            {
                                creationTime.dwLowDateTime = 0;
                                creationTime.dwHighDateTime = 0;
                                result = TRUE;
                                if (creatorTime.dwLowDateTime != 0 || creatorTime.dwHighDateTime != 0)
                                {
                                    result = GetProcessTimes(hProcess, &creationTime, &exitTime, &kernelTime, &userTime);
                                }
                                if(result)
                                {
                                    if(creatorTime.dwLowDateTime== creationTime.dwLowDateTime &&
                                       creatorTime.dwHighDateTime == creationTime.dwHighDateTime)
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
                    }
                }
                else
                {
                    fclose(key_file);
                }
            }

            if (FindNextFile(hFile, &fileData) == 0)
            {
                 last = 1;
            }
            else
            {
                os_strcpy(key_file_name, key_file_path);
                os_strcat(key_file_name, "\\");
                os_strcat(key_file_name, fileData.cFileName);
                key_file = fopen(key_file_name, "r");
            }
        }   /* end while */
    }
    FindClose(hFile);

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
    char *key_file_name;
    void *map_address = NULL;
    FILE *key_file;
    char line[512];

    key_file_name = os_findKeyFile(name);
    if (key_file_name != NULL) {
        key_file = fopen(key_file_name, "r");
        if (key_file != NULL) {
            fgets(line, sizeof(line), key_file);
            fgets(line, sizeof(line), key_file);
            sscanf(line, PA_ADDRFMT, (PA_ADDRCAST *)&map_address);
            fclose(key_file);
        }
        os_free(key_file_name);
    }
    return map_address;
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
            OS_REPORT_4(OS_ERROR, "OS Abstraction", 0,
                        "GetTempFileName failed for (%s, %s, 0, %s): System Error Code %d",
                        key_file_path, key_file_prefix, key_file_name,(int)GetLastError());
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
            OS_REPORT_1(OS_ERROR, "OS Abstraction", 0, "CreateFile failed, System Error Code: %d", (int)GetLastError());
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
            OS_REPORT_1(OS_ERROR, "os_getShmFile", 0,
                    "WriteFile failed, System Error Code:  %d", (int)GetLastError());
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
        OS_REPORT_1(OS_ERROR, "OS Abstraction", 0, "Can not Delete the key file %d", (int)GetLastError());
        os_free(key_file_name);
        return -1;
    }
    os_free(key_file_name);
    return 0;
}

static os_result
sharedMemoryGetNameFromId(
    int id,
    char **name)
{
    os_result rv;
    os_int32 r;
    r = os_findNameById(id,name);
    if (r) {
        rv = os_resultSuccess;
    } else {
        rv = os_resultFail;
    }

    return rv;
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
        OS_REPORT_1(OS_ERROR, "OS Abstraction", 0,
            "Failed to create key name for SHM file %s", shm_file_name);
        os_free(shm_file_name);
        return os_resultFail;
    }

    dd = CreateFile(shm_file_name,
                    GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (dd == INVALID_HANDLE_VALUE) {
        OS_REPORT_2(OS_ERROR, "OS Abstraction", 0,
                "Can not Create the database file (%s), System Error Code: %d",
                shm_file_name, (int)GetLastError());
        os_free(shm_file_name);
        return os_resultFail;
    }
    os_free(shm_file_name);

    if (SetFilePointer(dd, (LONG)size, 0, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
        OS_REPORT_1(OS_ERROR, "OS Abstraction", 0,
                "Can not Set File Pointer %d", (int)GetLastError());
        CloseHandle((HANDLE)dd);
        return os_resultFail;
    }
    if (SetEndOfFile(dd) == 0) {
        OS_REPORT_1(OS_ERROR, "OS Abstraction", 0,
                "Not enought diskspace %d", (int)GetLastError());
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
            OS_REPORT_2(OS_ERROR, "OS Abstraction", 0,
                    "Can not Create mapping object (%s) %d",
                    map_object_name, (int)GetLastError());
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
            OS_REPORT_1(OS_ERROR, "OS Abstraction", 0,
                "Failed to create key name for SHM file %s", shm_file_name);
            os_free(shm_file_name);
            return os_resultFail;
        }

        dd = CreateFile(shm_file_name,
                        GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (dd == INVALID_HANDLE_VALUE) {
            OS_REPORT_1(OS_ERROR, "OS Abstraction", 0,
                    "Can not Open the database file %d", (int)GetLastError());
            os_free(shm_file_name);
            return os_resultFail;
        }
        os_free(shm_file_name);
        sharedHandle->dataFile = dd;
        size = SetFilePointer(dd, 0, 0, FILE_END);
        if (size == INVALID_SET_FILE_POINTER) {
            OS_REPORT_1(OS_ERROR, "OS Abstraction", 0,
                    "Can not determine database size %d", (int)GetLastError());
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
                OS_REPORT_2(OS_ERROR, "OS Abstraction", 0,
                        "Can not Open mapping object (%s) %d",
                        map_object_name, (int)GetLastError());
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
        OS_REPORT(OS_ERROR, "OS Abstraction", 0,
            "Failed to get map_address from key file");
        return os_resultFail;
    }
    address = MapViewOfFileEx(sharedHandle->mapObject,
                              FILE_MAP_ALL_ACCESS, 0, 0,
                              sharedHandle->size, request_address);

    if (address == NULL) {
        OS_REPORT_1(OS_ERROR, "OS Abstraction", 0,
                "Can not Map View Of file %d", (int)GetLastError());
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
        OS_REPORT_1(OS_ERROR, "OS Abstraction", 0,
                    "Can not Unmap View Of file %d", (int)GetLastError());
        return os_resultFail;
    }
    sharedHandle->mapped_address = NULL;

    if (!sharedHandle->shm_created) {
        if (CloseHandle(sharedHandle->mapObject) == 0) {
            OS_REPORT_1(OS_ERROR, "OS Abstraction", 0,
                    "Can not close handle of map: System Error Code %d",
                    (int)GetLastError());
            return os_resultFail;
        }
        sharedHandle->mapObject = 0;

        if (CloseHandle(sharedHandle->dataFile) == 0) {
            OS_REPORT_1(OS_ERROR, "OS Abstraction", 0,
                    "Can not close data file %d", (int)GetLastError());
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
        OS_REPORT_1(OS_ERROR, "OS Abstraction", 0,
                "Can not close handle of map %d", (int)GetLastError());
        return os_resultFail;
    }
    sharedHandle->mapObject = 0;
    if (CloseHandle(sharedHandle->dataFile) == 0) {
        OS_REPORT_1(OS_ERROR, "OS Abstraction", 0,
                "Can not close data file %d", (int)GetLastError());
        return os_resultFail;
    }
    sharedHandle->dataFile = 0;
    shm_file_name = os_getShmFile(sharedHandle, 0, 0);
    if (DeleteFile(shm_file_name) == 0) {
        OS_REPORT_3(OS_ERROR, "OS Abstraction", 0,
                "Can not delete data file %s for domain %s (Error: %d)",
                shm_file_name, sharedHandle->name, (int)GetLastError());
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

            os_timeModuleReinit(sharedHandle->name);
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
os_sharedAttrInit(
    os_sharedAttr *sharedAttr)
{
    assert(sharedAttr != NULL);
    sharedAttr->lockPolicy = OS_LOCK_DEFAULT;
    sharedAttr->sharedImpl = OS_MAP_ON_FILE;
    sharedAttr->userCred.uid = 0;
    sharedAttr->userCred.gid = 0;
    sharedAttr->map_address = (void *)0x40000000;
    return os_resultSuccess;
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

void
os_sharedMemoryRegisterUserProcess(
    os_char* domainName,
    os_procId pid)
{
    char* keyFileName;
    HANDLE fileHandle;
    char buffer[512];
    DWORD written;
    BOOL writeResult;
    DWORD winpid;
    FILETIME creationTime;
    FILETIME exitTime;
    FILETIME kernelTime;
    FILETIME userTime;
    HANDLE hProcess;
    BOOL result;

    keyFileName = os_findKeyFile(domainName);
    if (keyFileName == NULL)
    {
        OS_REPORT_1(OS_ERROR, "os_sharedMemoryRegisterUserProcess", 0, "Unable to register process with PID '%d' as a user of shared "
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
            OS_REPORT_2(OS_ERROR, "os_sharedMemoryRegisterUserProcess", 0,
                    "CreateFile failed while trying to append to key file %s, "
                    "System Error Code:  %d", keyFileName, (int)GetLastError());
        } else
        {
            /* Fill the buffer with the pid info to be written */
            winpid = (DWORD) os_procIdToInteger(pid);
            hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, winpid);
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
            snprintf(buffer, sizeof(buffer), "%d %d %d\n", winpid, creationTime.dwLowDateTime, creationTime.dwHighDateTime);
            /* Now write the buffer to the key file, appending it at the end
             * as we opened the file with the FILE_APPEND_DATA flag
             */
            writeResult = WriteFile(fileHandle, buffer, (DWORD)strlen(buffer), &written, NULL);
            if(writeResult == 0)
            {
                OS_REPORT_1(OS_ERROR, "os_sharedMemoryRegisterUserProcess", 0,
                    "WriteFile failed, System Error Code:  %d", (int)GetLastError());
            }
            CloseHandle(fileHandle);
        }
    }
}
