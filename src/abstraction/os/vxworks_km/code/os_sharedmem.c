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
/** \file os/vxworks5.5/code/os_sharedmem.c
 *  \brief vxWorks shared memory management
 *
 * Implements shared memory management for vxWorks by
 * including the POSIX, SVR4 and heap implementation
 */

#define OS_SHAREDMEM_FILE_DISABLE
#define OS_SHAREDMEM_SEG_DISABLE

#include "../posix/code/os_sharedmem_file.c"
#include "../svr4/code/os_sharedmem_seg.c"
#include "../common/code/os_sharedmem_heap.c"
#include "../common/code/os_sharedmem.c"

/** \brief Search an entry by index in the linked list
 *
 * Search an entry in the linked list of created named
 * shared memory entries by \b index. If found return 0
 * else return \b -1;
 *
 * It is assumed that the \b os_smAdminLock mutex is claimed
 * by the calling thread.
 */
int
os_heap_search_entry_by_index (
    const int index, char * name, int * address)
{
    os_sm *sm;
    os_sm *rv = NULL;
    int i=0;

    sm = os_smAdmin;
    while (sm != NULL) {
        if (index == i) {
            rv = sm;
            sm = NULL;
        } else {
            sm = sm->next;
            i++;
        }
    }
    if (rv == NULL){
        return -1;
    } else {
        os_strcpy(name, rv->name);
        *address = (int)(rv->address);
        return 0;
    }
}

void
os_sharedAttrInit (
    os_sharedAttr *sharedAttr)
{
    assert (sharedAttr != NULL);
    sharedAttr->lockPolicy = OS_LOCK_DEFAULT;
    sharedAttr->sharedImpl = OS_MAP_ON_HEAP;
    sharedAttr->userCred.uid = 0;
    sharedAttr->userCred.gid = 0;
    sharedAttr->map_address = (void *)0x0;
}

