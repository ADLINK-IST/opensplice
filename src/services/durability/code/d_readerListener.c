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

#include "d__readerListener.h"
#include "d__listener.h"
#include "d__admin.h"
#include "d__subscriber.h"
#include "d__waitset.h"
#include "d__misc.h"
#include "d__durability.h"
#include "d_message.h"
#include "d_qos.h"
#include "d_networkAddress.h"
#include "u_observable.h"
#include "u_dataReader.h"
#include "v_event.h"
#include "v_observer.h"
#include "v_readerSample.h"
#include "v_dataReaderSample.h"
#include "v_state.h"
#include "os_report.h"

/**
 * Macro that checks the d_readerListener validity.
 * Because d_readerListener is an abstract class no type checking can
 * be done. The pointer can only be validated to be non-NULL.
 */
#define d_readerListenerIsValid(_this)  (_this != NULL)

struct createFieldArg
{
    const c_char * typeName;
    const c_char * fieldName;
    os_size_t      fieldOffset; /* result */
};

void
createField(
    v_public entity,
    c_voidp  argument )
{
    c_type type;
    c_metaObject userData;
    struct createFieldArg * arg = (struct createFieldArg *)argument;

    type = c_resolve(c_getBase(entity), arg->typeName);
    assert(type != NULL);
    userData = c_metaResolve(c_metaObject(type), arg->fieldName);
    c_free(type);

    arg->fieldOffset = c_property(userData)->offset;
    c_free(userData);
}

static void
d_readerListenerInitField(
    d_readerListener listener,
    d_subscriber   subscriber,
    const c_char * typeName)
{
    struct createFieldArg arg;
    const c_char* fieldName = "userData";

    assert(d_objectIsValid(d_object(listener), D_LISTENER));
    arg.typeName = typeName;
    arg.fieldName = fieldName;
    arg.fieldOffset = 0;
    (void)u_observableAction(u_observable(d_durabilityGetService(
                                          d_adminGetDurability(
                                              d_subscriberGetAdmin(subscriber)))),
                                          createField, &arg);
    listener->fieldOffset = arg.fieldOffset;
}

void
d_readerListenerInit(
    d_readerListener listener,
    d_listenerKind kind,
    d_listenerAction action,
    d_subscriber subscriber,
    const c_char* topicName,
    const c_char* fieldName,
    v_reliabilityKind reliability,
    v_historyQosKind historyKind,
    c_long historyDepth,
    os_threadAttr attr,
    d_objectDeinitFunc deinit)
{
    c_char* readerName;
    d_networkAddress addr;
    d_admin admin;

    v_readerQos readerQos;
    c_char *query;

    assert(d_readerListenerIsValid(listener));
    assert(d_subscriberIsValid(subscriber));

    if (listener) {
        /* Call super-init */
        d_listenerInit(d_listener(listener), kind, subscriber, action,
                       (d_objectDeinitFunc)deinit);
        /* Initialize readerListener */
        admin = d_listenerGetAdmin(d_listener(listener));
        readerName = (c_char*)(os_malloc(strlen(topicName) + 7));
        if (readerName) {
            os_sprintf(readerName, "%sReader", topicName);
        }
        addr = d_adminGetMyAddress(admin);
        listener->myAddr = addr->systemId;
        d_readerListenerInitField(listener, subscriber, fieldName);
        listener->takenSamples               = c_iterNew(NULL);
        listener->waitsetData                = NULL;
        listener->name                       = os_strdup(topicName);
        listener->samplesTotal               = 0;
        listener->samplesFromMe              = 0;
        listener->samplesForMe               = 0;
        listener->lastInsertTime             = OS_TIMEW_ZERO;
        listener->lastSourceTime             = OS_TIMEW_ZERO;
        listener->attr                       = attr;
        /* Each readerListener uses a DataReader that runs
         * on its own thread. Data that is received by the
         * DataReader is handled by that thread.
         */
        readerQos = d_readerQosNew(V_DURABILITY_VOLATILE, reliability);
        readerQos->history.v.kind = historyKind;
        readerQos->history.v.depth = historyDepth;
        readerQos->lifecycle.v.autopurge_nowriter_samples_delay = OS_DURATION_INIT(60,0);
        readerQos->lifecycle.v.autopurge_disposed_samples_delay = OS_DURATION_INIT(10,0);

        /* Message is meant for me or for all and not sent by me */

#define FILTER_EXPRESSION "select * from %s where " \
                          "parentMsg.addressee.systemId=%u OR " \
                          "parentMsg.addressee.systemId=0 AND " \
                          "parentMsg.senderAddress.systemId<>%u"

        query = (c_char*)(os_malloc(strlen(listener->name) + strlen(FILTER_EXPRESSION) + 32));
        os_sprintf(query, FILTER_EXPRESSION, topicName, addr->systemId, addr->systemId);
#undef FILTER_EXPRESSION

        listener->dataReader = u_dataReaderNew (d_subscriberGetSubscriber(subscriber), readerName, query, NULL, 0, readerQos);

        if (!listener->dataReader) {
            OS_REPORT(OS_ERROR, D_CONTEXT_DURABILITY, 0, "Could not create dataReader");
        } else {
            u_entityEnable(u_entity(listener->dataReader));
        }
        d_readerQosFree(readerQos);
        os_free(query);
        d_networkAddressFree(addr);
        os_free(readerName);
    }
}

