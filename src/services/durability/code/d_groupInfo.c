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
#include "d_groupInfo.h"
#include "d__storeMMF.h"
#include "d_storeMMF.h"
#include "d_storeMMFKernel.h"
#include "d_store.h"
#include "u_partitionQos.h"
#include "u_partition.h"
#include "u_group.h"
#include "u_entity.h"
#include "v_durability.h"
#include "v_group.h"
#include "v_topic.h"
#include "v_message.h"
#include "v_state.h"
#include "v_time.h"
#include "c_stringSupport.h"
#include "c_metabase.h"
#include "sd_serializer.h"
#include "sd_serializerXMLMetadata.h"
#include "os.h"
#include "os_report.h"

#define d_sample(s) ((d_sample)(s))

#define d_sampleTemplate(s) ((d_sampleTemplate)(s))

#define d_instanceTemplate(s) ((d_instanceTemplate)(s))

#define d_instance(s) ((d_instance)(s))

#define d_instanceGetHead(_this) d_sample((d_instanceTemplate(_this)->newest))

#define d_instanceSetHead(_this,_sample) \
        d_instanceTemplate(_this)->newest = \
        c_keep(d_sample(_sample))

#define d_instanceGetTail(_this) \
        d_sample(d_instance(_this)->oldest)

#define d_instanceSetTail(_this,_sample) \
        d_instance(_this)->oldest = \
        d_sample(_sample)

#define d_sampleGetMessage(_this) \
        (d_sampleTemplate(_this)->message)

#define d_sampleSetMessage(_this,_message) \
        (d_sampleTemplate(_this)->message = c_keep(_message))

static void
setKernelGroup(
    v_entity entity,
    c_voidp args)
{
    d_group group;

    group = d_group(args);
    d_groupSetKernelGroup(group,v_group(entity));
}

static d_instance
d_instanceNew(
    d_groupInfo groupInfo,
    const v_groupAction action)
{
    d_instance instance;
    c_type type;
    c_long nrOfKeys, i;
    c_array messageKeyList, instanceKeyList;

    instanceKeyList = c_tableKeyList(groupInfo->instances);
    type = c_subType(groupInfo->instances);
    instance = d_instance(c_new(type));
    c_free(type);

    if(instance){
        /*
         * copy the key value of the message into the newly created instance.
         */
        messageKeyList = v_topicMessageKeyList(v_groupTopic(action->group));
        nrOfKeys = c_arraySize(messageKeyList);
        assert(nrOfKeys == c_arraySize(instanceKeyList));

        for (i=0;i<nrOfKeys;i++) {
            c_fieldClone(messageKeyList[i],action->message,
                        instanceKeyList[i], instance);
        }
        c_free(instanceKeyList);

        d_instanceSetHead(instance, NULL);
        d_instanceSetTail(instance, NULL);

        instance->messageCount = 0;
        instance->count = 0;
        instance->state = 0;

        v_stateSet(instance->state, L_EMPTY);
    } else {
        OS_REPORT(OS_ERROR,
                  "d_instanceNew",0,
                  "Failed to allocate instance.");
        assert(FALSE);
    }
    return instance;
}

static d_instance
d_groupInfoLookupInstance (
    d_groupInfo _this,
    const v_groupAction action)
{
    c_long i, nrOfKeys;
    c_value keyValues[32];
    d_instance instance;
    c_array messageKeyList;

    assert(C_TYPECHECK(action->message,v_message));

    messageKeyList = v_topicMessageKeyList(action->group->topic);
    nrOfKeys = c_arraySize(messageKeyList);

    if (nrOfKeys > 32) {
        OS_REPORT_1(OS_ERROR,
                    "d_groupInfoGetInstance",0,
                    "too many keys %d exceeds limit of 32",
                    nrOfKeys);
        instance = NULL;
    } else {
        for (i=0;i<nrOfKeys;i++) {
            keyValues[i] = c_fieldValue(messageKeyList[i],action->message);
        }
        instance = c_tableFind(_this->instances, &keyValues[0]); /* c_tableFind already performs keep. */

        for (i=0;i<nrOfKeys;i++) {
            c_valueFreeRef(keyValues[i]);
        }
    }
    return instance;
}

