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
#include "v_historicalDataRequest.h"
#include "v_policy.h"
#include "os_report.h"

v_historicalDataRequest
v_historicalDataRequestNew(
    v_kernel kernel,
    c_char* filter,
    c_char* params[],
    c_ulong nofParams,
    c_time minSourceTime,
    c_time maxSourceTime,
    struct v_resourcePolicy *resourceLimits)
{
    v_historicalDataRequest request;
    c_ulong i;
    c_type type;
    c_base base;

    request = c_new(v_kernelType(kernel,K_HISTORICALDATAREQUEST));

    if (request) {
        if(filter){
            base            = c_getBase(kernel);
            request->filter = c_stringNew(base, filter);

            if(params){
                type                  = c_string_t(base);
                request->filterParams = c_arrayNew(type, nofParams);

                for(i=0; i<nofParams; i++){
                    request->filterParams[i] = c_stringNew(base, params[i]);
                }
            } else {
                request->filterParams = NULL;
            }
        } else {
            request->filter       = NULL;
            request->filterParams = NULL;
        }
        if ((minSourceTime.seconds     == C_TIME_INVALID.seconds) &&
            (minSourceTime.nanoseconds == C_TIME_INVALID.nanoseconds)) {
            request->minSourceTimestamp = C_TIME_ZERO;
        } else {
            request->minSourceTimestamp  = minSourceTime;
        }
        if ((maxSourceTime.seconds     == C_TIME_INVALID.seconds) &&
            (maxSourceTime.nanoseconds == C_TIME_INVALID.nanoseconds)) {
            request->maxSourceTimestamp = C_TIME_INFINITE;
        } else {
            request->maxSourceTimestamp  = maxSourceTime;
        }
        request->resourceLimits.max_samples              = resourceLimits->max_samples;
        request->resourceLimits.max_instances            = resourceLimits->max_instances;
        request->resourceLimits.max_samples_per_instance = resourceLimits->max_samples_per_instance;
    } else {
        OS_REPORT(OS_ERROR,
                  "v_historicalDataRequestNew",0,
                  "Failed to allocate request.");
        assert(FALSE);
    }

    return request;
}

c_bool
v_historicalDataRequestEquals(
    v_historicalDataRequest req1,
    v_historicalDataRequest req2)
{
    c_bool result;
    c_long i, size1, size2;

    if(req1 && req2){
        if(c_timeCompare(
                req1->minSourceTimestamp, req2->minSourceTimestamp) != C_EQ)
        {
            result = FALSE;
        } else if(c_timeCompare(
                req1->maxSourceTimestamp, req2->maxSourceTimestamp) != C_EQ)
        {
            result = FALSE;
        } else if(req1->resourceLimits.max_samples !=
                  req2->resourceLimits.max_samples)
        {
            result = FALSE;
        } else if(req1->resourceLimits.max_instances !=
                  req2->resourceLimits.max_instances)
        {
            result = FALSE;
        } else if(req1->resourceLimits.max_samples_per_instance !=
                  req2->resourceLimits.max_samples_per_instance)
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
        if(!v_resourcePolicyValid(request->resourceLimits)){
            result = FALSE;
        } else if((reader->qos->resource.max_samples != -1) &&
                  (reader->qos->resource.max_samples <
                   request->resourceLimits.max_samples))
        {
            result = FALSE;
        } else if((reader->qos->resource.max_instances != -1) &&
                  (reader->qos->resource.max_instances <
                   request->resourceLimits.max_instances))
        {
            result = FALSE;
        } else if((reader->qos->resource.max_samples_per_instance != -1) &&
                  (reader->qos->resource.max_samples_per_instance <
                   request->resourceLimits.max_samples_per_instance))
        {
            result = FALSE;
        } else if(!c_timeValid(request->minSourceTimestamp)){
            result = FALSE;
        } else if(!c_timeValid(request->maxSourceTimestamp)){
            result = FALSE;
        } else if(c_timeCompare(request->minSourceTimestamp,
                request->maxSourceTimestamp) == C_GT)
        {
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