void
d_readerListenerDeinit(
    d_readerListener listener)
{
    assert(d_readerListenerIsValid(listener));

    /* Stop the readerListener */
    d_readerListenerStop(listener);
    /* Deallocate */
    if (listener->name) {
        os_free(listener->name);
    }
    /* Remove the datareader that was created in d_readerListenerInitDataReader. */
    if (listener->dataReader) {
        d_listenerLock(d_listener(listener));
        u_objectFree(u_object (listener->dataReader));
        d_listenerUnlock(d_listener(listener));
    }
    if(listener->takenSamples) {
        c_iterFree(listener->takenSamples);
        listener->takenSamples = NULL;
    }
    /* Call super-deinit */
    d_listenerDeinit(d_listener(listener));
}

static v_actionResult
d_readerListenerCopyAll(
    c_object object,
    c_voidp copyArg)
{
    d_readerListener listener;
    v_actionResult result = 0;

    if(object != NULL) {
        v_actionResultSet(result, V_PROCEED);
        if (v_stateTest(v_readerSampleState(object), L_VALIDDATA)) {
            listener  = d_readerListener(copyArg);
            (void)c_iterAppend(listener->takenSamples, c_keep(object));
            listener->processMessage = TRUE;
        }
    }
    return result;
}

static c_ulong
d_readerListenerActionAll(
    u_object o,
    u_waitsetEvent event,
    c_voidp usrData)
{
    u_result result;
    d_readerListener listener;
    c_bool proceed;
    d_admin admin;
    d_durability durability;
    c_object sample;
    v_message readerMessage;

    listener = d_readerListener(usrData);
    admin = d_listenerGetAdmin(d_listener(listener));
    durability = d_adminGetDurability(admin);

    d_listenerLock(d_listener(listener));

    do {
        proceed = FALSE;
        listener->processMessage = FALSE;
        result = u_dataReaderTake(u_dataReader(o), U_STATE_ANY, d_readerListenerCopyAll, usrData, OS_DURATION_ZERO);

        if(result != U_RESULT_OK && result != U_RESULT_NO_DATA){
            OS_REPORT(OS_ERROR, D_CONTEXT_DURABILITY, 0, "Could not take data from reader (result: %d)", result);
        } else if(c_iterLength(listener->takenSamples)!= 0){
            d_listenerAction action = d_listenerGetAction(d_listener(listener));
            sample = (c_object)(c_iterTakeFirst(listener->takenSamples));

            while(sample && (d_durabilityMustTerminate(durability) == FALSE)){
                readerMessage = v_message(v_dataReaderSampleTemplate(sample)->message);

                listener->lastInsertTime = v_dataReaderSample(sample)->insertTime;
                listener->lastSourceTime = readerMessage->writeTime;
                listener->value = sample;

                if(action) {
                    action(d_listener(listener), C_DISPLACE(readerMessage, listener->fieldOffset));
                }

                c_free(sample);
                sample = (c_object)(c_iterTakeFirst(listener->takenSamples));
            }
            /* If while-loop stops, because durability terminates, free all
             * remaining samples;
             */
            while(sample){
                c_free(sample);
                sample = (c_object)(c_iterTakeFirst(listener->takenSamples));
            }
            listener->value = NULL;
            proceed = TRUE;
        }
    } while((result == U_RESULT_OK || result == U_RESULT_NO_DATA) && (proceed == TRUE) && (d_durabilityMustTerminate(durability) == FALSE));

    d_listenerUnlock(d_listener(listener));
    return (event->kind & V_EVENT_DATA_AVAILABLE);
}