static d_instance
d_groupInfoGetInstance (
    d_groupInfo _this,
    const v_groupAction action,
    d_storeResult* result)
{

    d_instance instance, old;

    assert(C_TYPECHECK(action->message,v_message));

    instance = d_groupInfoLookupInstance(_this, action);

    if (instance == NULL) {
        instance = d_instanceNew(_this, action);

        if (instance) {
            old = c_tableInsert(_this->instances, instance);
            assert(old == instance);

            if(old == instance){
                *result = D_STORE_RESULT_OK;
            } else {
                *result = D_STORE_RESULT_ERROR;
            }
        } else {
            *result = D_STORE_RESULT_OUT_OF_RESOURCES;
        }
    } else {
        *result = D_STORE_RESULT_OK;
    }
    return instance;
}

d_sample
d_groupInfoSampleNew (
    d_groupInfo _this,
    d_instance instance,
    v_message msg)
{
    d_sample sample;
    v_message mmfMessage, *mmfMessagePtr;

    sample = d_sample(c_new(_this->topic->sampleType));

    if (sample) {
        mmfMessagePtr = &mmfMessage;
        c_cloneIn(_this->topic->messageType, msg, (c_voidp*)mmfMessagePtr);
        d_sampleTemplate(sample)->message = mmfMessage;
        sample->instance = instance;
        sample->older = NULL;
        sample->newer = NULL;
    } else {
        OS_REPORT(OS_ERROR,
              "d_groupInfoSampleNew",0,
              "Failed to allocate sample.");
        assert(FALSE);
    }
    return sample;
}


static d_sampleTemplate
d_instanceLastTransactionSample(
    d_instance _this,
    v_message endOfTransaction)
{
    d_sampleTemplate found = NULL;
    d_sampleTemplate lastTransactionSample;

    assert(v_stateTest(v_nodeState(endOfTransaction), L_TRANSACTION));

    lastTransactionSample = d_instanceTemplate(_this)->newest;

    while((lastTransactionSample != NULL) && (!found)){
        /* Same sequence number */
        if(lastTransactionSample->message->sequenceNumber == endOfTransaction->sequenceNumber){
            /* Same transactionId */
            if(V_MESSAGE_GET_TRANSACTION_UNIQUE_ID(lastTransactionSample->message->transactionId)
                == V_MESSAGE_GET_TRANSACTION_UNIQUE_ID(endOfTransaction->transactionId))
            {
                /* Same DataWriter */
                if(v_gidCompare(lastTransactionSample->message->writerGID,
                        endOfTransaction->writerGID) == C_EQ)
                {
                    found = lastTransactionSample;
                }
            }
        }
        lastTransactionSample = d_sampleTemplate(d_sample(lastTransactionSample)->older);
    }
    return found;
}

