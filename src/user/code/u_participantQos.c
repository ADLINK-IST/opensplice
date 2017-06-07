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

#include "u_participantQos.h"
#include "v_scheduler.h"

u_participantQos
u_participantQosNew(
    const u_participantQos _template)
{
    u_participantQos _this;

    assert(!_template || ((v_qos)_template)->kind == V_PARTICIPANT_QOS);

    _this = os_malloc(sizeof(C_STRUCT(v_participantQos)));
    if (_template != NULL) {
        *_this = *_template;
        _this->userData.v.value = NULL;
        if (_template->userData.v.size > 0) {
            assert(_template->userData.v.value != NULL);
            _this->userData.v.value = os_malloc((c_ulong) _template->userData.v.size);
            _this->userData.v.size = _template->userData.v.size;
            memcpy(_this->userData.v.value, _template->userData.v.value, (c_ulong) _template->userData.v.size);
        }
    } else {
        ((v_qos)_this)->kind                               = V_PARTICIPANT_QOS;
        _this->userData.v.value                            = NULL;
        _this->userData.v.size                             = 0;
        _this->entityFactory.v.autoenable_created_entities = TRUE;
        _this->watchdogScheduling.v.kind                   = V_SCHED_DEFAULT;
        _this->watchdogScheduling.v.priorityKind           = V_SCHED_PRIO_RELATIVE;
        _this->watchdogScheduling.v.priority               = 0;
    }

    return _this;
}

void
u_participantQosFree(
    const u_participantQos _this)
{
    assert(_this && ((v_qos)_this)->kind == V_PARTICIPANT_QOS);
    os_free(_this->userData.v.value);
    os_free(_this);
}
