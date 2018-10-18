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

#include "d__remoteReaderListener.h"
#include "d__waitset.h"
#include "d__admin.h"
#include "d__listener.h"
#include "d__durability.h"
#include "d__configuration.h"
#include "d__listener.h"
#include "d__misc.h"
#include "d__table.h"
#include "d__subscriber.h"
#include "d__thread.h"
#include "d_qos.h"
#include "d_networkAddress.h"
#include "d__fellow.h"
#include "d__client.h"
#include "os_heap.h"
#include "os_report.h"
#include "u_participant.h"
#include "u_subscriber.h"
#include "u_listener.h"
#include "u_types.h"
#include "v_builtin.h"
#include "v_event.h"
#include "v_state.h"
#include "v_message.h"
#include "v_dataReaderSample.h"
#include "v_dataReaderInstance.h"
#include "kernelModuleI.h"

/**
 * Macro that checks the d_remoteReaderListener validity.
 * Because d_remoteReaderListener is a concrete class typechecking is required.
 */
#define             d_remoteReaderListenerIsValid(_this)   \
    d_listenerIsValidKind(d_listener(_this), D_REMOTE_READER_LISTENER)

/**
 * \brief The <code>d_remoteReaderListener</code> cast macro.
 *
 * This macro casts an object to a <code>d_remoteReaderListener</code> object.
 */
#define d_remoteReaderListener(_this) ((d_remoteReaderListener)(_this))


C_STRUCT(d_remoteReaderListener) {
    C_EXTENDS(d_listener);
    u_dataReader dataReader;
    d_waitsetEntity waitsetData;
    c_bool terminate;
    d_subscriber subscriber;
};

static
c_ulong
getReaderFlag(
    char *topic_name)
{
    if (topic_name) {
        if (strcmp(topic_name, D_NAMESPACES_TOPIC_NAME) == 0)                  return D_NAMESPACES_READER_FLAG;
        if (strcmp(topic_name, D_NAMESPACES_REQ_TOPIC_NAME) == 0)              return D_NAMESPACESREQUEST_READER_FLAG;
        if (strcmp(topic_name, D_STATUS_TOPIC_NAME) == 0)                      return D_STATUS_READER_FLAG;
        if (strcmp(topic_name, D_GROUPS_REQ_TOPIC_NAME) == 0)                  return D_GROUPSREQUEST_READER_FLAG;
        if (strcmp(topic_name, D_DELETE_DATA_TOPIC_NAME) == 0)                 return D_DELETEDATA_READER_FLAG;
        if (strcmp(topic_name, D_NEWGROUP_TOPIC_NAME) == 0)                    return D_NEWGROUP_READER_FLAG;
        if (strcmp(topic_name, D_SAMPLE_REQ_TOPIC_NAME) == 0)                  return D_SAMPLREQUEST_READER_FLAG;
        if (strcmp(topic_name, D_SAMPLE_CHAIN_TOPIC_NAME) == 0)                return D_SAMPLECHAIN_READER_FLAG;
        if (strcmp(topic_name, D_CAPABILITY_TOPIC_NAME) == 0)                  return D_CAPABILITY_READER_FLAG;
        if (strcmp(topic_name, D_DURABILITY_STATE_REQUEST_TOPIC_NAME) == 0)    return D_DURABILITYSTATEREQUEST_READER_FLAG;
        if (strcmp(topic_name, D_HISTORICAL_DATA_REQUEST_TOPIC_NAME) == 0)     return D_HISTORICALDATAREQUEST_READER_FLAG;
        if (strcmp(topic_name, D_DURABILITY_STATE_TOPIC_NAME) == 0)            return D_DURABILITYSTATE_READER_FLAG;
        if (strcmp(topic_name, D_HISTORICAL_DATA_TOPIC_NAME) == 0)             return D_HISTORICALDATA_READER_FLAG;
    }
    /* default, no durability reader */
    return 0;
}



