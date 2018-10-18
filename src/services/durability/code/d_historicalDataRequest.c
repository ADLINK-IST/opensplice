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
#include "d__readerRequest.h"
#include "d__historicalDataRequest.h"
#include "d__admin.h"
#include "d__thread.h"
#include "d__durability.h"
#include "d__misc.h"
#include "d__configuration.h"
#include "d__nameSpace.h"
#include "v_kernel.h"
#include "client_durabilitySplType.h"
#include "vortex_os.h"
#include "os_time.h"


static c_bool
vendor_is_prismtech_lite (struct _DDS_VendorId_t vendor)
{
  const struct _DDS_VendorId_t lite = D_VENDORID_PRISMTECH_LITE;
  return ((vendor.vendorId[0] == lite.vendorId[0]) && (vendor.vendorId[1] == lite.vendorId[1]));
}

static c_bool
is_Y2038READY(struct _DDS_DurabilityVersion_t version) {
    /* Extended timestamps are supported for all versions 2.0 and above */
    if ((version.major < (c_ushort)2)) {
        return FALSE;
    }
    return TRUE;
}


static os_timeW
ddsTimeToTimeW(struct _DDS_Time_t t, struct _DDS_DurabilityVersion_t version)
{
    os_timeW t2;

    if ((t.sec == (c_long)-1) && (t.nanosec == (c_ulong)4294967295U)) {
        /* DDS_TIME_INVALID */
        t2 = OS_TIMEW_INVALID;
    } else {
        if (is_Y2038READY(version)) {
            /* The sender of the historicalDataRequest used timestamps beyond 2038 */
            t2 = OS_TIMEW_INIT(0, (((os_uint64)(t.sec)) << 32) + (os_uint64)(t.nanosec));
        } else {
            /* The timestamp was a legacy timestamp.
             * Since the timestamp effectively contains a c_time, we convert
             * the corresponding c_time to an os_timeW. */
            c_time ct;

            ct.seconds = t.sec;
            ct.nanoseconds = t.nanosec;
            t2 = c_timeToTimeW(ct);
        }
    }
    return t2;
}


static void
historicalDataRequestCopyRequest(
    d_historicalDataRequest historicalDataRequest,
    const struct _DDS_HistoricalDataRequest *request)
{
    c_ulong i;
    struct _DDS_RequestId_t *requestId;
    os_timeE now = os_timeEGet();

    assert(d_historicalDataRequestIsValid(historicalDataRequest));
    assert(request);

    historicalDataRequest->version = request->version;
    requestId = (struct _DDS_RequestId_t *)os_malloc(sizeof(struct _DDS_RequestId_t));
    (*requestId) = request->requestId;
    historicalDataRequest->requestIds = c_iterNew(requestId);
    historicalDataRequest->topic = (request->topic == NULL) ? NULL : os_strdup(request->topic);
    historicalDataRequest->partitions = c_iterNew(NULL);
    for (i=0; i<c_sequenceSize(request->partitions); i++) {
        c_iterAppend(historicalDataRequest->partitions, os_strdup(request->partitions[i]));
    }
    historicalDataRequest->serializationFormat = request->serializationFormat;
    historicalDataRequest->dataReaderQos = request->dataReaderQos;
    /* Convert startTime and endTime */
    historicalDataRequest->startTime = ddsTimeToTimeW(request->startTime, request->version);
    historicalDataRequest->endTime = ddsTimeToTimeW(request->endTime, request->version);
    /* If an empty filter is received then do not use a filter */
    if ((request->sqlFilter == NULL) || (strcmp(request->sqlFilter, "")==0)) {
        historicalDataRequest->sqlFilter = NULL;
    } else {
        historicalDataRequest->sqlFilter = os_strdup(request->sqlFilter);
    }
    /* Create list of pointers that refer to parameters of the sql expression.
     * This is the required format for v_historicalDataRequestNew
     * (see d_historicalDataRequestListener.c) */
    historicalDataRequest->sqlFilterParamsSize = c_sequenceSize(request->sqlFilterParams);
    if (historicalDataRequest->sqlFilterParamsSize > 0) {
        historicalDataRequest->sqlFilterParams = (char **)os_malloc(historicalDataRequest->sqlFilterParamsSize * sizeof(*historicalDataRequest->sqlFilterParams));
        for (i=0; i<historicalDataRequest->sqlFilterParamsSize; i++) {
            historicalDataRequest->sqlFilterParams[i] = os_strdup(request->sqlFilterParams[i]);
        }
    } else {
        historicalDataRequest->sqlFilterParams = NULL;
    }
    historicalDataRequest->maxSamples = request->maxSamples;
    historicalDataRequest->maxInstances = request->maxInstances;
    historicalDataRequest->maxSamplesPerInstance = request->maxSamplesPerInstance;
    historicalDataRequest->alignmentPartition = c_iterNew(NULL);
    for (i=0; i<c_sequenceSize(request->alignmentPartition); i++) {
        c_iterAppend(historicalDataRequest->alignmentPartition, os_strdup(request->alignmentPartition[i]));
    }
    historicalDataRequest->serverIds = c_iterNew(NULL);
    for (i=0; i<c_sequenceSize(request->serverIds); i++) {
        struct _DDS_Gid_t *serverIds = (struct _DDS_Gid_t *)request->serverIds;
        struct _DDS_Gid_t *serverId = (struct _DDS_Gid_t *)os_malloc(sizeof(struct _DDS_Gid_t));
        *serverId = serverIds[i];
        c_iterAppend(historicalDataRequest->serverIds, serverId);
    }
    historicalDataRequest->receptionTime = now;
    /* Convert DDS_Duration to os_duration */
    historicalDataRequest->timeout = OS_DURATION_INIT(request->timeout.sec, request->timeout.nanosec);
    historicalDataRequest->extensions = c_iterNew(NULL);
    for (i=0; i<c_sequenceSize(request->extensions); i++) {
        c_ulong size;
        struct _DDS_NameValue_t *extensions = (struct _DDS_NameValue_t *)request->extensions;
        struct _DDS_NameValue_t *extension = (struct _DDS_NameValue_t *)os_malloc(sizeof(struct _DDS_NameValue_t));
        /* skip extensions that do not have a name or value */
        extension->name = (extensions[i].name == NULL) ? NULL : os_strdup(extensions[i].name);
        if (extension->name != NULL) {
            size = c_sequenceSize(extensions[i].value);
            extension->value = (size == 0) ? NULL : os_malloc(size * sizeof(c_octet));
            if (extension->value) {
                memcpy(extension->value, extensions[i].value, size);
                c_iterAppend(historicalDataRequest->extensions, extension);
            } else {
                os_free(extension->name);
            }
        }
    }
    return;
}


