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

#ifndef D_STORE_H
#define D_STORE_H

#include "d__types.h"
#include "d_group.h"
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

struct baseFind {
    c_base base;
};

#define d_groupList(g) ((d_groupList)(g))
#define d_store(s) ((d_store)(s))

d_store             d_storeOpen                 (const d_durability durability,
                                                 const d_storeType storeType);

void                d_storeDeinit               (d_object object);

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
                                                 const d_group group);

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

void                d_storeReport               (const d_store store,
                                                 d_level level,
                                                 const char * eventText,
                                                 ...);
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


void         d_storeGetBase(v_entity entity, c_voidp args);



#if defined (__cplusplus)
}
#endif

#endif /*D_STORE_H*/
