/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef __OS__DEBUG_H__
#define __OS__DEBUG_H__

#ifndef NDEBUG
#include "os_report.h"

#define OS_DEBUG(op, msg)                   OS_REPORT  (OS_INFO, op, 1, msg)
#define OS_DEBUG_1(op, msg, a1)             OS_REPORT_1(OS_INFO, op, 1, "OS_DEBUG : " msg, a1)
#define OS_DEBUG_2(op, msg, a1, a2)         OS_REPORT_2(OS_INFO, op, 1, "OS_DEBUG : " msg, a1, a2)
#define OS_DEBUG_3(op, msg, a1, a2, a3)     OS_REPORT_3(OS_INFO, op, 1, "OS_DEBUG : " msg, a1, a2, a3)
#define OS_DEBUG_4(op, msg, a1, a2, a3, a4) OS_REPORT_4(OS_INFO, op, 1, "OS_DEBUG : " msg, a1, a2, a3, a4)
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
