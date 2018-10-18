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
#include "vortex_os.h"
#include "d__sampleRequestListener.h"
#include "d__configuration.h"
#include "d__fellow.h"
#include "d__listener.h"
#include "d__admin.h"
#include "d__durability.h"
#include "d__actionQueue.h"
#include "d__readerListener.h"
#include "d__publisher.h"
#include "d__misc.h"
#include "d__group.h"
#include "d__thread.h"
#include "d__nameSpace.h"
#include "d_sampleRequest.h"
#include "d_sampleChain.h"
#include "d_message.h"
#include "d_networkAddress.h"
#include "v_group.h"
#include "v_topic.h"
#include "v_builtin.h"
#include "v_time.h"
#include "v_state.h"
#include "v_historicalDataRequest.h"
#include "v_groupInstance.h"
#include "v_entity.h"
#include "v_message.h" /* v_message() */
#include "v_topic.h"
#include "v_group.h"
#include "kernelModuleI.h"
#include "sd_serializer.h"
#include "sd_serializerBigE.h"
#include "os_heap.h"
#include "os_report.h"
#include "c_misc.h"
#include "v_messageExt.h"
#include "d__groupHash.h"

/**
 * Macro that checks the d_sampleRequestListener validity.
 * Because d_sampleRequestListener is a concrete class typechecking is required.
 */
#define d_sampleRequestListenerIsValid(_this)   \
        d_listenerIsValidKind(d_listener(_this), D_SAMPLE_REQ_LISTENER)

/**
 * \brief The d_sampleRequestListener cast macro.
 *
 * This macro casts an object to a d_sampleRequestListener object.
 */
#define d_sampleRequestListener(_this) ((d_sampleRequestListener)(_this))

/**
 * \brief The d_sampleRequestHelper cast macro.
 *
 * This macro casts an object to a d_sampleRequestHelper object.
 */
#define d_sampleRequestHelper(h) ((d_sampleRequestHelper)(h))

C_STRUCT(d_sampleRequestListener){
    C_EXTENDS(d_readerListener);
    c_bool mayProceed;
    d_action actor;
    d_actionQueue actionQueue;
    c_iter requests;
};

struct writeBeadHelper{
    c_iter list;

    d_sampleRequest request;
    c_ulong count;
    c_ulong writeCount;
    c_ulong disposeCount;
    c_ulong writeDisposeCount;
    c_ulong registerCount;
    c_ulong unregisterCount;
    c_ulong eotCount;
    c_ulong skipCount;
    c_ulong size;
    c_type xmsgType;
    sd_serializer serializer;      /* serializer for "normal" topic data */
    sd_serializer serializerEOT;   /* serializer for EOTs */
    d_sampleChain sampleChain;
    d_publisher publisher;
    d_networkAddress addressee;
    c_bool checkTimeRange;
    c_bool isDCPSTopicBuiltinGroup;
};

C_STRUCT(d_sampleRequestHelper){
    d_sampleRequestListener listener;
    d_sampleRequest request;
    c_iter addressees;
    os_timeM timeToAct;
};

static int
isBuiltinTopicSample(
    v_topicInfoTemplate tmpl)
{
    const struct v_topicInfo* data;
    data = &tmpl->userData;
    assert(strlen(data->name) <= OS_MAX_INTEGER(unsigned));
    return (d_inBuiltinTopicNames(data->name, (unsigned) strlen(data->name)) == 0) ? 0 : 1;
}


static void d_traceSampleRequestHelper (d_sampleRequestHelper request)
{
    c_iterIter iter;
    d_networkAddress addr;
    char str[256], tmp_str[256];

    str[0] = '\0';
    tmp_str[0] = '\0';
    iter = c_iterIterGet(request->addressees);
    while ((addr = (d_networkAddress)c_iterNext(&iter)) != NULL) {
        strncpy(tmp_str, str, sizeof(str));
        tmp_str[ (sizeof(tmp_str) - 1) ] = '\0'; /* ensure null-terminated */
        if (strncmp(tmp_str, "\0", sizeof(tmp_str)) == 0) {
            snprintf(str, sizeof(str), "%u", addr->systemId);
        } else {
            snprintf(str, sizeof(str), "%s,%u", tmp_str, addr->systemId);
        }
    }
    d_trace(D_TRACE_COMBINE_REQUESTS, " - request %p for %s.%s, addressees [%s] timeToAct=%"PA_PRItime".\n",
            (void *)request, request->request->partition, request->request->topic, str, OS_TIMEM_PRINT(request->timeToAct));
}

static void d_traceSampleRequestQueue (c_iter requests)
{
    c_iterIter iter;
    d_sampleRequestHelper srh;

    /* print the contents of the queue before */
    d_trace(D_TRACE_COMBINE_REQUESTS, "Sample request queue contents\n");
    iter = c_iterIterGet(requests);
    while ((srh = d_sampleRequestHelper(c_iterNext(&iter))) != NULL) {
        d_traceSampleRequestHelper(srh);
    }
}

static d_sampleRequestHelper
d_sampleRequestHelperNew(
    d_sampleRequestListener listener,
    const d_sampleRequest request,
    os_timeM timeToAct)
{
    d_sampleRequestHelper helper;
    d_networkAddress addr;

    assert(request);
    assert(d_sampleRequestListenerIsValid(listener));

    /* Allocate sampleRequestHelper */
    helper = d_sampleRequestHelper(os_malloc(C_SIZEOF(d_sampleRequestHelper)));
    helper->listener  = listener;
    helper->request   = d_sampleRequestCopy(request, TRUE);
    helper->timeToAct = timeToAct;
    addr = d_networkAddressNew(d_message(request)->senderAddress.systemId,
                               d_message(request)->senderAddress.localId,
                               d_message(request)->senderAddress.lifecycleId);
    d_messageSetAddressee     (d_message(helper->request), addr);
    helper->addressees = c_iterNew(addr);
    return helper;
}

