/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
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

#include "os_heap.h"
#include "os_thread.h"
#include "os_mutex.h"
#include "os_cond.h"
#include "code/os__debug.h"

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
    HANDLE dataFile;
    HANDLE mapObject;
    os_address size;
    os_int32 shm_created;
};

struct os_shmInfo {
    os_sharedHandle sharedHandle;
    struct os_shmInfo *next;
};

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
    const os_sharedAttr *sharedAttr)
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
            sh->mapped_address = (void *)0;
            sh->dataFile = 0;
            sh->mapObject = 0;
            sh->size = 0;
            sh->shm_created = 0;
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

static char *key_file_path = NULL;
static const char key_file_prefix[] = "osp";

/** \brief Return the file-path of the key file related
 *         to the identified shared memory
 *
 * \b os_findKeyFile tries to find the key file related to \b name
 * in the \b temporary directory. The key files are prefixed with \b
 * /<temporary directory>/sp2v3key_.
 *
 * \b os_findKeyFile first opens the temporary directory by calling
 * \b opendir. Then it reads all entries in serach for  any entry
 * that starts with the name \b sp2v3key_ by reading the entry with
 * \b readdir. If the a matching entry is found, it opens the file
 * and compares the contents with the required \b name. If the
 * \b name matches the contents, the entry is found, and the path
 * is returned to the caller. The memory for the path is allocated
 * from heap and is expected to be freed by the caller.
 *
 * If no matching entry is found, NULL is returned to the caller.
 */
static char *
os_findKeyFile(
    const char *name)
{
    HANDLE fileHandle;
    WIN32_FIND_DATA fileData;
    char key_file_name[MAX_PATH];
    char uri[512];
    FILE *key_file;
    int last = 0;
    char *kfn;
    int len;

    if (key_file_path == NULL) {
        key_file_path = os_getTempDir();
    }

    if (key_file_path == NULL) {
        OS_REPORT(OS_ERROR, "os_findKeyFile", 0, "Failed to determine temporary directory");
    }

    os_strcpy(key_file_name, key_file_path);
    os_strcat(key_file_name, "\\");
    os_strcat(key_file_name, key_file_prefix);
    os_strcat(key_file_name, "*.tmp");

    fileHandle = FindFirstFile(key_file_name, &fileData);

    if (fileHandle == INVALID_HANDLE_VALUE) {
        /* Try to communicate error message via thread-specific memory */
#define ERR_MESSAGE "os_findKeyFile: Could not find any key file: %s"
        char *message;
        unsigned int messageSize;

        messageSize = sizeof(ERR_MESSAGE) + strlen(key_file_name);
        /* Free any existing thread specific warnings */
        os_threadMemFree(OS_THREAD_WARNING);
        message = (char *)os_threadMemMalloc(OS_THREAD_WARNING, messageSize);
        if (message) {
            snprintf(message, messageSize, ERR_MESSAGE, key_file_name);
        } else {
            /* Allocation failed, use report mechanism */
            OS_REPORT_1(OS_WARNING, "os_findKeyFile", 0, "Could not find any key file: %s", key_file_name);
        }
        return NULL;
    }

    os_strcpy(key_file_name, key_file_path);
    os_strcat(key_file_name, "\\");
    os_strcat(key_file_name, fileData.cFileName);
    key_file = fopen(key_file_name, "r");

    while (!last) {
        if (key_file != NULL) {
            if (fgets(uri, sizeof(uri), key_file) != NULL) {
                len = strlen(uri);
                if (len > 0) {
                    uri[len-1] = 0;
                }
                if (strcmp(name, uri) == 0) {
                    fclose(key_file);
                    kfn = (char*)os_malloc(strlen (key_file_name) + 1);
                    if (kfn != NULL) {
                        os_strcpy(kfn, key_file_name);
                    }
                    FindClose(fileHandle);
                    return kfn;
                }
            }
        }

        if (FindNextFile(fileHandle, &fileData) == 0) {
            last = 1;
        } else {
            os_strcpy(key_file_name, key_file_path);
            os_strcat(key_file_name, "\\");
            os_strcat(key_file_name, fileData.cFileName);
            key_file = fopen(key_file_name, "r");
        }
    }
    if (key_file) {
       fclose(key_file);
    }
    FindClose(fileHandle);
    OS_REPORT_1(OS_INFO, "os_findKeyFile", 0, "Could not find matching key file for uri: %s", name);
    return NULL;
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
        snprintf(buf, sizeof(buf), "%s\n"PA_ADDRFMT"\n"PA_ADDRFMT"\nWIN-SHM\n%d\n",
            sharedHandle->name, (PA_ADDRCAST)sharedHandle->attr.map_address, size, pid);

        if (WriteFile((HANDLE)fileHandle, buf, strlen(buf), &written, NULL) == 0) {
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
os_sharedMemoryCreateFile(
    os_sharedHandle sharedHandle,
    os_address size)
{
    char *shm_file_name;
    char *map_object_name;
    HANDLE dd;
    HANDLE md;
    SECURITY_ATTRIBUTES security_attributes;
    BOOL sec_descriptor_ok;

    shm_file_name = os_getShmFile(sharedHandle, size, 1);
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
    if (SetFilePointer(dd, size, 0, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
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
        md = CreateFileMapping(dd, &security_attributes, PAGE_READWRITE, 0, size, map_object_name);
        LocalFree(security_attributes.lpSecurityDescriptor);
    }

    if (md == NULL)
    {
        /* Couldn't create a global mapping - we're probably not admin or local system */
        os_free(map_object_name);
        os_isSharedMemGlobal = 0;
        /* get a local file mapping name instead  and create that */
        map_object_name = os_getMapName(sharedHandle->name, 0);
        md = CreateFileMapping(dd, NULL, PAGE_READWRITE, 0, size, map_object_name);
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
        OS_REPORT_2(OS_ERROR, "OS Abstraction", 0,
                "Can not delete data file (%s): System error code %d",
                shm_file_name, (int)GetLastError());
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
        result = os_resultUnavailable;
    break;
    default:
    break;
    }
    return result;
}

