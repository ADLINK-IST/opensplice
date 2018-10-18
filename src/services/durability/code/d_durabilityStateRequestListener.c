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
#include "d__durabilityStateRequestListener.h"
#include "d__durabilityStateRequest.h"
#include "d__durabilityState.h"
#include "d__publisher.h"
#include "d__configuration.h"
#include "d__admin.h"
#include "d__durability.h"
#include "d__misc.h"
#include "d__thread.h"
#include "d__waitset.h"
#include "d__listener.h"
#include "d__subscriber.h"
#include "d__group.h"
#include "d__partitionTopicState.h"
#include "d_qos.h"
#include "u_listener.h"
#include "v_message.h"
#include "v_observer.h"

#include "v_group.h"
#include "v_builtin.h"
#include "v_time.h"
#include "v_state.h"
#include "v_historicalDataRequest.h"
#include "v_groupInstance.h"
#include "v_entity.h"
#include "v_message.h"
#include "v_dataReaderInstance.h"
#include "v_topic.h"
#include "sd_serializer.h"
#include "sd_serializerBigE.h"
#include "sd_cdr.h"
#include "os_heap.h"
#include "os_report.h"
#include "c_misc.h"

#include "client_durabilitySplType.h"


/**
 * Macro that checks the d_durabilityStateRequestListener validity.
 * Because d_durabilityStateRequestListener is a concrete class typechecking is required.
 */
#define d_durabilityStateRequestListenerIsValid(_this)   \
        d_listenerIsValidKind(d_listener(_this), D_DURABILITY_STATE_REQ_LISTENER)

/**
 * \brief The d_durabilityStateRequestListener cast macro.
 *
 * This macro casts an object to a d_durabilityStateRequestListener object.
 */
#define d_durabilityStateRequestListener(_this) ((d_durabilityStateRequestListener)(_this))


C_STRUCT(d_durabilityStateRequestListener) {
    C_EXTENDS(d_listener);
    u_dataReader dataReader;
    d_waitsetEntity waitsetData;
    c_iter requests;
    d_subscriber subscriber;
};


static c_iter *
partitionsCopyOut(
    c_iter *to,
    c_sequence from)
{
    c_ulong length, i;
    c_string *fromStr, toStr;
    size_t len;

    assert(to);

    fromStr = (c_string*)from;
    /* The generic copy routines do not always handle c_sequenceSize
     * correctly, but luckily we can just as well use c_arraySize. */
    length = c_arraySize(from);
    for (i = 0; i < length; i++) {
        len = strlen(fromStr[i]);
        toStr = os_malloc(len+1);  /* add '\0' */
        toStr = os_strncpy(toStr, fromStr[i], len);
        toStr[len] = '\0';
        *to = c_iterAppend(*to, toStr);
    } /* for */
    return to;
}


static c_iter *
serverIdsCopyOut(
    c_iter *to,
    c_sequence from)
{
    c_ulong length, i;
    struct _DDS_Gid_t *toServerId, *fromServerId;

    assert(*to);

    fromServerId = (struct _DDS_Gid_t *)from;
    /* The generic copy routines do not always handle c_sequenceSize
     * correctly, but luckily we can just as well use c_arraySize. */
    length = c_arraySize(from);

    for (i = 0; i < length; i++) {
        toServerId = os_malloc(sizeof(struct _DDS_Gid_t));
        *toServerId = fromServerId[i];
        *to = c_iterAppend(*to, toServerId);
    } /* for */
    return to;
}


static c_iter *
extensionsCopyOut(
    c_iter *to,
    c_sequence from)
{
    c_ulong length, i;
    struct _DDS_NameValue_t *toNameValue, *fromNameValue;

    assert(*to);

    fromNameValue = (struct _DDS_NameValue_t *)from;
    /* The generic copy routines do not always handle c_sequenceSize
     * correctly, but luckily we can just as well use c_arraySize. */
    length = c_arraySize(from);

    for (i = 0; i < length; i++) {
        toNameValue = os_malloc(sizeof(struct _DDS_NameValue_t));
        *toNameValue = fromNameValue[i];
        *to = c_iterAppend(*to, toNameValue);
    } /* for */
    return to;
}

