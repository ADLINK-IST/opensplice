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
#include "d__sampleChainListener.h"
#include "d__table.h"
#include "d__group.h"
#include "d__admin.h"
#include "d__misc.h"
#include "v_dataReader.h"
#include "v_reader.h"
#include "v_topic.h"
#include "v_partition.h"
#include "v_kernel.h"
#include "v_waitset.h"
#include "v_handle.h"
#include "os_heap.h"
#include "os_stdlib.h"

d_readerRequest
d_readerRequestProxyNew(
    v_handle source)
{
    d_readerRequest request;

    /* Allocate readerRequestProxy object */
    request = d_readerRequest(os_malloc(C_SIZEOF(d_readerRequest)));
    if (request) {
        /* Call super-init */
        d_lockInit(d_lock(request), D_READER_REQUEST,
                   (d_objectDeinitFunc)d_readerRequestDeinit);
        /* Initialize readerRequestProxy */
        request->admin               = NULL;
        request->readerHandle.index  = source.index;
        request->readerHandle.serial = source.serial;
        request->readerHandle.server = 0;
        request->requests            = NULL;
        request->filter              = NULL;
        request->filterParams        = NULL;
        request->filterParamsCount   = 0;
        request->groups              = NULL;
    }
    return request;
}

d_readerRequest
d_readerRequestNew(
    d_admin admin,
    v_waitsetEvent event)
{
    d_readerRequest request;
    c_ulong i;
    v_handleResult handleResult;
    v_reader vreader, *vreaderPtr;
    c_iter partitions;
    v_partition partition;
    v_topic topic;
    d_group dgroup;
    c_char *topicName;
    c_bool remove;

    assert(d_adminIsValid(admin));

    /* Allocate readerRequest object */
    request = d_readerRequest(os_malloc(C_SIZEOF(d_readerRequest)));
    if (request) {
        remove = FALSE;
        /* Call super-init */
        d_lockInit(d_lock(request), D_READER_REQUEST,
                   (d_objectDeinitFunc)d_readerRequestDeinit);
        /* Initialize readerRequest */
        request->admin               = admin;
        request->readerHandle        = v_waitsetEventSource(event);
        request->requests            = d_tableNew(d_chainCompare, d_chainFree);
        if (v_waitsetEventHistoryRequestFilter(event)) {
            request->filter          = os_strdup(v_waitsetEventHistoryRequestFilter(event));
        } else {
            request->filter          = NULL;
        }
        request->resourceLimits      = v_waitsetEventHistoryRequestResourceLimits(event);
        request->minSourceTimestamp  = c_timeFromTimeW(v_waitsetEventHistoryRequestMinTimestamp(event));
        request->maxSourceTimestamp  = c_timeFromTimeW(v_waitsetEventHistoryRequestMaxTimestamp(event));
        /* When maxSourceTimestamp is infinite and minSourceTimestamp is zero set
         * the minSourceTimestamp to MIN_INFINITE. In legacy products the
         * MIN_INFINITE lower time bound (when no other conditions are set) is used
         * to indicate that the request does not contain conditions. When the lower
         * time bound is ZERO the request is processed as if it had conditions.
         */
        if (c_timeIsInfinite(request->maxSourceTimestamp) &&
            c_timeIsZero(request->minSourceTimestamp)) {
            request->minSourceTimestamp = C_TIME_MIN_INFINITE;
        }

        request->filterParamsCount   = c_arraySize(v_waitsetEventHistoryRequestFilterParams(event));

        if (request->filterParamsCount > 0) {
            request->filterParams = (c_char**)(os_malloc(request->filterParamsCount*sizeof(c_char*)));
            for (i=0; i<request->filterParamsCount; i++) {
                request->filterParams[i] = os_strdup(v_waitsetEventHistoryRequestFilterParams(event)[i]);
            }
        } else {
            request->filterParams = NULL;
        }
        request->groupsIgnored = FALSE;
        request->groups = d_tableNew(d_groupCompare, d_groupFree);
        handleResult = v_handleClaim(request->readerHandle, (v_object*)(vreaderPtr = &vreader));

        if (handleResult == V_HANDLE_OK) {
            /* Apparently only reader requests for datareaders are allowed.
             * For non-datareaders the current implementation returns NULL.
             */
            if (v_objectKind(vreader) == K_DATAREADER) {
                topic = v_dataReaderGetTopic(v_dataReader(vreader));

                if(topic){
                    topicName  = v_entityName(topic);
                    partitions = v_readerGetPartitions(vreader);

                    if (partitions != NULL) {
                        partition  = v_partition(c_iterTakeFirst(partitions));

                        while (partition) {
                            dgroup = d_groupNew(
                                        v_entity(partition)->name, topicName,
                                        d_durabilityKindFromKernel (v_topicQosRef(topic)->durability.v.kind),
                                        D_GROUP_KNOWLEDGE_UNDEFINED, D_QUALITY_ZERO);
                            d_tableInsert(request->groups, dgroup);
                            c_free(partition);
                            partition  = v_partition(c_iterTakeFirst(partitions));
                        }
                        c_free(topic);
                        c_iterFree(partitions);
                    } else {
                        remove = TRUE;
                    }
                } else {
                    remove = TRUE;
                }
            } else {
                remove = TRUE;
            }
            v_handleRelease(request->readerHandle);
        } else {
            remove = TRUE;
        }
        if (remove == TRUE) {
            d_readerRequestFree(request);
            request = NULL;
        }
    }
    return request;
}

