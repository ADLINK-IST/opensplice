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
#ifndef OS_VXWORKS_KM_OS_OS_DEFS_H
#define OS_VXWORKS_KM_OS_OS_DEFS_H

#include <vxWorks.h>
#include <version.h>

/* regex2.h on vxworks 6.9 defined NDEBUG, so sort out afterwards! */
/* Added WR case 00052607, they have added defect VXW6-85317 */
#ifdef NDEBUG
#define OSPL_NDEBUG_FIX
#endif
#include <regex2.h>
#ifndef OSPL_NDEBUG_FIX
#undef NDEBUG
#endif
#undef OSPL_NDEBUG_FIX

#if defined (  _WRS_VXWORKS_MAJOR ) &&  ( _WRS_VXWORKS_MAJOR >= 6 )
#define VXWORKS_GTE_6
#if ( _WRS_VXWORKS_MINOR >=9 ) || ( WRS_VXWORKS_MAJOR > 6 )
#define VXWORKS_GTE_6_9
#endif
#endif

#define OSPL_NO_VMEM

#define OS_SOCKET_USE_FCNTL 0
#define OS_SOCKET_USE_IOCTL 1

#define OS_HAS_UCONTEXT_T

#include "os_os_types.h"

#include "os_os_utsname.h"
#include "os_os_abstract.h"
#include "os_os_alloca.h"
#include "os_os_if.h"
#include "os_os_library.h"
#include "os_os_process.h"
#include "os_os_signal.h"
#include "os_os_stdlib.h"
#include "os_os_thread.h"

#ifdef VXWORKS_GTE_6
#include <../posix/include/os_os_cond.h>
#include <../posix/include/os_os_mutex.h>
#include <../posix/include/os_os_semaphore.h>
#else
#include "os_os_cond.h"
#include "os_os_mutex.h"
#include "os_os_semaphore.h"
#endif

#include "os_os_rwlock.h"
#include "os_dynamicLib_plugin.h"
#include "os_os_library.h"

#endif /* OS_VXWORKS_KM_OS_OS_DEFS_H */
