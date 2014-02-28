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
#include "os.h"
#include "d__sampleChainListener.h"
#include "d_sampleChainListener.h"
#include "d_readerListener.h"
#include "d__readerListener.h"
#include "d__mergeAction.h"
#include "d__group.h"
#include "d_groupLocalListener.h"
#include "d_actionQueue.h"
#include "d_sampleRequest.h"
#include "d_nameSpacesRequestListener.h"
#include "d_eventListener.h"
#include "d_durability.h"
#include "d_configuration.h"
#include "d_newGroup.h"
#include "d_table.h"
#include "d_listener.h"
#include "d_fellow.h"
#include "d_group.h"
#include "d_nameSpace.h"
#include "d_admin.h"
#include "d_readerRequest.h"
#include "d_publisher.h"
#include "d_misc.h"
#include "d_sampleChain.h"
#include "d_groupsRequest.h"
#include "d_networkAddress.h"
#include "d_message.h"
#include "v_group.h"
#include "v_groupInstance.h"
#include "v_writer.h"
#include "v_topic.h"
#include "v_partition.h"
#include "v_time.h"
#include "v_entry.h"
#include "v_entity.h"
#include "v_public.h"
#include "v_state.h"
#include "v_reader.h"
#include "sd_serializer.h"
#include "sd_serializerBigE.h"
#include "os_heap.h"
#include "os_report.h"

C_CLASS(d_resendAction);

C_STRUCT(d_resendAction){
    c_list messages;
    c_list instances;
    d_group group;
    d_sampleChainListener listener;
};

#define d_resendAction(a) ((d_resendAction)(a))


static void
updateGroupStatistics(
    d_admin admin,
    d_group group)
{
    /*update statistics*/
    d_adminStatisticsInfo info = d_adminStatisticsInfoNew();
    info->kind = D_ADMIN_STATISTICS_GROUP;

    switch(d_groupGetKind(group)){
    case D_DURABILITY_VOLATILE:
        info->groupsIncompleteVolatileDif -=1;
        info->groupsCompleteVolatileDif +=1;
        break;
    case D_DURABILITY_TRANSIENT_LOCAL:
    case D_DURABILITY_TRANSIENT:
        info->groupsIncompleteTransientDif -=1;
        info->groupsCompleteTransientDif +=1;
        break;
    case D_DURABILITY_PERSISTENT:
        info->groupsIncompletePersistentDif -=1;
        info->groupsCompletePersistentDif +=1;
        break;
    default:
        break;
    }

    d_adminUpdateStatistics(admin, info);
    d_adminStatisticsInfoFree(info);

    return;
}

struct writeBeadHelper {
    d_resendAction action;
    v_entry entry;
    c_ulong totalCount;
    c_ulong writeCount;
    c_ulong disposeCount;
    c_ulong writeDisposeCount;
    c_ulong registerCount;
    c_ulong unregisterCount;
    d_mergePolicy mergePolicy;
};

struct chainRequestHelper{
    d_chain chain;
    d_durability durability;
    d_fellow fellow;
    d_networkAddress master;
    d_name role;
};

static c_bool
findAligner(
    d_fellow fellow,
    c_voidp args)
{
    c_ulong count;
    c_bool fellowComplete = FALSE;
    c_bool fellowMightKnowGroup = FALSE;
    c_bool checkFurther = TRUE;
    c_bool fellowHasGroup = FALSE;
    struct chainRequestHelper* helper;
    d_fellowAlignStatus status = D_ALIGN_FALSE;
    d_networkAddress fellowAddress, oldAddress;
    d_name fellowRole;


    helper = (struct chainRequestHelper*)args;
    fellowAddress = d_fellowGetAddress(fellow);

    if(d_fellowGetCommunicationState(fellow) == D_COMMUNICATION_STATE_APPROVED) {
        fellowRole = d_fellowGetRole(fellow);

        if(strcmp(fellowRole, helper->role) == 0){
            fellowComplete = d_fellowIsCompleteForGroup(fellow,
                                            helper->chain->request->partition,
                                            helper->chain->request->topic,
                                            helper->chain->request->durabilityKind);

            if(fellowComplete == FALSE){
                fellowHasGroup = d_fellowHasGroup(fellow,
                        helper->chain->request->partition,
                       helper->chain->request->topic,
                       helper->chain->request->durabilityKind);

                status = d_fellowIsAlignerForGroup(fellow,
                                            helper->chain->request->partition,
                                            helper->chain->request->topic,
                                            helper->chain->request->durabilityKind);

                if((status == D_ALIGN_TRUE) || (status == D_ALIGN_UNKNOWN)){
                    fellowMightKnowGroup = TRUE;
                }
            } else {
                status = d_fellowIsAlignerForGroup(fellow,
                                            helper->chain->request->partition,
                                            helper->chain->request->topic,
                                            helper->chain->request->durabilityKind);
            }
        }
        d_printTimedEvent(helper->durability, D_LEVEL_FINEST,
                D_THREAD_SAMPLE_CHAIN_LISTENER,
                "Finding aligner: myRole='%s', fellow=%u, fellowRole='%s', fellowComplete=%d, fellowHasGroup=%d, fellowMightKnowGroup=%d and status=%d.\n",
                helper->role, fellowAddress->systemId, fellowRole, fellowComplete,
                fellowHasGroup, fellowMightKnowGroup, status);
    } else {
        d_printTimedEvent(helper->durability, D_LEVEL_FINEST,
            D_THREAD_SAMPLE_CHAIN_LISTENER,
            "Finding aligner: fellow %u not approved (yet).\n",
            fellowAddress->systemId);
    }

    if((fellowComplete == TRUE) && (helper->fellow) && (status != D_ALIGN_FALSE)){
        /*fellow complete and aligner  and old complete fellow exists.*/
        count = d_fellowRequestCountGet(helper->fellow);

        if(d_networkAddressEquals(fellowAddress, helper->master)){
            /*select the master.*/
            d_printTimedEvent(helper->durability, D_LEVEL_FINEST,
                        D_THREAD_SAMPLE_CHAIN_LISTENER,
                        "Finding aligner: select fellow %u as it is the master.\n",
                        fellowAddress->systemId);
            d_tableFree(helper->chain->fellows);
            helper->chain->fellows = d_tableNew(d_fellowCompare, d_chainFellowFree);
            d_objectKeep(d_object(fellow));
            d_tableInsert(helper->chain->fellows, fellow);
            helper->fellow = fellow;
            d_fellowRequestAdd(fellow);
            checkFurther = FALSE;
        } else if(count > d_fellowRequestCountGet(fellow)){
            /*select the one with the least open requests.*/
            oldAddress = d_fellowGetAddress(helper->fellow);
            d_printTimedEvent(helper->durability, D_LEVEL_FINEST,
                    D_THREAD_SAMPLE_CHAIN_LISTENER,
                    "Finding aligner: select fellow %u as it has %u open requests whereas current fellow %u has %u open requests.\n",
                    fellowAddress->systemId, d_fellowRequestCountGet(fellow),
                    oldAddress->systemId, count);
            d_networkAddressFree(oldAddress);
            d_tableFree(helper->chain->fellows);
            helper->chain->fellows = d_tableNew(d_fellowCompare, d_chainFellowFree);
            d_objectKeep(d_object(fellow));
            d_tableInsert(helper->chain->fellows, fellow);
            helper->fellow = fellow;
            d_fellowRequestAdd(fellow);
        } else {
            /*Do nothing here*/
            d_printTimedEvent(helper->durability, D_LEVEL_FINEST,
                D_THREAD_SAMPLE_CHAIN_LISTENER,
                "Finding aligner: do nothing for fellow %u as current selected fellow is better.\n",
                fellowAddress->systemId);
        }
    } else if((fellowComplete == TRUE) && (status != D_ALIGN_FALSE)){
        /*Fellow complete and no old complete fellow exists*/
        d_tableFree(helper->chain->fellows);
        helper->chain->fellows = d_tableNew(d_fellowCompare, d_chainFellowFree);
        d_objectKeep(d_object(fellow));
        d_tableInsert(helper->chain->fellows, fellow);
        helper->fellow = fellow;
        d_fellowRequestAdd(fellow);

        if(d_networkAddressEquals(fellowAddress, helper->master)){
            checkFurther = FALSE;
            d_printTimedEvent(helper->durability, D_LEVEL_FINEST,
                            D_THREAD_SAMPLE_CHAIN_LISTENER,
                            "Finding aligner: selecting fellow %u and not checking further.\n",
                            fellowAddress->systemId);
        } else {
            d_printTimedEvent(helper->durability, D_LEVEL_FINEST,
                                        D_THREAD_SAMPLE_CHAIN_LISTENER,
                                        "Finding aligner: selecting fellow %u and checking further.\n",
                                        fellowAddress->systemId);
        }
    } else if((fellowMightKnowGroup == TRUE) && (!helper->fellow)){
        /*No complete fellow found (yet).*/
        d_objectKeep(d_object(fellow));
        d_fellowRequestAdd(fellow);
        d_tableInsert(helper->chain->fellows, fellow);
        d_printTimedEvent(helper->durability, D_LEVEL_FINEST,
                D_THREAD_SAMPLE_CHAIN_LISTENER,
                "Finding aligner: adding fellow %u to list of candidates that is now %u long.\n",
                fellowAddress->systemId, d_tableSize(helper->chain->fellows));
    } else {
        /*Do nothing here*/
        d_printTimedEvent(helper->durability, D_LEVEL_FINEST,
                D_THREAD_SAMPLE_CHAIN_LISTENER,
                "Finding aligner: do nothing for fellow %u as it is not suitable.\n",
                fellowAddress->systemId);
    }
    d_networkAddressFree(fellowAddress);

    return checkFurther;
}

static d_resendAction
d_resendActionNew(
    d_sampleChainListener listener,
    const d_group group)
{
    d_resendAction action;

    action = d_resendAction(os_malloc(C_SIZEOF(d_resendAction)));

    if(action){
        action->messages = NULL;
        action->instances = NULL;
        action->group = d_group(d_objectKeep(d_object(group)));
        action->listener = listener;
    }
    return action;
}

static void
d_resendActionFree(
    d_resendAction action)
{
    if(action){
        if(action->messages){
            c_free(action->messages);
        }
        if(action->group){
            d_groupFree(action->group);
        }
        os_free(action);
    }
    return;
}

