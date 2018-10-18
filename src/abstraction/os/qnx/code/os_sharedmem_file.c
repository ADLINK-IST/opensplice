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

#include <assert.h>

#include "os_heap.h"
#include "os_report.h"
#include "os_abstract.h"

#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include "os_errno.h"

/** Defines the permissions for the created shared memory file */
#define OS_PERMISSION   (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

/** Defines the file prefix for the database file */
static const char os_posix_key_file_prefix[] = "/tmp/spddskey_XXXXXX";

size_t getpagesize ()
{
    return 0x1000;
}

/** \brief Check if the contents of the identified key file
 *         matches the identified name
 *
 * \b os_posix_matchKeyFile tries to compare the contents of the identified
 * key file in \b key_file_name with the identified \b name.
 * On a match 1 will be returned, on a mismatch 0 will be returned.
 */
static int
os_posix_matchKeyFile (
    const char *key_file_name,
    const char *name)
{
    FILE *key_file;
    int rv = 0;
    char uri[512];

    key_file = fopen (key_file_name, "r");
    if (key_file != NULL) {
        if (fgets (uri, sizeof(uri), key_file) != NULL) {
            if (strcmp (name, uri) == 0) {
                rv = 1;
            }
        }
        fclose (key_file);
    }
    return rv;
}

/** \brief Check if the contents of the identified key file
 *         matches the identified id and and set the name
 *
 * \b os_posix_getNameById tries to compare the contents of the identified
 * key file in \b key_file_name with the identified \b id.
 * On a match 1 will be returned and the domain name will be set,
 * on a mismatch 0 will be returned and name will be NULL.
 */
int
os_posix_getNameById(
    const char *key_file_name,
    const int id,
    char **name)
{
    os_int32 f_id = 0;
    FILE *key_file;
    char line[512];

    if (key_file_name != NULL) {
    key_file = fopen(key_file_name, "r");
        if (key_file != NULL) {
           if (fgets(line, sizeof(line), key_file) != NULL) { /* line 1 name */
               *name =  os_strdup(line);
           }
           fgets(line, sizeof(line), key_file);
           fgets(line, sizeof(line), key_file);
           fgets(line, sizeof(line), key_file);
           fgets(line, sizeof(line), key_file);
           if (fgets(line, sizeof(line), key_file) != NULL) { /* line 6 id */
               sscanf(line, "%d", &f_id);
           }
           fclose(key_file);
        }
    }
    if (id != f_id) {
        os_free(*name);
        *name = NULL;
    }
    return id == f_id;
}

/** \brief Return the file-path of the key file related
 *         to the identified shared memory 
 *
 * \b os_posix_findKeyFile tries to find the key file related to \b name
 * in the \b /tmp directory. The key files are prefixed with \b
 * /tmp/spddskey_. 
 *
 * \b os_posix_findKeyFile first opens the directory \b /tmp by calling
 * \b opendir. Then it reads all entries in serach for  any entry
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
os_posix_findKeyFile (
    const char *name)
{
    DIR *key_dir;
    struct dirent *entry;
    char key_file_name [sizeof(os_posix_key_file_prefix)+1];
    char *kfn = NULL;

    key_dir = opendir ("/tmp");
    if (key_dir) {
        entry = readdir (key_dir);
        while (entry != NULL) {
            if (strncmp (entry->d_name, "spddskey_", 9) == 0) {
                snprintf (key_file_name, sizeof(os_posix_key_file_prefix)+1, "/tmp/%s", entry->d_name);
                if (os_posix_matchKeyFile (key_file_name, name)) {
                    kfn = os_malloc (strlen (key_file_name) + 1);
                    if (kfn != NULL) {
                        os_strcpy (kfn, key_file_name);
                    }
                    entry = NULL;
                } else {
                    entry = readdir (key_dir);
                }
            } else {
                entry = readdir (key_dir);
            }
        }
        closedir (key_dir);
    }
    return kfn;
}

static char *
os_posix_findKeyFileByIdAndName(
    const int id,
    const char *name)
{
    DIR *key_dir;
    struct dirent *entry;
    char key_file_name [sizeof(os_posix_key_file_prefix)+1];
    char *kfn = NULL;

    key_dir = opendir ("/tmp");
    if (key_dir) {
       entry = readdir (key_dir);
       while (entry != NULL) {
           if (strncmp (entry->d_name, "spddskey_", 9) == 0) {
               snprintf (key_file_name, sizeof(os_posix_key_file_prefix)+1, "/tmp/%s", entry->d_name);
               if (os_posix_matchKeyFileById (key_file_name, id, name)) {
                   kfn = os_malloc (strlen (key_file_name) + 1);
                   if (kfn != NULL) {
                       os_strcpy (kfn, key_file_name);
                   }
                   entry = NULL;
               } else {
                   entry = readdir (key_dir);
               }
           } else {
               entry = readdir (key_dir);
           }
       }
       closedir (key_dir);
    }
    return kfn;
}

static int
os_posix_findNameById(
    const int id,
    const char **name)
{
    DIR *key_dir;
    struct dirent *entry;
    char key_file_name [sizeof(os_posix_key_file_prefix)+1];
    int rv = 0;

    key_dir = opendir ("/tmp");
    if (key_dir) {
       entry = readdir (key_dir);
       while (entry != NULL) {
           if (strncmp (entry->d_name, "spddskey_", 9) == 0) {
               snprintf (key_file_name, sizeof(os_posix_key_file_prefix)+1, "/tmp/%s", entry->d_name);
               if (os_posix_getNameById (key_file_name, id, *name)) {
                   rv = 1;
                   entry = NULL;
               } else {
                   entry = readdir (key_dir);
               }
           } else {
               entry = readdir (key_dir);
           }
       }
       closedir (key_dir);
    }
    return rv;
}

/** \brief Get a POSIX shared object identifier for a shared
 *         memory segment by name
 *
 * \b os_posix_getShmObjName tries to find a POSIX shared object
 * name for a named shared memory segment by calling \b os_posix_findKeyFile.
 *
 * If the shared object identifier is not found, one is created by
 * calling \b mkstemp. And an related key file is produced containing
 * the name, the map address and the size. The name of the shared object
 * identifier is returned.
 *
 * If the shared object identifier is found, it's name is returned.
 *
 * The name of the shared object is equal to the path of the related
 * key file except for the leading "/tmp/"
 */
