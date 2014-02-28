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
/** \file os/common/code/os_sharedmem.c
 *  \brief common shared memory implementation
 *
 * Implements shared memory for UNIX like platyforms and knows
 * three types of implementation:
 * - POSIX shared objects
 * - SVR4 shared memory segments (IPC)
 * - Heap
 */

#include "os_heap.h"
#include <assert.h>

#include "../common/code/os_sharedmem_handle.c"

void
os_sharedMemoryInit(void)
{
    os_heap_sharedMemoryInit();
    return;
}

void
os_sharedMemoryExit(void)
{
    os_heap_sharedMemoryExit();
    return;
}

os_result
os_sharedMemoryCreate(
    os_sharedHandle sharedHandle,
    os_address size)
{
    os_result result = os_resultFail;

    assert(sharedHandle != NULL);
    assert(sharedHandle->name != NULL);

    switch (sharedHandle->attr.sharedImpl) {
    case OS_MAP_ON_FILE:
        result = os_posix_sharedMemoryCreate(sharedHandle->name, &sharedHandle->attr, size, sharedHandle->id);
    break;
    case OS_MAP_ON_SEG:
        result = os_svr4_sharedMemoryCreate(sharedHandle->name, &sharedHandle->attr, size, sharedHandle->id);
    break;
    case OS_MAP_ON_HEAP:
        result = os_heap_sharedMemoryCreate(sharedHandle->name, &sharedHandle->attr, size, sharedHandle->id);
    break;
    }
    return result;
}

os_result
os_sharedMemoryGetNameFromId(
    os_sharedHandle sharedHandle,
    char **name)
{
    os_result result = os_resultFail;

    assert(sharedHandle != NULL);
    switch (sharedHandle->attr.sharedImpl) {
    case OS_MAP_ON_FILE:
        result = os_posix_sharedMemoryGetNameFromId(sharedHandle->id, name);
    break;
    case OS_MAP_ON_SEG:
        result = os_svr4_sharedMemoryGetNameFromId(sharedHandle->id, name);
    break;
    case OS_MAP_ON_HEAP:
        result = os_heap_sharedMemoryGetNameFromId(sharedHandle->id, name);
    break;
    }
    return result;
}

os_result
os_sharedMemoryDestroy(
    os_sharedHandle sharedHandle)
{
    os_result result = os_resultFail;

    assert(sharedHandle != NULL);
    assert(sharedHandle->name != NULL);
    switch (sharedHandle->attr.sharedImpl) {
    case OS_MAP_ON_FILE:
        result = os_posix_sharedMemoryDestroy(sharedHandle->name);
    break;
    case OS_MAP_ON_SEG:
        result = os_svr4_sharedMemoryDestroy(sharedHandle->name);
    break;
    case OS_MAP_ON_HEAP:
        result = os_heap_sharedMemoryDestroy(sharedHandle->name);
    break;
    }
    return result;
}

os_result
os_sharedMemoryAttach(
    os_sharedHandle sharedHandle)
{
    os_result result = os_resultFail;

    assert(sharedHandle != NULL);
    assert(sharedHandle->name != NULL);
    assert(sharedHandle->mapped_address == NULL);
    switch (sharedHandle->attr.sharedImpl) {
    case OS_MAP_ON_FILE:
        result = os_posix_sharedMemoryAttach (sharedHandle->name, &sharedHandle->attr, &sharedHandle->mapped_address);
    break;
    case OS_MAP_ON_SEG:
        result = os_svr4_sharedMemoryAttach (sharedHandle->name, &sharedHandle->attr, &sharedHandle->mapped_address);
    break;
    case OS_MAP_ON_HEAP:
        result = os_heap_sharedMemoryAttach (sharedHandle->name, &sharedHandle->mapped_address);
    break;
    }
    return result;
}

os_result
os_sharedMemoryDetach(
    os_sharedHandle sharedHandle)
{
    os_result result = os_resultFail;

    assert(sharedHandle != NULL);
    assert(sharedHandle->name != NULL);

    switch (sharedHandle->attr.sharedImpl) {
    case OS_MAP_ON_FILE:
        assert(sharedHandle->mapped_address != NULL);
        result = os_posix_sharedMemoryDetach (sharedHandle->name, sharedHandle->mapped_address);
    break;
    case OS_MAP_ON_SEG:
            assert(sharedHandle->mapped_address != NULL);
        result = os_svr4_sharedMemoryDetach (sharedHandle->name, sharedHandle->mapped_address);
    break;
    case OS_MAP_ON_HEAP:
        /* sharedHandle->mapped_address may be 0 in heap configuration */
        result = os_heap_sharedMemoryDetach (sharedHandle->name, sharedHandle->mapped_address);
    break;
    }
    if (result == os_resultSuccess) {
	    sharedHandle->mapped_address = NULL;
    }
    return result;
}

os_result
os_sharedSize(
    os_sharedHandle sharedHandle,
    os_address *size)
{
    os_result result = os_resultFail;

    assert(sharedHandle != NULL);
    assert(sharedHandle->name != NULL);

    switch (sharedHandle->attr.sharedImpl) {
    case OS_MAP_ON_FILE:
        assert(sharedHandle->mapped_address != NULL);
        result = os_posix_sharedSize(sharedHandle->name, size);
    break;
    case OS_MAP_ON_SEG:
        assert(sharedHandle->mapped_address != NULL);
        result = os_svr4_sharedSize(sharedHandle->name, size);
    break;
    case OS_MAP_ON_HEAP:
        /* sharedHandle->mapped_address may be 0 in heap configuration */
        result = os_heap_sharedSize(sharedHandle->name, size);
    break;
    default:
    break;
    }
    return result;
}