static c_bool
d_resendRejected(
    d_action action,
    c_bool terminate)
{
    d_resendAction resendData;
    v_message message;
    v_group group;
    v_groupInstance instance;
    v_writeResult writeResult;
    c_bool callAgain;
    d_durability durability;
    d_admin admin;
    v_resendScope resendScope = V_RESEND_NONE; /* resendScope not yet used here beyond this function */

    callAgain = TRUE;
    resendData = d_resendAction(d_actionGetArgs(action));
    group = d_groupGetKernelGroup(resendData->group);
    admin = d_listenerGetAdmin(d_listener(resendData->listener));
    durability = d_adminGetDurability(admin);

    if(terminate == FALSE){
        message = c_take(resendData->messages);
        instance = c_take(resendData->instances);
        writeResult = V_WRITE_SUCCESS;

        while(message && (writeResult == V_WRITE_SUCCESS)){
            writeResult = v_groupWrite(group, message, &instance, V_NETWORKID_ANY, &resendScope);

            if (writeResult != V_WRITE_SUCCESS) {
                c_insert(resendData->messages, message);
                c_insert(resendData->instances, instance);
            } else {
                c_free(message);
                c_free(instance);
                message = c_take(resendData->messages);
                instance = c_take(resendData->instances);
            }
        }

        if(writeResult == V_WRITE_SUCCESS){
            assert(c_count(resendData->messages) == 0);
            d_groupSetComplete(resendData->group);
            /*Update statistics*/
            updateGroupStatistics(admin, resendData->group);
            d_sampleChainListenerReportGroup(resendData->listener, resendData->group);
            d_resendActionFree(resendData);
            callAgain = FALSE;
            d_printTimedEvent(durability, D_LEVEL_FINEST,
                              D_THREAD_RESEND_QUEUE,
                              "All data for group '%s.%s' has been resent. "
                              "Group is complete now.\n",
                              v_partitionName(v_groupPartition(group)),
                              v_topicName(v_groupTopic(group)));
        } else {
            d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_RESEND_QUEUE,
                              "Resending data for group '%s.%s' failed with code %s. "
                              "Trying again later.\n",
                              v_partitionName(v_groupPartition(group)),
                              v_topicName(v_groupTopic(group)),
                              v_writeResultString(writeResult));
        }
           c_free(group);
    } else {
        d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_RESEND_QUEUE,
                         "Terminating now, but alignment for group '%s.%s' "
                         "is not complete.\n",
                         v_partitionName(v_groupPartition(group)),
                         v_topicName(v_groupTopic(group)));
        OS_REPORT_2(OS_WARNING, D_CONTEXT, 0,
                    "Terminating now, but alignment for group '%s.%s' "
                    "is not complete.\n",
                    v_partitionName(v_groupPartition(group)),
                    v_topicName(v_groupTopic(group)));
    }
    return callAgain;
}

struct findMergeHelper {
    d_chain chain;
    d_mergeAction action;
};

static c_bool
findMergeAction(
    d_mergeAction action,
    struct findMergeHelper* helper)
{
    d_chain found;

    found = d_mergeActionGetChain(action, helper->chain);

    if(found){
        helper->action = action;
    }
    return !found;
}

static d_mergeAction
d_sampleChainListenerGetMergeAction(
    d_sampleChainListener listener,
    d_chain chain)
{
    struct findMergeHelper helper;

    helper.chain = chain;
    helper.action = NULL;
    d_tableWalk(listener->mergeActions, findMergeAction, &helper);
    return helper.action;
}

d_sampleChainListener
d_sampleChainListenerNew(
    d_subscriber subscriber)
{
    d_sampleChainListener listener;

    listener = NULL;

    if(subscriber){
        listener = d_sampleChainListener(os_malloc(C_SIZEOF(d_sampleChainListener)));
        d_listener(listener)->kind = D_SAMPLE_CHAIN_LISTENER;
        d_sampleChainListenerInit(listener, subscriber);
    }
    return listener;
}

void
d_sampleChainListenerInit(
    d_sampleChainListener listener,
    d_subscriber subscriber)
{
    os_time sleepTime;
    d_admin admin;
    d_durability durability;
    d_configuration config;

    admin       = d_subscriberGetAdmin(subscriber);
    durability  = d_adminGetDurability(admin);
    config      = d_durabilityGetConfiguration(durability);

    assert(d_objectIsValid(d_object(subscriber), D_SUBSCRIBER) == TRUE);
    assert(d_objectIsValid(d_object(admin), D_ADMIN) == TRUE);
    assert(d_objectIsValid(d_object(durability), D_DURABILITY) == TRUE);
    assert(d_objectIsValid(d_object(config), D_CONFIGURATION) == TRUE);

    d_readerListenerInit(d_readerListener(listener),
                         d_sampleChainListenerAction, subscriber,
                         D_SAMPLE_CHAIN_TOPIC_NAME, D_SAMPLE_CHAIN_TOP_NAME,
                         V_RELIABILITY_RELIABLE,
                         V_HISTORY_KEEPALL,
                         V_LENGTH_UNLIMITED,
                         config->aligneeScheduling,
                         d_sampleChainListenerDeinit);

    listener->chains = d_tableNew(d_chainCompare, d_chainFree);
    assert(listener->chains);

    listener->chainsWaiting = c_iterNew(NULL);
    assert (listener->chainsWaiting);

    listener->id = 0;
    listener->fellowListener = NULL;

    sleepTime.tv_sec = 1;
    sleepTime.tv_nsec = 0;
    listener->resendQueue = d_actionQueueNew("resendQueue",sleepTime, config->aligneeScheduling);
    assert(listener->resendQueue);
    listener->unfulfilledChains = c_iterNew(NULL);
    assert(listener->unfulfilledChains);

    listener->mergeActions = d_tableNew(d_mergeActionCompare, d_mergeActionFree);
    assert(listener->mergeActions);
}

void
d_sampleChainListenerFree(
    d_sampleChainListener listener)
{
    assert(d_listenerIsValid(d_listener(listener), D_SAMPLE_CHAIN_LISTENER));

    if(listener){
        d_readerListenerFree(d_readerListener(listener));
    }
}

void
d_sampleChainListenerDeinit(
    d_object object)
{
    d_admin admin;
    d_chain chain;
    d_sampleChainListener listener;

    assert(d_listenerIsValid(d_listener(object), D_SAMPLE_CHAIN_LISTENER));

    if(object){
        listener = d_sampleChainListener(object);
        d_sampleChainListenerStop(listener);

        if(listener->fellowListener){
            admin = d_listenerGetAdmin(d_listener(listener));
            d_adminRemoveListener(admin, listener->fellowListener);
            d_eventListenerFree(listener->fellowListener);
            listener->fellowListener = NULL;
        }
        if(listener->chains){
            d_tableFree(listener->chains);
            listener->chains = NULL;
        }
        if (listener->chainsWaiting) {
            chain = d_chain(c_iterTakeFirst(listener->chainsWaiting));

            while(chain){
                d_chainFree(chain);
                chain = d_chain(c_iterTakeFirst(listener->chainsWaiting));
            }
            c_iterFree(listener->chainsWaiting);
        }
        if(listener->unfulfilledChains){
            chain = d_chain(c_iterTakeFirst(listener->unfulfilledChains));

            while(chain){
                d_chainFree(chain);
                chain = d_chain(c_iterTakeFirst(listener->unfulfilledChains));
            }
            c_iterFree(listener->unfulfilledChains);
        }
        if(listener->resendQueue){
            d_actionQueueFree(listener->resendQueue);
            listener->resendQueue = NULL;
        }
        if(listener->mergeActions){
            d_tableFree(listener->mergeActions);
            listener->mergeActions = NULL;
        }
    }
}

c_bool
d_sampleChainListenerStart(
    d_sampleChainListener listener)
{
    d_admin admin;
    c_bool result = FALSE;

    assert(d_listenerIsValid(d_listener(listener), D_SAMPLE_CHAIN_LISTENER));

    if(listener){
        if(d_listenerIsAttached(d_listener(listener)) == FALSE){
            admin = d_listenerGetAdmin(d_listener(listener));
            listener->fellowListener = d_eventListenerNew(
                                    D_FELLOW_REMOVED,
                                    d_sampleChainListenerNotifyFellowRemoved,
                                    listener);
            d_adminAddListener(admin, listener->fellowListener);
            result = d_readerListenerStart(d_readerListener(listener));
        }
    }
    return result;
}

c_bool
d_sampleChainListenerStop(
    d_sampleChainListener listener)
{
    d_admin admin;
    c_bool result = FALSE;

    assert(d_listenerIsValid(d_listener(listener), D_SAMPLE_CHAIN_LISTENER));

    if(listener){
        if(d_listenerIsAttached(d_listener(listener)) == TRUE){
            admin = d_listenerGetAdmin(d_listener(listener));
            d_adminRemoveListener(admin, listener->fellowListener);
            d_eventListenerFree(listener->fellowListener);
            listener->fellowListener = NULL;
            result = d_readerListenerStop(d_readerListener(listener));
        }
    }
    return result;
}

static void
d_sampleChainListenerAddChain(
    d_sampleChainListener listener,
    d_chain chain,
    d_networkAddress addressee)
{
    d_chain activeChain;
    d_admin admin;
    d_durability durability;
    d_publisher publisher;

    admin      = d_listenerGetAdmin(d_listener(listener));
    durability = d_adminGetDurability (admin);
    publisher  = d_adminGetPublisher(admin);

    assert (listener);
    assert (listener->chains);

    /* Set addressee */
    d_messageSetAddressee(d_message(chain->request), addressee);

    /* Check if there are chains active for group */
    activeChain = d_tableFind(listener->chains, chain);
    if (!activeChain) {
        /* If no active chain is found, insert chain */
        d_tableInsert (listener->chains, chain);
        d_publisherSampleRequestWrite(publisher, chain->request, addressee);

        d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_SAMPLE_CHAIN_LISTENER,
                         "Write samplerequest for group %s.%s to fellow %u\n",
                         chain->request->partition,
                         chain->request->topic,
                         addressee->systemId);
    }else {
        /* If active chain is found, insert chain in waitlist */
        c_iterAppend (listener->chainsWaiting, chain);

        d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_SAMPLE_CHAIN_LISTENER,
                         "Samplerequest for group %s.%s is queued because request for the same group is active.\n",
                         chain->request->partition,
                         chain->request->topic,
                         addressee->systemId);
    }
}

struct takeWaitingChainHelper
{
    d_chain search;
    d_chain result;
};

static void
d_takeWaitingChainWalk (
    void* o,
    c_iterActionArg userData)
{
    d_chain chain;
    struct takeWaitingChainHelper* helper;

    chain = d_chain(o);
    helper = (struct takeWaitingChainHelper*)userData;

    if (!helper->result) {
        if (!d_chainCompare (chain, helper->search)) {
            helper->result = chain;
        }
    }
}

static d_chain
d_sampleChainListenerRemoveChain(
    d_sampleChainListener listener,
    d_chain chain)
{
    d_durability durability;
    d_admin admin;
    d_publisher publisher;
    d_networkAddress addressee;
    d_chain result;
    struct takeWaitingChainHelper walkData;

    assert (listener);
    assert (listener->chains);
    assert (chain);

    admin = d_listenerGetAdmin (d_listener(listener));
    durability = d_adminGetDurability (admin);
    publisher = d_adminGetPublisher(admin);

    /* Remove (complete) chain from listener */
    result = d_tableRemove (listener->chains, chain);

    /* Look for other chain containing same group in chainsWaiting list */
    walkData.search = chain;
    walkData.result = NULL;
    c_iterWalk (listener->chainsWaiting, d_takeWaitingChainWalk, &walkData);

    if (walkData.result) {
        c_iterTake (listener->chainsWaiting, walkData.result);

        /* Insert chain in chains list */
        d_tableInsert (listener->chains, walkData.result);

        /* Publish request */
        addressee = d_messageGetAddressee (d_message(chain->request));
        d_publisherSampleRequestWrite(publisher, chain->request, addressee);

        d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_SAMPLE_CHAIN_LISTENER,
                         "Write delayed samplerequest for group %s.%s to fellow %u\n",
                         chain->request->partition,
                         chain->request->topic,
                         addressee->systemId);
    }

    return result;
}

