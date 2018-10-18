/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
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

#endif /* __OS__DEBUG_H__ */
