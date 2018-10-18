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
#include "u__object.h"
#include "u__observable.h"
#include "u__entity.h"
#include "u_query.h"
#include "u_reader.h"
#include "u__instanceHandle.h"
#include "u__user.h"
#include "v_query.h"
#include "v_entity.h"
#include "v_collection.h"
#include "v_dataReader.h"
#include "v_dataView.h"

#include "u__types.h"
#include "q_expr.h"

#include "os_report.h"

static u_result
u__queryDeinitW(
    void *_this)
{
    return u__entityDeinitW(_this);
}

static void
u__queryFreeW(
    void *_this)
{
    u__entityFreeW(_this);
}

static u_result
u_queryInit(
    u_query _this,
    v_query query,
    const u_reader reader)
{
    return u_entityInit(u_entity(_this), v_entity(query), u_observableDomain(u_observable(reader)));
}

u_query
u_queryNew(
    const u_reader reader,
    const os_char *name,
    const os_char *predicate,
    const os_char *params[],
    const os_uint32 nrOfParams,
    const u_sampleMask sampleMask)
{
    u_query _this = NULL;
    v_collection kc;
    v_query query;
    u_result result;

    assert(reader);

    if (name == NULL) {
        name = "No name specified";
    }
    result = u_observableWriteClaim(u_observable(reader), (v_public *)(&kc), C_MM_RESERVATION_LOW);
    if (result == U_RESULT_OK) {
        assert(kc);
        query = v_queryNew (kc,name,predicate,params,nrOfParams,sampleMask);
        if (query != NULL) {
            _this = u_objectAlloc(sizeof(*_this), U_QUERY, u__queryDeinitW, u__queryFreeW);
            if (_this != NULL) {
                result = u_queryInit(_this, query, reader);
                if (result != U_RESULT_OK) {
                    OS_REPORT(OS_ERROR, "u_queryNew", result,
                                "Initialisation failed. "
                                "For query: <%s>.", name);
                    u_objectFree(u_object (_this));
                    _this = NULL;
                }
            } else {
                OS_REPORT(OS_ERROR, "u_queryNew", U_RESULT_OUT_OF_MEMORY,
                            "Create proxy failed. "
                            "For query: <%s>.", name);
            }
            c_free(query);
        } else {
            OS_REPORT(OS_ERROR, "u_queryNew", U_RESULT_INTERNAL_ERROR,
                        "Create kernel entity failed. "
                        "For query: <%s>.", name);
        }
        u_observableRelease(u_observable(reader), C_MM_RESERVATION_LOW);
    } else {
        OS_REPORT(OS_WARNING, "u_queryNew", result,
                    "Claim reader failed. "
                    "For query: <%s>", name);
    }
    return _this;
}

/***************************** read/take **************************************/

C_STRUCT(readActionArg) {
    u_readerAction action;
    void *arg;
    v_actionResult result;
};

C_CLASS(readActionArg);

static v_actionResult
readAction(
    c_object sample,
    void *arg)
{
    readActionArg a = (readActionArg)arg;

    if (sample == NULL) {
        a->action(NULL,a->arg);
    } else {
        a->result = a->action((v_dataReaderSample)sample,a->arg);
    }
    return a->result;
}

u_result
u_queryRead(
    const u_query _this,
    u_readerAction action,
    void *actionArg,
    const os_duration timeout)
{
    u_result result;
    v_query query;
    C_STRUCT(readActionArg) arg;

    assert(_this);
    assert(action);
    assert(OS_DURATION_ISPOSITIVE(timeout));

    result = u_observableReadClaim(u_observable(_this),(v_public *)(&query),C_MM_RESERVATION_LOW);
    if (result == U_RESULT_OK) {
        arg.action = action;
        arg.arg = actionArg;
        arg.result = 0;
        v_queryRead(query, readAction, &arg, timeout);
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_LOW);
    } else {
        OS_REPORT(OS_WARNING, "u_queryRead", U_RESULT_INTERNAL_ERROR,
                  "query could not be claimed.");
    }
    return result;
}