static u_actionResult
processSubscriptionInfo(
    c_object object,
    c_voidp copyArg)
{
    d_remoteReaderListener listener;
    d_admin admin;
    d_fellow fellow, fellow2;
    d_client client, duplicate;
    d_networkAddress fellowAddr, clientAddr;
    u_actionResult result = V_PROCEED;
    v_dataReaderSample sample;
    v_dataReaderInstance instance;
    v_message message;
    v_subscriptionInfoTemplate template;
    const struct v_subscriptionInfo *subscriptionInfo;
    c_ulong reader;
    d_durability durability;
    d_configuration config;

    if (object != NULL) {
       listener = d_remoteReaderListener(copyArg);
        admin = d_listenerGetAdmin(d_listener(listener));
        durability = d_adminGetDurability(admin);
        config = d_durabilityGetConfiguration(durability);
        sample = v_dataReaderSample(object);
        instance = v_readerSampleInstance(sample);
        message = v_dataReaderSampleTemplate(object)->message;
        template = (v_subscriptionInfoTemplate)message;

        /* The fellow address is NOT the message->writerGID because
         * the message is written by the local DDSI service. The address
         * of the actual fellow is the address of the subscriptionInfo->participant_key,
         * so use that one. */
       if (template) {
            subscriptionInfo = &template->userData;
            reader = getReaderFlag(subscriptionInfo->topic_name);

            /* Detect durability readers */
            if ((reader & (D_BASIC_DURABILITY_READER_FLAGS | D_CAPABILITY_READER_FLAG)) != 0) {
                fellowAddr = d_networkAddressNew(subscriptionInfo->participant_key.systemId, 0, 0);
                fellow = d_adminGetUnconfirmedFellow(admin, fellowAddr);
                if (!fellow) {
                     fellow = d_adminGetFellow(admin, fellowAddr);
                }
                if (!fellow) {
                    /* The fellow does not yet exist, try to create the fellow
                     * and add it to the list of unconfirmed fellows
                     * Assume the state is D_STATE_INIT until an update comes along */
                    fellow2 = d_fellowNew(fellowAddr, D_STATE_INIT, FALSE);
                    fellow = d_adminAddFellow(admin, fellow2);
                    /* If the fellow is NULL then it is terminating.
                       In that case we do not have to do anthing */
                    if (fellow != fellow2) {
                        d_fellowFree(fellow2);
                    }
                }
                if (fellow) {
                    if (v_dataReaderInstanceStateTest(instance, L_DISPOSED)) {
                       /* The durability reader has been disposed.
                        * Reset the corresponding flag from the fellow */
                        d_fellowRemoveReader(fellow, reader);
                    } else {
                        /* A durability reader is detected.
                         * Set the corresponding flag from the fellow */
                        d_fellowAddReader(fellow, reader);
                    }
                    d_fellowFree(fellow);
                }
                d_networkAddressFree(fellowAddr);

            /* Detect client-durability readers; only needed when client-durability enabled
             * and waitforRemoteReaders is */
            } else if ( (config->clientDurabilityEnabled) && (config->waitForRemoteReaders) && ((reader & D_FULL_CLIENT_DURABILITY_READER_FLAGS) != 0) ) {
                clientAddr = d_networkAddressNew(subscriptionInfo->participant_key.systemId, 0, 0);
                client = d_clientNew(clientAddr);
                if ((duplicate = d_adminAddClient(admin, client)) != client) {
                    d_clientFree(client);
                    client = duplicate;
                }
                /* ClientId will NOT be set yet because no historicalDataRequest of durabilityStateRequest is received */
                if (client) {
                    if (v_dataReaderInstanceStateTest(instance, L_DISPOSED)) {
                        d_clientRemoveReader(client, reader);
                    } else {
                        d_clientAddReader(client, reader);
                    }
                    d_clientFree(client);
                }
                d_networkAddressFree(clientAddr);
            }
        }
    }
    return result;
}


void
d_remoteReaderListenerCheckReaders(
    d_remoteReaderListener listener)
{
    u_result ur;
    d_admin admin = d_listenerGetAdmin(d_listener(listener));
    d_durability durability = d_adminGetDurability(admin);

    assert(d_remoteReaderListenerIsValid(listener));

    /* Read DCPSSubcriptions and process them */
    d_lockLock(d_lock(listener));
    ur = u_dataReaderTake(listener->dataReader,
                U_STATE_ANY_SAMPLE | U_STATE_ANY_VIEW | U_STATE_DISPOSED_INSTANCE,
                processSubscriptionInfo, listener, OS_DURATION_ZERO);
    if (ur != U_RESULT_OK && ur != U_RESULT_NO_DATA) {
        OS_REPORT(OS_ERROR, D_CONTEXT_DURABILITY, ur,
                "Failed to read data from DCPSSubscription reader (result: %s)", u_resultImage(ur));
        d_durabilityTerminate(durability, TRUE);
    }
    ur = u_dataReaderRead(listener->dataReader, U_STATE_ANY, processSubscriptionInfo,
                listener, OS_DURATION_ZERO);
    if (ur != U_RESULT_OK && ur != U_RESULT_NO_DATA) {
        OS_REPORT(OS_ERROR, D_CONTEXT_DURABILITY, ur,
                "Failed to read data from DCPSSubscription reader (result: %s)", u_resultImage(ur));
        d_durabilityTerminate(durability, TRUE);
    }
    d_lockUnlock(d_lock(listener));
}

