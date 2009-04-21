/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
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

struct takeData {
    d_persistentDataListener listener;
    d_store persistentStore;
    d_durability durability;
};

C_STRUCT(d_persistentDataListener){
    C_EXTENDS(d_listener);
    u_groupQueue queue;
    os_threadId queueThread;
    struct takeData* data;
    d_waitsetEntity waitsetData;
    d_table groups;
    c_ulong optimizeUpdateInterval;
    d_storeResult lastResult;
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
