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
#ifndef DDS_DATAREADERVIEW_H
#define DDS_DATAREADERVIEW_H

#include "dds_dcps.h"
#include "sac_common.h"

DDS_DataReaderView
DDS_DataReaderViewNew (
    const DDS_DataReader datareader,
    const DDS_char *name,
    const DDS_DataReaderViewQos *qos,
    const DDS_TopicDescription topicdescription);

DDS_ReturnCode_t
DDS_DataReaderViewFree (
    DDS_DataReaderView _this);

DDS_ReturnCode_t
DDS_DataReaderView_loan_buffers(
    _DataReaderView _this,
    DDS_sequence *data_seq,
    DDS_SampleInfoSeq *info_seq,
    DDS_long length);

int
DDS_DataReaderView_read_next_instance_internal (
    DDS_DataReaderView _this,
    const DDS_InstanceHandle_t a_handle,
    u_sampleMask mask,
    cmn_samplesList samplesList);

int
DDS_DataReaderView_take_next_instance_internal (
    DDS_DataReaderView _this,
    const DDS_InstanceHandle_t a_handle,
    u_sampleMask mask,
    cmn_samplesList samplesList);

#endif
