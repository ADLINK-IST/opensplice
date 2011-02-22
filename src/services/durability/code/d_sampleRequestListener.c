/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#include "os.h"
#include "d__sampleRequestListener.h"
#include "d_sampleRequestListener.h"
#include "d_readerListener.h"
#include "d__readerListener.h"
#include "d_sampleRequest.h"
#include "d_sampleChain.h"
#include "d_configuration.h"
#include "d_fellow.h"
#include "d_listener.h"
#include "d_admin.h"
#include "d_durability.h"
#include "d_message.h"
#include "d_networkAddress.h"
#include "d_publisher.h"
#include "d_group.h"
#include "d_misc.h"
#include "v_group.h"
#include "v_time.h"
#include "v_state.h"
#include "v_historicalDataRequest.h"
#include "sd_serializer.h"
#include "sd_serializerBigE.h"
#include "os_heap.h"
#include "d_actionQueue.h"

static d_sampleRequestHelper
d_sampleRequestListenerTakeActiveRequest(
    d_sampleRequestListener listener)
{
    d_sampleRequestHelper helper = NULL;
    os_time currentTime;
    os_compare eq;
    int i;

    assert(listener);

    if(listener){
        currentTime = os_timeGet();

        for(i=0; (i<c_iterLength(listener->requests)) && (!helper); i++){
            helper = d_sampleRequestHelper(c_iterObject(listener->requests, i));
            eq = os_timeCompare(helper->timeToAct, currentTime);

            if(eq == OS_MORE){
                helper = NULL;
            }
        }
        if(helper){
            c_iterTake(listener->requests, helper);
        }
    }
    return helper;
}

/**
 * Returns NULL if added and the inserted one otherwise
 */
static d_sampleRequestHelper
d_sampleRequestListenerAddRequest(
    d_sampleRequestListener listener,
    d_sampleRequest request)
{
    int result, i;
    d_sampleRequestHelper found = NULL;
    d_networkAddress addr, addr2;
    os_time timeToAct;
    d_durability durability;
    d_admin admin;
    d_configuration config;
    d_serviceState state;
    c_equality timeCompared;
    d_alignerStatistics stats;

    assert(d_listenerIsValid(d_listener(listener), D_SAMPLE_REQ_LISTENER));

    admin      = d_listenerGetAdmin(d_listener(listener));
    durability = d_adminGetDurability(admin);
    config     = d_durabilityGetConfiguration(durability);

    if(listener){
        stats      = d_alignerStatisticsNew();

        for(i=0; (i<c_iterLength(listener->requests)) && (!found); i++){
            found = d_sampleRequestHelper(c_iterObject(listener->requests, i));
            result = d_sampleRequestCompare(found->request, request);

            if(result == 0){
                addr = d_networkAddressNew(
                    d_message(found->request)->senderAddress.systemId,
                    d_message(found->request)->senderAddress.localId,
                    d_message(found->request)->senderAddress.lifecycleId);

                if(!d_networkAddressIsUnaddressed(addr)){
                    addr2 = d_networkAddressUnaddressed();
                    d_messageSetSenderAddress(d_message(found->request), addr2);
                    d_networkAddressFree(addr2);
                }
                /*Determine timeslot to send. If new request requires more, adjust the time*/
                timeCompared = v_timeCompare(request->endTime, found->request->endTime);

                if(timeCompared == C_GT) {
                   found->request->endTime.seconds = request->endTime.seconds;
                   found->request->endTime.nanoseconds = request->endTime.nanoseconds;
                }
                if(request->withTimeRange || found->request->withTimeRange){
                    timeCompared = v_timeCompare(request->beginTime, found->request->beginTime);

                    if(timeCompared == C_LT) {
                        found->request->beginTime.seconds     = request->beginTime.seconds;
                        found->request->beginTime.nanoseconds = request->beginTime.nanoseconds;
                        found->request->withTimeRange         = TRUE;
                    }
                }

                d_networkAddressFree(addr);

                if(c_iterLength(found->addressees) == 1){
                    stats->alignerRequestsCombinedOpenDif = 1;
                }
                addr = d_networkAddressNew(
					d_message(request)->senderAddress.systemId,
                    d_message(request)->senderAddress.localId,
                    d_message(request)->senderAddress.lifecycleId);
                found->addressees = c_iterInsert(found->addressees, addr);

                stats->alignerRequestsCombinedDif = 1;


                d_printTimedEvent(durability, D_LEVEL_FINE,
                    D_THREAD_SAMPLE_REQUEST_LISTENER,
                    "Sample request for group %s.%s from fellow %u combined with existing request.\n",
                    request->partition, request->topic,
                    d_message(request)->senderAddress.systemId);

            } else {
                found = NULL;
            }
        }

        if(!found){
            d_printTimedEvent(durability, D_LEVEL_FINE,
                    D_THREAD_SAMPLE_REQUEST_LISTENER,
                    "Sample request for group %s.%s from fellow %u added to queue.\n",
                    request->partition, request->topic,
                    d_message(request)->senderAddress.systemId);
            state = d_durabilityGetState(durability);

            if(state == D_STATE_COMPLETE){
                timeToAct = os_timeAdd(os_timeGet(), config->operationalRequestCombinePeriod);
            } else {
                timeToAct = os_timeAdd(os_timeGet(), config->initialRequestCombinePeriod);
            }
            found = d_sampleRequestHelperNew(listener, request, timeToAct);
            listener->requests = c_iterAppend(listener->requests, found);
            found = NULL;
        }

        stats->alignerRequestsOpenDif = 1;

        d_durabilityUpdateStatistics(durability, d_statisticsUpdateAligner, stats);
        d_alignerStatisticsFree(stats);
    }
    return found;
}