u_result
u_queryTake(
    const u_query _this,
    u_readerAction action,
    void *actionArg,
    const os_duration timeout)
{
    u_result result;
    v_query query;
    C_STRUCT(readActionArg) arg;

    assert(_this);
    assert(action);
    assert(OS_DURATION_ISPOSITIVE(timeout));

    result = u_observableReadClaim(u_observable(_this),(v_public *)(&query), C_MM_RESERVATION_LOW);
    if (result == U_RESULT_OK) {
        arg.action = action;
        arg.arg = actionArg;
        arg.result = 0;
        v_queryTake(query, readAction, &arg, timeout);
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_LOW);
    } else {
        OS_REPORT(OS_WARNING, "u_queryTake", result,
                  "query could not be claimed.");
    }
    return result;
}

u_bool
u_queryTest(
    const u_query _this,
    u_queryAction action,
    void *args)
{
    v_query query;
    u_bool result;
    u_result r;

    assert(_this);
    assert(action != NULL);

    r = u_observableReadClaim(u_observable(_this),(v_public *)(&query), C_MM_RESERVATION_ZERO);
    if (r == U_RESULT_OK) {
        assert(query);
        result = v_queryTest(query, action, args);
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_ZERO);
    } else {
        OS_REPORT(OS_WARNING, "u_queryTest", r,
                  "query could not be claimed.");
        result = FALSE;
    }
    return result;
}

u_result
u_querySet(
    const u_query _this,
    const os_char *params[],
    const os_uint32 nrOfParams)
{
    v_query query;
    u_result result;

    assert(_this);

    result = u_observableReadClaim(u_observable(_this),(v_public *)(&query), C_MM_RESERVATION_LOW);
    if (result == U_RESULT_OK) {
        assert(query);
        if (! v_querySetParams(query, params, nrOfParams)) {
            result = U_RESULT_ILL_PARAM;
            OS_REPORT(OS_ERROR, "u_querySet", result,
                      "Could not set kernel query parameters.");
        }
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_LOW);
    } else {
        OS_REPORT(OS_WARNING, "u_querySet", result,
                  "query could not be claimed.");
    }
    return result;
}

/***************************** read/take_(next)_instance **********************/

u_result
u_queryReadInstance(
    const u_query _this,
    u_instanceHandle handle,
    u_readerAction action,
    void *actionArg,
    const os_duration timeout)
{
    v_query  query;
    u_result result;
    v_dataReaderInstance instance;
    C_STRUCT(readActionArg) arg;

    assert(_this);
    assert(action);
    assert(OS_DURATION_ISPOSITIVE(timeout));

    result = u_observableReadClaim(u_observable(_this),(v_public *)(&query), C_MM_RESERVATION_LOW);
    if (result == U_RESULT_OK) {
        assert(query);
        handle = u_instanceHandleFix(handle,v_collection(query));
        result = u_instanceHandleClaim(handle, &instance);
        if (result == U_RESULT_OK) {
            assert(instance != NULL);
            arg.action = action;
            arg.arg = actionArg;
            arg.result = 0;
            result = u_resultFromKernel(v_queryReadInstance(query, instance, readAction, &arg, timeout));
            u_instanceHandleRelease(handle);
        }
        u_observableRelease(u_observable(_this),C_MM_RESERVATION_LOW);
    } else {
        OS_REPORT(OS_WARNING, "u_queryReadInstance", result,
                  "Could not claim query.");
    }
    return result;
}

