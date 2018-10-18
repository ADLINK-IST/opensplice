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
#ifndef DDS_PUBLISHER_H
#define DDS_PUBLISHER_H

#include "dds_dcps.h"
#include "sac_common.h"

DDS_Publisher
DDS_PublisherNew (
    DDS_DomainParticipant participant,
    const DDS_char *name,
    const DDS_PublisherQos *qos);

DDS_ReturnCode_t
DDS_PublisherFree (
    DDS_Publisher _this);

DDS_boolean
DDS_Publisher_contains_entity (
    DDS_Publisher _this,
    DDS_InstanceHandle_t  a_handle);

DDS_ReturnCode_t
DDS_Publisher_notify_listener(
    DDS_Publisher _this,
    v_listenerEvent event);

DDS_ReturnCode_t
DDS_Publisher_set_listener_mask (
    _Publisher _this,
    const DDS_StatusMask mask);

#endif