static c_bool
request_already_pending (d_sampleRequestHelper found, d_sampleRequest request)
{
    d_networkAddress addressee, senderAddress;
    c_iterIter addresseeIter;
    c_bool result;

    assert(found);
    assert(request);

    senderAddress = d_networkAddressNew(d_message(request)->senderAddress.systemId, d_message(request)->senderAddress.localId, d_message(request)->senderAddress.lifecycleId);
    /* Check if the senderAddress of the request is already present in the list of
     * addressees. If so, there is not need to target the same recipient
     * twice.
     */
    addresseeIter = c_iterIterGet(found->addressees);
    while ((addressee = d_networkAddress(c_iterNext(&addresseeIter))) != NULL) {
        if (d_networkAddressEquals(addressee, senderAddress)) {
            break;
        }
    }
    result = (addressee != NULL);
    d_networkAddressFree(senderAddress);
    return result;
}

/**
 * \brief Add a incoming sampleRequest to the table of pending sampleRequests.
 *
 * If there already exists a sampleRequest for the same partition/topic and
 * from the same source then the requests are combined to save bandwidth.
 * The answer to the combined request is send to (0,0,0).
 *
 * @return Returns NULL if added and the inserted one otherwise.
 */
static d_sampleRequestHelper
d_sampleRequestListenerAddRequest(
    d_sampleRequestListener listener,
    d_sampleRequest request)
{
    d_sampleRequestHelper found = NULL;
    d_networkAddress addr;
    os_timeM timeToAct;
    d_durability durability;
    d_admin admin;
    d_configuration config;
    d_serviceState state;
    os_compare eq;
    d_alignerStatistics stats;
    c_iterIter requestIter;
    c_bool foundExistingRequest;
    os_timeW found_beginTime, found_endTime, beginTime, endTime;

    assert(d_sampleRequestListenerIsValid(listener));

    admin      = d_listenerGetAdmin(d_listener(listener));
    durability = d_adminGetDurability(admin);
    config     = d_durabilityGetConfiguration(durability);

    d_traceSampleRequestQueue(listener->requests);

    if (listener) {
        stats      = d_alignerStatisticsNew();
        foundExistingRequest = FALSE;
        /* Start at the head of the listener->request and traverse the list
         * until a pending request has been found for the same partition/topic
         * and the same source. If so, update the pending request. If no
         * pending request is found then add a new request to the list.
         */
        requestIter = c_iterIterGet(listener->requests);
        found = d_sampleRequestHelper(c_iterNext(&requestIter));

        while ((foundExistingRequest == FALSE) && (found != NULL)) {
            if (d_sampleRequestCanCombine(found->request, request)) {
                /* Only combine if the found request is not already targeted
                 * to the sender of the request */
                if (!request_already_pending(found, request)) {
                    /* Combine the request in a single response to
                     * save bandwidth. Make sure to adjust time slot
                     */
                    d_timestampToTimeW(&found_beginTime, &found->request->beginTime, IS_Y2038READY(found->request));
                    d_timestampToTimeW(&found_endTime, &found->request->endTime, IS_Y2038READY(found->request));
                    d_timestampToTimeW(&beginTime, &request->beginTime, IS_Y2038READY(request));
                    d_timestampToTimeW(&endTime, &request->endTime, IS_Y2038READY(request));

                    eq = os_timeWCompare(endTime, found_endTime);
                    if (eq == OS_MORE) {
                       found->request->endTime = request->endTime;
                    }
                    if (request->withTimeRange || found->request->withTimeRange) {
                        eq = os_timeWCompare(beginTime, found_beginTime);
                        if(eq == OS_LESS) {
                            found->request->beginTime = request->beginTime;
                            found->request->withTimeRange = TRUE;
                        }
                    }
                    /* Add the origin of the sampleRequest as the addressee */
                    if (c_iterLength(found->addressees) == 1) {
                        stats->alignerRequestsCombinedOpenDif = 1;
                    }
                    addr = d_networkAddressNew(
                        d_message(request)->senderAddress.systemId,
                        d_message(request)->senderAddress.localId,
                        d_message(request)->senderAddress.lifecycleId);

                    found->addressees = c_iterInsert(found->addressees, addr);
                    stats->alignerRequestsCombinedDif = 1;
                    d_printTimedEvent(durability, D_LEVEL_FINER,
                        "Sample request for group %s.%s from fellow %u combined with existing request.\n",
                        request->partition, request->topic, d_message(request)->senderAddress.systemId);
                } else {
                    d_printTimedEvent(durability, D_LEVEL_FINER,
                        "Sample request for group %s.%s from fellow %u not combined because there already exists a pending request from this fellow.\n",
                        request->partition, request->topic, d_message(request)->senderAddress.systemId);
                }
                foundExistingRequest = TRUE;
            } else {
                found = d_sampleRequestHelper(c_iterNext(&requestIter));
            }
        } /* while */

        if (!found) {
            assert(foundExistingRequest == FALSE);
            /* No pending sampleRequest already exists, add this one. */
            d_printTimedEvent(durability, D_LEVEL_FINEST,
                    "Sample request for group %s.%s from fellow %u added to queue.\n",
                    request->partition, request->topic, d_message(request)->senderAddress.systemId);
            state = d_durabilityGetState(durability);
            if (state == D_STATE_COMPLETE) {
                timeToAct = os_timeMAdd(os_timeMGet(), config->operationalRequestCombinePeriod);
            } else {
                timeToAct = os_timeMAdd(os_timeMGet(), config->initialRequestCombinePeriod);
            }
            /* Make sure that DCPSTopics are the first to be aligned when
             * it is time to act.
             */
            if ((strcmp(request->partition, V_BUILTIN_PARTITION) == 0) &&
                (strcmp(request->topic, V_TOPICINFO_NAME) == 0)) {
                d_sampleRequestHelper first = d_sampleRequestHelper(c_iterObject(listener->requests, 0));
                if (first) {
                    timeToAct = first->timeToAct;
                }
                /* Add to the front */
                found = d_sampleRequestHelperNew(listener, request, timeToAct);
                listener->requests = c_iterInsert(listener->requests, found);
            } else {
                /* Add to the back */
                found = d_sampleRequestHelperNew(listener, request, timeToAct);
                listener->requests = c_iterAppend(listener->requests, found);
            }
            found = NULL;
        }
        stats->alignerRequestsOpenDif = 1;
        d_durabilityUpdateStatistics(durability, d_statisticsUpdateAligner, stats);
        d_alignerStatisticsFree(stats);
    }

    d_traceSampleRequestQueue(listener->requests);

    return found;
}


