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
/** \file os/svr4/code/os_sharedmem_seg.c
 *  \brief Shared memory management - SVR4 shared memory segment
 *
 * Implements shared memory management on basis of SVR4
 * shared memory segments
 */

#include "os_heap.h"
#include "os_abstract.h"
#include "os_stdlib.h"

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>

/** Defines the permissions for the created shared memory segment */
#define OS_PERMISSION \
        (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)

/** Defines the file format for the key file
 *
 * The key file defines on line 1 the identification of the shared memory
 * On line 2 the virtual address of the area is defined.
 * On the third line, the size of the shared memory is stored
 */
static const char os_svr4_key_file_format[] = "spddskey_XXXXXX";

static int os_destroyKey(const char *name);

static int
os_svr4_get_shmumask(void)
{
    mode_t cmask;

    cmask = umask(0); /* This implicitly sets umask to 0000  */
    umask(cmask);     /* Set it back to the original setting */
    return cmask;
}

static int
os_svr4_get_kfumask(void)
{
    mode_t cmask;

    cmask = umask(0); /* This implicitly sets umask to 0000  */
    umask(cmask);     /* Set it back to the original setting */
    return cmask;
}

/** \brief Check if the contents of the identified key file
 *         matches the identified name
 *
 * \b os_svr4_matchKey tries to compare the contents of the identified
 * key file in \b key_file_name with the identified \b name.
 * On a match 1 will be returned, on a mismatch 0 will be returned.
 */
static int
os_svr4_matchKey(
    const char *key_file_name,
    const char *name)
{
    FILE *key_file;
    int rv = 0;
    char uri[512];

    key_file = fopen(key_file_name, "r");
    if (key_file != NULL) {
        if (fgets(uri, sizeof(uri), key_file) != NULL) {
            if (strcmp(name, uri) == 0) {
                rv = 1;
            }
        }
        fclose(key_file);
    }
    return rv;
}

/** \brief Return the file-path of the key file related
 *         to the identified shared memory
 *
 * \b os_svr4_findKeyFile tries to find the key file related to \b name
 * in the \b temporary directory directory. The key files are prefixed with \b
 * /<temporay directory>/spddskey_.
 *
 * \b os_svr4_findKeyFile first opens the directory \b temporary directory by
 * calling \b opendir. Then it reads all entries in serach for  any entry
 * that starts with the name \b spddskey_ by reading the entry with
 * \b readdir. If the a matching entry is found, it calls os_svr4_matchKey
 * to check if the key file matches the identified \b name. If the
 * \b name matches the contents, the entry is found, and the path
 * is returned to the caller. The memory for the path is allocated
 * from heap and is expected to be freed by the caller.
 *
 * If no matching entry is found, NULL is returned to the caller.
 */
static char *
os_svr4_findKeyFile(
    const char *name)
{
    DIR *key_dir;
    struct dirent *entry;
    char *kfn = NULL;
    char * dir_name = NULL;
    char * key_file_name = NULL;
    int key_file_name_size;

    dir_name = os_getTempDir();
    key_dir = opendir(dir_name);
    if (key_dir) {
        entry = readdir(key_dir);
        while (entry != NULL) {
            if (strncmp(entry->d_name, "spddskey_", 9) == 0) {
                key_file_name_size = strlen(dir_name) + strlen(os_svr4_key_file_format) + 2;
                key_file_name = os_malloc (key_file_name_size);
                snprintf(key_file_name,
                         key_file_name_size,
                         "%s/%s",
                         dir_name,
                         entry->d_name);
                if (os_svr4_matchKey(key_file_name, name)) {
                    kfn = os_malloc(strlen(key_file_name) + 1);
                    if (kfn != NULL) {
                        os_strcpy(kfn, key_file_name);
                    }
                    entry = NULL;
                } else {
                    entry = readdir(key_dir);
                }
                os_free (key_file_name);
            } else {
                entry = readdir(key_dir);
            }
        }
        closedir(key_dir);
    }
    return kfn;
}

/** \brief Remove shared memory remnants from a previous session
 *
 * \b os_svr4_removeFromPreviousSession tries to find remnants from a previous
 * session and to destroy them. Remnants to look for are the key file
 * and the shared memory segment.
 * Conditions to remove them are:
 * - The current user must be owner of the key file
 * - The shared memory segment may not have users attached
 *
 * If it is not valid to preceed further actions because
 * remnants could not be removed, 0 is resturned, else 1 is returned.
 */