void
d_sampleChainListenerReportGroup(
    d_sampleChainListener listener,
    d_group group)
{
    d_newGroup newGroup;
    d_completeness completeness;
    d_networkAddress addr;
    c_char *partition, *topic;
    d_admin admin;
    d_publisher publisher;
    d_durabilityKind kind;
    c_bool inNameSpace;
    d_durability durability;
    d_quality quality;

    if(!d_groupIsPrivate(group)){
        admin        = d_listenerGetAdmin(d_listener(listener));
        durability   = d_adminGetDurability(admin);
        publisher    = d_adminGetPublisher(admin);

        completeness = d_groupGetCompleteness(group);
        partition    = d_groupGetPartition(group);
        topic        = d_groupGetTopic(group);
        quality      = d_groupGetQuality(group);
        kind         = d_groupGetKind(group);

        inNameSpace = d_adminGroupInAlignerNS(admin, partition, topic);

        if((inNameSpace == TRUE) || (completeness != D_GROUP_COMPLETE)) {
            d_printTimedEvent(durability, D_LEVEL_FINER,
                                D_THREAD_SAMPLE_CHAIN_LISTENER,
                                "Reporting group %s.%s, kind: %u, completeness: %u\n",
                                partition, topic, kind, completeness);
            newGroup = d_newGroupNew(admin, partition, topic, kind, completeness,
                                     quality);
            addr = d_networkAddressUnaddressed();
            d_publisherNewGroupWrite(publisher, newGroup, addr);
            d_networkAddressFree(addr);
            d_newGroupFree(newGroup);
        }
        os_free(partition);
        os_free(topic);
    }
    return;
}

void
d_sampleChainListenerTryFulfillChains(
    d_sampleChainListener listener,
    d_group group)
{
    c_iter copy, leftOver;
    d_chain chain;
    d_admin admin;
    d_durability durability;
    c_ulong length;
    d_topic topic;
    d_partition partition;

    assert(d_listenerIsValid(d_listener(listener), D_SAMPLE_CHAIN_LISTENER));

    if(listener){
        d_listenerLock(d_listener(listener));
        /*Check of length must be within lock*/
        length     = c_iterLength(listener->unfulfilledChains);

        if(length > 0){
            admin      = d_listenerGetAdmin(d_listener(listener));
            durability = d_adminGetDurability(admin);
            copy       = listener->unfulfilledChains;

            d_printTimedEvent(durability, D_LEVEL_INFO,
                    D_THREAD_SAMPLE_CHAIN_LISTENER,
                    "Trying to find aligner again for %d groups.\n",
                    length);

            listener->unfulfilledChains = c_iterNew(NULL);

            if(group){
                partition = d_groupGetPartition(group);
                topic     = d_groupGetTopic(group);
            } else {
                partition = NULL;
                topic = NULL;
            }
            d_listenerUnlock(d_listener(listener));
            leftOver = c_iterNew(NULL);
            chain = d_chain(c_iterTakeFirst(copy));

            while(chain){
                if(group){
                    if((strcmp(partition, chain->request->partition) == 0)&&
                       (strcmp(topic, chain->request->topic) == 0))
                    {
                        d_sampleChainListenerInsertRequest(listener, chain, FALSE);
                    } else {
                        leftOver = c_iterInsert(leftOver, chain);
                    }
                } else {
                    d_sampleChainListenerInsertRequest(listener, chain, FALSE);
                }
                chain = d_chain(c_iterTakeFirst(copy));
            }

            if(group){
                os_free(partition);
                os_free(topic);
            }

            /*Do NOT free chains themselves, this is handled by the insertRequest function.*/
            c_iterFree(copy);

            d_listenerLock(d_listener(listener));
            chain = d_chain(c_iterTakeFirst(leftOver));

            while(chain){
                listener->unfulfilledChains = c_iterInsert(listener->unfulfilledChains, chain);
                chain = d_chain(c_iterTakeFirst(leftOver));
            }
            d_listenerUnlock(d_listener(listener));
            c_iterFree(leftOver);
            d_printTimedEvent(durability, D_LEVEL_INFO,
                    D_THREAD_SAMPLE_CHAIN_LISTENER,
                    "Still waiting for alignment of %d groups.\n",
                    c_iterLength(listener->unfulfilledChains));
        } else {
            d_listenerUnlock(d_listener(listener));
        }
    }
    return;
}

c_bool
d_sampleChainListenerNotifyFellowRemoved(
    c_ulong event,
    d_fellow fellow,
    d_nameSpace ns,
    d_group group,
    c_voidp eventUserData,
    c_voidp userData)
{
    d_durability durability;
    struct chainCleanup data;
    d_chain chain;
    d_group dgroup;
    d_sampleChainListener listener;
    d_subscriber subscriber;
    d_groupLocalListener glListener;
    c_iter requests;
    d_sampleRequest sampleRequest;
    d_readerRequest readerRequest;
    d_networkAddress source;
    d_mergeAction mergeAction;
    c_ulong chainCount;

    OS_UNUSED_ARG(ns);
    OS_UNUSED_ARG(eventUserData);
    listener = d_sampleChainListener(userData);
    assert(d_listenerIsValid(d_listener(listener), D_SAMPLE_CHAIN_LISTENER));

    if(event && group){}

    if(listener){
        data.admin               = d_listenerGetAdmin(d_listener(listener));
        data.fellow              = d_fellowGetAddress(fellow);
        data.listener            = listener;
        data.toRemove            = c_iterNew(NULL);
        durability               = d_adminGetDurability(data.admin);

        d_printTimedEvent(durability, D_LEVEL_INFO,
                D_THREAD_SAMPLE_CHAIN_LISTENER,
                "Fellow removed, checking %d requests.\n",
                d_tableSize(listener->chains));

        d_listenerLock(d_listener(listener));
        d_tableWalk(listener->chains, d_sampleChainListenerCleanupRequests, &data);
        chain = c_iterTakeFirst(data.toRemove);
        requests = c_iterNew(NULL);

        while(chain){
            /* Chain might be part of a mergeAction, if so; skip it*/
            mergeAction = d_sampleChainListenerGetMergeAction(listener, chain);

            if(mergeAction){
                d_printTimedEvent(durability, D_LEVEL_INFO,
                        D_THREAD_SAMPLE_CHAIN_LISTENER,
                        "Removing chain from merge request for group %s.%s\n",
                        chain->request->partition, chain->request->topic);
                d_mergeActionRemoveChain(mergeAction, chain);
                chainCount = d_mergeActionGetChainCount(mergeAction);

                if(chainCount == 0){
                    d_printTimedEvent(durability, D_LEVEL_INFO,
                                D_THREAD_SAMPLE_CHAIN_LISTENER,
                                "Merge action removed.\n");
                    d_tableRemove(listener->mergeActions, mergeAction);
                    d_mergeActionFree(mergeAction);
                }

                /* Remove chain from chains list */
                if(d_objectIsValid(d_object(chain), D_CHAIN) == TRUE){
                    chain = d_sampleChainListenerRemoveChain(listener, chain);
                }

            } else {
                /**
                 * Chain might be freed. This is a valid situation, because the
                 * fellow that was assumed dead by this service might still send
                 * some data after all. The sending of data can lead to the
                 * completeness of a chain. The chain will freed after the
                 * injection of its data.
                 *
                 * The chain is freed between the d_sampleChainListenerCleanupRequests
                 * and this point. Because this function has the listener lock, it
                 * is safe to check whether the chain has been freed.
                 */
                if(d_objectIsValid(d_object(chain), D_CHAIN) == TRUE){
                    chain = d_sampleChainListenerRemoveChain(listener, chain);
                }
                d_printTimedEvent(durability, D_LEVEL_INFO,
                                D_THREAD_SAMPLE_CHAIN_LISTENER,
                                "Finding new aligner for group %s.%s\n",
                                chain->request->partition, chain->request->topic);

                requests = c_iterInsert(requests, chain->request);

                if(d_sampleRequestHasCondition(chain->request)){
                    source = d_networkAddressNew(chain->request->source.systemId,
                                                 chain->request->source.localId,
                                                 chain->request->source.lifecycleId);
                    readerRequest = d_adminGetReaderRequest(data.admin, source);
                    d_networkAddressFree(source);
                    d_readerRequestRemoveChain(readerRequest, chain);
                    d_readerRequestFree(readerRequest);
                }
            }

            chain->request = NULL;
            d_chainFree(chain);
            chain = c_iterTakeFirst(data.toRemove);
        }
        d_networkAddressFree(data.fellow);
        c_iterFree(data.toRemove);
        d_listenerUnlock(d_listener(listener));

        subscriber = d_adminGetSubscriber(data.admin);
        glListener = d_subscriberGetGroupLocalListener(subscriber);

        sampleRequest = d_sampleRequest(c_iterTakeFirst(requests));

        while(sampleRequest){
            dgroup = d_adminGetLocalGroup(data.admin,
                            sampleRequest->partition,
                            sampleRequest->topic,
                            sampleRequest->durabilityKind);

            if(d_sampleRequestHasCondition(sampleRequest)){
                source = d_networkAddressNew(
                        sampleRequest->source.systemId,
                         sampleRequest->source.localId,
                         sampleRequest->source.lifecycleId);
                readerRequest = d_adminGetReaderRequest(data.admin, source);
                d_networkAddressFree(source);
                d_readerRequestAddGroup(readerRequest, dgroup);
                d_groupLocalListenerHandleAlignment(glListener, dgroup, readerRequest);
                d_readerRequestFree(readerRequest);
            } else {
                d_groupLocalListenerHandleAlignment(glListener, dgroup, NULL);
            }
            sampleRequest = d_sampleRequest(c_iterTakeFirst(requests));
        }
        c_iterFree(requests);

        d_printTimedEvent(durability, D_LEVEL_FINE,
                D_THREAD_SAMPLE_CHAIN_LISTENER,
                "%d requests left.\n", d_tableSize(listener->chains));

    }
    return TRUE;
}

c_bool
d_sampleChainListenerCleanupRequests(
    d_chain chain,
    c_voidp userData)
{
    d_chainLink link, link2;
    d_chainBead bead;
    struct chainCleanup* data;
    d_durability durability;
    d_networkAddress addressee;
    c_bool complete;
    d_fellow dummy, found;

    data       = (struct chainCleanup*)userData;
    durability = d_adminGetDurability(data->admin);
    addressee  = data->fellow;
    dummy      = d_fellowNew(addressee, D_STATE_INIT);
    found      = d_tableRemove(chain->fellows, dummy);

    d_fellowFree(dummy);

    if(found){
        if(d_tableSize(chain->fellows) == 0){
            data->toRemove = c_iterInsert(data->toRemove, chain);
        } else {
            link = d_chainLinkNew(data->fellow, 0, data->admin);
            link2 = d_tableRemove(chain->links, link);
            d_chainLinkDummyFree(link);

            if(link2){
                chain->samplesExpect -= link2->sampleCount;
                d_chainLinkFree(link2);
            }
            data->beadsToRemove = c_iterNew(NULL);
            d_tableWalk(chain->beads, d_sampleChainListenerCleanupBeads, data);
            bead = c_iterTakeFirst(data->beadsToRemove);

            while(bead){
                bead = d_tableRemove(chain->beads, bead);
                assert(bead);
                d_chainBeadFree(bead);
                bead = c_iterTakeFirst(data->beadsToRemove);
            }
            c_iterFree(data->beadsToRemove);
            complete = d_sampleChainListenerCheckChainComplete(data->listener, chain);

            if(complete == TRUE) {
                data->toRemove = c_iterInsert(data->toRemove, chain);
            } else {
                d_printTimedEvent(durability, D_LEVEL_INFO,
                    D_THREAD_SAMPLE_CHAIN_LISTENER,
                    "%u fellows left to answer request for group %s.%s.\n",
                    d_tableSize(chain->fellows), chain->request->partition,
                    chain->request->topic);
            }
        }
        d_chainFellowFree(found);
    } else {
        /* This request has not been sent to the dead fellow.*/
        d_printTimedEvent(durability, D_LEVEL_INFO,
            D_THREAD_SAMPLE_CHAIN_LISTENER,
            "Request is not meant for fellow. %u fellows left to answer " \
            "request for group %s.%s.\n",
            d_tableSize(chain->fellows), chain->request->partition,
            chain->request->topic);
    }
    return TRUE;
}

