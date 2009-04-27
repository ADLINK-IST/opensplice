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

/** \file os/common/code/os_init.c
 *  \brief Initialization / Deinitialization
 */

#include <code/os__sharedmem.h>
#include <code/os__thread.h>

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
    os_sharedMemoryInit();
    os_threadModuleInit();
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
    os_sharedMemoryExit();
    os_threadModuleExit();
    return;
}