static void
d_sampleRequestListenerAnswer(
    d_sampleRequestHelper helper)
{
    d_admin admin;
    d_durability durability;
    d_publisher publisher;
    d_configuration config;
    d_group group;
    struct writeBeadHelper data;
    d_sampleChain sampleChain;
    c_bool sendData;
    v_group vgroup;
    c_object object;
    c_ulong i;
    d_networkAddress addr;
    d_sampleRequestListener listener;
    d_sampleRequest request;
    d_alignerStatistics stats;
    v_historicalDataRequest vrequest;
    struct v_resourcePolicy resourceLimits;

    listener        = helper->listener;
    request         = helper->request;
    admin           = d_listenerGetAdmin(d_listener(listener));
    durability      = d_adminGetDurability(admin);
    publisher       = d_adminGetPublisher(admin);
    config          = d_durabilityGetConfiguration(durability);
    group           = d_adminGetLocalGroup(admin, request->partition,
                                request->topic, request->durabilityKind);
    data.count             = 0;
    data.writeCount        = 0;
    data.disposeCount      = 0;
    data.writeDisposeCount = 0;
    data.registerCount     = 0;
    data.unregisterCount   = 0;
    data.skipCount         = 0;
    data.size              = 0;

    data.addressee  = d_networkAddressNew(
                                d_message(request)->senderAddress.systemId,
                                d_message(request)->senderAddress.localId,
                                d_message(request)->senderAddress.lifecycleId);
    sampleChain     = d_sampleChainNew(admin, request->partition, request->topic,
                                request->durabilityKind, &request->source);

    stats           = d_alignerStatisticsNew();

    sampleChain->addresseesCount = c_iterLength(helper->addressees);
    sampleChain->addressees = os_malloc(sampleChain->addresseesCount*C_SIZEOF(d_networkAddress));
    addr = d_networkAddress(sampleChain->addressees);

    for(i=0; i<sampleChain->addresseesCount; i++){
        addr[i] = *d_networkAddress(c_iterObject(helper->addressees, i));
    }

    stats->alignerRequestsAnsweredDif = (sampleChain->addresseesCount);
    stats->alignerRequestsOpenDif -= (sampleChain->addresseesCount);

    if(sampleChain->addresseesCount > 1){
        stats->alignerRequestsCombinedOpenDif = -1;
        stats->alignerRequestsCombinedAnsweredDif = 1;
    }

    if(!group){
        sendData = FALSE;
    } else {
        sendData = d_adminGroupInAlignerNS(
                                admin, request->partition,
                                request->topic, request->durabilityKind);
    }
    d_messageSetAddressee(d_message(sampleChain), data.addressee);

    if(sendData == TRUE){
        vgroup           = d_groupGetKernelGroup(group);
        data.request     = request;
        data.sampleChain = sampleChain;
        data.publisher   = publisher;
        data.serializer  = sd_serializerBigENewTyped(vgroup->topic->messageType);
        data.list        = c_iterNew(NULL);

        if(d_sampleRequestHasCondition(request)){
            d_printTimedEvent(durability, D_LEVEL_INFO,
                        D_THREAD_SAMPLE_REQUEST_LISTENER,
                        "Sending conditional set of samples for group %s.%s to fellow %u.\n",
                        request->partition, request->topic,
                        d_message(request)->senderAddress.systemId);
            data.checkTimeRange = FALSE;

            resourceLimits.max_samples              = request->maxSamples;
            resourceLimits.max_instances            = request->maxInstances;
            resourceLimits.max_samples_per_instance = request->maxSamplesPerInstance;

            vrequest = v_historicalDataRequestNew(
                            v_objectKernel(vgroup),
                            request->filter,
                            (c_char**)request->filterParams,
                            request->filterParamsCount,
                            request->beginTime,
                            request->endTime,
                            &resourceLimits);

            v_groupFlushActionWithCondition(
                    vgroup, vrequest, d_sampleRequestListenerAddList, &data);

            c_free(vrequest);
        } else {
            d_printTimedEvent(durability, D_LEVEL_INFO,
                            D_THREAD_SAMPLE_REQUEST_LISTENER,
                            "Sending all samples for group %s.%s to fellow %u.\n",
                            request->partition, request->topic,
                            d_message(request)->senderAddress.systemId);
            data.checkTimeRange = TRUE;
            v_groupFlushAction(vgroup, d_sampleRequestListenerAddList, &data);
        }
        c_free(vgroup);

        object = c_iterTakeFirst(data.list);

        while(object){
            d_sampleRequestListenerWriteBead(object, &data);
            c_free(object);
            object = c_iterTakeFirst(data.list);
        }
        c_iterFree(data.list);
        sd_serializerFree(data.serializer);
    } else {
        d_printTimedEvent(durability, D_LEVEL_FINE,
                        D_THREAD_SAMPLE_REQUEST_LISTENER,
                        "Sending no samples for group %s.%s to fellow %u.\n",
                        request->partition, request->topic,
                        d_message(request)->senderAddress.systemId);
    }
    sampleChain->msgBody._d = LINK;
    sampleChain->msgBody._u.link.nrSamples = data.count;

    if(group){
        sampleChain->msgBody._u.link.completeness = d_groupGetCompleteness(group);
    } else {
        sampleChain->msgBody._u.link.completeness = D_GROUP_UNKNOWN;
    }

    d_publisherSampleChainWrite(publisher, sampleChain, data.addressee);
    d_printTimedEvent(durability, D_LEVEL_FINE,
                                D_THREAD_SAMPLE_REQUEST_LISTENER,
                                "Sent %d samples for group %s.%s to fellow %u (%d fellows total).\n",
                                data.count, request->partition, request->topic,
                                d_message(request)->senderAddress.systemId,
                                sampleChain->addresseesCount);
    d_printTimedEvent(durability, D_LEVEL_FINEST,
                                D_THREAD_SAMPLE_REQUEST_LISTENER,
                                "WRITE: %d, DISPOSED: %d, WRITE_DISPOSED: %d, REGISTER: %d, UNREGISTER: %d (SKIPPED: %d)\n",
                                data.writeCount, data.disposeCount, data.writeDisposeCount,
                                data.registerCount, data.unregisterCount, data.skipCount);

    stats->alignerSamplesTotalDif           = data.count;
    stats->alignerSamplesRegisterDif        = data.registerCount;
    stats->alignerSamplesWriteDif           = data.writeCount;
    stats->alignerSamplesDisposeDif         = data.disposeCount;
    stats->alignerSamplesWriteDisposeDif    = data.writeDisposeCount;
    stats->alignerSamplesUnregisterDif      = data.unregisterCount;
    stats->alignerTotalSizeDif              = data.size;

    d_sampleChainFree(sampleChain);
    d_networkAddressFree(data.addressee);
    d_durabilityUpdateStatistics(durability, d_statisticsUpdateAligner, stats);
    d_alignerStatisticsFree(stats);

    return;
}


