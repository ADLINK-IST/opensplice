/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#include "v__kernel.h"
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
#include "v_proxy.h"
#include "v_policy.h"
#include "v_spliced.h"
#include "v_durabilityClient.h"
#include "v__partition.h"
#include "v_observable.h"
#include "v_historicalDataRequest.h"


#include "os_report.h"
#include "vortex_os.h"

#if 0
#define _TRACE_EVENTS_ printf
#else
#define _TRACE_EVENTS_(...)
#endif

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
static v_result
getHistoricalDataCommon(
    c_object o,
    v_historicalDataRequest historicalDataRequest,
    c_bool openTransactions)
{
    v_entry entry;
    c_iter proxies;
    v_proxy proxy;
    v_group group;
    v_result result = V_RESULT_OK;

    assert(o != NULL);

    entry = v_entry(o);
    assert(entry != NULL);
    assert(C_TYPECHECK(entry,v_entry));

    proxies = ospl_c_select(entry->groups, 0);
    proxy = c_iterTakeFirst(proxies);

    while ((proxy != NULL) && (result == V_RESULT_OK)) {
        group = v_group(v_proxyClaim(proxy));
        if (group) {
            if(historicalDataRequest == NULL) {
                result = v_groupGetHistoricalData(group, entry, openTransactions);
            } else {
                result = v_groupGetHistoricalDataWithCondition(group, entry, historicalDataRequest);
            }
            v_proxyRelease(proxy);
        }
        c_free(proxy);
        proxy = c_iterTakeFirst(proxies);
    }
    c_iterFree(proxies);
    return result;
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
    v_collectionInit(v_collection(r), name, enable);

    r->subscriber = s;
    r->qos = c_keep(qos);
    r->subQos = c_keep(s->qos); /* reference is readonly */
    r->entrySet.entries = c_setNew(v_kernelType(kernel,K_ENTRY));
    if (c_mutexInit(c_getBase(r), &r->entrySet.mutex) != SYNC_RESULT_SUCCESS) {
            OS_REPORT(OS_ERROR, "v_readerInit", 0,
                        "Failed to initialize mutex for reader '%s'", name);
           goto err_mutexInit;
    }
    r->historicalDataRequest  = NULL;
    r->historicalDataComplete = FALSE;
    c_condInit(c_getBase(r), &r->historicalDataCondition, &(v_observer(r)->mutex));
    return;

err_mutexInit:
    c_free(r->entrySet.entries);
    return;
}

void
v_readerFree(
    v_reader r)
{
    v_subscriber subscriber;

    assert(C_TYPECHECK(r,v_reader));
    assert(r->subscriber != NULL);

    V_READER_LOCK(r);
    subscriber = r->subscriber;
    r->subscriber = NULL;
    V_READER_UNLOCK(r);

    /*
     * For each readerInstance in each entry, v_subscriberRemoveReader will
     * eventually deliver an UNREGISTER message to the Reader using
     * v_dataReaderEntryWrite, which on its turn locks the reader itself.
     *
     * For that reason, we don't do any locking here so that the entryFree
     * function is called without the readerLock being locked.
     *
     * Also see OSPL-3348
     */
    v_subscriberRemoveReader(subscriber,r);

    /* Free all entries */
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
        } else
        {
            result = FALSE;
        }
    break;
    case K_GROUPQUEUE:
        result = v_groupStreamSubscribeGroup(v_groupStream(_this), group);
    break;
    case K_NETWORKREADER:
        result = FALSE;
        (void)result;
        assert(FALSE);
    break;
    default:
        OS_REPORT(OS_CRITICAL,"v_readerSubscribeGroup failed",V_RESULT_ILL_PARAM,
                    "illegal reader kind (%d) specified",
                    v_objectKind(_this));
        result = FALSE;
        (void)result;
        assert(FALSE);
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
        OS_REPORT(OS_CRITICAL,"v_readerUnSubscribeGroup failed",V_RESULT_ILL_PARAM,
                    "illegal reader kind (%d) specified",
                    v_objectKind(reader));
        assert(FALSE);
        result = FALSE;
    }

    return result;
}

