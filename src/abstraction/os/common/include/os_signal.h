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

#ifndef OS_COMMON_SIGNAL_H
#define OS_COMMON_SIGNAL_H

#include <signal.h>

#define OS_SIGTERM	SIGTERM
#define OS_SIGINT	SIGINT
#define OS_SIGHUP	SIGHUP
#define OS_SIGCHLD	SIGCHLD
#define OS_SIGKILL	SIGKILL
#define OS_SIGPIPE	SIGPIPE
#define OS_SIGALRM	SIGALRM
#define OS_SIGUSR1	SIGUSR1
#define OS_SIGUSR2	SIGUSR2
#define OS_SIGPOLL	SIGPOLL
#define OS_SIGVTALRM	SIGVTALRM
#define OS_SIGPROF	SIGPROF

#endif /* OS_COMMON_SIGNAL_H */
