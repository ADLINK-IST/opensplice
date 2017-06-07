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

/****************************************************************
 * Implementation for thread management conforming to           *
 * OpenSplice requirements                                      *
 ****************************************************************/

/** \file os/code/os_thread.c
 *  \brief Thread management - create threads
 *
 * A thread is a unit of execution. Threads can compete with each
 * other within the context of a process for the CPU (unbounded
 * threads, scheduling is on process scope) or threads can compete
 * with other processes or threads within the same or other processes
 * for the CPU (bounded threads, scheduling is on system scope).
 * A thread has its own stack, but shares the address space with
 * other threads within a process.
 *
 * \par Thread model on Solaris
 * Solaris supports bouded and unbounded threads, the OS layer
 * will only support bouded threads
 *
 * \par Thread model on Linux
 * Linux only supports bounded threads
 */

#include "os_thread.h"

/* include OS specific thread implementation			*/
#include "code/os_thread.c"
