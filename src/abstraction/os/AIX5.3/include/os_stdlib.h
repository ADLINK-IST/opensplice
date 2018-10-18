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
#ifndef OS_AIX_STDLIB_H
#define OS_AIX_STDLIB_H

#include <limits.h>

#if defined (__cplusplus)
extern "C" {
#endif


#ifndef MAXHOSTNAMELEN
/* taken from limits.h */
#define MAXHOSTNAMELEN HOST_NAME_MAX
#endif

#include "../posix/include/os_stdlib.h"

#if defined (__cplusplus)
}
#endif

#endif /* OS_AIX_STDLIB_H */
