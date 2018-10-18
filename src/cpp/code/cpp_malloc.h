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
#ifndef _CPP_MALLOC_
#define _CPP_MALLOC_

#include "stdincs.h"
#include "os_heap.h"

#ifdef __cplusplus
extern "C"
{
#endif

   extern char *copyofstr (const char *);
   extern char *copyofblk (const char *, int);
   extern void err_head (void);
   extern void Check_malloc (const char *);

#ifdef __cplusplus
}
#endif

#define NEW(type) ((type *) os_malloc(sizeof(type)))
#define OLD(x) os_free((char *) x)
#define check_os_malloc(ptr) Check_malloc((const char *)(ptr))

#endif
