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

#include "c_base.h"
#include "v_topicQos.h"
#include "d__pQos.h"

v_topicQos d_topicQosFromPQos (c_base base, const struct d_persistentTopicQosV0_s *pqos)
{
    struct v_topicInfo info;
    info.durability = pqos->durability;
    info.durabilityService = pqos->durabilityService;
    info.deadline = pqos->deadline;
    info.latency_budget = pqos->latency;
    info.liveliness = pqos->liveliness;
    info.reliability = pqos->reliability;
    info.transport_priority = pqos->transport;
    info.lifespan = pqos->lifespan;
    info.destination_order = pqos->orderby;
    info.history = pqos->history;
    info.resource_limits = pqos->resource;
    info.ownership = pqos->ownership;
    info.topic_data.value = pqos->topicData.value;
    return v_topicQosFromTopicInfo (base, &info);
}

d_persistentTopicQosV0 d_pQosFromTopicQos (const struct v_topicQos_s *qos)
{
    c_base base = c_getBase ((c_object) qos);
    c_type pqosType;
    struct v_topicInfo info;
    d_persistentTopicQosV0 pqos;

    pqosType = c_resolve (base, "durabilityModule2::d_persistentTopicQosV0");
    pqos = c_new (pqosType);
    c_free (pqosType);
    if (pqos == NULL) {
        return NULL;
    }

    if (v_topicQosFillTopicInfo (&info, qos) != V_RESULT_OK) {
        c_free (pqos);
        return NULL;
    }

    pqos->_parent.kind = V_TOPIC_QOS;
    pqos->durability = info.durability;
    pqos->durabilityService = info.durabilityService;
    pqos->deadline = info.deadline;
    pqos->latency = info.latency_budget;
    pqos->liveliness = info.liveliness;
    pqos->reliability = info.reliability;
    pqos->transport = info.transport_priority;
    pqos->lifespan = info.lifespan;
    pqos->orderby = info.destination_order;
    pqos->history = info.history;
    pqos->resource = info.resource_limits;
    pqos->ownership = info.ownership;
    pqos->topicData.value = info.topic_data.value; /* transfer refcount */
    pqos->topicData.size = (c_long) c_arraySize (info.topic_data.value);

    return pqos;
}
