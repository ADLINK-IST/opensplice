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
#include "d_sampleRequest.h"
#include "d__readerRequest.h"
#include "d__admin.h"
#include "d__misc.h"
#include "d_message.h"
#include "d_networkAddress.h"
#include "d__fellow.h"
#include "d__thread.h"
#include "d__durability.h"
#include "d__configuration.h"
#include "d__groupHash.h"
#include "vortex_os.h"


/**
 * Compare two filter expressions and their parameters.
 *
 * Note: This function is called only if the partition, topic and source
 * of the sampleRequest already match.
 */
static int
compareFilters(
    d_sampleRequest request1,  /* the current request in the table */
    d_sampleRequest request2)  /* the request to add */
{
    int result = 0;
    c_ulong i;

    assert(request1);
    assert(request2);

    if (request1->filter == request2->filter) {
        return 0;
    } else if (request1->filter == NULL) {
        return 1;
    } else if (request2->filter == NULL) {
        return -1;
    } else {
        /* request1->filter != NULL and request2->filter != NULL.
         * Compare the filter and the parameters
         */
        if (request1->filterParamsCount < request2->filterParamsCount) {
            return -1;
        } else if (request1->filterParamsCount > request2->filterParamsCount) {
            return 1;
        } else {
            result = strcmp(request1->filter, request2->filter);
            if (result == 0) {
                /* The requests and the number of parameters match, now
                 * now check if the parameters match
                 */
                for (i=0; i<request1->filterParamsCount; i++) {
                    result = strcmp(request1->filterParams[i], request2->filterParams[i]);
                    if (result != 0) {
                        /* Not equal */
                        break;
                    }
                } /* for */
            }
        }
    }
    return result;
}


d_sampleRequest
d_sampleRequestNew(
    d_admin admin,
    c_char* partition,
    c_char* topic,
    d_durabilityKind kind,
    os_timeW requestTime,
    c_bool withTimeRange,
    os_timeW beginTime,
    os_timeW endTime)
{
    d_sampleRequest sampleRequest = NULL;
    char *sqlExpression;

    assert(d_adminIsValid(admin));
    assert(partition);
    assert(topic);

    /* Allocate sampleRequest */
    sampleRequest = d_sampleRequest(os_malloc(C_SIZEOF(d_sampleRequest)));
    if (sampleRequest) {
        /* Initialize to NULL */
        memset(sampleRequest, 0, sizeof(C_STRUCT(d_sampleRequest)));
        /* Call super-init */
        d_messageInit(d_message(sampleRequest), admin);
        /* Initialize sampleRequest */
        sampleRequest->partition = os_strdup(partition);
        sampleRequest->topic = os_strdup(topic);
        sampleRequest->durabilityKind = kind;
        d_timestampFromTimeW(&sampleRequest->requestTime, &requestTime, IS_Y2038READY(sampleRequest));
        sampleRequest->withTimeRange = withTimeRange;
        d_timestampFromTimeW(&sampleRequest->beginTime, &beginTime, IS_Y2038READY(sampleRequest));
        d_timestampFromTimeW(&sampleRequest->endTime, &endTime, IS_Y2038READY(sampleRequest));
        /* Every fellow that responds to a request will copy
         * the source in its response. The response will be
         * stored in the chain that matches the partition,topic
         * and source.
         */
        sampleRequest->source.systemId = 0;
        sampleRequest->source.localId  = 0;
        sampleRequest->source.lifecycleId = 0;
        sqlExpression = d_adminGetStaticFilterExpression(admin, partition, topic);
        if (sqlExpression) {
            sampleRequest->filter = os_strdup(sqlExpression);
        } else {
            sampleRequest->filter = NULL;
        }
        /* For the feature AlignOnChange/equalityCheck the filterParams string
         * sequence is extended with a hash string, this hash string is not included
         * in the filterParamsCount. The hash string is the first following entry
         * after the filters in the filterParams sequence. The durability capabilities
         * topic announces if a hash string is included in the sampleRequest message.
         * When announced a hash string should be included in ALL sampleRequest
         * messages, when no hash is calculated the string can be empty.
         */
        sampleRequest->filterParams = os_malloc(sizeof(c_string));
        sampleRequest->filterParams[0] = os_strdup("");
        sampleRequest->filterParamsCount = 0;
        sampleRequest->maxSamples = -1;
        sampleRequest->maxInstances = -1;
        sampleRequest->maxSamplesPerInstance = -1;
    }
    return sampleRequest;
}


/**
 * Create a copy from a sampleRequest
 *
 * For backwards compatibility reasons an empty hash is included
 * in the target if the originating sampleRequest did not have
 * a hash.
 */
