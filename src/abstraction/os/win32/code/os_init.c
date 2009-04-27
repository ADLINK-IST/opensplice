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
/****************************************************************
 * Initialization / Deinitialization                            *
 ****************************************************************/

/** \file os/win32/code/os_init.c
 *  \brief Initialization / Deinitialization
 */

#include <os_defs.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include "code/os__process.h"
#include "code/os__sharedmem.h"
#include "code/os__thread.h"
#include "code/os__socket.h"
#include "code/os__time.h"
#include "code/os__debug.h"
#include "os_report.h"
#include "os_heap.h"
#include <stdio.h>
#include <process.h>


/** \brief OS layer initialization
 *
 * \b os_osInit calls:
 * - \b os_sharedMemoryInit
 * - \b os_threadInit
 */
void
os_osInit(void)
{
    os_timeModuleInit();
    os_socketModuleInit( os_report, os_malloc);
    os_processModuleInit();
    os_sharedMemoryInit();
    os_threadModuleInit();
    
    return;
}

/** \brief OS layer deinitialization
 */
void
os_osExit(
    void)
{
    os_threadModuleExit();
    os_sharedMemoryExit();
    os_processModuleExit();
    os_socketModuleExit();
    os_timeModuleExit();
    return;
}

#include "code/os_service.c"
