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

/** \file os/linux/code/os_sharedmem.c
 *  \brief Linux 2.6 shared memory management
 *
 * Implements shared memory management for Linux by
 * including the POSIX, SVR4 and heap implementation
 */

#include <assert.h>

#define OS_SHAREDMEM_SEG_DISABLE

size_t getpagesize ()
{
    return (sysconf(_SC_PAGESIZE));
}

#include "../posix/code/os_sharedmem_file.c"
#include "../svr4/code/os_sharedmem_seg.c"
#include "../common/code/os_sharedmem_heap.c"
#include "../common/code/os_sharedmem.c"

void
os_sharedAttrInit (
    os_sharedAttr *sharedAttr)
{
    assert (sharedAttr != NULL);
    sharedAttr->lockPolicy = OS_LOCK_DEFAULT;
    sharedAttr->sharedImpl = OS_MAP_ON_FILE;
    sharedAttr->userCred.uid = 0;
    sharedAttr->userCred.gid = 0;
#ifdef __x86_64__
    sharedAttr->map_address = (void *)0x140000000;
#else
    sharedAttr->map_address = (void *)0x20000000;
#endif
}