/**
 * \brief Construct and send an durabilityState answer to a
 *        durabilityStateRequest
 *
 */
static void
d_durabilityStateRequestListenerAnswer(
    d_durabilityStateRequestListener listener,
    d_durabilityStateRequest request)
{
    d_admin admin;
    d_durability durability;
    d_publisher publisher;
    d_durabilityState durabilityState;
    struct collectMatchingGroupsHelper helper;
    d_tableIter tableIter;
    d_group group;
    d_partitionTopicState state;
    struct _DDS_Gid_t myClientId;
    c_bool selfInflicted;

    assert(d_durabilityStateRequestListenerIsValid(listener));
    assert(d_durabilityStateRequestIsValid(request));

    admin = d_listenerGetAdmin(d_listener(listener));
    durability = d_adminGetDurability(admin);
    publisher = d_adminGetPublisher(admin);
    myClientId = d_durabilityGetMyServerId(durability);
    selfInflicted = ((myClientId.prefix == request->requestId.clientId.prefix) && (myClientId.suffix == request->requestId.clientId.suffix));
    d_printTimedEvent(durability, D_LEVEL_FINER,
         "Answering durabilityStateRequest (%"PA_PRId64".%"PA_PRId64",%"PA_PRIu32")%s\n",
         request->requestId.clientId.prefix,
         request->requestId.clientId.suffix,
         request->requestId.requestId,
         selfInflicted ? " (self-inflicted)" : "");
    /* Create a durabilityState message */
    durabilityState = d_durabilityStateNew(admin);
    if (!durabilityState) {
        d_printTimedEvent(durability, D_LEVEL_WARNING,
             "Unable to respond to durabilityStateRequest (%"PA_PRId64".%"PA_PRId64",%"PA_PRIu32"), I am going to drop this request\n",
             request->requestId.clientId.prefix, request->requestId.clientId.suffix, request->requestId.requestId);
        goto err_allocDurabilityState;
    }
    /* Self-inflicted requests do not have a requestId.
     * Clients will interpret DurabilityState messages
     * with an empty list of requestIds as a push message
     * from a server */
    if (!selfInflicted) {
        struct _DDS_RequestId_t *requestId;

        requestId = os_malloc(sizeof(struct _DDS_RequestId_t));
        *requestId = request->requestId;
        (void)c_iterAppend(durabilityState->requestIds, (c_voidp)requestId);
    }
    /* Set the request to answer */
    durabilityState->request = request;
    /* Retrieve the matching groups */
    helper.admin = admin;
    helper.topic = request->topic;
    helper.partitions = request->partitions;
    helper.matchingGroups = d_tableNew(d_groupCompare, d_groupFree);
    if (!helper.matchingGroups) {
        goto err_allocMatchingGroups;
    }
    helper.forMe = request->forMe;
    helper.forEverybody = request->forEverybody;
    helper.isAligner = FALSE;
    helper.isResponsible = FALSE;
    helper.masterKnown = FALSE;
    helper.groupFound = FALSE;
    helper.isHistoricalDataRequest = FALSE;
    d_adminCollectMatchingGroups(admin, &helper);
    if (helper.isAligner || helper.forMe) {
        /* Always reply if I am aligner or the request is addressed to me */
        d_printTimedEvent(durability, D_LEVEL_FINER,
              "Total %d groups matched durabilityStateRequest (%"PA_PRId64".%"PA_PRId64",%"PA_PRIu32"), going to reply\n",
               d_tableSize(helper.matchingGroups), request->requestId.clientId.prefix, request->requestId.clientId.suffix, request->requestId.requestId);
        /* Collect info for the matching groups */
        group = d_group(d_tableIterFirst(helper.matchingGroups, &tableIter));
        while (group) {
            state = d_partitionTopicStateNew(group);
            (void)c_iterAppend(durabilityState->dataState, (c_voidp)state);
            group = d_group(d_tableIterNext(&tableIter));
        }
        /* Publish the durabilityState. */
        d_publisherDurabilityStateWrite(publisher, durabilityState);
    } else {
        d_printTimedEvent(durability, D_LEVEL_FINER,
              "Igoring durabilityStateRequest (%"PA_PRId64".%"PA_PRId64",%"PA_PRIu32") because I cannot align any of the requested data\n",
               request->requestId.clientId.prefix, request->requestId.clientId.suffix, request->requestId.requestId);
    }
    /* Free the durabilityState */
    d_durabilityStateFree(durabilityState);
    /* Free allocated stuff */
    d_tableFree(helper.matchingGroups);
    d_durabilityStateRequestFree(request);
    return;

err_allocMatchingGroups:
    d_tableFree(helper.matchingGroups);
    d_durabilityStateFree(durabilityState);
err_allocDurabilityState:
    OS_REPORT(OS_ERROR, "d_durabilityStateRequestListenerAnswer", 0, "Unable to answer durabilityStateRequest %u",
            request->requestId.requestId);
    d_durabilityStateRequestFree(request);
    return;
}


