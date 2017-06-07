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

#include "v_query.h"
#include "v_queryStatistics.h"
#include "v__dataReader.h"
#include "v__dataView.h"
#include "v_dataViewInstance.h"
#include "v_dataReaderQuery.h"
#include "v_dataViewQuery.h"
#include "v_entity.h"
#include "v_event.h"
#include "v__observable.h"
#include "v__observer.h"
#include "v_public.h"
#include "v__collection.h"
#include "v__waitset.h"

#include "q_helper.h"
#include "os_report.h"
#include "os_abstract.h"
#include "os_heap.h"

v_query
v_queryNew (
    v_collection source,
    const os_char *name,
    const os_char *expression,
    const os_char *params[],
    const os_uint32 nrOfParams,
    const os_uint32 sampleMask)
{
    v_query _this;
    v_dataReader reader;
    v_dataView readerView;

    assert(C_TYPECHECK(source,v_collection));

    _this = NULL;

    switch(v_objectKind(source)) {
    case K_DATAREADER:
        reader = v_dataReader(source);
        _this = v_query(v_dataReaderQueryNew(reader, name, expression, params, nrOfParams, sampleMask));
    break;
    case K_DATAVIEW:
        readerView = v_dataView(source);
        _this = v_query(v_dataViewQueryNew(readerView, name, expression, params, nrOfParams, sampleMask));
    break;
    default:
        OS_REPORT(OS_ERROR,
                    "v_queryNew failed",V_RESULT_ILL_PARAM,
                    "illegal source kind (%d) specified",
                    v_objectKind(source));
    }
    return _this;
}

v_result
v_queryInit(
    v_query _this,
    v_collection src,
    const os_char *name,
    const os_char *expression)
{
    assert(C_TYPECHECK(_this,v_query));

    v_collectionInit(v_collection(_this), name, TRUE);

    _this->source = src;
    _this->expression = c_stringNew(c_getBase(_this), expression);
    _this->statistics = NULL;
    return V_RESULT_OK;
}

void
v_queryFree(
    v_query _this)
{
    if (_this != NULL) {
        assert(C_TYPECHECK(_this,v_query));
        v_collectionFree(v_collection(_this));
    }
}

void
v_queryDeinit(
    v_query _this)
{
    if (_this != NULL) {
        assert(C_TYPECHECK(_this,v_query));
        v_collectionDeinit(v_collection(_this));
    }
}

v_collection
v_querySource(
    v_query _this)
{
    v_collection c;

    if (_this == NULL) {
        return NULL;
    }

    assert(C_TYPECHECK(_this,v_query));

    c = v_collection(_this->source);
    if (c == NULL) {
        OS_REPORT(OS_CRITICAL,
                    "v_querySource failed",V_RESULT_ILL_PARAM,
                    "Query (0x%"PA_PRIxADDR") without source detected",
                    (os_address)_this);
        assert(FALSE);
        return NULL;
    }

    switch(v_objectKind(c)) {
    case K_DATAREADER:
    case K_DATAVIEW:
        c_keep(c);
    break;
    case K_DATAREADERQUERY:
    case K_DATAVIEWQUERY:
        c = v_querySource(v_query(c));
    break;
    default:
        OS_REPORT(OS_CRITICAL,
                    "v_querySource failed",V_RESULT_ILL_PARAM,
                    "illegal source kind (%d) detected",
                    v_objectKind(c));
        assert(FALSE);
        return NULL;
    }
    return c;
}

void
v_queryEnableStatistics(
    v_query _this,
    os_boolean enable)
{
    assert(_this);

    if ((enable) && (_this->statistics == NULL)) {
        _this->statistics = v_queryStatisticsNew(v_objectKernel(_this));
    }
    if ((!enable) && (_this->statistics != NULL)) {
        c_free(_this->statistics);
        _this->statistics = NULL;
    }
}

q_expr
v_queryGetPredicate(
    v_query _this)
{
    assert(_this);
    return q_parse(_this->expression);
}

c_bool
v_queryTest(
    v_query _this,
    v_queryAction action,
    c_voidp args)
{
    c_bool result = FALSE;

    if (_this == NULL) {
        return FALSE;
    }

    assert(C_TYPECHECK(_this,v_query));

    switch (v_objectKind(_this)) {
    case K_DATAREADERQUERY:
        result = v_dataReaderQueryTest(v_dataReaderQuery(_this), action, args);
    break;
    case K_DATAVIEWQUERY:
        result = v_dataViewQueryTest(v_dataViewQuery(_this), action, args);
    break;
    default:
        OS_REPORT(OS_ERROR,
                    "v_queryTest failed",V_RESULT_ILL_PARAM,
                    "illegal query kind (%d) specified",
                    v_objectKind(_this));
        assert(FALSE);
    }

    return result;
}