static c_ulong
d_remoteReaderListenerAction(
    u_object o,
    u_waitsetEvent event,
    c_voidp usrData)
{
    d_remoteReaderListener listener;
    u_result ur;
    d_admin admin;
    d_durability durability;
    d_thread self = d_threadLookupSelf();

    listener = d_remoteReaderListener(usrData);

    assert(d_remoteReaderListenerIsValid(listener));

    d_threadAwake(self);
    d_lockLock(d_lock(listener));
    /* Read DCPSSubcriptions and process them */
    ur = u_dataReaderTake(u_dataReader(o), U_STATE_ANY_SAMPLE | U_STATE_ANY_VIEW | U_STATE_DISPOSED_INSTANCE, processSubscriptionInfo, listener, OS_DURATION_ZERO);
    if (ur != U_RESULT_OK && ur != U_RESULT_NO_DATA) {
        admin = d_listenerGetAdmin(d_listener(listener));
        durability = d_adminGetDurability(admin);
        OS_REPORT(OS_ERROR, D_CONTEXT_DURABILITY, ur,
                "Failed to read data from dcpsHeartbeatReader (result: %s)", u_resultImage(ur));
        d_durabilityTerminate(durability, TRUE);
    }
    ur = u_dataReaderRead(u_dataReader(o), U_STATE_NOT_READ_SAMPLE | U_STATE_ANY_VIEW | U_STATE_ANY_INSTANCE, processSubscriptionInfo, listener, OS_DURATION_ZERO);
    if (ur != U_RESULT_OK && ur != U_RESULT_NO_DATA) {
        admin = d_listenerGetAdmin(d_listener(listener));
        durability = d_adminGetDurability(admin);
        OS_REPORT(OS_ERROR, D_CONTEXT_DURABILITY, ur,
                "Failed to read data from dcpsHeartbeatReader (result: %s)", u_resultImage(ur));
        d_durabilityTerminate(durability, TRUE);
    }
    d_lockUnlock(d_lock(listener));
    d_threadAsleep(self, ~0u);
    return event->kind;
}

static void
d_remoteReaderListenerDeinit(
    d_remoteReaderListener listener)
{
    assert(d_remoteReaderListenerIsValid(listener));

    /* Stop the remoteReaderListener */
    d_remoteReaderListenerStop(listener);

    if (listener->waitsetData) {
        d_waitsetEntityFree(listener->waitsetData);
        listener->waitsetData = NULL;
    }
    if (listener->dataReader) {
        u_objectFree(u_object(listener->dataReader));
        listener->dataReader = NULL;
    }
    listener->subscriber = NULL;
    /* Call super-deinit */
    d_listenerDeinit(d_listener(listener));
}

