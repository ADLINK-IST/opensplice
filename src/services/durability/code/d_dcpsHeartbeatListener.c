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

#include "d__dcpsHeartbeatListener.h"
#include "d__subscriber.h"
#include "d__admin.h"
#include "d__listener.h"
#include "d__durability.h"
#include "d_qos.h"
#include "d__waitset.h"
#include "d__misc.h"
#include "d__fellow.h"
#include "d__conflictResolver.h"
#include "u_types.h"
#include "u_participant.h"
#include "u_subscriber.h"
#include "u_dataReader.h"
#include "u_observable.h"
#include "v_builtin.h"
#include "v_event.h"
#include "v_readerSample.h"
#include "v_message.h"
#include "v_kernel.h"
#include "v_observer.h"
#include "os_heap.h"
#include "os_report.h"
#include "v_dataReaderSample.h"
#include "v_dataReaderInstance.h"

#define d_dcpsHeartbeatListener(_this) ((d_dcpsHeartbeatListener)(_this))

#define d_dcpsHeartbeatListenerIsValid(_this)   \
    d_listenerIsValidKind(d_listener(_this), D_DCPS_HEARTBEAT_LISTENER)

C_STRUCT(d_dcpsHeartbeatListener){
    C_EXTENDS(d_listener);
    d_waitsetEntity waitsetData;
    u_dataReader dataReader;
    d_subscriber subscriber;

    /* Federations that have not been discovered by means of the durability
     * protocol. They will be discovered by means of the DCPSHeartbeat before
     * durability protocol messages will trigger adding of a fellow. To ensure
     * no periods of (potential) loss of reliability are missed, this list
     * is needed next to the regular fellow administration.
     *
     * A federation will also be in this list in case no durability service runs
     * in that federation.
     */
    d_table discoveredFederations;
};

static void
processHeartbeat(
    v_dataReaderSample heartbeatSample,
    d_dcpsHeartbeatListener listener);

static u_actionResult
takeHeartbeats(
    c_object object,
    c_voidp copyArg);

static c_ulong
d_dcpsHeartbeatListenerAction(
    u_object o,
    u_waitsetEvent event,
    c_voidp usrData)
{
    d_dcpsHeartbeatListener listener;
    u_result result;
    d_admin admin;
    d_durability durability;
    c_iter heartbeatSamples = c_iterNew(NULL);

    listener = d_dcpsHeartbeatListener(usrData);

    assert(d_dcpsHeartbeatListenerIsValid(listener));

    d_listenerLock(d_listener(listener));
    /* Read the latest DCPSHeartbeat and process it */
    result = u_dataReaderTake(u_dataReader(o), U_STATE_ANY, takeHeartbeats, heartbeatSamples, OS_DURATION_ZERO);
    if (result == U_RESULT_OK) {
        v_dataReaderSample sample;

        while ((sample = v_dataReaderSample(c_iterTakeFirst(heartbeatSamples)))) {
            processHeartbeat(sample, listener);
            c_free(v_readerSampleInstance(sample));
            c_free(sample);
        }
    } else if (result != U_RESULT_NO_DATA) {
        admin = d_listenerGetAdmin(d_listener(listener));
        durability = d_adminGetDurability(admin);
        OS_REPORT(OS_ERROR, D_CONTEXT_DURABILITY, result,
                "Failed to read data from dcpsHeartbeatReader (result: %s)", u_resultImage(result));
        d_durabilityTerminate(durability, TRUE);
    }
    d_listenerUnlock(d_listener(listener));
    c_iterFree(heartbeatSamples);

    return event->kind;
}

static void
d_dcpsHeartbeatListenerDeinit(
    d_dcpsHeartbeatListener listener)
{
    assert(d_dcpsHeartbeatListenerIsValid(listener));

    /* Stop the listener */
    d_dcpsHeartbeatListenerStop(listener);

    if (listener->waitsetData) {
        d_waitsetEntityFree(listener->waitsetData);
        listener->waitsetData = NULL;
    }
    if (listener->dataReader) {
        u_objectFree(u_object(listener->dataReader));
        listener->dataReader = NULL;
    }
    if (listener->discoveredFederations) {
        d_tableFree(listener->discoveredFederations);
        listener->discoveredFederations = NULL;
    }
    listener->subscriber = NULL;
    /* Call super-deinit */
    d_listenerDeinit(d_listener(listener));

    return;
}

