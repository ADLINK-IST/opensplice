/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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

#include <os_utsname.h>
#include <os_stdlib.h>
#include "os_win32incs.h"

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
      case PROCESSOR_ARCHITECTURE_AMD64:
         os_sprintf (name->machine, "x86_64");
         break;
      case PROCESSOR_ARCHITECTURE_INTEL:
         os_sprintf (name->machine, "x86");
         break;
      default:
         os_sprintf (name->machine, "unknown");
   }

   os_gethostname (hostname, 255);

   os_sprintf (name->sysname, "Windows");
   os_strcpy (name->nodename, hostname);
   os_sprintf (name->release, "Build #%d", verInf.dwBuildNumber);
   os_sprintf (name->version, "%2d.%2d", verInf.dwMajorVersion, verInf.dwMinorVersion);

    return 0;
}

