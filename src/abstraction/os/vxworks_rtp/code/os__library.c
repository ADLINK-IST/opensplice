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

#include "os_library.h"

#ifdef OSPL_USE_STATIC_LINKED_SINGLE_PROCESS
#include "os/rtems4.10.0/code/os__library.c"
#else
void
os_libraryAttrInit(
    os_libraryAttr *attr)
{
   /* dynamic libraries not available on VxWorks */
   OS_UNUSED_ARG(attr);
}

os_library
os_libraryOpen(
    const char *name,
    os_libraryAttr *attr)
{
   /* dynamic libraries not available on VxWorks */
   (void)name;
   (void)attr;
   return NULL;
}

os_result
os_libraryClose(
    os_library library)
{
   /* dynamic libraries not available on VxWorks */
   (void)library;
   return os_resultUnavailable;
}

os_symbol
os_libraryGetSymbol(
    os_library library,
    const char *symbolName)
{
   /* dynamic libraries not available on VxWorks */
   (void)library;
   (void)symbolName;
   return NULL;
}
#endif