c_bool
d_sampleChainListenerCleanupBeads(
    d_chainBead bead,
    c_voidp userData)
{
    struct chainCleanup* data;

    data = (struct chainCleanup*)userData;

    if(d_networkAddressEquals(bead->sender, data->fellow)){
        assert(bead->refCount == 1);
        data->beadsToRemove = c_iterInsert(data->beadsToRemove, bead);
    } else if(d_networkAddressIsUnaddressed(bead->sender)){
        if(bead->refCount == 1){
            data->beadsToRemove = c_iterInsert(data->beadsToRemove, bead);
        } else {
            bead->refCount--;
        }
    }
    return TRUE;
}


static void
requestGroupFromMasterIfUnknown(
    d_admin admin,
    d_nameSpace nameSpace,
    d_partition partition,
    d_topic topic,
    d_durabilityKind kind)
{
    d_durability durability;
    d_publisher publisher;
    d_networkAddress master;
    d_fellow fellow;
    d_group fellowGroup;
    d_groupsRequest request;

    assert(d_objectIsValid(d_object(nameSpace), D_NAMESPACE));
    assert(d_objectIsValid(d_object(admin), D_ADMIN));

    if(admin && nameSpace && partition && topic){
        master = d_nameSpaceGetMaster(nameSpace);

        if(!d_networkAddressIsUnaddressed(master)){
            fellow = d_adminGetFellow(admin, master);

            if(fellow){
                fellowGroup = d_fellowGetGroup(fellow, partition, topic, kind);

                if(fellowGroup){
                    /*Group already known; no need to do anything*/
                    d_groupFree(fellowGroup);
                } else {
                    request    = d_groupsRequestNew(admin, partition, topic);
                    durability = d_adminGetDurability(admin);
                    publisher  = d_adminGetPublisher(admin);

                    d_messageSetAddressee(d_message(request), master);
                    d_publisherGroupsRequestWrite(publisher, request, master);

                    d_printTimedEvent(durability, D_LEVEL_FINE,
                            D_THREAD_SAMPLE_CHAIN_LISTENER,
                            "Requested info for group %s.%s from master fellow '%d'.\n",
                            partition, topic, master->systemId);
                    d_groupsRequestFree(request);
                }
                d_fellowFree(fellow);
            }
        }
        d_networkAddressFree(master);
    } else {
        OS_REPORT_4(OS_ERROR, "durability::requestGroupFromMasterIfUnknown", 0,
                    "Precondition not met: admin=0x%x, nameSpace=0x%x, partition=0x%x, topic=0x%x",
                    admin, nameSpace, partition, topic);
        assert(FALSE);
    }
    return;
}

void
d_sampleChainListenerInsertRequest(
    d_sampleChainListener listener,
    d_chain chain,
    c_bool reportGroupWhenUnfullfilled)
{
    d_admin admin;
    d_durability durability;
    d_configuration configuration;
    d_networkAddress addressee, source;
    struct chainRequestHelper data;
    d_group group;
    d_readerRequest readerRequest;
    c_bool iAmAligner;
    d_nameSpace nameSpace;
    c_bool notInitial, fulfilled;
    v_handle handle;
    d_aligneeStatistics as;

    assert(d_listenerIsValid(d_listener(listener), D_SAMPLE_CHAIN_LISTENER));
    assert(chain);

    if(listener && chain){
        admin      = d_listenerGetAdmin(d_listener(listener));
        durability = d_adminGetDurability(admin);

        d_listenerLock(d_listener(listener));
        assert(d_tableFind(listener->chains, chain) == NULL);

        configuration    = d_durabilityGetConfiguration(durability);
        nameSpace        = d_adminGetNameSpaceForGroup(
                                            admin,
                                            chain->request->partition,
                                            chain->request->topic);
        notInitial       = d_nameSpaceIsAlignmentNotInitial(nameSpace);
        data.chain       = chain;
        data.fellow      = NULL;
        data.role        = configuration->role;
        data.durability  = durability;

        if(nameSpace){
            data.master = d_nameSpaceGetMaster(nameSpace);

            d_printTimedEvent(durability, D_LEVEL_FINE,
                    D_THREAD_SAMPLE_CHAIN_LISTENER,
                    "Trying to find an aligner for group %s.%s for nameSpace %s that has master %u.\n",
                    chain->request->partition, chain->request->topic,
                    d_nameSpaceGetName(nameSpace),
                    data.master->systemId);

        } else {
            data.master = NULL;

            d_printTimedEvent(durability, D_LEVEL_FINE,
                        D_THREAD_SAMPLE_CHAIN_LISTENER,
                        "Trying to find an aligner for group %s.%s for nameSpace %s that has no master.\n",
                        chain->request->partition, chain->request->topic,
                        d_nameSpaceGetName(nameSpace));
        }

        d_adminFellowWalk(admin, findAligner, &data);

        if(d_tableSize(chain->fellows) == 0){
            d_printTimedEvent(durability, D_LEVEL_FINE,
                D_THREAD_SAMPLE_CHAIN_LISTENER,
                "Found no (potential) aligner for group %s.%s.\n",
                chain->request->partition, chain->request->topic);

            iAmAligner    = d_adminGroupInAlignerNS(
                                admin,
                                chain->request->partition,
                                chain->request->topic);

            if(iAmAligner == FALSE){
                d_printTimedEvent(durability, D_LEVEL_FINE,
                    D_THREAD_SAMPLE_CHAIN_LISTENER,
                    "Group %s.%s will not be aligned until an aligner fellow becomes available.\n",
                    chain->request->partition, chain->request->topic);

                if(reportGroupWhenUnfullfilled){
                    if(notInitial){
                        requestGroupFromMasterIfUnknown(admin, nameSpace,
                                                chain->request->partition,
                                                chain->request->topic,
                                                chain->request->durabilityKind);
                    }
                }
                listener->unfulfilledChains = c_iterInsert(listener->unfulfilledChains, chain);
            } else {
                d_printTimedEvent(durability, D_LEVEL_FINE,
                    D_THREAD_SAMPLE_CHAIN_LISTENER,
                    "Group %s.%s will be marked as complete as no (potential) other aligners exist.\n",
                    chain->request->partition, chain->request->topic);
                group = d_adminGetLocalGroup   (admin,
                                                chain->request->partition,
                                                chain->request->topic,
                                                chain->request->durabilityKind);

                d_groupSetComplete(group);
                updateGroupStatistics(admin, group);

                if(d_sampleRequestHasCondition(chain->request)){
                    source = d_networkAddressNew(
                            chain->request->source.systemId,
                            chain->request->source.localId,
                            chain->request->source.lifecycleId);
                    readerRequest = d_adminGetReaderRequest(admin, source);
                    d_networkAddressFree(source);
                    assert(readerRequest);
                    d_readerRequestRemoveChain(readerRequest, chain);
                    fulfilled = d_adminCheckReaderRequestFulfilled(admin, readerRequest);

                    if(fulfilled){
                        handle = d_readerRequestGetHandle(readerRequest);
                        d_printTimedEvent(durability, D_LEVEL_FINER,
                                D_THREAD_SAMPLE_CHAIN_LISTENER,
                                "historicalDataRequest from reader [%d, %d] fulfilled.\n",
                                 handle.index, handle.serial);
                    }

                    d_readerRequestFree(readerRequest);
                }
                d_chainFree(chain);

                if(reportGroupWhenUnfullfilled){
                    d_sampleChainListenerReportGroup(listener, group);
                }
            }
        } else {
            assert(nameSpace);
            as = d_aligneeStatisticsNew();

            if(data.fellow){
                addressee = d_fellowGetAddress(data.fellow);

                /* Add chain to listener, send samplerequest when chain becomes active */
                d_sampleChainListenerAddChain (listener, chain, addressee);

                d_printTimedEvent(durability, D_LEVEL_FINE,
                    D_THREAD_SAMPLE_CHAIN_LISTENER,
                    "Inserted new sampleRequest for group %s.%s for " \
                    "complete fellow %u.\n",
                    chain->request->partition, chain->request->topic,
                    d_message(chain->request)->addressee.systemId);
                as->aligneeRequestsSentDif = 1;
                as->aligneeRequestsOpenDif = 1;
                d_networkAddressFree(addressee);
            } else if(d_nameSpaceMasterIsMe(nameSpace, admin)){
                addressee = d_networkAddressUnaddressed();

                /* Add chain to listener, send samplerequest when chain becomes active */
                d_sampleChainListenerAddChain (listener, chain, addressee);

                d_printTimedEvent(durability, D_LEVEL_FINE,
                    D_THREAD_SAMPLE_CHAIN_LISTENER,
                    "No complete fellow found for group %s.%s, but I am the master for this one so inserted request for %u fellows.\n",
                    chain->request->partition, chain->request->topic,
                    d_tableSize(chain->fellows));
                as->aligneeRequestsSentDif = 1;
                as->aligneeRequestsOpenDif = 1;
                d_networkAddressFree(addressee);
            } else {
                d_printTimedEvent(durability, D_LEVEL_FINE,
                        D_THREAD_SAMPLE_CHAIN_LISTENER,
                        "Found %u (potential) aligners for group %s.%s.\n",
                        d_tableSize(chain->fellows),
                        chain->request->partition, chain->request->topic);

                d_tableFree(chain->fellows);
                chain->fellows = d_tableNew(d_fellowCompare, d_chainFellowFree);
                listener->unfulfilledChains = c_iterInsert(listener->unfulfilledChains, chain);

                group = d_adminGetLocalGroup   (admin,
                                                chain->request->partition,
                                                chain->request->topic,
                                                chain->request->durabilityKind);

                d_printTimedEvent(durability, D_LEVEL_INFO,
                        D_THREAD_SAMPLE_CHAIN_LISTENER,
                        "Group %s.%s will not be aligned until the master fellow becomes complete (reportGroupWhenUnfullfilled=%s, notInitial=%s).\n",
                        chain->request->partition, chain->request->topic,
                        reportGroupWhenUnfullfilled?"TRUE":"FALSE",
                        notInitial?"TRUE":"FALSE");

                if(reportGroupWhenUnfullfilled){
                    d_sampleChainListenerReportGroup(listener, group);

                    if(notInitial){
                        requestGroupFromMasterIfUnknown(admin, nameSpace,
                                                chain->request->partition,
                                                chain->request->topic,
                                                chain->request->durabilityKind);
                    }
                }
            }
            as->aligneeRequestsWaiting = c_iterLength(listener->unfulfilledChains);
            d_durabilityUpdateStatistics(durability, d_statisticsUpdateAlignee, as);
            d_aligneeStatisticsFree(as);
        }
        if(data.master){
            d_networkAddressFree(data.master);
        }
        d_listenerUnlock(d_listener(listener));
    }
}

