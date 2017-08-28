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
/****************************************************************
 * Initialization / Deinitialization                            *
 ****************************************************************/

/** \file os/win32/code/os_init.c
 *  \brief Initialization / Deinitialization
 */

#include "os_defs.h"
#ifndef LITE
#include "os__win32EntropyHook.h"
#include "os__sharedmem.h"
#endif
#include "os__process.h"
#include "os__thread.h"
#include "os__socket.h"
#include "os__debug.h"
#include "os__time.h"
#include "os__sync.h"
#include "os_report.h"
#include "os_uniqueNodeId.h"
#include "os_abstract.h"
#include "os_heap.h"
#include "os_time.h"
#include <stdio.h>
#include <process.h>
#include "os_atomics.h"
#include "os_uniqueNodeId.h"

#if !defined(_WIN32_WINNT) || _WIN32_WINNT < 0x0502
/* Minimum Windows Server 2003 SP1, Windows XP SP2 == _WIN32_WINNT_WS03 (0x0502) */
#error _WIN32_WINNT should be defined as a build argument and should be at least _WIN32_WINNT_WS03 (0x0502)
#endif

#if _WIN32_WINNT != 0x0502
/* Until target Windows support is explicitly defined, we expect exactly Windows Server 2003 SP1,
 * Windows XP SP2 support (_WIN32_WINNT_WS03/0x0502) */
#error _WIN32_WINNT should be exactly _WIN32_WINNT_WS03 (0x0502) until OSPL-7679 is realised.
#endif

/** \brief Counter that keeps track of number of times os-layer is initialized */
static pa_uint32_t _ospl_osInitCount = PA_UINT32_INIT(0);

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

    initCount = pa_inc32_nv(&_ospl_osInitCount);

    if (initCount == 1) {
#ifndef LITE
        os_uniqueIdSetEntropyHook( os_win32EntropyHook );
#endif
        os_syncModuleInit();
        os_threadModuleInit(); /* Allocates TLS needed by os_report(...) */
        os_timeModuleInit();
        os_reportInit(OS_FALSE);
        os_debugModeInit();
        os_socketModuleInit();
#ifndef LITE
        os_processModuleInit();
        os_sharedMemoryInit();
#endif
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

    initCount = pa_dec32_nv(&_ospl_osInitCount);

    if (initCount == 0) {
#ifndef LITE
	os_sharedMemoryExit();
        os_processModuleExit();
#endif
        os_socketModuleExit();
        os_debugModeExit();
        os_reportExit();
        os_timeModuleExit();
        os_threadModuleExit(); /* Deallocates TLS needed by os_report(...) */
        os_syncModuleDeinit();
    } else if ((initCount + 1) < initCount){
        /* The 0 boundary is passed, so os_osExit is called more often than
         * os_osInit. Therefore undo decrement as nothing happened and warn. */
        initCount = pa_inc32_nv(&_ospl_osInitCount);
        OS_REPORT(OS_WARNING, "os_osExit", 1, "OS-layer not initialized");
        /* Fail in case of DEV, as it is incorrect API usage */
        assert(0);
    }
    return;
}

#include "../common/code/os_service.c"
#include "os_win32incs.h"

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
        os_threadMemClear();
    break;
    case DLL_PROCESS_DETACH:
        /* Perform any necessary cleanup.
         */
        os_osExit();
    break;
    }
    return TRUE;  /* Successful DLL_PROCESS_ATTACH.*/
}
