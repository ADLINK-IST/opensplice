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
/* $Id$ */
/* Definition for Win32/GCC Export directives.
 * This file is generated automatically by generate_export_file.pl -n LIB4CPLUGIN
 * ------------------------------ */

#ifndef LIB4CPLUGIN_EXPORT_H
#define LIB4CPLUGIN_EXPORT_H

#include /* $(OSPL_HOME)/include/sys */ "os_if.h"

#if !defined (LIB4CPLUGIN_HAS_DLL)
#  define LIB4CPLUGIN_HAS_DLL 1
#endif /* ! LIB4CPLUGIN_HAS_DLL */

#if defined (LIB4CPLUGIN_HAS_DLL) && (LIB4CPLUGIN_HAS_DLL == 1)
#  if defined (LOG4CPLUGIN_BUILD_DLL)
#    define LIB4CPLUGIN_Export OS_API_EXPORT
#  else /* ! LIB4CPLUGIN_BUILD_DLL */
#    define LIB4CPLUGIN_Export OS_API_IMPORT
#  endif /* ! LIB4CPLUGIN_BUILD_DLL */
#else /* ! LIB4CPLUGIN_HAS_DLL == 1 */
#  define LIB4CPLUGIN_Export
#endif /* ! LIB4CPLUGIN_HAS_DLL == 1 */

#endif /* LIB4CPLUGIN_EXPORT_H */

// End of auto generated file.
