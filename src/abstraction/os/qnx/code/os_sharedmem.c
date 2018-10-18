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

/** \file os/qnx/code/os_sharedmem.c
 *  \brief QNX shared memory management
 *
 * Implements shared memory management for QNX by
 * providing a POSIX implementation and importing
 * the common heap implementation.
 */

#define OS_SHAREDMEM_SEG_DISABLE

#include "../qnx/code/os_mkstemp.c"
#include "../posix/code/os_sharedmem_file.c"
#include "../common/code/os_sharedmem_heap.c"
#include "../svr4/code/os_sharedmem_seg.c"
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
    sharedAttr->map_address = (void *)0x60000000;
    return os_resultSuccess;
}

