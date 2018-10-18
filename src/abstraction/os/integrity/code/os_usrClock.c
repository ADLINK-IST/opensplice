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
#include "os_time.h"
#include "os_process.h"
#include "os_defs.h"

typedef	int (*startClock)(void);
typedef int (*stopClock)(void);
typedef os_time (*getClock)(void);

static stopClock stopFunction;

os_result
os_userClockStart (
    char *userClockModule,
    char *startName,
    char *stopName,
    char *getName
    )
{
    os_result result = os_resultFail;
    return result;
}
    
os_result
os_userClockStop (
    void
    )
{
    os_result result = os_resultFail;
    return result;
}
