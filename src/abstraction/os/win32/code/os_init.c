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
/****************************************************************
 * Initialization / Deinitialization                            *
 ****************************************************************/

/** \file os/win32/code/os_init.c
 *  \brief Initialization / Deinitialization
 */

#include "os_defs.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include "os__process.h"
#include "os__sharedmem.h"
#include "os__thread.h"
#include "os__socket.h"
#include "os__debug.h"
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
 * - \b os_reportInit(OS_FALSE);
 * - \b os_debugModeInit();
 * - \b os_timeModuleInit();
 * - \b os_processModuleInit();
 * - \b os_sharedMemoryInit
 * - \b os_threadModuleInit
 */
void
os_osInit(void)
{
    os_uint32 initCount;

    initCount = pa_increment(&_ospl_osInitCount);

    if (initCount == 1) {
        os_reportInit(OS_FALSE);
        os_debugModeInit();
        os_timeModuleInit();
        os_processModuleInit();
        os_sharedMemoryInit();
        os_threadModuleInit();
	os_socketModuleInit();
    } /* Else initialization is already done. */
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
    	os_socketModuleExit();
        os_threadModuleExit();
        os_sharedMemoryExit();
        os_processModuleExit();
        os_timeModuleExit();
        os_debugModeExit();
        os_reportExit();
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

/* We need this on windows to make sure the main thread of MFC applications
 * calls os_osInit().
 */
BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  /* handle to DLL module */
    DWORD fdwReason,     /* reason for calling function */
    LPVOID lpReserved )  /* reserved */
{
    /* Perform actions based on the reason for calling.*/
    switch( fdwReason ) {
    case DLL_PROCESS_ATTACH:
        /* Initialize once for each new process.
         * Return FALSE to fail DLL load.
         */
        os_osInit();
    break;
    case DLL_THREAD_ATTACH:
         /* Do thread-specific initialization.
          */
    break;
    case DLL_THREAD_DETACH:
         /* Do thread-specific cleanup.
          */
    break;
    case DLL_PROCESS_DETACH:
        /* Perform any necessary cleanup.
         */
        os_osExit();
    break;
    }
    return TRUE;  /* Successful DLL_PROCESS_ATTACH.*/
}