/* When flushing the group, the kernel may return both v_messages and v_registrations. When
 * a v_registration is flushed this can indicate a register or an unregister which is
 * specified by the flushType. The flushed objects are stored in a temporary list which is
 * eventually used to align the samples to other nodes. When the function interpreting this list
 * encounters a v_registration it must be able to tell whether this was a register or an unregister.
 * That information is stored by using this struct.
 */
typedef struct d_sampleRequestAlignData {
    c_object object;            /* v_message or v_registration */
    v_groupFlushType flushType; /* MESSAGE, REGISTRATION or UNREGISTRATION */
}d_sampleRequestAlignData;


static c_bool
d_sampleRequestListenerWriteBead(
    c_object object,
    c_voidp userData)
{
    v_message message;
    v_messageEOT messageEOT;
    struct writeBeadHelper *data;
    c_ulong size;
    sd_serializedData serializedData;

    message = (v_message)object;
    data = (struct writeBeadHelper*)userData;

    if (v_stateTest(v_nodeState(message), L_ENDOFTRANSACTION)) {
        v_messageEOTExt xmsg_eot;
        messageEOT = (v_messageEOT)message;
        xmsg_eot = v_messageEOTExtCopyToExtType(messageEOT);
        serializedData = sd_serializerSerialize(data->serializerEOT, xmsg_eot);
        v_messageEOTExtFree (xmsg_eot);
    } else {
        v_messageExt xmsg;
        xmsg = v_messageExtCopyToExtType (data->xmsgType, message);
        serializedData = sd_serializerSerialize(data->serializer, xmsg);
        v_messageExtFree (xmsg);
    }

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
        } else if(v_stateTest(v_nodeState(message), L_ENDOFTRANSACTION)){
            data->eotCount++;
        }

        size = sd_serializedDataGetTotalSize(serializedData);
        assert(size <= OS_MAX_INTEGER(c_long));

        data->size += size;
        data->sampleChain->msgBody._d             = BEAD;
        data->sampleChain->msgBody._u.bead.size   = (c_long) size;
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

static c_bool
d_sampleRequestListenerAddList(
    c_object object,
    v_groupInstance instance,
    v_groupFlushType flushType,
    c_voidp userData)
{
    v_message message;
    v_registration registration;
    struct writeBeadHelper *data;
    c_bool process;
    os_timeW beginTime, endTime;
    os_compare timeCompared;

    data = (struct writeBeadHelper*)userData;

    process = TRUE;

    d_timestampToTimeW(&beginTime, &data->request->beginTime, IS_Y2038READY(data->request));
    d_timestampToTimeW(&endTime, &data->request->endTime, IS_Y2038READY(data->request));
    switch(flushType){
    case V_GROUP_FLUSH_REGISTRATION:
        registration = (v_registration)object;
        if(data->checkTimeRange) {
            timeCompared = os_timeWCompare(registration->writeTime, endTime);

            if (timeCompared == OS_MORE) {
                process = FALSE;
            } else if(instance && strcmp(v_groupName(instance->group), "Group<__BUILT-IN PARTITION__,DCPSTopic>") == 0) {
                /* Don't process registrations for DCPSTopic as this would introduce a scalability issue since
                 * there is always an alive writer per federation. If these registrations would be aligned there
                 * would be n registrations stored per builtin topic instance where n is the number of
                 * federations in a system.
                 * The only condition for aligning a registration is when there are multiple writers
                 * for the same instance. Therefore this is not an issue for other (builtin) topics.
                 */
                process = FALSE;
            } else if(data->request->withTimeRange == TRUE){
                timeCompared = os_timeWCompare(registration->writeTime, beginTime);

                if (timeCompared == OS_LESS) {
                    process = FALSE;  /* produced before the time-range */
                }
            }
        }
        break;
    case V_GROUP_FLUSH_UNREGISTRATION:
        registration = (v_registration)object;
        if(data->checkTimeRange){
            timeCompared = os_timeWCompare(registration->writeTime, endTime);

            if (timeCompared == OS_MORE) {
                process = FALSE;
            } else if(data->request->withTimeRange == TRUE){
                timeCompared = os_timeWCompare(registration->writeTime, beginTime);

                if (timeCompared == OS_LESS) {
                    process = FALSE;  /* produced before the time-range */
                }
            }
        }
        break;
    case V_GROUP_FLUSH_MESSAGE:
        message = (v_message)object;

        if(data->checkTimeRange){
            timeCompared = os_timeWCompare(message->writeTime, endTime);

            if(timeCompared == OS_MORE) {
                process = FALSE;
            } else if(data->request->withTimeRange == TRUE){
                timeCompared = os_timeWCompare(message->writeTime, beginTime);

                if (timeCompared == OS_LESS) {
                    process = FALSE;  /* produced before the time-range */
                }
            }
        }
        if((process == TRUE) && (data->isDCPSTopicBuiltinGroup == TRUE)){
            if(isBuiltinTopicSample((v_topicInfoTemplate)message)){
                process = FALSE;
            }
        }
        break;
    case V_GROUP_FLUSH_TRANSACTION:
        message = (v_message)object;

        if(data->checkTimeRange){
            timeCompared = os_timeWCompare(message->writeTime, endTime);

            if(timeCompared == OS_MORE) {
                process = FALSE;
            } else if(data->request->withTimeRange == TRUE){
                timeCompared = os_timeWCompare(message->writeTime, beginTime);

                if (timeCompared == OS_LESS) {
                    process = FALSE;  /* produced before the time-range */
                }
            }
        }
        break;
    default:
        process = FALSE;
        OS_REPORT(OS_ERROR, "durability::d_sampleRequestListenerAddList", 0,
                "Internal error (received unknown message type)");
        break;
    }

    if(process == TRUE) {
        struct v_groupFlushData *objectData = os_malloc(sizeof(struct v_groupFlushData));
        objectData->object = c_keep(object);
        objectData->instance = c_keep(instance);
        objectData->flushType = flushType;
        data->list = c_iterAppend(data->list, objectData);
    } else {
        data->skipCount++;
    }
    return FALSE;
}

