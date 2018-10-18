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
/** \file os/svr4/code/os_sharedmem_seg.c
 *  \brief Shared memory management - SVR4 shared memory segment
 *
 * Implements shared memory management on basis of SVR4
 * shared memory segments
 */

#ifdef OS_SHAREDMEM_SEG_DISABLE

static os_result
os_svr4_sharedMemoryAttach (
    const char *name,
    const os_sharedAttr *sharedAttr,
    void **mapped_address,
    os_int32 id)
{
    OS_UNUSED_ARG (sharedAttr);
    OS_UNUSED_ARG (name);
    OS_UNUSED_ARG (mapped_address);
    OS_UNUSED_ARG (id);
    return os_resultFail;
}

static os_result os_svr4_sharedMemoryDestroy (const char *name)
{
    OS_UNUSED_ARG (name);
    return os_resultFail;
}

static os_result
os_svr4_sharedMemoryCreate (
    const char *name,
    os_sharedAttr *sharedAttr,
    os_address size,
    const os_int32 id)
{
    OS_UNUSED_ARG (size);
    OS_UNUSED_ARG (sharedAttr);
    OS_UNUSED_ARG (name);
    OS_UNUSED_ARG (id);
    return os_resultFail;
}

static os_result os_svr4_sharedMemoryDetach (const char *name, void *address, os_int32 id)
{
    OS_UNUSED_ARG (address);
    OS_UNUSED_ARG (name);
    OS_UNUSED_ARG (id);
    return os_resultFail;
}

static os_result os_svr4_sharedSize (const char *name, os_address *size)
{
    OS_UNUSED_ARG (size);
    OS_UNUSED_ARG (name);
    return os_resultFail;
}

static os_result os_svr4_sharedMemoryGetNameFromId (os_int32 id, char **name)
{
    OS_UNUSED_ARG (id);
    OS_UNUSED_ARG (name);
    return os_resultFail;
}

static os_int32 os_svr4_listUserProcessesFree(os_iter pidList)
{
    OS_UNUSED_ARG (pidList);
    return 0;
}

static char * os_svr4_findKeyFileByIdAndName(const int id, const char *name)
{
    OS_UNUSED_ARG (id);
    OS_UNUSED_ARG (name);
    return NULL;
}

static char* os_svr4_findKeyFileIdentifierByIdAndName(os_int32 id, const char *name)
{
    OS_UNUSED_ARG (id);
    OS_UNUSED_ARG (name);
    return NULL;
}

static os_int32 os_svr4_listUserProcesses(os_iter pidList, const char * fileName)
{
    OS_UNUSED_ARG (pidList);
    OS_UNUSED_ARG (fileName);
    return 0;
}

static char *os_svr4_findKeyFile(const char *name)
{
    OS_UNUSED_ARG (name);
    return NULL;
}

static char *os_svr4_findKeyFileById(const int domain_id)
{
    OS_UNUSED_ARG (domain_id);
    return NULL;
}

static os_int32 os_svr4_listDomainNames(os_iter nameList)
{
    OS_UNUSED_ARG (nameList);
    return 0;
}

static void os_svr4_listDomainNamesFree(os_iter nameList)
{
    OS_UNUSED_ARG (nameList);
}

static os_int32 os_svr4_listDomainIds(os_int32 **idList, os_int32  *listSize)
{
    *idList = NULL;
    *listSize = 0;
    return 0;
}

static int os_svr4_destroyKeyFile(const char *name)
{
    OS_UNUSED_ARG (name);
    return 0;
}

static int os_svr4_destroyKey(const char *name)
{
    OS_UNUSED_ARG (name);
    return 0;
}

static void os_svr4_cleanSharedMemAndOrKeyFiles()
{
}

static int os_svr4_sharedMemorySegmentFree(const char *fileName)
{
    OS_UNUSED_ARG (fileName);
    return 0;
}

static os_state os_svr4_sharedMemoryGetState(const char *keyfile)
{
    OS_UNUSED_ARG(keyfile);
    return OS_STATE_NONE;
}

static os_result os_svr4_sharedMemorySetState(const char *keyfile, os_state state)
{
    OS_UNUSED_ARG(keyfile);
    OS_UNUSED_ARG(state);
    return os_resultUnavailable;
}

#else

#include "os_errno.h"
#include "os_heap.h"
#include "os_report.h"
#include "os_abstract.h"
#include "os_stdlib.h"