v_readerQos
v_readerGetQos(
    v_reader _this)
{
    v_readerQos qos;

    assert(C_TYPECHECK(_this,v_reader));

    V_READER_LOCK(_this);
    qos = c_keep(_this->qos);
    V_READER_UNLOCK(_this);

    return qos;
}

v_result
v_readerSetQos(
    v_reader _this,
    v_readerQos tmpl)
{
    v_result result;
    v_kernel kernel;
    v_readerQos qos;
    v_qosChangeMask cm;

    assert(C_TYPECHECK(_this,v_reader));

    kernel = v_objectKernel(_this);
    result = v_readerQosCheck(tmpl);
    if (result == V_RESULT_OK) {
        qos = v_readerQosNew(kernel, tmpl);
        if (qos != NULL) {
            V_READER_LOCK(_this);
            result = v_readerQosCompare(_this->qos, qos, v_entityEnabled(v_entity(_this)), &cm);
            if ((result == V_RESULT_OK) && (cm != 0)) {
                c_free(_this->qos);
                _this->qos = c_keep(qos);
                V_READER_UNLOCK(_this);
                if (v_objectKind(_this) == K_DATAREADER) {
                    v_dataReaderNotifyChangedQos(v_dataReader(_this));
                }
            } else {
                V_READER_UNLOCK(_this);
            }
            c_free(qos);
        } else {
            result = V_RESULT_OUT_OF_MEMORY;
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
v_result
v_readerSubscribe(
    v_reader r,
    v_partition d)
{
    v_result result = V_RESULT_INTERNAL_ERROR;

    assert(C_TYPECHECK(r,v_reader));

    switch(v_objectKind(r)) {
    case K_DATAREADER:
        if (v_dataReaderSubscribe(v_dataReader(r),d)) {
            result = V_RESULT_OK;
        }
    break;
    case K_DELIVERYSERVICE:
        if (v_deliveryServiceSubscribe(v_deliveryService(r),d)) {
            result = V_RESULT_OK;
        }
    break;
    case K_GROUPQUEUE:
        if (v_groupStreamSubscribe(v_groupStream(r),d)) {
            result = V_RESULT_OK;
        }
    break;
    case K_NETWORKREADER:
        (void)result;
        assert(FALSE);
    break;
    default:
        OS_REPORT(OS_CRITICAL,"v_readerSubscribe failed",V_RESULT_ILL_PARAM,
                    "illegal reader kind (%d) specified",
                    v_objectKind(r));
        result = V_RESULT_ILL_PARAM;
        (void)result;
        assert(FALSE);
    break;
    }

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
        OS_REPORT(OS_CRITICAL,"v_readerUnSubscribe failed",V_RESULT_ILL_PARAM,
                    "illegal reader kind (%d) specified",
                    v_objectKind(r));
        assert(FALSE);
    }

    return TRUE;
}

static c_bool
collectEntries(
    c_object o,
    c_voidp arg)
{
    c_iter *list = (c_iter *)arg;
    *list = c_iterAppend(*list, c_keep(o));
    return TRUE;
}

v_result
v_readerGetHistoricalData(
    v_reader _this)
{
    c_iter list = NULL;
    c_object o;
    v_result result;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_reader));

    result = V_RESULT_PRECONDITION_NOT_MET;
    if (_this != NULL) {
        result = V_RESULT_OK;
        if (_this->qos->durability.v.kind != V_DURABILITY_VOLATILE) {
            (void)v_readerWalkEntries(_this, collectEntries, &list);
            while (((o = c_iterTakeFirst(list)) != NULL) && (result == V_RESULT_OK)) {
                result = getHistoricalDataCommon(o, NULL, FALSE);
                c_free(o);
            }
            c_iterFree(list);
        }
    }
    return result;
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
            v_statusReset(status, V_EVENT_REQUESTED_DEADLINE_MISSED);
_TRACE_EVENTS_("v_readerGetDeadlineMissedStatus::v_statusReset(0x%x, 0x%x) reader 0x%x\n",
               status, V_EVENT_REQUESTED_DEADLINE_MISSED, _this);
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
    c_ulong i;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_reader));

    result = V_RESULT_PRECONDITION_NOT_MET;
    if (_this != NULL) {
        V_READER_LOCK(_this);
        status = v_entity(_this)->status;
        result = action(&v_readerStatus(status)->incompatibleQos, arg);
        if (reset) {
            v_statusReset(status, V_EVENT_REQUESTED_INCOMPATIBLE_QOS);
_TRACE_EVENTS_("v_readerGetDeadlineMissedStatus::v_statusReset(0x%x, 0x%x) reader 0x%x\n",
               status, V_RESULT_PRECONDITION_NOT_MET, _this);
        }
        v_readerStatus(status)->incompatibleQos.totalChanged = 0;
        for (i=0; i<V_POLICY_ID_COUNT; i++) {
            v_readerStatus(status)->incompatibleQos.policyCount[i] = 0;
        }
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
_TRACE_EVENTS_("v_readerGetDeadlineMissedStatus::v_statusReset(0x%x, 0x%x) reader 0x%x\n",
               status, V_EVENT_SAMPLE_REJECTED, _this);
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
_TRACE_EVENTS_("v_readerGetDeadlineMissedStatus::v_statusReset(0x%x, 0x%x) reader 0x%x\n",
               status, V_EVENT_SAMPLE_LOST, _this);
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
_TRACE_EVENTS_("v_readerGetDeadlineMissedStatus::v_statusReset(0x%x, 0x%x) reader 0x%x\n",
               status, V_EVENT_LIVELINESS_CHANGED, _this);
        }
        v_readerStatus(status)->livelinessChanged.activeChanged = 0;
        v_readerStatus(status)->livelinessChanged.inactiveChanged = 0;
        V_READER_UNLOCK(_this);
    }
    return result;
}

