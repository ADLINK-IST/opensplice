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

/** \file os/vxworks5.5/code/os_init.c
 *  \brief Initialization / Deinitialization
 */

/****************************************************************
 * Initialization / Deinitialization                            *
 ****************************************************************/

/** \file os/common/code/os_init.c
 *  \brief Initialization / Deinitialization
 */

#include "code/os__sharedmem.h"
#include "code/os__thread.h"
#include "os_abstract.h"
#include "os_report.h"
#include "os_process.h"
#include "os_stdlib.h"

#ifndef NDEBUG
#include "os_atomics.h"
extern atomic_t os__reallocdoublecopycount ;
#endif

/** \brief Counter that keeps track of number of times os-layer is initialized */
static pa_uint32_t _ospl_osInitCount = PA_UINT32_INIT(0);

/* Check expression is true at compile time ( no code generated )
     - will get negative sized array on failure */
#define EXPRCHECK(expr) (void)sizeof(char[1-(signed int)(2*(unsigned int)!(expr))]);


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

    /* Sanity checks, don't remove, they don't generate any code anyway. */
    EXPRCHECK(sizeof(void *)==sizeof(os_os_uintptr))
    EXPRCHECK(sizeof(os_os_char)==1)
    EXPRCHECK(sizeof(os_os_uchar)==1)
    EXPRCHECK(sizeof(os_os_short)==2)
    EXPRCHECK(sizeof(os_os_ushort)==2)
    EXPRCHECK(sizeof(os_os_int32)==4)
    EXPRCHECK(sizeof(os_os_uint32)==4)
    EXPRCHECK(sizeof(os_os_int64)==8)
    EXPRCHECK(sizeof(os_os_uint64)==8)
    EXPRCHECK(sizeof(os_os_float)==4)
    EXPRCHECK(sizeof(os_os_double)==8)
    EXPRCHECK(sizeof(void *)==sizeof(os_os_address))
    EXPRCHECK(sizeof(void *)==sizeof(os_os_saddress))

    #if ! defined(__PPC) && ! defined(__x86_64__)
      /* Check for heap realignment code which relies */
      /* on types below being 32bit. */
       EXPRCHECK(sizeof(size_t)==4)
       EXPRCHECK(sizeof(void *)==4)
    #endif

    initCount = pa_inc32_nv(&_ospl_osInitCount);

    if (initCount == 1) {
        /* init for programs using data base threads */
        os_threadModuleInit();
        os_reportInit(OS_FALSE);
        os_procInitialize();
        os_stdlibInitialize();
        os_sharedMemoryInit();
    } else {
        os_procInitialize();
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
#ifndef NDEBUG
    OS_REPORT(OS_INFO, "os__reallocdoublecopycount", 0, "count=%d", vxAtomicGet(&os__reallocdoublecopycount));
#endif
    initCount = pa_dec32_nv(&_ospl_osInitCount);

    if (initCount == 0) {
        os_sharedMemoryExit();
        os_reportExit();
        os_threadModuleExit();
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

/* This constructor is invoked when the library is loaded into a process. */
void __attribute__ ((constructor))
os__osInit(
        void)
{
    os_osInit();
}

/* This destructor is invoked when the library is unloaded from a process. */
void __attribute__ ((destructor))
os__osExit(
        void)
{
    os_osExit();
}

#include "../common/code/os_service.c"