static c_bool
sendAction(
    d_action action,
    c_bool terminate)
{
    d_sampleRequestHelper helper = NULL;
    d_admin admin;
    d_durability durability;
    d_sampleRequestListener listener;
    d_serviceState state;

    listener = d_sampleRequestListener(d_actionGetArgs(action));

    if(d_objectIsValid(d_object(listener), D_LISTENER)){
        if(terminate == FALSE){
            d_listenerLock(d_listener(listener));
            admin = d_listenerGetAdmin(d_listener(listener));
            durability = d_adminGetDurability(admin);

            if(listener->mayProceed == FALSE){
                state = d_durabilityGetState(durability);

                if((state != D_STATE_INIT) &&
                   (state != D_STATE_DISCOVER_FELLOWS_GROUPS) &&
                   (state != D_STATE_DISCOVER_LOCAL_GROUPS) &&
                   (state != D_STATE_DISCOVER_PERSISTENT_SOURCE) &&
                   (state != D_STATE_INJECT_PERSISTENT) &&
                   (state != D_STATE_TERMINATING))
                {
                    listener->mayProceed = TRUE;
                }
            }

            if(listener->mayProceed == TRUE){
                helper = d_sampleRequestListenerTakeActiveRequest(listener);

                if(helper){
                    d_printTimedEvent(durability, D_LEVEL_FINE,
                            D_THREAD_SAMPLE_REQUEST_LISTENER,
                            "Now sending data for group %s.%s to fellow(s) %u.\n",
                            helper->request->partition,
                            helper->request->topic,
                            d_message(helper->request)->senderAddress.systemId);

                    d_sampleRequestListenerAnswer(helper);
                    d_sampleRequestHelperFree(helper);
                }
            }
            d_listenerUnlock(d_listener(listener));
        }
    }
    return TRUE;
}

