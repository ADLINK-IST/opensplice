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

#ifndef V__QUERY_H
#define V__QUERY_H

#include "v_query.h"

v_result
v_queryInit(
    v_query q,
    v_collection src,
    const os_char *name,
    const os_char *expression);

q_expr
v_queryGetPredicate(
    v_query _this);

c_bool
v_queryNotifyDataAvailable (
    v_query _this,
    v_event e);

void
v_queryDetachWaitsets(
    v_query _this);

#endif
