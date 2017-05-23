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
#include "u_dataViewQos.h"
#include "os_stdlib.h"

u_dataViewQos
u_dataViewQosNew(
    const u_dataViewQos _template)
{
    u_dataViewQos _this;

    assert(!_template || ((v_qos)_template)->kind == V_DATAVIEW_QOS);

    _this = os_malloc(sizeof(C_STRUCT(v_dataViewQos)));
    if (_template != NULL) {
        *_this = *_template;

        _this->userKey.v.expression = NULL;
        if (_template->userKey.v.enable) {
            if (_template->userKey.v.expression != NULL) {
                _this->userKey.v.expression = os_strdup (_template->userKey.v.expression);
            }
        }
    } else {
        ((v_qos)_this)->kind      = V_DATAVIEW_QOS;
        _this->userKey.v.enable     = FALSE;
        _this->userKey.v.expression = NULL;
    }
    return _this;
}

void
u_dataViewQosFree(
    u_dataViewQos _this)
{
    assert(_this && ((v_qos)_this)->kind == V_DATAVIEW_QOS);
    os_free(_this->userKey.v.expression);
    os_free(_this);
}
