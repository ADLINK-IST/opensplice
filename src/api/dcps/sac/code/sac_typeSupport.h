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
#ifndef DDS_TYPESUPPORT_H
#define DDS_TYPESUPPORT_H

#include "dds_dcps.h"
#include "sac_common.h"

DDS_TypeSupport
DDS_TypeSupportKeep(
   DDS_TypeSupport _this);

DDS_long
DDS_TypeSupport_get_alloc_size (
    DDS_TypeSupport _this);

DDS_allocBuffer
DDS_TypeSupport_get_alloc_buffer (
    DDS_TypeSupport _this);

DDS_copyIn
DDS_TypeSupportCopyIn (
    DDS_TypeSupport _this);

DDS_copyOut
DDS_TypeSupportCopyOut (
    DDS_TypeSupport _this);

DDS_copyCache
DDS_TypeSupportCopyCache (
    DDS_TypeSupport _this);

DDS_ReturnCode_t
DDS_TypeSupport_compatible(
    DDS_TypeSupport _this,
    const DDS_DomainParticipant dp);

DDS_string
DDS_TypeSupport_get_internal_type_name(
    DDS_TypeSupport _this);

#endif