static
u_actionResult
processDurabilityStateRequest(
    c_object o,
    c_voidp userData /* listener * */)
{
    d_durabilityStateRequestListener listener;
    struct _DDS_DurabilityStateRequest *request;
    d_durabilityStateRequest durabilityStateRequest;
    v_dataReaderSample s;
    v_actionResult result = V_STOP;
    v_message message;
    d_admin admin;
    d_durability durability;
    c_ulong errorCode;
    c_iter partitions;
    c_iter serverIds;
    c_iter extensions;
    c_time timeout;
    d_client client, duplicate;
    d_networkAddress clientAddr;

    listener = d_durabilityStateRequestListener(userData);
    s = v_dataReaderSample(o);
    if (s != NULL) {
        admin = d_listenerGetAdmin(d_listener(listener));
        durability = d_adminGetDurability(admin);
        v_actionResultSet(result, V_PROCEED);
        if (v_dataReaderSampleStateTest(s, L_VALIDDATA) && !(v_dataReaderSampleInstanceStateTest(s, L_DISPOSED))) {
            message = v_dataReaderSampleTemplate(s)->message;
            request = (struct _DDS_DurabilityStateRequest *)C_DISPLACE(message, C_MAXALIGNSIZE(sizeof(*message)));
            /* Create a d_durabilityStateRequest object to store the request.
             * The request is freed when it has been answered successfully */
            timeout.seconds = request->timeout.sec;
            timeout.nanoseconds = request->timeout.nanosec;
            partitions = c_iterNew(NULL);
            if (!partitionsCopyOut(&partitions, request->partitions)) {
                goto err_allocRequestPartitions;
            }
            serverIds = c_iterNew(NULL);
            if (!serverIdsCopyOut(&serverIds, request->serverIds)) {
                goto err_allocRequestServerIds;
            }
            extensions = c_iterNew(NULL);
            if (!extensionsCopyOut(&extensions, request->extensions)) {
                goto err_allocRequestExtensions;
            }
            durabilityStateRequest = d_durabilityStateRequestNew(
                                                admin,
                                                request->version,
                                                request->requestId,
                                                request->topic,
                                                partitions,
                                                serverIds,
                                                timeout,
                                                extensions);
            c_iterFree(partitions);
            c_iterFree(serverIds);
            c_iterFree(extensions);
            /* Get or create client and store the requestId.clientId */
            clientAddr = d_networkAddressNew(message->writerGID.systemId, 0, 0);
            client = d_clientNew(clientAddr);
            if ((duplicate = d_adminAddClient(admin, client)) != client) {
                d_clientFree(client);
                client = duplicate;
            }
            d_clientSetClientId(client, durabilityStateRequest->requestId.clientId);
            d_printTimedEvent(durability, D_LEVEL_FINE,
                 "Received durabilityStateRequest (%"PA_PRId64".%"PA_PRId64",%"PA_PRIu32") for topic '%s' from federation %u\n",
                 request->requestId.clientId.prefix,
                 request->requestId.clientId.suffix,
                 request->requestId.requestId,
                 request->topic,
                 clientAddr->systemId);
            /* Check if the request is for me or for everybody*/
            if (!d_durabilityRequestIsForMe(durability, durabilityStateRequest->serverIds,
                                            &durabilityStateRequest->forMe, &durabilityStateRequest->forEverybody)) {
                d_printTimedEvent(durability, D_LEVEL_FINEST,
                     "durabilityStateRequest %"PA_PRId64".%"PA_PRId64",%"PA_PRIu32") for topic '%s' from federation %u is not meant for me, ignoring this request\n",
                     durabilityStateRequest->requestId.clientId.prefix,
                     durabilityStateRequest->requestId.clientId.suffix,
                     durabilityStateRequest->requestId.requestId,
                     durabilityStateRequest->topic,
                     clientAddr->systemId);
                goto skip_notMeantForMe;
            }
            /* Let's check if the request itself contains any sanity errors.
             * If there are any errors then ignore the request. */
            errorCode = d_durabilityStateRequestSanityCheck(durabilityStateRequest);
            if (errorCode != 0) {
                d_printTimedEvent(durability, D_LEVEL_FINEST,
                      "Sanity error in durabilitystateRequest (%"PA_PRId64".%"PA_PRId64",%"PA_PRIu32"), ignoring this request : '%s'\n",
                     durabilityStateRequest->requestId.clientId.prefix,
                     durabilityStateRequest->requestId.clientId.suffix,
                     durabilityStateRequest->requestId.requestId,
                     d_getErrorMessage(errorCode));
                goto skip_sanityError;
            }
            /* Add the request to the listener.
             * The request is freed once it is handled properly. */
            d_printTimedEvent(durability, D_LEVEL_FINER,
                 "Adding durabilityStateRequest %lu to queue\n",
                 request->requestId.requestId);
            d_durabilityStateRequestListenerAddRequest(listener, durabilityStateRequest);
            /* Free the stuff */
            d_networkAddressFree(clientAddr);
            d_clientFree(client);
        } else if (v_dataReaderSampleInstanceStateTest(s, L_DISPOSED)) {
            /* The request is disposed, so in this case we should remove pending requests. */
        }
    }
    return result;

skip_sanityError:
skip_notMeantForMe:
    d_durabilityStateRequestFree(durabilityStateRequest);
    d_networkAddressFree(clientAddr);
    return result;
err_allocRequestExtensions:
    c_iterFree(serverIds);
err_allocRequestServerIds:
    c_iterFree(partitions);
err_allocRequestPartitions:
    return result;
}



