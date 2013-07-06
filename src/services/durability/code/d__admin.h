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

#ifndef D__ADMIN_H
#define D__ADMIN_H

#include "d__types.h"
#include "d_lock.h"
#include "u_user.h"
#include "os_cond.h"
#include "os_mutex.h"
#include "c_iterator.h"

#if defined (__cplusplus)
extern "C" {
#endif

C_STRUCT(d_admin){
    C_EXTENDS(d_lock);
    d_durability        durability;
    d_table             fellows;
    d_table             groups;
    d_table             readerRequests;
    d_networkAddress    myAddress;
    c_ulong             alignerGroupCount;
    d_fellow            cachedFellow;

    d_publisher         publisher;
    d_subscriber        subscriber;

    u_topic             statusRequestTopic;
    u_topic             groupsRequestTopic;
    u_topic             sampleRequestTopic;
    u_topic             statusTopic;
    u_topic             newGroupTopic;
    u_topic             sampleChainTopic;
    u_topic             nameSpacesTopic;
    u_topic             nameSpacesRequestTopic;
    u_topic             deleteDataTopic;

    d_actionQueue       actionQueue;

    os_mutex            eventMutex;
    c_iter              eventListeners;
    c_iter              eventQueue;
    os_cond             eventCondition;
    os_threadId         eventThread;
    c_bool              eventThreadTerminate;

    c_iter              nameSpaces;
};

C_CLASS(d_adminEvent);

C_STRUCT(d_adminEvent){
    C_EXTENDS(d_object);
    c_ulong  event;
    d_fellow fellow;
    d_nameSpace nameSpace;
    d_group  group;
    c_voidp userData;
};

#define d_adminEvent(e) ((d_adminEvent)(e))

struct cleanupData{
    c_iter fellows;
    d_timestamp stamp;
};

struct sendData{
    d_admin admin;
    d_configuration configuration;
    d_networkAddress addressee;
};

u_topic                 d_adminInitTopic                    (d_admin admin,
                                                             const c_char* topicName,
                                                             const c_char* typeName,
                                                             const c_char* keyList,
                                                             v_reliabilityKind reliability,
                                                             v_historyQosKind historyKind,
                                                             c_long historyDepth);

void                    d_adminInitAddress                  (v_entity entity,
                                                             c_voidp args);

c_bool                  d_adminLocalGroupsCompleteAction    (d_group group,
                                                             c_voidp userData);

c_bool                  d_adminSendLocalGroupsAction        (d_group group,
                                                             c_voidp userData);

c_bool                  d_adminCleanupFellowsAction         (d_fellow fellow,
                                                             c_voidp args);

d_adminEvent            d_adminEventNew                     (c_ulong event,
                                                             d_fellow fellow,
                                                             d_nameSpace nameSpace,
                                                             d_group group,
                                                             c_voidp userData);

void                    d_adminDeinit                       (d_object object);

void                    d_adminEventFree                    (d_adminEvent event);

void                    d_adminEventDeinit                  (d_object object);

void*                   d_adminEventThreadStart             (void* arg);

#if defined (__cplusplus)
}
#endif

#endif /* D__ADMIN_H */