static c_bool
d_dcpsHeartbeatListenerInit(
    d_dcpsHeartbeatListener listener,
    d_subscriber subscriber)
{
    d_admin admin;
    d_durability durability;
    u_participant participant;
    c_bool result;
    v_readerQos readerQos;
    os_threadAttr attr;
    v_gid gid;
    c_value ps[1];

    assert(d_subscriberIsValid(subscriber));
    assert(listener);

    /* Call super-init */
    d_listenerInit(d_listener(listener), D_DCPS_HEARTBEAT_LISTENER, subscriber, NULL,
                          (d_objectDeinitFunc)d_dcpsHeartbeatListenerDeinit);

    result = FALSE;
    admin = d_listenerGetAdmin(d_listener(listener));
    durability = d_adminGetDurability(admin);
    participant = u_participant(d_durabilityGetService(durability));

    if (participant != NULL) {
        listener->subscriber = subscriber;
        listener->discoveredFederations = d_tableNew(d_fellowCompare, d_fellowFree);
        assert(listener->discoveredFederations);

        if (subscriber->builtinSubscriber != NULL) {
            /* The reader uses V_DURABILITY_VOLATILE for backwards compatibility.
             * In order to obtain the new V_DURABILITY_TRANSIENT samples as well,
             * a wait_for_historical_data is done.
             */
            readerQos = d_readerQosNew(V_DURABILITY_VOLATILE, V_RELIABILITY_BESTEFFORT);

            if(readerQos){
                readerQos->history.v.kind = V_HISTORY_KEEPALL;

                gid = u_observableGid((u_observable)subscriber->builtinSubscriber);
                ps[0].kind = V_ULONG;
                ps[0].is.ULong = gid.systemId;

                listener->dataReader = u_subscriberCreateDataReader(
                    subscriber->builtinSubscriber,
                    "DCPSHeartbeat_Reader",
                    "select * from " V_HEARTBEATINFO_NAME " where id.systemId <> %0",
                    ps, 1, readerQos);

                if (listener->dataReader != NULL) {
                    if (u_entityEnable(u_entity(listener->dataReader)) == U_RESULT_OK) {
                        os_threadAttrInit(&attr);
                        listener->waitsetData = d_waitsetEntityNew(
                                        "dcpsHeartbeatListener",
                                        u_object(listener->dataReader),
                                        d_dcpsHeartbeatListenerAction,
                                        V_EVENT_DATA_AVAILABLE,
                                        attr, listener);
                        assert(listener->waitsetData);

                        (void) u_dataReaderWaitForHistoricalData(listener->dataReader, OS_DURATION_ZERO);
                        result = TRUE;
                    }
                }
                u_readerQosFree(readerQos);
            }
        }
    }
    return result;
}

d_dcpsHeartbeatListener
d_dcpsHeartbeatListenerNew(
    d_subscriber subscriber)
{
    d_dcpsHeartbeatListener listener;
    c_bool success;

    assert(d_subscriberIsValid(subscriber));

    listener = d_dcpsHeartbeatListener(os_malloc(C_SIZEOF(d_dcpsHeartbeatListener)));

    if(listener){
        success = d_dcpsHeartbeatListenerInit(listener, subscriber);

        if(success != TRUE){
            os_free(listener);
            listener = NULL;
        }
    }
    return listener;
}

void
d_dcpsHeartbeatListenerFree(
    d_dcpsHeartbeatListener listener)
{
    assert(d_dcpsHeartbeatListenerIsValid(listener));

    d_objectFree(d_object(listener));
}

c_bool
d_dcpsHeartbeatListenerStop(
    d_dcpsHeartbeatListener listener)
{
    c_bool result;
    d_admin admin;
    d_subscriber subscriber;
    d_waitset waitset;

    assert(d_dcpsHeartbeatListenerIsValid(listener));

    result = FALSE;

    if (listener) {
        d_listenerLock(d_listener(listener));

        if (d_listener(listener)->attached == TRUE) {
            admin = d_listenerGetAdmin(d_listener(listener));
            subscriber = d_adminGetSubscriber(admin);
            waitset = d_subscriberGetWaitset(subscriber);
            d_listenerUnlock(d_listener(listener));
            result = d_waitsetDetach(waitset, listener->waitsetData);
            d_listenerLock(d_listener(listener));

            if (result == TRUE) {
                d_listener(listener)->attached = FALSE;
            }
        } else {
            result = TRUE;
        }
        d_listenerUnlock(d_listener(listener));
    }
    return result;
}

