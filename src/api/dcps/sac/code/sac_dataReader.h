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
#ifndef DDS_DATAREADER_H
#define DDS_DATAREADER_H

#include "dds_dcps.h"
#include "sac_common.h"

DDS_DataReader
DDS_DataReaderNew (
    DDS_Subscriber subscriber,
    const DDS_char *name,
    const DDS_DataReaderQos *qos,
    const DDS_TopicDescription tdesc);

DDS_ReturnCode_t
DDS_DataReaderFree (
    DDS_DataReader _this);

DDS_boolean
DDS_DataReader_contains_entity (
    DDS_DataReader _this,
    DDS_InstanceHandle_t a_handle);

DDS_ReturnCode_t
DDS_DataReader_read_instance_action (
    DDS_DataReader _this,
    const DDS_InstanceHandle_t a_handle,
    v_actionResult (*action)(void *, void *),
    void *arg);

DDS_ReturnCode_t
DDS_DataReader_notify_listener (
    DDS_DataReader _this,
    v_listenerEvent event);

DDS_ReturnCode_t
DDS_DataReader_read_next_instance_internal (
    DDS_DataReader _this,
    const DDS_InstanceHandle_t a_handle,
    u_sampleMask mask,
    cmn_samplesList samplesList);

DDS_ReturnCode_t
DDS_DataReader_take_next_instance_internal (
    DDS_DataReader _this,
    const DDS_InstanceHandle_t a_handle,
    u_sampleMask mask,
    cmn_samplesList samplesList);

DDS_ReturnCode_t
DDS_DataReader_set_listener_mask (
    _DataReader _this,
    const DDS_StatusMask mask);

#endif
