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

/****************************************************************
 * Implementation for shared memory conforming to               *
 * OpenSplice requirements                                      *
 ****************************************************************/

/** \file os/code/os_sharedmem.c
 *  \brief Shared memory management - create, attach, detach and
 *         destroy shared memory
 *
 * Shared memory provides services for creating, destroying,
 * attaching to, dedatching from named shared memory.
 * After creation by name and a specified required size,
 * the named shared memory can be attached to. Depending on
 * the platform and required implementation, a virtual address
 * must be specified at which the memory must be mapped in during
 * attachment.
 *
 * When a process terminates, it must detached from the named
 * shared memory. When all processes are detached, the named shared
 * memory can be destroyed.
 *
 * The following implementations are available:
 * - OS_MAP_ON_FILE: Uses POSIX services to map shared memory
 * onto a file in the file system (on UNIX like platforms, the
 * scope is system).
 * - OS_MAP_ON_SEG: Uses SVR4 (unnamed) shared memory segments
 * (on UNIX like platforms, the scope is system).
 * - OS_MAP_ON_HEAP: Uses the heap to allocate the shared memory
 * (on all platforms, but the scope on UNIX like platforms is process).
 */

#include "os_sharedmem.h"

/* include OS specific shared memory implementation		*/
#include "code/os_sharedmem.c"

const char*
os_stateImage(os_state state)
{
    char *image;

    switch (state) {
    case OS_STATE_NONE: image = "NONE"; break;
    case OS_STATE_INITIALIZING: image = "INITIALIZING"; break;
    case OS_STATE_OPERATIONAL: image = "OPERATIONAL"; break;
    case OS_STATE_TERMINATING: image = "TERMINATING"; break;
    case OS_STATE_TERMINATED: image = "TERMINATED"; break;
    default: image = "(unknown)"; break;
    }
    return image;
}
