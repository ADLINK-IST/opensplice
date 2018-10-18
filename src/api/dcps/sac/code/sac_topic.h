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
#ifndef DDS_TOPIC_H
#define DDS_TOPIC_H

#include "dds_dcps.h"
#include "sac_common.h"

DDS_Topic
DDS_TopicNew (
    DDS_DomainParticipant participant,
    const DDS_char *topic_name,
    const DDS_char *type_name,
    const DDS_TypeSupport type_support,
    const u_topic uTopic);

DDS_ReturnCode_t
DDS_Topic_notify_listener(
    DDS_Topic _this,
    v_listenerEvent event);

DDS_ReturnCode_t
DDS_Topic_set_participantListenerInterest(
    DDS_Topic _this,
    const DDS_StatusMask interest);

DDS_ReturnCode_t
DDS_Topic_validate_filter(
    DDS_Topic _this,
    const DDS_char *filter_expression,
    const DDS_StringSeq *filter_parameters);

DDS_ReturnCode_t
DDS_Topic_set_listener_mask (
    _Topic _this,
    const DDS_StatusMask mask);

#endif