d_historicalDataRequest
d_historicalDataRequestNew(
    d_admin admin,
    struct _DDS_HistoricalDataRequest *request)
{
    d_historicalDataRequest historicalDataRequest;
    d_durability durability;

    assert(request);

    durability = d_adminGetDurability(admin);
    OS_UNUSED_ARG(durability);
    /* Allocate historical data request */
    historicalDataRequest = d_historicalDataRequest(os_malloc(C_SIZEOF(d_historicalDataRequest)));
    memset(historicalDataRequest, 0, (os_uint32)sizeof(C_STRUCT(d_historicalDataRequest)));
    /* Call super-init */
    d_objectInit(d_object(historicalDataRequest), D_HISTORICAL_DATA_REQ,
                 (d_objectDeinitFunc)d_historicalDataRequestDeinit);
    /* Initialize historicalDataRequest */

    historicalDataRequestCopyRequest(historicalDataRequest, request);
    /* Determine if the request is meant for me */
    (void)d_durabilityRequestIsForMe(durability, historicalDataRequest->serverIds,
                               &historicalDataRequest->forMe, &historicalDataRequest->forEverybody);
    /* Response control settings */
    historicalDataRequest->include_payload_unregistrations = (c_bool) (vendor_is_prismtech_lite(request->version.vendorId) ? FALSE : TRUE);
    return historicalDataRequest;
}


void
d_historicalDataRequestDeinit(
    d_historicalDataRequest historicalDataRequest)
 {
    char *str;
    struct _DDS_Gid_t *serverId;
    struct _DDS_NameValue_t *extension;
    c_ulong i;


    assert(d_historicalDataRequestIsValid(historicalDataRequest));

    if (historicalDataRequest->topic) {
        os_free(historicalDataRequest->topic);
    }
    if (historicalDataRequest->partitions) {
        while ((str = (char *)c_iterTakeFirst(historicalDataRequest->partitions)) != NULL) {
            os_free(str);
        }
        c_iterFree(historicalDataRequest->partitions);
    }
    if (historicalDataRequest->sqlFilter) {
        os_free(historicalDataRequest->sqlFilter);
    }
    if (historicalDataRequest->sqlFilterParams) {
        for (i=0; i<historicalDataRequest->sqlFilterParamsSize; i++) {
            os_free(historicalDataRequest->sqlFilterParams[i]);
        }
        os_free(historicalDataRequest->sqlFilterParams);
    }
    if (historicalDataRequest->alignmentPartition) {
        while ((str = (char *)c_iterTakeFirst(historicalDataRequest->alignmentPartition)) != NULL) {
            os_free(str);
        }
        c_iterFree(historicalDataRequest->alignmentPartition);
    }
    if (historicalDataRequest->serverIds) {
        while ((serverId = (struct _DDS_Gid_t *)c_iterTakeFirst(historicalDataRequest->serverIds)) != NULL) {
            os_free(serverId);
        }
        c_iterFree(historicalDataRequest->serverIds);
    }
    if (historicalDataRequest->extensions) {
        while ((extension = (struct _DDS_NameValue_t *)c_iterTakeFirst(historicalDataRequest->extensions)) != NULL) {
            os_free(extension->name);
            os_free(extension->value);
            os_free(extension);
        }
        c_iterFree(historicalDataRequest->extensions);
    }
    /* call super-deinit */
    d_objectDeinit(d_object(historicalDataRequest));
}