void
d_sampleChainListenerAction(
    d_listener listener,
    d_message message)
{
    d_sampleChain sampleChain;
    d_sampleChainListener sampleChainListener;
    d_chain chain;
    d_durability durability;
    d_admin admin;
    c_bool complete;
    d_chainBead bead;
    d_chainBead inserted;
    d_chainLink link;
    v_message vmessage;
    d_fellow fellow, dummy;
    d_networkAddress sender;
    c_memoryThreshold status;
    c_base base;
    d_networkAddress myAddr;


    admin = d_listenerGetAdmin(listener);
    durability = d_adminGetDurability(admin);
    sampleChain = d_sampleChain(message);
    sampleChainListener = d_sampleChainListener(listener);
    chain = d_sampleChainListenerFindChain(sampleChainListener, sampleChain);

    sender = d_networkAddressNew(message->senderAddress.systemId,
                                 message->senderAddress.localId,
                                 message->senderAddress.lifecycleId);

    if(chain){
        dummy = d_fellowNew(sender, D_STATE_COMPLETE);
        fellow = d_tableFind(chain->fellows, dummy);

        if(!fellow){
            d_printTimedEvent(
                        durability,
                        D_LEVEL_FINE,
                        D_THREAD_SAMPLE_CHAIN_LISTENER,
                        "Received chain message for group %s.%s. from unknown fellow %u\n",
                        chain->request->partition,
                        chain->request->topic,
                        message->senderAddress.systemId);
            chain = NULL;
        }
        d_fellowFree(dummy);

    }
    /*
     * Chain might already have been removed, because this service assumed the
     * sender to be dead and chose a new fellow to align with.
     */
    if(chain){
        assert(d_objectIsValid(d_object(chain), D_CHAIN) == TRUE);

        base = d_findBase(durability);
        status = c_baseGetMemThresholdStatus(base);

        if(status == C_MEMTHRESHOLD_SERV_REACHED){
            d_printTimedEvent(durability, D_LEVEL_SEVERE,
                D_THREAD_SAMPLE_CHAIN_LISTENER,
                "Unrecoverable error: service memory threshold reached; terminating.");
            OS_REPORT(OS_ERROR, D_CONTEXT_DURABILITY, 0,
                "Unrecoverable error: service memory threshold reached; terminating.");
            d_durabilityTerminate(durability, TRUE);
        } else {
            switch(sampleChain->msgBody._d){
                case BEAD:
                    vmessage = (v_message)(sd_serializerDeserialize(
                                            chain->serializer,
                                            (sd_serializedData)(sampleChain->msgBody._u.bead.value)));

                    /* Do not insert implicit unregistrations and disposes about yourself that are
                     * aligned by fellows. Typically, this situation occurs when the fellow's splice
                     * daemon unregisters (and/or  disposes) writers on a node that is disconnected.
                     * This reflects the state that the fellow THINKS the writers on my node are not
                     * alive anymore. When connection is restored, the conclusion of the fellow that
                     * the writer is not alive should NOT be forwarded to the node that got reconnected
                     * because it is local knowledge of fellow that is not true.
                     */
                    myAddr = d_adminGetMyAddress(admin);
                    if ( ! ( v_messageStateTest(vmessage, L_IMPLICIT) &&
                             (vmessage->writerGID.systemId == myAddr->systemId) ) ) {
                        /* the message is not implicit or is not supposedly written by
                         * myself so can be inserted safely
                         */
                        bead = d_chainBeadNew(sender, vmessage, chain);
                        inserted = d_tableInsert(chain->beads, bead);
                        /* Duplicates are not inserted
                         * A message is considered a duplicate if the writer GID and the
                         * source timestamp are equal to the ones in the other message.
                         */
                        if(inserted != NULL){
                            /* Number of expected samples must be lowered.
                             */
                            chain->samplesExpect--;
                            d_chainBeadFree(bead);
                        } else {
                            chain->receivedSize += sd_serializedDataGetTotalSize((sd_serializedData)(sampleChain->msgBody._u.bead.value));
                        }
                    } else {
                        /* The message was an implicit unregister or dispose message with 
                         * a writerGID from myself, so it is a local conclusion by the 
                         * fellow about my presence. No need to add it to the bead, I know
                         * best whether I am alive or not.
                         */
                        chain->samplesExpect--;
                    }
                    d_networkAddressFree(myAddr);
                    c_free(vmessage);
                    break;
                case LINK:
                    chain->samplesExpect += sampleChain->msgBody._u.link.nrSamples;
                    link = d_chainLinkNew(sender, sampleChain->msgBody._u.link.nrSamples, admin);
                    d_tableInsert(chain->links, link);
                    d_printTimedEvent(
                            durability,
                            D_LEVEL_FINE,
                            D_THREAD_SAMPLE_CHAIN_LISTENER,
                            "Received link for group %s.%s. #links == %u\n",
                            chain->request->partition,
                            chain->request->topic,
                            d_tableSize(chain->links));
                    d_printTimedEvent(
                            durability,
                            D_LEVEL_FINE,
                            D_THREAD_SAMPLE_CHAIN_LISTENER,
                            "Fellow sent %u samples\n",
                            sampleChain->msgBody._u.link.nrSamples);
                    break;
                default:
                    OS_REPORT_1(OS_ERROR, "d_sampleChainListenerAction", 0,
                                "Illegal message discriminator value (%d) detected.",
                                sampleChain->msgBody._d);
                    assert(FALSE);
                    break;
            }

            complete = d_sampleChainListenerCheckChainComplete(sampleChainListener, chain);

            if(complete == TRUE) {
                chain = d_sampleChainListenerRemoveChain (sampleChainListener, chain);
                assert(d_objectIsValid(d_object(chain), D_CHAIN) == TRUE);
                d_chainFree(chain);
            }
        }
    }
    d_networkAddressFree(sender);
    return;
}

struct findEntryHelper {
    c_char* partition;
    c_char* topic;
    v_entry current;
    v_entry entry;
};

static c_bool
findEntryGroup(
    v_proxy proxy,
    c_voidp args)
{
    v_group vgroup;
    v_handleResult handleResult;
    c_bool result;
    c_string topicName, partitionName;

    struct findEntryHelper *entryHelper;
    entryHelper = (struct findEntryHelper*)args;
    result = TRUE;

    handleResult = v_handleClaim(proxy->source, (v_object*)(&vgroup));

    if(handleResult == V_HANDLE_OK){
        topicName = v_entityName(v_groupTopic(vgroup));
        partitionName = v_entityName(v_groupPartition(vgroup));

        if(topicName && partitionName){
            if(strcmp(entryHelper->topic, topicName) == 0){
                if(strcmp(entryHelper->partition, partitionName) == 0){
                    entryHelper->entry = entryHelper->current;
                    result = FALSE;
                }
            }
        }
        v_handleRelease(proxy->source);
    }
    return result;
}

static c_bool
findEntry(
    v_entry entry,
    c_voidp args)
{
    struct findEntryHelper *entryHelper;
    entryHelper = (struct findEntryHelper*)args;
    entryHelper->current = entry;

    return c_tableWalk(entry->groups, (c_action)findEntryGroup, args);

}

