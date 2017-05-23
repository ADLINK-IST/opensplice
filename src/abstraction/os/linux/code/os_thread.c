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

/** \file
 *  \brief Linux thread management
 *
 * Implements thread management for Linux
 * by including the POSIX implementation
 */

#ifndef OS_HAS_NO_SET_NAME_PRCTL
#include <sys/prctl.h>
#ifndef PR_SET_NAME
#define OS_HAS_NO_SET_NAME_PRCTL
#endif
#endif
#include "os__thread.h"
#include "../posix/code/os_thread.c"
#include "code/os_thread_attr.c"
