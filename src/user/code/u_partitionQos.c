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

#include "u_partitionQos.h"

u_partitionQos
u_partitionQosNew(
    const u_partitionQos _template)
{
    u_partitionQos _this;

    assert(!_template || ((v_qos)_template)->kind == V_PARTITION_QOS);

    _this = os_malloc(sizeof(C_STRUCT(v_partitionQos)));
    if (_template != NULL) {
        *_this = *_template;
        _this->userData.v.value = NULL;
        if (_template->userData.v.size > 0) {
            assert(((v_qos)_template)->kind == V_PARTITION_QOS);
            _this->userData.v.value = os_malloc((c_ulong) _template->userData.v.size);
            _this->userData.v.size = _template->userData.v.size;
            memcpy(_this->userData.v.value, _template->userData.v.value, (c_ulong) _template->userData.v.size);
        }
    } else {
        ((v_qos)_this)->kind  = V_PARTITION_QOS;
        _this->userData.v.size  = 0;
        _this->userData.v.value = NULL;
    }
    return _this;
}

void
u_partitionQosFree(
    const u_partitionQos _this)
{
    assert(_this && ((v_qos)_this)->kind == V_PARTITION_QOS);
    os_free(_this->userData.v.value);
    os_free(_this);
}