c_bool
d_sampleChainListenerCheckChainComplete(
    d_sampleChainListener listener,
    d_chain chain)
{
    d_group dgroup;
    v_group vgroup;
    c_char *partition, *topic;
    d_admin admin;
    d_durability durability;
    c_bool result = FALSE;
    os_time actionTime, sleepTime;
    d_resendAction resendData;
    d_action action;
    d_networkAddress source;
    d_readerRequest readerRequest;
    d_aligneeStatistics as;
    v_handle handle;
    c_bool fulfilled;
    struct writeBeadHelper beadHelper;
    struct findEntryHelper entryHelper;
    v_handleResult handleResult;
    v_reader vreader, *vreaderPtr;
    d_nameSpace nameSpace, myNameSpace;
    d_mergeAction mergeAction;
    d_mergeState newState;
    d_mergePolicy mergePolicy;
    c_bool success;
    c_ulong chainCount;
    d_subscriber subscriber;
    d_nameSpacesRequestListener nsrListener;
    v_writeResult writeResult;

    myNameSpace = NULL;

    assert(d_objectIsValid(d_object(chain), D_CHAIN) == TRUE);

    if(d_tableSize(chain->fellows) == d_tableSize(chain->links)){
        admin = d_listenerGetAdmin(d_listener(listener));
        durability = d_adminGetDurability(admin);

        /*All fellows have sent their link*/
        dgroup = d_adminGetLocalGroup(
                            admin,
                            chain->request->partition,
                            chain->request->topic,
                            chain->request->durabilityKind);

        if(chain->samplesExpect >= 0){
            if(((c_ulong)chain->samplesExpect) == d_tableSize(chain->beads)){
                /*All samples have been received.*/
                vgroup = d_groupGetKernelGroup(dgroup);
                partition = d_groupGetPartition(dgroup);
                topic = d_groupGetTopic(dgroup);

                d_printTimedEvent(
                    durability, D_LEVEL_INFO,
                    D_THREAD_SAMPLE_CHAIN_LISTENER,
                    "Received %u beads for group %s.%s.\n",
                    d_tableSize(chain->beads), partition, topic);

                resendData = d_resendActionNew(listener, dgroup);
                as = d_aligneeStatisticsNew();
                beadHelper.action = resendData;
                beadHelper.totalCount = 0;
                beadHelper.writeCount = 0;
                beadHelper.disposeCount = 0;
                beadHelper.writeDisposeCount = 0;
                beadHelper.registerCount = 0;
                beadHelper.unregisterCount = 0;
                beadHelper.entry = NULL;

                if(d_sampleRequestHasCondition(chain->request)){
                    source = d_networkAddressNew(
                            chain->request->source.systemId,
                            chain->request->source.localId,
                            chain->request->source.lifecycleId);
                    readerRequest = d_adminGetReaderRequest(admin, source);
                    d_networkAddressFree(source);
                    assert(readerRequest);

                    handle = d_readerRequestGetHandle(readerRequest);
                    handleResult = v_handleClaim(handle, (v_object*)(vreaderPtr = &vreader));

                    if(handleResult == V_HANDLE_OK){
                        entryHelper.partition = chain->request->partition;
                        entryHelper.topic     = chain->request->topic;
                        entryHelper.entry     = NULL;
                        v_readerWalkEntries(vreader, (c_action)findEntry, &entryHelper);

                        if(entryHelper.entry){
                            beadHelper.entry = entryHelper.entry;
                            d_tableWalk(chain->beads, d_chainBeadInject, &beadHelper);
                        } else {
                            d_printTimedEvent(
                                    durability, D_LEVEL_INFO,
                                    D_THREAD_SAMPLE_CHAIN_LISTENER,
                                    "Unable to lookup entry for reader "\
                                    "[%d, %d] for group %s.%s.\n",
                                    handle.index, handle.serial,
                                    chain->request->partition,
                                    chain->request->topic);
                        }
                        v_handleRelease(handle);
                    }

                    d_readerRequestRemoveChain(readerRequest, chain);
                    fulfilled = d_adminCheckReaderRequestFulfilled(admin, readerRequest);
                    d_resendActionFree(resendData);
                    d_readerRequestFree(readerRequest);
                    d_printTimedEvent(
                        durability, D_LEVEL_INFO,
                        D_THREAD_SAMPLE_CHAIN_LISTENER,
                        "Reader [%d, %d] has now received requested "\
                        "historical data for group %s.%s.\n",
                        handle.index, handle.serial,
                        chain->request->partition, chain->request->topic);

                    if(fulfilled){
                        d_printTimedEvent(durability, D_LEVEL_FINER,
                            D_THREAD_SAMPLE_CHAIN_LISTENER,
                            "historicalDataRequest from reader [%d, %d] fulfilled.\n",
                             handle.index, handle.serial);
                    }

                    d_printTimedEvent(
                        durability, D_LEVEL_FINEST,
                        D_THREAD_SAMPLE_CHAIN_LISTENER,
                        "Injected: TOTAL: %d, WRITE: %d, DISPOSE: %d, WRITE_DISPOSE: %d, REGISTER: %d, UNREGISTER: %d.\n",
                            beadHelper.totalCount, beadHelper.writeCount,
                            beadHelper.disposeCount, beadHelper.writeDisposeCount,
                            beadHelper.registerCount, beadHelper.unregisterCount);

                    as->aligneeSamplesTotalDif          = beadHelper.totalCount;
                    as->aligneeSamplesRegisterDif       = beadHelper.registerCount;
                    as->aligneeSamplesWriteDif          = beadHelper.writeCount;
                    as->aligneeSamplesDisposeDif        = beadHelper.disposeCount;
                    as->aligneeSamplesWriteDisposeDif   = beadHelper.writeDisposeCount;
                    as->aligneeSamplesUnregisterDif     = beadHelper.unregisterCount;
                    as->aligneeTotalSizeDif             = chain->receivedSize;
                } else {
                    /** Need to find out whether this chain is part of a
                     *  mergeAction.
                     */
                    mergeAction = d_sampleChainListenerGetMergeAction(listener, chain);

                    if(mergeAction){
                        nameSpace = d_mergeActionGetNameSpace(mergeAction);
                        myNameSpace = d_adminGetNameSpace(admin, d_nameSpaceGetName(nameSpace));
                        newState = d_mergeActionGetNewState(mergeAction);
                        mergePolicy = d_nameSpaceGetMergePolicy(myNameSpace, newState->role);
                        /* set the merge policy in the beadHelper */
                        beadHelper.mergePolicy = mergePolicy; 

                        switch(mergePolicy){
                        case D_MERGE_DELETE:
                            /* apply the DELETE merge policy */
                            d_printTimedEvent(
                                    durability, D_LEVEL_INFO,
                                    D_THREAD_SAMPLE_CHAIN_LISTENER,
                                    "Applying DELETE merge policy for group %s.%s\n",
                                    chain->request->partition, chain->request->topic);
                            /* Dispose all data before the requestTime of the 
                             * the sampleRequest that lead to the retrieval of the
                             * sampleChain.
                             */
                            writeResult = v_groupDisposeAll(vgroup, chain->request->requestTime, L_REPLACED);
                            if ( writeResult == V_WRITE_SUCCESS ) {
                                d_printTimedEvent(durability, D_LEVEL_FINE,
                                                    D_THREAD_SAMPLE_CHAIN_LISTENER,
                                                    "Samples before timestamp %d.%9.9d disposed for group %s.%s\n",
                                                    chain->request->requestTime.seconds,
                                                    chain->request->requestTime.nanoseconds,
                                                    chain->request->partition,
                                                    chain->request->topic);
                                /* Delete all historical data for the group with a writeTime up to and
                                 * including timestamp.
                                 */
                                writeResult = v_groupDeleteHistoricalData(vgroup, chain->request->requestTime);
                                if ( writeResult == V_WRITE_SUCCESS ) {
                                    d_printTimedEvent(durability, D_LEVEL_FINE,
                                                        D_THREAD_SAMPLE_CHAIN_LISTENER,
                                                        "Historical data before timestamp %d.%9.9d deleted for group %s.%s\n",
                                                        chain->request->requestTime.seconds,
                                                        chain->request->requestTime.nanoseconds,
                                                        chain->request->partition,
                                                        chain->request->topic);
                                } else {
                                    OS_REPORT_4(OS_ERROR,
                                                "d_sampleChainListenerCheckChainComplete",0,
                                                "Failed to delete historical data before timestamp %d.%9.9d for group %s.%s",
                                                chain->request->requestTime.seconds,
                                                chain->request->requestTime.nanoseconds,
                                                chain->request->partition,
                                                chain->request->topic);
                                }
                            } else {
                                OS_REPORT_2(OS_ERROR,
                                            "d_sampleChainListenerCheckChainComplete",0,
                                            "Failed to dispose all instances for group %s.%s",
                                            chain->request->partition, chain->request->topic);
                            }
                            break;
                        case D_MERGE_REPLACE:
                            /* Apply the REPLACE merge policy */
                            d_printTimedEvent(
                                    durability, D_LEVEL_INFO,
                                    D_THREAD_SAMPLE_CHAIN_LISTENER,
                                    "Applying REPLACE merge policy for group %s.%s\n",
                                    chain->request->partition, chain->request->topic);
                            /* Dispose all data before the requestTime of the 
                             * the sampleRequest that lead to the retrieval of the
                             * sampleChain.
                             */
                            writeResult = v_groupDisposeAll(vgroup, chain->request->requestTime, L_REPLACED);
                            if ( writeResult == V_WRITE_SUCCESS ) {
                                d_printTimedEvent(durability, D_LEVEL_FINE,
                                                    D_THREAD_SAMPLE_CHAIN_LISTENER,
                                                    "Samples before timestamp %d.%9.9d disposed for group %s.%s\n",
                                                    chain->request->requestTime.seconds,
                                                    chain->request->requestTime.nanoseconds,
                                                    chain->request->partition,
                                                    chain->request->topic);
                                /* Mark all reader instances with the L_REPLACED flag to indicate
                                 * that the REPLACE merge policy is about to inject historical samples.
                                 */
                                v_groupMarkReaderInstanceStates(vgroup, L_REPLACED);
                                /* Delete all historical data for the group with a writeTime up to and
                                 * including timestamp.
                                 */
                                writeResult = v_groupDeleteHistoricalData(vgroup, chain->request->requestTime);
                                if ( writeResult == V_WRITE_SUCCESS ) {
                                    d_printTimedEvent(durability, D_LEVEL_FINE,
                                                        D_THREAD_SAMPLE_CHAIN_LISTENER,
                                                        "Historical data before timestamp %d.%9.9d deleted for group %s.%s\n",
                                                        chain->request->requestTime.seconds,
                                                        chain->request->requestTime.nanoseconds,
                                                        chain->request->partition,
                                                        chain->request->topic);
                                    /* Per bead, remove the dispose message that was generated by the
                                     * v_groupDisposeAll() from the reader instance, update the reader
                                     * state, and inject historical data.
                                     */
                                     d_tableWalk(chain->beads, d_chainBeadInject, &beadHelper);
                                } else {
                                    OS_REPORT_4(OS_ERROR,
                                                "d_sampleChainListenerCheckChainComplete",0,
                                                "Failed to delete historical data before timestamp %d.%9.9d for group %s.%s",
                                                chain->request->requestTime.seconds,
                                                chain->request->requestTime.nanoseconds,
                                                chain->request->partition,
                                                chain->request->topic);
                                }
                                /* Reset the marker. */
                                v_groupUnmarkReaderInstanceStates(vgroup, L_REPLACED);
                            } else {
                                OS_REPORT_2(OS_ERROR,
                                            "d_sampleChainListenerCheckChainComplete",0,
                                            "Failed to dispose all instances for group %s.%s",
                                            chain->request->partition, chain->request->topic);
                            }
                            break;
                        case D_MERGE_MERGE:
                            /* Apply the MERGE merge policy */
                            d_printTimedEvent(
                                    durability, D_LEVEL_INFO,
                                    D_THREAD_SAMPLE_CHAIN_LISTENER,
                                    "Applying MERGE merge policy for group %s.%s\n",
                                    chain->request->partition, chain->request->topic);
                             /* inject the beads */
                            d_tableWalk(chain->beads, d_chainBeadInject, &beadHelper);
                            break;
                        case D_MERGE_IGNORE:
                            /*I shouldn't get here!*/
                            assert(FALSE);
                            /* Apply the IGNORE merge policy */
                            d_printTimedEvent(
                                    durability, D_LEVEL_INFO,
                                    D_THREAD_SAMPLE_CHAIN_LISTENER,
                                    "Applying IGNORE merge policy for group %s.%s\n",
                                    chain->request->partition, chain->request->topic);
                            break;
                        default:
                            /* A new merge policy? How exciting...*/
                            assert(FALSE);
                            break;
                        }
                    } else {
                        /* The chain is not part of a mergeAction, but it can be
                         * an initial merge action. Simply inject the beads
                         */
                        beadHelper.mergePolicy = D_MERGE_IGNORE;
                        d_tableWalk(chain->beads, d_chainBeadInject, &beadHelper);
                    }

                    /** Messages have been rejected. A resend must be scheduled.
                     */
                    if(resendData->messages != NULL){
                        actionTime = os_timeGet();
                        actionTime.tv_sec += 1;

                        sleepTime.tv_sec = 1;
                        sleepTime.tv_nsec = 0;

                        action = d_actionNew(actionTime, sleepTime, d_resendRejected, resendData);
                        d_actionQueueAdd(listener->resendQueue, action);
                    } else{
                        d_resendActionFree(resendData);
                        d_groupSetComplete(dgroup);
                        updateGroupStatistics(admin, dgroup);

                        d_printTimedEvent(
                            durability, D_LEVEL_INFO,
                            D_THREAD_SAMPLE_CHAIN_LISTENER,
                            "Group %s.%s is now complete.\n", partition, topic);
                        d_printTimedEvent(
                            durability, D_LEVEL_FINEST,
                            D_THREAD_SAMPLE_CHAIN_LISTENER,
                            "Injected: TOTAL: %d, WRITE: %d, DISPOSE: %d, WRITE_DISPOSE: %d, REGISTER: %d, UNREGISTER: %d.\n",
                                    beadHelper.totalCount, beadHelper.writeCount,
                                    beadHelper.disposeCount, beadHelper.writeDisposeCount,
                                    beadHelper.registerCount, beadHelper.unregisterCount);

                        as->aligneeSamplesTotalDif          = beadHelper.totalCount;
                        as->aligneeSamplesRegisterDif       = beadHelper.registerCount;
                        as->aligneeSamplesWriteDif          = beadHelper.writeCount;
                        as->aligneeSamplesDisposeDif        = beadHelper.disposeCount;
                        as->aligneeSamplesWriteDisposeDif   = beadHelper.writeDisposeCount;
                        as->aligneeSamplesUnregisterDif     = beadHelper.unregisterCount;
                        as->aligneeTotalSizeDif             = chain->receivedSize;
                        d_sampleChainListenerReportGroup(listener, dgroup);
                    }

                    d_adminFellowWalk(
                            admin,
                            d_sampleChainListenerRemoveGroupWithFellows,
                            dgroup);

                    if(mergeAction){
                        success = d_mergeActionRemoveChain(mergeAction, chain);
                        assert(success);

                        chainCount = d_mergeActionGetChainCount(mergeAction);

                        if(chainCount == 0){
                            subscriber = d_adminGetSubscriber(admin);
                            nsrListener = d_subscriberGetNameSpacesRequestListener(subscriber);

                            d_tableRemove(listener->mergeActions, mergeAction);
                            nameSpace = d_mergeActionGetNameSpace(mergeAction);

                            assert (myNameSpace);

                            /* If I am client (and the node from which I'm merging is my master) I need to copy the merge states from the master fellow */
                            if (!d_nameSpaceMasterIsMe (myNameSpace, admin)) {
                                d_nameSpaceReplaceMergeStates (myNameSpace, nameSpace);
                            }

                            /* Update native role state */
                            d_nameSpaceSetMergeState(myNameSpace, d_mergeActionGetNewState(mergeAction));

                            d_printTimedEvent(durability, D_LEVEL_FINE,
                                    D_THREAD_SAMPLE_CHAIN_LISTENER,
                                    "Updating state of namespace '%s' to '%d' for role '%s'\n",
                                    d_nameSpaceGetName(nameSpace),
                                    d_mergeActionGetNewState(mergeAction)->value,
                                    d_mergeActionGetNewState(mergeAction)->role);

                            /* Publish new namespace states */
                            d_nameSpacesRequestListenerReportNameSpaces(nsrListener);

                            d_mergeActionFree(mergeAction);
                        }
                    }

                    if (myNameSpace) {
                        d_nameSpaceFree (myNameSpace);
                    }
                }
                c_free(vgroup);

                as->aligneeRequestsWaiting = c_iterLength(listener->unfulfilledChains);
                as->aligneeRequestsOpenDif = -1;
                d_durabilityUpdateStatistics(durability, d_statisticsUpdateAlignee, as);
                d_aligneeStatisticsFree(as);

                result = TRUE;
                os_free(partition);
                os_free(topic);
            } else {
                d_printTimedEvent(durability, D_LEVEL_FINE,
                                    D_THREAD_SAMPLE_CHAIN_LISTENER,
                                    "Expecting %u samples, received %u so far\n",
                                    chain->samplesExpect, d_tableSize(chain->beads));
            }
        } else {
            d_printTimedEvent(durability, D_LEVEL_FINE,
                                    D_THREAD_SAMPLE_CHAIN_LISTENER,
                                    "Expecting %u samples, received %u so far\n",
                                    chain->samplesExpect, d_tableSize(chain->beads));
        }
    }
    return result;
}

