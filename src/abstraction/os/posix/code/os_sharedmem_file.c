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

/** \file os/posix/code/os_sharedmem_file.c
 *  \brief Posix shared memory management
 *
 * Implements shared memory management for POSIX.
 * This implementation maps shared memory on a file.
 */

#include "os_heap.h"
#include "os_report.h"
#include "os_abstract.h"
#include "os_stdlib.h"

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>

/** Defines the permissions for the created shared memory file */
#define OS_PERMISSION \
        (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)

/** Defines the file format for the database file */
static const char os_posix_key_file_format[] = "spddskey_XXXXXX";

static int
os_posix_get_shmumask(void)
{
    mode_t cmask;

    cmask = umask(0); /* This implicitly sets umask to 0000  */
    umask (cmask);     /* Set it back to the original setting */
    return cmask;
}

static int
os_posix_get_kfumask(void)
{
    mode_t cmask;

    cmask = umask(0); /* This implicitly sets umask to 0000  */
    umask(cmask);     /* Set it back to the original setting */
    return cmask;
}

/** \brief Check if the contents of the identified key file
 *         matches the identified name
 *
 * \b os_posix_matchKeyFile tries to compare the contents of the identified
 * key file in \b key_file_name with the identified \b name.
 * On a match 1 will be returned, on a mismatch 0 will be returned.
 */
