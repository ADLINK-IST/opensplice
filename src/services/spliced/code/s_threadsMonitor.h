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
#ifndef S_THREADSMONITOR_H
#define S_THREADSMONITOR_H

#include "s_types.h"
#include "report.h"

#if defined (__cplusplus)
extern "C" {
#endif

extern const char* s_main_tread_name;

void 
s_threadsMonitorSetInterval(
    spliced splicedaemon);


s_threadsMonitor
s_threadsMonitorNew(
    spliced splicedaemon);

os_boolean
s_threadsMonitorFree(
    s_threadsMonitor _this);

#if defined (__cplusplus)
}
#endif

#endif /* S_THREADSMONITOR_H */
