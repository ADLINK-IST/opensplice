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
#include "d__deleteDataListener.h"
#include "d__readerListener.h"
#include "d__listener.h"
#include "d__admin.h"
#include "d__durability.h"
#include "d__configuration.h"
#include "d__misc.h"
#include "d__fellow.h"
#include "d__actionQueue.h"
#include "d_message.h"
#include "d_object.h"
#include "d_deleteData.h"
#include "d_networkAddress.h"
#include "u_observable.h"
#include "v_entity.h"
#include "v_groupSet.h"
#include "v_group.h"
#include "v_partition.h"
#include "v_topic.h"
#include "v_participant.h"
#include "os_heap.h"

/**
 * Macro that checks the d_deleteDataListener validity.
 * Because d_deleteDataListener is a concrete class typechecking is required.
 */
#define             d_deleteDataListenerIsValid(_this)   \
    d_listenerIsValidKind(d_listener(_this), D_DELETE_DATA_LISTENER)

/**
 * \brief The d_deleteDataListener cast macro.
 *
 * This macro casts an object to a d_deleteDataListener object.
 */
#define d_deleteDataListener(_this) ((d_deleteDataListener)(_this))

C_STRUCT(d_deleteDataListener){
    C_EXTENDS(d_readerListener);
};

C_CLASS(deleteGroupData);

C_STRUCT(deleteGroupData) {
    c_char* partitionExpression;
    c_char* topicExpression;
    os_timeE deleteTime;
    d_fellow sender;
    d_deleteDataListener listener;
};

#define deleteGroupData(d) ((deleteGroupData)(d))

static deleteGroupData
deleteGroupDataNew(
    const c_char* partitionExpression,
    const c_char* topicExpression,
    os_timeE deleteTime,
    d_fellow sender,
    d_deleteDataListener listener)
{
    deleteGroupData data;

    assert(sender);
    assert(listener);
    assert(d_deleteDataListenerIsValid(listener));
    assert(d_fellowIsValid(sender));

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
        data->deleteTime = deleteTime;
        data->sender     = d_fellow(d_objectKeep(d_object(sender)));
        data->listener   = listener;
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

static void
deleteAction(
    v_public entity,
    c_voidp args)
{
    deleteGroupData data;
    c_iter matchingGroups;
    v_group group;
    v_participant participant;
    os_timeE t;
    c_value params[2];
    d_admin admin;
    c_char *partition, *topic;

    assert(entity != NULL);
    assert(C_TYPECHECK(entity, v_participant));

    data            = deleteGroupData(args);
    participant     = v_participant(entity);
    t               = data->deleteTime;
    admin           = d_listenerGetAdmin(d_listener(data->listener));

    params[0]       = c_stringValue(data->partitionExpression);
    params[1]       = c_stringValue(data->topicExpression);

    matchingGroups = v_groupSetSelect(
                        v_kernel(v_object(participant)->kernel)->groupSet,
                        "partition.name like %0 AND topic.name like %1",
                        params);
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

static c_bool
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
                (void)u_observableAction(u_observable(
                    d_durabilityGetService(durability)),
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

static void
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
    os_duration sleepTime = OS_DURATION_INIT(1, 0);
    os_timeW actionTimeW;
    os_timeE actionTimeE;
    os_duration duration;

    assert(d_deleteDataListenerIsValid(listener));

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
            d_printTimedEvent(durability, D_LEVEL_FINE,
                      "Received deleteData message for partition/topic expression '%s.%s'\n",
                      (delData->partitionExpr) ? delData->partitionExpr : "NULL",
                      (delData->topicExpr) ? delData->topicExpr : "NULL");
            /* To delete data the v_groupDeleteHistoricalData() is called.
             * This function requires a timeE. The deleteData message however
             * contains a timeW indicating that all data before this time must
             * be purged. Therefore, we need to calculate the corresponding local
             * timeE. Note that this only works if clocks on nodes are aligned! */
            d_timestampToTimeW(&actionTimeW, &delData->actionTime, IS_Y2038READY(delData));
            duration = os_timeWDiff(os_timeWGet(), actionTimeW);
            actionTimeE = os_timeEAdd(os_timeEGet(), duration);

            data = deleteGroupDataNew(delData->partitionExpr, delData->topicExpr,
                                      actionTimeE, fellow,
                                      d_deleteDataListener(listener));
            action = d_actionNew(os_timeMGet(), sleepTime, deleteGroupDataAction, data);
            d_actionQueueAdd(queue, action);
        } else {
            d_printTimedEvent(durability, D_LEVEL_WARNING,
                "Fellow not approved, so ignore the message.\n");
        }
        d_fellowFree(fellow);
    } else {
        d_printTimedEvent(durability, D_LEVEL_WARNING,
             "Fellow unknown so far, so ignore the message.\n");
    }
    d_networkAddressFree(sender);

    return;
}

static void
d_deleteDataListenerDeinit(
    d_deleteDataListener listener)
{
    assert(d_deleteDataListenerIsValid(listener));

    /* Stop the deleteDataListener */
    if (d_deleteDataListenerStop(listener)) {
        /* Nothing to clean for the deleteDataListener because
         * nothing was allocated.
         * Call super-deinit */
        d_readerListenerDeinit(d_readerListener(listener));
    }
}


static void
d_deleteDataListenerInit(
    d_deleteDataListener listener,
    d_subscriber subscriber)
{
    os_threadAttr attr;

    /* Do not assert the listener because the initialization
     * of the listener has not yet completed */

    assert(d_subscriberIsValid(subscriber));

    if (subscriber) {
        os_threadAttrInit(&attr);
        /* Call super-init */
        d_readerListenerInit(   d_readerListener(listener),
                                D_DELETE_DATA_LISTENER,
                                d_deleteDataListenerAction,
                                subscriber,
                                D_DELETE_DATA_TOPIC_NAME,
                                D_DELETE_DATA_TOP_NAME,
                                V_RELIABILITY_RELIABLE,
                                V_HISTORY_KEEPALL,
                                V_LENGTH_UNLIMITED,
                                attr,
                                (d_objectDeinitFunc)d_deleteDataListenerDeinit);
    }
}

d_deleteDataListener
d_deleteDataListenerNew(
    d_subscriber subscriber)
{
    d_deleteDataListener listener;

    listener = NULL;

    if (subscriber) {
        /* Allocate deleteDataListener object */
        listener = d_deleteDataListener(os_malloc(C_SIZEOF(d_deleteDataListener)));
        if (listener) {
            d_deleteDataListenerInit(listener, subscriber);
        }
    }
    return listener;
}

void
d_deleteDataListenerFree(
    d_deleteDataListener listener)
{
    assert(d_deleteDataListenerIsValid(listener)); 

    d_objectFree(d_object(listener));
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

