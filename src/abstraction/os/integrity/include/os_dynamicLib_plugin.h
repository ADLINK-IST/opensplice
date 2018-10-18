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