static void
d_sampleRequestListenerAnswer(
    d_sampleRequestHelper helper)
{
    d_thread self = d_threadLookupSelf ();
    d_admin admin;
    d_durability durability;
    d_publisher publisher;
    d_group group;
    struct writeBeadHelper data;
    d_sampleChain sampleChain;
    c_bool sendData;
    v_group vgroup;
    v_kernel kernel;
    struct v_groupFlushData *objectData;
    c_ulong i;
    d_networkAddress addr;
    d_sampleRequestListener listener;
    d_sampleRequest request;
    d_alignerStatistics stats;
    v_historicalDataRequest vrequest;
    v_message vmessage;
    v_groupInstance instance;
    v_registration registration;
    v_resourcePolicyI resourceLimits;
    c_char *t, *p;
    c_bool equalHash = FALSE;
    c_ulong dataListLength = 0;

    listener        = helper->listener;
    request         = helper->request;
    admin           = d_listenerGetAdmin(d_listener(listener));
    durability      = d_adminGetDurability(admin);
    publisher       = d_adminGetPublisher(admin);
    group           = d_adminGetLocalGroup(admin, request->partition,
                                           request->topic, request->durabilityKind);
    data.count             = 0;
    data.writeCount        = 0;
    data.disposeCount      = 0;
    data.writeDisposeCount = 0;
    data.registerCount     = 0;
    data.unregisterCount   = 0;
    data.eotCount          = 0;
    data.skipCount         = 0;
    data.size              = 0;

    /* Determine the addressee. */
    assert(c_iterLength(helper->addressees) != 0);
    if (c_iterLength(helper->addressees) == 1) {
        d_networkAddress addr = d_networkAddress(c_iterObject(helper->addressees, 0));
        data.addressee = d_networkAddressNew(addr->systemId, addr->localId, addr->lifecycleId);
    } else {
        data.addressee  = d_networkAddressUnaddressed();
    }
    sampleChain     = d_sampleChainNew(admin, request->partition, request->topic,
                                request->durabilityKind, &request->source);

    stats           = d_alignerStatisticsNew();

    sampleChain->addresseesCount = c_iterLength(helper->addressees);
    sampleChain->addressees = os_malloc(sampleChain->addresseesCount*C_SIZEOF(d_networkAddress));
    addr = d_networkAddress(sampleChain->addressees);
    /* Retrieve the targeted addressees from the sampleChain */
    for(i=0; i<sampleChain->addresseesCount; i++) {
        addr[i] = *d_networkAddress(c_iterObject(helper->addressees, i));
    }
    stats->alignerRequestsAnsweredDif = (sampleChain->addresseesCount);
    stats->alignerRequestsOpenDif -= (c_long) (sampleChain->addresseesCount);

    if (sampleChain->addresseesCount > 1) {
        stats->alignerRequestsCombinedOpenDif = -1;
        stats->alignerRequestsCombinedAnsweredDif = 1;
    }

    if(!group){
        /* A request was received for a group that is unknown.
         * In that case do not send data, just send a LINK
         * indicating that no samples are available.
         */
        sendData = FALSE;
    } else {
        /* Only send the data if it is part of an aligner namespace */
        sendData = d_adminGroupInAlignerNS(
                                admin, request->partition,
                                request->topic);
    }
    d_messageSetAddressee(d_message(sampleChain), data.addressee);

    if (sendData == TRUE) {
        /* Collect all data that needs to be sent. */
        vgroup           = d_groupGetKernelGroup(group);
        kernel           = v_objectKernel(vgroup);
        data.request     = request;
        data.sampleChain = sampleChain;
        data.publisher   = publisher;
        data.xmsgType    = v_messageExtTypeNew(vgroup->topic);
        data.serializer  = sd_serializerBigENewTyped(data.xmsgType);
        data.serializerEOT = sd_serializerBigENewTyped(c_resolve(c_getBase((c_object)vgroup), "kernelModule::v_messageEOTExt"));
        data.list        = c_iterNew(NULL);
        data.isDCPSTopicBuiltinGroup = FALSE;

        p = v_entity(vgroup->partition)->name;
        t = v_entity(vgroup->topic)->name;

        if((strlen(p) == strlen(V_BUILTIN_PARTITION)) &&
           (strlen(t) == strlen(V_TOPICINFO_NAME))){
            if(strncmp(p, V_BUILTIN_PARTITION, strlen(V_BUILTIN_PARTITION)) == 0){
                if(strncmp(t, V_TOPICINFO_NAME, strlen(V_TOPICINFO_NAME)) == 0){
                    data.isDCPSTopicBuiltinGroup = TRUE;
                }
            }
        }
        if (d_sampleRequestSpecificReader(request)) {
            /* This a request that originates from a reader */
            if (!d_sampleRequestHasCondition(request)) {
                /* This is an alignment request without any condition.
                 * align all samples in this case
                 */
                d_printTimedEvent(durability, D_LEVEL_FINE,
                        "Sending all samples for group %s.%s to fellow %u using source (%u,%u,%u).\n",
                        request->partition, request->topic,
                        data.addressee->systemId,
                        request->source.systemId,
                        request->source.localId,
                        request->source.lifecycleId);
                data.checkTimeRange = TRUE;
                v_kernelGroupTransactionBeginAccess(kernel);
                v_groupFlushAction(vgroup, d_sampleRequestListenerAddList, &data);
                v_kernelGroupTransactionEndAccess(kernel);
            } else {
                d_printTimedEvent(durability, D_LEVEL_FINE,
                        "Sending conditional set of samples for group %s.%s to fellow %u.\n",
                        request->partition, request->topic,
                        data.addressee->systemId);
                data.checkTimeRange = FALSE;

                resourceLimits.v.max_samples              = request->maxSamples;
                resourceLimits.v.max_instances            = request->maxInstances;
                resourceLimits.v.max_samples_per_instance = request->maxSamplesPerInstance;

                vrequest = v_historicalDataRequestNew(
                        kernel,
                        request->filter,
                        (const c_char**)request->filterParams,
                        request->filterParamsCount,
                        c_timeToTimeW(request->beginTime),
                        c_timeToTimeW(request->endTime),
                        &resourceLimits,
                        OS_DURATION_ZERO);

                (void)d_shmAllocAssert(vrequest, "Allocation of historicalDataRequest failed.");

                v_groupFlushActionWithCondition(
                        vgroup, vrequest, d_sampleRequestListenerAddList, &data);

                c_free(vrequest);
            }
        } else if (request->filter != NULL) {
            /* This is a normal alignment request with a static content filter. */
            d_printTimedEvent(durability, D_LEVEL_FINE,
                "Sending samples for group '%s.%s' to fellow %u that match filter expression '%s'.\n",
                request->partition, request->topic,
                data.addressee->systemId,
                request->filter);
            data.checkTimeRange = FALSE;

            resourceLimits.v.max_samples              = request->maxSamples;
            resourceLimits.v.max_instances            = request->maxInstances;
            resourceLimits.v.max_samples_per_instance = request->maxSamplesPerInstance;

            vrequest = v_historicalDataRequestNew(
                            kernel,
                            request->filter,
                            (const c_char**)request->filterParams,
                            request->filterParamsCount,
                            c_timeToTimeW(request->beginTime),
                            c_timeToTimeW(request->endTime),
                            &resourceLimits,
                            OS_DURATION_ZERO);

            (void)d_shmAllocAssert(vrequest, "Allocation of historicalDataRequest failed.");

            v_groupFlushActionWithCondition(
                    vgroup, vrequest, d_sampleRequestListenerAddList, &data);

            c_free(vrequest);
        } else {
            /* This is an alignment request without any filter.
             * align all samples in this case
             */
            d_printTimedEvent(durability, D_LEVEL_FINE,
                "Sending all samples for group %s.%s to fellow %u using source (%u,%u,%u).\n",
                request->partition, request->topic,
                data.addressee->systemId,
                request->source.systemId,
                request->source.localId,
                request->source.lifecycleId);
            data.checkTimeRange = TRUE;
            v_kernelGroupTransactionBeginAccess(kernel);
            v_groupFlushAction(vgroup, d_sampleRequestListenerAddList, &data);
            v_kernelGroupTransactionEndAccess(kernel);
        }

        c_free(vgroup);

        /* Compare the group hash of the sender with the local one.
         * Note that for combined requests (senderAddress 0,0,0) hash checking is skipped.
         */
        if (!d_networkAddressIsUnaddressed(d_messageGetAddressee(d_message(sampleChain)))) {
            d_fellow fellow = d_adminGetFellow(admin, &(d_message(request)->senderAddress));

            if (fellow) {
                if (d_fellowHasCapabilitySupport(fellow) &&
                    d_fellowHasCapabilityGroupHash(fellow)) {
                    c_char *hash;
                    struct d_groupHash senderGroupHash;
                    struct d_groupHash groupHash;

                    hash = request->filterParams[request->filterParamsCount];
                    if (d_groupHashFromString(&senderGroupHash, hash) == TRUE) {
                        dataListLength = c_iterLength(data.list);
                        if (dataListLength != senderGroupHash.nrSamples) {
                            d_groupHashCalculate(&groupHash, data.list);
                            equalHash = d_groupHashIsEqual(&senderGroupHash, &groupHash);
                        }
                    }
                }
                d_fellowFree(fellow);
            }
        }

        /* The data to send is collected in data.list.
         * Now write the beads.
         */
        objectData = c_iterTakeFirst(data.list);
        while (objectData) {
            d_threadAwake (self);
            instance = objectData->instance;

            /* Only write beads when hash is unequal */
            if (!equalHash) {
                switch(objectData->flushType) {
                case V_GROUP_FLUSH_REGISTRATION:
                case V_GROUP_FLUSH_UNREGISTRATION:
                    /* Registration or unregistration */
                    registration = (v_registration)objectData->object;
                    vmessage = v_groupInstanceCreateMessage(instance);
                    if (vmessage) {
                        vmessage->writerGID = registration->writerGID;
                        vmessage->qos = c_keep(registration->qos);
                        vmessage->writeTime = registration->writeTime;
                        vmessage->sequenceNumber = registration->sequenceNumber;
                        if (objectData->flushType == V_GROUP_FLUSH_REGISTRATION) {
                            v_stateSet(v_nodeState(vmessage), L_REGISTER);
                        } else if(objectData->flushType == V_GROUP_FLUSH_UNREGISTRATION) {
                            v_stateSet(v_nodeState(vmessage), L_UNREGISTER);
                        }
                        if (v_stateTest(registration->state, L_IMPLICIT) == TRUE) {
                            v_stateSet(v_nodeState(vmessage), L_IMPLICIT);
                        }
                        d_sampleRequestListenerWriteBead(vmessage, &data);
                        c_free(vmessage);
                    } else {
                        d_printTimedEvent(durability, D_LEVEL_SEVERE,
                            "Failed to allocate message, alignment for group %s.%s to fellow %u will be incomplete.\n",
                            request->partition, request->topic,
                            d_message(request)->senderAddress.systemId);
                        OS_REPORT(OS_ERROR,
                                "durability::d_sampleRequestListenerAnswer", 0,
                                "Failed to allocate message, alignment will be incomplete");
                        (void)d_shmAllocAssert(vmessage, "Allocation of sample failed.");
                    }
                    break;
                case V_GROUP_FLUSH_MESSAGE:
                    /* Normal data */
                    vmessage = v_message(objectData->object);
                    assert(vmessage != NULL);

                    if (c_getType(vmessage) != v_kernelType(kernel, K_MESSAGE)) {
                        d_sampleRequestListenerWriteBead(vmessage, &data);
                    } else {
                        /* If the message is a mini-message without keys, temporarily replace it with
                         * a typed message that does include the keys. That way the bead becomes self-
                         * describing and so the receiving node can deduct its instance again.
                         */
                        v_message msg= v_groupInstanceCreateTypedInvalidMessage(instance, vmessage);
                        if (msg) {
                            d_sampleRequestListenerWriteBead(msg, &data);
                            c_free(msg);
                        } else {
                            d_printTimedEvent(durability, D_LEVEL_SEVERE,
                                "Failed to allocate message, alignment for group %s.%s to fellow %u will be incomplete.\n",
                                request->partition, request->topic,
                                d_message(request)->senderAddress.systemId);
                            OS_REPORT(OS_ERROR,
                                    "durability::d_sampleRequestListenerAnswer", 0,
                                    "Failed to allocate message, alignment will be incomplete");
                            (void)d_shmAllocAssert(msg, "Allocation of sample failed.");
                        }
                    }
                    break;
                case V_GROUP_FLUSH_TRANSACTION:
                    /* Normal data */
                    vmessage = v_message(objectData->object);
                    d_sampleRequestListenerWriteBead(vmessage, &data);
                    break;
                default:
                    assert(0);
                    break;
                }
            }
            c_free(instance);
            c_free(objectData->object);
            os_free(objectData);
            objectData = c_iterTakeFirst(data.list);
        } /* while */
        c_iterFree(data.list);
        v_messageExtTypeFree (data.xmsgType);
        sd_serializerFree(data.serializer);
        sd_serializerFree(data.serializerEOT);
    } else {
        d_printTimedEvent(durability, D_LEVEL_FINE,
            "Sending no samples for group %s.%s to fellow %u.\n",
            request->partition, request->topic,
            d_message(request)->senderAddress.systemId);
    }
    /* Send a LINK */
    sampleChain->msgBody._d = LINK;
    sampleChain->msgBody._u.link.nrSamples = data.count;

    if (group) {
        sampleChain->msgBody._u.link.completeness = d_groupGetCompleteness(group);
        if (equalHash) {
            sampleChain->msgBody._u.link.nrSamples = D_GROUP_IS_EQUAL;
        }
    } else {
        sampleChain->msgBody._u.link.completeness = D_GROUP_UNKNOWN;
    }
    d_publisherSampleChainWrite(publisher, sampleChain, data.addressee);

    if (equalHash) {
        c_ulong skipCount = 0;
        d_printTimedEvent(durability, D_LEVEL_FINE,
            "Group %s.%s (with %u samples) equal to fellow %u (%d fellows total), no samples sent.\n",
            request->partition, request->topic, dataListLength,
            data.addressee->systemId,
            sampleChain->addresseesCount);
        skipCount += data.writeCount;
        skipCount += data.disposeCount;
        skipCount += data.writeDisposeCount;
        skipCount += data.registerCount;
        skipCount += data.unregisterCount;
        skipCount += data.eotCount;
        skipCount += data.skipCount;
        d_printTimedEvent(durability, D_LEVEL_FINE,
             "WRITE: 0, DISPOSED: 0, WRITE_DISPOSED: 0, REGISTER: 0, UNREGISTER:0, EOT: 0 (SKIPPED: %d)\n",
             skipCount);
    } else {
        d_printTimedEvent(durability, D_LEVEL_FINEST,
            "Sent %d samples for group %s.%s to fellow %u (%d fellows total).\n",
            data.count, request->partition, request->topic,
            data.addressee->systemId,
            sampleChain->addresseesCount);
        d_printTimedEvent(durability, D_LEVEL_FINE,
            "WRITE: %d, DISPOSED: %d, WRITE_DISPOSED: %d, REGISTER: %d, UNREGISTER: %d EOT: %d (SKIPPED: %d)\n",
            data.writeCount, data.disposeCount, data.writeDisposeCount,
            data.registerCount, data.unregisterCount, data.eotCount, data.skipCount);
        /* Update the statistics */
        stats->alignerSamplesTotalDif           = data.count;
        stats->alignerSamplesRegisterDif        = data.registerCount;
        stats->alignerSamplesWriteDif           = data.writeCount;
        stats->alignerSamplesDisposeDif         = data.disposeCount;
        stats->alignerSamplesWriteDisposeDif    = data.writeDisposeCount;
        stats->alignerSamplesUnregisterDif      = data.unregisterCount;
        stats->alignerTotalSizeDif              = data.size;

    }

    d_sampleChainFree(sampleChain);
    d_networkAddressFree(data.addressee);
    d_durabilityUpdateStatistics(durability, d_statisticsUpdateAligner, stats);
    d_alignerStatisticsFree(stats);

    return;
}


