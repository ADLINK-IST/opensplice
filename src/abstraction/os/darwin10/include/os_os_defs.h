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
#ifndef OS_DARWIN_DEFS_H
#define OS_DARWIN_DEFS_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "os_os_abstract.h"
#include "os_os_types.h"

#include "os_os_alloca.h"
#include "os_os_mutex.h"
#include "os_os_rwlock.h"
#include "os_os_cond.h"
#include "os_os_thread.h"
#include "os_os_library.h"
#include "os_os_semaphore.h"
#include "os_os_signal.h"
#include "os_os_stdlib.h"
#include "os_os_process.h"
#include "os_os_if.h"
#include "os_os_utsname.h"

#define OS_SOCKET_USE_FCNTL 1
#define OS_SOCKET_USE_IOCTL 0
#define OS_HAS_UCONTEXT_T
#define OS_HAS_TSD_USING_THREAD_KEYWORD 1

#if defined (__cplusplus)
}
#endif

#endif /* OS_DARWIN_DEFS_H */

