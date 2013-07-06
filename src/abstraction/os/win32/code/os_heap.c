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
/** \file os/win32/code/os_heap.c
 *  \brief Heap memory management service
 *
 * Implements functions for allocation and freeing
 * memory from and to heap respectively.
 */

#include <memory.h>
#include <stdlib.h>
#include <malloc.h>
#include <assert.h>

static void *os__realloc(void *memblk, os_size_t size);

static void *(* ptr_malloc)(size_t) = malloc;
static void (* ptr_free)(void *) = free;
static void *(* ptr_realloc)(void *,size_t) = (void *(*)(void *,size_t))os__realloc;

static void *
os__realloc(
    void *memblk,
    os_size_t size)
{
    os_size_t oldsize;
    void *new_block;

    if (!memblk) {
        return os_malloc(size);
    }
    if (size == 0) {
        ptr_free(memblk);
        return NULL;
    }
    new_block = os_malloc(size);
    if (new_block) {
        oldsize = _msize(memblk);
        if (oldsize > size) {
            memcpy(new_block, memblk, size);
        } else {
            memcpy(new_block, memblk, oldsize);
        }
	ptr_free (memblk);
    }

    return new_block;
}

/** \brief Allocate memory from heap
 *
 * \b os_malloc calls \b ptr_malloc which is a function pointer
 * which defaults to \b malloc, but can be redefined via
 * \b os_heapSetService.
 */
void *
os_malloc (
    os_size_t size)
{
    return (ptr_malloc((size_t)size));
}

void *
os_realloc(
    void *memblk,
    os_size_t size)
{
    return (ptr_realloc(memblk, (size_t)size));
}

/** \brief Free memory to heap
 *
 * \b os_free calls \b ptr_free which is a function pointer
 * which defaults to \b free, but can be redefined via
 * \b os_heapSetService.
 */
void
os_free (
    void *ptr)
{
    if (ptr != NULL) {
        ptr_free (ptr);
    }
    return;
}

/** \brief Set heap memory management services
 *
 * \b os_heapSetService enables the feature to redefine the SPLICE-DDS
 * heap memory management functions. Either to specific functions
 * identified by \b pmalloc (pmalloc != NULL) and \b pfree
 * (pfree != NULL). Or back to the default values when \b pmalloc
 * = NULL and \b pfree = NULL.
 */
void
os_heapSetService (
    void *(* pmalloc)(os_size_t),
    void *(* prealloc)(void *,os_size_t),
    void (* pfree)(void *))
{
    assert (((pmalloc != NULL) && (prealloc != NULL) && (pfree != NULL)) ||
            ((pmalloc == NULL) && (prealloc == NULL) && (pfree == NULL)));
    if (pmalloc == NULL) {
	ptr_malloc = malloc;
        ptr_realloc = realloc;
	ptr_free = free;
    } else {
	ptr_malloc = (void *(*)(size_t))pmalloc;
        ptr_realloc = (void *(*)(void *,size_t))prealloc;
	ptr_free = pfree;
    }
    return;
}