static void
d_sampleRequestHelperFree(
    d_sampleRequestHelper helper)
{
    d_networkAddress addr;

    assert(helper);

    /* The sampleRequestHelper does not inherit from
     * d_object, so there is no deinit to call. Simply
     * deallocate all stuff that was allocated in
     * d_sampleRequestHelperNew.
     */
    if (helper->request) {
        d_sampleRequestFree(helper->request);
    }
    /* Clean all addresses */
    addr = d_networkAddress(c_iterTakeFirst(helper->addressees));
    while (addr){
        d_networkAddressFree(addr);
        addr = d_networkAddress(c_iterTakeFirst(helper->addressees));
    }
    c_iterFree(helper->addressees);
    os_free(helper);
}

/**
 * \brief Send responses to pending sampleRequests whose timeToAct has expired
 */
static c_bool
sendAction(
    d_action action,
    c_bool terminate)
{
    d_thread self = d_threadLookupSelf ();
    d_sampleRequestHelper helper = NULL;
    d_admin admin;
    d_durability durability;
    d_sampleRequestListener listener;
    d_serviceState state;
    os_timeM currentTime;
    c_bool mightHaveNext;

    listener = d_sampleRequestListener(d_actionGetArgs(action));
    if (d_objectIsValid(d_object(listener), D_LISTENER)) {
        if (terminate == FALSE) {
            d_listenerLock(d_listener(listener));
            admin = d_listenerGetAdmin(d_listener(listener));
            durability = d_adminGetDurability(admin);
            if (listener->mayProceed == FALSE) {
                state = d_durabilityGetState(durability);
                if ((state != D_STATE_INIT) &&
                    (state != D_STATE_DISCOVER_FELLOWS_GROUPS) &&
                    (state != D_STATE_DISCOVER_LOCAL_GROUPS) &&
                    (state != D_STATE_DISCOVER_PERSISTENT_SOURCE) &&
                    (state != D_STATE_INJECT_PERSISTENT) &&
                    (state != D_STATE_TERMINATING) &&
                    (state != D_STATE_TERMINATED)) {
                    /* Only if the state of the durability service has
                     * progressed 'far enough' it is allowed to send
                     * responses to a sampleRequest.
                     */
                    listener->mayProceed = TRUE;
                }
            }
            if (listener->mayProceed == TRUE) {
                mightHaveNext = TRUE;

                do {
                    currentTime = os_timeMGet();
                    d_threadAwake(self);
                    helper = d_sampleRequestHelper(c_iterTakeFirst(listener->requests));

                    if(helper != NULL){
                        if (os_timeMCompare(helper->timeToAct, currentTime) != OS_MORE) {
                            c_iterIter iter;
                            d_networkAddress addr;
                            char str[256], tmp_str[256];

                            str[0] = '\0';
                            tmp_str[0] = '\0';
                            iter = c_iterIterGet(helper->addressees);
                            while ((addr = (d_networkAddress)c_iterNext(&iter)) != NULL) {
                                strncpy(tmp_str, str, sizeof(str));
                                 if (strncmp(tmp_str, "\0", sizeof(tmp_str)) == 0) {
                                    snprintf(str, sizeof(str), "%u", addr->systemId);
                                } else {
                                    snprintf(str, sizeof(str), "%s,%u", tmp_str, addr->systemId);
                                }
                            }

                            d_printTimedEvent(durability, D_LEVEL_FINER,
                                    "Now sending data for group %s.%s to %d fellow(s) [%s].\n",
                                    helper->request->partition,
                                    helper->request->topic,
                                    c_iterLength(helper->addressees),
                                    str);

                            /* Send the answer to the request */
                            d_lockUnlock(d_lock(listener));
                            d_sampleRequestListenerAnswer(helper);
                            d_lockLock(d_lock(listener));

                            d_sampleRequestHelperFree(helper);
                        } else {
                            /* No time to act yet, so prepend taken action again */
                            listener->requests = c_iterInsert(listener->requests, helper);
                            mightHaveNext = FALSE;
                        }
                    } else {
                        mightHaveNext = FALSE;
                    }
                } while(!d_durabilityMustTerminate(durability) && (mightHaveNext == TRUE));
            }
            d_lockUnlock(d_lock(listener));
        }
    }
    return TRUE;
}


