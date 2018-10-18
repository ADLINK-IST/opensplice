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

#ifndef D_STORE_H
#define D_STORE_H

#include "d__types.h"
#include "d__group.h"
#include "d__misc.h"
#include "d__groupHash.h"
#include "u_participant.h"

#if defined (__cplusplus)
extern "C" {
#endif

C_STRUCT(d_groupList){
    d_partition partition;
    d_topic topic;
    d_quality quality;
    d_completeness completeness;
    c_bool optimized;
    c_voidp next;
};

#define d_groupList(g) ((d_groupList)(g))
#define d_store(s) ((d_store)(s))

d_store             d_storeOpen                 (const d_durability durability,
                                                 const d_storeType storeType);

void                d_storeDeinit               (d_store store);

d_storeResult       d_storeClose                (d_store store);

d_storeResult       d_storeGetQuality           (const d_store store,
                                                 const d_nameSpace nameSpace,
                                                 d_quality* quality);

d_storeResult       d_storeBackup               (const d_store store,
                                                 const d_nameSpace nameSpace);

d_storeResult       d_storeGroupsRead           (const d_store store,
                                                 d_groupList *list);

d_storeResult       d_storeGroupInject          (const d_store store,
                                                 const c_char* partition,
                                                 const c_char* topic,
                                                 const u_participant participant,
                                                 d_group *group);

d_storeResult       d_storeActionStart          (const d_store store);

d_storeResult       d_storeActionStop           (const d_store store);

d_storeResult       d_storeGroupStore           (const d_store store,
                                                 const d_group group,
                                                 const d_nameSpace nameSpace);

d_storeResult       d_storeGroupListFree        (const d_store store,
                                                 d_groupList list);


d_storeResult       d_storeMessageStore         (const d_store store,
                                                 const v_groupAction message);

d_storeResult       d_storeInstanceDispose      (const d_store store,
                                                 const v_groupAction message);

d_storeResult       d_storeInstanceRegister     (const d_store store,
                                                 const v_groupAction message);

d_storeResult       d_storeInstanceUnregister   (const d_store store,
                                                 const v_groupAction message);

d_storeResult       d_storeMessageExpunge       (const d_store store,
                                                 const v_groupAction message);

d_storeResult       d_storeInstanceExpunge      (const d_store store,
                                                 const v_groupAction message);

d_storeResult       d_storeDeleteHistoricalData (const d_store store,
                                                 const v_groupAction message);

d_storeResult       d_storeMessagesInject       (const d_store store,
                                                 const d_group group);

d_storeResult       d_storeOptimizeGroup        (const d_store store,
                                                 const d_group group);

d_storeResult       d_storeTransactionComplete  (const d_store store,
                                                 const v_groupAction message);

void                d_storeReport               (const d_store store,
                                                 d_level level,
                                                 const char * eventText,
                                                 ...);

d_storeResult       d_storeMessagesLoad         (const d_store store,
                                                 const d_group group,
                                                 struct d_groupHash *groupHash);

d_storeResult       d_storeMessagesLoadFlush    (const d_store store,
                                                 const d_group group,
                                                 c_bool inject);

d_storeResult
d_storeCreatePersistentSnapshot(
    const d_store store,
    const c_char* partitionExpr,
    const c_char* topicExpr,
    const c_char* uri);

d_storeResult
d_storeCopyFile(
    os_char* fileStorePath,
    os_char* destStorePath);


d_storeResult           d_storeNsIsComplete             (const d_store store,
                                                                                                 const d_nameSpace nameSpace,
                                                                                                 c_bool* isComplete);

d_storeResult           d_storeNsMarkComplete           (const d_store store,
                                                                                                 const d_nameSpace nameSpace,
                                                                                                 c_bool isComplete);

d_storeResult           d_storeRestoreBackup            (const d_store store,
                                                                                                 const d_nameSpace nameSpace);

c_char*      d_storeDirNew(d_store store, const c_char *name);


void         d_storeGetBase(v_public entity, c_voidp args);



#if defined (__cplusplus)
}
#endif

#endif /* D_STORE_H */
