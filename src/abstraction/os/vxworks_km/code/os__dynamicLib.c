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
#include <vxWorks.h>
#include <elf.h>
#include <unistd.h>
#include <loadLib.h>
#include <unldLib.h>
#include <symLib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ioLib.h>
#include <sysSymTbl.h>
#include "os_library.h"
#include "os_stdlib.h"
#include "os_report.h"
#include "os_heap.h"
#include "include/os_dynamicLib_plugin.h"
#include "os_errno.h"

static os_library
os_dynamicLibraryOpen(
    const char *name,
    os_libraryAttr *attr)
{
    MODULE_ID library;
    int fd=0;
    os_library osl = NULL;
    char libName[64];
    
    if (name && (strlen(name) > 0)) {
        if(attr->autoTranslate == OS_TRUE){
          if ( strlen(name)+7 < sizeof(libName)) {
             sprintf(libName, "lib%s.so", name);
             fd = open(libName, O_RDONLY, 0);
          } else {
             OS_REPORT(OS_ERROR, "os_libraryOpen", 0, "library file name too long");
          } 
        } else {
            fd = open(name, O_RDONLY, 0);
        }
        if (fd >= 0) {
            library = loadModule(fd, attr->flags);
            
            if (!library) {
                OS_REPORT(OS_ERROR, "os_libraryOpen", 0,
                    "loadModule: %s, error code %d", name, os_getErrno());
            }
            else
            {
               osl = (os_library)os_malloc(sizeof(*osl));
               osl->l.library = library;
               osl->isStatic = 0;
            }
            close (fd);
        } else {
            OS_REPORT(OS_ERROR, "os_libraryOpen", 0,
                "open: %s, error code %d", name, os_getErrno());
        }
    }
    return (void *)osl;
}

static os_result
os_dynamicLibraryClose(
    os_library library)
{
    os_result result;
    if (library != NULL && library->l.library) {
        if (unldByModuleId(library->l.library, 0) != OK) {
            OS_REPORT(OS_ERROR, "os_libraryClose", 0,
                "unldByModuleId error: %s", os_getErrno());
            result = os_resultFail;
        } else {
            os_free( library );
            result = os_resultSuccess;
        }
    } else {
        result = os_resultInvalid;
    }
    return result;
}

static os_symbol
os_dynamicLibraryGetSymbol(
    os_library library,
    const char *symbolName)
{
    os_symbol symbol = NULL;
    SYM_TYPE pType;
 
    assert(library);
    assert(library->l.library);
    assert(library->isStatic == 0);
    assert(symbolName);

    if ((symbolName != NULL) && (strlen(symbolName) != 0)) {
        if (symFindByName(sysSymTbl, (char *)symbolName, (char **)&symbol, &pType) == ERROR) {
            OS_REPORT(OS_ERROR, "os_libraryGetSymbol", 0,
                "symFindByName: %s, error code %d", symbolName, os_getErrno());
        }
    } 
    return symbol;
}