static c_ulong
d_durabilityStateRequestListenerAction(
    u_object o,            /* the observable that triggered the event */
    v_waitsetEvent event,  /* the event that was triggered */
    c_voidp userData       /* d_historicalDataRequestListener */)
{
    d_durabilityStateRequestListener listener = d_durabilityStateRequestListener(userData);
    d_thread self = d_threadLookupSelf();
    u_result ur;

    OS_UNUSED_ARG(o);

    d_threadAwake(self);
    if (event->kind & V_EVENT_DATA_AVAILABLE) {
        d_lockLock(d_lock(listener));

        ur = u_dataReaderTake(listener->dataReader, U_STATE_ANY, processDurabilityStateRequest, listener, OS_DURATION_ZERO);

        if (ur != U_RESULT_OK && ur != U_RESULT_NO_DATA) {
            OS_REPORT(OS_ERROR, D_CONTEXT_DURABILITY, ur,
                    "Failed to read data from DurabilityStateReader (result: %s)",
                    u_resultImage(ur));
        }
        d_lockUnlock(d_lock(listener));
    }
    d_threadAsleep(self, ~0u);
    return event->kind;
}


/**
 * \brief Deinitialize the durabilityStateRequestListener
 */
static void
d_durabilityStateRequestListenerDeinit(
    d_durabilityStateRequestListener listener)
{
    d_durabilityStateRequest request;

    assert(d_durabilityStateRequestListenerIsValid(listener));

    /* Stop the durabilityStateRequestListener */
    d_durabilityStateRequestListenerStop(listener);
    if (listener->waitsetData) {
        d_waitsetEntityFree(listener->waitsetData);
        listener->waitsetData = NULL;
    }
    if (listener->dataReader) {
        u_objectFree(u_object(listener->dataReader));
        listener->dataReader = NULL;
    }
    /* Clean up pending requests */
    request = d_durabilityStateRequest(c_iterTakeFirst(listener->requests));
    while (request) {
        d_durabilityStateRequestFree(request);
        request = d_durabilityStateRequest(c_iterTakeFirst(listener->requests));
    }
    listener->subscriber = NULL;
    /* Call super-deinit */
    d_listenerDeinit(d_listener(listener));
}

