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
#include "d__deleteDataListener.h"
#include "d_deleteDataListener.h"
#include "d_readerListener.h"
#include "d__readerListener.h"
#include "d_listener.h"
#include "d_admin.h"
#include "d_durability.h"
#include "d_configuration.h"
#include "d_message.h"
#include "d_misc.h"
#include "d_fellow.h"
#include "d_object.h"
#include "d_actionQueue.h"
#include "d_deleteData.h"
#include "d_networkAddress.h"
#include "v_entity.h"
#include "v_groupSet.h"
#include "v_group.h"
#include "v_partition.h"
#include "v_topic.h"
#include "v_participant.h"
#include "os_heap.h"

C_CLASS(deleteGroupData);

C_STRUCT(deleteGroupData) {
    c_char* partitionExpression;
    c_char* topicExpression;
    d_timestamp deleteTime;
    d_fellow sender;
    d_deleteDataListener listener;
};

#define deleteGroupData(d) ((deleteGroupData)(d))

static deleteGroupData
deleteGroupDataNew(
    const c_char* partitionExpression,
    const c_char* topicExpression,
    d_timestamp deleteTime,
    d_fellow sender,
    d_deleteDataListener listener)
{
    deleteGroupData data;

    assert(sender);
    assert(listener);
    assert(d_listenerIsValid(d_listener(listener), D_DELETE_DATA_LISTENER));
    assert(d_objectIsValid(d_object(sender), D_FELLOW) == TRUE);

    data = deleteGroupData(os_malloc(C_SIZEOF(deleteGroupData)));

    if(data){
        if(partitionExpression){
            data->partitionExpression = os_strdup(partitionExpression);
        } else {
            data->partitionExpression = NULL;
        }
        if(topicExpression){
            data->topicExpression = os_strdup(topicExpression);
        } else {
            data->topicExpression = NULL;
        }
        data->deleteTime.seconds     = deleteTime.seconds;
        data->deleteTime.nanoseconds = deleteTime.nanoseconds;
        data->sender                 = d_fellow(d_objectKeep(d_object(sender)));
        data->listener               = listener;
    }
    return data;
}

static void
deleteGroupDataFree(
    deleteGroupData data)
{
    if(data){
        if(data->partitionExpression){
            os_free(data->partitionExpression);
        }
        if(data->topicExpression){
            os_free(data->topicExpression);
        }
        if(data->sender){
            d_fellowFree(data->sender);
        }
        os_free(data);
    }
    return;
}

d_deleteDataListener
d_deleteDataListenerNew(
    d_subscriber subscriber)
{
    d_deleteDataListener listener;

    listener = NULL;

    if(subscriber){
        listener = d_deleteDataListener(os_malloc(C_SIZEOF(d_deleteDataListener)));
        d_listener(listener)->kind = D_DELETE_DATA_LISTENER;
        d_deleteDataListenerInit(listener, subscriber);
    }
    return listener;
}

void
d_deleteDataListenerInit(
    d_deleteDataListener listener,
    d_subscriber subscriber)
{
    os_threadAttr attr;

    os_threadAttrInit(&attr);

    d_readerListenerInit(   d_readerListener(listener),
                            d_deleteDataListenerAction, subscriber,
                            D_DELETE_DATA_TOPIC_NAME,
                            D_DELETE_DATA_TOP_NAME,
                            V_RELIABILITY_RELIABLE,
                            V_HISTORY_KEEPALL,
                            V_LENGTH_UNLIMITED,
                            attr,
                            d_deleteDataListenerDeinit);

}

void
d_deleteDataListenerFree(
    d_deleteDataListener listener)
{
    assert(d_listenerIsValid(d_listener(listener), D_DELETE_DATA_LISTENER));

    if(listener){
        d_readerListenerFree(d_readerListener(listener));
    }
}

void
d_deleteDataListenerDeinit(
    d_object object)
{
    OS_UNUSED_ARG(object);
    assert(d_listenerIsValid(d_listener(object), D_DELETE_DATA_LISTENER));

    return;
}

c_bool
d_deleteDataListenerStart(
    d_deleteDataListener listener)
{
    return d_readerListenerStart(d_readerListener(listener));
}

c_bool
d_deleteDataListenerStop(
    d_deleteDataListener listener)
{
    return d_readerListenerStop(d_readerListener(listener));
}

static void
deleteAction(
    v_entity entity,
    c_voidp args)
{
    deleteGroupData data;
    c_iter matchingGroups;
    v_group group;
    v_participant participant;
    c_time t;
    c_value params[2];
    d_admin admin;
    c_char *partition, *topic;

    assert(entity != NULL);
    assert(C_TYPECHECK(entity, v_participant));

    data            = deleteGroupData(args);
    participant     = v_participant(entity);
    t.seconds       = data->deleteTime.seconds;
    t.nanoseconds   = data->deleteTime.nanoseconds;
    admin           = d_listenerGetAdmin(d_listener(data->listener));

    params[0]       = c_stringValue(data->partitionExpression);
    params[1]       = c_stringValue(data->topicExpression);

    c_lockRead(&participant->lock);
    matchingGroups = v_groupSetSelect(
                        v_kernel(v_object(participant)->kernel)->groupSet,
                        "partition.name like %0 AND topic.name like %1",
                        params);
    c_lockUnlock(&participant->lock);
    group = v_group(c_iterTakeFirst(matchingGroups));

    while(group){
        partition = v_partitionName(v_groupPartition(group));
        topic = v_topicName(v_groupTopic(group));

        if(d_adminGroupInAligneeNS        (admin,
                                           partition,
                                           topic) == TRUE){
            if(d_fellowIsGroupInNameSpaces(data->sender,
                                           partition,
                                           topic,
                                           D_DURABILITY_ALL) == TRUE){
                v_groupDeleteHistoricalData(group, t);
            }
        }
        c_free(group);
        group = v_group(c_iterTakeFirst(matchingGroups));
    }
    c_iterFree(matchingGroups);

    return;
}

