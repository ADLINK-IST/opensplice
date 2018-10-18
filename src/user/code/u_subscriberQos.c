/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
#include "u_subscriberQos.h"
#include "os_stdlib.h"

u_subscriberQos
u_subscriberQosNew(
    const u_subscriberQos _template)
{
    u_subscriberQos _this;

    assert(!_template || ((v_qos)_template)->kind == V_SUBSCRIBER_QOS);

    _this = os_malloc(sizeof(C_STRUCT(v_subscriberQos)));
    if (_template != NULL) {
        *_this = *_template;

        _this->groupData.v.value = NULL;
        _this->partition.v = NULL;
        _this->share.v.name = NULL;
        if (_template->groupData.v.size > 0) {
            assert(_template->groupData.v.value);
            _this->groupData.v.value = os_malloc((c_ulong)_template->groupData.v.size);
            _this->groupData.v.size = _template->groupData.v.size;
            memcpy(_this->groupData.v.value, _template->groupData.v.value, (c_ulong) _template->groupData.v.size);
        }
        if (_template->partition.v != NULL) {
            _this->partition.v = os_strdup(_template->partition.v);
        }
        if (_template->share.v.enable) {
            assert(_template->share.v.name);
            _this->share.v.name = os_strdup(_template->share.v.name);
        }
    } else {
        ((v_qos)_this)->kind                               = V_SUBSCRIBER_QOS;
        _this->groupData.v.value                           = NULL;
        _this->groupData.v.size                            = 0;
        _this->presentation.v.access_scope                 = V_PRESENTATION_INSTANCE;
        _this->presentation.v.coherent_access              = FALSE;
        _this->presentation.v.ordered_access               = FALSE;
        _this->partition.v                                 = NULL;
        _this->entityFactory.v.autoenable_created_entities = TRUE;
        _this->share.v.name                                = NULL;
        _this->share.v.enable                              = FALSE;
    }
    return _this;
}

void
u_subscriberQosFree(
    u_subscriberQos _this)
{
    assert(_this && ((v_qos)_this)->kind == V_SUBSCRIBER_QOS);
    os_free(_this->groupData.v.value);
    os_free(_this->partition.v);
    os_free(_this->share.v.name);
    os_free(_this);
}
