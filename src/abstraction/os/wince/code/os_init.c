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
/****************************************************************
 * Initialization / Deinitialization                            *
 ****************************************************************/

/** \file os/win32/code/os_init.c
 *  \brief Initialization / Deinitialization
 */

#include "os_defs.h"
#include "code/os__process.h"
#include "code/os__sharedmem.h"
#include "code/os__thread.h"
#include "code/os__socket.h"
#include "code/os__cond.h"
#include "code/os__mutex.h"
#include "code/os__time.h"
#include "os__debug.h"
#include "os_report.h"
#include "os_abstract.h"
#include "os_heap.h"
#include "os_mutex.h"
#include "os__service.h"
#include "os_stdlib.h"
#include "os_atomics.h"

/** \brief Counter that keeps track of number of times os-layer is initialized */
static pa_uint32_t _ospl_osInitCount = PA_UINT32_INIT(0);

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

    initCount = pa_inc32_nv(&_ospl_osInitCount);

    if (initCount == 1)
    {
        os_processModuleInit();
        os_reportInit(OS_FALSE);
        //os_debugModeInit();  // Not in CE

        os_serviceInitialiseMsgQueueData();

        os_mutexModuleInit();
        os_condModuleInit();
        os_timeModuleInit();
        os_sharedMemoryInit();
        os_threadModuleInit();
        os_socketModuleInit();
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

    initCount = pa_dec32_nv(&_ospl_osInitCount);

    if (initCount == 0)
    {
        os_socketModuleExit();
        os_threadModuleExit();
        os_sharedMemoryExit();
        os_timeModuleExit();
        os_condModuleExit();
        os_mutexModuleExit();

        os_serviceFinaliseMsgQueueData();

        //os_debugModeExit(); // Not in CE
        os_processModuleExit();
    }
    else if ((initCount + 1) < initCount)
    {
        // The 0 boundary is passed, so os_osExit is called more often than
        //  os_osInit. Therefore undo decrement as nothing happened and warn.
        initCount = pa_inc32_nv(&_ospl_osInitCount);
        OS_REPORT(OS_WARNING, "os_osExit", 1, "OS-layer not initialized");
        // Fail in case of DEV, as it is incorrect API usage
        assert(0);
    }
    return;
}

#include "code/os_service.c"

/* We need this on windows to make sure the main thread of MFC applications
 * calls os_osInit().
 */
/*
BOOL WINAPI DllMain(     NOTE: This might have to be added again in original build environment
    HINSTANCE hinstDLL,  // handle to DLL module
    DWORD fdwReason,     // reason for calling function
    LPVOID lpReserved )  // reserved
{
    // Perform actions based on the reason for calling.
    switch( fdwReason ) {
    case DLL_PROCESS_ATTACH:
        // Initialize once for each new process.
         // Return FALSE to fail DLL load.
         //
        os_osInit();
    break;
    case DLL_THREAD_ATTACH:
         // Do thread-specific initialization.
          //
    break;
    case DLL_THREAD_DETACH:
         // Do thread-specific cleanup.
          //
    break;
    case DLL_PROCESS_DETACH:
        // Perform any necessary cleanup.
         //
        os_osExit();
    break;
    }
    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}*/
