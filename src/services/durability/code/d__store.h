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

#ifndef D__STORE_H
#define D__STORE_H

#include "d__types.h"
#include "d__lock.h"
#include "d__groupHash.h"
#include "d__misc.h"
#include "u_participant.h"

#if defined (__cplusplus)
extern "C" {
#endif

void    d_storeInit     (d_store store, d_objectDeinitFunc deinit);

void    d_storeDeinit   (d_store store);

typedef d_storeResult   (*d_storeOpenFunc)                  (d_store store);

typedef d_storeResult   (*d_storeCloseFunc)                 (d_store store);

typedef d_storeResult   (*d_storeGroupsReadFunc)            (const d_store store,
                                                             d_groupList *list);

typedef d_storeResult   (*d_storeGroupListFreeFunc)         (const d_store store,
                                                             d_groupList list);

typedef d_storeResult   (*d_storeActionStartFunc)           (const d_store store);

typedef d_storeResult   (*d_storeActionStopFunc)            (const d_store store);

typedef d_storeResult   (*d_storeGroupInjectFunc)           (const d_store store,
                                                             const c_char* partition,
                                                             const c_char* topic,
                                                             const u_participant participant,
                                                             d_group *group);

typedef d_storeResult   (*d_storeGroupStoreFunc)            (const d_store store,
                                                             const d_group group,
                                                             const d_nameSpace nameSpace);

typedef d_storeResult   (*d_storeGetQualityFunc)            (const d_store store,
                                                             const d_nameSpace nameSpace,
                                                             d_quality* quality);

typedef d_storeResult   (*d_storeBackupFunc)                (const d_store store,
                                                             const d_nameSpace nameSpace);

typedef d_storeResult 	(*d_storeRestoreBackupFunc)         (const d_store store,
                                                             const d_nameSpace nameSpace);

typedef d_storeResult   (*d_storeMessageStoreFunc)          (const d_store store,
                                                             const v_groupAction message);

typedef d_storeResult   (*d_storeInstanceDisposeFunc)       (const d_store store,
                                                             const v_groupAction message);

typedef d_storeResult   (*d_storeInstanceExpungeFunc)       (const d_store store,
                                                             const v_groupAction message);

typedef d_storeResult   (*d_storeMessageExpungeFunc)        (const d_store store,
                                                             const v_groupAction message);

typedef d_storeResult   (*d_storeDeleteHistoricalDataFunc)  (const d_store store,
                                                             const v_groupAction message);

typedef d_storeResult   (*d_storeMessagesInjectFunc)        (const d_store store,
                                                             const d_group group);

typedef d_storeResult   (*d_storeInstanceRegisterFunc)      (const d_store store,
                                                       	     const v_groupAction message);

typedef d_storeResult   (*d_storeInstanceUnregisterFunc)    (const d_store store,
                                                             const v_groupAction message);

typedef d_storeResult   (*d_storeCreatePersistentSnapshotFunc)(const d_store store,
                                                              const os_char* partitionExpr,
                                                              const os_char* topicExpr,
                                                              const os_char* uri);

typedef d_storeResult   (*d_storeOptimizeGroupFunc)         (const d_store store,
                                                             const d_group group);

typedef d_storeResult   (*d_storeNsIsCompleteFunc)          (const d_store store,
                                                             const d_nameSpace nameSpace,
                                                             c_bool* isComplete);

typedef d_storeResult   (*d_storeNsMarkCompleteFunc)        (const d_store store,
                                                             const d_nameSpace nameSpace,
                                                             c_bool isComplete);

typedef d_storeResult   (*d_storeTransactionCompleteFunc)   (const d_store store,
                                                             const v_groupAction message);

typedef d_storeResult   (*d_storeMessagesLoadFunc)          (const d_store store,
                                                             const d_group group,
                                                             struct d_groupHash *groupHash);

typedef d_storeResult   (*d_storeMessagesLoadFlushFunc)     (const d_store store,
                                                             const d_group group,
                                                             c_bool inject);

C_STRUCT(d_store){
    C_EXTENDS(d_lock);
    d_admin                         admin;
    d_configuration                 config;
    d_storeType                     type;

    d_storeOpenFunc                 openFunc;
    d_storeCloseFunc                closeFunc;

    d_storeActionStartFunc          actionStartFunc;
    d_storeActionStopFunc           actionStopFunc;
    d_storeGetQualityFunc           getQualityFunc;
    d_storeBackupFunc               backupFunc;
    d_storeRestoreBackupFunc        restoreBackupFunc;
    d_storeGroupsReadFunc           groupsReadFunc;
    d_storeGroupListFreeFunc        groupListFreeFunc;
    d_storeGroupStoreFunc           groupStoreFunc;
    d_storeGroupInjectFunc          groupInjectFunc;
    d_storeMessageStoreFunc         messageStoreFunc;
    d_storeInstanceDisposeFunc      instanceDisposeFunc;
    d_storeInstanceExpungeFunc      instanceExpungeFunc;
    d_storeMessageExpungeFunc       messageExpungeFunc;
    d_storeDeleteHistoricalDataFunc deleteHistoricalDataFunc;
    d_storeMessagesInjectFunc       messagesInjectFunc;
    d_storeInstanceRegisterFunc     instanceRegisterFunc;
    d_storeInstanceUnregisterFunc   instanceUnregisterFunc;
    d_storeCreatePersistentSnapshotFunc createPersistentSnapshotFunc;
    d_storeOptimizeGroupFunc        optimizeGroupFunc;
    d_storeNsIsCompleteFunc         nsIsCompleteFunc;
    d_storeNsMarkCompleteFunc       nsMarkCompleteFunc;
    d_storeTransactionCompleteFunc  transactionCompleteFunc;
    d_storeMessagesLoadFunc         messagesLoadFunc;
    d_storeMessagesLoadFlushFunc    messagesLoadFlushFunc;
};

#if defined (__cplusplus)
}
#endif

#endif /* D__STORE_H */