#include "../common/include/os_keyfile.h"

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>

/** Defines the permissions for the created shared memory segment */
#define OS_PERMISSION \
        (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)
#define OS_FILEPERMBITS (S_IRWXU | S_IRWXG | S_IRWXO)

/** Defines the file format for the database file */
const char os_svr4_key_file_format[] = "spddskey_XXXXXX";

static mode_t
os_svr4_get_shmumask(void)
{
    mode_t cmask;

    cmask = umask(0); /* This implicitly sets umask to 0000  */
    umask(cmask);     /* Set it back to the original setting */
    return cmask;
}



static mode_t
os_svr4_get_kfumask(void)
{
    mode_t cmask;

    cmask = umask(0); /* This implicitly sets umask to 0000  */
    umask(cmask);     /* Set it back to the original setting */
    return cmask;
}



/** \brief Parse the keyfile into an os_keyfile_data struct.
 *
 */
static os_result
os_svr4_keyFileParser(
    FILE *key_file,
    os_keyfile_data *data)
{
    os_result rv;
    int tmp;

    assert(key_file);
    assert(data);

    /*
     * The svr4 keyfile contains the following lines:
     *      - Domain name
     *      - Address
     *      - Size
     *      - Implementation id
     *      - Creator pid
     *      - Domain id
     *      - Group pid
     *      - List of domain states (last is current)
     */
    rv = os_keyfile_getString(key_file, data->domain_name, OS_KEYFILE_DOMAIN_NAME_SIZE);
    if (rv == os_resultSuccess) {
        rv = os_keyfile_getAddress(key_file, &(data->address));
    }
    if (rv == os_resultSuccess) {
        rv = os_keyfile_getAddress(key_file, &(data->size));
    }
    if (rv == os_resultSuccess) {
        rv = os_keyfile_getString(key_file, data->implementation_id, OS_KEYFILE_IMPL_ID_SIZE);
    }
    if (rv == os_resultSuccess) {
        rv = os_keyfile_getInt(key_file, &(data->creator_pid));
    }
    if (rv == os_resultSuccess) {
        rv = os_keyfile_getInt(key_file, &(data->domain_id));
    }
    if (rv == os_resultSuccess) {
        rv = os_keyfile_getInt(key_file, &(data->group_pid));
    }
    if (rv == os_resultSuccess) {
        /* Loop through remaining part of keyfile to find last (== current) state. */
        data->domain_state = OS_STATE_NONE;
        while (os_resultSuccess == os_keyfile_getInt(key_file, &tmp)) {
            data->domain_state = (os_state)tmp;
        }
    }

    return rv;
}






/** \brief Return the file-path of the key file related
 *         to the identified shared memory
 *
 * \b os_svr4_findKeyFile tries to find the key file related to the domain name.
 * If the given name matches the key file contents, the proper file is found, and
 * the path is returned to the caller. The memory for the path is allocated
 * from heap and is expected to be freed by the caller.
 *
 * \b If no matching entry is found, NULL is returned to the caller.
 *
 * \b os_svr4_findKeyFileById and os_svr4_findKeyFileByNameAndId do basically
 * the same.
 */
static char *
os_svr4_findKeyFile(
    const char *name)
{
    return os_keyfile_findByName(os_svr4_keyFileParser, name);
}

static char *
os_svr4_findKeyFileById(
    os_int32 domain_id)
{
    return os_keyfile_findById(os_svr4_keyFileParser, domain_id);
}

static char *
os_svr4_findKeyFileByIdAndName(
    const os_int32 id,
    const char *name)
{
    return os_keyfile_findByIdAndName(os_svr4_keyFileParser, id, name);
}

#define MKSTEMPKEYLEN (6)

static char *
os_svr4_findKeyFileIdentifierByIdAndName(
    const os_int32 id,
    const char *name)
{
    char* key_file_name = NULL;
    char* fileIdentifier = NULL;
    size_t len = 0;
    /* mem attach is ignoring the id. The id is found by name. Do here the same to be in sync */
    (void) id;
    key_file_name = os_svr4_findKeyFileByIdAndName(os_keyfile_getIdFromName(os_svr4_keyFileParser, name), name);

    if (key_file_name != NULL) {
        len = strlen(key_file_name);
        if (len > MKSTEMPKEYLEN) {
            fileIdentifier = os_strdup(&key_file_name[len-MKSTEMPKEYLEN]);
        }
        os_free(key_file_name);
        key_file_name = NULL;
    }
    return fileIdentifier;
}





