/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef IDL_DLL_H
#define IDL_DLL_H

#include "os_defs.h"

/* Define the macro's used in the template files
 * to be replaced by actual values
 */
#define IDL_DLL_TMPLMACRO_MACRO_NAME  "DLL_IMPORTEXPORT"
#define IDL_DLL_TMPLMACRO_HEADER_NAME "DLL_IMPORTEXPORT_INCLUDE"

os_int32       idl_dllSetOption(const char *option);
const os_char *idl_dllGetMacro (void);
const os_char *idl_dllGetHeader(void);
const os_char * idl_dllGetHeaderFile(void);
void  idl_dllInitialize(void);
void  idl_dllExit(void);

#endif /* IDL_DLL_H */
