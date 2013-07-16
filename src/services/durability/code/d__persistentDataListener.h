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

#ifndef D__PERSISTENTDATALISTENER_H
#define D__PERSISTENTDATALISTENER_H

#include "d__types.h"
#include "d__listener.h"
#include "u_groupQueue.h"

#if defined (__cplusplus)
extern "C" {
#endif

C_CLASS(d_persistentGroup);

C_STRUCT(d_persistentGroup){
    v_group group;
    c_ulong count;
};

#define d_persistentGroup(g) ((d_persistentGroup)(g))

struct persistentStatistics {
    os_uint32 samplesStored;
    os_uint32 samplesLifespanExpired;
    os_uint32 instancesDisposed;
    os_uint32 instancesCleanupDelayExpired;
    os_uint32 instancesRegistered;
    os_uint32 instancesUnregistered;
    os_uint32 eventsDeleteHistoricalData;
    os_uint32 eventsDisposeAll;
};

struct takeData {
    d_persistentDataListener listener;
    d_store persistentStore;
    d_durability durability;
};

C_STRUCT(d_persistentDataListener){
    C_EXTENDS(d_listener);
    u_groupQueue queue;
    /*os_threadId queueThread;*/
    /*struct takeData* data;*/
    struct persistentStatistics pstats;
    c_iter persistentThreads;
    os_time totalTime;
    os_uint32 totalActions;
    c_ulong runCount;
    os_mutex pmutex;
    os_cond pcond;
    os_mutex pauseMutex;
    os_cond pauseCond;
    d_waitsetEntity waitsetData;
    d_table groups;
    c_ulong optimizeUpdateInterval;
    d_storeResult lastResult;
    c_bool logStatistics;
};

void        d_persistentDataListenerInit                (d_persistentDataListener listener,
                                                         d_subscriber subscriber);

void        d_persistentDataListenerDeinit              (d_object object);

c_ulong     d_persistentDataListenerAction              (u_dispatcher o,
                                                         u_waitsetEvent event,
                                                         c_voidp usrData);

c_bool      d_persistentDataListenerHandleNewGroupLocal (c_ulong event,
                                                         d_fellow fellow,
                                                         d_group group,
                                                         c_voidp userData);

#if defined (__cplusplus)
}
#endif

#endif /*D__PERSISTENTDATALISTENER_H*/