/**
 * \brief Create a durabilityStateRequestListener object
 *
 * The listener listens to durabilityStateRequest messages that
 * are published on the durability partition. Requests that
 * originate from myself are discarded.
 */
d_durabilityStateRequestListener
d_durabilityStateRequestListenerNew(
    d_subscriber subscriber)
{
    d_durabilityStateRequestListener listener;
    v_readerQos readerQos;
    d_admin admin;
    d_durability durability;
    d_configuration config;
    c_value ps[2];
    struct _DDS_Gid_t myServerId;

    assert(d_subscriberIsValid(subscriber));

    admin = d_subscriberGetAdmin(subscriber);
    assert(d_adminIsValid(admin));
    durability = d_adminGetDurability(admin);
    assert(d_durabilityIsValid(durability));
    config = d_durabilityGetConfiguration(durability);
    /* Allocate historicalDataRequestListener object */
    listener = d_durabilityStateRequestListener(os_malloc(C_SIZEOF(d_durabilityStateRequestListener)));
    /* Call super-init */
     d_listenerInit(d_listener(listener), D_DURABILITY_STATE_REQ_LISTENER, subscriber, NULL,
                    (d_objectDeinitFunc)d_durabilityStateRequestListenerDeinit);
    /* Initialize the durabilityStateRequestListener */
    listener->subscriber = subscriber;
    if ((readerQos = d_readerQosNew(V_DURABILITY_VOLATILE, V_RELIABILITY_RELIABLE)) == NULL) {
        goto err_reader_qos;
    }
    /* d_readerQosNew does not always return the proper settings.
     * The following settings ensure that the reader qos is correct */
    readerQos->history.v.kind = V_HISTORY_KEEPALL;
    readerQos->history.v.depth = V_LENGTH_UNLIMITED;
    readerQos->liveliness.v.lease_duration = OS_DURATION_INFINITE;
    readerQos->reliability.v.max_blocking_time = OS_DURATION_INIT(0,100000000);
    /* Prevent that the DurabilityStateRequestReader listens to requests from itself. */
    myServerId = d_durabilityGetMyServerId(durability);
    ps[0].kind= V_LONGLONG;
    ps[0].is.LongLong = myServerId.prefix;
    ps[1].kind= V_LONGLONG;
    ps[1].is.LongLong = myServerId.suffix;
    listener->dataReader = u_subscriberCreateDataReader(
        subscriber->subscriber2,
        D_DURABILITY_STATE_REQUEST_TOPIC_NAME "Reader",
        "select * from " D_DURABILITY_STATE_REQUEST_TOPIC_NAME " where requestId.clientId.prefix <> %0 OR requestId.clientId.suffix <> %1",
        ps, 2,
        readerQos);
    if (listener->dataReader == NULL) {
        goto err_dataReader;
    }

    if(u_entityEnable(u_entity(listener->dataReader)) != U_RESULT_OK) {
        goto err_dataReaderEnable;
    }
    listener->waitsetData = d_waitsetEntityNew(
                    "durabilityStateRequestListener",
                    u_object(listener->dataReader),
                    d_durabilityStateRequestListenerAction,
                    V_EVENT_DATA_AVAILABLE,
                    config->alignerScheduling, listener);
    if (!listener->waitsetData) {
        goto err_waitsetData;
    }
    /* No need to obtain historical data because topic is volatile */
    d_readerQosFree(readerQos);
    listener->requests = c_iterNew(NULL);

    return listener;

err_waitsetData:
    /* No undo for u_entityEnable(...) */
err_dataReaderEnable:
err_dataReader:
    d_readerQosFree(readerQos);
err_reader_qos:
    d_durabilityStateRequestListenerFree(listener);
    return NULL;
}


