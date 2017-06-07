/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "u_topicQos.h"

u_topicQos
u_topicQosNew(
    u_topicQos _template)
{
    u_topicQos _this;

    assert(!_template || ((v_qos)_template)->kind == V_TOPIC_QOS);

    _this = os_malloc(sizeof(C_STRUCT(v_topicQos)));
    if (_template != NULL) {
        *_this = *_template;

        _this->topicData.v.value = NULL;
        if (_template->topicData.v.size > 0) {
            assert(_template->topicData.v.value);
            _this->topicData.v.value = os_malloc((c_ulong) _template->topicData.v.size);
            _this->topicData.v.size = _template->topicData.v.size;
            memcpy(_this->topicData.v.value, _template->topicData.v.value, (c_ulong) _template->topicData.v.size);
        }
    } else {
        ((v_qos)_this)->kind                                = V_TOPIC_QOS;
        _this->topicData.v.size                             = 0;
        _this->topicData.v.value                            = NULL;
        _this->durability.v.kind                            = V_DURABILITY_VOLATILE;
        _this->durabilityService.v.history_kind             = V_HISTORY_KEEPLAST;
        _this->durabilityService.v.history_depth            = 1;
        _this->durabilityService.v.max_samples              = V_LENGTH_UNLIMITED;
        _this->durabilityService.v.max_instances            = V_LENGTH_UNLIMITED;
        _this->durabilityService.v.max_samples_per_instance = V_LENGTH_UNLIMITED;
        _this->durabilityService.v.service_cleanup_delay    = OS_DURATION_ZERO;
        _this->deadline.v.period                            = OS_DURATION_INFINITE;
        _this->latency.v.duration                           = OS_DURATION_ZERO;
        _this->liveliness.v.kind                            = V_LIVELINESS_AUTOMATIC;
        _this->liveliness.v.lease_duration                  = OS_DURATION_ZERO;
        _this->reliability.v.kind                           = V_RELIABILITY_BESTEFFORT;
        _this->reliability.v.max_blocking_time              = OS_DURATION_ZERO;
        _this->reliability.v.synchronous                    = FALSE;
        _this->orderby.v.kind                               = V_ORDERBY_RECEPTIONTIME;
        _this->history.v.kind                               = V_HISTORY_KEEPLAST;
        _this->history.v.depth                              = 1;
        _this->resource.v.max_samples                       = V_LENGTH_UNLIMITED;
        _this->resource.v.max_instances                     = V_LENGTH_UNLIMITED;
        _this->resource.v.max_samples_per_instance          = V_LENGTH_UNLIMITED;
        _this->transport.v.value                            = 0;
        _this->lifespan.v.duration                          = OS_DURATION_INFINITE;
        _this->ownership.v.kind                             = V_OWNERSHIP_SHARED;
    }
    return _this;
}

void
u_topicQosFree(
    u_topicQos _this)
{
    assert(_this && ((v_qos)_this)->kind == V_TOPIC_QOS);
    os_free(_this->topicData.v.value);
    os_free(_this);
}