static d_storeResult
d_instanceInsert(
    d_instance instance,
    v_message msg,
    d_groupInfo groupInfo,
    d_sample toInsert)
{
    d_sample sample;
    d_sample oldest;
    d_sample ptr;
    d_sampleTemplate lastTransactionSample;
    v_state state;
    c_equality equality;
    v_topicQos topicQos;
    v_message mmfMessage;

    assert(C_TYPECHECK(instance, d_instance));
    assert(C_TYPECHECK(msg, v_message));
    assert(C_TYPECHECK(groupInfo, d_groupInfo));

    if (msg == NULL) {
        return D_STORE_RESULT_ILL_PARAM;
    }
    topicQos = groupInfo->topic->qos;

    if(toInsert){
        sample = toInsert;
        sample->instance = instance;
    } else {
        sample = d_groupInfoSampleNew(groupInfo, instance, msg);
    }

    if (!sample) {
        return D_STORE_RESULT_OUT_OF_RESOURCES;
    }

    if (v_stateTest(instance->state, L_EMPTY)) {
        assert(d_instanceGetHead(instance) == NULL);
        assert(d_instanceGetTail(instance) == NULL);
        assert(instance->count == 0);
        d_instanceSetHead(instance,sample);
        v_stateClear(instance->state, L_EMPTY);
    } else {
        assert(d_instanceGetHead(instance) != NULL);
        assert(d_instanceGetTail(instance) != NULL);
        assert(instance->count != 0);

        /* This message marks the end of a transaction. It needs to replace
         * the last message of the transaction if it still exists in this
         * instance. If not, it will be stored as a 'normal' message while
         * taking into account the history depth.
         */
        if(v_stateTest(v_nodeState(msg), L_TRANSACTION)){
            /* Look up the last sample in the transaction */
            lastTransactionSample = d_instanceLastTransactionSample(
                    instance, msg);

            /* If it is found, replace the message and return success.*/
            if(lastTransactionSample){
                c_free(lastTransactionSample->message);
                c_cloneIn(groupInfo->topic->messageType, msg, (c_voidp*)&(mmfMessage));
                lastTransactionSample->message = mmfMessage;
                c_free(sample);

                return D_STORE_RESULT_OK;
            }
            /* if it is not found continue business as usual. */
        }
        if(topicQos->orderby.kind == V_ORDERBY_RECEPTIONTIME){
            sample->older = d_instanceGetHead(instance);
            d_instanceSetHead(instance, sample);
        } else {
            ptr = d_instanceGetHead(instance);
            equality = v_timeCompare(msg->writeTime,
                    d_sampleGetMessage(ptr)->writeTime);

            if (equality == C_LT) {
                while (ptr->older != NULL) {
                    equality = v_timeCompare(msg->writeTime,
                                 d_sampleGetMessage(ptr->older)->writeTime);

                    if (equality != C_LT) {
                        break;
                    }
                    ptr = ptr->older;
                }
                sample->newer = ptr;
                sample->older = ptr->older;
                ptr->older = c_keep(sample);
            } else {
                sample->older = d_instanceGetHead(instance);
                d_instanceSetHead(instance,sample);
            }
        }
    }
    assert(c_refCount(sample) == 2);

    if (sample->older != NULL) {
        sample->older->newer = sample;
    } else {
        d_instanceSetTail(instance,sample);
    }
    sample->instance = instance;
    state = v_nodeState(msg);

    if (v_stateTest(state,L_DISPOSED)) {
        instance->count++;
    }
    if (v_stateTest(state, L_WRITE)) {
        if (topicQos->history.kind == V_HISTORY_KEEPALL
            || instance->messageCount < topicQos->history.depth) {
            instance->count++;
            instance->messageCount++;
        } else {
            ptr = NULL;
            oldest = instance->oldest;

            while (!v_messageStateTest(d_sampleGetMessage(oldest),L_WRITE)) {
                instance->count--;
                ptr = oldest;
                oldest = oldest->newer;
            }
            if (v_messageStateTest(d_sampleGetMessage(oldest),L_DISPOSED)) {
                instance->count--;
            }
            ptr = oldest;
            oldest = oldest->newer;

            while (!v_messageStateTest(d_sampleGetMessage(oldest),L_WRITE)) {
                instance->count--;
                ptr = oldest;
                oldest = oldest->newer;
            }
            d_instanceSetTail(instance,oldest);
            oldest->older = NULL;
            ptr->newer = NULL;

            while(ptr){
                oldest = ptr->older;
                ptr->older = NULL;
                c_free(ptr);
                ptr = oldest;
            }
        }
    }
    if (v_messageStateTest(d_sampleGetMessage(d_instanceGetHead(instance)), L_DISPOSED)) {
        v_stateSet(instance->state, L_DISPOSED);
    } else {
        if (v_stateTest(instance->state, L_DISPOSED)) {
            v_stateClear(instance->state, L_DISPOSED);
        }
    }
    c_free(sample);

    assert((instance->count == 0) == (d_instanceGetTail(instance) == NULL));
    assert((instance->count == 0) == (d_instanceGetHead(instance) == NULL));

    return D_STORE_RESULT_OK;
}

