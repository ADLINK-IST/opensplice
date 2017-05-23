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
#ifndef DDS_DATAWRITER_H
#define DDS_DATAWRITER_H

#include "dds_dcps.h"
#include "sac_common.h"

DDS_ReturnCode_t
DDS_DataWriterNew (
    u_writer uWriter,
    const DDS_Publisher publisher,
    const DDS_Topic topic,
    DDS_DataWriter *writer);

DDS_ReturnCode_t
DDS_DataWriterFree (
    DDS_DataWriter _this);

DDS_ReturnCode_t
DDS_DataWriter_notify_listener (
    DDS_DataWriter _this,
    v_listenerEvent event);

DDS_ReturnCode_t
DDS_DataWriter_set_listener_mask (
    _DataWriter _this,
    const DDS_StatusMask mask);

#endif
