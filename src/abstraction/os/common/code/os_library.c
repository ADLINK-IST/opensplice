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
#include "os_report.h"
#include "os_heap.h"
#include "include/os_dynamicLib_plugin.h"
#include <string.h>

void
os_libraryAttrInit( os_libraryAttr *attr )
{
   attr->flags = OS_LOAD_GLOBAL_SYMBOLS;
   attr->autoTranslate = OS_TRUE;
   attr->staticLibOnly = OS_FALSE;
}

os_library
os_libraryOpen( const char *name, os_libraryAttr *attr)
{
   os_library handle = NULL;

   if(name && (strlen(name) > 0))
   {
      os_librarySymbols *ls;
      for ( ls = &os_staticLibraries[0]; ls->execname; ls++ )
      {
         if ( !strcmp( ls->execname, name ) )
         {
            handle = os_malloc( sizeof( *handle ));
            handle->l.entryPoints = ls->entryPoints;
            handle->isStatic = 1;
            break;
         }
      }
   }

   if ( (handle == NULL && os_dynamicLibPlugin != NULL )
        && ( attr == NULL || !attr->staticLibOnly ) )
   {
      handle = os_dynamicLibPlugin->dlp_open( name, attr );
   }

   if ( !handle && ( attr == NULL || !attr->staticLibOnly ) )
   {
      OS_REPORT (OS_ERROR, "os_libraryOpen", 0,
                   "no entry in os_entrypoint found for %s", name ? name : "" );
   }
   return handle;
}

os_result
os_libraryClose( os_library library )
{
   if ( !library->isStatic && os_dynamicLibPlugin != NULL )
   {
      return( os_dynamicLibPlugin->dlp_close( library ) );
   }
   else
   {
      os_free( library );
      return os_resultSuccess;
   }
}

os_symbol
os_libraryGetSymbol( os_library library, const char *symbolName )
{
   os_symbol symbol = NULL;

   assert (library);
   assert (symbolName);

   if ( !library->isStatic && os_dynamicLibPlugin != NULL )
   {
      symbol = os_dynamicLibPlugin->dlp_getSymbol( library, symbolName );
   }
   else
   {
      os_entryPoint *ep;
      for ( ep = library->l.entryPoints; ep->symname != NULL; ep++ )
      {
         if ( !strcmp( ep->symname, symbolName ) )
         {
            symbol = ep->entrypoint;
            break;
         }
      }
   }

   if (!symbol)
   {
       OS_REPORT (OS_ERROR, "os_libraryGetSymbol", 0,
                    "no entry in os_entrypoint found for symbol \"%s\"", symbolName ? symbolName : "" );
   }

   return symbol;
}
