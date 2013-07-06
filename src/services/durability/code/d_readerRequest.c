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
#include "d__readerRequest.h"
#include "d_table.h"
#include "d_group.h"
#include "d_sampleChainListener.h"
#include "v_dataReader.h"
#include "v_subscriber.h"
#include "v_topic.h"
#include "v_partition.h"
#include "v_kernel.h"
#include "v_handle.h"
#include "os_heap.h"
#include "os_stdlib.h"

d_readerRequest
d_readerRequestProxyNew(
    v_handle source)
{
    d_readerRequest request;

    request = d_readerRequest(os_malloc(C_SIZEOF(d_readerRequest)));

    if(request){
        d_lockInit(d_lock(request), D_READER_REQUEST, d_readerRequestDeinit);

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
    v_handle source,
    c_char* filter,
    c_char** filterParams,
    c_long filterParamsCount,
    struct v_resourcePolicy resourceLimits,
    c_time minSourceTimestamp,
    c_time maxSourceTimestamp)
{
    d_readerRequest request;
    c_long i;
    v_handleResult handleResult;
    v_reader vreader, *vreaderPtr;
    c_iter partitions;
    v_partition partition;
    v_topic topic;
    d_group dgroup;
    c_char *topicName;
    d_quality quality;

    request = d_readerRequest(os_malloc(C_SIZEOF(d_readerRequest)));

    if(request){
        d_lockInit(d_lock(request), D_READER_REQUEST, d_readerRequestDeinit);

        request->admin               = admin;
        request->readerHandle.index  = source.index;
        request->readerHandle.serial = source.serial;
        request->readerHandle.server = source.server;
        request->requests            = d_tableNew(d_chainCompare, d_chainFree);

        if(filter){
            request->filter          = os_strdup(filter);
        } else {
            request->filter          = NULL;
        }
        request->resourceLimits      = resourceLimits;
        request->minSourceTimestamp  = minSourceTimestamp;
        request->maxSourceTimestamp  = maxSourceTimestamp;
        request->filterParamsCount   = filterParamsCount;

        if(filterParamsCount > 0){
            request->filterParams = (c_char**)(os_malloc(
                                filterParamsCount*sizeof(c_char*)));

            for(i=0; i<filterParamsCount; i++){
                request->filterParams[i] = os_strdup(filterParams[i]);
            }
        } else {
            request->filterParams = NULL;
        }
        request->groups = d_tableNew(d_groupCompare, d_groupFree);

        handleResult = v_handleClaim(source, (v_object*)(vreaderPtr = &vreader));

        if(handleResult == V_HANDLE_OK){
            if(v_object(vreader)->kind == K_DATAREADER){
                topic      = v_dataReaderGetTopic(v_dataReader(vreader));
                topicName  = v_entity(topic)->name;
                partitions = ospl_c_select(v_subscriber(vreader->subscriber)->partitions->partitions, 0);
                partition  = v_partition(c_iterTakeFirst(partitions));

                while(partition){
                    quality.seconds = 0;
                    quality.nanoseconds = 0;
                    dgroup = d_groupNew(
                                v_entity(partition)->name, topicName,
                                topic->qos->durability.kind,
                                D_GROUP_KNOWLEDGE_UNDEFINED,
                                quality);
                    d_tableInsert(request->groups, dgroup);
                    c_free(partition);
                    partition  = v_partition(c_iterTakeFirst(partitions));
                }
                c_free(topic);
            } else {
                d_readerRequestFree(request);
                request = NULL;
            }
            v_handleRelease(source);
        } else {
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

    assert(d_objectIsValid(d_object(request), D_READER_REQUEST) == TRUE);

    if(request){
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

    assert(d_objectIsValid(d_object(request), D_READER_REQUEST) == TRUE);

    if(request){
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

    assert(d_objectIsValid(d_object(request), D_READER_REQUEST) == TRUE);

    if(request){
        d_lockLock(d_lock(request));
        dgroup = d_tableRemove(request->groups, group);
        d_lockUnlock(d_lock(request));

        if(dgroup){
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

    if(request1 && request2){
        result = request1->readerHandle.index - request2->readerHandle.index;

        if(result == 0){
            result = request1->readerHandle.serial - request2->readerHandle.serial;
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
    d_object object)
{
    d_readerRequest request;
    c_ulong i;

    assert(d_objectIsValid(object, D_READER_REQUEST) == TRUE);

    if(object){
        request = d_readerRequest(object);

        if(request->requests){
            d_tableFree(request->requests);
            request->requests = NULL;
        }
        if(request->filter){
            os_free(request->filter);
            request->filter = NULL;
        }
        for(i=0; i<request->filterParamsCount; i++){
            os_free(request->filterParams[i]);
        }
        if(request->filterParams){
            os_free(request->filterParams);
            request->filterParams = NULL;
        }
        if(request->groups){
            d_tableFree(request->groups);
            request->groups = NULL;
        }
    }
    return;
}

void
d_readerRequestFree(
    d_readerRequest request)
{
    assert(d_objectIsValid(d_object(request), D_READER_REQUEST) == TRUE);

    if(request){
        d_lockFree(d_lock(request), D_READER_REQUEST);
    }
    return;
}

c_bool
d_readerRequestAddChain(
    d_readerRequest request,
    d_chain chain)
{
    c_bool result;
    d_chain found;

    assert(d_objectIsValid(d_object(request), D_READER_REQUEST) == TRUE);
    assert(d_objectIsValid(d_object(chain), D_CHAIN) == TRUE);

    if(request && chain){
        d_lockLock(d_lock(request));
        found = d_tableInsert(request->requests, chain);
        d_lockUnlock(d_lock(request));

        if(found){
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

    assert(d_objectIsValid(d_object(request), D_READER_REQUEST) == TRUE);
    assert(d_objectIsValid(d_object(chain), D_CHAIN) == TRUE);

    if(request && chain){
        d_lockLock(d_lock(request));
        found = d_tableRemove(request->requests, chain);
        d_lockUnlock(d_lock(request));

        if(found){
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

    assert(d_objectIsValid(d_object(request), D_READER_REQUEST) == TRUE);

    if(request){
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

    if(d_groupGetCompleteness(group) == D_GROUP_COMPLETE){
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

    assert(d_objectIsValid(d_object(request), D_READER_REQUEST) == TRUE);

    if(request){
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

    assert(d_objectIsValid(d_object(request), D_READER_REQUEST) == TRUE);

    if(request){
        d_lockLock(d_lock(request));
        found = d_tableInsert(request->groups, group);
        d_lockUnlock(d_lock(request));

        if(!found){
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

    assert(d_objectIsValid(d_object(request), D_READER_REQUEST) == TRUE);

    if(request){
        printf(
                "- source:\n"\
                "    index                    : '%d'\n"\
                "    serial                   : '%d'\n"\
                "- filter:\n",
                request->readerHandle.index,
                request->readerHandle.serial);

        if(request->filter){
            printf("    expression               : '%s'\n", request->filter);
        } else {
            printf("    expression               : NULL\n");
        }
        if(request->filterParamsCount > 0){
            printf("    params                   :\n");
        } else {
            printf("    params                   : NULL\n");
        }

        for(i=0; i<request->filterParamsCount; i++){
            printf("        [%d]                  : '%s'\n", i, request->filterParams[i]);
        }
        printf    ("- resourceLimits:\n");
        printf    ("    max_instances            : '%d'\n", request->resourceLimits.max_instances);
        printf    ("    max_samples              : '%d'\n", request->resourceLimits.max_samples);
        printf    ("    max_samples_per_instance : '%d'\n", request->resourceLimits.max_samples_per_instance);
        printf    ("- minSourceTimestamp:\n");
        printf    ("    seconds                  : '%d'\n", request->minSourceTimestamp.seconds);
        printf    ("    nanoseconds              : '%d'\n", request->minSourceTimestamp.nanoseconds);
        printf    ("- maxSourceTimestamp:\n");
        printf    ("    seconds                  : '%d'\n", request->maxSourceTimestamp.seconds);
        printf    ("    nanoseconds              : '%d'\n", request->maxSourceTimestamp.nanoseconds);
        printf    ("- groups involved:\n");

        d_tableWalk(request->groups, printGroup, NULL);
    }
}
