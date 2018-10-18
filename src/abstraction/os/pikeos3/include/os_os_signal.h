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

#ifndef OS_PIKEOS_SIGNAL_H
#define OS_PIKEOS_SIGNAL_H

#include "../common/include/os_signal.h"

typedef sigset_t		os_os_sigset;
typedef struct sigaction	os_os_sigaction;
typedef int			os_os_signal;
typedef void (*os_os_actionHandler)(int,  siginfo_t *, void *);

#endif /* OS_PIKEOS_SIGNAL_H */
