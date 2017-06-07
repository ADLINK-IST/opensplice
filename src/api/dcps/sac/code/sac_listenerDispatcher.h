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
#ifndef DDS_LISTENERDISPATCHER_H
#define DDS_LISTENERDISPATCHER_H

#include "vortex_os.h"

os_schedClass
DDS_ListenerDispatcher_scheduling_class (
    const DDS_SchedulingQosPolicy *scheduling);

os_int32
DDS_ListenerDispatcher_scheduling_priority (
    const DDS_SchedulingQosPolicy *scheduling);

void
DDS_ListenerDispatcher_event_handler (
    v_listenerEvent event,
    c_voidp arg);

#endif /* DDS_LISTENERDISPATCHER_H */
