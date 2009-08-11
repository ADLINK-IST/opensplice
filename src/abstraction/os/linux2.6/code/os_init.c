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

/** \file os/linux/code/os_init.c
 *  \brief Initialization / Deinitialization
 */

#include <code/os__process.h>
#include <code/os__thread.h>
#include <code/os__sharedmem.h>

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
    os_processModuleInit();
    os_threadModuleInit();
    os_sharedMemoryInit();
    os_mutexModuleInit();
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
    os_mutexModuleExit();
    os_sharedMemoryExit();
    os_threadModuleExit();
    os_processModuleExit();
    return;
}

#include <../common/code/os_service.c>