d_sampleRequest
d_sampleRequestCopy(
    d_sampleRequest request,
    c_bool inSharedMemory)
{
    d_sampleRequest copy = NULL;
    c_ulong i;

    if (request != NULL) {
        copy = d_sampleRequest(os_malloc(C_SIZEOF(d_sampleRequest)));

        /* Copy the message header of the request.
         * Make sure to also the copy the productionTimestamp because
         * bit 30 encodes how time stamps must be interpreted. This
         * knowledge is used to answer the request later on.
         */
        d_messageSetAddressee(d_message(copy), &(d_message(request)->addressee));
        d_messageSetSenderAddress(d_message(copy), &(d_message(request)->senderAddress));
        d_message(copy)->productionTimestamp = d_message(request)->productionTimestamp;
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

        copy->filterParamsCount = request->filterParamsCount;
        /* Create sequence for filters and equalityCheck hash string (+1) */
        copy->filterParams = (c_sequence)os_malloc(
                                        (request->filterParamsCount + 1) * sizeof(c_string));
        for(i=0; i<request->filterParamsCount; i++){
            copy->filterParams[i]  = os_strdup(request->filterParams[i]);
        }
        /* Add equalityCheck hash string as last entry to filterParams sequence.
         * Only do this if the originating request has a hash, otherwise assume
         * that the hash is empty (which is interpreted as 'there is no hash')
         */
        if (!inSharedMemory || c_arraySize (request->filterParams) > request->filterParamsCount) {
            copy->filterParams[copy->filterParamsCount] = os_strdup(request->filterParams[request->filterParamsCount]);
        } else {
            copy->filterParams[copy->filterParamsCount] = os_strdup("");
        }
        copy->maxSamples               = request->maxSamples;
        copy->maxInstances             = request->maxInstances;
        copy->maxSamplesPerInstance    = request->maxSamplesPerInstance;
    }

    return copy;
}


void
d_sampleRequestSetCondition(
    d_sampleRequest request,
    d_readerRequest condition)
{
    c_ulong i;
    c_string hash;

    if(request){
        if(request->filter){
            os_free(request->filter);
            request->filter = NULL;
        }
        if(condition->filter){
            request->filter = os_strdup(condition->filter);
        }
        /* filterParams can never be NULL, because of the equalityCheck hash string */
        assert(request->filterParams);
        for(i=0; i<request->filterParamsCount; i++){
            os_free(request->filterParams[i]);
        }
        hash = request->filterParams[i];
        os_free(request->filterParams);

        /* filterParams can never be NULL, make sure it never is */
        request->filterParamsCount = condition->filterParamsCount;
        /* Create sequence for filters and equalityCheck hash string (+1) */
        request->filterParams = (c_sequence)os_malloc(
                            (condition->filterParamsCount + 1) * sizeof(c_char*));

        for(i=0; i<condition->filterParamsCount; i++){
            request->filterParams[i] = os_strdup(condition->filterParams[i]);
        }
        /* Add equalityCheck hash string as last entry to filterParams sequence */
        request->filterParams[i] = hash;

        request->source.systemId       = condition->readerHandle.index;
        request->source.localId        = condition->readerHandle.serial;
        request->source.lifecycleId    = 0;
        request->withTimeRange         = TRUE;
        request->beginTime             = condition->minSourceTimestamp;
        request->endTime               = condition->maxSourceTimestamp;
        request->maxSamples            = condition->resourceLimits.v.max_samples;
        request->maxInstances          = condition->resourceLimits.v.max_instances;
        request->maxSamplesPerInstance = condition->resourceLimits.v.max_samples_per_instance;
    }
    return;
}

void
d_sampleRequestSetSource(
    d_sampleRequest request,
    d_networkAddress source)
{
    if (request && source) {
        request->source.systemId       = source->systemId;
        request->source.localId        = source->localId;
        request->source.lifecycleId    = source->lifecycleId;
    }
    return;
}

void
d_sampleRequestSetHash(
    d_sampleRequest request,
    c_string hash)
{
    if (request) {
        assert(request->filterParams);
        os_free(request->filterParams[request->filterParamsCount]);
        request->filterParams[request->filterParamsCount] = os_strdup(hash);
    }
    return;
}

c_bool
d_sampleRequestHasHash(
    d_sampleRequest request,
    d_fellow fellow)
{
    c_bool set = FALSE;
    c_string hash;

    assert(fellow);

    if (request && d_fellowHasCapabilityGroupHash(fellow)) {
        assert(request->filterParams);
        hash = request->filterParams[request->filterParamsCount];
        if (hash[0] != 0) {
            set = TRUE;
        }
    }
    return set;
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
            os_free(sampleRequest->filterParams[i]);
            os_free(sampleRequest->filterParams);
        }
        d_messageDeinit(d_message(sampleRequest));
        os_free(sampleRequest);
    }
}


c_bool
d_sampleRequestSpecificReader(
    d_sampleRequest request)
{
    c_bool result = FALSE;
    d_networkAddress source;

    assert(request);

    if (request) {
        source = &request->source;
        if (d_networkAddressIsUnaddressed(source) ||
            (source->systemId == 0)) {
            /* The request does not originate from a specific reader */
            result = FALSE;
        } else {
            /* A specific reader originated the request */
            result = TRUE;
        }
    }
    return result;
}