c_bool
v_queryTestSample(
    v_query _this,
    v_readerSample sample)
{
    c_bool result = FALSE;

    assert(_this);
    assert(C_TYPECHECK(_this,v_query));

    switch (v_objectKind(_this)) {
    case K_DATAREADERQUERY:
        result = v_dataReaderQueryTestSample(v_dataReaderQuery(_this), v_dataReaderSample(sample));
    break;
    case K_DATAVIEWQUERY:
        result = v_dataViewQueryTestSample(v_dataViewQuery(_this), v_dataViewSample(sample));
    break;
    default:
        OS_REPORT(OS_ERROR,
                    "v_queryTestSample failed",V_RESULT_ILL_PARAM,
                    "illegal query kind (%d) specified",
                    v_objectKind(_this));
        assert(FALSE);
    }

    return result;
}

static c_bool
queryContainsInstance (
    const v_query _this,
    const v_dataReaderInstance i)
{
    c_bool result = FALSE;
    v_dataReader reader;
    v_dataView view;
    v_collection src;

    switch (v_objectKind(_this)) {
    case K_DATAREADERQUERY:
        src = v_querySource(_this);
        reader = v_dataReader(src);
        result = v_dataReaderContainsInstance(reader,i);
        c_free(src);
    break;
    case K_DATAVIEWQUERY:
        src = v_querySource(_this);
        view = v_dataView(src);
        assert(C_TYPECHECK(i, v_dataViewInstance));
        result = v_dataViewContainsInstance(view,v_dataViewInstance(i));
        c_free(src);
    break;
    default:
        assert(FALSE);
    }

    return result;
}

static v_result
v_queryReadInternal(
    v_query _this,
    v_dataReaderInstance instance,
    c_bool readNext,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout)
{
    v_result result = V_RESULT_OK;
    v_dataReaderQuery drq;
    v_dataViewQuery dvq;

    if (_this == NULL) {
        return V_RESULT_ILL_PARAM;
    }

    assert(C_TYPECHECK(_this,v_query));

    switch (v_objectKind(_this)) {
    case K_DATAREADERQUERY:
        drq = v_dataReaderQuery(_this);
        if (readNext) {
            result = v_dataReaderQueryReadNextInstance(
                              drq, instance,
                              (v_readerSampleAction)action, arg, timeout);
        } else {
            if (instance) {
                result = v_dataReaderQueryReadInstance(
                              drq, instance,
                              (v_readerSampleAction)action, arg, timeout);
            } else {
                result = v_dataReaderQueryRead(
                              drq,
                              (v_readerSampleAction)action, arg, timeout);
            }
        }
    break;
    case K_DATAVIEWQUERY:
        dvq = v_dataViewQuery(_this);
        if (readNext) {
            result = v_dataViewQueryReadNextInstance(dvq,
                          (v_dataViewInstance)instance,
                          (v_readerSampleAction)action, arg, timeout);
        } else {
            if (instance) {
                result = v_dataViewQueryReadInstance(dvq,
                              (v_dataViewInstance)instance,
                              (v_readerSampleAction)action, arg, timeout);
            } else {
                result = v_dataViewQueryRead(dvq,
                              (v_readerSampleAction)action, arg, timeout);
            }
        }
    break;
    default:
        result = V_RESULT_ILL_PARAM;
        OS_REPORT(OS_ERROR,"v_queryRead failed",result,
                    "illegal query kind (%d) specified",v_objectKind(_this));
        assert(FALSE);
    }

    return result;
}

v_result
v_queryRead(
    v_query _this,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout)
{
    return v_queryReadInternal(_this, NULL, FALSE, action, arg, timeout);
}

static v_result
v_queryTakeInternal(
    v_query _this,
    v_dataReaderInstance instance,
    c_bool readNext,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout)
{
    v_result result = V_RESULT_OK;
    v_dataReaderQuery drq;
    v_dataViewQuery dvq;

    if (_this == NULL) {
        return V_RESULT_ILL_PARAM;
    }

    assert(C_TYPECHECK(_this,v_query));

    switch (v_objectKind(_this)) {
    case K_DATAREADERQUERY:
        drq = v_dataReaderQuery(_this);
        if (readNext) {
            result = v_dataReaderQueryTakeNextInstance(drq, instance,
                          (v_readerSampleAction)action,arg, timeout);
        } else {
            if (instance) {
                result = v_dataReaderQueryTakeInstance(drq,
                              instance,
                              (v_readerSampleAction)action, arg, timeout);
            } else {
                result = v_dataReaderQueryTake(drq,
                              (v_readerSampleAction)action, arg, timeout);
            }
        }
    break;
    case K_DATAVIEWQUERY:
        dvq = v_dataViewQuery(_this);
        if (readNext) {
            result = v_dataViewQueryTakeNextInstance(dvq,
                              (v_dataViewInstance)instance,
                              (v_readerSampleAction)action, arg, timeout);
        } else {
            if (instance) {
                result = v_dataViewQueryTakeInstance(dvq,
                              (v_dataViewInstance)instance,
                              (v_readerSampleAction)action,
                              arg, timeout);
            } else {
                result = v_dataViewQueryTake(dvq,
                              (v_readerSampleAction)action, arg, timeout);
            }
        }
    break;
    default:
        result = V_RESULT_ILL_PARAM;
        OS_REPORT(OS_ERROR,"v_queryTake failed",result,
                    "illegal query kind (%d) specified",v_objectKind(_this));
        assert(FALSE);
    }

    return result;
}

