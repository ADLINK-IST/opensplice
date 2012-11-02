/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
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
#include <strings.h>

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
        result = os_posix_sharedMemoryCreate(sharedHandle->name, &sharedHandle->attr, size);
    break;
    case OS_MAP_ON_SEG:
        result = os_svr4_sharedMemoryCreate(sharedHandle->name, &sharedHandle->attr, size);
    break;
    case OS_MAP_ON_HEAP:
        result = os_heap_sharedMemoryCreate(sharedHandle->name, &sharedHandle->attr, size);
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
    assert(sharedHandle->mapped_address != NULL);
    switch (sharedHandle->attr.sharedImpl) {
    case OS_MAP_ON_FILE:
        result = os_posix_sharedMemoryDetach (sharedHandle->name, sharedHandle->mapped_address);
    break;
    case OS_MAP_ON_SEG:
        result = os_svr4_sharedMemoryDetach (sharedHandle->name, sharedHandle->mapped_address);
    break;
    case OS_MAP_ON_HEAP:
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
    assert(sharedHandle->mapped_address != NULL);
    switch (sharedHandle->attr.sharedImpl) {
    case OS_MAP_ON_FILE:
        result = os_posix_sharedSize(sharedHandle->name, size);
    break;
    case OS_MAP_ON_SEG:
        result = os_svr4_sharedSize(sharedHandle->name, size);
    break;
    case OS_MAP_ON_HEAP:
        result = os_heap_sharedSize(sharedHandle->name, size);
    break;
    default:
    break;
    }
    return result;
}