c_bool
d_sampleRequestHasCondition(
     d_sampleRequest request)
{
    c_bool result = FALSE;

    assert(request);

    if (request) {
        if ( (request->filter) ||
            (!c_timeIsMinInfinite(request->beginTime) && !(c_timeIsZero(request->beginTime) && c_timeIsInfinite(request->endTime))) ||
            (request->maxInstances != -1) ||
            (request->maxSamples != -1) ||
            (request->maxSamplesPerInstance != -1)) {
            result = TRUE;
        }
    }

    return result;
}


static c_bool
has_matching_capabilities(d_sampleRequest request1, d_sampleRequest request2)
{
    d_fellow fellow1;
    d_fellow fellow2;
    d_durability durability = d_threadsDurability();
    d_admin admin = durability->admin;
    c_bool result = FALSE;  /* Initially no match if one of the fellow do not exist */

    fellow1 = d_adminGetFellow(admin, &d_message(request1)->senderAddress);
    fellow2 = d_adminGetFellow(admin, &d_message(request2)->senderAddress);

    if (fellow1 && fellow2) {
        /* Assume match until there is a mismatch */
        result = TRUE;

        /* check hash capability mismatch */
        if (d_fellowHasCapabilityGroupHash(fellow1) != d_fellowHasCapabilityGroupHash(fellow2)) {

            d_trace(D_TRACE_COMBINE_REQUESTS, "  %s - no matching group hash\n", OS_FUNCTION);
            result = FALSE;

        /* check EOT support mismatch */
        } else  if (d_fellowHasCapabilityEOTSupport(fellow1) != d_fellowHasCapabilityEOTSupport(fellow2)) {
            d_trace(D_TRACE_COMBINE_REQUESTS, "  %s - no matching group eot support\n", OS_FUNCTION);
            result = FALSE;
        }
    }
    if (fellow1) {

        d_fellowFree(fellow1);
    }
    if (fellow2) {
        d_fellowFree(fellow2);
    }
    return result;
}

/* This function is used to check if two sample requests can be combined
 * when the group hash is used by the fellows.
 * When the group hash is available in the sample requests then the hash
 * values of the sample requests are compared and when they are the same
 * the sample requests can be combined. When the fellows do not have
 * the group hash capability then the function returns TRUE.
 *
 * The precondition is that either both fellows have the group hash
 * capability or both do not have this.
 */
static c_bool
has_matching_hash_values(d_sampleRequest request1, d_sampleRequest request2)
{
    d_fellow fellow1;
    d_fellow fellow2;
    d_durability durability = d_threadsDurability();
    d_admin admin = durability->admin;
    c_bool result = FALSE;

    fellow1 = d_adminGetFellow(admin, &d_message(request1)->senderAddress);
    fellow2 = d_adminGetFellow(admin, &d_message(request2)->senderAddress);

    if (fellow1 && fellow2) {
        result = TRUE;
        if (d_fellowHasCapabilitySupport(fellow1) && d_fellowHasCapabilityGroupHash(fellow1)) {
            c_char *hstr1, *hstr2;
            struct d_groupHash hash1, hash2;

            assert(d_fellowHasCapabilitySupport(fellow2));
            assert(d_fellowHasCapabilityGroupHash(fellow2));

            hstr1 = request1->filterParams[request1->filterParamsCount];
            hstr2 = request2->filterParams[request2->filterParamsCount];
            if (d_groupHashFromString(&hash1, hstr1) && d_groupHashFromString(&hash2, hstr2)) {
                result = d_groupHashIsEqual(&hash1, &hash2);
            }
        }
    }

    if (fellow1) {
        d_fellowFree(fellow1);
    }
    if (fellow2) {
        d_fellowFree(fellow2);
    }

    return result;
}


/* Check if sampleRequests can be combined */
c_bool
d_sampleRequestCanCombine(
    d_sampleRequest request1,
    d_sampleRequest request2)
{
    assert(request1);
    assert(request2);

    /* Check for partition match */
    if (strcmp(request1->partition, request2->partition) != 0) {
        return FALSE;
    }
    /* Check for topic match */
    if (strcmp(request1->topic, request2->topic) != 0) {
        return FALSE;
    }
    /* Check if durabilityKind matches */
    if (request1->durabilityKind != request2->durabilityKind) {
        return FALSE;
    }
    /* Check if source matches */
    if (d_networkAddressCompare(&request1->source, &request2->source) != 0) {
        return FALSE;
    }
    /* Check if filter matches */
    if (compareFilters(request1, request2) != 0) {
        d_trace(D_TRACE_COMBINE_REQUESTS, "%s - no matching filter\n", OS_FUNCTION);
        return FALSE;
    }
    /* Check if capabilities match */
    if (!has_matching_capabilities(request1, request2)) {
        d_trace(D_TRACE_COMBINE_REQUESTS, "%s - no matching capabilities\n", OS_FUNCTION);
        return FALSE;
    }

    /* Check if hashes match */
    if (!has_matching_hash_values(request1, request2)) {
        d_trace(D_TRACE_COMBINE_REQUESTS, "%s - no matching hash values\n", OS_FUNCTION);
        return FALSE;
    }

    return TRUE;
}