d_admin
d_readerRequestGetAdmin(
    d_readerRequest request)
{
    d_admin admin;

    assert(d_readerRequestIsValid(request));

    if (request) {
        admin = request->admin;
    } else {
        admin = NULL;
    }
    return admin;
}

static c_bool
addGroup(
    d_group group,
    c_voidp args)
{
    d_table table;

    table = d_table(args);
    d_tableInsert(table, d_objectKeep(d_object(group)));
    return TRUE;
}

d_table
d_readerRequestGetGroups(
    d_readerRequest request)
{
    d_table result;

    assert(d_readerRequestIsValid(request));

    if (request) {
        d_lockLock(d_lock(request));
        result = d_tableNew(d_groupCompare, d_groupFree);
        d_tableWalk(request->groups, addGroup, result);
        d_lockUnlock(d_lock(request));
    } else {
        result = NULL;
    }
    return result;
}

void
d_readerRequestRemoveGroup(
    d_readerRequest request,
    d_group group)
{
    d_group dgroup;

    assert(d_readerRequestIsValid(request));

    if (request) {
        d_lockLock(d_lock(request));
        dgroup = d_tableRemove(request->groups, group);
        d_lockUnlock(d_lock(request));

        if (dgroup) {
            d_groupFree(dgroup);
        }
    }
    return;
}

v_handle
d_readerRequestGetHandle(
    d_readerRequest request)
{
    v_handle handle;

    handle.index  = request->readerHandle.index;
    handle.serial = request->readerHandle.serial;
    handle.server = request->readerHandle.server;
    return handle;
}

c_long
d_readerRequestCompare(
    d_readerRequest request1,
    d_readerRequest request2)
{
    c_long result;

    if (request1 && request2) {
        if (request1->readerHandle.index < request2->readerHandle.index) {
            result = -1;
        } else if (request1->readerHandle.index > request2->readerHandle.index) {
            result = 1;
        } else if (request1->readerHandle.serial < request2->readerHandle.serial) {
            result = -1;
        } else if (request1->readerHandle.serial > request2->readerHandle.serial) {
            result = 1;
        } else {
            result = 0;
        }
    } else if(request1){
        result = 1;
    } else if(request2){
        result = -1;
    } else {
        result = 0;
    }
    return result;
}


void
d_readerRequestDeinit(
    d_readerRequest request)
{
    c_ulong i;

    assert(d_readerRequestIsValid(request));

    if (request->requests) {
        d_tableFree(request->requests);
        request->requests = NULL;
    }
    if (request->filter) {
        os_free(request->filter);
        request->filter = NULL;
    }
    for (i=0; i<request->filterParamsCount; i++) {
        os_free(request->filterParams[i]);
    }
    if (request->filterParams) {
        os_free(request->filterParams);
        request->filterParams = NULL;
    }
    if (request->groups) {
        d_tableFree(request->groups);
        request->groups = NULL;
    }
    /* Call super-deinit */
    d_lockDeinit(d_lock(request));
}

void
d_readerRequestFree(
    d_readerRequest request)
{
    assert(d_readerRequestIsValid(request));

    d_objectFree(d_object(request));
}

c_bool
d_readerRequestAddChain(
    d_readerRequest request,
    d_chain chain)
{
    c_bool result;
    d_chain found;

    assert(d_readerRequestIsValid(request));
    assert(d_chainIsValid(chain));

    if (request && chain) {
        d_lockLock(d_lock(request));
        found = d_tableInsert(request->requests, chain);
        d_lockUnlock(d_lock(request));

        if (found) {
            result = FALSE;
        } else {
            d_objectKeep(d_object(chain));
            result = TRUE;
        }
    } else {
        result = FALSE;
    }
    return result;
}

