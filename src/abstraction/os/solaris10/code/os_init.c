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

/** \file os/solaris/code/os_init.c
 *  \brief Initialization / Deinitialization
 */

#include "code/os__process.h"
#include "code/os__thread.h"
#include "code/os__sharedmem.h"
#include "os_abstract.h"
#include "os_report.h"

/** \brief Counter that keeps track of number of times os-layer is initialized */
static os_uint32 _ospl_osInitCount = 0;

/** \brief OS layer initialization
 *
 * \b os_osInit calls:
 * - \b os_sharedMemoryInit
 * - \b os_threadInit
 */
void
os_osInit (
    void)
{
    os_uint32 initCount;

    initCount = pa_increment(&_ospl_osInitCount);

    if (initCount == 1) {
        os_reportInit(OS_FALSE);
        //os_processModuleInit();
        os_threadModuleInit();
        os_sharedMemoryInit();
    } else {
#ifndef NDEBUG
        OS_REPORT_1(OS_INFO, "os_osInit", 1,
                "OS-layer initialization called %d times", initCount);
#endif /* NDEBUG */
    }
    return;
}

/** \brief OS layer deinitialization
 *
 * \b os_osExit calls:
 * - \b os_sharedMemoryExit
 * - \b os_threadExit
 */
void
os_osExit (
    void)
{
    os_uint32 initCount;

    initCount = pa_decrement(&_ospl_osInitCount);

    if (initCount == 0) {
       // os_processModuleExit();
        os_sharedMemoryExit();
        os_threadModuleExit();
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

#include "../common/code/os_service.c"
