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

#ifndef D__STORE_H
#define D__STORE_H

#include "d__types.h"
#include "u_participant.h"
#include "d_lock.h"

#if defined (__cplusplus)
extern "C" {
#endif

void    d_storeInit     (d_store store, d_objectDeinitFunc deinit);

void    d_storeFree     (d_store store);

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
                                                             const d_group group);

typedef d_storeResult   (*d_storeGetQualityFunc)            (const d_store store,
                                                             const d_nameSpace nameSpace,
                                                             d_quality* quality);

typedef d_storeResult   (*d_storeBackupFunc)                (const d_store store,
                                                             const d_nameSpace nameSpace);

typedef d_storeResult 	(*d_storeRestoreBackupFunc)			(const d_store store,
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

typedef d_storeResult	(*d_storeNsIsCompleteFunc)			(const d_store store,
															 const d_nameSpace nameSpace,
															 c_bool* isComplete);

typedef d_storeResult 	(*d_storeNsMarkCompleteFunc)		(const d_store store,
															 const d_nameSpace nameSpace,
															 c_bool isComplete);

C_STRUCT(d_store){
    C_EXTENDS(d_lock);
    d_admin                         admin;
    d_configuration             	config;
    d_storeType                 	type;

    d_storeOpenFunc             	openFunc;
    d_storeCloseFunc            	closeFunc;

    d_storeActionStartFunc          actionStartFunc;
    d_storeActionStopFunc           actionStopFunc;
    d_storeGetQualityFunc       	getQualityFunc;
    d_storeBackupFunc           	backupFunc;
    d_storeRestoreBackupFunc		restoreBackupFunc;
    d_storeGroupsReadFunc       	groupsReadFunc;
    d_storeGroupListFreeFunc        groupListFreeFunc;
    d_storeGroupStoreFunc       	groupStoreFunc;
    d_storeGroupInjectFunc      	groupInjectFunc;
    d_storeMessageStoreFunc     	messageStoreFunc;
    d_storeInstanceDisposeFunc   	instanceDisposeFunc;
    d_storeInstanceExpungeFunc      instanceExpungeFunc;
    d_storeMessageExpungeFunc   	messageExpungeFunc;
    d_storeDeleteHistoricalDataFunc deleteHistoricalDataFunc;
    d_storeMessagesInjectFunc   	messagesInjectFunc;
    d_storeInstanceRegisterFunc		instanceRegisterFunc;
    d_storeInstanceUnregisterFunc	instanceUnregisterFunc;
    d_storeCreatePersistentSnapshotFunc createPersistentSnapshotFunc;
    d_storeOptimizeGroupFunc        optimizeGroupFunc;
    d_storeNsIsCompleteFunc			nsIsCompleteFunc;
    d_storeNsMarkCompleteFunc		nsMarkCompleteFunc;
};

#if defined (__cplusplus)
}
#endif

#endif /*D__STORE_H*/
