/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
#include "v__builtin.h"
#include "v__reader.h"
#include "v__readerQos.h"
#include "v__subscriberQos.h"
#include "v__observer.h"
#include "v_subscriber.h"
#include "v__entry.h"
#include "v_query.h"
#include "v__dataReader.h"
#include "v__deliveryService.h"
#include "v_deliveryServiceEntry.h"
#include "v__groupStream.h"
#include "v__networkReader.h"
#include "v__entity.h"
#include "v_status.h"
#include "v_public.h"
#include "v__collection.h"
#include "v_event.h"
#include "v__group.h"
#include "v_time.h"
#include "v_proxy.h"
#include "v_policy.h"
#include "v_spliced.h"
#include "v_durabilityClient.h"
#include "v__partition.h"
#include "v__observable.h"
#include "v_historicalDataRequest.h"

#include "os_report.h"
#include "vortex_os.h"

#if 0
#define _TRACE_EVENTS_ printf
#else
#define _TRACE_EVENTS_(...)
#endif

#define V_READER_GET_LOCK(_this) (v_observable(_this)->mutex)

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

void
v_readerInit(
    _Inout_ v_reader r,
    _In_z_ const c_char *name,
    _In_ v_subscriber s,
    _In_ v_readerQos qos)
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
    v_collectionInit(v_collection(r), name);

    r->subscriber = s;
    r->qos = c_keep(qos);
    r->subQos = c_keep(s->qos); /* reference is readonly */
    r->entries = c_setNew(v_kernelType(kernel,K_ENTRY));
    r->historicalDataRequest  = NULL;
    r->historicalDataComplete = FALSE;
    c_condInit(c_getBase(r), &r->historicalDataCondition, &v_observable(r)->mutex);
}

void
v_readerFree(
    v_reader r)
{
    v_subscriber subscriber;
    c_collection entries;
    v_entry entry;

    assert(C_TYPECHECK(r,v_reader));
    assert(r->subscriber != NULL);

    OSPL_LOCK(r);
    subscriber = r->subscriber;
    r->subscriber = NULL;
    OSPL_UNLOCK(r);

    /* For each readerInstance in each entry, v_subscriberRemoveReader will
     * eventually deliver an UNREGISTER message to the Reader using
     * v_dataReaderEntryWrite, which on its turn locks the reader itself.
     *
     * For that reason, we don't do any locking here so that the entryFree
     * function is called without the readerLock being locked.
     */
    v_subscriberRemoveReader(subscriber,r);

    /* Free all entries */
    OSPL_LOCK(r);
    entries = r->entries;
    r->entries = NULL;
    OSPL_UNLOCK(r);
    while ((entry = v_entry(c_take(entries))) != NULL) {
        v_entryFree(entry);
        c_free(entry);
    }
    c_free(entries);
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

v_readerQos
v_readerGetQos(
    v_reader _this)
{
    v_readerQos qos;

    assert(C_TYPECHECK(_this,v_reader));

    OSPL_LOCK(_this);
    qos = c_keep(_this->qos);
    OSPL_UNLOCK(_this);

    return qos;
}

static c_bool
groupGetOpenTransactions(
    c_object o,
    c_voidp arg)
{
    v_group g = v_group(o);
    v_entry e = v_entry(arg);
    /* Do not get the open transactions from the groupAdmin as the reader
     * missed those completely. Only get the open transactions form the
     * transactionAdmin as those could still be received and completed.
     */
    v_groupGetOpenTransactions(g,e,FALSE);
    return TRUE;
}

static c_bool
entryGetOpenTransactions(
    c_object o,
    c_voidp arg)
{
    v_entry e = v_entry(o);
    OS_UNUSED_ARG(arg);
    if (v_objectKind(e) != K_NETWORKREADERENTRY) {
        v_entryWalkGroups(e, groupGetOpenTransactions, e);
    }
    return TRUE;
}

void
v_readerPublishBuiltinInfo(
    v_reader _this)
{
    v_kernel kernel;
    v_message builtinMsg, builtinCMMsg;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_reader));

    if (v_entityEnabled(v_entity(_this))) {
        kernel = v_objectKernel(_this);
        builtinMsg = v_builtinCreateSubscriptionInfo(kernel->builtin, _this);
        if (builtinMsg) {
            v_writeBuiltinTopic(kernel, V_SUBSCRIPTIONINFO_ID, builtinMsg);
            c_free(builtinMsg);
        }
        builtinCMMsg = v_builtinCreateCMDataReaderInfo(kernel->builtin, _this);
        if (builtinCMMsg) {
            v_writeBuiltinTopic(kernel, V_CMDATAREADERINFO_ID, builtinCMMsg);
            c_free(builtinCMMsg);
        }
    }
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
            OSPL_LOCK(_this);
            result = v_readerQosCompare(_this->qos, qos, v__entityEnabled_nl(v_entity(_this)),
                                        v_subscriberQosIsGroupCoherent(_this->subQos), &cm);
            if ((result == V_RESULT_OK) && (cm != 0)) {
                c_free(_this->qos);
                _this->qos = c_keep(qos);
                OSPL_UNLOCK(_this);
                v_readerPublishBuiltinInfo(_this);
                /* On change of qos try to get all open transactions from the
                 * group, so that the group is able to determine completeness
                 * of the transaction.
                 */
                (void)c_setWalk(_this->entries, entryGetOpenTransactions, NULL);
            } else {
                OSPL_UNLOCK(_this);
            }
            c_free(qos);
        } else {
            result = V_RESULT_OUT_OF_MEMORY;
        }
    }
    return result;
}

