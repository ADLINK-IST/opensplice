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
#include "v__reader.h"
#include "v__readerQos.h"
#include "v__observer.h"
#include "v_subscriber.h"
#include "v__entry.h"
#include "v_query.h"
#include "v__dataReader.h"
#include "v__deliveryService.h"
#include "v__groupStream.h"
#include "v__networkReader.h"
#include "v_entity.h"
#include "v_status.h"
#include "v_public.h"
#include "v__collection.h"
#include "v_event.h"
#include "v_group.h"
#include "v_time.h"
#include "v_proxy.h"
#include "v_policy.h"
#include "v__partition.h"
#include "v_observable.h"
#include "v_historicalDataRequest.h"


#include "os_report.h"
#include "os.h"

#define v_readerEntrySetLock(_this) \
        c_mutexLock(&_this->entrySet.mutex)

#define v_readerEntrySetUnlock(_this) \
        c_mutexUnlock(&_this->entrySet.mutex)

#define V_READER_LOCK(_this)   v_observerLock(v_observer(_this))
#define V_READER_UNLOCK(_this) v_observerUnlock(v_observer(_this))
#define V_READER_GET_LOCK(_this) (v_observer(_this)->mutex)

/**************************************************************
 * Private functions
 **************************************************************/

static c_bool
entryFree(
    c_object o,
    c_voidp arg)
{
    v_entry entry;

    OS_UNUSED_ARG(arg);
    assert(o != NULL);
    assert(arg == NULL);

    entry = v_entry(o);
    assert(entry != NULL);

    v_entryFree(entry);
    return TRUE;
}

/* This getHistoricalDataCommon function takes two optional parameters:
 * - the v_group parameter indicates the group from which to get the
 *   historical data.  NULL indicates all groups.
 * - the v_historicalDataRequest is an optional historical condition,
 *   the existence of which switches the function that is called.
 */
static c_bool
getHistoricalDataCommon(
    c_object o,
    v_group specificGroup,
    v_historicalDataRequest historicalDataRequest)
{
    v_entry entry;
    c_iter proxies;
    v_proxy proxy;
    v_group group;
    c_bool result;

    assert(o != NULL);

    entry = v_entry(o);
    assert(entry != NULL);
    assert(C_TYPECHECK(entry,v_entry));

    proxies = ospl_c_select(entry->groups, 0);
    proxy = c_iterTakeFirst(proxies);
    result = TRUE;

    while (proxy != NULL) {
        group = v_group(v_proxyClaim(proxy));
        if (group) {
            if (specificGroup == NULL || specificGroup == group) {
                if(historicalDataRequest == NULL) {
                    v_groupGetHistoricalData(group, entry);
                } else {
                    result = v_groupGetHistoricalDataWithCondition(group,
                        entry, historicalDataRequest);
                }
            }
            v_proxyRelease(proxy);
        }
        c_free(proxy);
        proxy = c_iterTakeFirst(proxies);
    }
    c_iterFree(proxies);
    return result;
}

static c_bool
getHistoricalData(
    c_object o,
    c_voidp arg)
{
    /* request the historical data for all of the reader's groups with
     * a v_historicalDataRequest that may or may not be set */
    v_historicalDataRequest historicalDataRequest = NULL;
    if (arg) {
        historicalDataRequest = (v_historicalDataRequest)arg;
    }

    return getHistoricalDataCommon (o, NULL, historicalDataRequest);
}

static c_bool
getHistoricalDataSpecificGroup(
    c_object o,
    c_voidp arg)
{
    /* request the historical data for only the specified group */
    return getHistoricalDataCommon (o, (v_group)arg, NULL);
}

static void
readerGetHistoricalData(
    v_reader r)
{
    assert(C_TYPECHECK(r, v_reader));

    if (r->qos->durability.kind != V_DURABILITY_VOLATILE) {
        v_readerWalkEntries(r, getHistoricalData, NULL);
    }
}

