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
#ifndef SAC_TOPICDESCRIPTION_H
#define SAC_TOPICDESCRIPTION_H

#include "dds_dcps.h"
#include "sac_common.h"
#include "u_topic.h"

DDS_ReturnCode_t
DDS_TopicDescription_init (
    DDS_TopicDescription _this,
    const DDS_DomainParticipant participant,
    const DDS_char *topic_name,
    const DDS_char *type_name,
    const DDS_char *expression,
    const DDS_TypeSupport typeSupport,
    const u_topic uTopic);

DDS_ReturnCode_t
DDS_TopicDescription_deinit (
    DDS_TopicDescription _this);

DDS_TopicDescription
DDS_TopicDescription_keep(
    DDS_TopicDescription _this);

DDS_ReturnCode_t
DDS_TopicDescription_free (
    DDS_TopicDescription _this);

os_char *
DDS_TopicDescription_get_expr (
    DDS_TopicDescription _this);

DDS_ReturnCode_t
DDS_TopicDescription_get_typeSupport_locked_dp (
    DDS_TopicDescription _this,
    DDS_TypeSupport *typeSupport);

DDS_ReturnCode_t
DDS_TopicDescription_get_typeSupport (
    DDS_TopicDescription _this,
    DDS_TypeSupport *typeSupport);

DDS_ReturnCode_t
DDS_TopicDescription_get_qos (
    DDS_TopicDescription _this,
    DDS_TopicQos *qos);

#endif /* SAC_TOPICDESCRIPTION_H */