void
d_historicalDataRequestFree(
    d_historicalDataRequest historicalDataRequest)
{
    assert(d_historicalDataRequestIsValid(historicalDataRequest));

    d_objectFree(d_object(historicalDataRequest));
}


static c_bool
time_is_sane(os_timeW t, c_bool isY2038Ready)
{
    c_bool is_sane;

    if (isY2038Ready) {
        /* The startTime is a 64-bit time (os_timeW).
         * A valid time must be normalized */
        is_sane = (OS_TIMEW_ISINVALID(t) || OS_TIMEW_ISNORMALIZED(t));
    } else {
        /* The startTime is a legacy time (c_time).
         * Either the time must be C_TIME_INVALID or it must be a valid time */
        c_time ct = c_timeFromTimeW(t);
        is_sane = (c_timeIsInvalid(ct) || c_timeValid(ct));
    }
    return is_sane;
}


/**
 *
 *
 * \brief Check if the historicalDataRequest contains errors.
 *
 * If errors are encountered, the error code and error string are returned.
 * Only one error message will be returned.
 *
 * @return 0, if no errors are detected
 * @return errno, if an error is detected
 */
c_ulong
d_historicalDataRequestSanityCheck(
    d_historicalDataRequest historicalDataRequest)
{
    assert(d_historicalDataRequestIsValid(historicalDataRequest));

    /* perform sanity checks */
    if (historicalDataRequest->topic == NULL) {
        /* topic is NULL */
        return 2;
    } else if (!time_is_sane(historicalDataRequest->startTime, is_Y2038READY(historicalDataRequest->version))) {
        /* not a valid startTime */
        return 3;
    } else if (!time_is_sane(historicalDataRequest->endTime, is_Y2038READY(historicalDataRequest->version))) {
        /* not a valid endTime */
        return 4;
    } else if ( (!OS_TIMEW_ISINVALID(historicalDataRequest->startTime)) &&
                (!OS_TIMEW_ISINVALID(historicalDataRequest->endTime)) &&
                (os_timeWCompare(historicalDataRequest->startTime, historicalDataRequest->endTime) == OS_MORE) ) {
        /* starttime > endTime */
        return 5;
    } else if ( (historicalDataRequest->serializationFormat > PAYLOAD_SERIALIZATION_FORMAT_CDR_LE) ) {
        /* invalid serializationFormat */
        return 6;
    } else if ( c_iterLength(historicalDataRequest->partitions) == 0 ) {
        /* no partitions */
        return 7;
    } else if ( (historicalDataRequest->maxSamples != V_LENGTH_UNLIMITED) && (historicalDataRequest->maxSamples <= 0) ) {
        /* invalid maxSamples */
        return 8;
    } else if ( (historicalDataRequest->maxInstances != V_LENGTH_UNLIMITED) && (historicalDataRequest->maxInstances <= 0) ) {
        /* invalid maxInstances */
        return 9;
    } else if ( (historicalDataRequest->maxSamplesPerInstance != V_LENGTH_UNLIMITED) && (historicalDataRequest->maxSamplesPerInstance <= 0) ) {
        /* invalid maxSamplesPerInstance */
        return 10;
    } else if ( (OS_DURATION_ISINVALID(historicalDataRequest->timeout)) ||
                (os_durationCompare(historicalDataRequest->timeout, OS_DURATION_ZERO) == OS_LESS) ) {
        /* invalid timeout */
        return 11;
    }
    /* No sanity error */
    return 0;
}



int
d_historicalDataRequestCompareByRequestId(
    d_historicalDataRequest historicalDataRequest1,
    d_historicalDataRequest historicalDataRequest2)
{
    struct _DDS_RequestId_t *request1, *request2;

    assert(d_historicalDataRequestIsValid(historicalDataRequest1));
    assert(d_historicalDataRequestIsValid(historicalDataRequest2));

    /* NOTE : for the moment assume there is only a single requestId */

    assert(c_iterLength(historicalDataRequest1->requestIds) == 1);
    assert(c_iterLength(historicalDataRequest2->requestIds) == 1);

    request1 = (struct _DDS_RequestId_t *)c_iterObject(historicalDataRequest1->requestIds, 0);
    request2 = (struct _DDS_RequestId_t *)c_iterObject(historicalDataRequest2->requestIds, 0);
    if (request1->clientId.prefix < request2->clientId.prefix) {
        return -1;
    } else if (request1->clientId.prefix > request2->clientId.prefix) {
        return 1;
    } else if (request1->clientId.suffix < request2->clientId.suffix) {
        return -1;
    } else if (request1->clientId.suffix > request2->clientId.suffix) {
        return 1;
    } else if (request1->requestId < request2->requestId) {
        return -1;
    } else if (request1->requestId < request2->requestId) {
        return 1;
    }
    return 0;
}

