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
/*******************************************************************
 * Interface definition for all of SPLICE-DDS OS layer functions   *
 *******************************************************************/

/** \file vortex_os.h
 *  \brief Aggregate OS layer include files
 *
 * vortex_os.h aggregates all OS layer include files. By including
 * this file, all type definitions and services are available.
 */

#ifndef OS_OS_H
#define OS_OS_H

#include "os_defs.h"
#include "os_init.h"
#include "os_errno.h"
#include "os_abstract.h"
#include "os_mutex.h"
#include "os_rwlock.h"
#include "os_cond.h"
#include "os_time.h"
#include "os_thread.h"
#include "os_process.h"
#include "os_signalHandler.h"
#include "os_signal.h"
#include "os_sharedmem.h"
#include "os_heap.h"
#include "os_stdlib.h"
#include "os_string.h"
#include "os_library.h"
/* #include "os_if.h"
 * May NEVER be included by this file!
 */

#if defined (__cplusplus)
extern "C" {
#endif

#ifndef OPENSPLICE_MAIN 
#define OPENSPLICE_MAIN(n) int main (int argc, char ** argv)
#endif

#ifndef OPENSPLICE_ENTRYPOINT
#define OPENSPLICE_ENTRYPOINT(n) int n (int argc, char ** argv)
#endif

#ifndef OPENSPLICE_SERVICE_ENTRYPOINT
#define OPENSPLICE_SERVICE_ENTRYPOINT(n, instancename) OPENSPLICE_ENTRYPOINT(n)
#endif

#ifndef OPENSPLICE_ENTRYPOINTCALL
#define OPENSPLICE_ENTRYPOINTCALL(n, argc, argv) n ( ( argc ), ( argv ) )
#endif

#ifndef OPENSPLICE_ENTRYPOINT_DECL
#define OPENSPLICE_ENTRYPOINT_DECL(n) int n (int argc, char ** argv)
#endif


#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* OS_OS_H */
