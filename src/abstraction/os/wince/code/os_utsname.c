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
#include <os_stdlib.h>

os_int32 os_uname(os_utsname *name)
{
   OSVERSIONINFO verInf;
   SYSTEM_INFO sysInf;
   char hostname[255];

   verInf.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
   GetVersionEx(&verInf);
   GetSystemInfo(&sysInf);

   switch(sysInf.wProcessorArchitecture)
   {
      case PROCESSOR_ARCHITECTURE_INTEL:
         os_sprintf (name->machine, "x86");
         break;
      case PROCESSOR_ARCHITECTURE_ARM:
         os_sprintf (name->machine, "arm");
         break;
      default:
         os_sprintf (name->machine, "unknown");
   }

   os_gethostname (hostname, 255);

   os_sprintf (name->sysname, "WindowsCE");
   os_strcpy (name->nodename, hostname);
   os_sprintf (name->release, "Build #%d", verInf.dwBuildNumber);
   os_sprintf (name->version, "%2d.%2d", verInf.dwMajorVersion, verInf.dwMinorVersion);

    return 0;
}

