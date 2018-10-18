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

/** \file os/int509/code/os_sharedmem.c
 *  \brief Integrity shared memory management
 *
 * Implements shared memory management for integrity.
 * Assumes shared memory is already setup in the integrate file.
 */

#include "../common/code/os_sharedmem_handle.c"

const char *domainName;

void
os_sharedMemoryInit (
    void)
{
    return;
}

void
os_sharedMemoryExit (
    void)
{
    return;
}

os_result
os_sharedMemoryCreate (
    os_sharedHandle sharedHandle,
    os_address size)
{
    domainName =  os_strdup(sharedHandle->name);
    return os_resultSuccess;
}

os_result
os_sharedMemoryGetNameFromId(
    os_sharedHandle sharedHandle,
    char **name)
{
    if(domainName)
    {
        *name =  os_strdup(domainName);
    }
    else
    {
        *name = NULL;
    }
    return os_resultSuccess;
}

os_result
os_sharedMemoryDestroy (
    os_sharedHandle sharedHandle)
{
    return os_resultSuccess;
}

os_result
os_sharedMemoryAttach (
    os_sharedHandle sharedHandle)
{
    assert (sharedHandle != NULL);
    assert (sharedHandle->name != NULL);
    assert (sharedHandle->mapped_address == NULL);

    sharedHandle->mapped_address = sharedHandle->attr.map_address;
    return os_resultSuccess;
}

os_result
os_sharedMemoryDetach (
    os_sharedHandle sharedHandle)
{
    return os_resultSuccess;
}

os_result
os_sharedMemoryDetachUnclean (
    os_sharedHandle sharedHandle)
{
    return os_resultSuccess;
}

void
os_sharedAttrInit (
    os_sharedAttr *sharedAttr)
{
    assert (sharedAttr != NULL);
    sharedAttr->lockPolicy = OS_LOCK_DEFAULT;
    sharedAttr->sharedImpl = OS_MAP_ON_FILE;
    sharedAttr->userCred.uid = 0;
    sharedAttr->userCred.gid = 0;
    sharedAttr->map_address = (void *)0x20000000;
}


os_result
os_sharedSize(
    os_sharedHandle sharedHandle,
    os_address *size)
{
    *size = 0;

    return os_resultSuccess;
}

void
os_shmClientFree(
    os_shmClient client)
{
    return;
}

os_result
os_sharedMemoryWaitForClientChanges(
    os_sharedHandle sharedHandle,
    os_duration maxBlockingTime,
    os_shmClient* changedClients)
{
    ospl_os_sleep(maxBlockingTime);
    return os_resultTimeout;
}

char *
os_findKeyFile(
    const char * name)
{
    return NULL;
}

os_int32
os_destroyKeyFile(
    const char * name)
{
    return 0;
}

os_int32
os_sharedMemorySegmentFree(
    const char * fname)
{
    return 0;
}

/* Dummy state functions as ospl tool not supported on integrity */
/* and spliced only sets the state. */
os_state
os_sharedMemoryGetState(
    os_sharedHandle sharedHandle)
{
    OS_UNUSED_ARG(sharedHandle);
    return OS_STATE_NONE;
}

os_result
os_sharedMemorySetState(
    os_sharedHandle sharedHandle,
    os_state state)
{
    OS_UNUSED_ARG(sharedHandle);
    OS_UNUSED_ARG(state);
    return os_resultSuccess;
}

os_result os_sharedMemoryLock(os_sharedHandle sharedHandle) {
    OS_UNUSED_ARG(sharedHandle);
    return os_resultUnavailable;
}

void os_sharedMemoryUnlock(os_sharedHandle sharedHandle) {
    OS_UNUSED_ARG(sharedHandle);
    return;
}

void
os_sharedMemoryImplDataCreate(
    os_sharedHandle sharedHandle)
{
    OS_UNUSED_ARG(sharedHandle);
}

void
os_sharedMemoryImplDataDestroy(
    os_sharedHandle sharedHandle)
{
    OS_UNUSED_ARG(sharedHandle);
}