c_bool
d_sampleChainListenerRemoveGroupWithFellows(
    d_fellow fellow,
    c_voidp args)
{
    d_group group;
    d_group fellowGroup;

    group = d_group(args);

    fellowGroup = d_fellowRemoveGroup(fellow, group);

    if(fellowGroup){
        d_groupFree(fellowGroup);
    }
    return TRUE;
}

/*struct mergeActionChain {
    d_chain dummy;
    d_chain found;
};

static c_bool
findMergeActionChain(
    d_mergeAction action,
    c_voidp args)
{
    struct mergeActionChain* mac;

    mac = (struct mergeActionChain*)args;
    mac->found = d_mergeActionGetChain(action, dummy);

    return !(mac->found);
}*/

d_chain
d_sampleChainListenerFindChain(
    d_sampleChainListener listener,
    d_sampleChain sampleChain)
{
    d_chain chain, dummy;
    d_sampleRequest request;
    d_timestamp stamp;
    d_admin admin;
    d_durability durability;
    c_bool forMe;
    d_networkAddress myAddr;

    assert(listener);
    assert(sampleChain);

    stamp.seconds     = 0;
    stamp.nanoseconds = 0;
    admin             = d_listenerGetAdmin(d_listener(listener));
    myAddr            = d_adminGetMyAddress(admin);
    forMe             = d_sampleChainContainsAddressee(sampleChain, myAddr);

    if(forMe){
        request = d_sampleRequestNew(admin, sampleChain->partition,
            sampleChain->topic, sampleChain->durabilityKind, stamp, FALSE, stamp, stamp);
        d_sampleRequestSetSource(request, &sampleChain->source);

        dummy = d_chainNew(NULL, request);

        chain = d_tableFind(listener->chains, dummy);

        if(!chain){
           durability = d_adminGetDurability(admin);

           d_printTimedEvent(
                   durability, D_LEVEL_FINER, D_THREAD_SAMPLE_CHAIN_LISTENER,
                   "Could not find chain for message where group is: %s.%s, kind is %u and source is %u\n",
                   sampleChain->partition, sampleChain->topic, sampleChain->durabilityKind, sampleChain->source.systemId);
        }
        /*request is also freed by d_chainFree */
        d_chainFree(dummy);
    } else {
        chain = NULL;
    }
    d_networkAddressFree(myAddr);

    return chain;
}

void
d_sampleChainListenerReportStatus(
    d_sampleChainListener listener)
{
    d_admin admin;
    d_durability durability;
    d_chain chain;
    c_long i;

    assert(d_listenerIsValid(d_listener(listener), D_SAMPLE_CHAIN_LISTENER));

    if(listener){
        admin = d_listenerGetAdmin(d_listener(listener));
        durability = d_adminGetDurability(admin);

        d_listenerLock(d_listener(listener));

        d_printTimedEvent(durability, D_LEVEL_FINEST,
                                    D_THREAD_SAMPLE_CHAIN_LISTENER,
                                    "The following groups are currently being aligned:\n");

        d_tableWalk(listener->chains, d_chainReportStatus, durability);

        d_printTimedEvent(durability, D_LEVEL_FINEST,
                            D_THREAD_SAMPLE_CHAIN_LISTENER,
                            "The following groups have no aligner yet:\n");

        for(i=0; i<c_iterLength(listener->unfulfilledChains); i++){
            chain = c_iterObject(listener->unfulfilledChains, i);
            d_printTimedEvent(durability, D_LEVEL_FINEST,
                    D_THREAD_SAMPLE_CHAIN_LISTENER,
                    "- No aligner yet for group: '%s.%s'.\n",
                    chain->request->partition,
                    chain->request->topic);

        }

        d_listenerUnlock(d_listener(listener));
    }
    return;
}

struct processChainsHelper {
    d_sampleChainListener listener;
    d_publisher publisher;
    d_networkAddress addressee;
    d_durability durability;
    d_fellow fellow;
    d_aligneeStatistics as;
};

static c_bool
processChains(
    d_chain chain,
    c_voidp args)
{
    struct processChainsHelper* helper;

    helper = (struct processChainsHelper*)args;

    helper->as->aligneeRequestsSentDif += 1;
    helper->as->aligneeRequestsOpenDif += 1;

    d_tableFree (chain->fellows);
    chain->fellows = d_tableNew(d_fellowCompare, d_chainFellowFree);
    d_objectKeep(d_object(helper->fellow));
    d_tableInsert(chain->fellows, helper->fellow);
    d_fellowRequestAdd(helper->fellow);

    d_sampleChainListenerAddChain (helper->listener, chain, helper->addressee);

    d_printTimedEvent(helper->durability, D_LEVEL_FINE,
                D_THREAD_SAMPLE_CHAIN_LISTENER,
                "Inserted new sampleRequest to merge for group %s.%s for " \
                "fellow %u.\n",
                chain->request->partition,
                chain->request->topic,
                d_message(chain->request)->addressee.systemId);

    return TRUE;
}

c_bool
d_sampleChainListenerInsertMergeAction(
    d_sampleChainListener listener,
    d_mergeAction action)
{
    d_admin admin;
    struct processChainsHelper helper;
    d_mergeAction duplicate;
    c_bool result;

    assert(d_listenerIsValid(d_listener(listener), D_SAMPLE_CHAIN_LISTENER));
    assert(action);
    result = FALSE;

    if(listener && action){
        admin = d_listenerGetAdmin(d_listener(listener));

        d_listenerLock(d_listener(listener));

        duplicate = d_mergeAction(d_tableInsert(listener->mergeActions, action));

        if(duplicate == NULL){
            helper.fellow = d_mergeActionGetFellow(action);
            helper.addressee = d_fellowGetAddress(helper.fellow);
            helper.as = d_aligneeStatisticsNew();
            helper.durability = d_adminGetDurability(admin);
            helper.publisher = d_adminGetPublisher(admin);
            helper.listener = listener;

            /* Walk over all chains in the mergeAction to send out sample requests*/
            d_mergeActionChainWalk(action, processChains, &helper);

            d_durabilityUpdateStatistics(helper.durability,
                    d_statisticsUpdateAlignee, helper.as);

            d_aligneeStatisticsFree(helper.as);
            d_networkAddressFree(helper.addressee);

            result = TRUE;
        }
        d_listenerUnlock(d_listener(listener));
    }
    return result;
}

void
d_sampleChainListenerCheckUnfulfilled(
    d_sampleChainListener listener,
    d_nameSpace nameSpace,
    d_networkAddress fellowAddress) {

    d_admin admin;
    d_chain chain;
    d_groupsRequest request;
    d_publisher publisher;
    int i;

    if (listener) {
        admin = d_listenerGetAdmin(d_listener(listener));
        publisher = d_adminGetPublisher(admin);

        d_listenerLock(d_listener(listener));

        for(i=0; i<c_iterLength(listener->unfulfilledChains); i++){
            chain = c_iterObject(listener->unfulfilledChains, i);

            if (d_nameSpaceIsIn(nameSpace, chain->request->partition, chain->request->topic)) {
                /* Re-request group from (master) fellow so we're sure to have the latest group completeness */
                request = d_groupsRequestNew(admin, chain->request->partition, chain->request->topic);

                /* Write request */
                d_publisherGroupsRequestWrite(publisher, request, fellowAddress);

                /* Free request */
                d_groupsRequestFree(request);
            }
        }

        d_listenerUnlock(d_listener(listener));
    }
}

void
d_chainFellowFree(
    d_fellow fellow)
{
    d_fellowRequestRemove(fellow);
    d_fellowFree(fellow);
}

c_bool
d_chainReportStatus(
    d_chain chain,
    d_durability durability)
{
    assert(d_objectIsValid(d_object(chain), D_CHAIN) == TRUE);

    d_printTimedEvent(durability, D_LEVEL_FINEST,
                                D_THREAD_SAMPLE_CHAIN_LISTENER,
                                "- Group: '%s.%s', #Aligners: '%d', #Beads: '%d', #Links: '%d'\n",
                                chain->request->partition,
                                chain->request->topic,
                                d_tableSize(chain->fellows),
                                d_tableSize(chain->beads),
                                d_tableSize(chain->links));

    return TRUE;
}

d_chain
d_chainNew(
    d_admin admin,
    d_sampleRequest request)
{
    d_chain chain;
    d_group group;

    assert(request);
    chain = NULL;

    if(request){
        chain = d_chain(os_malloc(C_SIZEOF(d_chain)));
        d_objectInit(d_object(chain), D_CHAIN, d_chainDeinit);
        chain->request         = request;
        chain->beads           = d_tableNew(d_chainBeadCompare, d_chainBeadFree);
        chain->links           = d_tableNew(d_chainLinkCompare, d_chainLinkFree);
        chain->fellows         = d_tableNew(d_fellowCompare, d_chainFellowFree);
        chain->samplesExpect   = 0;
        chain->receivedSize    = 0;

        if(admin){
            group = d_adminGetLocalGroup(admin,
                                     request->partition, request->topic,
                                     request->durabilityKind);
            chain->vgroup = d_groupGetKernelGroup(group);
            chain->serializer = sd_serializerBigENewTyped(v_topicMessageType(chain->vgroup->topic));
        } else {
            chain->serializer = NULL;
            chain->vgroup = NULL;
        }
    }
    return chain;
}

int
d_chainCompare(
    d_chain chain1,
    d_chain chain2)
{
    int result;

    assert(d_objectIsValid(d_object(chain1), D_CHAIN) == TRUE);
    assert(d_objectIsValid(d_object(chain2), D_CHAIN) == TRUE);
    result = 0;

    if(chain1 != chain2){
        result = 1;

        if(chain1->request && chain2->request){
            result = d_sampleRequestCompare(chain1->request, chain2->request);
        }
    }
    return result;
}