static void
d_sampleRequestListenerDeinit(
    d_sampleRequestListener listener)
{
    d_sampleRequestHelper helper;
    c_bool removed;

    assert(d_sampleRequestListenerIsValid(listener));

    /* Stop the sampleRequestListener before cleaning up. */
    d_sampleRequestListenerStop(listener);
    /* Clean up stuff that was allocated */
    removed = d_actionQueueRemove(listener->actionQueue, listener->actor);
    if (removed == TRUE) {
        d_actionFree(listener->actor);
    }
    d_actionQueueFree(listener->actionQueue);
    /* Clean up pending requests */
    d_lockLock(d_lock(listener));
    helper = d_sampleRequestHelper(c_iterTakeFirst(listener->requests));
    while (helper) {
        d_sampleRequestHelperFree(helper);
        helper = d_sampleRequestHelper(c_iterTakeFirst(listener->requests));
    }
    c_iterFree(listener->requests);
    d_lockUnlock(d_lock(listener));
    /* Call super-deinit */
    d_readerListenerDeinit(d_readerListener(listener));
}

static void
d_sampleRequestListenerAction(
    d_listener listener,
    d_message message)
{
    d_durability durability;
    d_admin admin;
    d_sampleRequest request;
    d_sampleChain sampleChain;
    d_networkAddress addr, addr1, addressee;
    d_publisher publisher;
    d_configuration config;
    d_fellow fellow;
    d_alignerStatistics stats;
    d_name fellowRole;

    assert(d_sampleRequestListenerIsValid(listener));

    admin      = d_listenerGetAdmin(listener);
    durability = d_adminGetDurability(admin);
    request    = d_sampleRequest(message);
    publisher  = d_adminGetPublisher(admin);
    addr       = d_networkAddressNew(
                    message->senderAddress.systemId,
                    message->senderAddress.localId,
                    message->senderAddress.lifecycleId);
    fellow     = d_adminGetFellow(admin, addr);

    fellowRole = d_fellowGetRole(fellow);
    config = d_durabilityGetConfiguration(durability);
    addressee = d_networkAddressNew(
                        message->addressee.systemId,
                        message->addressee.localId,
                        message->addressee.lifecycleId);
    stats      = d_alignerStatisticsNew();

    stats->alignerRequestsReceivedDif = 1;

    if ( (!fellow) ||
         (d_fellowGetCommunicationState(fellow) == D_COMMUNICATION_STATE_INCOMPATIBLE_STATE) ||
         (d_fellowGetCommunicationState(fellow) == D_COMMUNICATION_STATE_INCOMPATIBLE_DATA_MODEL)) {
        d_printTimedEvent(durability, D_LEVEL_FINE,
            "Ignoring sample request for group %s.%s from fellow %u using source (%u,%u,%u)\n",
            request->partition, request->topic,
            d_message(request)->senderAddress.systemId,
            request->source.systemId,
            request->source.localId,
            request->source.lifecycleId);

        /* Send immediately a LINK message containing 0 samples to
         * prevent that the requester keeps waiting. */
        sampleChain = d_sampleChainNew(admin, request->partition,
                                    request->topic, request->durabilityKind,
                                    &request->source);
        /* Add the recipient to the link */
        sampleChain->addresseesCount = 1;
        sampleChain->addressees = os_malloc(C_SIZEOF(d_networkAddress));
        addr1 = d_networkAddress(sampleChain->addressees);
        addr1[0] = *d_networkAddress(addr);

        d_messageSetAddressee(d_message(sampleChain), addr);

        sampleChain->msgBody._d = LINK;
        sampleChain->msgBody._u.link.nrSamples = 0;
        if (!fellow) {
            /* The fellow is not known, so there might non-symmetric
             * communication. To let the fellow know that I don't know
             * him I sent D_GROUP_KNOWLEDGE_UNDEFINED. This may be
             * used by the recipient to act on the non-symmetrical
             * communication situation, e.g., by removing me as fellow.
             */
            sampleChain->msgBody._u.link.completeness = D_GROUP_KNOWLEDGE_UNDEFINED;
        } else {
            sampleChain->msgBody._u.link.completeness = D_GROUP_UNKNOWN;
        }
        d_publisherSampleChainWrite(publisher, sampleChain, addr);
        d_sampleChainFree(sampleChain);

        stats->alignerRequestsIgnoredDif = 1;

        if (fellow) {
            d_fellowFree(fellow);
        }
    }
    /* If this request was sent to everyone, it is meant for everyone with
     * the same role as the role of the sender. Therefore ignore
     * the request if my role is not the same as the role of the sender
     */
    else if (d_networkAddressIsUnaddressed(addressee)
            && fellowRole && strcmp(config->role, fellowRole) != 0) {
        d_printTimedEvent(durability, D_LEVEL_FINE,
                "Received sample request for group %s.%s from fellow %u %susing source (%u,%u,%u), but "
                "my role is '%s' whereas the role of the sender is '%s'. "
                "Therefore the request is not meant for me and I am ignoring "
                "this request.\n",
                request->partition, request->topic,
                d_message(request)->senderAddress.systemId,
                d_sampleRequestHasHash(request, fellow) ? "with hash " : "",
                request->source.systemId,
                request->source.localId,
                request->source.lifecycleId,
                config->role, fellowRole);
        d_fellowFree(fellow);
    } else {
        /* I must respond to the sampleRequest. Let's queue the
         * request so it can be handled later.
         */
        if (request->filter) {
            d_printTimedEvent(durability, D_LEVEL_FINE,
                "Received sample request for group %s.%s from fellow %u %susing source (%u,%u,%u) using filter expression '%s', adding it to queue.\n",
                request->partition, request->topic,
                d_message(request)->senderAddress.systemId,
                d_sampleRequestHasHash(request, fellow) ? "with hash " : "",
                request->source.systemId,
                request->source.localId,
                request->source.lifecycleId,
                request->filter);
        } else {
            d_printTimedEvent(durability, D_LEVEL_FINE,
                "Received sample request for group %s.%s from fellow %u %susing source (%u,%u,%u), adding it to queue.\n",
                request->partition, request->topic,
                d_message(request)->senderAddress.systemId,
                d_sampleRequestHasHash(request, fellow) ? "with hash " : "",
                request->source.systemId,
                request->source.localId,
                request->source.lifecycleId);
        }
        d_sampleRequestListenerAddRequest(d_sampleRequestListener(listener), request);
        d_fellowFree(fellow);
    }
    d_durabilityUpdateStatistics(durability, d_statisticsUpdateAligner, stats);
    d_alignerStatisticsFree(stats);
    d_networkAddressFree(addressee);
    d_networkAddressFree(addr);

    return;
}

