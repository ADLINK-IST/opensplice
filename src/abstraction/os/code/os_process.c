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
    isSpliceD = thisIsSpliceD;

    /* The os_reportInit hook to initialise the base process values for reporting
    is called from os_osInit. On windows the os_osInit call has been linked
    into the DLL load of ddsos so it will have already occured by the time
    os_procInitializeOpenSpliceService occurs, even if it is first action in main.
    We therefore need to call this again now and pass a 'true' value for
    forceReInit to make it repeat its actions again taking account of the new values
    we have just set */
#if defined ( _WIN32 ) || defined ( _WRS_KERNEL )
    os_reportInit(OS_TRUE);
#else
    /* It is possible (depending on environment variables for instance) that the
     * os_reportInit() has to do slightly different things when it is initialized
     * by spliced. So, do it again in that case. */
    if (thisIsSpliceD)
    {
        os_reportInit(OS_TRUE);
    }
#endif
}