c_bool
d_readerRequestRemoveChain(
    d_readerRequest request,
    d_chain chain)
{
    d_chain found;
    c_bool result;

    assert(d_readerRequestIsValid(request));
    assert(d_chainIsValid(chain));

    if (request && chain) {
        d_lockLock(d_lock(request));
        found = d_tableRemove(request->requests, chain);
        d_lockUnlock(d_lock(request));

        if (found) {
            result = TRUE;
            d_chainFree(found);
        } else {
            result = FALSE;
        }
    } else {
        result = FALSE;
    }
    return result;
}

c_bool
d_readerRequestHasChains(
    d_readerRequest request)
{
    c_bool result;

    assert(d_readerRequestIsValid(request));

    if (request) {
        d_lockLock(d_lock(request));
        result = (d_tableSize(request->requests) != 0);
        d_lockUnlock(d_lock(request));
    } else {
        result = FALSE;
    }
    return result;
}

static c_bool
checkCompleteness(
    d_group group,
    c_voidp args)
{
    c_bool result;

    OS_UNUSED_ARG(args);
    assert(!args);

    if (d_groupGetCompleteness(group) == D_GROUP_COMPLETE) {
        result = TRUE;
    } else {
        result = FALSE;
    }
    return result;
}

c_bool
d_readerRequestAreGroupsComplete(
    d_readerRequest request)
{
    c_bool result;

    assert(d_readerRequestIsValid(request));

    if (request) {
        d_lockLock(d_lock(request));
        result = d_tableWalk(request->groups, checkCompleteness, NULL);
        d_lockUnlock(d_lock(request));
    } else {
        result = FALSE;
    }
    return result;
}

c_bool
d_readerRequestAddGroup(
    d_readerRequest request,
    d_group group)
{
    c_bool result;
    d_group found;

    assert(d_readerRequestIsValid(request));

    if (request) {
        d_lockLock(d_lock(request));
        found = d_tableInsert(request->groups, group);
        d_lockUnlock(d_lock(request));

        if (!found) {
            d_objectKeep(d_object(group));
            result = TRUE;
        } else {
            result = FALSE;
        }
    } else {
        result = FALSE;
    }
    return result;
}

void
d_readerRequestSetGroupIgnored(
    d_readerRequest request)
{
    assert(d_readerRequestIsValid(request));
    request->groupsIgnored = TRUE;
}

c_bool
d_readerRequestGetGroupIgnored(
    d_readerRequest request)
{
    assert(d_readerRequestIsValid(request));
    return request->groupsIgnored;
}

static c_bool
printGroup(
    d_group group,
    c_voidp args)
{
    c_char *partition, *topic;

    OS_UNUSED_ARG(args);
    assert(!args);
    partition = d_groupGetPartition(group);
    topic = d_groupGetTopic(group);

    printf("    %s.%s\n", partition, topic);

    os_free(partition);
    os_free(topic);

    return TRUE;
}

void
d_readerRequestPrint(
    d_readerRequest request)
{
    c_ulong i;

    assert(d_readerRequestIsValid(request));

    if (request) {
        printf(
                "- source:\n"\
                "    index                    : '%d'\n"\
                "    serial                   : '%d'\n"\
                "- filter:\n",
                request->readerHandle.index,
                request->readerHandle.serial);

        if (request->filter) {
            printf("    expression               : '%s'\n", request->filter);
        } else {
            printf("    expression               : NULL\n");
        }
        if (request->filterParamsCount > 0) {
            printf("    params                   :\n");
        } else {
            printf("    params                   : NULL\n");
        }

        for (i=0; i<request->filterParamsCount; i++) {
            printf("        [%d]                  : '%s'\n", i, request->filterParams[i]);
        }
        printf    ("- resourceLimits:\n");
        printf    ("    max_instances            : '%d'\n", request->resourceLimits.v.max_instances);
        printf    ("    max_samples              : '%d'\n", request->resourceLimits.v.max_samples);
        printf    ("    max_samples_per_instance : '%d'\n", request->resourceLimits.v.max_samples_per_instance);
        printf    ("- minSourceTimestamp:\n");
        printf    ("    seconds                  : '%d'\n", request->minSourceTimestamp.seconds);
        printf    ("    nanoseconds              : '%d'\n", C_TIME_NANOS(request->minSourceTimestamp.nanoseconds));
        printf    ("- maxSourceTimestamp:\n");
        printf    ("    seconds                  : '%d'\n", request->maxSourceTimestamp.seconds);
        printf    ("    nanoseconds              : '%d'\n", C_TIME_NANOS(request->maxSourceTimestamp.nanoseconds));
        printf    ("- groups involved:\n");

        d_tableWalk(request->groups, printGroup, NULL);
    }
}