c_bool
v_readerWalkEntries_nl(
    v_reader r,
    c_action action,
    c_voidp arg)
{
    c_bool result = TRUE;

    assert(C_TYPECHECK(r,v_reader));
    result = c_setWalk(r->entries, action, arg);
    return result;
}

c_bool
v_readerWalkEntries(
    v_reader r,
    c_action action,
    c_voidp arg)
{
    c_bool result = TRUE;

    assert(C_TYPECHECK(r,v_reader));
    OSPL_LOCK(r);
    result = c_setWalk(r->entries, action, arg);
    OSPL_UNLOCK(r);
    return result;
}

c_iter
v_readerCollectEntries(
    v_reader r)
{
    c_iter result = NULL;

    assert(C_TYPECHECK(r,v_reader));

    if(r){
        OSPL_LOCK(r);
        result = ospl_c_select(r->entries, 0);
        OSPL_UNLOCK(r);
    }
    return result;
}

void
v_readerAddEntry(
    _Inout_ v_reader r,
    _In_ v_entry e)
{
    assert(C_TYPECHECK(r,v_reader));
    assert(C_TYPECHECK(e,v_entry));

    OSPL_LOCK(r);
    (void)c_setInsert(r->entries, e);
    /* c_setInsert will ALWAYS return e. If it was already in and if it wasn't.
     * In both cases it is in the set afterwards */
    OSPL_UNLOCK(r);
}

static c_bool
readerEntryAddTransactionAdmin(
    _Inout_ c_object o,
    _Inout_opt_ c_voidp arg)
{
    assert(C_TYPECHECK(o, v_dataReaderEntry));

    v_dataReaderEntrySetTransactionAdmin(v_dataReaderEntry(o), arg);

    return TRUE;
}