/** \brief Return list of processes defined in key file \b fileName
 *         as an iterator contained in \b pidList
 *
 * \b linux key file format only supports creator pid entry in key file
 *
 * \b returns 0 on success and 1 if key file not found or unreadable
 */
static os_int32
os_svr4_listUserProcesses(
    os_iter pidList,
    const char * fileName)
{
    return os_keyfile_listUserProcesses(os_svr4_keyFileParser, pidList, fileName);
}

/** \brief frees memory used by iterator created by prior call to
 *  \b os_svr4_listUserProcesses creating list in \b pidList
 */
static os_int32
os_svr4_listUserProcessesFree(
    os_iter pidList)
{
    return os_keyfile_listUserProcessesFree(pidList);
}


/** \brief Return list in \b nameList of running opensplice domains defined
 * by presence of associated key files in relevant temporary directory
 *
 * \b linux key file format only supports creator pid entry in key file
 *
 * \b returns 0 on success and 1 if key file not found or unreadable
 */
static os_int32
os_svr4_listDomainNames(
    os_iter nameList)
{
    return os_keyfile_listDomainNames(os_svr4_keyFileParser, nameList);
}

/** \brief frees memory used by iterator created by prior call to
 *  \b os_svr4_listDomainNames creating list in \b nameList
 */
static void
os_svr4_listDomainNamesFree(
    os_iter nameList)
{
    os_keyfile_listDomainNamesFree(nameList);
}


/** \brief Return list in \b idList of running opensplice domains defined
 * by presence of associated key files in relevant temporary directory
 *
 * \b linux key file format only supports creator pid entry in key file
 *
 * \b returns 0 on success and 1 if key file not found or unreadable
 */
static os_int32
os_svr4_listDomainIds(
    os_int32 **idList,
    os_int32  *listSize)
{
    return os_keyfile_listDomainIds(os_svr4_keyFileParser, idList, listSize);
}


/** \brief frees shared memory and/or key files if any key files found
 *  and creator process not running
 */

static os_result
os_svr4_cleanSharedMemAndOrKeyFilesAction(
    const char *key_file_name,
    os_keyfile_data *data,
    void *action_arg)
{
    uid_t *uid = (uid_t*)action_arg;
    struct stat info;

    /* get key file user id */
    if (stat(key_file_name, &info) == 0)
    {
        if (info.st_uid == *uid)
        {
            if (kill(data->creator_pid, 0) == -1)
            {
                /* unable to send signal to the process so it must have terminated */
                /* remove segment then file */
                os_sharedMemorySegmentFree(key_file_name);
                /* delete key file  */
                os_destroyKeyFile(key_file_name);
            }
        }
    }

    return os_resultBusy; /* busy: keep searching */;
}

static void
os_svr4_cleanSharedMemAndOrKeyFiles(
    void)
{
    uid_t uid = getuid(); /* get current user id */
    (void)os_keyfile_loopAllFiles(
            os_svr4_keyFileParser,
            os_svr4_cleanSharedMemAndOrKeyFilesAction,
            (void*)&uid);
}


/** \brief Get a SVR4 IPC key for a shared memory segment by name
 *
 * \b os_svr4_getKey tries to find the SVR4 IPC key for a named
 * shared memory segment by calling \b os_svr4_findKeyFile.
 *
 * If the key file is found, the key is created by calling \b ftok
 * which translates a file path into a key. The key is then returned.
 *
 * If the key file could not be found, -1 is returned to the caller
 * if \b map_address = 0. If \b map_address is != 0 however, it creates a new
 * key file by calling \b mkstemp, which creates and opens a new
 * unique file based upon the provided path. The \b name, the map address
 * and the size is then written in the key file after which it is closed.
 * With the call to \b ftok, the key file is then translated into a key
 * which is returned after the \b key_file_name is freed.
 */