static os_result
os_procLoad(
    char *executable_file,
    void **startRoutine)
{
    Elf32_Ehdr exec;
    int        fd;
    char       *locatedexe;
    char       *ptext = LD_NO_ADDRESS;
    char       *pdata = LD_NO_ADDRESS;
    char       *pbss = LD_NO_ADDRESS;
    int        task_symbols;
    char       *exeNameWithPostfix = NULL;
    size_t     exe_name_len;
    const char *dkm_postfix = ".out";
    const size_t dkm_postfix_len = strlen(dkm_postfix);
    exe_name_len = strlen(executable_file);
    locatedexe = os_locate(executable_file, OS_ROK);
    if ( locatedexe == NULL
	 && exe_name_len > dkm_postfix_len
	 && strcmp(&executable_file[exe_name_len-dkm_postfix_len],
		   dkm_postfix) != 0 ) {
        exeNameWithPostfix=os_malloc(strlen(executable_file)+1+dkm_postfix_len);
        sprintf(exeNameWithPostfix, "%s%s", executable_file, dkm_postfix );
        locatedexe = os_locate(exeNameWithPostfix, OS_ROK);
        os_free(exeNameWithPostfix);
    }
	
    if ( locatedexe == NULL ) {
      OS_REPORT(OS_WARNING, "os_procCreate", 1,
		  "os_procLoad : could not locate module on PATH (%s)", executable_file);
      return (os_resultInvalid);
    } else {

       fd = open(locatedexe, O_RDONLY, 0);
       if (fd == ERROR) {
           OS_REPORT(OS_WARNING, "os_procCreate", 1,
               "os_procLoad : open failed with error %d (%s)", os_getErrno(), executable_file);
           return (os_resultInvalid);
       }
       if (read(fd, (char *)&exec, sizeof(exec)) != sizeof(exec)) {
           OS_REPORT(OS_WARNING, "os_procCreate", 1,
               "os_procLoad : read failed with error %d (%s)", os_getErrno(), executable_file);
           close(fd);
           return (os_resultInvalid);
       }
       if (lseek(fd, 0, L_SET) != 0) {
           OS_REPORT(OS_WARNING, "os_procCreate", 1,
               "os_procLoad : lseek failed with error %d (%s)", os_getErrno(), executable_file);
           close(fd);
           return (os_resultInvalid);
       }
       task_symbols = GLOBAL_SYMBOLS; /* ??? NO_SYMBOLS */
#ifndef _WRS_VX_SMP
       /* Not available in SMP, so no protection against loading simultaneously */
       taskLock();
#endif
       if ((int)loadModuleAt(fd, task_symbols, &ptext, &pdata, &pbss) == ERROR) {
	 OS_REPORT(OS_WARNING, "os_procCreate", 1,
		     "os_procLoad : loadModuleAt failed with error %d (%s)", os_getErrno(), executable_file);
	 close(fd);
#ifndef _WRS_VX_SMP
	 taskUnlock();
#endif
	 return (os_resultInvalid);
       }
#ifndef _WRS_VX_SMP
       taskUnlock();
#endif
       close (fd);
       *startRoutine = (void *)(((unsigned long)ptext + (unsigned long)exec.e_entry));
       os_free(locatedexe);
    }
    return (os_resultSuccess);
}

/** \brief Helper function for os_procCreate
 *
 * checkEntryPointPreLoaded attempts to locate the expected
 * entrypoint symbol based on the executable name.
 * For VxWorks with a single address space, each service entrypoint
 * is derived by using the OPENSPLICE_ENTRYPOINT macro, which will create
 * an entrypoint name "ospl_x".  If available, this symbol will be
 * used as the entrypoint during os_procCreate and avoid the
 * potentially slow loadModuleAt call.  It gives the user the option
 * to preload all components using RAM, flash, or others, when the
 * dynamic loading of symbols could be slow (especially as seen with
 * the Target Server File System connection during testing).
 */

static os_boolean
checkEntryPointPreLoaded(
    const char *executableName,
    void **startRoutine)
{
    SYM_TYPE ptype;
    char * entryPoint = NULL;
    os_boolean result = FALSE;

    entryPoint = os_malloc (strlen (executableName) + 6);
    strcpy (entryPoint,"ospl_");
    strcat (entryPoint,executableName);

    OS_REPORT(OS_INFO, "checkEntryPointPreLoaded", 1,
                "For %s checking for preloaded symbol (%s)",
                executableName, entryPoint);

    if (symFindByName(sysSymTbl, entryPoint,(char**)startRoutine,&ptype) == OK)
    {
        OS_REPORT(OS_INFO, "checkEntryPointPreLoaded", 0, "Symbol (%s) already loaded", entryPoint);
        result = TRUE;
    }
    else
    {
        OS_REPORT(OS_INFO, "checkEntryPointPreLoaded", 0, "Symbol (%s) not yet loaded", entryPoint);
    }

    os_free (entryPoint);

    return result;
}

static void *
os_dynamicLoadLib( const char *executable_file)
{
    void *startRoutine=NULL;

    /* Optimisation to use the preloaded entrypoint symbol (if it exists
     * yet) for the executable.  If it has not yet been loaded, fall back
     * to loadModuleAt within os_procLoad which will load the component and
     * locate the entrypoint.
     * We have noted that loadModuleAt can be very slow, especially when
     * using the 'Target Server File System', which is known to have slow I/O.
     */
    if (checkEntryPointPreLoaded (executable_file, &startRoutine)) {
        OS_REPORT(OS_INFO, "os__dynamicLoadProc", 0,
                    "Using preloaded entry point for %s", executable_file);
    }
    else if (os_procLoad((char *)executable_file, &startRoutine) 
             != os_resultSuccess) {
        OS_REPORT(OS_WARNING, "os__dynamicLoadProc", 1,
                    "Unable to load %s",
                    executable_file);
    }
    return ( startRoutine );
}


struct os_dynamicLoad_plugin os_dynamicLibPluginImpl =
{
   os_dynamicLibraryOpen,
   os_dynamicLibraryClose,
   os_dynamicLibraryGetSymbol,
   os_dynamicLoadLib
};

