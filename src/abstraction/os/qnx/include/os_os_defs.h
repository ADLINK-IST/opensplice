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

#ifndef OS_QNX_DEFS_H
#define OS_QNX_DEFS_H

#define OS_SOCKET_USE_FCNTL 1
#define OS_SOCKET_USE_IOCTL 0
#define OS_HAS_UCONTEXT_T

#include "os_os_if.h"
#include "os_os_types.h"

#include "os_os_abstract.h"
#include "os_os_alloca.h"
#include "os_os_process.h"
#include "os_os_signal.h"
#include "os_os_utsname.h"

#include <../posix/include/os_os_cond.h>
#include <../posix/include/os_os_library.h>
#include <../posix/include/os_os_mutex.h>
#include <../posix/include/os_os_semaphore.h>
#include <../posix/include/os_os_thread.h>
#include <../posix/include/os_os_stdlib.h>

#include "os_os_rwlock.h"

#endif /* OS_QNX_DEFS_H */
