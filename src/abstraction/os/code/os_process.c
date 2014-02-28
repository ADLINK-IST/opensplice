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

#include "os_process.h"
#include "os_report.h"

/**
* Indicates if this process is an OpenSplice service process.
* @see os_procIsOpenSpliceService
*/
static os_boolean isOpenSpliceService = OS_FALSE;

/**
* Indicates if this process is the OpenSplice domain daemon process.
* @see os_procIsOpenSpliceDomainDaemon
*/
static os_boolean isSpliceD = OS_FALSE;

/**
* Determines whether this process is an OpenSplice service.
* OpenSplice service processes are: 1/ spliced and
* 2/ those processes whose executable is part of the OpenSplice distribution and which
* are typically started by spliced &/or without a console.
*/
os_boolean
os_procIsOpenSpliceService(void)
{
    return isOpenSpliceService;
}

/**
* Determines whether this process is the OpenSplice domain daemon,
* i.e. spliced.
*/
os_boolean
os_procIsOpenSpliceDomainDaemon(void)
{
    return isSpliceD;
}

/**
* Hook to set up the process as an OpenSplice service.
* This should be called as soon as possible in a given service
* executable i.e. at the start of main if possible, & before
* any other OS calls whose behaviour may need to change
* depending on whether this process is a service or not
* @param thisIsSpliceD If this process
* @see os_procIsOpenSpliceService
*/
void
os_procInitializeOpenSpliceService(os_boolean thisIsSpliceD)
{
    isOpenSpliceService = OS_TRUE;

    if (thisIsSpliceD)
    {
        isSpliceD = thisIsSpliceD;
    }

    /* The os_reportInit hook to initialise the base process values for reporting
    is called from os_osInit. On windows the os_osInit call has been linked
    into the DLL load of ddsos so it will have already occured by the time
    os_procInitializeOpenSpliceService occurs, even if it is first action in main.
    We therefore need to call this again now and pass a 'true' value for
    forceReInit to make it repeat its actions again taking account of the new values
    we have just set */
#if defined _WIN32
    os_reportInit(OS_TRUE);
#endif
}