static key_t
os_svr4_getKey(
    const char *name,
    void *map_address,
    os_address size,
    const int id)
{
    int err = 0;
    char *str = NULL;
    const char *dir = NULL;
    FILE *fh = NULL;
    int fd = -1;
    os_size_t len;
    key_t key = -1;
    mode_t maxperm, reqperm;
    static const char func[] = "os_svr4_getKey";

    str = os_svr4_findKeyFileByIdAndName(id, name);
    if ((map_address != NULL) && (str == NULL)) {
        dir = os_getTempDir();
        len = strlen(dir) + strlen(os_svr4_key_file_format) + 2;

        str = os_malloc (len);
        if (str == NULL) {
            err = ENOMEM;
            OS_REPORT (OS_ERROR, func, 0, "malloc: %s", os_strError (err));
        } else {
            (void)snprintf (str, len, "%s/%s", dir, os_svr4_key_file_format);
        }

        if (err == 0) {
            /* The inverse of the umask is the resulting maximum permission on a new file. */
            maxperm = ~os_svr4_get_kfumask() & OS_FILEPERMBITS;
            reqperm = 0;
            /* For each of USR, GRP and OTH check if both are set and remove both if one missing. */
            if ((maxperm & S_IWUSR) && (maxperm & S_IRUSR)) {
                reqperm |= S_IWUSR | S_IRUSR;
            }
            if ((maxperm & S_IWGRP) && (maxperm & S_IRGRP)) {
                reqperm |= S_IWGRP | S_IRGRP;
            }
            if ((maxperm & S_IWOTH) && (maxperm & S_IROTH)) {
                reqperm |= S_IWOTH | S_IROTH;
            }
            if (maxperm != reqperm) {
                OS_REPORT_WID (OS_INFO, func, 0, id,
                    "The user file-creation mask (%04o) set for the service specifies\n"
                    "exclusive read or write access for at least one of the access categories.\n"
                    "Read and write access should always be paired,\n"
                    "both prohibit or granted for each access category.\n"
                    "Therefore the service has set the user access permissions\n"
                    "for the shared memory segment associated to this domain to (%04o).\n"
                    "Domain      : \"%s\"",
                    ~maxperm & OS_FILEPERMBITS,
                    reqperm & OS_PERMISSION,
                    name);
            }

            if ((fd = mkstemp (str)) == -1) {
                err = os_getErrno();
                OS_REPORT (OS_ERROR, func, 0, "mkstemp: %s", os_strError (err));
            } else if (fchmod(fd, reqperm & OS_PERMISSION) != 0) {
                err = os_getErrno();
                OS_REPORT (OS_ERROR, func, 0, "fchmod: %s", os_strError (err));
            } else if ((fh = fdopen (fd, "w+")) == NULL) {
                err = os_getErrno();
                OS_REPORT (OS_ERROR, func, 0, "fdopen: %s", os_strError (err));
            }

            if (err != 0 && fd != -1) {
                (void)close (fd);
            }
        }

        if (err == 0) {
            static const char fmt[] =
                "%s\n" /* domain name */
                PA_ADDRFMT "\n" /* address of mapped memory */
                PA_ADDRFMT "\n" /* size of mapped memory */
                "SVR4-IPCSHM\n"
                "%d\n" /* process identifier */
                "%d\n" /* domain identifier */
                "%d\n" /* process group */;

            (void)setpgrp (); /* make process session leader */

            if (fprintf (
                fh,
                fmt,
                name,
                (PA_ADDRCAST)map_address,
                (PA_ADDRCAST)size,
                (int)getpid(),
                id,
                (int)getpgrp()) < 0)
            {
                err = os_getErrno();
                OS_REPORT (OS_ERROR, func, 0, "fprintf: %s", os_strError (err));
            }
        }
    }

    if (err == 0 && str != NULL) {
        key = ftok (str, 'S');
    }
    if (fh != NULL) {
        fclose (fh);
    }
    if (str != NULL) {
        os_free (str);
    }

    return key;
}

/** \brief Destroy the key related to the named shared memory object
 *
 * The key file related to name is destroyed.
 * First \b os_svr4_destroyKey finds the path of the key
 * file by calling \b os_svr4_findKeyFile. If the key file
 * is not found, -1 is returned. If the key file is
 * found, the file is destroyed by calling \b unlink.
 * Depending on the result of \b unlink, 0 or -1
 * is returned after \b key_file_name is freed.
 */