static d_storeResult
d_instanceRemove (
    d_instance instance,
    v_message message)
{
    d_sample current, sample;
    v_state state;
    v_message m;
    d_storeResult result;

    if ((instance != NULL) && (message != NULL)) {
        sample = NULL;
        current = d_instanceGetHead(instance);

        while(!sample && current){
            m = d_sampleGetMessage(current);

            if(c_timeCompare(m->writeTime, message->writeTime) == C_EQ){
                if(v_gidCompare(m->writerGID, message->writerGID) == C_EQ){
                    if(m->sequenceNumber == message->sequenceNumber){
                        sample = current;
                    }
                }
            }
            if(!sample){
                current = current->older;
            }
        }

        if(sample){
            if (sample->newer != NULL) {
                d_sample(sample->newer)->older = c_keep(sample->older);
            } else {
                assert(d_instanceGetHead(instance) == sample);
                d_instanceSetHead(instance,sample->older);
            }
            if (sample->older != NULL) {
                d_sample(sample->older)->newer = sample->newer;
            } else {
                assert(d_instanceGetTail(instance) == sample);
                d_instanceSetTail(instance,sample->newer);
            }
            state = v_nodeState(d_sampleGetMessage(sample));

            if (v_stateTest(state, L_WRITE)) {
                instance->count--;
                instance->messageCount--;
            }
            if (v_stateTest(state, L_DISPOSED)) {
                instance->count--;
            }
            c_free(sample);

            if (instance->oldest == NULL) {
                v_stateSet(instance->state, L_EMPTY);
            }
        }
        assert((instance->count == 0) == (d_instanceGetTail(instance) == NULL));
        assert((instance->count == 0) == (d_instanceGetHead(instance) == NULL));
        result = D_STORE_RESULT_OK;
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

static d_storeResult
d_instanceRemoveHistoricalData(
    d_instance instance,
    const v_groupAction until)
{
    d_sample ptr, current;
    d_storeResult result;
    c_equality equality;
    v_state state;

    ptr = d_instanceGetTail(instance);

    if(ptr){
        equality = v_timeCompare(until->actionTime,
                d_sampleGetMessage(ptr)->writeTime);

        while ((equality != C_LT) && ptr) {
            current = ptr;
            ptr = ptr->newer;
            current->newer = NULL;
            d_instanceSetTail(instance, ptr);

            state = v_nodeState(d_sampleGetMessage(current));

            if (v_stateTest(state, L_WRITE)) {
                instance->count--;
                instance->messageCount--;
            }
            if (v_stateTest(state, L_DISPOSED)) {
                instance->count--;
            }

            if(ptr){
                ptr->older = NULL;
                equality = v_timeCompare(until->actionTime,
                        d_sampleGetMessage(ptr)->writeTime);
            } else {
                d_instanceSetHead(instance, NULL);
            }
            c_free(current);
        }

        if (instance->oldest == NULL) {
            v_stateSet(instance->state, L_EMPTY);
        }
        assert((instance->count == 0) == (d_instanceGetTail(instance) == NULL));
        assert((instance->count == 0) == (d_instanceGetHead(instance) == NULL));

        result = D_STORE_RESULT_OK;
    } else {
        result = D_STORE_RESULT_OK;
    }
    return result;
}

static c_bool
removeHistoricalData(
    c_object o,
    c_voidp arg)
{
    d_storeResult result;
    c_bool success;

    assert(C_TYPECHECK(o,d_instance));

    result = d_instanceRemoveHistoricalData(d_instance(o), v_groupAction(arg));

    if(result == D_STORE_RESULT_OK){
        success = TRUE;
    } else {
        success = FALSE;
    }
    return success;

}

static v_message
createUnregisterMessage(
    v_group group,
    v_message message)
{
    c_array            messageKeyList;
    c_long                i, nrOfKeys;
    v_message            unregisterMessage;

    assert(!v_stateTest(v_nodeState(message), L_UNREGISTER));

    /* Create new message objec */
    unregisterMessage = v_topicMessageNew
                                (group->topic);

    /* Copy keyvalues to unregistermessage */
    messageKeyList = v_topicMessageKeyList(v_groupTopic(group));
    nrOfKeys = c_arraySize(messageKeyList);
    for (i=0;i<nrOfKeys;i++) {
        c_fieldAssign (messageKeyList[i],
                unregisterMessage,
                c_fieldValue(messageKeyList[i],message));
    }

    /* Set instance & writer GID */
    unregisterMessage->writerGID =
            message->writerGID;
    unregisterMessage->writerInstanceGID =
            message->writerInstanceGID;

    /* Copy messageQos */
    c_keep (message->qos);
    unregisterMessage->qos = message->qos;

    /* Set nodestate to unregister */
    v_nodeState(unregisterMessage) = L_UNREGISTER;

    unregisterMessage->writeTime = v_timeGet();
#ifndef _NAT_
    unregisterMessage->allocTime = unregisterMessage->writeTime;
#endif

    return unregisterMessage;
}

static v_message
findUnregisterMessage(
    c_iter msgs,
    v_message tmpl)
{
    c_long i;
    v_message result = NULL;

    if(msgs && tmpl){
        for(i=0; i<c_iterLength(msgs) && !result; i++){
            result = (v_message)(c_iterObject(msgs, i));

            if(v_gidCompare(result->writerGID, tmpl->writerGID) != C_EQ){
                result = NULL;
            }
        }
    }
    return result;
}


struct d_instanceInjectArg {
    v_group vgroup;
    c_type messageType;
    d_store store;
    d_storeResult result;
};

static c_bool
d_instanceInject(
   c_object o,
   c_voidp arg)
{
    d_instance instance;
    d_sample sample;
    v_message message, *messagePtr, storeMessage, unregisterMsg;
    struct d_instanceInjectArg* inj;
    v_writeResult wr;
    os_time oneSec;
    c_iter unregisterMessagesToInject;
    c_bool success;

    assert(o != NULL);
    assert(C_TYPECHECK(o, d_instance));

    instance = d_instance(o);
    inj = (struct d_instanceInjectArg*)(arg);
    unregisterMessagesToInject = c_iterNew(NULL);

    sample = d_sample(instance->oldest);

    while ((sample != NULL) && (inj->result == D_STORE_RESULT_OK)) {
        storeMessage = d_sampleGetMessage(sample);

        /* copy message */
        messagePtr = &message;
        c_cloneIn(inj->messageType, storeMessage, (c_voidp*)messagePtr);

        /* inject message */
        wr = v_groupWriteNoStream(inj->vgroup, message, NULL, V_NETWORKID_ANY);
        oneSec.tv_sec  = 1;
        oneSec.tv_nsec = 0;

        while(wr == V_WRITE_REJECTED){
            wr = v_groupWriteNoStream(inj->vgroup, message, NULL, V_NETWORKID_ANY);
            os_nanoSleep(oneSec);
        }

        if((wr != V_WRITE_SUCCESS) &&
           (wr != V_WRITE_REGISTERED) &&
           (wr != V_WRITE_UNREGISTERED)) {
            OS_REPORT_1(OS_ERROR, D_CONTEXT, 0,
                "Unable to write persistent data to group. (result: '%d')\n",
                wr);
            inj->result = D_STORE_RESULT_ERROR;
        } else {
            /* If a sample is written or registered, add unregister action */
            unregisterMsg = findUnregisterMessage(unregisterMessagesToInject, message);

            if(!unregisterMsg){
                unregisterMsg = createUnregisterMessage (inj->vgroup, message);

                /* Add the newly created unregister message to the list of extra messages to inject */
                unregisterMessagesToInject = c_iterAppend (
                        unregisterMessagesToInject, unregisterMsg);
            }
            /* Set valid sequence number */
            if (message->sequenceNumber >= unregisterMsg->sequenceNumber) {
                unregisterMsg->sequenceNumber = message->sequenceNumber + 1;
            }
        }
        sample = sample->newer;
        c_free(message);
    }
    if(inj->result == D_STORE_RESULT_OK){
        oneSec.tv_sec  = 1;
        oneSec.tv_nsec = 0;
        /* inject the extra unregister messages */
        unregisterMsg = v_message(c_iterTakeFirst(unregisterMessagesToInject));

        while (unregisterMsg) {
            wr = v_groupWriteNoStream(inj->vgroup, unregisterMsg, NULL, V_NETWORKID_ANY);

            while(wr == V_WRITE_REJECTED){
                wr = v_groupWriteNoStream(inj->vgroup, unregisterMsg, NULL, V_NETWORKID_ANY);
                os_nanoSleep(oneSec);
            }
            c_free(unregisterMsg);
            unregisterMsg = v_message(c_iterTakeFirst(unregisterMessagesToInject));
        }
        c_iterFree(unregisterMessagesToInject);
    }

    if(inj->result == D_STORE_RESULT_OK){
        success = TRUE;
    } else {
        success = FALSE;
    }
    return success;
}

d_groupInfo
d_groupInfoNew (
    const d_storeMMFKernel kernel,
    const d_topicInfo topic,
    const d_group dgroup)
{
    d_groupInfo group;
    c_base base;
    c_char* partition;
    c_type instanceType, groupInfoType;
    c_char *keyExpr;

    if(kernel && topic && dgroup){
        base = c_getBase(kernel);
        groupInfoType = c_resolve(base,"durabilityModule2::d_groupInfo");
        group = d_groupInfo(c_new(groupInfoType));
        c_free(groupInfoType);

        if (group) {
            group->kernel = kernel; /* Unmanaged pointer */
            group->topic = c_keep(topic);

            partition = d_groupGetPartition(dgroup);
            group->partition = c_stringNew(base, partition);
            os_free(partition);

            group->quality = d_groupGetQuality(dgroup);
            group->completeness = d_groupGetCompleteness(dgroup);

            instanceType = d_topicInfoGetInstanceType(topic);
            keyExpr = d_topicInfoGetInstanceKeyExpr(topic);
            group->instances = c_tableNew(instanceType, keyExpr);
            c_free(keyExpr);
            c_free(instanceType);
        } else {
            OS_REPORT(OS_ERROR,
                      "d_groupInfoNew",0,
                      "Failed to allocate d_groupInfo.");
            assert(FALSE);
            group = NULL;
        }
    } else {
        OS_REPORT(OS_ERROR,
                  "d_groupInfoNew",0,
                  "Illegal constructor parameter.");
        group = NULL;
    }
    return group;
}

d_storeResult
d_groupInfoInject(
    d_groupInfo _this,
    const d_store store,
    u_participant participant,
    d_group* group)
{
    d_storeResult result;
    u_group ugroup;
    u_partition upartition;
    v_partitionQos partitionQos;
    v_duration timeout;
    c_string name;

    if(_this && store && participant){
        result = d_topicInfoInject(_this->topic, store, participant);

        if(result == D_STORE_RESULT_OK){
           partitionQos = u_partitionQosNew(NULL);

           if(partitionQos) {
               d_storeReport(store, D_LEVEL_FINE, "PartitionQoS created.\n");

               upartition = u_partitionNew(participant, _this->partition,
                       partitionQos);

               if(upartition) {
                   name = d_topicInfoGetName(_this->topic);
                   d_storeReport(store, D_LEVEL_FINE,
                           "Partition %s created.\n", _this->partition);

                   timeout.seconds = 0;
                   timeout.nanoseconds = 0;
                   ugroup = u_groupNew(participant, _this->partition, name,
                           timeout);

                   if(ugroup) {
                       d_storeReport(store,
                               D_LEVEL_INFO, "Group %s.%s created.\n",
                               _this->partition, name);

                       *group = d_groupNew(_this->partition, name,
                               D_DURABILITY_PERSISTENT, _this->completeness,
                               _this->quality);

                       u_entityAction(u_entity(ugroup), setKernelGroup, *group);
                       u_entityFree(u_entity(ugroup));
                       result = D_STORE_RESULT_OK;
                   } else {
                       result = D_STORE_RESULT_OUT_OF_RESOURCES;
                       d_storeReport(store, D_LEVEL_SEVERE,
                               "Group %s.%s could NOT be created.\n",
                               _this->partition, name);
                       OS_REPORT_2(OS_ERROR, "d_groupInfoInject",
                               (os_int32)result,
                               "Group %s.%s could NOT be created.\n",
                               _this->partition, name);
                   }
                   c_free(name);
                   u_partitionFree(upartition);
               } else {
                   result = D_STORE_RESULT_OUT_OF_RESOURCES;
                   d_storeReport(store, D_LEVEL_SEVERE,
                           "Partition %s could NOT be created.\n",
                           _this->partition);
                   OS_REPORT_1(OS_ERROR, "d_groupInfoInject", (os_int32)result,
                           "Partition %s could NOT be created.\n",
                           _this->partition);
               }
               u_partitionQosFree(partitionQos);
           } else {
               result = D_STORE_RESULT_OUT_OF_RESOURCES;
               d_storeReport(store, D_LEVEL_SEVERE,
                       "PartitionQos could NOT be created.\n");
               OS_REPORT(OS_ERROR, "d_groupInfoInject", (os_int32)result,
                       "PartitionQos could NOT be created.\n");
           }
        }
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_groupInfoWrite(
    d_groupInfo _this,
    const d_store store,
    const v_groupAction action,
    d_sample sample)
{
    d_storeResult result;
    d_instance instance;

    OS_UNUSED_ARG(store);
    if(_this && action && action->message){
        instance = d_groupInfoGetInstance(_this, action, &result);

        if(instance){
            result = d_instanceInsert(instance, action->message, _this, sample);
            c_free(instance);
        }
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_groupInfoDispose(
    d_groupInfo _this,
    const d_store store,
    const v_groupAction action,
    d_sample sample)
{
    d_storeResult result;
    d_instance instance;

    OS_UNUSED_ARG(store);
    if(_this && action && action->message){
        instance = d_groupInfoGetInstance(_this, action, &result);

        if(instance){
            result = d_instanceInsert(instance, action->message, _this, sample);
            c_free(instance);

            if(result == D_STORE_RESULT_OK){
                _this->quality = action->actionTime;
            }
        }
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_groupInfoExpungeInstance(
    d_groupInfo _this,
    const d_store store,
    const v_groupAction action)
{
    d_storeResult result;
    d_instance instance, removed;

    OS_UNUSED_ARG(store);
    if(_this && action && action->message){
        instance = d_groupInfoLookupInstance(_this, action);

        if(instance){
            removed = c_remove(_this->instances, instance, NULL, NULL);
            assert(removed == instance);

            if(removed == instance){
                _this->quality = action->actionTime;
                result = D_STORE_RESULT_OK;
            } else {
                result = D_STORE_RESULT_MUTILATED;
            }
            c_free(removed);
            c_free(instance);
        } else {
            result = D_STORE_RESULT_OK;
        }
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_groupInfoExpungeSample(
    d_groupInfo _this,
    const d_store store,
    const v_groupAction action)
{
    d_storeResult result;
    d_instance instance;

    OS_UNUSED_ARG(store);
    if(_this && action && action->message){
        instance = d_groupInfoGetInstance(_this, action, &result);

        if(instance){
            result = d_instanceRemove(instance, action->message);
            c_free(instance);

            if(result == D_STORE_RESULT_OK){
                _this->quality = action->actionTime;
            }
        }
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_groupInfoDataInject(
    d_groupInfo _this,
    const d_store store,
    d_group group)
{
    d_storeResult result;
    struct d_instanceInjectArg inject;
    c_type mmfMessageType;
    c_char* typeName;

    OS_UNUSED_ARG(store);
    if(_this && group){
        inject.vgroup = d_groupGetKernelGroup(group);

        mmfMessageType = d_topicInfoGetMessageType(_this->topic);
        typeName = c_metaScopedName(c_metaObject(mmfMessageType));

        inject.messageType = c_type(c_metaResolveType(
                c_metaObject(c_getBase(inject.vgroup)), typeName));

        if(inject.messageType){
            inject.result = D_STORE_RESULT_OK;

            c_tableWalk(_this->instances, d_instanceInject, &inject);

            c_free(inject.messageType);
            result = inject.result;
        } else {
            result = D_STORE_RESULT_PRECONDITION_NOT_MET;
        }
        c_free(inject.vgroup);
        c_free(mmfMessageType);
        os_free(typeName);
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_groupInfoBackup(
    d_groupInfo _this,
    const d_store store,
    d_groupInfo* backup)
{
    c_base base;
    c_type groupInfoType, instanceType;
    c_string keyExpr;
    d_storeResult result;

    assert(_this);
    assert(backup);

    OS_UNUSED_ARG(store);
    base = c_getBase(_this->kernel);
    groupInfoType = c_resolve(base,"durabilityModule2::d_groupInfo");
    *backup = d_groupInfo(c_new(groupInfoType));
    c_free(groupInfoType);

    if (*backup) {
        (*backup)->kernel = _this->kernel; /* Unmanaged pointer */
        (*backup)->topic = c_keep(_this->topic);
        (*backup)->partition = c_keep(_this->partition);
        (*backup)->quality = _this->quality;
        (*backup)->completeness = _this->completeness;

        instanceType = d_topicInfoGetInstanceType(_this->topic);
        keyExpr = d_topicInfoGetInstanceKeyExpr(_this->topic);

        (*backup)->instances = _this->instances; /*Here's the trick */
        _this->instances = c_tableNew(instanceType, keyExpr);

        c_free(keyExpr);
        c_free(instanceType);

        if(_this->instances){
            result = D_STORE_RESULT_OK;
        } else {
            _this->instances = (*backup)->instances;

            (*backup)->instances = NULL;
            c_free(*backup);
            *backup = NULL;

            result = D_STORE_RESULT_OUT_OF_RESOURCES;
        }
    } else {
        assert(FALSE);
        result = D_STORE_RESULT_OUT_OF_RESOURCES;
    }
    return result;
}

d_storeResult
d_groupInfoDeleteHistoricalData(
    d_groupInfo _this,
    const d_store store,
    const v_groupAction action)
{
    c_bool success;
    d_storeResult result;

    assert(_this);
    assert(action);

    OS_UNUSED_ARG(store);
    if(_this && action){
        success = c_tableWalk(_this->instances, removeHistoricalData, action);

        if(success){
            result = D_STORE_RESULT_OK;
        } else {
            result = D_STORE_RESULT_ERROR;
        }
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}
