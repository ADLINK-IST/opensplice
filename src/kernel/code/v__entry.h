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

#ifndef V__ENTRY_H
#define V__ENTRY_H

#include "v_entry.h"

#define v_entryReader(_this) \
        v_reader(v_entry(_this)->reader)

#define v_entryReaderQos(_this) \
        (v_entryReader(_this)->qos)

void
v_entryInit (
    v_entry _this,
    v_reader r);

void
v_entryFree (
    v_entry _this);

void
v_entryRemoveGroup (
    v_entry _this,
    v_group g);

c_bool
v_entryGroupExists(
    v_entry entry,
    v_group group);

v_alignState
v_entryNotifyGroupStateChange(
    v_entry _this,
    v_handle groupHandle,
    v_alignState alignState);

c_long
v_entryDurableGroupCount(
    v_entry _this);

c_bool
v_entryWalkGroups(
    v_entry _this,
    c_action action,
    c_voidp actionArg);

c_iter
v_entryGetGroups(
    v_entry _this);

#endif
