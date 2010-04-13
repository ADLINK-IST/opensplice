/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

/** \file os/darwin/code/os_sharedmem.c
 *  \brief Darwin shared memory management
 *
 * Implements shared memory management for Darwin by
 * including the POSIX, SVR4 and heap implementation
 */

#include <assert.h>

#include <../posix/code/os_sharedmem_file.c>
#include <../svr4/code/os_sharedmem_seg.c>
#include <../common/code/os_sharedmem_heap.c>
#include <../common/code/os_sharedmem.c>

os_result
os_sharedAttrInit (
    os_sharedAttr *sharedAttr)
{
    assert (sharedAttr != NULL);
    sharedAttr->lockPolicy = OS_LOCK_DEFAULT;
    sharedAttr->sharedImpl = OS_MAP_ON_SEG;
    /*sharedAttr->sharedImpl = OS_MAP_ON_FILE;*/
    sharedAttr->userCred.uid = 0;
    sharedAttr->userCred.gid = 0;
    /*sharedAttr->map_address = (void *)0x300000000;*/
    sharedAttr->map_address = (void *)0x60000000;
    return os_resultSuccess;
}