c_bool
d_readerListenerStart(
    d_readerListener listener)
{
    c_bool result;
    u_object object;
    u_result ur;
    c_bool wsResult;
    d_waitset waitset;
    d_admin admin;
    d_subscriber subscriber;

    assert(listener);
    result = FALSE;
    assert(d_objectIsValid(d_object(listener), D_LISTENER));

    if(listener){
        d_listenerLock(d_listener(listener));
        object  = u_object(listener->dataReader);
        if (object) {
            if(d_listener(listener)->attached == FALSE){
                ur = u_observableSetListenerMask(u_observable(object), V_EVENT_DATA_AVAILABLE);

                if(ur == U_RESULT_OK){
                    listener->value = NULL;

                    admin       = d_listenerGetAdmin(d_listener(listener));
                    subscriber  = d_adminGetSubscriber(admin);
                    waitset     = d_subscriberGetWaitset(subscriber);

                    listener->waitsetData = d_waitsetEntityNew(
                            listener->name,
                            object, d_readerListenerActionAll,
                            V_EVENT_DATA_AVAILABLE | V_EVENT_TRIGGER,
                            listener->attr, listener);

                    wsResult    = d_waitsetAttach(waitset, listener->waitsetData);

                    if(wsResult == TRUE) {
                        ur = U_RESULT_OK;
                    } else {
                        ur = U_RESULT_ILL_PARAM;
                    }

                    if(ur == U_RESULT_OK){
                        d_listener(listener)->attached = TRUE;
                        result = TRUE;
                        d_listenerUnlock(d_listener(listener));
                        (void) u_observableNotify(u_observable(object));
                    } else {
                        d_listenerUnlock(d_listener(listener));
                    }
                } else {
                    d_listenerUnlock(d_listener(listener));
                }
            } else {
                d_listenerUnlock(d_listener(listener));
            }
        } else {
            d_listenerUnlock(d_listener(listener));
            result = TRUE;
        }
    }
    return result;
}

c_bool
d_readerListenerStop(
    d_readerListener listener)
{
    c_bool result;
    u_result ur;
    d_admin admin;
    d_subscriber subscriber;
    d_waitset waitset;

    assert(d_objectIsValid(d_object(listener), D_LISTENER));
    result = FALSE;

    if(listener){
        d_listenerLock(d_listener(listener));

        if(d_listener(listener)->attached == TRUE){
            admin      = d_listenerGetAdmin(d_listener(listener));
            subscriber = d_adminGetSubscriber(admin);
            waitset    = d_subscriberGetWaitset(subscriber);
            d_listenerUnlock(d_listener(listener));
            result     = d_waitsetDetach(waitset, listener->waitsetData);
            d_listenerLock(d_listener(listener));

            if(result == TRUE) {
                d_waitsetEntityFree(listener->waitsetData);
                ur = U_RESULT_OK;
            } else {
                ur = U_RESULT_ILL_PARAM;
            }

            if(ur == U_RESULT_OK){
                d_listener(listener)->attached = FALSE;
                result = TRUE;
            } else {
                assert(FALSE);
            }
        } else {
            result = TRUE;
        }
        d_listenerUnlock(d_listener(listener));
    }
    return result;
}