/**
 * \brief Free the durabilitystateRequestListener object
 */
void
d_durabilityStateRequestListenerFree(
    d_durabilityStateRequestListener listener)
{
    assert(d_durabilityStateRequestListenerIsValid(listener));

    d_objectFree(d_object(listener));
}



/**
 * \brief Start the durabilityStateRequestListener
 */
c_bool
d_durabilityStateRequestListenerStart(
    d_durabilityStateRequestListener listener)
{
    c_bool result;
    d_waitset waitset;
    u_result ur;

    result = FALSE;

    assert(d_durabilityStateRequestListenerIsValid(listener));

    d_lockLock(d_lock(listener));
    if (d_listener(listener)->attached == FALSE) {
        waitset = d_subscriberGetWaitset(listener->subscriber);
        result = d_waitsetAttach(waitset, listener->waitsetData);
        if (result == TRUE) {
            /* A V_DATA_AVAILABLE event is only generated when new data
             * arrives. It is NOT generated when historical data is inserted.
             * In case this durability service receives historical
             * DCPSSubscriptions from another durability service that
             * was started earlier, all these DCPSSubscriptions are
             * inserted in the reader at creation time. To read these
             * we must explicitly trigger a take action. */
            ur = u_dataReaderTake(listener->dataReader, U_STATE_ANY, processDurabilityStateRequest, listener, OS_DURATION_ZERO);
            if (ur != U_RESULT_OK && ur != U_RESULT_NO_DATA) {
                OS_REPORT(OS_ERROR, D_CONTEXT_DURABILITY, ur,
                        "Failed to read data from historicalDataRequestListenerReader (result: %s)", u_resultImage(ur));
            } else {
                d_listener(listener)->attached = TRUE;
                result = TRUE;
            }
        }
    } else {
        result = TRUE;
    }
    d_lockUnlock(d_lock(listener));
    return result;
}


/**
 * \brief Stop the durabilityStateRequestListener
 */
c_bool
d_durabilityStateRequestListenerStop(
    d_durabilityStateRequestListener listener)
{
    c_bool result = FALSE;
    d_admin admin;
    d_subscriber subscriber;
    d_waitset waitset;

    assert(d_durabilityStateRequestListenerIsValid(listener));

    d_lockLock(d_lock(listener));
    if (d_listener(listener)->attached == TRUE) {
        admin = d_listenerGetAdmin(d_listener(listener));
        subscriber = d_adminGetSubscriber(admin);
        waitset = d_subscriberGetWaitset(subscriber);
        d_lockUnlock(d_lock(listener));
        result = d_waitsetDetach(waitset, listener->waitsetData);
        d_lockLock(d_lock(listener));
        if (result) {
            d_listener(listener)->attached = FALSE;
            result = TRUE;
        }
    } else {
        result = TRUE;
    }
    d_lockUnlock(d_lock(listener));

    return result;
}


struct d_durabilityStateRequestHelper {
    d_durabilityStateRequest request;
    d_table clientIds;
};



/**
 * \brief Add a durabilityStateRequest to the list of pending requests.
 *
 * NOTE: similar requests are not combined, but immediately answered.
 */
void
d_durabilityStateRequestListenerAddRequest(
    d_durabilityStateRequestListener listener,
    d_durabilityStateRequest request)
{
    assert(d_durabilityStateRequestListenerIsValid(listener));
    assert(d_durabilityStateRequestIsValid(request));

    d_durabilityStateRequestListenerAnswer(listener, request);

    return;
}
