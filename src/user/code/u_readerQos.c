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
#include "u_readerQos.h"
#include "os_stdlib.h"

u_readerQos
u_readerQosNew(
    const u_readerQos _template)
{
    u_readerQos _this;

    assert(!_template || ((v_qos)_template)->kind == V_READER_QOS);

    _this = os_malloc(sizeof(C_STRUCT(v_readerQos)));
    if (_template != NULL) {
        *_this = *_template;

        _this->userData.v.value = NULL;
        _this->share.v.name = NULL;
        _this->userKey.v.expression = NULL;
        if (_template->userData.v.size > 0) {
            assert(_template->userData.v.value);
            _this->userData.v.value = os_malloc((c_ulong)_template->userData.v.size);
            _this->userData.v.size = _template->userData.v.size;
            memcpy(_this->userData.v.value, _template->userData.v.value, (c_ulong) _template->userData.v.size);
        }
        if (_template->share.v.enable) {
            assert(_template->share.v.name);
            _this->share.v.name = os_strdup(_template->share.v.name);
            _this->share.v.enable = TRUE;
        }
        if (_template->userKey.v.enable) {
            assert(_template->userKey.v.expression);
            _this->userKey.v.expression = os_strdup(_template->userKey.v.expression);
            _this->userKey.v.enable = TRUE;
        }
    } else {
        ((v_qos)_this)->kind                                = V_READER_QOS;
        _this->durability.v.kind                            = V_DURABILITY_VOLATILE;
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
        _this->userData.v.size                              = 0;
        _this->userData.v.value                             = NULL;
        _this->ownership.v.kind                             = V_OWNERSHIP_SHARED;
        _this->pacing.v.minSeperation                       = OS_DURATION_ZERO;
        _this->lifecycle.v.autopurge_nowriter_samples_delay = OS_DURATION_INFINITE;
        _this->lifecycle.v.autopurge_disposed_samples_delay = OS_DURATION_INFINITE;
        _this->lifecycle.v.autopurge_dispose_all            = FALSE;
        _this->lifecycle.v.enable_invalid_samples           = TRUE;
        _this->lifecycle.v.invalid_sample_visibility        = V_VISIBILITY_MINIMUM_INVALID_SAMPLES;
        _this->lifespan.v.used                              = FALSE;
        _this->lifespan.v.duration                          = OS_DURATION_INFINITE;
        _this->share.v.enable                               = FALSE;
        _this->share.v.name                                 = NULL;
        _this->userKey.v.enable                             = FALSE;
        _this->userKey.v.expression                         = NULL;
    }
    return _this;
}

void
u_readerQosFree(
    u_readerQos _this)
{
    assert (_this && ((v_qos)_this)->kind == V_READER_QOS);
    os_free(_this->userData.v.value);
    os_free(_this->share.v.name);
    os_free(_this->userKey.v.expression);
    os_free(_this);
}
