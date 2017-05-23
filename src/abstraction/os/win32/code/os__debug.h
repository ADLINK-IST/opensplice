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
#ifndef __OS__DEBUG_H__
#define __OS__DEBUG_H__

#ifndef NDEBUG
#include "os_report.h"

#define OS_DEBUG(op, msg)                   OS_REPORT  (OS_INFO, op, 1, msg)
#define OS_DEBUG_1(op, msg, a1)             OS_REPORT(OS_INFO, op, 1, "OS_DEBUG : " msg, a1)
#define OS_DEBUG_2(op, msg, a1, a2)         OS_REPORT(OS_INFO, op, 1, "OS_DEBUG : " msg, a1, a2)
#define OS_DEBUG_3(op, msg, a1, a2, a3)     OS_REPORT(OS_INFO, op, 1, "OS_DEBUG : " msg, a1, a2, a3)
#define OS_DEBUG_4(op, msg, a1, a2, a3, a4) OS_REPORT(OS_INFO, op, 1, "OS_DEBUG : " msg, a1, a2, a3, a4)
#else
#define OS_DEBUG(op, msg)
#define OS_DEBUG_1(op, msg, a1)
#define OS_DEBUG_2(op, msg, a1, a2)
#define OS_DEBUG_3(op, msg, a1, a2, a3)
#define OS_DEBUG_4(op, msg, a1, a2, a3, a4)
#endif

void os_debugModeInit();
void os_debugModeExit();

/* These declarations have no earthly business here but I point blank refuse
 * to add *another* __local__ include file to the abstraction layer and this file
 * is the only 'common' include. */
void os_timeModuleInit(void);
void os_timeModuleExit(void);

#endif /* __OS__DEBUG_H__ */