static void
readerGetHistoricalDataSpecificGroup(
    v_reader r,
    v_group g)
{
    assert(C_TYPECHECK(r, v_reader));
    assert(C_TYPECHECK(g, v_group));

    if (r->qos->durability.kind != V_DURABILITY_VOLATILE) {
        v_readerWalkEntries(r, getHistoricalDataSpecificGroup, g);
    }
}

/**************************************************************
 * constructor/destructor
 **************************************************************/
void
v_readerInit(
    v_reader r,
    const c_char *name,
    v_subscriber s,
    v_readerQos qos,
    v_statistics rs,
    c_bool enable)
{
    v_kernel kernel;

    assert(r != NULL);
    assert(s != NULL);
    assert(C_TYPECHECK(r,v_reader));
    assert(C_TYPECHECK(s,v_subscriber));
    assert(C_TYPECHECK(qos, v_readerQos));
    /* We demand the qos to be allocated in the kernel, by v_readerQosNew().
     * This way we are sure that the qos is consistent!
     */

    kernel = v_objectKernel(r);
    v_collectionInit(v_collection(r),name,rs,enable);

    r->subscriber = s;
    r->qos = c_keep(qos);
    r->subQos = c_keep(s->qos); /* reference is readonly */
    r->entrySet.entries = c_setNew(v_kernelType(kernel,K_ENTRY));
    c_mutexInit(&r->entrySet.mutex, SHARED_MUTEX);

    r->historicalDataRequest  = NULL;
    r->historicalDataComplete = FALSE;
    c_condInit(&r->historicalDataCondition, &(v_observer(r)->mutex), SHARED_COND);

}

void
v_readerFree(
    v_reader r)
{
    v_subscriber tmp;

    assert(C_TYPECHECK(r,v_reader));

    v_observerLock(r);
    assert (r->subscriber != NULL);
    tmp = v_subscriber(r->subscriber);
    r->subscriber = NULL;
    v_observerUnlock(r);

    /* For each readerInstance in each entry, v_subscriberRemoveReader will
     * eventually deliver an UNREGISTER message to the Reader using
     * v_dataReaderEntryWrite, which on its turn locks the reader itself.
     * For that reason, the entryFree function must be called when the readerLock
     * has been released.
     */
    v_subscriberRemoveReader(tmp, r);

    /* Free all entries. */
    v_readerWalkEntries(r, entryFree, NULL);

    /* Call inherited free */
    v_collectionFree(v_collection(r));
}

void
v_readerDeinit(
    v_reader r)
{
    assert(r != NULL);
    assert(C_TYPECHECK(r,v_reader));

    v_collectionDeinit(v_collection(r));
}

/**************************************************************
 * Protected functions
 **************************************************************/
c_bool
v_readerSubscribeGroup(
    v_reader _this,
    v_group group)
{
    c_bool result;

    result = FALSE;

    assert(C_TYPECHECK(_this, v_reader));
    switch(v_objectKind(_this)) {
    case K_DATAREADER:
        /* ES, dds1576: For the K_DATAREADER object we need to verify if
         * the access rights are correct. No subscriptions may be made onto
         * groups which have a v_accessMode of write only.
         */
        if(v_groupPartitionAccessMode(group) == V_ACCESS_MODE_READ_WRITE ||
           v_groupPartitionAccessMode(group) == V_ACCESS_MODE_READ)
        {
            result = v_dataReaderSubscribeGroup(v_dataReader(_this), group);
            /* need to get historical data only for the group that was just subscribed to */
            readerGetHistoricalDataSpecificGroup(_this, group);
        } else
        {
            result = FALSE;
        }
    break;
    case K_GROUPQUEUE:
        result = v_groupStreamSubscribeGroup(v_groupStream(_this), group);
    break;
    case K_NETWORKREADER:
        assert(FALSE);
    break;
    default:
        OS_REPORT_1(OS_ERROR,"v_readerSubscribeGroup failed",0,
                    "illegal reader kind (%d) specified",
                    v_objectKind(_this));
        assert(FALSE);
        result = FALSE;
    }
    return result;
}


