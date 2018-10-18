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
#ifndef U__INSTANCEHANDLE_H
#define U__INSTANCEHANDLE_H

#include "u_instanceHandle.h"
#include "v_collection.h"

v_gid
u_instanceHandleToGID (
    u_instanceHandle _this);

/* Depricated : only for GID publication_handle legacy. */

u_instanceHandle
u_instanceHandleFix(
    u_instanceHandle _this,
    v_collection reader);

u_result
u_instanceHandleClaim (
    u_instanceHandle _this,
    void *instance);

u_result
u_instanceHandleRelease (
    u_instanceHandle _this);

#endif
