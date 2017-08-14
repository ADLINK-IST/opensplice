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
#ifndef SAC_ENTITY_H
#define SAC_ENTITY_H

#include "dds_dcps.h"
#include "dds_dcps_private.h"
#include "sac_common.h"

#define SAC_ENTITY_MAX_SUPPORTED_SECONDS(e) \
    (_Entity(e)->maxSupportedSeconds)

DDS_ReturnCode_t
_Entity_deinit (
    _Object _this);

DDS_ReturnCode_t
DDS_Entity_init (
    DDS_Entity _this,
    u_entity uEntity);

DDS_ReturnCode_t
DDS_Entity_set_listenerDispatcher (
    DDS_Entity _this,
    cmn_listenerDispatcher listenerDispatcher);

cmn_listenerDispatcher
DDS_Entity_get_listenerDispatcher (
    DDS_Entity _this);

DDS_boolean
_Entity_is_enabled (
    _Entity _this);

DDS_boolean
DDS_Entity_check_handle (
    DDS_Entity _this,
    DDS_InstanceHandle_t handle);

DDS_ReturnCode_t
DDS_Entity_get_user_entity (
    DDS_Entity _this,
    DDS_ObjectKind kind,
    u_entity *uEntity);

u_entity
_Entity_get_user_entity (
    _Entity _this);

DDS_ReturnCode_t
DDS_Entity_set_listener_interest (
    DDS_Entity _this,
    const DDS_StatusMask mask);

DDS_StatusMask
DDS_Entity_get_listener_interest (
    DDS_Entity _this);

DDS_ReturnCode_t
DDS_Entity_reset_dataAvailable_status (
    DDS_Entity _this);

DDS_ReturnCode_t
DDS_Entity_reset_on_data_on_readers_status (
    DDS_Entity _this);

DDS_ReturnCode_t
DDS_Entity_disable_callbacks (
    DDS_Entity _this);

void
DDS_Entity_wait_listener_removed_wlReq(
    DDS_Entity _this);

void
DDS_Entity_notify_listener_removed(
    DDS_Entity _this);

DDS_string
DDS_Entity_get_name(
    DDS_Entity _this);

#endif /* SAC_ENTITY_H */