c_bool
v_readerUnSubscribeGroup(
    v_reader reader,
    v_group group)
{
    c_bool result;

    assert(C_TYPECHECK(reader, v_reader));

    switch(v_objectKind(reader)) {
    case K_DATAREADER:
        result = v_dataReaderUnSubscribeGroup(v_dataReader(reader), group);
    break;
    case K_GROUPQUEUE:
        result = v_groupStreamUnSubscribeGroup(v_groupStream(reader), group);
    break;
    case K_NETWORKREADER:
        result = v_networkReaderUnSubscribeGroup(v_networkReader(reader), group);
    break;
    default:
        OS_REPORT_1(OS_ERROR,"v_readerUnSubscribeGroup failed",0,
                    "illegal reader kind (%d) specified",
                    v_objectKind(reader));
        assert(FALSE);
        result = FALSE;
    }

    return result;
}


v_result
v_readerSetQos(
    v_reader r,
    v_readerQos qos)
{
    v_result result;
    v_qosChangeMask cm;

    assert(C_TYPECHECK(r,v_reader));

    v_readerEntrySetLock(r);
    result = v_readerQosSet(r->qos, qos, v_entity(r)->enabled, &cm);
    v_readerEntrySetUnlock(r);
    if ((result == V_RESULT_OK) && (cm != 0)) {
        if (v_objectKind(r) == K_DATAREADER) {
            v_dataReaderNotifyChangedQos(v_dataReader(r));
        }
    }
    return result;
}

c_bool
v_readerWalkEntries(
    v_reader r,
    c_action action,
    c_voidp arg)
{
    c_bool result;

    assert(C_TYPECHECK(r,v_reader));

    v_readerEntrySetLock(r);
    result = c_setWalk(r->entrySet.entries, action, arg);
    v_readerEntrySetUnlock(r);

    return result;
}

c_iter
v_readerCollectEntries(
    v_reader r)
{
    c_iter result;

    assert(C_TYPECHECK(r,v_reader));

    if(r){
        v_readerEntrySetLock(r);
        result = ospl_c_select(r->entrySet.entries, 0);
        v_readerEntrySetUnlock(r);
    } else {
        result = NULL;
    }
    return result;
}

v_entry
v_readerAddEntry(
    v_reader r,
    v_entry e)
{
    v_entry found;

    assert(C_TYPECHECK(r,v_reader));
    assert(C_TYPECHECK(e,v_entry));

    v_readerEntrySetLock(r);
    found = c_setInsert(r->entrySet.entries, e);
    v_readerEntrySetUnlock(r);

    return c_keep(found);
}

v_entry
v_readerRemoveEntry(
    v_reader r,
    v_entry e)
{
    v_entry found;

    assert(C_TYPECHECK(r,v_reader));

    v_readerEntrySetLock(r);
    found = c_keep(c_remove(r->entrySet.entries, e, NULL, NULL));
    v_readerEntrySetUnlock(r);

    return found;
}

/**************************************************************
 * Public functions
 **************************************************************/
c_bool
v_readerSubscribe(
    v_reader r,
    v_partition d)
{
    c_bool result;

    assert(C_TYPECHECK(r,v_reader));
    result = FALSE;

    switch(v_objectKind(r)) {
    case K_DATAREADER:
        result = v_dataReaderSubscribe(v_dataReader(r),d);
    break;
    case K_DELIVERYSERVICE:
        result = v_deliveryServiceSubscribe(v_deliveryService(r),d);
    break;
    case K_GROUPQUEUE:
        result = v_groupStreamSubscribe(v_groupStream(r),d);
    break;
    case K_NETWORKREADER:
        assert(FALSE);
    break;
    default:
        OS_REPORT_1(OS_ERROR,"v_readerSubscribe failed",0,
                    "illegal reader kind (%d) specified",
                    v_objectKind(r));
        assert(FALSE);
        result = FALSE;
    break;
    }
    readerGetHistoricalData(r);

    return result;
}

