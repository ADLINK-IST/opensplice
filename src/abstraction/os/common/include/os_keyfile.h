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

#ifndef OS_COMMON_KEYFILE_H
#define OS_COMMON_KEYFILE_H

/** Defines the maximum line size within the database file */
#define OS_KEYFILE_LINE_SIZE_MAX    (256)
#define OS_KEYFILE_DOMAIN_NAME_SIZE (OS_KEYFILE_LINE_SIZE_MAX)
#define OS_KEYFILE_IMPL_ID_SIZE     (OS_KEYFILE_LINE_SIZE_MAX)

/** \brief Structure providing keyfile data
 *
 */
typedef struct os_keyfile_data {
    os_int32   domain_id;
    os_char    domain_name[OS_KEYFILE_DOMAIN_NAME_SIZE];
    os_address address;
    os_address size;
    os_char    implementation_id[OS_KEYFILE_IMPL_ID_SIZE];
    int        creator_pid;
    int        group_pid;
    os_state   domain_state;
} os_keyfile_data;


/*
 * Return  os_resultSuccess - Complete action succeeded, no more calls expected
 *         os_resultBusy    - Action succeded, but expects more (or none) calls
 *         other            - Failure
 */
typedef os_result
(*os_keyfile_loop_action)(const char *key_file_name, os_keyfile_data *data, void *action_arg);


typedef os_result
(*os_keyfile_parser)(FILE *key_file, os_keyfile_data *data);

/* Parser helper functions. */
os_result os_keyfile_getAddress(FILE *key_file, os_address *data);
os_result os_keyfile_getInt    (FILE *key_file, int *data);
os_result os_keyfile_getString (FILE *key_file, os_char *data, int data_size);

os_result os_keyfile_appendInt   (const char *key_file_name, int data);
os_result os_keyfile_appendString(const char *key_file_name, const char *data);

/* Data helper functions. */
char *os_keyfile_findByName     (os_keyfile_parser parser, const char *name);
char *os_keyfile_findById       (os_keyfile_parser parser, os_int32 id);
char *os_keyfile_findByIdAndName(os_keyfile_parser parser, os_int32 id, const char *name);

os_int32 os_keyfile_listUserProcesses    (os_keyfile_parser parser, os_iter pidList, const char * fileName);
os_int32 os_keyfile_listUserProcessesFree(os_iter pidList);
os_int32 os_keyfile_listDomainNames      (os_keyfile_parser parser, os_iter nameList);
void     os_keyfile_listDomainNamesFree  (os_iter nameList);
os_int32 os_keyfile_listDomainIds        (os_keyfile_parser parser, os_int32 **idList, os_int32  *listSize);

void*     os_keyfile_getMapAddress(os_keyfile_parser parser, const char *name);
os_int32  os_keyfile_getIdFromName(os_keyfile_parser parser, const char *name);
os_result os_keyfile_getSharedSize(os_keyfile_parser parser, const char *name, os_address *size);
os_result os_keyfile_getNameFromId(os_keyfile_parser parser, os_int32 id, char **name);
os_state  os_keyfile_getState     (os_keyfile_parser parser, const char *key_file_name);

os_result os_keyfile_loopAllFiles(os_keyfile_parser parser, os_keyfile_loop_action action, void *action_arg);


#endif /* OS_COMMON_KEYFILE_H */