c_bool
deleteGroupDataAction(
    d_action action,
    c_bool terminate)
{
    deleteGroupData data;
    d_durability durability;
    d_admin admin;
    c_bool callOnceMore;
    d_communicationState comState;

    data = deleteGroupData(d_actionGetArgs(action));

    if(terminate == FALSE){
        admin = d_listenerGetAdmin(d_listener(data->listener));
        durability = d_adminGetDurability(admin);

        if(d_durabilityGetState(durability) == D_STATE_COMPLETE){
            comState = d_fellowGetCommunicationState(data->sender);

            if(comState == D_COMMUNICATION_STATE_APPROVED){
                /*fellow approved and I am complete, so take action now.*/
                u_entityAction(u_entity(d_durabilityGetService(durability)),
                               deleteAction, data);
                deleteGroupDataFree(data);
                callOnceMore = FALSE;
            } else if(comState == D_COMMUNICATION_STATE_UNKNOWN){
                /*communication state unknown so far, so try again later.*/
                callOnceMore = TRUE;
            } else {
                /*fellow not approved, so ignore deletion.*/
                deleteGroupDataFree(data);
                callOnceMore = FALSE;
            }
        } else {
            callOnceMore = TRUE;
        }
    } else {
        /*durability is terminating, so clean up data and forget action */
        deleteGroupDataFree(data);
        callOnceMore = FALSE;
    }
    return callOnceMore;
}

void
d_deleteDataListenerAction(
    d_listener listener,
    d_message message)
{
    d_networkAddress sender;
    d_fellow fellow;
    d_admin admin;
    d_durability durability;
    d_deleteData delData;
    deleteGroupData data;
    d_actionQueue queue;
    d_action action;
    os_time sleepTime;

    assert(d_listenerIsValid(d_listener(listener), D_DELETE_DATA_LISTENER));

    admin      = d_listenerGetAdmin(listener);
    queue      = d_adminGetActionQueue(admin);
    durability = d_adminGetDurability(admin);
    delData    = d_deleteData(message);
    sender     = d_networkAddressNew(message->senderAddress.systemId,
                                     message->senderAddress.localId,
                                     message->senderAddress.lifecycleId);

    fellow     = d_adminGetFellow(admin, sender);

    if(fellow){
        if(d_fellowGetCommunicationState(fellow) == D_COMMUNICATION_STATE_APPROVED){
            if(delData->partitionExpr && delData->topicExpr){
                d_printTimedEvent(durability, D_LEVEL_FINE,
                           D_THREAD_DELETE_DATA_LISTENER,
                          "Received deleteData message. partitionExpr: '%s', topicExpr: '%s'.\n",
                          delData->partitionExpr,
                          delData->topicExpr);
            } else if(delData->partitionExpr){
                d_printTimedEvent(durability, D_LEVEL_FINE,
                           D_THREAD_DELETE_DATA_LISTENER,
                          "Received deleteData message. partitionExpr: '%s', topicExpr: 'NULL'.\n",
                          delData->partitionExpr);
            } else if(delData->topicExpr){
                d_printTimedEvent(durability, D_LEVEL_FINE,
                           D_THREAD_DELETE_DATA_LISTENER,
                          "Received deleteData message. partitionExpr: 'NULL', topicExpr: '%s'.\n",
                          delData->topicExpr);
            } else {
                d_printTimedEvent(durability, D_LEVEL_FINE,
                           D_THREAD_DELETE_DATA_LISTENER,
                          "Received deleteData message. partitionExpr: 'NULL', topicExpr: 'NULL'.\n");
            }

            data = deleteGroupDataNew(delData->partitionExpr, delData->topicExpr,
                                      delData->actionTime, fellow,
                                      d_deleteDataListener(listener));
            sleepTime.tv_sec  = 1;
            sleepTime.tv_nsec = 0;
            action = d_actionNew(os_timeGet(), sleepTime, deleteGroupDataAction, data);
            d_actionQueueAdd(queue, action);
        } else {
            d_printTimedEvent(durability, D_LEVEL_WARNING,
                       D_THREAD_DELETE_DATA_LISTENER,
                      "Fellow not approved, so ignore the message.\n");
        }
        d_fellowFree(fellow);
    } else {
        d_printTimedEvent(durability, D_LEVEL_WARNING,
                D_THREAD_DELETE_DATA_LISTENER,
                "Fellow unknown so far, so ignore the message.\n");
    }
    d_networkAddressFree(sender);

    return;
}