c_bool
v_readerUnSubscribe(
    v_reader r,
    v_partition d)
{
    assert(C_TYPECHECK(r,v_reader));

    switch(v_objectKind(r)) {
    case K_DATAREADER:
        return v_dataReaderUnSubscribe(v_dataReader(r),d);
    case K_DELIVERYSERVICE:
        return v_deliveryServiceUnSubscribe(v_deliveryService(r),d);
    case K_GROUPQUEUE:
        return v_groupStreamUnSubscribe(v_groupStream(r),d);
    case K_NETWORKREADER:
        return v_networkReaderUnSubscribe(v_networkReader(r),d);
    default:
        OS_REPORT_1(OS_ERROR,"v_readerUnSubscribe failed",0,
                    "illegal reader kind (%d) specified",
                    v_objectKind(r));
        assert(FALSE);
    }

    return TRUE;
}

v_result
v_readerGetDeadlineMissedStatus(
    v_reader _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg)
{
    v_result result;
    v_status status;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_reader));

    result = V_RESULT_PRECONDITION_NOT_MET;
    if (_this != NULL) {
        V_READER_LOCK(_this);
        status = v_entity(_this)->status;
        result = action(&v_readerStatus(status)->deadlineMissed, arg);
        if (reset) {
            v_statusReset(status, V_EVENT_DEADLINE_MISSED);
        }
        v_readerStatus(status)->deadlineMissed.totalChanged = 0;
        V_READER_UNLOCK(_this);
    }
    return result;
}

v_result
v_readerGetIncompatibleQosStatus(
    v_reader _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg)
{
    v_result result;
    v_status status;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_reader));

    result = V_RESULT_PRECONDITION_NOT_MET;
    if (_this != NULL) {
        V_READER_LOCK(_this);
        status = v_entity(_this)->status;
        result = action(&v_readerStatus(status)->incompatibleQos, arg);
        if (reset) {
            v_statusReset(status, V_EVENT_INCOMPATIBLE_QOS);
        }
        v_readerStatus(status)->incompatibleQos.totalChanged = 0;
        V_READER_UNLOCK(_this);
    }
    return result;
}

v_result
v_readerGetSampleRejectedStatus(
    v_reader _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg)
{
    v_result result;
    v_status status;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_reader));

    result = V_RESULT_PRECONDITION_NOT_MET;
    if (_this != NULL) {
        V_READER_LOCK(_this);
        status = v_entity(_this)->status;
        result = action(&v_readerStatus(status)->sampleRejected, arg);
        if (reset) {
            v_statusReset(status, V_EVENT_SAMPLE_REJECTED);
        }
        v_readerStatus(status)->sampleRejected.totalChanged = 0;
        V_READER_UNLOCK(_this);
    }
    return result;
}

v_result
v_readerGetSampleLostStatus(
    v_reader _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg)
{
    v_result result;
    v_status status;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_reader));

    result = V_RESULT_PRECONDITION_NOT_MET;
    if (_this != NULL) {
        V_READER_LOCK(_this);
        status = v_entity(_this)->status;
        result = action(&v_readerStatus(status)->sampleLost, arg);
        if (reset) {
            v_statusReset(status, V_EVENT_SAMPLE_LOST);
        }
        v_readerStatus(status)->sampleLost.totalChanged = 0;
        V_READER_UNLOCK(_this);
    }
    return result;
}

v_result
v_readerGetLivelinessChangedStatus(
    v_reader _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg)
{
    v_result result;
    v_status status;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_reader));

    result = V_RESULT_PRECONDITION_NOT_MET;
    if (_this != NULL) {
        V_READER_LOCK(_this);
        status = v_entity(_this)->status;
        result = action(&v_readerStatus(status)->livelinessChanged, arg);
        if (reset) {
            v_statusReset(status, V_EVENT_LIVELINESS_CHANGED);
        }
        v_readerStatus(status)->livelinessChanged.activeChanged = 0;
        v_readerStatus(status)->livelinessChanged.inactiveChanged = 0;
        V_READER_UNLOCK(_this);
    }
    return result;
}

