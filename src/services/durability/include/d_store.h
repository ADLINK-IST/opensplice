
#ifndef D_STORE_H
#define D_STORE_H

#include "d__types.h"
#include "d_group.h"

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

d_store             d_storeOpen                 (const d_configuration config,
                                                 const d_storeType storeType);

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

#if defined (__cplusplus)
}
#endif

#endif /*D_STORE_H*/
