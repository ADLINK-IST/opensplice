/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
