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
#ifndef SAC_OBJECT_H
#define SAC_OBJECT_H

#include "dds_dcps.h"
#include "dds_dcps_private.h"
#include "sac_common.h"

#define _ObjectKind(o) (_Object(o)->kind)

DDS_ReturnCode_t
DDS_Object_new (
    DDS_ObjectKind kind,
    _Object_destructor_t destructor,
    _Object *_this);

DDS_ReturnCode_t
DDS_Object_claim (
    DDS_Object _this,
    DDS_ObjectKind kind,
    _Object *object);

DDS_ReturnCode_t
DDS_Object_wait_wlReq (
    DDS_Object _this);

DDS_ReturnCode_t
DDS_Object_trigger (
    DDS_Object _this);

DDS_ReturnCode_t
_Object_trigger_claimed(
    _Object object);

DDS_ReturnCode_t
DDS_Object_release (
    DDS_Object _this);

DDS_ObjectKind
DDS_Object_get_kind(
    DDS_Object _this);

DDS_ReturnCode_t
DDS_Object_check (
    DDS_Object _this,
    DDS_ObjectKind kind);

DDS_ReturnCode_t
DDS_Object_check_and_assign(
    DDS_Object _this,
    DDS_ObjectKind kind,
    _Object *object);

void
DDS_Object_set_domain_id(
    _Object object,
    os_int32 domainId);


#endif /* SAC_OBJECT_H */
