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
#ifndef DDS_SUBSCRIBER_H
#define DDS_SUBSCRIBER_H

#include "dds_dcps.h"
#include "sac_common.h"

DDS_Subscriber
DDS_SubscriberNew (
    DDS_DomainParticipant participant,
    const DDS_char *name,
    const DDS_SubscriberQos *qos);

DDS_ReturnCode_t
DDS_SubscriberFree (
    DDS_Subscriber _this);

DDS_boolean
DDS_Subscriber_contains_entity (
    DDS_Subscriber _this,
    DDS_InstanceHandle_t  a_handle);

DDS_ReturnCode_t
DDS_SubscriberQos_from_mapping (
    const v_subscriberQos mapping,
    DDS_SubscriberQos *qos);

DDS_ReturnCode_t
DDS_Subscriber_copy_from_topicdescription (
    DDS_Subscriber _this,
    DDS_DataReaderQos *qos,
    const DDS_TopicDescription description);

DDS_ReturnCode_t
DDS_Subscriber_notify_listener (
    DDS_Subscriber _this,
    v_listenerEvent event);

DDS_ReturnCode_t
DDS_Subscriber_set_listener_mask (
    _Subscriber _this,
    const DDS_StatusMask mask);

#endif
