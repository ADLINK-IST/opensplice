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
#ifndef D__STOREMMF_H
#define D__STOREMMF_H

#include "d__types.h"
#include "d__store.h"
#include "d_store.h"
#include "os_memMapFile.h"

#if defined (__cplusplus)
extern "C" {
#endif

C_STRUCT(d_storeMMF){
    C_EXTENDS(d_store);
    c_bool opened;
    c_char* storeFilePath;
    os_mmfHandle mmfHandle;
    d_groupList groups; /* local groupList reconstructed from storeKernel */
    os_uint32 actionsInProgress;
    os_cond actionCondition;
    d_storeMMFKernel storeKernel;
    c_base base;
};


void            d_storeDeinitMMF                (d_object object);

d_storeMMF      d_storeNewMMF                   (u_participant participant);

d_storeResult   d_storeFreeMMF                  (d_storeMMF store);

d_storeResult   d_storeOpenMMF                  (d_store store);

d_storeResult   d_storeCloseMMF                 (d_store store);

d_storeResult   d_storeActionStartMMF           (const d_store store);

d_storeResult   d_storeActionStopMMF            (const d_store store);

d_storeResult   d_storeGetQualityMMF            (const d_store store,
                                                 const d_nameSpace nameSpace,
                                                 d_quality* quality);

d_storeResult   d_storeBackupMMF                (const d_store store,
                                                 const d_nameSpace nameSpace);

d_storeResult   d_storeRestoreBackupMMF                 (const d_store store,
                                                                                                 const d_nameSpace nameSpace);

d_storeResult   d_storeGroupsReadMMF            (const d_store store,
                                                 d_groupList *list);

d_storeResult   d_storeGroupInjectMMF           (const d_store store,
                                                 const c_char* partition,
                                                 const c_char* topic,
                                                 const u_participant participant,
                                                 d_group *group);

d_storeResult   d_storeGroupStoreMMF            (const d_store store,
                                                 const d_group group);

d_storeResult   d_storeMessageStoreMMF          (const d_store store,
                                                 const v_groupAction message);

d_storeResult   d_storeInstanceDisposeMMF       (const d_store store,
                                                 const v_groupAction message);

d_storeResult   d_storeInstanceExpungeMMF       (const d_store store,
                                                 const v_groupAction message);

d_storeResult   d_storeMessageExpungeMMF        (const d_store store,
                                                 const v_groupAction message);

d_storeResult   d_storeDeleteHistoricalDataMMF  (const d_store store,
                                                 const v_groupAction message);

d_storeResult   d_storeMessagesInjectMMF        (const d_store store,
                                                 const d_group group);

d_storeResult   d_storeInstanceRegisterMMF          (const d_store store,
                                                 const v_groupAction message);

d_storeResult   d_storeCreatePersistentSnapshotMMF  (const d_store store,
                                                    const c_char* partitionExpr,
                                                    const c_char* topicExpr,
                                                    const c_char* uri);

d_storeResult   d_storeInstanceUnregisterMMF    (const d_store store,
                                                 const v_groupAction message);

d_storeResult   d_storeOptimizeGroupMMF         (const d_store store,
                                                 const d_group group);

d_storeResult   d_storeNsIsCompleteMMF                  (const d_store store,
                                                                                                 const d_nameSpace nameSpace,
                                                                                                 c_bool* isComplete);

d_storeResult   d_storeNsMarkCompleteMMF                (const d_store store,
                                                                                                 const d_nameSpace nameSpace,
                                                                                                 c_bool isComplete);

#if defined (__cplusplus)
}
#endif

#endif /*D__STOREMMF_H*/
