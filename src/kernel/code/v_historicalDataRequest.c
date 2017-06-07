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
#include "v_historicalDataRequest.h"
#include "v_policy.h"
#include "os_report.h"

v_historicalDataRequest
v_historicalDataRequestNew(
    v_kernel kernel,
    const c_char* filter,
    const c_char* params[],
    c_ulong nofParams,
    os_timeW minSourceTime,
    os_timeW maxSourceTime,
    v_resourcePolicyI *resourceLimits,
    os_duration timeout)
{
    v_historicalDataRequest request;
    c_ulong i;
    c_base base;

    request = v_historicalDataRequest(v_objectNew_s(kernel,K_HISTORICALDATAREQUEST));
    if (request) {
        if(filter){
            base = c_getBase(kernel);
            request->filter = c_stringNew_s(base, filter);
            if (!request->filter){
                goto err_alloc_filter;
            }

            if(params){
                request->filterParams = c_arrayNew_s(c_string_t(base), nofParams);
                if ((nofParams > 0) && (!request->filterParams)) {
                    goto err_alloc_params;
                }

                for(i=0; i<nofParams; i++){
                    request->filterParams[i] = c_stringNew_s(base, params[i]);
                    if (!request->filterParams[i]) {
                        goto err_alloc_param;
                    }
                }
            } else {
                request->filterParams = NULL;
            }
        } else {
            request->filter       = NULL;
            request->filterParams = NULL;
        }
        if (OS_TIMEW_ISINVALID(minSourceTime)) {
            request->minSourceTimestamp = OS_TIMEW_ZERO;
        } else {
            request->minSourceTimestamp = minSourceTime;
        }
        if (OS_TIMEW_ISINVALID(maxSourceTime)) {
            request->maxSourceTimestamp = OS_TIMEW_INFINITE;
        } else {
            request->maxSourceTimestamp = maxSourceTime;
        }

        request->resourceLimits.v.max_samples              = resourceLimits->v.max_samples;
        request->resourceLimits.v.max_instances            = resourceLimits->v.max_instances;
        request->resourceLimits.v.max_samples_per_instance = resourceLimits->v.max_samples_per_instance;
        request->timeout = timeout;
    } else {
        OS_REPORT(OS_FATAL,
                  "v_historicalDataRequestNew",V_RESULT_OUT_OF_MEMORY,
                  "Failed to allocate request.");
        assert(FALSE);
    }

    return request;

err_alloc_param:
err_alloc_params:
err_alloc_filter:
    OS_REPORT(OS_FATAL,
          "v_historicalDataRequestNew",V_RESULT_OUT_OF_MEMORY,
          "Failed to allocate request.");
    c_free(request);
    return NULL;
}


static c_bool
sourceTimestampIsEqual(
    os_timeW t1,
    os_timeW t2)
{
    c_bool result = TRUE;

    /* The sourceTimestamps can be OS_TIMEW_INVALID */
    if (!OS_TIMEW_ISINVALID(t1) && OS_TIMEW_ISINVALID(t2)) {
        result = FALSE;
    } else if (OS_TIMEW_ISINVALID(t1) && !OS_TIMEW_ISINVALID(t2)) {
        result = FALSE;
    } else if (!OS_TIMEW_ISINVALID(t1) && !OS_TIMEW_ISINVALID(t2)) {
        result = (os_timeWCompare(t1, t2) == OS_EQUAL);
    }
    return result;
}


c_bool
v_historicalDataRequestEquals(
    v_historicalDataRequest req1,
    v_historicalDataRequest req2)
{
    c_bool result = TRUE;
    c_ulong i, size1, size2;

    if(req1 && req2){
        if (!sourceTimestampIsEqual(req1->minSourceTimestamp, req2->minSourceTimestamp))
        {
            result = FALSE;
        } else if (!sourceTimestampIsEqual(req1->maxSourceTimestamp, req2->maxSourceTimestamp))
        {
            result = FALSE;
        } else if(req1->resourceLimits.v.max_samples !=
                  req2->resourceLimits.v.max_samples)
        {
            result = FALSE;
        } else if(req1->resourceLimits.v.max_instances !=
                  req2->resourceLimits.v.max_instances)
        {
            result = FALSE;
        } else if(req1->resourceLimits.v.max_samples_per_instance !=
                  req2->resourceLimits.v.max_samples_per_instance)
        {
            result = FALSE;
        } else if((req1->filter && req2->filter)){
            if(strcmp(req1->filter, req2->filter) != 0){
                result = FALSE;
            } else if(req1->filterParams && req2->filterParams){
                size1 = c_arraySize(req1->filterParams);
                size2 = c_arraySize(req2->filterParams);

                if(size1 == size2){
                    result = TRUE;
                    for(i=0; i<size1 && result; i++){
                        if(strcmp(req1->filterParams[i],
                                  req2->filterParams[i]) != 0)
                        {
                            result = FALSE;
                        }
                    }
                } else {
                    result = FALSE;
                }
            } else if(!req1->filterParams && !req2->filterParams){
                result = TRUE;
            } else {
                result = FALSE;
            }
        } else if((!req1->filter && !req2->filter)){
            result = TRUE;
        } else {
            result = FALSE;
        }
    } else if(req1 || req2){
        result = FALSE;
    } else {
        result = TRUE;
    }
    return result;
}

c_bool
v_historicalDataRequestIsValid(
    v_historicalDataRequest request,
    v_reader reader)
{
    c_bool result;
    q_expr expr;

    assert(C_TYPECHECK(request,v_historicalDataRequest));
    assert(C_TYPECHECK(reader,v_reader));

    if(request && reader){
        if(!v_resourcePolicyIValid(request->resourceLimits)) {
            result = FALSE;
        } else if((reader->qos->resource.v.max_samples != -1) &&
                  (reader->qos->resource.v.max_samples <
                   request->resourceLimits.v.max_samples)) {
            result = FALSE;
        } else if((reader->qos->resource.v.max_instances != -1) &&
                  (reader->qos->resource.v.max_instances <
                   request->resourceLimits.v.max_instances)) {
            result = FALSE;
        } else if((reader->qos->resource.v.max_samples_per_instance != -1) &&
                  (reader->qos->resource.v.max_samples_per_instance <
                   request->resourceLimits.v.max_samples_per_instance)) {
            result = FALSE;
        } else if(OS_TIMEW_ISINVALID(request->minSourceTimestamp)) {
            result = FALSE;
        } else if(OS_TIMEW_ISINVALID(request->maxSourceTimestamp)) {
            result = FALSE;
        } else if(os_timeWCompare(request->minSourceTimestamp,request->maxSourceTimestamp) == OS_MORE) {
            result = FALSE;
        } else if(request->filter){
            expr = q_parse(request->filter);

            if(expr){
                q_dispose(expr);
                result = TRUE;
            } else {
                result = FALSE;
            }
        } else {
            result = TRUE;
        }
    } else {
        result = FALSE;
    }
    return result;
}
