/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
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
