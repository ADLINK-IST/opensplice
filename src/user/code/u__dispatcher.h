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

#ifndef U__DISPATCHER_H
#define U__DISPATCHER_H

#include "u_observable.h"
#include "u_entity.h"

/* Returns mask of events that have been handled by the listener. */
typedef u_eventMask (*u_dispatcherListener)(u_dispatcher o, u_eventMask event, void *usrData);
typedef void (*u_dispatcherThreadAction)(u_dispatcher o, void *usrData);

#define  u_dispatcher(o) ((u_dispatcher)(o))

u_dispatcher
u_dispatcherNew(
    const u_observable observable);

u_result
u_dispatcherFree(
    u_dispatcher _this);

u_result
u_dispatcherInsertListener(
    const u_dispatcher _this,
    const u_eventMask eventMask,
    const u_observableListener l,
    void *userData);

u_result
u_dispatcherRemoveListener(
     const u_dispatcher _this,
     const u_observableListener l);

#endif

