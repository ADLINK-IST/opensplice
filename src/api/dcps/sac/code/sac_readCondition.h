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
#ifndef DDS_READCONDITION_H
#define DDS_READCONDITION_H

#include "dds_dcps.h"
#include "sac_common.h"

DDS_ReadCondition
DDS_ReadConditionNew (
    DDS_Entity source,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states);

DDS_ReturnCode_t
_ReadCondition_deinit (
    _Object _this);

DDS_ReturnCode_t
DDS_ReadCondition_read (
    DDS_ReadCondition _this,
    DDS_Entity source,
    _DDS_sequence data_seq,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples);

DDS_ReturnCode_t
DDS_ReadCondition_take (
    DDS_ReadCondition _this,
    DDS_Entity source,
    _DDS_sequence data_seq,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples);

DDS_ReturnCode_t
DDS_ReadCondition_read_next_instance (
    DDS_ReadCondition _this,
    DDS_Entity source,
    const DDS_InstanceHandle_t a_handle,
    _DDS_sequence data_seq,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples);

DDS_ReturnCode_t
DDS_ReadCondition_take_next_instance (
    DDS_ReadCondition _this,
    DDS_Entity source,
    const DDS_InstanceHandle_t a_handle,
    _DDS_sequence data_seq,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples);

DDS_boolean
_ReadConditionGetTriggerValue(
    _Condition _this);

u_query
DDS_ReadCondition_get_uQuery (
    DDS_ReadCondition _this);

#endif