c_bool
d_dcpsHeartbeatListenerStart(
    d_dcpsHeartbeatListener listener)
{
    c_bool result;
    d_waitset waitset;
    u_result ur;
    c_iter heartbeatSamples = c_iterNew(NULL);

    assert(d_dcpsHeartbeatListenerIsValid(listener));

    result = FALSE;

    if (listener) {
        d_listenerLock(d_listener(listener));

        if (d_listener(listener)->attached == FALSE) {
            waitset = d_subscriberGetWaitset(listener->subscriber);
            result = d_waitsetAttach(waitset, listener->waitsetData);

            if (result == TRUE) {
                /* A V_DATA_AVAILABLE event is only generated when new data
                 * arrives. It is NOT generated when historical data is inserted.
                 * In case this durability service receives historical
                 * DCPSHeartbeat from another durability service that
                 * was started earlier, all these DCPSHeartbeat are
                 * inserted in the reader at creation time. To read these
                 * we must explicitly trigger a take action.
                 */
                ur = u_dataReaderTake(listener->dataReader, U_STATE_ANY, takeHeartbeats, heartbeatSamples, OS_DURATION_ZERO);
                if (ur == U_RESULT_OK || ur == U_RESULT_NO_DATA) {
                    v_dataReaderSample sample;

                    while ((sample = v_dataReaderSample(c_iterTakeFirst(heartbeatSamples)))) {
                        processHeartbeat(sample, listener);
                        c_free(v_readerSampleInstance(sample));
                        c_free(sample);
                    }
                    d_listener(listener)->attached = TRUE;
                    result = TRUE;
                } else {
                    OS_REPORT(OS_ERROR, D_CONTEXT_DURABILITY, ur,
                            "Failed to read data from dcpsHeartbeatReader (result: %s)",
                            u_resultImage(ur));
                }
            }
        } else {
            result = TRUE;
        }
        d_listenerUnlock(d_listener(listener));
    }
    c_iterFree(heartbeatSamples);
    return result;
}

static u_actionResult
takeHeartbeats(
    c_object object,
    c_voidp copyArg)
{
    if (object) {
        c_iter heartbeatSamples = (c_iter) copyArg;
        (void) c_iterAppend(heartbeatSamples, c_keep(v_dataReaderSample(object)));
        /* Because the backref to the instance is not refCounted, also keep the instance itself. */
        (void) c_keep(v_readerSampleInstance(object));
    }
    return V_PROCEED;
}

