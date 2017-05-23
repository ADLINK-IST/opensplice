#include <stdio.h>
#include "os_defs.h"
#include "os_EntryPoints.h"
#include "os_cfg.h"
#include "os_heap.h"

static const char fileURI[] = "file://";
static const size_t fileURILen = sizeof(fileURI)-1;
static const char osplcfgURI[] = "osplcfg://";
static const size_t osplcfgURILen = sizeof(osplcfgURI)-1;

os_cfg_file *os_fileOpenURI(const char *uri)
{
   if ( strncmp(uri, fileURI, fileURILen ) == 0 )
   {
      return(fopen(&uri[fileURILen], "r"));
   }
   else
   {
      return(NULL);
   }
}


os_cfg_handle *os_fileReadURI(const char *uri)
{
   os_cfg_file *cfg_file = os_fileOpenURI(uri);
   if ( cfg_file )
   {
      size_t count, r;
      os_cfg_handle *handle = (os_cfg_handle *)os_malloc(sizeof *handle);

      /* Determine worst-case size of the file-contents. On Windows ftell will
       * provide the number of characters available (including \r\n), but fread
       * will replace \r\n by \n, so fread may actually return less. */
      fseek(cfg_file, 0, SEEK_END);
      count = (size_t)ftell(cfg_file);
      fseek(cfg_file, 0, SEEK_SET);

      handle->ptr = os_malloc(count + 2);

      /* On Windows fread may return less due to \r\n replacements, so NULL-terminate
       * the resulting string on position r instead of count. */
      r = fread(handle->ptr, 1, count, cfg_file);

      /* Double null-terminate the buffer should anyone which to use yy_scan_buffer
       * on the result. */
      handle->ptr[r] = '\0';
      handle->ptr[r+1] = '\0';

      handle->size = r+2;
      fclose(cfg_file);

      handle->isStatic=0;
      return handle;
   }
   else
   {
     return NULL;
   }
}


os_cfg_handle *os_cfgRead(const char *uri)
{
   const os_URIListNode *current;
   int i;

   if ( uri == NULL || uri[0] == '\0' )
   {
       return NULL;
   }

   if ( strncmp(uri, fileURI, fileURILen ) == 0 )
   {
      return os_fileReadURI(uri);
   }
   else
   {
     for (i=0; (current=&os_cfg_cfgs[i])->uri != NULL; i++ )
     {
        if ( strncmp(current->uri, osplcfgURI, osplcfgURILen ) == 0 )
        {
           os_cfg_handle *handle = (os_cfg_handle *)os_malloc(sizeof *handle);
           handle->ptr = current->config;
           handle->isStatic=1;
           handle->size = current->size;
           return handle;
        }
     }
     return NULL;
   }
}

void os_cfgRelease(os_cfg_handle *cfg)
{
   if( ! cfg->isStatic )
   {
      os_free(cfg->ptr);
   }
   os_free(cfg);
}
