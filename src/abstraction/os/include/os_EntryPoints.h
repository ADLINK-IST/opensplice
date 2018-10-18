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

#ifndef OS_ENTRYPOINTS_H
#define OS_ENTRYPOINTS_H

typedef struct os_entryPoint
{
  const char *symname;
#ifdef _WRS_KERNEL
  int (*entrypoint)(char *args);
#else
  int (*entrypoint)(int argc, char **argv);
#endif
} os_entryPoint;

typedef struct os_librarySymbols
{
  const char *execname;
  os_entryPoint *entryPoints;
} os_librarySymbols;


extern os_librarySymbols os_staticLibraries[];

typedef struct os_URIListNode
{
   const char * const uri;
   char * const config; /* double null terminated to use with */
                            /* yy_scan_buffer */
   const unsigned long size;
} os_URIListNode;

extern const os_URIListNode os_cfg_cfgs[];

#endif