void
d_chainDeinit(
    d_object object)
{
    d_chain chain;

    assert(d_objectIsValid(object, D_CHAIN) == TRUE);

    if(object){
        chain = d_chain(object);

        if(chain->vgroup){
            c_free(chain->vgroup);
            chain->vgroup = NULL;
        }
        if(chain->beads){
            d_tableFree(chain->beads);
            chain->beads = NULL;
        }
        if(chain->links){
            d_tableFree(chain->links);
            chain->links = NULL;
        }
        if(chain->request){
            d_sampleRequestFree(chain->request);
            chain->request = NULL;
        }
        if(chain->fellows){
            d_tableFree(chain->fellows);
            chain->fellows = NULL;
        }
        if(chain->serializer){
            sd_serializerFree(chain->serializer);
            chain->serializer = NULL;
        }
    }
}

void
d_chainFree(
    d_chain chain)
{
    assert(d_objectIsValid(d_object(chain), D_CHAIN) == TRUE);

    if(chain){
        d_objectFree(d_object(chain), D_CHAIN);
    }
}

d_chainBead
d_chainBeadNew(
    d_networkAddress sender,
    v_message message,
    d_chain chain)
{
    d_chainBead chainBead = NULL;
    c_array messageKeyList;
    c_ulong i;

    assert(message);
    chainBead = d_chainBead(os_malloc(C_SIZEOF(d_chainBead)));
    memset(chainBead->keyValues, 0, sizeof(chainBead->keyValues));

    messageKeyList = v_topicMessageKeyList(v_groupTopic(chain->vgroup));
    chainBead->nrOfKeys = c_arraySize(messageKeyList);

    if (chainBead->nrOfKeys > 32) {
        OS_REPORT_1(OS_ERROR,
                    "d_sampleChainListener::d_chainBeadNew",0,
                    "too many keys %d exceeds limit of 32",
                    chainBead->nrOfKeys);
    } else {
        for (i=0;i<chainBead->nrOfKeys;i++) {
            chainBead->keyValues[i] = c_fieldValue(messageKeyList[i], message);
        }
    }

    /* In case of an unregister message, store the instance and an
     * untyped sample.
     */
    if(v_messageStateTest(message, L_UNREGISTER)){
        chainBead->message = v_groupCreateUntypedInvalidMessage(
                v_kernel(v_object(chain->vgroup)->kernel), message);
        assert(c_refCount(chainBead->message) == 1);
    } else {
        chainBead->message = c_keep(message);
    }
#ifndef _NAT_
    chainBead->message->allocTime = v_timeGet();
#endif
    chainBead->sender = d_networkAddressNew(
                                    sender->systemId,
                                    sender->localId,
                                    sender->lifecycleId);
    chainBead->refCount = 1;

    return chainBead;
}



void
d_chainBeadFree(
    d_chainBead chainBead)
{
    c_ulong i;

    assert(chainBead);

    if(chainBead){
        if(chainBead->message){
            c_free(chainBead->message);
            chainBead->message = NULL;
        }
        if(chainBead->sender){
            d_networkAddressFree(chainBead->sender);
            chainBead->sender = NULL;
        }
        for (i=0;i<chainBead->nrOfKeys;i++) {
            c_valueFreeRef(chainBead->keyValues[i]);
        }
        os_free(chainBead);
    }
}

c_bool
d_chainBeadCorrect(
    d_chainBead bead,
    c_voidp args)
{
    d_table correctedBeads;

    correctedBeads = d_table(args);
    d_tableInsert(correctedBeads, bead);

    return TRUE;
}

int
d_chainBeadCompare(
    d_chainBead bead1,
    d_chainBead bead2)
{
    int result = 0;
    c_equality eq;

    assert(bead1);
    assert(bead2);

    result = d_networkAddressCompare(bead1->sender, bead2->sender);

    if(result == 0){
        if(bead1->message == bead2->message){
            result = 0;
        } else if(bead1->message && bead2->message){
            eq = v_timeCompare(bead1->message->writeTime, bead2->message->writeTime);

            if (eq == C_EQ) {
                eq = v_gidCompare(bead1->message->writerGID,bead2->message->writerGID);

                if(eq == C_GT){
                    result = 1;
                } else if(eq == C_LT){
                    result = -1;
                } else {
                    if(v_nodeState(bead1->message) > v_nodeState(bead2->message)){
                        result = 1;
                    } else if(v_nodeState(bead1->message) < v_nodeState(bead2->message)){
                        result = -1;
                    } else {
                        result = 1;
                    }
                }
            } else if (eq == C_GT) {
                result = 1;
            } else {
                assert(eq == C_LT);
                result = -1;
            }
        } else if(!bead1->message){
            result = -1;
        } else {
            result = 1;
        }
    }
    return result;
}

int
d_chainBeadContentCompare(
    d_chainBead bead1,
    d_chainBead bead2)
{
    int result = 0;
    c_equality eq;

    assert(bead1);
    assert(bead2);


    if(bead1->message && bead2->message){
        eq = v_gidCompare(bead1->message->writerGID,bead2->message->writerGID);
        if (eq == C_EQ) {
            eq = v_timeCompare(bead1->message->writeTime, bead2->message->writeTime);
            if(eq == C_GT){
                result = 1;
            } else if(eq == C_LT){
                result = -1;
            } else {
                if(v_nodeState(bead1->message) > v_nodeState(bead2->message)){
                    result = 1;
                } else if(v_nodeState(bead1->message) < v_nodeState(bead2->message)){
                    result = -1;
                } else {
                    result = 0;
                }
            }
        } else if (eq == C_GT) {
            result = 1;
        } else {
            assert(eq == C_LT);
            result = -1;
        }
    } else if(!bead1->message && !bead2->message){
        result = 0;
    } else if(!bead1->message){
        result = -1;
    } else {
        result = 1;
    }
    return result;
}

c_bool
d_chainBeadInject(
    d_chainBead bead,
    c_voidp args)
{
    d_resendAction action;
    v_group group;
    v_groupInstance instance;
    c_bool doRegister;
    v_writeResult writeResult;
    struct writeBeadHelper *helper;
    v_resendScope resendScope = V_RESEND_NONE; /*TODO: resendScope not yet used here beyond this function */
    v_registration registration;
    v_message registerMessage;
    c_array messageKeyList;
    c_long i, nrOfKeys;

    helper = (struct writeBeadHelper*)args;
    action = d_resendAction(helper->action);
    group = d_groupGetKernelGroup(action->group);

    /* When the REPLACE policy is applied, mark the message in
     * the bead with the L_REPLACED flag.
     */
    if (helper->mergePolicy == D_MERGE_REPLACE) {
        v_stateSet(v_nodeState(bead->message), L_REPLACED);
    }

    instance = v_groupLookupInstanceAndRegistration(
                   group, bead->keyValues, bead->message->writerGID, v_gidCompare, &registration);

    /* We need to determine whether the instance still has a registration
     * for the DataWriter that wrote bead->message, because the instance
     * pipeline will have been destroyed already for existing data-readers
     * if such a registration does not exist. If it does not exist, the instance
     * handle cannot be used as an implicit registration is needed to ensure the
     * instance pipeline is reconstructed for existing data-readers.
     */
    if(instance && registration){
        c_free(registration);
        doRegister = FALSE;
    } else {
        doRegister = TRUE;
    }

    if(doRegister){
        registerMessage = v_topicMessageNew(v_groupTopic(group));

        if (registerMessage != NULL){
            v_nodeState(registerMessage) = L_REGISTER;
            registerMessage->writerGID = bead->message->writerGID;
            registerMessage->writeTime = bead->message->writeTime;
            registerMessage->qos = c_keep(bead->message->qos);

            messageKeyList = v_topicMessageKeyList(v_groupTopic(group));
            nrOfKeys = c_arraySize(messageKeyList);

            for (i=0;i<nrOfKeys;i++){
                c_fieldAssign(
                        messageKeyList[i], registerMessage, bead->keyValues[i]);
            }
            if(instance == NULL){
                v_groupWrite(
                        group, registerMessage, &instance, V_NETWORKID_ANY,
                        &resendScope);
            } else {
                v_groupWrite(
                        group, registerMessage, NULL, V_NETWORKID_ANY,
                        &resendScope);
            }
            c_free(registerMessage);
        } else {
            c_free(instance);
            instance = NULL;
            OS_REPORT(OS_ERROR, D_CONTEXT, 0, "Unable to allocate sample.");
        }
    }

    if(instance){
        resendScope = V_RESEND_NONE;

        if(helper->entry){
            writeResult = v_groupWriteNoStreamWithEntry(group, bead->message,
                    &instance, V_NETWORKID_ANY, helper->entry);
        } else {
            writeResult = v_groupWrite(group, bead->message,
                    &instance, V_NETWORKID_ANY, &resendScope);
        }

        if (writeResult != V_WRITE_SUCCESS) {
            if(!action->messages){
                action->messages = c_listNew(c_getType(bead->message));
                action->instances = c_listNew(c_getType(instance));
            }
            c_append(action->messages, bead->message);
            c_append(action->instances, instance);
        } else {
            helper->totalCount++;

            if((v_stateTest(v_nodeState(bead->message), L_WRITE)) &&
               (v_stateTest(v_nodeState(bead->message), L_DISPOSED))){
                helper->writeDisposeCount++;
            } else if(v_stateTest(v_nodeState(bead->message), L_WRITE)){
                helper->writeCount++;
            } else if(v_stateTest(v_nodeState(bead->message), L_DISPOSED)){
                helper->disposeCount++;
            } else if(v_stateTest(v_nodeState(bead->message), L_REGISTER)){
                helper->registerCount++;
            } else if(v_stateTest(v_nodeState(bead->message), L_UNREGISTER)){
                helper->unregisterCount++;
            }
        }
        c_free(instance);
    } else {
        OS_REPORT(OS_ERROR, D_CONTEXT, 0,
                "Unable to deliver aligned sample to local readers.");
    }
    c_free(group);

    return TRUE;
}

d_chainLink
d_chainLinkNew(
    d_networkAddress sender,
    c_ulong sampleCount,
    d_admin admin)
{
    d_chainLink link = NULL;

    link = d_chainLink(os_malloc(C_SIZEOF(d_chainLink)));
    link->sender = d_networkAddressNew(
                                sender->systemId,
                                sender->localId,
                                sender->lifecycleId);
    link->sampleCount = sampleCount;
    link->admin = admin;

    return link;
}

void
d_chainLinkFree(
    d_chainLink link)
{
    if(link->sender){
        d_networkAddressFree(link->sender);
        link->sender = NULL;
    }
    os_free(link);
}

void
d_chainLinkDummyFree(
 d_chainLink link)
{
    if(link->sender){
        d_networkAddressFree(link->sender);
        link->sender = NULL;
    }
    os_free(link);
}

int
d_chainLinkCompare(
    d_chainLink link1,
    d_chainLink link2)
{
    int result = 0;

    if(link1->sender->systemId > link2->sender->systemId){
       result = 1;
    } else if(link1->sender->systemId < link2->sender->systemId){
       result = -1;
    } else {
        if(link1->sender->localId > link2->sender->localId){
           result = 1;
        } else if(link1->sender->localId < link2->sender->localId){
           result = -1;
        } else {
            if(link1->sender->lifecycleId > link2->sender->lifecycleId){
               result = 1;
            } else if(link1->sender->lifecycleId < link2->sender->lifecycleId){
               result = -1;
            } else {
                result = 0;
            }
        }
    }
    return result;
}