d_remoteReaderListener
d_remoteReaderListenerNew(
    d_subscriber subscriber)
{
    d_remoteReaderListener listener;
    d_admin admin;
    d_durability durability;
    u_participant uParticipant;
    v_gid gid;
    c_value ps[1];
    v_readerQos readerQos = NULL;
    os_threadAttr attr;

    assert(d_subscriberIsValid(subscriber));

    /* Allocate remoteReaderListener object */
    if ((listener = d_remoteReaderListener(os_malloc(C_SIZEOF(d_remoteReaderListener)))) == NULL) {
        goto err_alloc_listener;
    }
    /* Call super-init */
    d_listenerInit(d_listener(listener), D_REMOTE_READER_LISTENER, subscriber, NULL,
                   (d_objectDeinitFunc)d_remoteReaderListenerDeinit);
    /* Initialize remoteReaderListener */
    admin = d_listenerGetAdmin(d_listener(listener));
    durability = d_adminGetDurability(admin);
    if ((uParticipant = u_participant(d_durabilityGetService(durability))) == NULL) {
        goto err_uparticipant;
    }
    listener->terminate = FALSE;
    listener->subscriber = subscriber;
    /* Create a reader that listens to remote DCPSSubscription
     * topics. The reader uses the builtin subscriber, and
     * only listens to topics that have a different systemId
     * than myself. */
    if ((readerQos = d_readerQosNew(V_DURABILITY_TRANSIENT, V_RELIABILITY_RELIABLE)) == NULL) {
        goto err_readerQos;
    }
    readerQos->history.v.kind = V_HISTORY_KEEPLAST;
    readerQos->history.v.depth = 1;
    gid = u_observableGid((u_observable)subscriber->builtinSubscriber);
    ps[0].kind = V_ULONG;
    ps[0].is.ULong = gid.systemId;
    listener->dataReader = u_subscriberCreateDataReader(
        subscriber->builtinSubscriber,
        "remoteReaderListenerReader",
        "select * from " V_SUBSCRIPTIONINFO_NAME " where key.systemId <> %0",
        ps, 1,
        readerQos);
    if (listener->dataReader == NULL) {
        goto err_dataReader;
    }
    if(u_entityEnable(u_entity(listener->dataReader)) != U_RESULT_OK) {
        goto err_dataReaderEnable;
    }
    os_threadAttrInit(&attr);
    listener->waitsetData = d_waitsetEntityNew(
                    "remoteReader",
                    u_object(listener->dataReader),
                    d_remoteReaderListenerAction,
                    V_EVENT_DATA_AVAILABLE,
                    attr, listener);
    assert(listener->waitsetData);
    /* Obtain historical data */
    (void) u_dataReaderWaitForHistoricalData(listener->dataReader, OS_DURATION_ZERO);
    d_readerQosFree(readerQos);
    return listener;

err_dataReaderEnable:
    u_objectFree(u_object(listener->dataReader));
err_dataReader:
    d_readerQosFree(readerQos);
err_readerQos:
err_uparticipant:
    d_remoteReaderListenerFree(listener);
err_alloc_listener:
    listener = NULL;
    return NULL;
}


void
d_remoteReaderListenerFree(
    d_remoteReaderListener listener)
{
    assert(d_remoteReaderListenerIsValid(listener));

    d_objectFree(d_object(listener));
}


c_bool
d_remoteReaderListenerStart(
    d_remoteReaderListener listener)
{
    c_bool result;
    d_waitset waitset;
    u_result ur;

    result = FALSE;

    assert(d_remoteReaderListenerIsValid(listener));

    if (listener) {
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
                ur = u_dataReaderRead(listener->dataReader, U_STATE_ANY, processSubscriptionInfo, listener, OS_DURATION_ZERO);
                if (ur != U_RESULT_OK && ur != U_RESULT_NO_DATA) {
                    OS_REPORT(OS_ERROR, D_CONTEXT_DURABILITY, ur,
                            "Failed to read data from remoteReaderListenerReader (result: %s)", u_resultImage(ur));
                } else {
                    d_listener(listener)->attached = TRUE;
                    result = TRUE;
                }
            }
        } else {
            result = TRUE;
        }
        d_lockUnlock(d_lock(listener));
    }
    return result;
}


c_bool
d_remoteReaderListenerStop(
    d_remoteReaderListener listener)
{
    c_bool result;
    d_admin admin;
    d_subscriber subscriber;
    d_waitset waitset;

    assert(d_remoteReaderListenerIsValid(listener));

    result = FALSE;

    if (listener) {
        d_lockLock(d_lock(listener));

        if (d_listener(listener)->attached == TRUE) {
            admin = d_listenerGetAdmin(d_listener(listener));
            subscriber = d_adminGetSubscriber(admin);
            waitset = d_subscriberGetWaitset(subscriber);
            d_lockUnlock(d_lock(listener));
            result = d_waitsetDetach(waitset, listener->waitsetData);
            d_lockLock(d_lock(listener));

            if (result == TRUE) {
                d_listener(listener)->attached = FALSE;
            }
        } else {
            result = TRUE;
        }
        d_lockUnlock(d_lock(listener));
    }
    return result;
}
