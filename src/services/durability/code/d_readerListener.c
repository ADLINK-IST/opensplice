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

#include "d_readerListener.h"
#include "d__readerListener.h"
#include "d__listener.h"
#include "d_listener.h"
#include "d_admin.h"
#include "d_misc.h"
#include "d_message.h"
#include "d_qos.h"
#include "d_durability.h"
#include "d_subscriber.h"
#include "d_networkAddress.h"
#include "d_waitset.h"
#include "u_dataReader.h"
#include "u_dispatcher.h"
#include "v_event.h"
#include "v_readerSample.h"
#include "v_dataReaderSample.h"
#include "v_state.h"
#include "os_report.h"

void
d_readerListenerInit(
    d_readerListener listener,
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

    if(listener){
        d_listenerInit(d_listener(listener), subscriber, action, d_readerListenerDeinit);
        admin = d_listenerGetAdmin(d_listener(listener));
        readerName = (c_char*)(os_malloc(strlen(topicName) + 7));
        os_sprintf(readerName, "%sReader", topicName);

        addr = d_adminGetMyAddress(admin);
        listener->myAddr = addr->systemId;
        d_networkAddressFree(addr);

        d_readerListenerInitField(listener, subscriber, fieldName);

        listener->message                    = NULL;
        listener->waitsetData                = NULL;
        listener->name                       = os_strdup(topicName);
        listener->samplesTotal               = 0;
        listener->samplesFromMe              = 0;
        listener->samplesForMe               = 0;
        listener->lastInsertTime.seconds     = 0;
        listener->lastInsertTime.nanoseconds = 0;
        listener->lastSourceTime.seconds     = 0;
        listener->lastSourceTime.nanoseconds = 0;
        listener->attr                       = attr;
        listener->deinit                     = deinit;

        d_readerListenerInitDataReader(listener,
                                       subscriber,
                                       readerName,
                                       reliability,
                                       historyKind,
                                       historyDepth);

        os_free(readerName);
    }
}

void
d_readerListenerDeinit(
    d_object object)
{
    d_readerListener listener;

    assert(d_objectIsValid(object, D_LISTENER));

    if(object){
        listener = d_readerListener(object);
        d_readerListenerStop(listener);

        if(listener->deinit){
            listener->deinit(object);
        }
        d_listenerLock(d_listener(listener));
        u_dataReaderFree(listener->dataReader);
        os_free(listener->name);
        d_listenerUnlock(d_listener(listener));
    }
}

void
d_readerListenerFree(
    d_readerListener listener)
{
    assert(d_objectIsValid(d_object(listener), D_LISTENER));

    if(listener){
        d_listenerFree(d_listener(listener));
    }
}

c_ulong
d_readerListenerAction(
    u_dispatcher o,
    u_waitsetEvent event,
    c_voidp usrData)
{
    u_result result;
    d_readerListener listener;
    c_bool proceed;
    d_admin admin;
    d_durability durability;

    listener = d_readerListener(usrData);
    admin = d_listenerGetAdmin(d_listener(listener));
    durability = d_adminGetDurability(admin);

    d_listenerLock(d_listener(listener));

    do {
        proceed = FALSE;
        listener->message = NULL;
        listener->processMessage = FALSE;
        result = u_dataReaderTake(u_dataReader(o), d_readerListenerCopy, usrData);

        {
            d_admin admin;
            d_durability durability;
            admin = d_listenerGetAdmin((d_listener)listener);
            durability = d_adminGetDurability(admin);
            d_printTimedEvent(durability, D_LEVEL_FINEST,
                                  D_THREAD_UNSPECIFIED,
                                  "DEBUG: dispatcher %p: dataReaderTake (result=%d, processMessage=%d)\n",
                                  o,result, listener->processMessage);
        }

        if(result != U_RESULT_OK){
            OS_REPORT_1(OS_ERROR, D_CONTEXT_DURABILITY, 0,
                    "Could not take data from reader (result: %d)", result);
        } else if(listener->message != NULL){
            if(listener->processMessage == TRUE) {
                d_readerListenerProcessAction(listener->message, usrData);
            }
            c_free(listener->value);
            listener->value = NULL;
            proceed = TRUE;
        }
    } while((result == U_RESULT_OK) && (proceed == TRUE) && (d_durabilityMustTerminate(durability) == FALSE));

    d_listenerUnlock(d_listener(listener));
    return (event->events & V_EVENT_DATA_AVAILABLE);
}

