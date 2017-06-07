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

/** \file os/common/code/os_keyfile.c
 *  \brief Keyfile handling for shared memory management
 *
 * Implements common keyfile handling where possible.
 */


#include "os_heap.h"
#include "os_report.h"
#include "os_abstract.h"
#include "os_stdlib.h"
#include "os_string.h"
#include "os_errno.h"

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

/** Defines the file format for the database file
 */
const char os_keyfile_key_file_format[] = "spddskey_XXXXXX";



/*****************************************************
 *
 * Keyfile parsing helper functions.
 *
 *****************************************************/

os_result
os_keyfile_getAddress(
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
os_keyfile_getInt(
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
os_keyfile_getString(
    FILE *key_file,
    os_char *data,
    int data_size)
{
    os_result rv = os_resultFail;
    os_char *ptr;
    if (fgets(data, data_size, key_file) != NULL) {
        /* remove trailing whitespace */
        ptr = os_strrchrs (data, " \t\r\n", OS_FALSE);
        if (ptr != NULL) {
            ptr++;
           *ptr = '\0';
        }
        rv = os_resultSuccess;
    }
    return rv;
}

os_result
os_keyfile_appendInt(
    const char *key_file_name,
    int data)
{
    char line[OS_KEYFILE_LINE_SIZE_MAX];
    assert(key_file_name);
    snprintf(line, OS_KEYFILE_LINE_SIZE_MAX, "%d", data);
    return os_keyfile_appendString(key_file_name, line);
}

os_result
os_keyfile_appendString(
    const char *key_file_name,
    const char *data)
{
    os_result rv = os_resultUnavailable;
    FILE *key_file;

    assert(key_file_name);
    assert(data);

    key_file = fopen(key_file_name, "a");
    if (key_file != NULL) {
        fprintf(key_file, "%s\n", data);
        fflush(key_file);
        fclose(key_file);
        rv = os_resultSuccess;
    }

    return rv;
}



/*****************************************************
 *
 * Internal data searching helper functions.
 *
 *****************************************************/

typedef struct os_keyfile_dataArg {
    os_int32 id;
    const char *name;
    os_keyfile_data data;
} os_keyfile_dataArg;


static os_result
os_keyfile_getDataByIdAction(
    const char *key_file_name,
    os_keyfile_data *data,
    void *action_arg)
{
    os_result rv = os_resultBusy; /* busy: keep searching */
    os_keyfile_dataArg *arg = (os_keyfile_dataArg*)action_arg;

    assert(arg);
    assert(data);
    assert(key_file_name);
    OS_UNUSED_ARG(key_file_name);

    if (data->domain_id == arg->id) {
        memcpy(&(arg->data), data, sizeof(os_keyfile_data));
        rv = os_resultSuccess; /* success: stop searching */
    }

    return rv;
}

static os_result
os_keyfile_getDataByNameAction(
    const char *key_file_name,
    os_keyfile_data *data,
    void *action_arg)
{
    os_result rv = os_resultBusy; /* busy: keep searching */
    os_keyfile_dataArg *arg = (os_keyfile_dataArg*)action_arg;

    assert(arg);
    assert(data);
    assert(arg->name);
    assert(key_file_name);
    OS_UNUSED_ARG(key_file_name);

    if (strcmp(data->domain_name, arg->name) == 0) {
        memcpy(&(arg->data), data, sizeof(os_keyfile_data));
        rv = os_resultSuccess; /* success: stop searching */
    }

    return rv;
}




/*****************************************************
 *
 * Internal parsing and searching helper functions.
 *
 *****************************************************/

static os_result
os_keyfile_parse(
    os_keyfile_parser parser,
    const char *key_file_name,
    os_keyfile_data *data)
{
    os_result rv = os_resultUnavailable;
    FILE *key_file;

    key_file = fopen(key_file_name, "r");
    if (key_file != NULL) {
        rv = parser(key_file, data);
        fclose(key_file);
    }

    return rv;
}



os_int32 os_keyfile_compareNameWithSocketFileAction(void *o, os_iterActionArg arg)
{
    char* pcKeyName = (char*)o;
    char* pcSocketName = (char*)arg;

    if (strncmp(&pcSocketName[9], &pcKeyName[strlen(pcKeyName)-6] , 6) == 0) {
        return 1;
    }
    else {
        return 0;
    }
}

/** \brief Find all key files and call the action functions with it.
 *
 * \b os_keyfile_loopAllFiles tries to find all the key files in the
 * temporary directory directory. The key files are prefixed with
 * /<temporay directory>/spddskey_.
 *
 * \b os_keyfile_loopAllFiles first opens the temporary directory by
 * calling opendir. Then it reads all entries in search for any entry
 * that starts with the name "spddskey_" by reading the entry with
 * readdir. If the a matching entry is found, it calls given action
 * function with the found key file.
 */
/*
 * Return  os_resultSuccess - The action function was called at least once and succeeded.
 *         other            - Failure or no calls to action.
 */
os_result
os_keyfile_loopAllFiles(
    os_keyfile_parser parser,
    os_keyfile_loop_action action,
    void *action_arg)
{
    os_result rv = os_resultFail;
    DIR *key_dir;
    struct dirent *entry;
    const char * dir_name = NULL;
    char * key_file_name = NULL;
    char * socket_file_name = NULL;
    os_size_t key_file_name_size;
    os_keyfile_data data;
    int closeret;
    char* pcSocketName = NULL;
    char* pcKeyName = NULL;
    os_iter keyNamesIter = NULL;
    os_iter socketNamesIter = NULL;
    os_boolean fContinue = OS_TRUE;
    assert(parser);
    assert(action);

    dir_name = os_getTempDir();
    key_dir = opendir(dir_name);
    if (key_dir) {
        entry = readdir(key_dir);
        while (entry != NULL) {
            if (strncmp(entry->d_name, "spddskey_", 9) == 0) {
                key_file_name_size = strlen(dir_name) + strlen(os_keyfile_key_file_format) + 2;
                key_file_name = os_malloc (key_file_name_size);
                if (key_file_name != NULL) {
                    snprintf(key_file_name,
                             key_file_name_size,
                             "%s/%s",
                             dir_name,
                             entry->d_name);
                    if (fContinue) {
                        rv = os_keyfile_parse(parser, key_file_name, &data);
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
                        }
                        if (rv != os_resultBusy) {
                            /* Complete action succeeded or failed continue looping to find all keyfiles,
                             * but don't use action function on coming key files. */
                            fContinue = OS_FALSE;
                        }
                    }
                    keyNamesIter = os_iterAppend(keyNamesIter, key_file_name);
                } else {
                    rv = os_resultFail;
                }
            } else {
                /* This wasn't a key file, check if it is a socket file to delete stale socket files*/
                if (strncmp(entry->d_name, "osplsock_", 9) == 0) {
                    /* check if this socket file has already a corresponding key file */
                    pcKeyName = (char*)os_iterReadAction(keyNamesIter, os_keyfile_compareNameWithSocketFileAction, (os_iterActionArg)entry->d_name);
                    if (pcKeyName == NULL) {
                        /* no keyfile found yet for this socket file, store it in the list to try later when all keyfiles are known */
                        socketNamesIter = os_iterAppend(socketNamesIter, (void*)os_strdup(entry->d_name));
                    }
                }
            }
            entry = readdir(key_dir);
        }
        /* check if all socket files do have a corresponding key file */
        pcSocketName = (char*)os_iterTakeFirst(socketNamesIter);
        while (pcSocketName != NULL) {
            pcKeyName = (char*)os_iterReadAction(keyNamesIter, os_keyfile_compareNameWithSocketFileAction, (os_iterActionArg)pcSocketName);
            if (pcKeyName == NULL) {
                /* No key file found for this socket file, delete it */
                socket_file_name = os_malloc (strlen(dir_name) + strlen(pcSocketName) + 2);
                if (socket_file_name != NULL) {
                    snprintf(socket_file_name,
                            strlen(dir_name) + strlen(pcSocketName) + 2,
                             "%s/%s",
                             dir_name,
                             pcSocketName);
                    (void)unlink(socket_file_name);
                    os_free(socket_file_name);
                    socket_file_name = NULL;
                }
            }
            os_free(pcSocketName);
            pcSocketName = (char*)os_iterTakeFirst(socketNamesIter);
        }
        os_iterFree(socketNamesIter);
        pcKeyName = (char*)os_iterTakeFirst(keyNamesIter);
        while (pcKeyName != NULL) {
            os_free(pcKeyName);
            pcKeyName = (char*)os_iterTakeFirst(keyNamesIter);
        }
        os_iterFree(keyNamesIter);
        do {
            closeret = closedir(key_dir);
        } while ( closeret != 0 && os_getErrno() == EINTR );
        assert( closeret == 0 );
    }
    return rv;
}




/*****************************************************
 *
 * Search for keyfile helper functions.
 *
 *****************************************************/

/** \brief Return the file-path of the key file related
 *         to the identified shared memory
 *
 * \b os_keyfile_findKeyFile tries to find the key file related to the domain name.
 * If the given name matches the key file contents, the proper file is found, and
 * the path is returned to the caller. The memory for the path is allocated
 * from heap and is expected to be freed by the caller.
 *
 * \b If no matching entry is found, NULL is returned to the caller.
 *
 * \b os_keyfile_findKeyFileById and os_keyfile_findKeyFileByNameAndId do basically
 * the same.
 */

typedef struct os_keyfile_findArg {
    os_int32 id;
    const char *name;
    char *key_file;
} os_keyfile_findArg;

static os_result
os_keyfile_findByNameAction(
    const char *key_file_name,
    os_keyfile_data *data,
    void *action_arg)
{
    os_result rv = os_resultBusy; /* busy: keep searching */
    os_keyfile_findArg *arg = (os_keyfile_findArg*)action_arg;

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
os_keyfile_findByName(
    os_keyfile_parser parser,
    const char *name)
{
    os_keyfile_findArg arg;
    arg.id = 0;
    arg.name = name;
    arg.key_file = NULL;
    (void)os_keyfile_loopAllFiles(parser, os_keyfile_findByNameAction, (void*)&arg);
    return arg.key_file;
}



static os_result
os_keyfile_findByIdAction(
    const char *key_file_name,
    os_keyfile_data *data,
    void *action_arg)
{
    os_result rv = os_resultBusy; /* busy: keep searching */
    os_keyfile_findArg *arg = (os_keyfile_findArg*)action_arg;

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
os_keyfile_findById(
    os_keyfile_parser parser,
    os_int32 id)
{
    os_keyfile_findArg arg;
    arg.id = id;
    arg.name = NULL;
    arg.key_file = NULL;
    (void)os_keyfile_loopAllFiles(parser, os_keyfile_findByIdAction, (void*)&arg);
    return arg.key_file;
}


static os_result
os_keyfile_findByIdAndNameAction(
    const char *key_file_name,
    os_keyfile_data *data,
    void *action_arg)
{
    os_result rv = os_resultBusy; /* busy: keep searching */
    os_keyfile_findArg *arg = (os_keyfile_findArg*)action_arg;

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
os_keyfile_findByIdAndName(
    os_keyfile_parser parser,
    os_int32 id,
    const char *name)
{
    os_keyfile_findArg arg;
    arg.id = id;
    arg.name = name;
    arg.key_file = NULL;
    (void)os_keyfile_loopAllFiles(parser, os_keyfile_findByIdAndNameAction, (void*)&arg);
    return arg.key_file;
}




/*****************************************************
 *
 * Data lists helper functions.
 *
 *****************************************************/

/** \brief Return list of processes defined in key file \b fileName
 *         as an iterator contained in \b pidList
 *
 * \b linux key file format only supports creator pid entry in key file
 *
 * \b returns 0 on success and 1 if key file not found or unreadable
 */
os_int32
os_keyfile_listUserProcesses(
    os_keyfile_parser parser,
    os_iter pidList,
    const char * fileName)
{
    os_keyfile_data data;
    char pidstr[16];
    os_int32 result = 1;

    if (os_keyfile_parse(parser, fileName, &data) == os_resultSuccess) {
        /* change pid to string to match iterator model */
        snprintf(pidstr, 16, "%d", data.creator_pid);
        os_iterAppend(pidList, os_strdup(pidstr));
        result = 0;
    }

    return result;
}

/** \brief frees memory used by iterator created by prior call to
 *  \b os_keyfile_listUserProcesses creating list in \b pidList
 */
os_int32
os_keyfile_listUserProcessesFree(
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
 * \b linux key file format only supports creator pid entry in key file
 *
 * \b returns 0 on success and 1 if key file not found or unreadable
 */
static os_result
os_keyfile_listDomainNamesAction(
    const char *key_file_name,
    os_keyfile_data *data,
    void *action_arg)
{
    os_iter nameList = (os_iter)action_arg;
    OS_UNUSED_ARG(key_file_name);
    os_iterAppend(nameList, os_strdup(data->domain_name));
    return os_resultBusy; /* busy: keep searching */
}

os_int32
os_keyfile_listDomainNames(
    os_keyfile_parser parser,
    os_iter nameList)
{
    os_int32 retVal = 1;
    if (os_keyfile_loopAllFiles(parser,
                                os_keyfile_listDomainNamesAction,
                                (void*)nameList) == os_resultBusy) {
        retVal = 0;
    }
    return retVal;
}

/** \brief frees memory used by iterator created by prior call to
 *  \b os_keyfile_listDomainNames creating list in \b nameList
 */
void
os_keyfile_listDomainNamesFree(
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
 * \b linux key file format only supports creator pid entry in key file
 *
 * \b returns 0 on success and 1 if key file not found or unreadable
 */

typedef struct os_keyfile_listDomainIdsActionArg {
    os_int32 **idList;
    os_int32  *listSize;
} os_keyfile_listDomainIdsActionArg;

static os_result
os_keyfile_listDomainIdsAction(
    const char *key_file_name,
    os_keyfile_data *data,
    void *action_arg)
{
    os_keyfile_listDomainIdsActionArg *arg = (os_keyfile_listDomainIdsActionArg *)action_arg;
    os_result rv = os_resultFail;
    os_int32 size = *(arg->listSize);

    OS_UNUSED_ARG(key_file_name);

    *(arg->idList) = (os_int32*)os_realloc(*(arg->idList), (os_size_t)(sizeof(os_int32) * (os_uint32)(size + 1)));
    if (*(arg->idList) != NULL) {
        (*(arg->idList))[size] = data->domain_id;
        *(arg->listSize) = size + 1;
        rv = os_resultBusy; /* busy: keep searching */;
    }

    return rv;
}

os_int32
os_keyfile_listDomainIds(
    os_keyfile_parser parser,
    os_int32 **idList,
    os_int32  *listSize)
{
    os_keyfile_listDomainIdsActionArg arg;
    os_int32 retVal = 1;

    *idList   = NULL;
    *listSize = 0;

    arg.idList = idList;
    arg.listSize = listSize;
    if (os_keyfile_loopAllFiles(parser,
                                os_keyfile_listDomainIdsAction,
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




/*****************************************************
 *
 * Data acquiring helper functions.
 *
 *****************************************************/

/** \brief Get a file map address by name
 *
 * \b os_keyfile_getMapAddress returns the map address of the named shared memory object.
 */
void *
os_keyfile_getMapAddress(
    os_keyfile_parser parser,
    const char *name)
{
    os_keyfile_dataArg arg;
    os_address map_address;
    os_result rv;

    map_address = 0;

    arg.name = name;

    rv = os_keyfile_loopAllFiles(parser, os_keyfile_getDataByNameAction, &arg);
    if (rv == os_resultSuccess) {
        map_address = arg.data.address;
    }

    return (void*)map_address;
}


/** \brief Get a file id by name
 *
 * \b os_keyfile_getIdFromName returns the id of the named shared memory object.
 */
os_int32
os_keyfile_getIdFromName(
    os_keyfile_parser parser,
    const char *name)
{
    os_keyfile_dataArg arg;
    os_result rv;
    os_int32 id = 0;

    arg.name = name;

    rv = os_keyfile_loopAllFiles(parser, os_keyfile_getDataByNameAction, &arg);
    if (rv == os_resultSuccess) {
        id = arg.data.domain_id;
    }

    return id;
}





/** \brief Get the size of sharedmem
 *
 * \b os_keyfile_getSharedSize returns the size of the named shared memory object.
 */
os_result
os_keyfile_getSharedSize(
    os_keyfile_parser parser,
    const char *name,
    os_address *size)
{
    os_keyfile_dataArg arg;
    os_result rv;

    arg.name = name;

    rv = os_keyfile_loopAllFiles(parser, os_keyfile_getDataByNameAction, &arg);
    if (rv == os_resultSuccess) {
        *size = arg.data.size;
    }

    return rv;
}


os_result
os_keyfile_getNameFromId(
    os_keyfile_parser parser,
    os_int32 id,
    char **name)
{
    os_keyfile_dataArg arg;
    os_result rv;

    arg.id = id;
    arg.name = NULL;

    rv = os_keyfile_loopAllFiles(parser, os_keyfile_getDataByIdAction, &arg);
    if (rv == os_resultSuccess) {
        *name = os_strdup(arg.data.domain_name);
        if (*name == NULL) {
            rv = os_resultFail;
        }
    }

    return rv;
}


os_state
os_keyfile_getState(
    os_keyfile_parser parser,
    const char *key_file_name)
{
    os_keyfile_data data;
    os_result rv;

    assert(key_file_name);

    rv = os_keyfile_parse(parser, key_file_name, &data);
    if (rv != os_resultSuccess) {
        data.domain_state = OS_STATE_NONE;
    }

    return data.domain_state;
}