static int
os_posix_matchKeyFile(
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
 * \b os_posix_findKeyFile tries to find the key file related to \b name
 * in the \b temporary directory. The key files are prefixed with \b
 * /<temporary directory>/spddskey_.
 *
 * \b os_posix_findKeyFile first opens the directory \b temporary directory by
 * calling \b opendir. Then it reads all entries in serach for  any entry
 * that starts with the name \b spddskey_ by reading the entry with
 * \b readdir. If the a matching entry is found, it calls os_posix_matchKeyFile
 * to check if the key file matches the identified \b name. If the
 * \b name matches the contents, the entry is found, and the path
 * is returned to the caller. The memory for the path is allocated
 * from heap and is expected to be freed by the caller.
 *
 * If no matching entry is found, NULL is returned to the caller.
 */
static char *
os_posix_findKeyFile(
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
                key_file_name_size = strlen(dir_name) + strlen(os_posix_key_file_format) + 2;
                key_file_name = os_malloc (key_file_name_size);
                snprintf(key_file_name,
                         key_file_name_size,
                         "%s/%s",
                         dir_name,
                         entry->d_name);
                if (os_posix_matchKeyFile(key_file_name, name)) {
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
    int size)
{
    int key_file_fd;
    int cmask;
    char * dir_name = NULL;
    char *key_file_name;
    unsigned int name_len;
    char *db_file_name;
    char buffer[50];
    int invalid_access;
    int index;

    key_file_name = os_posix_findKeyFile(name);
    if ((map_address != NULL) && (key_file_name == NULL)) {
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
                int pmask = os_posix_get_kfumask();
                OS_REPORT_7(OS_INFO,
                            "os_posix_getShmObjName", 1,
                            "The user file-creation mask (0%o%o%o) set for the "
                            "service\n              specifies exclusive read "
                            "or write access for at least\n              "
                            "one of the access catagories.\n              "
                            "Read and write access should always be paired,\n"
                            "              both prohibit or granted for each "
                            "access catagory.\n              Therefore the "
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
            snprintf(buffer, sizeof (buffer), "%x\n", (unsigned int)size);
            write(key_file_fd, buffer, strlen(buffer));
            snprintf(buffer, sizeof (buffer), "POSIX-SMO\n");
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
        db_file_name = os_malloc(strlen(key_file_name));
        if (db_file_name != NULL) {
           /* This function must return the populated string of the form
            * "/spddskey_XXXXXX" (for the sake of the shm_open system call).
            * So calculate the index of where this string begins in the
            * key_file_name so only that part of it can be returned.
            * The - 1 is for the intermediate '/' added above
            */
            index = strlen(key_file_name) - 1 - strlen(os_posix_key_file_format);
            os_strcpy(db_file_name, &key_file_name[index]);
        }
        os_free(key_file_name);
    } else {
        db_file_name = NULL;
    }
    return db_file_name;
}

/** \brief Get a file map address by name
 *
 * \b os_posix_getMapAddress returns the map address of the named shared memory object.
 */
static void *
os_posix_getMapAddress(
    const char *name)
{
    char *key_file_name;
    void *map_address = NULL;
    FILE *key_file;
    char line[512];

    key_file_name = os_posix_findKeyFile(name);
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

/** \brief Get a file map address by name
 *
 * \b os_posix_getSize returns the size of the named shared memory object.
 */
static os_uint32
os_posix_getSize(
    const char *name)
{
    char *key_file_name;
    os_uint size = 0;
    FILE *key_file;
    char line[512];

    key_file_name = os_posix_findKeyFile(name);
    if (key_file_name != NULL) {
	key_file = fopen(key_file_name, "r");
	if (key_file != NULL) {
	    fgets(line, sizeof(line), key_file);
	    fgets(line, sizeof(line), key_file);
	    fgets(line, sizeof(line), key_file);
	    sscanf(line, "%x", (os_uint32 *)&size);
	    fclose(key_file);
	}
        os_free(key_file_name);
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
os_posix_destroyKeyFile(
    const char *name)
{
    char *key_file_name;
    int rv;

    key_file_name = os_posix_findKeyFile(name);
    if (key_file_name ==  NULL) {
        rv = -1;
    } else if (unlink(key_file_name) == -1) {
	OS_REPORT_2(OS_WARNING,
                    "os_posix_destroyKeyFile", 1,
                    "unlink failed with error %d (%s)",
                    errno, name);
        os_free(key_file_name);
        rv = -1;
    } else {
        os_free(key_file_name);
        rv = 0;
    }
    return rv;
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
os_result
os_posix_sharedMemoryCreate(
    const char *name,
    os_sharedAttr *sharedAttr,
    os_address size)
{
    char *shmname;
    int shmfd;
    int cmask;
    int invalid_access;
    os_result rv = os_resultSuccess;

    assert(name != NULL);
    assert(sharedAttr != NULL);
    /* roundup to page boundaries */
    if ((size % getpagesize()) != 0) {
        size += getpagesize() - (size % getpagesize());
    }
    shmname = os_posix_getShmObjName(name, sharedAttr->map_address, size);
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
            int pmask = os_posix_get_shmumask();
            OS_REPORT_7(OS_INFO,
                        "os_posix_sharedMemoryCreate", 1,
                        "The shared-memory-creation mask (0%o%o%o) set for the "
                        "service \n              specifies exclusive read or write "
                        "access for at least one of the\n              "
                        "access catagories.\n              Read and write "
                        "access should always be paired,\n              both "
                        "prohibit or granted for each access catagory.\n"
                        "              Therefore the service has set the "
                        "user access permissions\n              for the "
                        "shared memory segment associated to this domain "
                        "to (0%o%o%o).\nDomain      : \"%s\"",
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
	    OS_REPORT_2(OS_WARNING,
                        "os_posix_sharedMemoryCreate", 1,
                        "shm_open failed with error %d (%s)",
                        errno, name);
	    rv = os_resultFail;
        } else {
            if (ftruncate(shmfd, size) == -1) {
	        OS_REPORT_2(OS_ERROR,
                            "os_posix_sharedMemoryCreate", 1,
                            "ftruncate failed with error %d (%s)",
                            errno, name);
	        close(shmfd);
	        rv = os_resultFail;
            } else {
                if (sharedAttr->userCred.uid != 0 &&
                    sharedAttr->userCred.gid != 0)
                {
	            if (getuid() == 0 || geteuid() == 0) {
	                if (chown(shmname,
                                  sharedAttr->userCred.uid,
                                  sharedAttr->userCred.gid) == -1) {
	        	    OS_REPORT_2(OS_WARNING,
                                        "os_posix_sharedMemoryCreate", 1,
                                        "chown failed with error %d (%s)",
                                        errno, name);
			}
	            } else {
	        	OS_REPORT_1(OS_WARNING,
                                    "os_posix_sharedMemoryCreate", 2,
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
os_result
os_posix_sharedMemoryDestroy(
    const char *name)
{
    char *shmname;
    os_result rv = os_resultSuccess;

    assert (name != NULL);
    shmname = os_posix_getShmObjName(name, NULL, 0);
    if (shmname != NULL) {
        if (shm_unlink(shmname) == -1) {
	    OS_REPORT_2(OS_WARNING,
                        "os_posix_sharedMemoryDestroy", 1,
                        "shm_unlink failed with error %d (%s)",
                        errno, name);
	    rv = os_resultFail;
	}
	if (os_posix_destroyKeyFile(name) == -1) {
	    rv = os_resultFail;
	}
        os_free(shmname);
    }

    return rv;
}

/** \brief Attach to the POSIX shared memory object related to name
 *
 * First \b os_posix_sharedMemoryDestroy finds the shared object
 * identifier related to \b name by calling \b os_svr4_getKey.
 * If the identifier is found, the request map address is
 * determined by calling os_posix_getMapAddress.
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
os_result
os_posix_sharedMemoryAttach(
    const char *name,
    os_sharedAttr *sharedAttr,
    void **mapped_address)
{
    char *shmname;
    void *request_address;
    int shmfd;
    os_address size;
    os_result rv = os_resultSuccess;

    assert(name != NULL);
    assert(sharedAttr != NULL);
    assert(mapped_address != NULL);
    shmname = os_posix_getShmObjName(name, NULL, 0);
    if (shmname != NULL) {
        request_address = os_posix_getMapAddress(name);
	size = os_posix_getSize(name);
	if (request_address != NULL && size > 0) {
            shmfd = shm_open(shmname, O_RDWR, OS_PERMISSION);
            if (shmfd == -1) {
	    	OS_REPORT_2(OS_ERROR,
                            "os_posix_sharedMemoryAttach", 1,
                            "shm_open failed with error %d (%s)",
                            errno, name);
	        os_free(shmname);
		rv = os_resultFail;
	    } else {
	        *mapped_address = mmap(request_address, (size_t)size,
                                       PROT_READ | PROT_WRITE,
                                       MAP_FIXED | MAP_SHARED,
                                       shmfd, 0);
		if (*mapped_address == (void *)-1) {
	    	    OS_REPORT_2(OS_ERROR,
                                "os_posix_sharedMemoryAttach", 1,
                                "mmap failed with error %d (%s)",
                                errno, name);
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
os_result
os_posix_sharedMemoryDetach (
    const char *name,
    void *address)
{
    os_result rv = os_resultSuccess;
    os_address size;

    assert (address != NULL);
    size = os_posix_getSize(name);
    if (munmap(address, (size_t)size) == -1) {
	OS_REPORT_2(OS_WARNING,
                    "os_posix_sharedMemoryDetach", 1,
                    "munmap failed with error %d (%s)",
                    errno, name);
	rv = os_resultFail;
    }
    return rv;
}

os_result
os_posix_sharedSize(
    const char *name,
    os_address *size)
{
    os_uint s;
    os_result rv;

    s = os_posix_getSize(name);
    if (s == 0) {
	OS_REPORT_1(OS_WARNING, "os_posix_sharedSize", 1, "get size of sgement faild: %s", name);
        rv = os_resultFail;
    } else {
        *size = s;
        rv = os_resultSuccess;
    }
    return rv;
}

#undef OS_PERMISSION

