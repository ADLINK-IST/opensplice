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

#ifndef V__LIFESPANADMIN_H
#define V__LIFESPANADMIN_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "v_kernel.h"
#include "v_lifespanSample.h"

#define v_lifespanAdmin(o) (C_CAST((o),v_lifespanAdmin))

v_lifespanAdmin
v_lifespanAdminNew(
    v_kernel kernel);

void
v_lifespanAdminInsert(
    v_lifespanAdmin _this,
    v_lifespanSample sample);

void
v_lifespanAdminRemove(
    v_lifespanAdmin _this,
    v_lifespanSample sample);
void
v_lifespanAdminTakeExpired(
    v_lifespanAdmin _this,
    os_timeE now,
    v_lifespanSampleAction action,
    c_voidp arg);

c_long
v_lifespanAdminSampleCount(
    v_lifespanAdmin _this);

#if defined (__cplusplus)
}
#endif

#endif /* V__LIFESPANADMIN_H */
