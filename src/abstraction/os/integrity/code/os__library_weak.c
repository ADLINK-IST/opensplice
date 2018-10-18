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
 */
#include <stddef.h>
#include "os_EntryPoints.h"

#pragma weak os_dynamicLibPlugin
struct os_dynamicLoad_plugin *os_dynamicLibPlugin = NULL;
#pragma weak os_staticLibraries
struct os_librarySymbols os_staticLibraries[] = { NULL, NULL };