d_sampleRequestListener
d_sampleRequestListenerNew(
    d_subscriber subscriber)
{
    d_sampleRequestListener listener;

    listener = NULL;

    if(subscriber){
        listener = d_sampleRequestListener(os_malloc(C_SIZEOF(d_sampleRequestListener)));
        d_listener(listener)->kind = D_SAMPLE_REQ_LISTENER;
        d_sampleRequestListenerInit(listener, subscriber);
    }
    return listener;
}

void
d_sampleRequestListenerInit(
    d_sampleRequestListener listener,
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

    d_readerListenerInit(   d_readerListener(listener),
                            d_sampleRequestListenerAction, subscriber,
                            D_SAMPLE_REQ_TOPIC_NAME, D_SAMPLE_REQ_TOP_NAME,
                            V_RELIABILITY_RELIABLE,
                            V_HISTORY_KEEPALL,
                            V_LENGTH_UNLIMITED,
                            config->alignerScheduling,
                            d_sampleRequestListenerDeinit);

    listener->mayProceed  = FALSE;
    listener->requests    = c_iterNew(NULL);
    sleepTime.tv_sec      = 0;
    sleepTime.tv_nsec     = 50000000; /*50ms*/
    listener->actionQueue = d_actionQueueNew("sampleRequestHandler", sleepTime, config->alignerScheduling);
    listener->actor       = d_actionNew(os_timeGet(), sleepTime, sendAction, listener);

    d_actionQueueAdd(listener->actionQueue, listener->actor);
}

void
d_sampleRequestListenerFree(
    d_sampleRequestListener listener)
{
    assert(d_listenerIsValid(d_listener(listener), D_SAMPLE_REQ_LISTENER));

    if(listener){
        d_readerListenerFree(d_readerListener(listener));
    }
}

