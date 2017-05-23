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

/** \file os/posix/code/os_sharedmem_file.c
 *  \brief Posix shared memory management
 *
 * Implements shared memory management for POSIX.
 * This implementation maps shared memory  on a file.
 */

#ifdef OS_SHAREDMEM_FILE_DISABLE

static os_result
os_posix_sharedMemoryAttach (
    const char *name,
    const os_sharedAttr *sharedAttr,
    void **mapped_address,
    os_int32 id)
{
    OS_UNUSED_ARG (sharedAttr);
    OS_UNUSED_ARG (name);
    OS_UNUSED_ARG (mapped_address);
    return os_resultFail;
}

static os_result os_posix_sharedMemoryDestroy (const char *name)
{
    OS_UNUSED_ARG (name);
    return os_resultFail;
}

static os_result
os_posix_sharedMemoryCreate (
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

static os_result os_posix_sharedMemoryDetach (const char *name, void *address, os_int32 id)
{
    OS_UNUSED_ARG (address);
    OS_UNUSED_ARG (name);
    return os_resultFail;
}

static os_result os_posix_sharedSize (const char *name, os_address *size)
{
    OS_UNUSED_ARG (size);
    OS_UNUSED_ARG (name);
    return os_resultFail;
}

static os_result os_posix_sharedMemoryGetNameFromId (os_int32 id, char **name)
{
    OS_UNUSED_ARG (id);
    OS_UNUSED_ARG (name);
    return os_resultFail;
}

static os_int32 os_posix_listUserProcessesFree(os_iter pidList)
{
    OS_UNUSED_ARG (pidList);
    return 0;
}

static char * os_posix_findKeyFileByIdAndName(const int id, const char *name)
{
    OS_UNUSED_ARG (id);
    OS_UNUSED_ARG (name);
    return NULL;
}

static os_int32 os_posix_listUserProcesses(os_iter pidList, const char * fileName)
{
    OS_UNUSED_ARG (pidList);
    OS_UNUSED_ARG (fileName);
    return 0;
}

static char *os_posix_findKeyFile(const char *name)
{
    OS_UNUSED_ARG (name);
    return NULL;
}

static char *os_posix_findKeyFileById(const int domain_id)
{
    OS_UNUSED_ARG (domain_id);
    return NULL;
}

static os_int32 os_posix_listDomainNames(os_iter nameList)
{
    OS_UNUSED_ARG (nameList);
    return 0;
}

static void os_posix_listDomainNamesFree(os_iter nameList)
{
    OS_UNUSED_ARG (nameList);
}

static os_int32 os_posix_listDomainIds(os_int32 **idList, os_int32  *listSize)
{
    *idList = NULL;
    *listSize = 0;
    return 0;
}

static int os_posix_destroyKeyFile(const char *name)
{
    OS_UNUSED_ARG (name);
    return 0;
}

static int os_posix_destroyKey(const char *name)
{
    OS_UNUSED_ARG (name);
    return 0;
}

static void os_posix_cleanSharedMemAndOrKeyFiles()
{
}

static int os_posix_sharedMemorySegmentFree(const char *fileName)
{
    OS_UNUSED_ARG (fileName);
    return 0;
}

static os_state os_posix_sharedMemoryGetState(const char *keyfile)
{
    OS_UNUSED_ARG(keyfile);
    return OS_STATE_NONE;
}

static os_result os_posix_sharedMemorySetState(const char *keyfile, os_state state)
{
    OS_UNUSED_ARG(keyfile);
    OS_UNUSED_ARG(state);
    return os_resultUnavailable;
}

static os_result os_posix_sharedMemoryLock() {
    return os_resultUnavailable;
}

static void os_posix_sharedMemoryUnlock() {
    return;
}

#else
#include "os_errno.h"
#include "os_heap.h"
#include "os_report.h"
#include "os_abstract.h"
#include "os_stdlib.h"

#include "../common/include/os_keyfile.h"

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>

/** Defines the permissions for the created shared memory file */
#define OS_PERMISSION \
        (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)

/** Defines the file format for the database file */
const char os_posix_key_file_format[] = "spddskey_XXXXXX";

/** Defines the file name for the shared mem creation lock file */
static const char os_posix_key_file_creation_lock[] = "spddscreationLock";




static mode_t
os_posix_get_shmumask(void)
{
    mode_t cmask;

    cmask = umask(0); /* This implicitly sets umask to 0000  */
    umask(cmask);     /* Set it back to the original setting */
    return cmask;
}



static mode_t
os_posix_get_kfumask(void)
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
os_posix_keyFileParser(
    FILE *key_file,
    os_keyfile_data *data)
{
    os_result rv;
    int tmp;

    assert(key_file);
    assert(data);

    /*
     * The posix keyfile contains the following lines:
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
    /* Loop through remaining part of keyfile to find last (== current) state. */
    data->domain_state = OS_STATE_NONE;
    while (os_resultSuccess == os_keyfile_getInt(key_file, &tmp)) {
        data->domain_state = (os_state)tmp;
    }

    return rv;
}






/** \brief Return the file-path of the key file related
 *         to the identified shared memory
 *
 * \b os_posix_findKeyFile tries to find the key file related to the domain name.
 * If the given name matches the key file contents, the proper file is found, and
 * the path is returned to the caller. The memory for the path is allocated
 * from heap and is expected to be freed by the caller.
 *
 * \b If no matching entry is found, NULL is returned to the caller.
 *
 * \b os_posix_findKeyFileById and os_posix_findKeyFileByNameAndId do basically
 * the same.
 */
static char *
os_posix_findKeyFile(
    const char *name)
{
    return os_keyfile_findByName(os_posix_keyFileParser, name);
}

static char *
os_posix_findKeyFileById(
    os_int32 domain_id)
{
    return os_keyfile_findById(os_posix_keyFileParser, domain_id);
}

static char *
os_posix_findKeyFileByIdAndName(
    const os_int32 id,
    const char *name)
{
    return os_keyfile_findByIdAndName(os_posix_keyFileParser, id, name);
}




/** \brief Return list of processes defined in key file \b fileName
 *         as an iterator contained in \b pidList
 *
 * \b linux key file format only supports creator pid entry in key file
 *
 * \b returns 0 on success and 1 if key file not found or unreadable
 */
static os_int32
os_posix_listUserProcesses(
    os_iter pidList,
    const char * fileName)
{
    return os_keyfile_listUserProcesses(os_posix_keyFileParser, pidList, fileName);
}

/** \brief frees memory used by iterator created by prior call to
 *  \b os_posix_listUserProcesses creating list in \b pidList
 */
static os_int32
os_posix_listUserProcessesFree(
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
os_posix_listDomainNames(
    os_iter nameList)
{
    return os_keyfile_listDomainNames(os_posix_keyFileParser, nameList);
}

/** \brief frees memory used by iterator created by prior call to
 *  \b os_posix_listDomainNames creating list in \b nameList
 */
static void
os_posix_listDomainNamesFree(
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
os_posix_listDomainIds(
    os_int32 **idList,
    os_int32  *listSize)
{
    return os_keyfile_listDomainIds(os_posix_keyFileParser, idList, listSize);
}


/** \brief frees shared memory and/or key files if any key files found
 *  and creator process not running
 */

static os_result
os_posix_cleanSharedMemAndOrKeyFilesAction(
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
os_posix_cleanSharedMemAndOrKeyFiles(
    void)
{
    uid_t uid = getuid(); /* get current user id */
    (void)os_keyfile_loopAllFiles(
            os_posix_keyFileParser,
            os_posix_cleanSharedMemAndOrKeyFilesAction,
            (void*)&uid);
}




/** \brief Get a POSIX shared object identifier for a shared
 *         memory segment by name
 *
 * \b os_posix_getShmObjName tries to find a POSIX shared object
 * name for a named shared memory segment by calling \b os_posix_findKeyFile.
 *
 * If the shared object identifier is not found, one is created by
 * calling \b mkstemp. A related key file is produced containing
 * the name, the map address and the size. The name of the shared object
 * identifier is returned.
 *
 * If the shared object identifier is found, it's name is returned.
 *
 * The name of the shared object is equal to the path of the related
 * key file except for the leading temporary directory path
 */

static char *
os_posix_getShmObjName(
    const char *name,
    void *map_address,
    os_address size,
    const os_int32 id)
{
    int key_file_fd;
    mode_t cmask;
    const char * dir_name = NULL;
    char *key_file_name;
    os_size_t name_len;
    char *db_file_name;
    char buffer[50];
    int invalid_access;

    key_file_name = os_posix_findKeyFileByIdAndName(id,name);

    if (key_file_name == NULL) {
        if ((map_address != NULL)) {
            dir_name = os_getTempDir();
            name_len = strlen(dir_name) + strlen(os_posix_key_file_format) + 2;
            key_file_name = os_malloc(name_len);
            if (key_file_name != NULL) {
                snprintf(key_file_name, name_len, "%s/%s", dir_name, os_posix_key_file_format);
                key_file_fd = mkstemp(key_file_name);
                invalid_access = 0;
                cmask = os_posix_get_kfumask();
                if ((cmask & (S_IRUSR | S_IWUSR)) &&
                    ((cmask & (S_IRUSR | S_IWUSR)) != (S_IRUSR | S_IWUSR))) {
                    cmask |= (S_IRUSR | S_IWUSR);
                    invalid_access = 1;
                }
                if ((cmask & (S_IRGRP | S_IWGRP)) &&
                    ((cmask & (S_IRGRP | S_IWGRP)) != (S_IRGRP | S_IWGRP))) {
                    cmask |= (S_IRGRP | S_IWGRP);
                    invalid_access = 1;
                }
                if ((cmask & (S_IROTH | S_IWOTH)) &&
                    ((cmask & (S_IROTH | S_IWOTH)) != (S_IROTH | S_IWOTH))) {
                    cmask |= (S_IROTH | S_IWOTH);
                    invalid_access = 1;
                }
                if (invalid_access) {
                    mode_t pmask = os_posix_get_kfumask();
                    OS_REPORT_WID(OS_INFO,
                              "os_posix_getShmObjName", 1, id,
                              "The user file-creation mask (0%o%o%o) set for the "
                              "service\n              specifies exclusive read "
                              "or write access for at least\n              "
                              "one of the access categories.\n              "
                              "Read and write access should always be paired,\n"
                              "              both prohibit or granted for each "
                              "access category.\n              Therefore the "
                              "service has set the user access permissions\n"
                              "              for the key file associated to "
                              "this domain to (0%o%o%o).\nDomain      : \"%s\"",
                               (pmask & (S_IWUSR | S_IRUSR)) >> 6,
                               (pmask & (S_IWGRP | S_IRGRP)) >> 3,
                                pmask & (S_IWOTH | S_IROTH),
                               (cmask & (S_IWUSR | S_IRUSR)) >> 6,
                               (cmask & (S_IWGRP | S_IRGRP)) >> 3,
                                cmask & (S_IWOTH | S_IROTH),
                                name);
                }
                fchmod(key_file_fd, OS_PERMISSION & (~cmask));
                write(key_file_fd, name, strlen(name) + 1);
                write(key_file_fd, "\n", 1);
                snprintf(buffer, sizeof (buffer), PA_ADDRFMT"\n", (PA_ADDRCAST)map_address);
                write(key_file_fd, buffer, strlen(buffer));
                snprintf(buffer, sizeof (buffer), "%"PA_PRIxADDR"\n", size);
                write(key_file_fd, buffer, strlen(buffer));
                snprintf(buffer, sizeof (buffer), "POSIX-SMO\n");
                write(key_file_fd, buffer, strlen(buffer));
                snprintf(buffer, sizeof (buffer), "%d\n", (int)getpid());
                write(key_file_fd, buffer, strlen(buffer));
                snprintf(buffer, sizeof (buffer), "%d\n", id);
                write(key_file_fd, buffer, strlen(buffer));
                setpgrp(); /* Make this process the session leader. */
                snprintf(buffer, sizeof (buffer), "%d\n", (int)getpgrp());
                write(key_file_fd, buffer, strlen(buffer));
                close(key_file_fd);
            }
        }
    }
    if (key_file_name != NULL) {
        db_file_name = os_malloc(strlen(key_file_name));
        if (db_file_name != NULL) {
           /* This function must return the populated string of the form
            * "/spddskey_XXXXXX" (for the sake of the shm_open system call).
            * So calculate the index of where this string begins in the
            * key_file_name so only that part of it can be returned.
            * The - 1 is for the intermediate '/' added above
            */
            os_size_t index = strlen(key_file_name) - 1 - strlen(os_posix_key_file_format);
            os_strcpy(db_file_name, &key_file_name[index]);
        }
        os_free(key_file_name);
    } else {
        db_file_name = NULL;
    }
    return db_file_name;
}

/** \brief Get the size of sharedmem
 *
 * \b os_posix_getSize returns the size of the named shared memory object.
 */
static os_address
os_posix_getSize(
    const char *name)
{
    os_address size;
    if (os_keyfile_getSharedSize(os_posix_keyFileParser, name, &size) != os_resultSuccess) {
        size = 0;
    }
    return size;
}


/** \brief Destroy the key related to the named shared memory object
 *
 * The key file related to name is destroyed.
 * First \b os_posix_destroyKey finds the path of the key
 * file by calling \b os_posix_findKeyFile. If the key file
 * is not found, -1 is returned. If the key file is
 * found, the file is destroyed by calling \b unlink.
 * Depending on the result of \b unlink, 0 or -1
 * is returned after \b key_file_name is freed.
 */

static int
os_posix_destroyKey(
    const char *name)
{
    char *key_file_name;
    int rv;

    key_file_name = os_posix_findKeyFile(name);
    if (key_file_name ==  NULL) {
        rv = -1;
    } else if (unlink(key_file_name) == -1) {
        OS_REPORT(OS_WARNING,
                  "os_destroyKey", 1,
                  "Operation unlink failed with error (%d) = \"%s\"\n"
                  "Domain      : \"%s\"",
                  os_getErrno(), os_strError(os_getErrno()), name);
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
os_posix_destroyKeyFile(
    const char *name)
{
    int rv = 0;

    if (unlink(name) == -1 && os_getErrno() != ENOENT)
    {
        OS_REPORT(OS_WARNING,
                  "os_posix_destroyKeyFile", 1,
                  "Operation unlink failed with error (%d) = \"%s\"\n"
                  "Domain      : \"%s\"",
                  os_getErrno(), os_strError(os_getErrno()), name);
        rv = -1;
    }
    return rv;
}

static int
os_posix_sharedMemorySegmentFree(
    const char *fname)
{
    OS_UNUSED_ARG(fname);
    return 0;
}

/** \brief Create a named shared memory area based upon
 *         POSIX shared memory object
 *
 * \b os_posix_sharedMemoryCreate gets a database file name for \b name
 * by calling \b os_posix_getShmObjName.
 *
 * When the file already exists, an error is returned.
 * Otherwise the file is created with \b shm_open and it's size is set according
 * the required database size by calling \b ftruncate.
 *
 * User credentials are taken into account by setting the correct ownership
 * by calling \b chown.
 */
static os_result
os_posix_sharedMemoryCreate(
    const char *name,
    os_sharedAttr *sharedAttr,
    os_address size,
    const os_int32 id)

{
    const os_address pagesize = (os_address) getpagesize();
    char *shmname;
    int shmfd;
    mode_t cmask;
    int invalid_access;
    os_result rv = os_resultSuccess;

    assert(name != NULL);
    assert(sharedAttr != NULL);
    /* roundup to page boundaries */
    if ((size % pagesize) != 0) {
        size += pagesize - (size % pagesize);
    }
    shmname = os_posix_getShmObjName(name, sharedAttr->map_address, size,id);
    if (shmname != NULL) {
        invalid_access = 0;
        cmask = os_posix_get_shmumask();
        if ((cmask & (S_IRUSR | S_IWUSR)) &&
            ((cmask & (S_IRUSR | S_IWUSR)) != (S_IRUSR | S_IWUSR))) {
            cmask |= (S_IRUSR | S_IWUSR);
            invalid_access = 1;
        }
        if ((cmask & (S_IRGRP | S_IWGRP)) &&
            ((cmask & (S_IRGRP | S_IWGRP)) != (S_IRGRP | S_IWGRP))) {
            cmask |= (S_IRGRP | S_IWGRP);
            invalid_access = 1;
        }
        if ((cmask & (S_IROTH | S_IWOTH)) &&
            ((cmask & (S_IROTH | S_IWOTH)) != (S_IROTH | S_IWOTH))) {
            cmask |= (S_IROTH | S_IWOTH);
            invalid_access = 1;
        }
        if (invalid_access) {
            mode_t pmask = os_posix_get_shmumask();
            OS_REPORT_WID(OS_INFO,
                      "os_posix_sharedMemoryCreate", 1, id,
                      "The shared-memory-creation mask (0%o%o%o) set for the "
                      "service \n              specifies exclusive read or write "
                      "access for at least one of the\n              "
                      "access categories.\n              Read and write "
                      "access should always be paired,\n              both "
                      "prohibit or granted for each access category.\n"
                      "              Therefore the service has set the "
                      "user access permissions\n              for the "
                      "shared memory segment associated to this domain "
                      "to (0%o%o%o).\n              Domain: \"%s\"",
                       (pmask & (S_IWUSR | S_IRUSR)) >> 6,
                       (pmask & (S_IWGRP | S_IRGRP)) >> 3,
                        pmask & (S_IWOTH | S_IROTH),
                       (cmask & (S_IWUSR | S_IRUSR)) >> 6,
                       (cmask & (S_IWGRP | S_IRGRP)) >> 3,
                        cmask & (S_IWOTH | S_IROTH),
                        name);
        }
        shmfd = shm_open(shmname,
                         O_CREAT | O_RDWR | O_EXCL,
                         OS_PERMISSION & (~cmask));
        if (shmfd == -1) {
            OS_REPORT_WID(OS_WARNING,
                      "os_posix_sharedMemoryCreate", 1, id,
                      "shm_open failed with error %d (%s)",
                      os_getErrno(), name);
            rv = os_resultFail;
        } else {
            if (ftruncate(shmfd, (off_t) size) == -1) {
                OS_REPORT_WID(OS_ERROR,
                          "os_posix_sharedMemoryCreate", 1, id,
                          "ftruncate failed with error %d (%s)",
                          os_getErrno(), name);

                rv = os_resultFail;
            } else {
                if (sharedAttr->userCred.uid != 0 &&
                    sharedAttr->userCred.gid != 0) {
                    if (getuid() == 0 || geteuid() == 0) {
                        if (chown(shmname,
                                  sharedAttr->userCred.uid,
                                  sharedAttr->userCred.gid) == -1) {
                            OS_REPORT_WID(OS_WARNING,
                                      "os_posix_sharedMemoryCreate", 1, id,
                                      "chown failed with error %d (%s)",
                                      os_getErrno(), name);
                        }
                    } else {
                        OS_REPORT_WID(OS_WARNING,
                                  "os_posix_sharedMemoryCreate", 2, id,
                                  "Can not change ownership of the shared "
                                  "memory segment because of privilege "
                                  "problems (%s)",
                                  name);
                    }
                }
            }
        }
        close(shmfd);
        os_free(shmname);
    }
    return rv;
}

/** \brief Destroy the POSIX shared memory object related to name
 *
 * First \b os_posix_sharedMemoryDestroy finds the shared object identifier
 * related to \b name by calling \b os_posix_getShmObjName. If the identifier is
 * found, the shared object is destroyed by calling \b shm_unlink.
 * After that the key file related to name is detroyed by calling
 * \b os_posix_destroyKeyFile.
 */
static os_result
os_posix_sharedMemoryDestroy(
    const char *name)
{
    char *shmname;
    os_result rv = os_resultSuccess;

    assert (name != NULL);
    shmname = os_posix_getShmObjName(name, NULL, 0, os_keyfile_getIdFromName(os_posix_keyFileParser, name));
    if (shmname != NULL) {
        if (shm_unlink(shmname) == -1) {
            OS_REPORT(OS_WARNING,
                      "os_posix_sharedMemoryDestroy", 1,
                      "shm_unlink failed with error %d (%s)",
                      os_getErrno(), name);
        rv = os_resultFail;
    }

    if (os_posix_destroyKey(name) == -1) {
        rv = os_resultFail;
    }
        os_free(shmname);
    }

    return rv;
}

/** \brief Attach to the POSIX shared memory object related to name
 *
 * First \b os_posix_sharedMemoryDestroy finds the shared object
 * identifier related to \b name by calling \b os_posix_getKey.
 * If the identifier is found, the request map address is
 * determined by calling os_keyfile_getMapAddress.
 * Via \b shm_open and mmap the shared memory object is mapped into the
 * process local address space. The memory is mapped at the address
 * specified by \b request_address and \b size determined by calling
 * os_posix_getSize.
 *
 * If it fails to map the shared memory at the requested address, the
 * shared memory is unmapped again and \b os_resultFail is returned.
 * The actually mapped address is returned in \b *mapped_address.
 * When applicable the memory is locked to prevent paging.
 * It is expected that the creator of the shared memory also
 * attaches to the shared memory and by that determines the locking.
 */
static os_result
os_posix_sharedMemoryAttach(
    const char *name,
    os_sharedAttr *sharedAttr,
    void **mapped_address,
    os_int32 id)
{
    char *shmname;
    void *request_address;
    int shmfd;
    os_address size;
    os_result rv = os_resultSuccess;

    OS_UNUSED_ARG(sharedAttr);

    assert(name != NULL);
    assert(sharedAttr != NULL);
    assert(mapped_address != NULL);
    shmname = os_posix_getShmObjName(name, NULL, 0, os_keyfile_getIdFromName(os_posix_keyFileParser, name));
    if (shmname != NULL) {
        request_address = os_keyfile_getMapAddress(os_posix_keyFileParser, name);
        size = os_posix_getSize(name);
        if (request_address != NULL && size > 0) {
            shmfd = shm_open(shmname, O_RDWR, OS_PERMISSION);
            if (shmfd == -1) {
                OS_REPORT_WID(OS_ERROR,
                          "os_posix_sharedMemoryAttach", 1, id,
                          "shm_open failed with error %d (%s)",
                          os_getErrno(), name);
            os_free(shmname);
        rv = os_resultFail;
        } else {
            *mapped_address = mmap(request_address, (size_t)size,
                                       PROT_READ | PROT_WRITE,
                                       MAP_FIXED | MAP_SHARED,
                                       shmfd, 0);
        if (*mapped_address == (void *)-1) {
            OS_REPORT_WID(OS_ERROR,
                      "os_posix_sharedMemoryAttach", 1, id,
                      "mmap failed with error %d (%s)",
                      os_getErrno(), name);
            rv = os_resultFail;
        } else if (*mapped_address != request_address) {
            munmap(*mapped_address, size);
            rv = os_resultFail;
        }
        close(shmfd);
        }
    }
        os_free(shmname);
    } else {
    rv = os_resultFail;
    }
    return rv;
}

/** \brief Detach from the POSIX shared memory object related to name
 *
 * \b os_posix_sharedMemoryDetach detaches from the shared memory
 * object by calling \b munmap with the address \b address.
 * First the size is determined via \b os_posix_getSize.
 * The \b name is ignored for this implementation.
 */
static os_result
os_posix_sharedMemoryDetach (
    const char *name,
    void *address,
    os_int32 id)
{
    os_result rv = os_resultSuccess;
    os_address size;

    assert (address != NULL);
    size = os_posix_getSize(name);
    if (munmap(address, (size_t)size) == -1) {
        OS_REPORT_WID(OS_WARNING,
                  "os_posix_sharedMemoryDetach", 1, id,
                  "munmap failed with error %d (%s)",
                  os_getErrno(), name);
        rv = os_resultFail;
    }
    return rv;
}

/** \brief Get the size of sharedmem
 *
 * \b os_posix_getSize returns the size of the named shared memory segment.
 */
static os_result
os_posix_sharedSize(
    const char *name,
    os_address *size)
{
    return os_keyfile_getSharedSize(os_posix_keyFileParser, name, size);
}

static os_result
os_posix_sharedMemoryGetNameFromId(
    os_int32 id,
    char **name)
{
    return os_keyfile_getNameFromId(os_posix_keyFileParser, id, name);
}

static os_state
os_posix_sharedMemoryGetState(
    const char *keyfile)
{
    return os_keyfile_getState(os_posix_keyFileParser, keyfile);
}

static os_result
os_posix_sharedMemorySetState(
    const char *keyfile,
    os_state state)
{
    return os_keyfile_appendInt(keyfile, (int)state);
}

/* Prevent race condition between creation and attach shared memory segment
 * create an exclusive lock file during creation.
 * Don block to long, a previous process could have created the lock file
 * and didn't clean it up properly due to an exception in creating shared memory segment
 */
static os_result os_posix_sharedMemoryLock() {
    int ifileHandle = -1;
    int iRetriesToDo = 8;
    char *str = NULL;
    const char *dir = NULL;
    os_size_t len;

    dir = os_getTempDir();
    len = strlen(dir) + strlen(os_posix_key_file_creation_lock) + 2;
    str = os_malloc (len);
    if (str != NULL) {
        (void)snprintf (str, len, "%s/%s", dir, os_posix_key_file_creation_lock);
        while (ifileHandle == -1 && iRetriesToDo > 0) {
            ifileHandle = open(str, O_CREAT | O_EXCL, S_IRWXU | S_IRWXG | S_IRWXO);
            if (ifileHandle == -1) {
                os_sleep(OS_DURATION_SECOND/2);
            } else {
                close(ifileHandle);
            }
            iRetriesToDo--;
        }
        os_free(str);
    }
    if (ifileHandle != -1) {
        return os_resultSuccess;
    } else {
        return os_resultFail;
    }
}

static void os_posix_sharedMemoryUnlock() {
    char *str = NULL;
    const char *dir = NULL;
    os_size_t len;

    dir = os_getTempDir();
    len = strlen(dir) + strlen(os_posix_key_file_creation_lock) + 2;
    str = os_malloc (len);
    if (str != NULL) {
        (void)snprintf (str, len, "%s/%s", dir, os_posix_key_file_creation_lock);
        remove(str);
        os_free(str);
    }

}

#undef OS_PERMISSION
#endif   /* OS_SHAREDMEM_FILE_DISABLE */