v_result
v_readerGetSubscriptionMatchedStatus(
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
            v_statusReset(status, V_EVENT_SUBSCRIPTION_MATCHED);
        }
        v_readerStatus(status)->subscriptionMatch.totalChanged = 0;
        v_readerStatus(status)->subscriptionMatch.currentChanged = 0;
        V_READER_UNLOCK(_this);
    }
    return result;
}

static v_result
waitForHistoricalData(
    v_reader _this,
    os_duration timeout)
{
    v_result result = V_RESULT_OK;
    os_timeM starttime = os_timeMGet();
    V_READER_LOCK(_this);
    while (!_this->historicalDataComplete && result == V_RESULT_OK && timeout > OS_DURATION_ZERO)
    {
        if (v_condWait(&_this->historicalDataCondition, &V_READER_GET_LOCK(_this), timeout) != V_RESULT_OK) {
            result = V_RESULT_TIMEOUT;
        }
        timeout -= os_timeMDiff(os_timeMGet(), starttime);
    }
    V_READER_UNLOCK(_this);
    return result;
}

static v_result
v_readerDurableDataRequest(
    v_reader _this,
    const c_char* filter,
    const c_char* params[],
    c_ulong paramsLength,
    os_timeW minSourceTime,
    os_timeW maxSourceTime,
    v_resourcePolicyI *limits)
{
    v_result result = V_RESULT_OK;
    v_kernel kernel;
    c_iter services;
    v_service durability_client = NULL;
    v_service durability_service = NULL;
    c_bool conditional;

    kernel = v_objectKernel(_this);

    /* Determine if a subset of durable data is requested. */
    conditional = (filter || params ||
                   !OS_TIMEW_ISINVALID(minSourceTime) || !OS_TIMEW_ISINVALID(maxSourceTime) ||
                   limits->v.max_samples != -1 ||
                   limits->v.max_instances != -1 ||
                   limits->v.max_samples_per_instance != -1);

    /* conditional requests only makes sense for volatile readers,
     * non-volatile readers will automatically be aligned, so therefore
     * return precondition not met when requesting conditional alignment on non-volatile readers.
     */
    if (conditional && _this->qos->durability.v.kind > V_DURABILITY_TRANSIENT_LOCAL) {
        return V_RESULT_PRECONDITION_NOT_MET;
    }
    /* Lookup the Durability Service */
    services = v_resolveServiceByServiceType(kernel, V_SERVICETYPE_DURABILITY);
    assert(c_iterLength(services) <= 1);
    durability_service = c_iterTakeFirst(services);
    c_iterFree(services);
    /* Lookup the Client Service (Spliced) */
    services = v_resolveServiceByServiceType(kernel, V_SERVICETYPE_SPLICED);
    assert(c_iterLength(services) <= 1);
    durability_client = c_iterTakeFirst(services);
    c_iterFree(services);

    /* Send a Request for durable data if a durable data source exists. */
    if (durability_service || durability_client) {
        v_historicalDataRequest request;
        V_DC_TRACE("%s - request historical data for reader %s\n", OS_FUNCTION, v_entityName(_this));

        request = v_historicalDataRequestNew(kernel, filter, params, paramsLength,
                                             minSourceTime, maxSourceTime,
                                             limits, OS_DURATION_ZERO);
        if (request != NULL) {
            if (_this->historicalDataRequest) {
                /* Historical data request already in progress or complete, check
                 * whether request is equal to the original one.
                 */
                if (!v_historicalDataRequestEquals(request, _this->historicalDataRequest)) {
                    /* Request is NOT equal to original request*/
                    result = V_RESULT_PRECONDITION_NOT_MET;
                }
            } else {
                C_STRUCT(v_event) event;
                _this->historicalDataRequest = c_keep(request);
                /* Volatile readers don't automatically receive non-volatile data,
                 * therefore for each entry copy the data from the associated groups into
                 * the reader.
                 */
                if (_this->qos->durability.v.kind == V_DURABILITY_VOLATILE) {
                    c_iter entries;
                    v_entry e;

                    v_readerEntrySetLock(_this);
                    entries = ospl_c_select(_this->entrySet.entries, 0);
                    v_readerEntrySetUnlock(_this);
                    while ((e = (v_entry)c_iterTakeFirst(entries)) != NULL) {
                        if (result == V_RESULT_OK) {
                            if (conditional) {
                                result = getHistoricalDataCommon(e, _this->historicalDataRequest, TRUE);
                            } else {
                                result = getHistoricalDataCommon(e, NULL, TRUE);
                            }
                        }
                        c_free(e);
                    }
                    c_iterFree(entries);
                }
                /* Create an event and attach the request for durable data and notify all appropriate
                 * durable data sources.
                 */
                event.kind = V_EVENT_HISTORY_REQUEST;
                event.source = v_observable (_this);
                event.data = request;
                if (durability_service) {
                    /* Trigger and request the durability service to provide historical data */
                    v_observableNotify(v_observable(durability_service),&event);
                }
                if (durability_client) {
                    /* Trigger the durability client to send a historical data request */
                    v_observableNotify(v_observable(durability_client),&event);
                }
            }
            c_free(request);
        } else {
            result = V_RESULT_OUT_OF_MEMORY;
        }
        c_free(durability_service);
        c_free(durability_client);
    }
    return result;
}

