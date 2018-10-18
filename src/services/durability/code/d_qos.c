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

#include "d_qos.h"
#include "os_stdlib.h"

v_publisherQos
d_publisherQosNew(
    const c_char* partition)
{
    v_publisherQos qos = u_publisherQosNew(NULL);

    if (qos) {
        qos->presentation.v.access_scope = V_PRESENTATION_TOPIC;
        if (qos->partition.v) {
            os_free(qos->partition.v);
        }
        qos->partition.v = os_malloc(strlen(partition)+1);

        if (qos->partition.v == NULL) {
            d_publisherQosFree(qos);
            qos = NULL;
        } else {
            /* QAC EXPECT 5007; use of strcpy */
            os_strcpy(qos->partition.v, partition);
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
        qos->entityFactory.v.autoenable_created_entities = FALSE;
        if (qos->partition.v) {
            os_free(qos->partition.v);
        }
        if(partition){
            qos->partition.v = os_malloc(strlen(partition)+1);
            /* QAC EXPECT 5007; use of strcpy */
            os_strcpy(qos->partition.v, partition);
        } else {
            qos->partition.v = NULL;
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
        qos->durability.v.kind  = durability;
        qos->reliability.v.kind = reliability;
        qos->latency.v.duration = OS_DURATION_INFINITE;
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
    v_orderbyKind orderKind,
    os_duration latencyBudget,
    c_long transportPriority)
{
    v_writerQos qos = u_writerQosNew(NULL);

    if (qos) {
        qos->durability.v.kind                            = durability;
        qos->reliability.v.kind                           = reliability;
        qos->reliability.v.max_blocking_time              = OS_DURATION_INIT(1,0);
        qos->latency.v.duration                           = latencyBudget;
        qos->transport.v.value                            = transportPriority;
        qos->history.v.kind                               = V_HISTORY_KEEPALL;
        qos->orderby.v.kind                               = orderKind;
        qos->resource.v.max_samples                       = 1;
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
        qos->durability.v.kind  = durability;
        qos->reliability.v.kind = reliability;
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