char *
os_findKeyFile(
    const char * name)
{
     char* result;
     os_sharedAttr shmAttr;
     os_sharedAttrInit(&shmAttr);
     result = NULL;

     switch (shmAttr.sharedImpl) {
     case OS_MAP_ON_FILE:
         result = os_posix_findKeyFile(name);
     break;
     case OS_MAP_ON_SEG:
         result = os_svr4_findKeyFile(name);
     break;
     case OS_MAP_ON_HEAP:
         result = NULL;
     break;
     }
     return result;
}

char *
os_findKeyFileByNameAndId(
    const char * name,
    const os_int32 id)
{
     char* result;
     os_sharedAttr shmAttr;
     os_sharedAttrInit(&shmAttr);
     result = NULL;

     switch (shmAttr.sharedImpl) {
     case OS_MAP_ON_FILE:
         result = os_posix_findKeyFileByIdAndName(id, name);
     break;
     case OS_MAP_ON_SEG:
         result = os_svr4_findKeyFileByNameAndId(name, id);
     break;
     case OS_MAP_ON_HEAP:
         result = NULL;
     break;
     }
     return result;
}

os_int32
os_destroyKeyFile(
    const char * name)
{
     os_int32 result;
     os_sharedAttr shmAttr;
     os_sharedAttrInit(&shmAttr);
     result = 0;

     switch (shmAttr.sharedImpl) {
     case OS_MAP_ON_FILE:
         result = os_posix_destroyKeyFile(name);
     break;
     case OS_MAP_ON_SEG:
         result = os_svr4_destroyKeyFile(name);
     break;
     case OS_MAP_ON_HEAP:
         result = 0;
     break;
     }
     return result;
}

os_int32
os_destroyKey(
    const char * name)
{
     os_int32 result;
     os_sharedAttr shmAttr;
     os_sharedAttrInit(&shmAttr);
     result = 0;

     switch (shmAttr.sharedImpl) {
     case OS_MAP_ON_FILE:
         result = os_posix_destroyKey(name);
     break;
     case OS_MAP_ON_SEG:
         result = os_svr4_destroyKey(name);
     break;
     case OS_MAP_ON_HEAP:
         result = 0;
     break;
     }
     return result;
}

os_int32
os_sharedMemoryListDomainNames(
    os_iter nameList)
{
     os_int32 result;
     os_sharedAttr shmAttr;
     os_sharedAttrInit(&shmAttr);
     result = 0;

     switch (shmAttr.sharedImpl) {
     case OS_MAP_ON_FILE:
         result = os_posix_listDomainNames(nameList);
     break;
     case OS_MAP_ON_SEG:
         result = os_svr4_listDomainNames(nameList);
     break;
     case OS_MAP_ON_HEAP:
         result = 0;
     break;
     }
     return result;
}

os_int32
os_sharedMemoryListDomainNamesFree(
    os_iter nameList)
{
     os_int32 result;
     os_sharedAttr shmAttr;
     os_sharedAttrInit(&shmAttr);
     result = 0;

     switch (shmAttr.sharedImpl) {
     case OS_MAP_ON_FILE:
         result = os_posix_listDomainNamesFree(nameList);
     break;
     case OS_MAP_ON_SEG:
         result = os_svr4_listDomainNamesFree(nameList);
     break;
     case OS_MAP_ON_HEAP:
         result = 0;
     break;
     }
     return result;
}

os_int32
os_sharedMemoryListUserProcesses(
    os_iter pidList,
    const char * fileName)
{
     os_int32 result;
     os_sharedAttr shmAttr;
     os_sharedAttrInit(&shmAttr);
     result = 0;

     switch (shmAttr.sharedImpl) {
     case OS_MAP_ON_FILE:
         result = os_posix_listUserProcesses(pidList, fileName);
     break;
     case OS_MAP_ON_SEG:
         result = os_svr4_listUserProcesses(pidList, fileName);
     break;
     case OS_MAP_ON_HEAP:
         result = 0;
     break;
     }
     return result;
}

os_int32
os_sharedMemoryListUserProcessesFree(
    os_iter pidList)
{
     os_int32 result;
     os_sharedAttr shmAttr;
     os_sharedAttrInit(&shmAttr);
     result = 0;

     switch (shmAttr.sharedImpl) {
     case OS_MAP_ON_FILE:
         result = os_posix_listUserProcessesFree(pidList);
     break;
     case OS_MAP_ON_SEG:
         result = os_svr4_listUserProcessesFree(pidList);
     break;
     case OS_MAP_ON_HEAP:
         result = 0;
     break;
     }
     return result;
}

os_int32
os_sharedMemorySegmentFree(
    const char * fname)
{
     os_int32 result;
     os_sharedAttr shmAttr;
     os_sharedAttrInit(&shmAttr);
     result = 0;

     switch (shmAttr.sharedImpl) {
     case OS_MAP_ON_FILE:
         result = os_posix_sharedMemorySegmentFree(fname);
     break;
     case OS_MAP_ON_SEG:
         result = os_svr4_sharedMemorySegmentFree(fname);
     break;
     case OS_MAP_ON_HEAP:
         result = 0;
     break;
     }
     return result;
}

void
os_cleanSharedMemAndOrKeyFiles(
    void)
{
     os_int32 result;
     os_sharedAttr shmAttr;
     os_sharedAttrInit(&shmAttr);
     result = 0;

     switch (shmAttr.sharedImpl) {
     case OS_MAP_ON_FILE:
         os_posix_cleanSharedMemAndOrKeyFiles();
     break;
     case OS_MAP_ON_SEG:
         os_svr4_cleanSharedMemAndOrKeyFiles();
     break;
     case OS_MAP_ON_HEAP:
         result = 0;
     break;
     }
     /*return result;*/
}
