/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
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
#include <stdio.h>

#define OS_DEBUG(msg)                   printf(msg "\n"); fflush(stdout)
#define OS_DEBUG_1(msg, a1)             printf(msg "\n", a1); fflush(stdout)
#define OS_DEBUG_2(msg, a1, a2)         printf(msg "\n", a1, a2); fflush(stdout)
#define OS_DEBUG_3(msg, a1, a2, a3)     printf(msg "\n", a1, a2, a3); fflush(stdout)
#define OS_DEBUG_4(msg, a1, a2, a3, a4) printf(msg "\n", a1, a2, a3, a4); fflush(stdout)
#else
#define OS_DEBUG(msg)
#define OS_DEBUG_1(msg, a1)
#define OS_DEBUG_2(msg, a1, a2)
#define OS_DEBUG_3(msg, a1, a2, a3)
#define OS_DEBUG_4(msg, a1, a2, a3, a4)
#endif

void os_debugModeInit();
void os_debugModeExit();

#endif /* __OS__DEBUG_H__ */