static v_result
checkParameters(
    v_reader reader,
    const c_char* filter,
    const c_char* params[],
    os_timeW minSourceTime,
    os_timeW maxSourceTime,
    v_resourcePolicyI *limits)
{
    v_result result = V_RESULT_OK;
    OS_UNUSED_ARG(params);

    if((limits->v.max_samples != -1 && limits->v.max_samples < 0) ||
       (limits->v.max_instances != -1 && limits->v.max_instances < 0) ||
       (limits->v.max_samples_per_instance != -1 && limits->v.max_samples_per_instance < 0))
    {
        result = V_RESULT_ILL_PARAM;
    } else if((reader->qos->resource.v.max_samples != -1) &&
              (reader->qos->resource.v.max_samples < limits->v.max_samples))
    {
        result = V_RESULT_ILL_PARAM;
    } else if((reader->qos->resource.v.max_instances != -1) &&
              (reader->qos->resource.v.max_instances < limits->v.max_instances))
    {
        result = V_RESULT_ILL_PARAM;
    } else if((reader->qos->resource.v.max_samples_per_instance != -1) &&
              (reader->qos->resource.v.max_samples_per_instance < limits->v.max_samples_per_instance))
    {
        result = V_RESULT_ILL_PARAM;
    } else if (!OS_TIMEW_ISINVALID(minSourceTime) && !OS_TIMEW_ISINVALID(maxSourceTime) &&
               os_timeWCompare(minSourceTime, maxSourceTime) == OS_MORE)
    {
        result = V_RESULT_ILL_PARAM;
    } else if (filter) {
        q_expr expr = q_parse(filter);
        if(expr){
            q_dispose(expr);
        } else {
            result = V_RESULT_ILL_PARAM;
        }
    }
    return result;
}