v_result
v_queryTake(
    v_query _this,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout)
{
    return v_queryTakeInternal(_this, NULL, FALSE, action, arg, timeout);
}

void
v_queryNotify(
    v_query _this,
    v_event event,
    c_voidp userData)
{
    OS_UNUSED_ARG(userData);
    if (_this && event) {
        if (event->kind != V_EVENT_DATA_AVAILABLE) {
            OS_REPORT(OS_WARNING, "v_query", V_RESULT_ILL_PARAM,
                        "Unexpected event %d", event->kind);
        }
    }
}

void
v_queryDetachWaitsets(
    v_query _this)
{
    v_observer observer = v_observer(_this);
    v_observable observable = v_observable(_this);
    v_proxy proxy, next;

    v_observerLock(observer);
    proxy = observable->observers;
    while (proxy) {
        next = proxy->next;
        if (v_objectKind(proxy->source2) == K_WAITSET) {
            v_observerUnlock(observer);
            (void)v_waitsetDetach(v_waitset(proxy->source2), observable);
            v_observerLock(observer);
        }
        proxy = next;
    }
    v_observerUnlock(observer);
}

c_bool
v_queryNotifyDataAvailable(
    v_query _this,
    v_event event)
{
    c_bool result;

    switch (v_objectKind(_this)) {
    case K_DATAREADERQUERY:
        result = v_dataReaderQueryNotifyDataAvailable(
                     v_dataReaderQuery(_this),
                     event);
    break;
    case K_DATAVIEWQUERY:
        result = v_dataViewQueryNotifyDataAvailable(
                     v_dataViewQuery(_this),
                     event);
    break;
    default:
        OS_REPORT(OS_ERROR,
                    "v_queryNotifyDataAvailable failed",V_RESULT_ILL_PARAM,
                    "illegal query kind (%d) specified",
                    v_objectKind(_this));
        result = TRUE;
        (void)result;
        assert(FALSE);
    }
    return result;
}

v_result
v_queryReadInstance(
    v_query _this,
    v_dataReaderInstance instance,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout)
{
    v_result result = V_RESULT_ILL_PARAM;

    if (queryContainsInstance(_this,instance)) {
        result = v_queryReadInternal(_this, instance, FALSE, action, arg, timeout);
    } else {
        result = V_RESULT_PRECONDITION_NOT_MET;
    }
    return result;
}

v_result
v_queryReadNextInstance(
    v_query _this,
    v_dataReaderInstance instance,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout)
{
    v_result result;

    if (instance == NULL || queryContainsInstance(_this,instance)) {
        result = v_queryReadInternal(_this, instance, TRUE, action, arg, timeout);
    } else {
        result = V_RESULT_PRECONDITION_NOT_MET;
    }
    return result;
}

v_result
v_queryTakeInstance(
    v_query _this,
    v_dataReaderInstance instance,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout)
{
    v_result result = V_RESULT_ILL_PARAM;

    if (queryContainsInstance(_this,instance)) {
        result = v_queryTakeInternal(_this, instance, FALSE, action, arg, timeout);
    } else {
        result = V_RESULT_PRECONDITION_NOT_MET;
    }
    return result;
}

v_result
v_queryTakeNextInstance(
    v_query _this,
    v_dataReaderInstance instance,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout)
{
    v_result result;

    if (instance == NULL || queryContainsInstance(_this,instance)) {
        result = v_queryTakeInternal(_this, instance, TRUE, action, arg, timeout);
    } else {
        result = V_RESULT_PRECONDITION_NOT_MET;
    }

    return result;
}

c_bool
v_querySetParams(
    v_query _this,
    const os_char *params[],
    const os_uint32 nrOfParams)
{
    c_bool result = FALSE;

    assert(C_TYPECHECK(_this,v_query));

    if (_this != NULL) {
        switch (v_objectKind(_this)) {
        case K_DATAREADERQUERY:
            result = v_dataReaderQuerySetParams(v_dataReaderQuery(_this), params, nrOfParams);
        break;
        case K_DATAVIEWQUERY:
            result = v_dataViewQuerySetParams(v_dataViewQuery(_this), params, nrOfParams);
        break;
        default:
            OS_REPORT(OS_ERROR,
                        "v_querySetParams failed",V_RESULT_ILL_PARAM,
                        "illegal query kind (%d) specified",
                        v_objectKind(_this));
            assert(FALSE);
        }
    }
    return result;
}
