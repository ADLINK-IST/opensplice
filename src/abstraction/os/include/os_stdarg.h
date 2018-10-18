/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
#ifndef OS_STDARG_H_
#define OS_STDARG_H_

#include <stdarg.h>

#if defined (_WIN32) && _MSC_VER < 1800
/* va_copy is available in Visual Studio 2013 and up */
#define os_va_copy(d,s) ((d)=(s))
#else
#define os_va_copy va_copy
#endif

#endif /* OS_STDARG_H_ */