v_result
v_readerGetTopicMatchStatus(
    v_reader _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg)
{
    v_result result;
    v_status status;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_reader));

    result = V_RESULT_PRECONDITION_NOT_MET;
    if (_this != NULL) {
        V_READER_LOCK(_this);
        status = v_entity(_this)->status;
        result = action(&v_readerStatus(status)->subscriptionMatch, arg);
        if (reset) {
            v_statusReset(status, V_EVENT_TOPIC_MATCHED);
        }
        v_readerStatus(status)->subscriptionMatch.totalChanged = 0;
        v_readerStatus(status)->subscriptionMatch.currentChanged = 0;
        V_READER_UNLOCK(_this);
    }
    return result;
}

struct historicalWaitArg {
    c_time _expire_time;
    c_bool _status;
};

static c_bool
waitForHistoricalData(
    c_object o,
    c_voidp arg)
{
    v_entry entry;
    c_iter proxies;
    v_proxy proxy;
    v_group group;
    c_time waitTime;
    struct historicalWaitArg *parms = (struct historicalWaitArg *)arg;

    assert(o != NULL);
    assert(arg != NULL);

    entry = v_entry(o);
    assert(entry != NULL);
    assert(C_TYPECHECK(entry,v_entry));

    proxies = ospl_c_select(entry->groups, 0);
    proxy = c_iterTakeFirst(proxies);
    while ((proxy != NULL) && (parms->_status == TRUE)) {
        group = v_group(v_proxyClaim(proxy));
        if (group) {
            if (group->complete == FALSE) {
                waitTime  = c_timeSub(parms->_expire_time, v_timeGet());
                if (c_timeCompare(waitTime, C_TIME_ZERO) == C_GT) {
                    parms->_status = v_groupWaitForComplete(group, waitTime);
                } else {
                    parms->_status = FALSE; /* time out */
                }
            }
            v_proxyRelease(proxy);
        }
        c_free(proxy);
        proxy = c_iterTakeFirst(proxies);
    }
    c_iterFree(proxies);
    return parms->_status;
}

c_bool
v_readerWaitForHistoricalData(
    v_reader r,
    c_time timeout)
{
    struct historicalWaitArg arg;
    c_iter entries;
    c_object e;

    v_readerEntrySetLock(r);
    entries = ospl_c_select(r->entrySet.entries, 0);
    v_readerEntrySetUnlock(r);

    arg._expire_time = c_timeAdd(v_timeGet(), timeout);
    arg._status = TRUE;

    e = c_iterTakeFirst(entries);
    while (e != NULL) {
        if (arg._status == TRUE) {
            if (r->qos->durability.kind == V_DURABILITY_VOLATILE) {
                getHistoricalData(e, NULL);
            }
            waitForHistoricalData(e, &arg);
        }
        c_free(e);
        e = c_iterTakeFirst(entries);
    }
    c_iterFree(entries);

    return(arg._status);
}

