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

#include "d_qos.h"
#include "os_stdlib.h"

v_publisherQos
d_publisherQosNew(
    const c_char* partition)
{
    v_publisherQos qos = u_publisherQosNew(NULL);
    
    if (qos) {
        qos->presentation.access_scope = V_PRESENTATION_TOPIC;
        if (qos->partition) {
            os_free(qos->partition);
        }
        qos->partition = os_malloc(strlen(partition)+1);
        
        if (qos->partition == NULL) {
            d_publisherQosFree(qos);
            qos = NULL;
        } else {
            /* QAC EXPECT 5007; use of strcpy */
            os_strcpy(qos->partition, partition);
        }
    }
    return qos;
}

void
d_publisherQosFree(
    v_publisherQos qos)
{
    if (qos) {
        u_publisherQosFree(qos);
    }
}

v_subscriberQos
d_subscriberQosNew(
    const c_char* partition)
{
    v_subscriberQos qos = u_subscriberQosNew(NULL);
    if (qos) {
        qos->entityFactory.autoenable_created_entities = FALSE;
        if (qos->partition) {
            os_free(qos->partition);
        }
        if(partition){
            qos->partition = os_malloc(strlen(partition)+1);
            if (qos->partition == NULL) {
                d_subscriberQosFree(qos);
                qos = NULL;
            } else {
                /* QAC EXPECT 5007; use of strcpy */
                os_strcpy(qos->partition, partition);
            }
        } else {
            qos->partition = NULL;
        }
    }
    return qos;
}

void
d_subscriberQosFree(
    v_subscriberQos qos)
{
    if (qos) {
        u_subscriberQosFree(qos);
    }
}

v_readerQos
d_readerQosNew(
    v_durabilityKind durability,
    v_reliabilityKind reliability)
{
    v_readerQos qos = u_readerQosNew(NULL);
    if (qos) {
        qos->durability.kind  = durability;
        qos->reliability.kind = reliability;
        qos->latency.duration = C_TIME_INFINITE;
    }
    return qos;
}

void
d_readerQosFree(
    v_readerQos qos)
{
    if (qos) {
        u_readerQosFree(qos);
    }
}

v_writerQos
d_writerQosNew(
    v_durabilityKind durability,
    v_reliabilityKind reliability,
    v_duration latencyBudget,
    c_long transportPriority)
{
    v_writerQos qos = u_writerQosNew(NULL);
    
    if (qos) {
        qos->durability.kind                            = durability;
        qos->reliability.kind                           = reliability;
        qos->reliability.max_blocking_time.seconds      = 1;
        qos->reliability.max_blocking_time.nanoseconds  = 0;
        qos->latency.duration.seconds                   = latencyBudget.seconds;
        qos->latency.duration.nanoseconds               = latencyBudget.nanoseconds;
        qos->transport.value                            = transportPriority;
        qos->history.kind                               = V_HISTORY_KEEPALL;
        qos->resource.max_samples                       = 1;
    }
    return qos;
}

void
d_writerQosFree(
    v_writerQos qos)
{
    if (qos) {
        u_writerQosFree(qos);
    }
}

v_topicQos
d_topicQosNew(
    v_durabilityKind durability,
    v_reliabilityKind reliability)
{
    v_topicQos qos = u_topicQosNew(NULL);
    if (qos) {
        qos->durability.kind  = durability;
        qos->reliability.kind = reliability;
    }
    return qos;
}

void
d_topicQosFree(
    v_topicQos qos)
{
    if (qos) {
        u_topicQosFree(qos);
    }
}
