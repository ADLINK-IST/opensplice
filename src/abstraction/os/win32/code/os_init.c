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
#include "os_abstract.h"
#include "os_heap.h"
#include <stdio.h>
#include <process.h>

/** \brief Counter that keeps track of number of times os-layer is initialized */
static os_uint32 _ospl_osInitCount = 0;

/** \brief OS layer initialization
 *
 * \b os_osInit calls:
 * - \b os_sharedMemoryInit
 * - \b os_threadInit
 */
void
os_osInit(void)
{
    os_uint32 initCount;

    initCount = pa_increment(&_ospl_osInitCount);

    if (initCount == 1) {
        os_debugModeInit();
        os_timeModuleInit();
        os_socketModuleInit( os_report, os_malloc);
        os_processModuleInit();
        os_sharedMemoryInit();
        os_threadModuleInit();
    } else {
        OS_REPORT_1(OS_INFO, "os_osInit", 1,
                "OS-layer initialization called %d times", initCount);
    }
    return;
}

/** \brief OS layer deinitialization
 */
void
os_osExit(
    void)
{
    os_uint32 initCount;

    initCount = pa_decrement(&_ospl_osInitCount);

    if (initCount == 0) {
        os_threadModuleExit();
        os_sharedMemoryExit();
        os_processModuleExit();
        os_socketModuleExit();
        os_timeModuleExit();
        os_debugModeExit();
    } else if ((initCount + 1) < initCount){
        /* The 0 boundary is passed, so os_osExit is called more often than
         * os_osInit. Therefore undo decrement as nothing happened and warn. */
        initCount = pa_increment(&_ospl_osInitCount);
        OS_REPORT(OS_WARNING, "os_osExit", 1, "OS-layer not initialized");
        /* Fail in case of DEV, as it is incorrect API usage */
        assert(0);
    }
    return;
}

#include "code/os_service.c"
