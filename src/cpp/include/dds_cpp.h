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
#ifndef DDS_CPP_H
#define DDS_CPP_H

#ifdef __cplusplus
extern "C" {
#endif
#include "os_if.h"

/* if you want to build the C preprocessor as a dll, please define:
#define OSPL_BUILD_CPP_SHARED
*/
#ifdef OSPL_BULD_CPP_SHARED

#ifdef OSPL_BUILD_CPP
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif /* OSPL_BUILD_CPP */
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#else /* !defined OSPL_BULD_CPP_SHARED */

#define OS_API

#endif /* OSPL_BUILD_CPP_SHARED */


#define DEF_CMDLINE 1

extern int fstackdepth;

OS_API void init_preprocess(void);
OS_API void preprocess(FILE *infile, const char *infilename);
OS_API int preprocess_getc(void);
OS_API void Ifile (const char *);
OS_API void define (const char * name, int nargs, unsigned char * repl, int how);

#undef OS_API

#ifdef __cplusplus
}
#endif

#endif /* DDS_CPP_H */