v_historyResult
v_readerWaitForHistoricalDataWithCondition(
    v_reader _this,
    c_char* filter,
    c_char* params[],
    c_ulong paramsLength,
    c_time minSourceTime,
    c_time maxSourceTime,
    struct v_resourcePolicy *resourceLimits,
    c_time timeout)
{
    c_iter entries;
    c_object e;
    v_historyResult result;
    v_historicalDataRequest request;
    c_bool doRequest, doWait;
    v_result durabilityAvailable;
    struct historicalWaitArg arg;
    C_STRUCT(v_event) event;
    c_bool success;

    success = TRUE;
    arg._expire_time = c_timeAdd(v_timeGet(), timeout);
    arg._status = TRUE;

    durabilityAvailable = v_kernelWaitForDurabilityAvailability(
            v_objectKernel(_this), timeout);

    if(durabilityAvailable == V_RESULT_OK){
        request = v_historicalDataRequestNew(v_objectKernel(_this), filter, params,
                    paramsLength, minSourceTime, maxSourceTime, resourceLimits);

        if(request){
            V_READER_LOCK(_this);

            if(_this->historicalDataRequest) {
                /* Historical data request already in progress or complete, check
                 * whether request is equal to the original one.
                 */
                doRequest = FALSE;

                if(v_historicalDataRequestEquals(request, _this->historicalDataRequest)){
                    /* Request is equal to original request*/
                    result = V_HISTORY_RESULT_OK;

                    if(_this->historicalDataComplete){
                        /* Request has already been fulfilled. Consider this call
                         * a no-operation.
                         */
                        doWait = FALSE;
                    } else {
                        /* Request is still in progress, wait for data to arrive*/
                        doWait = TRUE;
                    }
                } else {
                    /* The requested parameters are not equal to the originally
                     * requested set. Return a precondition not met.
                     */
                    doWait = FALSE;
                    result = V_HISTORY_RESULT_PRE_NOT_MET;
                }
                c_free(request);
            } else {
                /* No active request, so validate it now.*/
                if(v_historicalDataRequestIsValid(request, _this)){
                    /* This request is valid, so request data.*/
                    doRequest = TRUE;
                    doWait    = TRUE;
                    result    = V_HISTORY_RESULT_OK;
                    _this->historicalDataRequest = request;
                } else {
                    /* Request is not valid, so return bad parameter.*/
                    doRequest = FALSE;
                    doWait    = FALSE;
                    result    = V_HISTORY_RESULT_BAD_PARAM;
                    c_free(request);
                }
            }
            V_READER_UNLOCK(_this);
        } else {
            doRequest = FALSE;
            doWait    = FALSE;
            result    = V_HISTORY_RESULT_ERROR;
        }

        if(doWait){
            v_readerEntrySetLock(_this);
            entries = ospl_c_select(_this->entrySet.entries, 0);
            v_readerEntrySetUnlock(_this);

            if(doRequest){
                /* Historical data must be requested, since this is the first time
                 * the operation is called and the request is valid.
                 */
                if (_this->qos->durability.kind == V_DURABILITY_VOLATILE) {
                    /* If reader is volatile, the historical data from the
                     * group(s) has/have not been retrieved yet, so do it now.
                     */
                    e = c_iterTakeFirst(entries);

                    while (e != NULL && success) {
                        success = getHistoricalData(e, _this->historicalDataRequest);
                        c_free(e);
                        e = c_iterTakeFirst(entries);
                    }
                    c_iterFree(entries);
                }
                if(success){
                    event.kind = V_EVENT_HISTORY_REQUEST;
                    event.source = v_publicHandle(v_public(_this));
                    event.userData = _this->historicalDataRequest;
                    v_observableNotify(v_observable(v_objectKernel(_this)),&event);
                } else {
                    result = V_HISTORY_RESULT_BAD_PARAM;
                }
            }

            V_READER_LOCK(_this);

            if(!_this->historicalDataComplete && success){
                if (c_timeCompare(timeout, C_TIME_INFINITE) != C_EQ) {
                    if (c_condTimedWait(&_this->historicalDataCondition,
                                        &V_READER_GET_LOCK(_this),
                                        timeout) != SYNC_RESULT_SUCCESS)
                    {
                        result = V_HISTORY_RESULT_TIMEOUT;
                    }
                } else if (c_condWait(&_this->historicalDataCondition,
                                &V_READER_GET_LOCK(_this)) != SYNC_RESULT_SUCCESS)
                {
                        result = V_HISTORY_RESULT_TIMEOUT;
                }
                assert( (result == V_HISTORY_RESULT_OK) ==
                         _this->historicalDataComplete);
            }
            V_READER_UNLOCK(_this);

        }
    } else {
        /* The durability service is not running, so pre-condition not met. */
        OS_REPORT(OS_WARNING, "v_readerWaitForHistoricalDataWithCondition", 0,
                "wait_for_historical_data_w_condition() could not be " \
                "performed as no durability service is available. Please " \
                "reconfigure your domain to include a durability service.");
        result = V_HISTORY_RESULT_PRE_NOT_MET;
    }
    return result;
}


void
v_readerNotifyHistoricalDataAvailable(
    v_reader _this)
{
    assert(C_TYPECHECK(_this, v_reader));

    if(_this){
        V_READER_LOCK(_this);
        _this->historicalDataComplete = TRUE;
        c_condBroadcast(&_this->historicalDataCondition);
        V_READER_UNLOCK(_this);
    }
    return;
}