static void
d_sampleRequestListenerInit(
    d_sampleRequestListener listener,
    d_subscriber subscriber)
{
    os_duration sleepTime = OS_DURATION_INIT(0, 50000000);  /* 50 ms */
    d_admin admin;
    d_durability durability;
    d_configuration config;

    /* Do not assert the listener because the initialization
     * of the listener has not yet completed
     */

    assert(d_subscriberIsValid(subscriber));

    admin       = d_subscriberGetAdmin(subscriber);
    assert(d_adminIsValid(admin));
    durability  = d_adminGetDurability(admin);
    assert(d_durabilityIsValid(durability));
    config      = d_durabilityGetConfiguration(durability);
    assert(d_configurationIsValid(config));

    /* Call super-init */
    d_readerListenerInit(   d_readerListener(listener),
                            D_SAMPLE_REQ_LISTENER,
                            d_sampleRequestListenerAction,
                            subscriber,
                            D_SAMPLE_REQ_TOPIC_NAME,
                            D_SAMPLE_REQ_TOP_NAME,
                            V_RELIABILITY_RELIABLE,
                            V_HISTORY_KEEPALL,
                            V_LENGTH_UNLIMITED,
                            config->alignerScheduling,
                            (d_objectDeinitFunc)d_sampleRequestListenerDeinit);
    /* Initialize the sampleRequestListener */
    listener->mayProceed  = FALSE;
    listener->requests    = c_iterNew(NULL);
    listener->actionQueue = d_actionQueueNew("sampleRequestHandler", sleepTime, config->alignerScheduling);
    listener->actor       = d_actionNew(os_timeMGet(), sleepTime, sendAction, listener);
    d_actionQueueAdd(listener->actionQueue, listener->actor);
}

d_sampleRequestListener
d_sampleRequestListenerNew(
    d_subscriber subscriber)
{
    d_sampleRequestListener listener;

    assert(d_subscriberIsValid(subscriber));

    /* Allocate sampleRequestListener object */
    listener = d_sampleRequestListener(os_malloc(C_SIZEOF(d_sampleRequestListener)));
    if (listener) {
        /* Initialize the sampleRequestListener */
        d_sampleRequestListenerInit(listener, subscriber);
    }
    return listener;
}

void
d_sampleRequestListenerFree(
    d_sampleRequestListener listener)
{
    assert(d_sampleRequestListenerIsValid(listener));

    d_objectFree(d_object(listener));
}


c_bool
d_sampleRequestListenerStart(
    d_sampleRequestListener listener)
{
    assert(d_sampleRequestListenerIsValid(listener));

    return d_readerListenerStart(d_readerListener(listener));
}

c_bool
d_sampleRequestListenerStop(
    d_sampleRequestListener listener)
{
    return d_readerListenerStop(d_readerListener(listener));
}