void
d_sampleRequestListenerDeinit(
    d_object object)
{
    d_admin admin;
    d_sampleRequestHelper helper;
    c_bool removed;
    d_sampleRequestListener listener;

    assert(d_listenerIsValid(d_listener(object), D_SAMPLE_REQ_LISTENER));

    if(object){
        listener = d_sampleRequestListener(object);
        admin = d_listenerGetAdmin(d_listener(listener));
        removed = d_actionQueueRemove(listener->actionQueue, listener->actor);

        if(removed == TRUE){
            d_actionFree(listener->actor);
        }

        d_actionQueueFree(listener->actionQueue);

        d_listenerLock(d_listener(listener));
        helper = d_sampleRequestHelper(c_iterTakeFirst(listener->requests));

        while(helper){
            d_sampleRequestHelperFree(helper);
            helper = d_sampleRequestHelper(c_iterTakeFirst(listener->requests));
        }
        c_iterFree(listener->requests);
        d_listenerUnlock(d_listener(listener));
    }
}

c_bool
d_sampleRequestListenerStart(
    d_sampleRequestListener listener)
{
    assert(d_listenerIsValid(d_listener(listener), D_SAMPLE_REQ_LISTENER));

    return d_readerListenerStart(d_readerListener(listener));
}

c_bool
d_sampleRequestListenerStop(
    d_sampleRequestListener listener)
{
    return d_readerListenerStop(d_readerListener(listener));
}

void
d_sampleRequestListenerAction(
    d_listener listener,
    d_message message)
{
    d_durability durability;
    d_admin admin;
    d_sampleRequest request;
    d_sampleChain sampleChain;
    d_networkAddress addr;
    d_publisher publisher;
    d_fellow fellow;
    d_alignerStatistics stats;

    assert(d_listenerIsValid(d_listener(listener), D_SAMPLE_REQ_LISTENER));

    admin      = d_listenerGetAdmin(listener);
    durability = d_adminGetDurability(admin);
    request    = d_sampleRequest(message);
    publisher  = d_adminGetPublisher(admin);
    addr       = d_networkAddressNew(
                    message->senderAddress.systemId,
                    message->senderAddress.localId,
                    message->senderAddress.lifecycleId);
    fellow     = d_adminGetFellow(admin, addr);
    stats      = d_alignerStatisticsNew();

    stats->alignerRequestsReceivedDif = 1;

    if( (!fellow) ||
        (d_fellowGetCommunicationState(fellow) == D_COMMUNICATION_STATE_INCOMPATIBLE_STATE) ||
        (d_fellowGetCommunicationState(fellow) == D_COMMUNICATION_STATE_INCOMPATIBLE_DATA_MODEL))
    {
        d_printTimedEvent(durability, D_LEVEL_FINE,
            D_THREAD_SAMPLE_REQUEST_LISTENER,
            "Ignoring sample request for group %s.%s from fellow %u.\n",
            request->partition, request->topic,
            d_message(request)->senderAddress.systemId);

        if (fellow)  {
            sampleChain = d_sampleChainNew(admin, request->partition,
                                        request->topic, request->durabilityKind,
                                        &request->source);

            d_messageSetAddressee(d_message(sampleChain), addr);

            sampleChain->msgBody._d = LINK;
            sampleChain->msgBody._u.link.nrSamples = 0;
            sampleChain->msgBody._u.link.completeness = D_GROUP_UNKNOWN;
            d_publisherSampleChainWrite(publisher, sampleChain, addr);
            d_sampleChainFree(sampleChain);
        }

        stats->alignerRequestsIgnoredDif = 1;
    } else {
        d_printTimedEvent(durability, D_LEVEL_FINE,
            D_THREAD_SAMPLE_REQUEST_LISTENER,
            "Received sample request for group %s.%s from fellow %u, addding it to queue.\n",
            request->partition, request->topic,
            d_message(request)->senderAddress.systemId);
        d_sampleRequestListenerAddRequest(d_sampleRequestListener(listener), request);
        d_fellowFree(fellow);
    }
    d_durabilityUpdateStatistics(durability, d_statisticsUpdateAligner, stats);
    d_alignerStatisticsFree(stats);
    d_networkAddressFree(addr);

    return;
}