static int
os_svr4_destroyKey(
    const char *name)
{
    char *key_file_name;
    int rv;

    key_file_name = os_svr4_findKeyFile(name);
    if (key_file_name ==  NULL) {
        rv = -1;
    } else if (unlink(key_file_name) == -1) {
        OS_REPORT(OS_WARNING,
                  "os_destroyKey", 1,
                  "Operation unlink failed with error (%d) = \"%s\"\n"
                  "Domain      : \"%s\"",
                  os_getErrno (), os_strError (os_getErrno ()), name);
        os_free(key_file_name);
        rv = -1;
    } else {
        os_free(key_file_name);
        rv = 0;
    }
    return rv;
}

/** \brief Destroy the key file with filename name
 */

static int
os_svr4_destroyKeyFile(
    const char *name)
{
    int rv = 0;

    if (unlink(name) == -1 && os_getErrno() != ENOENT)
    {
        OS_REPORT(OS_WARNING,
                  "os_svr4_destroyKeyFile", 1,
                  "Operation unlink failed with error (%d) = \"%s\"\n"
                  "Domain      : \"%s\"",
                  os_getErrno (), os_strError (os_getErrno ()), name);
        rv = -1;
    }
    return rv;
}

static int
os_svr4_sharedMemorySegmentFree(
    const char *fname)
{
    key_t key;
    int shmid;
    int retCode = 0;

    key = ftok (fname, 'S');
    if (key != -1)
    {
        shmid = shmget (key, 0, 0);
        if (shmid != -1)
        {
            int val;

            val = shmctl (shmid, IPC_RMID, NULL);
            if(val >= 0)
            {
                retCode = 0;
            }
            else
            {
                retCode = 1;
            }
        }
    }
    return retCode;
}


/** \brief Create a named shared memory segment based upon
 *         SVR4 IPC shared memory segments
 *
 * \b os_svr4_sharedMemoryCreate tries to get a key for \b name
 * by calling \b os_svr4_getKey. By setting the \b create flag to 1
 * it indicates the a key should be created if it is not found.
 *
 * When the key is available, the shared memory segment is created
 * by calling \b shmget.
 *
 */
static os_result
os_svr4_sharedMemoryCreate(
    const char *name,
    const os_sharedAttr *sharedAttr,
    os_address size,
    const int id)
{
    const os_address pagesize = (os_address) getpagesize();
    int shmid;
    mode_t reqperm, maxperm;
    key_t key;
    os_result rv;

    /* roundup to page boundaries */
    if ((size % pagesize) != 0) {
        size += pagesize - (size % pagesize);
    }
    key = os_svr4_getKey(name, sharedAttr->map_address, size, id);
    if (key == -1) {
        rv = os_resultFail;
    } else {
        /* roundup to page boundaries */
        if ((size % pagesize) != 0) {
            size += pagesize - (size % pagesize);
        }
        /* The inverse of the umask is the resulting maximum permission on a new file. */
        maxperm = reqperm = ~os_svr4_get_shmumask() & OS_FILEPERMBITS;
        /* For each of USR, GRP and OTH check if either one is set and set both if so. */
        if (maxperm & (S_IWUSR | S_IRUSR)) {
            reqperm |= (S_IWUSR | S_IRUSR);
        }
        if (maxperm & (S_IWGRP | S_IRGRP)) {
            reqperm |= (S_IWGRP | S_IRGRP);
        }
        if (maxperm & (S_IWOTH | S_IROTH)) {
            reqperm |= (S_IWOTH | S_IROTH);
        }
        if (maxperm != reqperm) {
            OS_REPORT_WID(OS_INFO,
                      "os_svr4_sharedMemoryCreate", 1, id,
                      "The shared-memory-creation mask (%04o) set for the service specifies"
                      OS_REPORT_NL "exclusive read or write access for at least one of the access categories."
                      OS_REPORT_NL "Read and write access should always be paired,"
                      OS_REPORT_NL "both prohibit or granted for each access category."
                      OS_REPORT_NL "Therefore the service has set the user access permissions"
                      OS_REPORT_NL "for the shared memory segment associated to this domain to (%04o).\n"
                      "Domain      : \"%s\"",
                       ~maxperm & OS_FILEPERMBITS,
                       reqperm & OS_PERMISSION,
                       name);
        }
        shmid = shmget (key, size, IPC_CREAT | IPC_EXCL | (int)(reqperm & OS_PERMISSION) );
        if (shmid == -1) {
            OS_REPORT_WID(OS_ERROR,
                      "os_svr4_sharedMemoryCreate", 1, id,
                      "Operation shmget failed with error (%d) = \"%s\""
                      OS_REPORT_NL "The required SHM size was "PA_SIZEFMT" bytes.\n"
                      "Domain      : \"%s\"",
                      os_getErrno (), os_strError(os_getErrno ()), size, name);
            rv = os_resultFail;
        } else {
            rv = os_resultSuccess;
        }
    }
    return rv;
}

