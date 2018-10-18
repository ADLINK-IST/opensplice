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

#ifndef OS_LINUX_SIGNAL_H
#define OS_LINUX_SIGNAL_H

/* Include common header file              */
#include "../common/include/os_signal.h"

#include "signal.h"

typedef sigset_t		os_os_sigset;

typedef struct sigaction	os_os_sigaction;

typedef int			os_os_signal;

typedef void (*os_os_actionHandler)(int,  siginfo_t *, void *);

#endif /* OS_LINUX_SIGNAL_H */
