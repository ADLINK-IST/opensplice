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

#include "v_partition.h"
#include "v__partition.h"
#include "v__entity.h"
#include "v_public.h"
#include "v_handle.h"
#include "v_participant.h"
#include "v_publisher.h"
#include "v_subscriber.h"
#include "v_configuration.h"
#include "v__policy.h"
#include "os_heap.h"
#include "os_report.h"

/* Protected */

c_bool
v_partitionExpressionIsAbsolute(
    const c_char* expression)
{
    c_bool result;

    assert(expression);

    /* Absolute expressions are those which do not contain a '?' or a '*' */
    if(strchr(expression, '*') == NULL && strchr(expression, '?') == NULL)
    {
        result = TRUE;
    } else
    {
        result = FALSE;
    }

    return result;
}

c_bool
v_partitionStringMatchesExpression(
    const c_char* string,
    const c_char* expression)
{
    c_bool result = FALSE;
    c_value matchResult;
    c_value expressionValue;
    c_value stringValue;

    assert(string);
    assert(expression);

    if(v_partitionExpressionIsAbsolute(expression))
    {
        if(strcmp(expression, string) == 0)
        {
             result  = TRUE;
        }
    } else
    {
        expressionValue.kind = V_STRING;
        expressionValue.is.String = (c_char*)expression;
        stringValue.kind = V_STRING;
        stringValue.is.String = (c_char*)string;
        matchResult = c_valueStringMatch(expressionValue, stringValue);
        if(matchResult.is.Boolean == TRUE)
        {
            result = TRUE;
        }
    }
    return result;
}

/* Public */

v_partition
v_partitionNew(
    v_kernel kernel,
    const c_char *name,
    v_partitionQos qos)
{
    v_partition partition, found;

    OS_UNUSED_ARG(qos);
    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));

    assert(name != NULL);
    assert(v_partitionExpressionIsAbsolute(name));

    partition = v_partition(v_objectNew(kernel,K_DOMAIN));
    v_entityInit(v_entity(partition),name, NULL, TRUE);

    found = v_addPartition(kernel,partition);

    if (found != partition) {
        v_partitionFree(partition);
        c_free(partition); /* v_partitionFree has removed all dependancies, now delete local reference */
        partition = c_keep(found); /* this one will be returned, so a keep is required */
    }
    return partition;
}

void
v_partitionFree(
    v_partition partition)
{
    assert(C_TYPECHECK(partition,v_partition));

    v_publicFree(v_public(partition));
}

void
v_partitionDeinit(
    v_partition partition)
{
    assert(C_TYPECHECK(partition,v_partition));

    v_entityDeinit(v_entity(partition));
}

c_iter
v_partitionLookupPublishers(
    v_partition partition)
{
    c_iter participants;
    c_iter result;
    c_iter entities;
    c_iter partitions;
    v_participant participant;
    v_entity entity;
    v_entity partition2;

    result = NULL;
    participants = v_resolveParticipants(v_objectKernel(partition), "*");
    participant = v_participant(c_iterTakeFirst(participants));

    while (participant != NULL) {
        c_lockRead(&participant->lock);
        entities = ospl_c_select(participant->entities, 0);
        c_lockUnlock(&participant->lock);
        entity = v_entity(c_iterTakeFirst(entities));

        while (entity != NULL) {
            if (v_objectKind(entity) == K_PUBLISHER) {
                partitions = v_publisherLookupPartitions(v_publisher(entity),
                                                   v_partitionName(partition));

                if (c_iterLength(partitions) > 0) {
                    result = c_iterInsert(result, entity); /* transfer refcount */
                } else {
                    c_free(entity);
                }
                partition2 = v_entity(c_iterTakeFirst(partitions));

                while (partition2 != NULL) {
                    c_free(partition2);
                    partition2 = v_entity(c_iterTakeFirst(partitions));
                }
                c_iterFree(partitions);
            }
            /* entity is already free or refcount transferred to result */
            entity = v_entity(c_iterTakeFirst(entities));
        }
        c_iterFree(entities);
        c_free(participant);
        participant = v_participant(c_iterTakeFirst(participants));
    }
    c_iterFree(participants);
    return result;
}

c_iter
v_partitionLookupSubscribers(
    v_partition partition)
{
    c_iter participants;
    c_iter result;
    c_iter entities;
    c_iter partitions;
    v_participant participant;
    v_entity entity;
    v_entity partition2;

    result = NULL;
    participants = v_resolveParticipants(v_objectKernel(partition), "*");
    participant = v_participant(c_iterTakeFirst(participants));

    while (participant != NULL) {
        c_lockRead(&participant->lock);
        entities = ospl_c_select(participant->entities, 0);
        c_lockUnlock(&participant->lock);
        entity = v_entity(c_iterTakeFirst(entities));

        while (entity != NULL) {
            if(v_objectKind(entity) == K_SUBSCRIBER) {
                partitions = v_subscriberLookupPartitions(v_subscriber(entity),
                                                    v_partitionName(partition));

                if (c_iterLength(partitions) > 0) {
                    result = c_iterInsert(result, entity); /* transfer refcount */
                } else {
                    c_free(entity);
                }
                partition2 = v_entity(c_iterTakeFirst(partitions));

                while (partition2 != NULL) {
                    c_free(partition2);
                    partition2 = v_entity(c_iterTakeFirst(partitions));
                }
                c_iterFree(partitions);
            }
            /* entity is already free or refcount transferred to result */
            entity = v_entity(c_iterTakeFirst(entities));
        }
        c_iterFree(entities);
        c_free(participant);
        participant = v_participant(c_iterTakeFirst(participants));
    }
    c_iterFree(participants);
    return result;
}


v_partitionInterest
v_partitionInterestNew(
    v_kernel kernel,
    const char *partitionExpression)
{
    v_partitionInterest result = NULL;


    result = c_new(v_kernelType(kernel, K_DOMAININTEREST));
    if (result) {
        result->expression = c_stringNew(c_getBase(c_object(kernel)), partitionExpression);
    } else {
        OS_REPORT(OS_ERROR,
                  "v_partitionInterestNew",0,
                  "Failed to allocate partition interest.");
        assert(FALSE);
    }

    return result;
}
