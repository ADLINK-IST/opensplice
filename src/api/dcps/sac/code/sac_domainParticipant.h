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
#ifndef SAC_DOMAINPARTICIPANT_H
#define SAC_DOMAINPARTICIPANT_H

#include "dds_dcps.h"
#include "sac_common.h"

DDS_DomainParticipant
DDS_DomainParticipantNew(
    const DDS_DomainParticipantFactory factory,
    const DDS_char *name,
    const DDS_DomainId_t domain_id,
    const DDS_DomainParticipantQos *qos);

DDS_ReturnCode_t
DDS_DomainParticipantFree (
    DDS_DomainParticipant _this);

DDS_boolean
DDS_DomainParticipant_has_contained_entities (
    DDS_DomainParticipant _this);

DDS_ReturnCode_t
DDS_DomainParticipant_find_type_locked (
    _DomainParticipant _this,
    const DDS_char *type_name,
    DDS_TypeSupport *type);

DDS_ReturnCode_t
DDS_DomainParticipant_find_type (
    DDS_DomainParticipant _this,
    const DDS_char *type_name,
    DDS_TypeSupport *type);

DDS_ReturnCode_t
DDS_DomainParticipant_register_type (
    DDS_DomainParticipant _this,
    const DDS_char *type_name,
    const DDS_TypeSupport type);

/* Add Entity to the participant's listener thread.
 * The participant will monitor the entity for events and
 * invoke the entity's listener callback operations upon retrieval
 * of events of interest.
 */
DDS_ReturnCode_t
DDS_DomainParticipant_add_observable (
    DDS_DomainParticipant _this,
    DDS_Entity observable);

/* Remove Entity from the participant's listener thread */
DDS_ReturnCode_t
DDS_DomainParticipant_remove_observable (
    DDS_DomainParticipant _this,
    DDS_Entity observable);

DDS_Topic
DDS_DomainParticipant_lookup_builtin_topic (
    DDS_DomainParticipant _this,
    const DDS_char *name);

DDS_ReturnCode_t
DDS_DomainParticipant_notify_listener(
    DDS_DomainParticipant _this,
    v_listenerEvent event);

DDS_ReturnCode_t
DDS_DomainParticipant_set_listener_mask (
    _DomainParticipant _this,
    const DDS_StatusMask mask);

#endif /* SAC_DOMAINPARTICIPANT_H */