static void
processHeartbeat(
    v_dataReaderSample sample,
    d_dcpsHeartbeatListener listener)
{
    d_admin admin;
    d_durability durability;
    v_dataReaderInstance instance;
    v_message message;
    v_heartbeatInfoTemplate template;
    d_networkAddress address;
    d_fellow fellow, existingFellow;
    c_long prevDisposeCount;
    const struct v_heartbeatInfo* heartbeat;

    assert(sample != NULL);
    admin = d_listenerGetAdmin(d_listener(listener));
    durability = d_adminGetDurability(admin);
    instance = v_readerSampleInstance(sample);
    message = v_dataReaderSampleTemplate(sample)->message;
    template = (v_heartbeatInfoTemplate)message;

    if(template){
        heartbeat = &template->userData;
        address = d_networkAddressNew(heartbeat->id.systemId, 0, 0);
        fellow = d_adminGetFellow(admin, address);
        d_printTimedEvent(durability, D_LEVEL_FINE,
                                      "Incoming federation heartbeat for federation '%u', state = '%u', instance state = '%u', noWriterCount = '%d', period = %d.%d\n",
                                      address->systemId,
                                      v_messageState(message),
                                      v_dataReaderInstanceState(instance),
                                      instance->noWritersCount,
                                      heartbeat->period.seconds,
                                      heartbeat->period.nanoseconds);

        if(fellow != NULL) {
            if (v_dataReaderInstanceStateTest(instance, L_DISPOSED)) {
                d_printTimedEvent(durability, D_LEVEL_FINEST,
                      "Known fellow '%u' has been disconnected, removing it from my administration\n",
                      address->systemId);

                existingFellow = d_adminRemoveFellow(admin, fellow, FALSE);

                if(existingFellow){
                    d_fellowFree(existingFellow);
                }
                existingFellow = d_tableRemove(listener->discoveredFederations, fellow);

                if(existingFellow){
                    d_fellowFree(existingFellow);
                }
                /* Generate a federation disconnect event */
                d_conflictMonitorCheckFederationDisconnected(admin->conflictMonitor);
            } else {
                 existingFellow = d_tableInsert(listener->discoveredFederations, fellow);

                 if(!existingFellow){
                     /* Keeping fellow as it has been inserted into the
                      * table, but will be freed later on.
                      */
                     existingFellow = d_fellow(d_objectKeep(d_object(fellow)));
                     d_printTimedEvent(durability, D_LEVEL_FINEST,
                          "Discovered federation '%u' (disposeCount = %d), but already knew fellow.\n",
                          address->systemId, sample->disposeCount);
                     (void)d_fellowSetLastDisposeCount(existingFellow, sample->disposeCount);
                 } else {
                     prevDisposeCount = d_fellowSetLastDisposeCount(existingFellow, sample->disposeCount);
                     /* In case the disposeCount has increased, it means
                      * that the fellow has been disconnected for a while
                      * and is now connected again before I saw the
                      * disconnect. It means reliability has been lost for
                      * a while and I'll have to remove the fellow and
                      * (re-)align with it.
                      */
                     if(prevDisposeCount != sample->disposeCount){
                         d_printTimedEvent(durability, D_LEVEL_FINEST,
                             "Fellow '%u' got disconnected, but reconnected already before I noticed the disconnect. "
                             "Removing fellow recover from (potential) loss of reliability and trigger (re)alignment.\n",
                             address->systemId);
                         existingFellow = d_adminRemoveFellow(admin, fellow, FALSE);

                         if(existingFellow){
                             d_fellowFree(existingFellow);
                         }
                         /* Generate a federation disconnect event */
                         d_conflictMonitorCheckFederationDisconnected(admin->conflictMonitor);
                     } else {
                         d_printTimedEvent(durability, D_LEVEL_FINEST,
                              "Fellow '%u' (disposeCount = %d) is still connected.\n",
                              address->systemId, sample->disposeCount);
                     }
                 }
            }
            d_fellowFree(fellow);
        } else if (v_dataReaderInstanceStateTest(instance, L_DISPOSED)) {
            fellow = d_fellowNew(address, D_STATE_INIT, FALSE);
            existingFellow = d_tableRemove(listener->discoveredFederations, fellow);
            d_fellowFree(fellow);

            /* Fellow may never have been seen at all, in which case it
             * won't exist in the table;
             */
            if(existingFellow){
                d_printTimedEvent(durability, D_LEVEL_FINEST,
                    "Federation '%u' got disconnected. I don't know any fellow durability service for it.\n",
                    address->systemId);
                d_fellowFree(existingFellow);
            } else {
                d_printTimedEvent(durability, D_LEVEL_FINE,
                    "Federation '%u' got disconnected, but I never saw it anyway, so ignoring.\n",
                    address->systemId);
            }
            /* Generate a federation disconnect event */
            d_conflictMonitorCheckFederationDisconnected(admin->conflictMonitor);
        } else {
            fellow = d_fellowNew(address, D_STATE_INIT, FALSE);
            existingFellow = d_tableInsert(listener->discoveredFederations, fellow);

            /* Federation already exists */
            if(existingFellow){
                d_fellowFree(fellow); /* throw away the newly created one */
                prevDisposeCount = d_fellowSetLastDisposeCount(existingFellow, sample->disposeCount);
                d_printTimedEvent(durability, D_LEVEL_FINEST,
                    "Updating known federation '%u' (disposeCount: %d -> %d). I haven't discovered any fellow for it yet.\n",
                    address->systemId, prevDisposeCount, sample->disposeCount);
            } else {
                d_printTimedEvent(durability, D_LEVEL_FINEST,
                    "Discovered new federation '%u' (disposeCount = %d). I haven't discovered any fellow for it yet.\n",
                    address->systemId, sample->disposeCount);
                (void)d_fellowSetLastDisposeCount(fellow, sample->disposeCount);
            }
        }

        d_printTimedEvent(durability, D_LEVEL_FINER,
            "I currently know %u federations.\n",
             d_tableSize(listener->discoveredFederations));

        if(v_readerSampleTestState(sample, L_VALIDDATA) && c_timeIsInfinite(heartbeat->period)) {
            d_conflict conflict = d_conflictNew(D_CONFLICT_HEARTBEAT_PROCESSED, address, NULL, NULL);
            d_conflictSetId(conflict, durability);
            d_printTimedEvent(durability, D_LEVEL_FINEST, "Adding heartbeat processed (for federation %u) conflict %d to the conflict resolver queue\n",
                    address->systemId, d_conflictGetId(conflict));

            d_conflictResolverAddConflict(durability->admin->conflictResolver, conflict);
        }

        d_networkAddressFree(address);
    }
}