static v_result
v_readerCheckDurableDataSupport(
    v_reader _this)
{
    v_result result = V_RESULT_OK;
    if (!v_kernelGetDurabilitySupport(v_objectKernel(_this))) {
        result = V_RESULT_PRECONDITION_NOT_MET;
    }
    return result;
}

static c_long
v_readerDurableGroupCount(
    v_reader _this)
{
    c_iter entries;
    v_entry e;
    c_long count = 0;

    v_readerEntrySetLock(_this);
    entries = ospl_c_select(_this->entrySet.entries, 0);
    v_readerEntrySetUnlock(_this);
    while ((e = (v_entry)c_iterTakeFirst(entries)) != NULL) {
        count += v_entryDurableGroupCount(e);
        c_free(e);
    }
    return count;
}

v_result
v_readerWaitForHistoricalData(
    v_reader r,
    os_duration timeout)
{
    return v_readerWaitForHistoricalDataWithCondition(r, NULL, NULL, 0,
                                                      OS_TIMEW_INVALID, OS_TIMEW_INVALID,
                                                      -1, -1, -1, timeout);
}

v_result
v_readerWaitForHistoricalDataWithCondition(
    v_reader _this,
    const c_char* filter,
    const c_char* params[],
    c_ulong paramsLength,
    os_timeW minSourceTime,
    os_timeW maxSourceTime,
    c_long max_samples,
    c_long max_instances,
    c_long max_samples_per_instance,
    os_duration timeout)
{
    v_result result = V_RESULT_OK;
    v_resourcePolicyI limits;

    limits.v.max_samples = max_samples;
    limits.v.max_instances = max_instances;
    limits.v.max_samples_per_instance = max_samples_per_instance;

    /* Return BAD_PARAMETER in case an invalid value is passed. */
    if (timeout < 0) {
        result = V_RESULT_ILL_PARAM;
    } else {
        result = checkParameters(_this, filter, params, minSourceTime, maxSourceTime, &limits);
        if (result == V_RESULT_OK) {
            if (v_readerDurableGroupCount(_this) == 0) {
                /* Not connected to durable groups so no need to wait for data */
                return result;
            }
        }
    }

    if (result == V_RESULT_OK) {
        /* Return PRECONDITION_NOT_MET in case there is no source for durable data. */
        result = v_readerCheckDurableDataSupport(_this);
    }
    if (result == V_RESULT_OK) {
        /* send a request for durable data to available providers */
        result = v_readerDurableDataRequest(_this, filter, params, paramsLength,
                                            minSourceTime, maxSourceTime, &limits);
    }
    if (result == V_RESULT_OK) {
        /* Wait for availability of durable data */
        result = waitForHistoricalData(_this, timeout);
    }
    return result;
}

void
v_readerNotifyStateChange(
    v_reader _this,
    c_bool complete)
{
    assert(C_TYPECHECK(_this, v_reader));

    if(_this){
        V_READER_LOCK(_this);
        if (_this->historicalDataComplete != complete) {
            _this->historicalDataComplete = complete;
            c_condBroadcast(&_this->historicalDataCondition);
        }
        V_READER_UNLOCK(_this);
    }
    return;
}

c_bool
v_readerIsAligned(
    v_reader _this)
{
    c_bool isAligned = FALSE;
    if (_this) {
        isAligned = _this->historicalDataComplete;
    }
    return isAligned;
}

c_iter
v_readerGetPartitions(
    v_reader _this)
{
    c_iter partitions = NULL;

    if (_this) {
        V_READER_LOCK(_this);
        if (_this->subscriber) {
            partitions = v_subscriberLookupPartitions(_this->subscriber, "*");
        }
        V_READER_UNLOCK(_this);
    }

    return partitions;
}
