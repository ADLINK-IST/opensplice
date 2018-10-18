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

#include <os_utsname.h>

os_int32 os_uname(os_utsname *name)
{
    char hostname[255];

    os_gethostname (hostname, 255);
    os_sprintf (name->sysname, "%s %s %s", runtimeName, runtimeVersion,sysModel(
));
    os_strcpy (name->machine, sysModel());
    os_strcpy (name->nodename, hostname);
    os_sprintf (name->release, "%s", runtimeName);
    os_sprintf (name->version, "%s", runtimeVersion);
    return (OK);
}
