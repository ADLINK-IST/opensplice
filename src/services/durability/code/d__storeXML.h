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
#ifndef D__STOREXML_H
#define D__STOREXML_H

#include "d__types.h"
#include "d__store.h"
#include "d_store.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define EXTRA_BACKSLS   (4)
#define EXTRA_BACKSLS_U (4U)
#define MAX_MESSAGE_SIZE (10485760)
#define MAX_KEY_SIZE (8192)
#define D_PERSISTENT_STORE_DIR_SIZE (512)
#define RR_NO_DIRECTORY                   "No PersistentStoreDirectory found in URI. "\
                                          "No persistent service possible!\n"
#define RR_TOPIC_SHOULD_BEGIN_WITH_TOPIC_TAG " topic '%s' should begin with topic tag\n"
#define RR_META_DATA_MISMATCH     " Could not read topic from disk '%s' meta data does not match\n"
#define RR_COULD_NOT_PROTECT              " Could not Protect ('%s')\n"
#define RR_COULD_NOT_WRITE_META_DATA      " Could not write meta data to disk '%s'\n"
#define RR_STORE_MODE_NOT_YET_SUPPORTED   " storeMode '%s' not yet supported\n"
#define RR_CREATE_GROUP_FAILED            " createGroup '%s'.'%s' failed\n"
#define RR_UNKNOWN_STORE_MODE             " unknown storeMode\n"
#define RR_COULD_NOT_OPEN_STOREPATH       "Could not open persistent store path '%s'\n"
#define RR_COULD_NOT_MAKE_STOREDIR        "Failed to make storeDirectory '%s'\n"

#define RR_COULD_NOT_WRITE_TOPIC          "Could not write topic '%s' to disk"
#define RR_COULD_NOT_WRITE__TOPIC_NOT_FOUND \
                "Could not write topic '%s' to disk (TOPIC not found)\n"
#define RR_COULD_NOT_READ_TOPIC           " Could not read topic '%s' from disk"
#define RR_TOPIC_TAG_MISSING_IN_FILE \
                "Could not read topic '%s' from disk, TOPIC tag missing in file\n"
#define RR_TOPIC_TAG_MISSING \
                "Could not read topic '%s' from disk, TOPIC tag missing in backup file \n"
#define RR_TOPIC_TAG_ALSO_MISSING_IN_BACKUP_FILE \
                "Could not read topic '%s' from disk, TOPIC tag also missing in backup file \n"
#define RR_COULD_NOT_READ_TOPIC__MISSING_TAG \
                "Could not read topic '%s' from disk, TOPIC tag missing using backup file \n"
#define RR_TOPIC_NOT_YET_KNOWN \
                "Could not read topic '%s' from disk: not yet know by persistent service\n"

#define STORE_READ_TOPIC_XML      "persistentStoreReadTopicXML"
#define STORE_STORE_TOPIC_XML     "persistentStoreStoreTopicXML"
#define STORE_GET_SERIALIZED_DATA "persistentStoreGetSerializedData"
#define STORE_STORE_TOPIC         "persistentStoreStoreTopic"


#define STRLEN_BACKSLASH_N               ((os_uint32)(2))
#define STRLEN_NEEDED_FOR_A_BIG_TAG      ((os_uint32)(13)) /* adapt for a bigger </bigTagName> */

#define DOT_XML                "_meta.xml"
#define STRLEN_DOT_XML                   ((os_uint32)(9))
#define DOT_SAVED              ".saved"
#define STRLEN_DOT_SAVED                 ((os_uint32)(6))
#define TOPIC_TAG_OPEN         "<METADATA>"
#define STRLEN_TOPIC_TAG_OPEN            ((os_uint32)(10))
#define TOPIC_TAG_CLOSE        "</METADATA>"
#define STRLEN_TOPIC_TAG_CLOSE           ((os_uint32)(11))

#define KEY_START_TAG "<key>"
#define KEY_END_TAG   "</key>"
#define D_STORE_VERSION (6)
#define D_STORE_START_STRING "<TOPIC><message version=\"6\">Do_not_edit_this_file</message>\n"
#define D_STORE_END_STRING "</TOPIC>\n"
#define D_STORE_END_STRING_NO_NL "</TOPIC>"


C_CLASS(d_storeFile);

C_STRUCT(d_storeFile){
    c_char* path;
    FILE*   fdes;
    c_char* mode;
};

#define d_storeFile(t) ((d_storeFile)(t))

C_STRUCT(d_storeXML){
    C_EXTENDS(d_store);
    c_bool opened;
    d_groupList groups;
    c_ulong maxPathLen;
    c_char* diskStorePath;
    c_bool sessionAlive;
    d_table openedFiles;
    d_storeFile dummyFile;
    c_char* dataBuffer;
    c_char* keyBuffer;
    d_table expungeActions;
    os_mutex mutex;
};

void            d_storeDeinitXML                (d_object object);

d_storeXML      d_storeNewXML                   (u_participant participant);

d_storeResult   d_storeFreeXML                  (d_storeXML store);

d_storeResult   d_storeOpenXML                  (d_store store);

d_storeResult   d_storeCloseXML                 (d_store store);

d_storeResult   d_storeActionStartXML           (const d_store store);

d_storeResult   d_storeActionStopXML            (const d_store store);

d_storeResult   d_storeGetQualityXML            (const d_store store,
                                                 const d_nameSpace nameSpace,
                                                 d_quality* quality);

d_storeResult   d_storeBackupXML                (const d_store store,
                                                 const d_nameSpace nameSpace);

d_storeResult   d_storeRestoreBackupXML                 (const d_store store,
                                                                                                 const d_nameSpace nameSpace);

d_storeResult   d_storeGroupsReadXML            (const d_store store,
                                                 d_groupList *list);

d_storeResult   d_storeGroupInjectXML           (const d_store store,
                                                 const c_char* partition,
                                                 const c_char* topic,
                                                 const u_participant participant,
                                                 d_group *group);

d_storeResult   d_storeGroupStoreXML            (const d_store store,
                                                 const d_group group);

d_storeResult   d_storeMessageStoreXML          (const d_store store,
                                                 const v_groupAction message);

d_storeResult   d_storeInstanceDisposeXML       (const d_store store,
                                                 const v_groupAction message);

d_storeResult   d_storeInstanceExpungeXML       (const d_store store,
                                                 const v_groupAction message);

d_storeResult   d_storeMessageExpungeXML        (const d_store store,
                                                 const v_groupAction message);

d_storeResult   d_storeDeleteHistoricalDataXML  (const d_store store,
                                                 const v_groupAction message);

d_storeResult   d_storeMessagesInjectXML        (const d_store store,
                                                 const d_group group);

d_storeResult   d_storeInstanceRegisterXML          (const d_store store,
                                                 const v_groupAction message);

d_storeResult   d_storeCreatePersistentSnapshotXML  (const d_store store,
                                                    const c_char* partitionExpr,
                                                    const c_char* topicExpr,
                                                    const c_char* uri);

d_storeResult   d_storeInstanceUnregisterXML    (const d_store store,
                                                 const v_groupAction message);

d_storeResult   d_storeOptimizeGroupXML         (const d_store store,
                                                 const d_group group);

d_storeResult   d_storeNsIsCompleteXML                  (const d_store store,
                                                                                                 const d_nameSpace nameSpace,
                                                                                                 c_bool* isComplete);

d_storeResult   d_storeNsMarkCompleteXML                (const d_store store,
                                                                                                 const d_nameSpace nameSpace,
                                                                                                 c_bool isComplete);

#if defined (__cplusplus)
}
#endif

#endif /*D__STOREXML_H*/