static int
os_svr4_removeFromPreviousSession(
    const char *name,
    void *map_address,
    os_address size)
{
    char *kfn = NULL;
    int shmid;
    int result = 1;
    struct shmid_ds shmid_ds;
    struct stat filestat;
    key_t key;

    kfn = os_svr4_findKeyFile(name);
    if (kfn) {
        if (stat (kfn, &filestat) != -1) {
            if (filestat.st_uid == getuid()) {
                key = ftok(kfn, 'S');
                if (key != -1) {
                    shmid = shmget(key, 0, 0);
                    if (shmid == -1) {
                        if (errno == ENOENT) {
                            /* No matching shared segment is found */
                            unlink(kfn);
                            if (ftok(kfn, 'S') != -1) {
                                /* key file could not be removed */
                                result = 0;
                            }
                        } else {
                            OS_REPORT_3(OS_ERROR,
                                        "os_svr4_removeFromPreviousSession", 1,
                                        "Operation shmget failed with error (%d) = \"%s\"\n"
                                        "Domain      : \"%s\"",
                                        errno, strerror(errno), name);
                            result = 0;
                        }
                    } else {
                        if (shmctl(shmid, IPC_STAT, &shmid_ds) == -1) {
                            OS_REPORT_3(OS_ERROR,
                                        "os_svr4_removeFromPreviousSession", 1,
                                        "Operation shmctl (IPC_STAT) failed with error (%d) = \"%s\"\n"
                                        "Domain      : \"%s\"",
                                        errno, strerror(errno), name);
                            result = 0;
                        } else if (shmid_ds.shm_nattch) {
                            OS_REPORT_1(OS_WARNING,
                                        "os_svr4_removeFromPreviousSession", 2,
                                        "still users attached to Domain: (%s)",
                                        name);
                            result = 0;
                        } else if (shmctl(shmid, IPC_RMID, NULL) == -1) {
                            OS_REPORT_3(OS_ERROR,
                                        "os_svr4_removeFromPreviousSession", 1,
                                        "Operation shmctl (IPC_RMID) failed with error (%d) = \"%s\"\n"
                                        "Domain      : \"%s\"",
                                        errno, strerror(errno), name);
                            result = 0;
                        } else if (os_destroyKey(name) == -1) {
                            result = 0;
                        }
                    }
                } else {
                    OS_REPORT_3(OS_ERROR,
                                "os_svr4_removeFromPreviousSession", 1,
                                "Operation ftok failed with error (%d) = \"%s\"\n"
                                "Domain      : \"%s\"",
                                errno, strerror(errno), name);
                }
	    } else {
                OS_REPORT_1(OS_WARNING,
                            "os_svr4_removeFromPreviousSession", 3,
                            "Keyfile from previoues session is found but "
                            "owned by a different user with id %d",
                            filestat.st_uid);
                result = 0;
	    }
        } else {
            if (errno != ENOENT) {
                OS_REPORT_3(OS_ERROR,
                            "os_svr4_removeFromPreviousSession", 1,
                            "Operation stat failed with error (%d) = \"%s\"\n"
                            "Domain      : \"%s\"",
                            errno, strerror(errno), name);
                result = 0;
            }
        }
        os_free(kfn);
    }
    return result;
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
    os_address size)
{
    int key_file_fd;
    char *key_file_name;
    unsigned int name_len;
    char buffer[50];
    key_t key;
    int cmask;
    int invalid_access;
    char * dir_name = NULL;

    key_file_name = os_svr4_findKeyFile(name);
    if ((map_address != NULL) && (key_file_name == NULL)) {
        dir_name = os_getTempDir();
        name_len = strlen(dir_name) + strlen(os_svr4_key_file_format) + 2;
        key_file_name = os_malloc(name_len);
        if (key_file_name != NULL) {
            snprintf(key_file_name, name_len, "%s/%s", dir_name, os_svr4_key_file_format);
            key_file_fd = mkstemp(key_file_name);
            invalid_access = 0;
            cmask = os_svr4_get_kfumask();
            if ((cmask & (S_IWUSR | S_IRUSR)) &&
                ((cmask & (S_IWUSR | S_IRUSR)) != ((S_IWUSR | S_IRUSR)))) {
                cmask |= (S_IWUSR | S_IRUSR);
                invalid_access = 1;
            }
            if ((cmask & (S_IWGRP | S_IRGRP)) &&
                ((cmask & (S_IWGRP | S_IRGRP)) != ((S_IWGRP | S_IRGRP)))) {
                cmask |= (S_IWGRP | S_IRGRP);
                invalid_access = 1;
            }
            if ((cmask & (S_IWOTH | S_IROTH)) &&
                ((cmask & (S_IWOTH | S_IROTH)) != ((S_IWOTH | S_IROTH)))) {
                cmask |= (S_IWOTH | S_IROTH);
                invalid_access = 1;
            }
            if (invalid_access) {
                int pmask = os_svr4_get_kfumask();
                OS_REPORT_7(OS_INFO,
                            "os_svr4_getKey", 1,
                            "The user file-creation mask (0%o%o%o) set for the service specifies"
                            OS_REPORT_NL "exclusive read or write access for at least one of the access catagories."
                            OS_REPORT_NL "Read and write access should always be paired,"
                            OS_REPORT_NL "both prohibit or granted for each access catagory."
                            OS_REPORT_NL "Therefore the service has set the user access permissions"
                            OS_REPORT_NL "for the shared memory segment associated to this domain to (0%o%o%o).\n"
                            "Domain      : \"%s\"",
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
            snprintf(buffer, sizeof (buffer), PA_ADDRFMT"\n",
                     (PA_ADDRCAST)map_address);
            write(key_file_fd, buffer, strlen(buffer));
            snprintf(buffer, sizeof (buffer), PA_ADDRFMT"\n", (PA_ADDRCAST)size);
            write(key_file_fd, buffer, strlen(buffer));
            snprintf(buffer, sizeof (buffer), "SVR4-IPCSHM\n");
            write(key_file_fd, buffer, strlen(buffer));
            snprintf(buffer, sizeof (buffer), "%d\n", (int)getpid());
            write(key_file_fd, buffer, strlen(buffer));
            setpgrp(); /* Make this process the session leader. */
            snprintf(buffer, sizeof (buffer), "%d\n", (int)getpgrp());
            write(key_file_fd, buffer, strlen(buffer));
            close(key_file_fd);
        }
    }
    if (key_file_name != NULL) {
        key = ftok(key_file_name, 'S');
        os_free(key_file_name);
    } else {
        key = -1;
    }
    return key;
}

/** \brief Get a file map address by name
 *
 * \b os_svr4_getMapAddress returns the required map address of the named shared memory segment.
 *
 * Find the key file related to the named shared memory area.
 * When found, read the map address from the 2nd line.
 */
static void *
os_svr4_getMapAddress(
    const char *name)
{
    char *key_file_name;
    void *map_address = NULL;
    FILE *key_file;
    char line[512];

    key_file_name = os_svr4_findKeyFile(name);
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

/** \brief Get the size of sharedmem
 *
 * \b os_svr4_getSize returns the size of the named shared memory segment.
 *
 * Find the key file related to the named shared memory area.
 * When found, read the size from the 3rd line.
 */
static os_address
os_svr4_getSize(
    const char *name)
{
    char *key_file_name;
    os_address size = 0;
    FILE *key_file;
    char line[512];

    key_file_name = os_svr4_findKeyFile(name);
    if (key_file_name != NULL) {
        key_file = fopen(key_file_name, "r");
        if (key_file != NULL) {
            fgets(line, sizeof(line), key_file);
            fgets(line, sizeof(line), key_file);
            fgets(line, sizeof(line), key_file);
            sscanf(line,  PA_ADDRFMT, (PA_ADDRCAST *)&size);
            fclose(key_file);
        }
        os_free(key_file_name);
    }
    return size;
}

/** \brief Destroy the key related to the name
 *
 * The key file related to name is destroyed.
 * First \b os_destroyKey finds the path of the key
 * file by calling \b os_svr4_findKeyFile. If the key file
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
    int rv;

    key_file_name = os_svr4_findKeyFile(name);
    if (key_file_name ==  NULL) {
        rv = -1;
    } else if (unlink(key_file_name) == -1) {
        OS_REPORT_3(OS_WARNING,
                    "os_destroyKey", 1,
                    "Operation unlink failed with error (%d) = \"%s\"\n"
                    "Domain      : \"%s\"",
                    errno, strerror(errno), name);
        os_free(key_file_name);
        rv = -1;
    } else {
        os_free(key_file_name);
        rv = 0;
    }
    return rv;
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
os_result
os_svr4_sharedMemoryCreate(
    const char *name,
    const os_sharedAttr *sharedAttr,
    os_address size)
{
    int shmid;
    int cmask;
    int invalid_access;
    key_t key;
    os_result rv;

    /* roundup to page boundaries */
    if ((size % getpagesize()) != 0) {
        size += getpagesize() - (size % getpagesize());
    }
    key = os_svr4_getKey(name, sharedAttr->map_address, size);
    if (key == -1) {
        rv = os_resultFail;
    } else {
        /* roundup to page boundaries */
        if ((size % getpagesize()) != 0) {
            size += getpagesize() - (size % getpagesize());
	}
        invalid_access = 0;
        cmask = os_svr4_get_shmumask();
        if ((cmask & (S_IWUSR | S_IRUSR)) &&
            ((cmask & (S_IWUSR | S_IRUSR)) != ((S_IWUSR | S_IRUSR)))) {
            cmask |= (S_IWUSR | S_IRUSR);
            invalid_access = 1;
        }
        if ((cmask & (S_IWGRP | S_IRGRP)) &&
            ((cmask & (S_IWGRP | S_IRGRP)) != ((S_IWGRP | S_IRGRP)))) {
            cmask |= (S_IWGRP | S_IRGRP);
            invalid_access = 1;
        }
        if ((cmask & (S_IWOTH | S_IROTH)) &&
            ((cmask & (S_IWOTH | S_IROTH)) != ((S_IWOTH | S_IROTH)))) {
            cmask |= (S_IWOTH | S_IROTH);
            invalid_access = 1;
        }
        if (invalid_access) {
            int pmask = os_svr4_get_shmumask();
            OS_REPORT_7(OS_INFO,
                        "os_svr4_sharedMemoryCreate", 1,
                        "The shared-memory-creation mask (0%o%o%o) set for the service specifies"
                        OS_REPORT_NL "exclusive read or write access for at least one of the access catagories."
                        OS_REPORT_NL "Read and write access should always be paired,"
                        OS_REPORT_NL "both prohibit or granted for each access catagory."
                        OS_REPORT_NL "Therefore the service has set the user access permissions"
                        OS_REPORT_NL "for the shared memory segment associated to this domain to (0%o%o%o).\n"
                        "Domain      : \"%s\"",
                         (pmask & (S_IWUSR | S_IRUSR)) >> 6,
                         (pmask & (S_IWGRP | S_IRGRP)) >> 3,
                          pmask & (S_IWOTH | S_IROTH),
                         (cmask & (S_IWUSR | S_IRUSR)) >> 6,
                         (cmask & (S_IWGRP | S_IRGRP)) >> 3,
                          cmask & (S_IWOTH | S_IROTH),
                          name);

        }
        shmid = shmget (key, size,
                        IPC_CREAT | IPC_EXCL | (OS_PERMISSION & (~cmask)));
        if (shmid == -1) {
            OS_REPORT_4(OS_ERROR,
                        "os_svr4_sharedMemoryCreate", 1,
                        "Operation shmget failed with error (%d) = \"%s\""
                        OS_REPORT_NL "The required SHM size was "PA_SIZEFMT" bytes.\n"
                        "Domain      : \"%s\"",
                        errno, strerror(errno), size, name);
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
os_result
os_svr4_sharedMemoryDestroy(
    const char *name)
{
    key_t key;
    int shmid;
    struct shmid_ds shmid_ds;
    os_result rv;
    int result;

    key = os_svr4_getKey(name, NULL, 0);
    if (key == -1) {
        OS_REPORT_4(OS_ERROR,
                    "os_svr4_sharedMemoryDestroy", 1,
                    "Operation os_svr4_getKey(%s,NULL,0) failed with error (%d) = \"%s\"\n"
                    "Domain name : \"%s\"",
                    key, errno, strerror(errno), name);
        rv = os_resultFail;
    } else {
        shmid = shmget(key, 0, 0);
        if (shmid == -1) {
            OS_REPORT_4(OS_ERROR,
                        "os_svr4_sharedMemoryDestroy", 1,
                        "Operation shmget(%d,0,0) failed with error (%d) = \"%s\"\n"
                        "Domain name : \"%s\"",
                        key, errno, strerror(errno), name);
            rv = os_resultFail;
        } else {
            result = shmctl(shmid, IPC_STAT, &shmid_ds);
            if (result == -1) {
                OS_REPORT_5(OS_ERROR,
                            "os_svr4_sharedMemoryDestroy", 1,
                            "Operation shmctl (%d,IPC_STAT,0x%x) failed with error (%d) = \"%s\"\n"
                            "Domain name : \"%s\"",
                            shmid, &shmid_ds, errno, strerror(errno), name);
                rv = os_resultFail;
            } else if (shmid_ds.shm_nattch) {
                OS_REPORT_2(OS_ERROR,
                            "os_svr4_sharedMemoryDestroy", 3,
                            "Failed to destroy shm for Domain=\"%s\"."
                            OS_REPORT_NL "Reason: still %d users attached.",
                            name, shmid_ds.shm_nattch);
                rv = os_resultFail;
            } else if (shmctl(shmid, IPC_RMID, NULL) == -1) {
                OS_REPORT_4(OS_ERROR,
                            "os_svr4_sharedMemoryDestroy", 1,
                            "Operation shmctl (%d,IPC_RMID,NULL) failed with error (%d) = \"%s\"\n"
                            "Domain name : \"%s\"",
                            shmid, errno, strerror(errno), name);
                rv = os_resultFail;
            } else if (os_destroyKey(name) == -1) {
                OS_REPORT_1(OS_ERROR,
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
 * ID is determined by calling \b shmget. By calling os_svr4_getMapAddress
 * the requested map address is determined.
 * Via \b shmat the segment is mapped in the process local address
 * space. The memory is mapped at the address specified by \b request_address
 * If it fails to map the segment at the requested address, the
 * segment is unmapped again and \b os_resultFail is returned.
 * The actually mapped address is returned in \b *mapped_address.
 */
os_result
os_svr4_sharedMemoryAttach(
    const char *name,
    const os_sharedAttr *sharedAttr,
    void **mapped_address)
{
    key_t key;
    int shmid, success;
    void *map_address;
    void *request_address;
    os_result rv;

    key = os_svr4_getKey(name, NULL, 0);
    if (key == -1) {
	rv = os_resultFail;
    } else {
        request_address = os_svr4_getMapAddress(name);
        shmid = shmget(key, 0, 0);
        if (shmid == -1) {
            OS_REPORT_5(OS_ERROR,
                        "os::svr4::os_svr4_sharedMemoryAttach", 1,
                        "Operation shmget(%d,0,0) failed."
                        OS_REPORT_NL "result = \"%s\" (%d)"
			OS_REPORT_NL "Domain id = \"%s\" (0x%x)",
                        key,strerror(errno),errno, name,request_address);
            rv = os_resultFail;
        }
        else {
            map_address = shmat(shmid, request_address, SHM_RND);
            if (map_address != request_address) {
                rv = os_resultFail;
                if (map_address == (void *)-1) {
                    OS_REPORT_4 (OS_ERROR,
                                 "os_svr4_sharedMemoryAttach", 1,
                                 "Operation shmat failed for %s "
                                 "with errno (%d) = \"%s\""
                                 OS_REPORT_NL "requested address was %p",
                                 name, errno, strerror(errno), request_address);
                    shmdt(map_address);
                } else {
                    OS_REPORT_3(OS_WARNING,
                                "os_svr4_sharedMemoryAttach", 1,
                                "mapped address doesn't match requested"
                                OS_REPORT_NL "Requested address "PA_ADDRFMT" "
                                "is not aligned using "PA_ADDRFMT" instead.\n"
                                "Domain      : \"%s\"",
                                request_address, map_address, name);
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
os_result
os_svr4_sharedMemoryDetach(
    const char *name,
    void *address)
{
    os_result rv;

    if (shmdt(address) == -1) {
        OS_REPORT_3(OS_ERROR,
                    "os_svr4_sharedMemoryDetach", 1,
                    "Operation shmdt failed with error (%d) = \"%s\"\n"
                    "Domain      : \"%s\"",
                    errno, strerror(errno), name);
        rv = os_resultFail;
    } else {
        rv = os_resultSuccess;
    }
    return rv;
}

os_result
os_svr4_sharedSize(
    const char *name,
    os_address *size)
{
    os_address s;
    os_result rv;

    s = os_svr4_getSize(name);
    if (s == 0) {
        rv = os_resultFail;
    } else {
        *size = s;
        rv = os_resultSuccess;
    }
    return rv;
}

#undef OS_PERMISSION