static char *
os_posix_getShmObjName (
    const char *name,
    void *map_address,
    os_address size,
    const int id)
{
    int key_file_fd;
    char *key_file_name;
    unsigned int name_len;
    char *db_file_name;
    char buffer[50];

    key_file_name = os_posix_findKeyFileByIdAndName (id,name);

    if (key_file_name == NULL) {
        if (map_address != NULL) {
            name_len = strlen (os_posix_key_file_prefix) + 1;
            key_file_name = os_malloc (name_len);
            if (key_file_name != NULL) {
                snprintf (key_file_name, name_len, "%s", os_posix_key_file_prefix);
                key_file_fd = os_mkstemp (key_file_name);
                write (key_file_fd, name, strlen(name) + 1);
                write (key_file_fd, "\n", 1);
                snprintf (buffer, sizeof (buffer), PA_ADDRFMT"\n", (PA_ADDRCAST)map_address);
                write (key_file_fd, buffer, strlen(buffer));
                snprintf (buffer, sizeof (buffer), PA_ADDRFMT"\n", (PA_ADDRCAST)size);
                write (key_file_fd, buffer, strlen(buffer));
                snprintf (buffer, sizeof (buffer), "POSIX-SMO\n");
                write (key_file_fd, buffer, strlen(buffer));
                snprintf (buffer, sizeof (buffer), "%d\n", getpid());
                write (key_file_fd, buffer, strlen(buffer));
                snprintf(buffer, sizeof (buffer), "%d\n", id);
                write(key_file_fd, buffer, strlen(buffer));
                close (key_file_fd);
            }
        }
    }
    if (key_file_name != NULL) {
        db_file_name = os_malloc(strlen(key_file_name));
        if (db_file_name != NULL) {
            os_strcpy (db_file_name, "/");
            os_strcat (db_file_name, &key_file_name[4]);
        }
        os_free (key_file_name);
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
os_posix_getMapAddress (
    const char *name)
{
    char *key_file_name;
    void *map_address = NULL;
    FILE *key_file;
    char line[512];

    key_file_name = os_posix_findKeyFile (name);
    if (key_file_name != NULL) {
        key_file = fopen (key_file_name, "r");
        if (key_file != NULL) {
            fgets (line, sizeof (line), key_file);
            fgets (line, sizeof(line), key_file);
            sscanf (line, PA_ADDRFMT, (PA_ADDRCAST *)&map_address);
            fclose (key_file);
        }
        os_free (key_file_name);
    }
    return map_address;
}

/** \brief Get a file map address by name
 *
 * \b os_posix_getSize returns the size of the named shared memory object.
 */
static os_address
os_posix_getSize (
    const char *name)
{
    char *key_file_name;
    os_address size = 0;
    FILE *key_file;
    char line[512];

    key_file_name = os_posix_findKeyFile (name);
    if (key_file_name != NULL) {
        key_file = fopen (key_file_name, "r");
        if (key_file != NULL) {
            fgets (line, sizeof (line), key_file);
            fgets (line, sizeof(line), key_file);
            fgets (line, sizeof(line), key_file);
            sscanf (line, PA_ADDRFMT, (PA_ADDRCAST *)&size);
            fclose (key_file);
        }
        os_free (key_file_name);
    }
    return size;
}

/** \brief Check if the contents of the identified key file
 *         matches the identified id and and set the name
 *
 * \b os_posix_getNameById tries to compare the contents of the identified
 * key file in \b key_file_name with the identified \b id.
 * On a match 1 will be returned and the domain name will be set,
 * on a mismatch 0 will be returned and name will be NULL.
 */
int
os_posix_getNameById(
    const char *key_file_name,
    const int id,
    char **name)
{
    os_int32 f_id = 0;
    FILE *key_file;
    char line[512];

    if (key_file_name != NULL) {
    key_file = fopen(key_file_name, "r");
        if (key_file != NULL) {
           if (fgets(line, sizeof(line), key_file) != NULL) { /* line 1 name */
               *name =  os_strdup(line);
           }
           fgets(line, sizeof(line), key_file);
           fgets(line, sizeof(line), key_file);
           fgets(line, sizeof(line), key_file);
           fgets(line, sizeof(line), key_file);
           if (fgets(line, sizeof(line), key_file) != NULL) { /* line 6 id */
               sscanf(line, "%d", &f_id);
           }
           fclose(key_file);
        }
    }
    if (id != f_id) {
        os_free(*name);
        *name = NULL;
    }
    return id == f_id;
}

/** \brief Check if the contents of the identified key file
 *         matches the identified id
 *
 * \b os_posix_matchKeyFile tries to compare the contents of the identified
 * key file in \b key_file_name with the identified \b id.
 * On a match 1 will be returned, on a mismatch 0 will be returned.
 */
static int
os_posix_matchKeyFileById(
    const char *key_file_name,
    const int id,
    const char *name)
{
    os_int32 f_id = 0;
    FILE *key_file;
    char line[512];
    int rv =0;

    if (key_file_name != NULL) {
    key_file = fopen(key_file_name, "r");
    if (key_file != NULL) {
        if (fgets(line, sizeof(line), key_file) != NULL) { /* line 1 name */
            if (strcmp(name, line) == 0) {
                rv = 1;
            }
        }
       fgets(line, sizeof(line), key_file);
       fgets(line, sizeof(line), key_file);
       fgets(line, sizeof(line), key_file);
       fgets(line, sizeof(line), key_file);
       if (fgets(line, sizeof(line), key_file) != NULL) { /* line 6 id */
           sscanf(line, "%d", &f_id);
       }
       fclose(key_file);
    }
       os_free(key_file_name);
    }
    return (id == f_id && rv);
}

/** \brief Destory the key related to the named shared memory object
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
os_posix_destroyKeyFile (
    const char *name)
{
    char *key_file_name;
    int rv;

    key_file_name = os_posix_findKeyFile (name);
    if (key_file_name ==  NULL) {
        rv = -1;
    } else if (unlink (key_file_name) == -1) {
        OS_REPORT (OS_WARNING, "os_posix_destroyKeyFile", 1, "unlink failed with error %d (%s)", os_getErrno(), name);
        os_free (key_file_name);
        rv = -1;
    } else {
        free (key_file_name);
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
os_posix_sharedMemoryCreate (
    const char *name,
    os_sharedAttr *sharedAttr,
    os_address size)
{
    char *shmname;
    int shmfd;
    os_result rv = os_resultSuccess;

    assert (name != NULL);
    assert (sharedAttr != NULL);
    /* roundup to page boundaries */
    shmname = os_posix_getShmObjName (name, sharedAttr->map_address, size, name);
    if (shmname != NULL) {
        shmfd = shm_open (shmname, O_CREAT | O_RDWR | O_EXCL, OS_PERMISSION);
        if (shmfd == -1) {
            OS_REPORT (OS_WARNING, "os_posix_sharedMemoryCreate", 1, "shm_open failed with error %d (%s)", os_getErrno(), name);
            rv = os_resultFail;
        } else {
#ifdef INTEGRITY
        if ( size % getpagesize() != 0 ) { 
            size += getpagesize() - ( size % getpagesize() );
        }
#endif
            if (ftruncate (shmfd, size) == -1) {
                OS_REPORT (OS_ERROR, "os_posix_sharedMemoryCreate", 1,
                    "ftruncate failed with error %d (%s)", os_getErrno(), name);
                close (shmfd);
                rv = os_resultFail;
            } else {
                if (sharedAttr->userCred.uid != 0 && sharedAttr->userCred.gid != 0) {
                    if (getuid() == 0 || geteuid() == 0) {
                        if (chown (shmname, sharedAttr->userCred.uid, sharedAttr->userCred.gid) == -1) {
                            OS_REPORT (OS_WARNING, "os_posix_sharedMemoryCreate", 1,
                                "chown failed with error %d (%s)", os_getErrno(), name);
                        }
                    } else {
                        OS_REPORT (OS_WARNING, "os_posix_sharedMemoryCreate", 2,
                            "Can not change ownership because of privilege problems (%s)", name);
                    }
                }
            }
        }
        close (shmfd);
        os_free (shmname);
    }
    return rv;
}

/** \brief Destroy the POSIX shared memory object related to name
 *
 * First \b os_posix_sharedMemoryDestroy finds the shared object identifier
 * related to \b name by calling \b os_posix_getShmObjName. If the identifier is
 * found, the shared object is detroyed by calling \b shm_unlink.
 * After that the key file related to name is detroyed by calling
 * \b os_posix_destroyKeyFile.
 */
os_result
os_posix_sharedMemoryDestroy (
    const char *name)
{
    char *shmname;
    os_result rv = os_resultSuccess;

    assert (name != NULL);
    shmname = os_posix_getShmObjName (name, NULL, 0);
    if (shmname != NULL) {
        if (shm_unlink (shmname) == -1) {
            OS_REPORT (OS_WARNING, "os_posix_sharedMemoryDestroy", 1,
                "shm_unlink failed with error %d (%s)", os_getErrno(), name);
            rv = os_resultFail;
        }
        if (os_posix_destroyKeyFile (name) == -1) {
            rv = os_resultFail;
        }
        os_free (shmname);
    }
    
    return rv;
}

os_result
os_posix_sharedMemoryGetNameFromId(
    int id,
    char **name)
{
    os_result rv;
    os_int32 r;
    r = os_posix_findNameById(id,name);
    if (r) {
        rv = os_resultSuccess;
    } else {
        rv = os_resultFail;
    }

    return rv;
}

/** \brief Attach to the POSIX shared memory object related to name
 *  
 * First \b os_posix_sharedMemoryAttach finds the shared object
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
os_posix_sharedMemoryAttach (
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

    assert (name != NULL);
    assert (sharedAttr != NULL);
    assert (mapped_address != NULL);
    shmname = os_posix_getShmObjName (name, NULL, 0);
    if (shmname != NULL) {
        request_address = os_posix_getMapAddress (name);
        size = os_posix_getSize (name);
        if (request_address != NULL && size > 0) {
            shmfd = shm_open (shmname, O_RDWR, OS_PERMISSION); 
            if (shmfd == -1) {
                OS_REPORT_WID (OS_ERROR, "os_posix_sharedMemoryAttach", 1, id,
                    "shm_open failed with error %d (%s)", os_getErrno(), name);
                rv = os_resultFail;
            } else {
#ifndef INTEGRITY
                *mapped_address = mmap (request_address, size, PROT_READ | PROT_WRITE, MAP_FIXED | MAP_SHARED, shmfd, 0);
#else
                if ( size % getpagesize() != 0 ) {
                    size += getpagesize() - ( size % getpagesize() );
                }

                *mapped_address = NULL + 0x20000000;
#endif
#endif
                if (*mapped_address == MAP_FAILED) {
                    OS_REPORT_WID (OS_ERROR, "os_posix_sharedMemoryAttach", 1, id,
                        "mmap failed with error %d (%s)", os_getErrno(), name);
                    rv = os_resultFail;
                } else if (*mapped_address != request_address) {
                    munmap (*mapped_address, size);
                    rv = os_resultFail;
                } else {
                    if (sharedAttr->lockPolicy == OS_LOCKED) {
                        if (mlock (*mapped_address, size) == -1) {
                            OS_REPORT_WID (OS_WARNING, "os_posix_sharedMemoryAttach", 1, id,
                                "mlock failed with error %d (%s)", os_getErrno(), name);
                        }
                    }
                }
                close (shmfd);
            }
        }
        os_free (shmname);
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
    void *address,
    os_int32 id)
{
    os_result rv = os_resultSuccess;
    unsigned int size;

    assert (address != NULL);
    size = os_posix_getSize (name);
#ifdef INTEGRITY
    if ( size % getpagesize() != 0 ) { 
        size += getpagesize() - ( size % getpagesize() );
    }
#endif
    if (munmap (address, size) == -1) {
        OS_REPORT_WID (OS_WARNING, "os_posix_sharedMemoryDetach", 1, id,
            "munmap failed with error %d (%s)", os_getErrno(), name);
        rv = os_resultFail;
    }
    return rv;
}