void
v_readerAddTransactionAdmin(
    _Inout_ v_reader r,
    _In_opt_ v_transactionGroupAdmin a)
{
    assert(a == NULL || v__readerIsGroupCoherent(r));

    OSPL_LOCK(r);
    (void)c_setWalk(r->entries, &readerEntryAddTransactionAdmin, a);
    OSPL_UNLOCK(r);
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
        OSPL_LOCK(_this);
        status = v_entity(_this)->status;
        result = action(&v_readerStatus(status)->deadlineMissed, arg);
        if (reset) {
            v_statusReset(status, V_EVENT_REQUESTED_DEADLINE_MISSED);
_TRACE_EVENTS_("v_readerGetDeadlineMissedStatus::v_statusReset(0x%x, 0x%x) reader 0x%x\n",
               status, V_EVENT_REQUESTED_DEADLINE_MISSED, _this);
        }
        v_readerStatus(status)->deadlineMissed.totalChanged = 0;
        OSPL_UNLOCK(_this);
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
        OSPL_LOCK(_this);
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
        OSPL_UNLOCK(_this);
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
        OSPL_LOCK(_this);
        status = v_entity(_this)->status;
        result = action(&v_readerStatus(status)->sampleRejected, arg);
        if (reset) {
            v_statusReset(status, V_EVENT_SAMPLE_REJECTED);
_TRACE_EVENTS_("v_readerGetDeadlineMissedStatus::v_statusReset(0x%x, 0x%x) reader 0x%x\n",
               status, V_EVENT_SAMPLE_REJECTED, _this);
        }
        v_readerStatus(status)->sampleRejected.totalChanged = 0;
        OSPL_UNLOCK(_this);
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
        OSPL_LOCK(_this);
        status = v_entity(_this)->status;
        result = action(&v_readerStatus(status)->sampleLost, arg);
        if (reset) {
            v_statusReset(status, V_EVENT_SAMPLE_LOST);
_TRACE_EVENTS_("v_readerGetDeadlineMissedStatus::v_statusReset(0x%x, 0x%x) reader 0x%x\n",
               status, V_EVENT_SAMPLE_LOST, _this);
        }
        v_readerStatus(status)->sampleLost.totalChanged = 0;
        OSPL_UNLOCK(_this);
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
        OSPL_LOCK(_this);
        status = v_entity(_this)->status;
        result = action(&v_readerStatus(status)->livelinessChanged, arg);
        if (reset) {
            v_statusReset(status, V_EVENT_LIVELINESS_CHANGED);
_TRACE_EVENTS_("v_readerGetDeadlineMissedStatus::v_statusReset(0x%x, 0x%x) reader 0x%x\n",
               status, V_EVENT_LIVELINESS_CHANGED, _this);
        }
        v_readerStatus(status)->livelinessChanged.activeChanged = 0;
        v_readerStatus(status)->livelinessChanged.inactiveChanged = 0;
        OSPL_UNLOCK(_this);
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
        OSPL_LOCK(_this);
        status = v_entity(_this)->status;
        result = action(&v_readerStatus(status)->subscriptionMatch, arg);
        if (reset) {
            v_statusReset(status, V_EVENT_SUBSCRIPTION_MATCHED);
        }
        v_readerStatus(status)->subscriptionMatch.totalChanged = 0;
        v_readerStatus(status)->subscriptionMatch.currentChanged = 0;
        OSPL_UNLOCK(_this);
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
    OSPL_LOCK(_this);

    while (!_this->historicalDataComplete && result == V_RESULT_OK && timeout > OS_DURATION_ZERO)
    {
        if (v_condWait(&_this->historicalDataCondition, &V_READER_GET_LOCK(_this), timeout) != V_RESULT_OK) {
            result = V_RESULT_TIMEOUT;
        }
        timeout -= os_timeMDiff(os_timeMGet(), starttime);
    }
    OSPL_UNLOCK(_this);
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
    if (conditional && _this->qos->durability.v.kind > V_DURABILITY_VOLATILE) {
        return V_RESULT_PRECONDITION_NOT_MET;
    }

    kernel = v_objectKernel(_this);
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

    /* Send a Request for durable data if a durable data source exists, or if no networking services configured,
     * in which case the explicit request of historical data for a volatile reader can still be done.
     */
    if (durability_service || durability_client) {
        v_historicalDataRequest request;
        V_DC_TRACE("%s - request historical data for reader %s\n", OS_FUNCTION, v_entityName(_this));

        request = v_historicalDataRequestNew(kernel, filter, params, paramsLength,
                                             minSourceTime, maxSourceTime,
                                             limits, OS_DURATION_ZERO);
        if (request != NULL) {
            OSPL_LOCK(_this);
            if (_this->historicalDataRequest) {
                /* Historical data request already in progress or complete, check
                 * whether request is equal to the original one.
                 */
                if (!v_historicalDataRequestEquals(request, _this->historicalDataRequest)) {
                    /* Request is NOT equal to original request*/
                    result = V_RESULT_PRECONDITION_NOT_MET;
                }
                OSPL_UNLOCK(_this);
            } else {
                C_STRUCT(v_event) event;
                c_bool notDurable = (_this->qos->durability.v.kind == V_DURABILITY_VOLATILE);
                _this->historicalDataRequest = c_keep(request);
                OSPL_UNLOCK(_this);
                /* Volatile readers don't automatically receive non-volatile data,
                 * therefore for each entry copy the data from the associated groups into
                 * the reader.
                 */
                if (notDurable) {
                    c_iter entries;
                    v_entry e;

                    OSPL_LOCK(_this);
                    entries = ospl_c_select(_this->entries, 0);
                    OSPL_UNLOCK(_this);
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
                event.handled = TRUE;
                if (durability_service) {
                    /* Trigger and request the durability service to provide historical data */
                    OSPL_THROW_EVENT(durability_service, &event);
                }
                if (durability_client) {
                    /* Trigger the durability client to send a historical data request */
                    OSPL_THROW_EVENT(durability_client, &event);
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
    _In_ _Const_ v_reader _this)
{
    v_kernel kernel = v_objectKernel(_this);
    v_result result = V_RESULT_OK;
    if (!v_kernelGetDurabilitySupport(kernel)) {
        result = V_RESULT_PRECONDITION_NOT_MET;
    }
    return result;
}

static c_bool DurableGroupCount(c_object o, c_voidp arg)
{
    v_entry e = v_entry(o);
    c_long *count = (c_long *)arg;
    *count += v_entryDurableGroupCount(e);
    return TRUE;
}

static c_long
v_readerDurableGroupCount(
    v_reader _this)
{
    c_long count = 0;
    OSPL_LOCK(_this);
    (void)c_setWalk(_this->entries, DurableGroupCount, &count);
    OSPL_UNLOCK(_this);
    return count;
}

v_result
v_readerWaitForHistoricalData(
    v_reader r,
    os_duration timeout,
    c_bool enabling)
{
    return v_readerWaitForHistoricalDataWithCondition(r, NULL, NULL, 0,
                                                      OS_TIMEW_INVALID, OS_TIMEW_INVALID,
                                                      -1, -1, -1, timeout, enabling);
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
    os_duration timeout,
    c_bool enabling)
{
    v_result result = V_RESULT_OK;
    v_resourcePolicyI limits;

    /* Check enabled status only if not currently being enabled via v_dataReaderEnable() */
    if (!enabling) {
        if(!v_entityEnabled(v_entity(_this))) {
            return V_RESULT_NOT_ENABLED;
        }
    }

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

/* Precondition: reader is locked */
void
v__readerNotifyStateChange_nl(
    v_reader _this,
    c_bool complete)
{
    if (_this->historicalDataComplete != complete) {
        _this->historicalDataComplete = complete;
        c_condBroadcast(&_this->historicalDataCondition);
    }
}

void
v_readerNotifyStateChange(
    v_reader _this,
    c_bool complete)
{
    assert(C_TYPECHECK(_this, v_reader));

    if(_this){
        OSPL_LOCK(_this);
        v__readerNotifyStateChange_nl(_this, complete);
        OSPL_UNLOCK(_this);
    }
}

c_iter
v_readerGetPartitions(
    v_reader _this)
{
    c_iter partitions = NULL;
    v_subscriber s;

    if (_this) {
        OSPL_LOCK(_this);
        s = c_keep(_this->subscriber);
        OSPL_UNLOCK(_this);
        if (s) {
            partitions = v_subscriberLookupPartitions(s, "*");
        }
        c_free(s);
    }

    return partitions;
}

static c_bool
getTopic (
    c_object o,
    c_voidp arg)
{
    v_topic *topic = (v_topic *)arg;
    c_bool result = TRUE;

    switch (v_objectKind(o)) {
    case K_DATAREADERENTRY:
    {
        v_dataReaderEntry entry = v_dataReaderEntry(o);
        if (*topic == NULL) {
            *topic = c_keep(entry->topic);
        } else {
            /* Already a topic was found so this must be a Multi Topic reader.
             * In that case abort and clear the topic.
             */
            c_free(*topic);
            *topic = NULL;
            result = FALSE;
        }
    }
    break;
    case K_DELIVERYSERVICEENTRY:
    {
        v_deliveryServiceEntry entry = v_deliveryServiceEntry(o);
        *topic = c_keep(entry->topic);
    }
    break;
    default:
    break;
    }
    return result;
}

v_topic
v_readerGetTopic_nl(
    v_reader _this)
{
    v_topic topic = NULL;

    switch (v_objectKind(_this)) {
    case K_DATAREADER:
    case K_DELIVERYSERVICE:
        (void)v_readerWalkEntries_nl(_this, getTopic, &topic);
    break;
    default:
    break;
    }
    return topic;
}

v_topic
v_readerGetTopic(
    v_reader _this)
{
    v_topic topic = NULL;
    OSPL_LOCK(_this);
    topic = v_readerGetTopic_nl(_this);
    OSPL_UNLOCK(_this);
    return topic;
}

v_subscriber
v_readerGetSubscriber(
    v_reader _this)
{
    v_subscriber s;
    OSPL_LOCK(_this);
    s = c_keep(_this->subscriber);
    OSPL_UNLOCK(_this);
    return s;
}