c_bool
d_sampleRequestListenerAddList(
    c_object object,
    c_voidp userData)
{
    v_message message;
    struct writeBeadHelper *data;
    c_bool process;
    c_equality timeCompared;
    c_object objectToAdd;

    message = (v_message)object;
    data = (struct writeBeadHelper*)userData;
    process = TRUE;

    if(data->checkTimeRange){
        timeCompared = c_timeCompare(message->writeTime, data->request->endTime);

        if(timeCompared == C_GT) {
            process = FALSE;
        } else if(data->request->withTimeRange == TRUE){
            timeCompared = c_timeCompare(message->writeTime, data->request->beginTime);

            if (timeCompared == C_LT) {
                process = FALSE;  /* produced before the time-range */
            }
        }
    }

    if(process == TRUE) {
        objectToAdd = c_keep(message);
        data->list = c_iterInsert(data->list, objectToAdd);
    } else {
        data->skipCount++;
    }
    return FALSE;
}

c_bool
d_sampleRequestListenerWriteBead(
    c_object object,
    c_voidp userData)
{
    v_message message;
    struct writeBeadHelper *data;
    c_ulong size;
    sd_serializedData serializedData;

    message = (v_message)object;
    data = (struct writeBeadHelper*)userData;

    serializedData = sd_serializerSerialize(data->serializer, message);

    if (serializedData != NULL) {
        data->count++;

        if((v_stateTest(v_nodeState(message), L_WRITE)) &&
           (v_stateTest(v_nodeState(message), L_DISPOSED))){
            data->writeDisposeCount++;
        } else if(v_stateTest(v_nodeState(message), L_WRITE)){
            data->writeCount++;
        } else if(v_stateTest(v_nodeState(message), L_DISPOSED)){
            data->disposeCount++;
        } else if(v_stateTest(v_nodeState(message), L_REGISTER)){
            data->registerCount++;
        } else if(v_stateTest(v_nodeState(message), L_UNREGISTER)){
            data->unregisterCount++;
        }

        size = sd_serializedDataGetTotalSize(serializedData);

        data->size += size;
        data->sampleChain->msgBody._d             = BEAD;
        data->sampleChain->msgBody._u.bead.size   = size;
        data->sampleChain->msgBody._u.bead.value  = os_malloc(size);
        memcpy(data->sampleChain->msgBody._u.bead.value, serializedData, size);

        d_publisherSampleChainWrite(data->publisher, data->sampleChain, data->addressee);

        os_free(data->sampleChain->msgBody._u.bead.value);
        data->sampleChain->msgBody._u.bead.value = NULL;
        sd_serializedDataFree(serializedData);
    } else {
        /*This message will be missed by the fellow.*/
        assert(FALSE);
    }
    return FALSE;
}

d_sampleRequestHelper
d_sampleRequestHelperNew(
    d_sampleRequestListener listener,
    const d_sampleRequest request,
    os_time timeToAct)
{
    d_sampleRequestHelper helper;
    d_admin admin;
    d_networkAddress addr;

    assert(request);
    assert(listener);

    admin  = d_listenerGetAdmin(d_listener(listener));
    helper = d_sampleRequestHelper(os_malloc(C_SIZEOF(d_sampleRequestHelper)));

    helper->listener  = listener;
    helper->request   = d_sampleRequestCopy(request);

    helper->timeToAct.tv_sec = timeToAct.tv_sec;
    helper->timeToAct.tv_nsec = timeToAct.tv_nsec;

    addr = d_networkAddressNew(d_message(request)->senderAddress.systemId,
                               d_message(request)->senderAddress.localId,
                               d_message(request)->senderAddress.lifecycleId);
    d_messageSetAddressee     (d_message(helper->request), addr);
    d_messageSetSenderAddress (d_message(helper->request), addr);

    helper->addressees = c_iterNew(addr);

    return helper;
}

void
d_sampleRequestHelperFree(
    d_sampleRequestHelper helper)
{
    d_networkAddress addr;
    assert(helper);

    if(helper){
        if(helper->request){
            d_sampleRequestFree(helper->request);
        }
        addr = d_networkAddress(c_iterTakeFirst(helper->addressees));

        while(addr){
            d_networkAddressFree(addr);
            addr = d_networkAddress(c_iterTakeFirst(helper->addressees));
        }
        c_iterFree(helper->addressees);
        os_free(helper);
    }
}