/** \brief Destroy the SVR4 IPC shared memory segment related to name
 *
 * First \b os_svr4_sharedMemoryDestroy finds the key related to \b name
 * by calling \b os_svr4_getKey. If the key is
 * found, the shared memory ID is determined by calling \b shmget.
 * Via \b shmctl (\b IPC_STAT) the current status of the segment is
 * determined. If any process is still attached (\b shm_nattch != 0),
 * the shared memory segment is not destroyed and \b os_resultFail
 * is returned. If the condition is OK, the segment is destroyed
 * by calling \b shmctl (\b IPC_RMID), after which the key file
 * is detroyed by calling \b os_destroyKey.
 */
static os_result
os_svr4_sharedMemoryDestroy(
    const char *name)
{
    key_t key;
    int shmid;
    struct shmid_ds shmid_ds;
    os_result rv;
    int result;

    key = os_svr4_getKey(name, NULL, 0, os_keyfile_getIdFromName(os_svr4_keyFileParser, name));
    if (key == -1) {
        OS_REPORT(OS_ERROR,
                  "os_svr4_sharedMemoryDestroy", 1,
                  "Operation os_svr4_getKey(%d,NULL,0) failed with error (%d) = \"%s\"\n"
                  "Domain name : \"%s\"",
                  key, os_getErrno(), os_strError(os_getErrno()), name);
        rv = os_resultFail;
    } else {
        shmid = shmget(key, 0, 0);
        if (shmid == -1) {
            OS_REPORT(OS_ERROR,
                      "os_svr4_sharedMemoryDestroy", 1,
                      "Operation shmget(%d,0,0) failed with error (%d) = \"%s\"\n"
                      "Domain name : \"%s\"",
                      key, os_getErrno(), os_strError(os_getErrno()), name);
            rv = os_resultFail;
        } else {
            result = shmctl(shmid, IPC_STAT, &shmid_ds);
            if (result == -1) {
                OS_REPORT(OS_ERROR,
                          "os_svr4_sharedMemoryDestroy", 1,
                          "Operation shmctl (%d,IPC_STAT,0x%"PA_PRIxADDR") failed with error (%d) = \"%s\"\n"
                          "Domain name : \"%s\"",
                          shmid, (os_address)&shmid_ds, os_getErrno(), os_strError(os_getErrno()), name);
                rv = os_resultFail;
            } else if (shmid_ds.shm_nattch) {
                OS_REPORT(OS_ERROR,
                          "os_svr4_sharedMemoryDestroy", 3,
                          "Failed to destroy shm for Domain=\"%s\"."
                          OS_REPORT_NL "Reason: still %u users attached.",
                          name, (unsigned)shmid_ds.shm_nattch);
                rv = os_resultFail;
            } else if (shmctl(shmid, IPC_RMID, NULL) == -1) {
                OS_REPORT(OS_ERROR,
                          "os_svr4_sharedMemoryDestroy", 1,
                          "Operation shmctl (%d,IPC_RMID,NULL) failed with error (%d) = \"%s\"\n"
                          "Domain name : \"%s\"",
                          shmid, os_getErrno(), os_strError(os_getErrno()), name);
                rv = os_resultFail;
            } else if (os_svr4_destroyKey(name) == -1) {
                OS_REPORT(OS_ERROR,
                          "os_svr4_sharedMemoryDestroy", 3,
                          "Failed to destroy shm key for Domain=\"%s\".",
                          name);
                rv = os_resultFail;
            } else {
                rv = os_resultSuccess;
            }
        }
    }
    return rv;
}



/** \brief Attach to the SVR4 IPC shared memory segment related to name
 *
 * First \b os_svr4_sharedMemoryAttach finds the key related to \b name
 * by calling \b os_svr4_getKey. If the key is found, the shared memory
 * ID is determined by calling \b shmget. By calling os_keyfile_getMapAddress
 * the requested map address is determined.
 * Via \b shmat the segment is mapped in the process local address
 * space. The memory is mapped at the address specified by \b request_address
 * If it fails to map the segment at the requested address, the
 * segment is unmapped again and \b os_resultFail is returned.
 * The actually mapped address is returned in \b *mapped_address.
 */
