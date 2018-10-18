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
#ifndef OS_DYNAMICLIB_PLUGIN_H
#define OS_DYNAMICLIB_PLUGIN_H
#include "os_library.h"

typedef void *(* os_dynamicLoadLib_fn)( const char *executable_file );

typedef os_os_library (* os_dynamicLibraryOpen_fn)(const char *name,
					           os_os_libraryAttr *attr);

typedef os_result (* os_dynamicLibraryClose_fn)(os_os_library library);

typedef os_os_symbol (* os_dynamicLibraryGetSymbol_fn)(os_os_library library,
						       const char *symbolName);


typedef struct os_dynamicLoad_plugin
{
  os_dynamicLibraryOpen_fn dlp_open;
  os_dynamicLibraryClose_fn dlp_close;
  os_dynamicLibraryGetSymbol_fn dlp_getSymbol;
  os_dynamicLoadLib_fn dlp_loadLib;
} os_dynamicLoad_plugin;

extern struct os_dynamicLoad_plugin *os_dynamicLibPlugin;
extern struct os_dynamicLoad_plugin os_dynamicLibPluginImpl;

#endif
