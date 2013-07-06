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
#include "d_sampleRequest.h"
#include "d__readerRequest.h"
#include "d_message.h"
#include "d_networkAddress.h"
#include "os.h"

d_sampleRequest
d_sampleRequestNew(
    d_admin admin,
    const c_char* partition,
    const c_char* topic,
    d_durabilityKind kind,
    d_timestamp requestTime,
    c_bool withTimeRange,
    d_timestamp beginTime,
    d_timestamp endTime)
{
    d_sampleRequest sampleRequest = NULL;

    if(admin){
        sampleRequest = d_sampleRequest(os_malloc(C_SIZEOF(d_sampleRequest)));
        d_messageInit(d_message(sampleRequest), admin);

        sampleRequest->partition                = os_strdup(partition);
        sampleRequest->topic                    = os_strdup(topic);
        sampleRequest->durabilityKind           = kind;
        sampleRequest->requestTime              = requestTime;
        sampleRequest->withTimeRange            = withTimeRange;
        sampleRequest->beginTime                = beginTime;
        sampleRequest->endTime                  = endTime;
        sampleRequest->source.systemId          = 0;
        sampleRequest->source.localId           = 0;
        sampleRequest->source.lifecycleId       = 0;
        sampleRequest->filter                   = NULL;
        sampleRequest->filterParams             = NULL;
        sampleRequest->filterParamsCount        = 0;
        sampleRequest->maxSamples               = -1;
        sampleRequest->maxInstances             = -1;
        sampleRequest->maxSamplesPerInstance    = -1;
    }
    return sampleRequest;
}

d_sampleRequest
d_sampleRequestCopy(
    d_sampleRequest request)
{
    d_sampleRequest copy;
    c_ulong i;

    if(request){
        copy = d_sampleRequest(os_malloc(C_SIZEOF(d_sampleRequest)));

        d_messageSetAddressee(d_message(copy), &(d_message(request)->addressee));
        d_messageSetSenderAddress(d_message(copy), &(d_message(request)->senderAddress));
        d_message(copy)->senderState   = d_message(request)->senderState;

        copy->partition                = os_strdup(request->partition);
        copy->topic                    = os_strdup(request->topic);
        copy->durabilityKind           = request->durabilityKind;
        copy->requestTime              = request->requestTime;
        copy->withTimeRange            = request->withTimeRange;
        copy->beginTime                = request->beginTime;
        copy->endTime                  = request->endTime;
        copy->source.systemId          = request->source.systemId;
        copy->source.localId           = request->source.localId;
        copy->source.lifecycleId       = request->source.lifecycleId;

        if(request->filter){
            copy->filter               = os_strdup(request->filter);
        } else {
            copy->filter               = NULL;
        }

        if(request->filterParamsCount > 0){
            copy->filterParamsCount = request->filterParamsCount;
            copy->filterParams = (c_array)os_malloc(
                                request->filterParamsCount * sizeof(c_char*));

            for(i=0; i<request->filterParamsCount; i++){
                copy->filterParams[i]  = os_strdup(request->filterParams[i]);
            }
        } else {
            copy->filterParamsCount    = 0;
            copy->filterParams         = NULL;
        }
        copy->maxSamples               = request->maxSamples;
        copy->maxInstances             = request->maxInstances;
        copy->maxSamplesPerInstance    = request->maxSamplesPerInstance;
    } else {
        copy = NULL;
    }

    return copy;
}

void
d_sampleRequestSetCondition(
    d_sampleRequest request,
    d_readerRequest condition)
{
    c_ulong i;

    if(request){
        if(request->filter){
            os_free(request->filter);
            request->filter = NULL;
        }
        if(condition->filter){
            request->filter = os_strdup(condition->filter);
        }
        if(request->filterParams){
            for(i=0; i<request->filterParamsCount; i++){
                os_free(request->filterParams[i]);
            }
            os_free(request->filterParams);
            request->filterParamsCount = 0;
        }
        if(condition->filterParamsCount > 0){
            request->filterParamsCount = condition->filterParamsCount;
            request->filterParams = (c_array)os_malloc(
                                condition->filterParamsCount * sizeof(c_char*));

            for(i=0; i<condition->filterParamsCount; i++){
                request->filterParams[i] = os_strdup(condition->filterParams[i]);
            }
        }

        request->source.systemId       = condition->readerHandle.index;
        request->source.localId        = condition->readerHandle.serial;
        request->source.lifecycleId    = 0;
        request->withTimeRange         = TRUE;
        request->beginTime             = condition->minSourceTimestamp;
        request->endTime               = condition->maxSourceTimestamp;
        request->maxSamples            = condition->resourceLimits.max_samples;
        request->maxInstances          = condition->resourceLimits.max_instances;
        request->maxSamplesPerInstance = condition->resourceLimits.max_samples_per_instance;
    }
    return;
}

void
d_sampleRequestSetSource(
    d_sampleRequest request,
    d_networkAddress source)
{
    if(request && source){
        request->source.systemId       = source->systemId;
        request->source.localId        = source->localId;
        request->source.lifecycleId    = source->lifecycleId;
    }
    return;
}

void
d_sampleRequestFree(
    d_sampleRequest sampleRequest)
{
    c_ulong i;

    if(sampleRequest){
        if(sampleRequest->partition){
            os_free(sampleRequest->partition);
        }
        if(sampleRequest->topic){
            os_free(sampleRequest->topic);
        }
        if(sampleRequest->filter){
            os_free(sampleRequest->filter);
        }
        if(sampleRequest->filterParams){
            for(i=0; i<sampleRequest->filterParamsCount; i++){
                os_free(sampleRequest->filterParams[i]);
            }
            os_free(sampleRequest->filterParams);
        }
        d_messageDeinit(d_message(sampleRequest));
        os_free(sampleRequest);
    }
}

int
d_sampleRequestCompare(
    d_sampleRequest request1,
    d_sampleRequest request2)
{
    int result;
    assert(request1);
    assert(request2);

    if(request1 && request2){
        result = strcmp(request1->partition, request2->partition);

        if(result == 0){
            result = strcmp(request1->topic, request2->topic);

            if(result == 0){
                if(request1->durabilityKind == request2->durabilityKind){
                    result = d_networkAddressCompare(
                                    &request1->source, &request2->source);

                } else if(request1->durabilityKind > request2->durabilityKind){
                    result = 1;
                } else {
                    result = -1;
                }
            }
        }
    } else if(request1){
        result = 1;
    } else {
        result = -1;
    }
    return result;
}

c_bool
d_sampleRequestHasCondition(
    d_sampleRequest request)
{
    c_bool result;

    assert(request);

    if(request){
        if(d_networkAddressIsUnaddressed(&request->source)){
            result = FALSE;
        } else {
            result = TRUE;
        }
    } else {
        result = FALSE;
    }
    return result;
}
