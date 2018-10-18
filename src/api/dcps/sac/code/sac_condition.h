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
#ifndef DDS_CONDITION_H
#define DDS_CONDITION_H

#include "dds_dcps.h"

DDS_ReturnCode_t
DDS_Condition_init (
    DDS_Condition _this,
    u_object uObject,
    GetTriggerValue getTriggerValue);

DDS_ReturnCode_t
_Condition_deinit (
    _Object _this);

DDS_boolean
DDS_Condition_get_trigger_value (
    DDS_Condition _this);

u_object
DDS_Condition_get_user_object (
    DDS_Condition _this);

u_object
_Condition_get_user_object (
    _Condition _this);

DDS_ReturnCode_t
DDS_Condition_attach_waitset (
    DDS_Condition _this,
    DDS_WaitSet waitset);

DDS_ReturnCode_t
DDS_Condition_detach_waitset (
    DDS_Condition _this,
    DDS_WaitSet waitset);

DDS_ReturnCode_t
DDS_Condition_trigger (
    DDS_Condition _this);

DDS_ReturnCode_t
DDS_Condition_is_alive(
   DDS_Condition _this);

#endif