static os_result
os_svr4_sharedMemoryAttach(
    const char *name,
    const os_sharedAttr *sharedAttr,
    void **mapped_address,
    os_int32 id)
{
    key_t key;
    int shmid;
    void *map_address;
    void *request_address;
    os_result rv;

    OS_UNUSED_ARG(sharedAttr);

    key = os_svr4_getKey(name, NULL, 0, os_keyfile_getIdFromName(os_svr4_keyFileParser, name));

    if (key == -1) {
        rv = os_resultFail;
    } else {
        request_address = os_keyfile_getMapAddress(os_svr4_keyFileParser, name);
        shmid = shmget(key, 0, 0);
        if (shmid == -1) {
            OS_REPORT_WID(OS_ERROR,
                      "os::svr4::os_svr4_sharedMemoryAttach", 1, id,
                      "Operation shmget(%d,0,0) failed."
                      OS_REPORT_NL "result = \"%s\" (%d)"
                      OS_REPORT_NL "Domain id = \"%s\" (0x%"PA_PRIxADDR")",
                      key, os_strError(os_getErrno()),os_getErrno(), name,(os_address)request_address);
            rv = os_resultFail;
        } else {
            map_address = shmat(shmid, request_address, SHM_RND);
            if (map_address != request_address) {
                rv = os_resultFail;
                if (map_address == (void *)-1) {
                    OS_REPORT_WID (OS_ERROR,
                               "os_svr4_sharedMemoryAttach", 1, id,
                               "Operation shmat failed for %s "
                               "with errno (%d) = \"%s\""
                               OS_REPORT_NL "requested address was %p",
                               name, os_getErrno(), os_strError(os_getErrno()), request_address);
                    shmdt(map_address);
                } else {
                    OS_REPORT_WID(OS_WARNING,
                              "os_svr4_sharedMemoryAttach", 1, id,
                              "mapped address doesn't match requested"
                              OS_REPORT_NL "Requested address "PA_ADDRFMT" "
                              "is not aligned using "PA_ADDRFMT" instead.\n"
                              "Domain      : \"%s\"",
                              (os_address)request_address, (os_address)map_address, name);
                    *mapped_address = map_address;
                    rv = os_resultSuccess;
                }
            } else {
                *mapped_address = map_address;
                rv = os_resultSuccess;
            }
        }
    }
    return rv;
}

/** \brief Detach from the SVR4 IPC shared memory segment related to name
 *
 * \b os_svr4_sharedMemoryDetach detaches from the shared memory
 * segment by calling \b shmdt with the address \b address.
 * The \b name is ignored for this implementation.
 */
static os_result
os_svr4_sharedMemoryDetach(
    const char *name,
    void *address,
    os_int32 id)
{
    os_result rv;

    if (shmdt(address) == -1) {
        OS_REPORT_WID(OS_ERROR,
                  "os_svr4_sharedMemoryDetach", 1, id,
                  "Operation shmdt failed with error (%d) = \"%s\"\n"
                  "Domain      : \"%s\"",
                  os_getErrno(), os_strError(os_getErrno()), name);
        rv = os_resultFail;
    } else {
        rv = os_resultSuccess;
    }
    return rv;
}

/** \brief Get the size of sharedmem
 *
 * \b os_svr4_getSize returns the size of the named shared memory segment.
 */
static os_result
os_svr4_sharedSize(
    const char *name,
    os_address *size)
{
    return os_keyfile_getSharedSize(os_svr4_keyFileParser, name, size);
}

static os_result
os_svr4_sharedMemoryGetNameFromId(
    os_int32 id,
    char **name)
{
    return os_keyfile_getNameFromId(os_svr4_keyFileParser, id, name);
}

static os_state
os_svr4_sharedMemoryGetState(
    const char *keyfile)
{
    return os_keyfile_getState(os_svr4_keyFileParser, keyfile);
}

static os_result
os_svr4_sharedMemorySetState(
    const char *keyfile,
    os_state state)
{
    return os_keyfile_appendInt(keyfile, (int)state);
}

#undef OS_PERMISSION

#endif /* OS_SHAREDMEM_SEG_DISABLE */
