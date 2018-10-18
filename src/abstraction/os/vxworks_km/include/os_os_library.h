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
#ifndef OS_VXWORKS_LIBARY_H
#define OS_VXWORKS_LIBARY_H

#include "os_EntryPoints.h"
#include "os_os_types.h"
#include <loadLib.h>

#if defined (__cplusplus)
extern "C" {
#endif

#define OS_LOAD_GLOBAL_SYMBOLS LOAD_GLOBAL_SYMBOLS

typedef struct os_os_libraryAttr {
    os_os_int32 flags;
    os_boolean autoTranslate; /* Determines whether the library name is automatically mapped to its platform dependent name*/
    os_boolean staticLibOnly;
} os_os_libraryAttr;

typedef struct os_os_library_t
{
   union
   {
      os_entryPoint *entryPoints;
      MODULE_ID library;
   } l;
   char isStatic;
} *os_os_library;
typedef void *os_os_symbol;

#if defined (__cplusplus)
}
#endif

#endif /* OS_VXWORKS_LIBRARY_H */