v_actionResult
d_readerListenerCopy(
    c_object object,
    c_voidp copyArg)
{
    d_readerListener listener;
    d_message        message;
    v_message        readerMessage;
    v_actionResult result = 0;

    {
        d_admin admin;
        d_durability durability;
        listener  = d_readerListener(copyArg);
        admin = d_listenerGetAdmin((d_listener)listener);
        durability = d_adminGetDurability(admin);
        d_printTimedEvent(durability, D_LEVEL_FINEST,
                              D_THREAD_UNSPECIFIED,
                              "DEBUG: readerListenerCopy: object=%p\n",
                              object);
    }

    if(object != NULL) {
        {
            d_admin admin;
            d_durability durability;
            listener  = d_readerListener(copyArg);
            admin = d_listenerGetAdmin((d_listener)listener);
            durability = d_adminGetDurability(admin);
            d_printTimedEvent(durability, D_LEVEL_FINEST,
                                  D_THREAD_UNSPECIFIED,
                                  "DEBUG: readerListenerCopy: state=%p\n",
                                  v_stateTest(v_readerSampleState(object), L_VALIDDATA));
        }
        if (v_stateTest(v_readerSampleState(object), L_VALIDDATA)) {
            listener  = d_readerListener(copyArg);
            readerMessage = v_message(v_dataReaderSampleTemplate(object)->message);
            message   = C_DISPLACE(readerMessage, listener->fieldOffset);
            listener->lastInsertTime = v_dataReaderSample(object)->insertTime;
            listener->lastSourceTime = readerMessage->writeTime;
            listener->value = c_keep(object);

            if(listener->value){
                listener->message = message;
            }
            listener->processMessage = TRUE;
        } else {
            /* Ignore data that is not valid but continue with the read action */
            v_actionResultSet(result, V_PROCEED);
        }
    }
    return result;
}

void
d_readerListenerProcessAction(
    d_message message,
    c_voidp copyArg)
{
    d_readerListener listener;
    d_listenerAction action;

    listener  = d_readerListener(copyArg);
    action = d_listenerGetAction(d_listener(listener));

    if(action) {
        action(d_listener(listener), message);
    }
}

c_bool
d_readerListenerStart(
    d_readerListener listener)
{
    c_bool result;
    u_dispatcher dispatcher;
    u_result ur;
    d_waitsetAction action;
    c_bool wsResult;
    d_waitset waitset;
    d_admin admin;
    d_subscriber subscriber;

    assert(listener);
    result = FALSE;
    assert(d_objectIsValid(d_object(listener), D_LISTENER));

    if(listener){
        d_listenerLock(d_listener(listener));
        dispatcher  = u_dispatcher(listener->dataReader);
        action      = d_readerListenerAction;

        if(d_listener(listener)->attached == FALSE){
            ur = u_dispatcherSetEventMask(dispatcher, V_EVENT_DATA_AVAILABLE);

            if(ur == U_RESULT_OK){
                listener->value = NULL;
                listener->message = NULL;

                admin       = d_listenerGetAdmin(d_listener(listener));
                subscriber  = d_adminGetSubscriber(admin);
                waitset     = d_subscriberGetWaitset(subscriber);

                listener->waitsetData = d_waitsetEntityNew(
                            listener->name,
                            dispatcher, action,
                            V_EVENT_DATA_AVAILABLE, listener->attr, listener);

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
                    u_dispatcherNotify(dispatcher);
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
            result     = d_waitsetDetach(waitset, listener->waitsetData);

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

void
d_readerListenerInitDataReader(
    d_readerListener listener,
    d_subscriber subscriber,
    const c_char* name,
    v_reliabilityKind reliability,
    v_historyQosKind historyKind,
    c_long historyDepth)
{
    v_readerQos readerQos;
    u_subscriber s;
    d_networkAddress unaddressed, myAddr;
    d_admin admin;
    c_char *query;
    q_expr expr;

    assert(d_objectIsValid(d_object(listener), D_LISTENER));

    if(listener && subscriber){
        readerQos = d_readerQosNew(V_DURABILITY_VOLATILE, reliability);
        readerQos->history.kind = historyKind;
        readerQos->history.depth = historyDepth;
        readerQos->lifecycle.autopurge_nowriter_samples_delay.seconds = 60;
        readerQos->lifecycle.autopurge_nowriter_samples_delay.nanoseconds = 0;
        readerQos->lifecycle.autopurge_disposed_samples_delay.seconds = 10;
        readerQos->lifecycle.autopurge_disposed_samples_delay.nanoseconds = 0;
        s = d_subscriberGetSubscriber(subscriber);
        admin       = d_subscriberGetAdmin(subscriber);
        myAddr      = d_adminGetMyAddress(admin);
        unaddressed = d_networkAddressUnaddressed();

        /*
         * Message is:
         * - meant for me or
         * - meant for all and not sent by me
         */
#define FILTER_EXPRESSION "select * from %s where " \
                          "parentMsg.addressee.systemId=%u OR " \
                          "parentMsg.addressee.systemId=%u AND " \
                          "parentMsg.senderAddress.systemId!=%u"

        query = (c_char*)(os_malloc(strlen(listener->name) +
                                    strlen(FILTER_EXPRESSION) + 32));
        os_sprintf(query, FILTER_EXPRESSION, listener->name,
                myAddr->systemId, unaddressed->systemId, myAddr->systemId);

        expr = q_parse(query);

        listener->dataReader =
                u_dataReaderNew (s, name, expr, NULL, readerQos, TRUE);

        q_dispose(expr);
        os_free(query);

#undef FILTER_EXPRESSION
        d_networkAddressFree(myAddr);
        d_networkAddressFree(unaddressed);
        d_readerQosFree(readerQos);
    }
}

struct createFieldArg
{
    const c_char * typeName;
    const c_char * fieldName;
    c_ulong        fieldOffset; /* result */
};

void
createField(
    v_entity entity,
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

void
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
    u_entityAction(u_entity(d_durabilityGetService(
                                d_adminGetDurability(
                                    d_subscriberGetAdmin(subscriber)))),
                                createField, &arg);
    listener->fieldOffset = arg.fieldOffset;
    assert(listener->fieldOffset > 0);
}