u_result
u_queryTakeInstance(
    const u_query _this,
    u_instanceHandle handle,
    u_readerAction action,
    void *actionArg,
    const os_duration timeout)
{
    v_query query;
    u_result result;
    v_dataReaderInstance instance;
    C_STRUCT(readActionArg) arg;

    assert(_this);
    assert(action);
    assert(OS_DURATION_ISPOSITIVE(timeout));

    result = u_observableReadClaim(u_observable(_this),(v_public *)(&query), C_MM_RESERVATION_LOW);
    if (result == U_RESULT_OK) {
        assert(query);
        handle = u_instanceHandleFix(handle,v_collection(query));
        result = u_instanceHandleClaim(handle, &instance);
        if (result == U_RESULT_OK) {
            assert(instance != NULL);
            arg.action = action;
            arg.arg = actionArg;
            arg.result = 0;
            result = u_resultFromKernel(v_queryTakeInstance(query, instance, readAction, &arg, timeout));
            u_instanceHandleRelease(handle);
        }

        u_observableRelease(u_observable(_this), C_MM_RESERVATION_LOW);
    } else {
        OS_REPORT(OS_WARNING, "u_queryTakeInstance", result,
                  "query could not be claimed.");
    }
    return result;
}

u_result
u_queryReadNextInstance(
    const u_query _this,
    u_instanceHandle handle,
    u_readerAction action,
    void *actionArg,
    const os_duration timeout)
{
    v_query query;
    u_result result;
    v_dataReaderInstance instance;
    C_STRUCT(readActionArg) arg;

    assert(_this);
    assert(action);
    assert(OS_DURATION_ISPOSITIVE(timeout));

    result = u_observableReadClaim(u_observable(_this),(v_public *)(&query), C_MM_RESERVATION_LOW);
    if (result == U_RESULT_OK) {
        arg.action = action;
        arg.arg = actionArg;
        arg.result = 0;
        assert(query);
        if ( u_instanceHandleIsNil(handle) ) {
            v_queryReadNextInstance(query, NULL, readAction, &arg, timeout);
        } else {
            handle = u_instanceHandleFix(handle,v_collection(query));
            result = u_instanceHandleClaim(handle, &instance);
            if (result == U_RESULT_HANDLE_EXPIRED) {
                /* The handle has become invalid and no instance including
                 * the key value can be found. Therefore set the instance
                 * to null and start reading from scratch.
                 */
                v_queryReadNextInstance(query, NULL, readAction, &arg, timeout);
                result = U_RESULT_OK;
            } else if (result == U_RESULT_OK) {
                result = u_resultFromKernel(v_queryReadNextInstance(query, instance, readAction, &arg, timeout));
                u_instanceHandleRelease(handle);
            }
        }
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_LOW);
    } else {
        OS_REPORT(OS_WARNING, "u_queryReadNextInstance", result,
                  "query could not be claimed.");
    }

    return result;
}

u_result
u_queryTakeNextInstance(
    const u_query _this,
    u_instanceHandle handle,
    u_readerAction action,
    void *actionArg,
    const os_duration timeout)
{
    v_query  query;
    u_result result;
    v_dataReaderInstance instance;
    C_STRUCT(readActionArg) arg;

    assert(_this);
    assert(action);
    assert(OS_DURATION_ISPOSITIVE(timeout));

    result = u_observableReadClaim(u_observable(_this),(v_public *)(&query), C_MM_RESERVATION_LOW);
    if (result == U_RESULT_OK) {
        arg.action = action;
        arg.arg = actionArg;
        arg.result = 0;
        assert(query);
        if ( u_instanceHandleIsNil(handle) ) {
            v_queryTakeNextInstance(query, NULL, readAction, &arg, timeout);
        } else {
            handle = u_instanceHandleFix(handle,v_collection(query));
            result = u_instanceHandleClaim(handle, &instance);
            if (result == U_RESULT_HANDLE_EXPIRED) {
                /* The handle has become invalid and no instance including
                 * the key value can be found. Therefore set the instance
                 * to null and start reading from scratch.
                 */
                v_queryTakeNextInstance(query, NULL, readAction, &arg, timeout);
                result = U_RESULT_OK;
            } else if (result == U_RESULT_OK) {
                result = u_resultFromKernel(v_queryTakeNextInstance(query, instance, readAction, &arg, timeout));
                u_instanceHandleRelease(handle);
            }
        }
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_LOW);
    } else {
        OS_REPORT(OS_WARNING, "u_queryTakeNextInstance", result,
                  "query could not be claimed.");
    }

    return result;
}

