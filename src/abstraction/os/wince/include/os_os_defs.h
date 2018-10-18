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

#ifndef OS_WINCE_DEFS_H
#define OS_WINCE_DEFS_H

#if defined (__cplusplus)
extern "C" {
#endif

// Suppress spurious 'child' : inherits 'parent::member' via dominance warnings
#if defined (_MSC_VER)
    #pragma warning(disable:4250)
#endif /* _MSC_VER */

#define OS_SOCKET_USE_FCNTL 0
#define OS_SOCKET_USE_IOCTL 1
#define OS_HAS_UCONTEXT_T

#include "os_os_if.h"
#include "os_os_types.h"

#define EPOCH_DIFF			116444736000000000		// 100-nanoseconds between January 1, 1601 and January 1, 1970
#define	UNITY_DIFF			10000000			// 100*10^9 nanoseconds diff betwee FILETIME and UNIXTIME
#define IP_MULTICAST_LOOPBACK_NOTSUPPORTED                              // This define is used to enable filtering of own messages by the networking service as IP_MULTICAST_LOOP isn't supported on Windows CE.

/* Windows CE doesn't have abort, so use exit instead */
#define abort() exit(1)

#include "os_os_abstract.h"
#include "os_os_alloca.h"
#include "os_os_mutex.h"
#include "os_os_cond.h"
#include "os_os_thread.h"
#include "os_os_library.h"
#include "os_os_semaphore.h"
#include "os_os_signal.h"
#include "os_os_stdlib.h"
#include "os_os_process.h"
#include "os_os_utsname.h"

#include "os_os_rwlock.h"


#if defined (__cplusplus)
}
#endif

#endif /* OS_WINCE_DEFS_H */
