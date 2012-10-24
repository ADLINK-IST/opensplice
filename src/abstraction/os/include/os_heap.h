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
/****************************************************************
 * Interface definition for OS layer heap memory managment      *
 ****************************************************************/

/** \file os_heap.h
 *  \brief Heap memory management
 *
 * os_heap.h provides abstraction to heap memory management functions.
 * It allows to redefine the default malloc and free function used
 * by the implementation.
 */

#ifndef OS_HEAP_H
#define OS_HEAP_H

#if defined (__cplusplus)
extern "C" {
#endif

/* Define all types used in this interface                      */
#include "os_defs.h"

/* include OS specific header file              */
#include "include/os_alloca.h"
#include "os_if.h"

#ifdef OSPL_BUILD_OS
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/** \brief Allocate memory from heap
 *
 * Allocate memory from heap with the identified size, when
 * no memory is available, return NULL
 *
 * Possible Results:
 * - assertion failure: size <= 0
 * - returns pointer to allocated memory if
 *     memory with required size could be allocated
 * - returns NULL if
 *     memory with required size could not be allocated
 */
OS_API void *
os_malloc(
    os_size_t size);

/** \brief Reallocate memory from heap
 *
 * Reallocate memory from heap. If memblk is NULL
 * the function returns malloc(size). When the size
 * is 0 (zero) the memory pointed at by memblk is
 * freed and NULL is returned. All other cases the
 * the memory block pointed at by memblk is re-sized
 * to size.
 * Possible Results:
 * - returns NULL: if out of memory, or size == 0
 * - return pointer to reallocated memory otherwise.
 */
OS_API void *
os_realloc(
    void *memblk,
    os_size_t size);

/** \brief Free allocated memory and return it to heap
 *
 * Free the allocated memory pointed to by \b ptr
 * and release it to the heap. When \b ptr is NULL,
 * os_free will return without doing any action.
 */
OS_API void
os_free(
    void *ptr);

/** \brief Set alternative heap management functions
 *
 * Set alternative heap memory management functions.
 * To reset the functions to their default, supply NULL.
 *
 * Possible Results:
 * - assertion failure: 
 *   (pmalloc != NULL && prealloc != NULL && pfree != NULL) ||
 *   (pmalloc = NULL && prealloc = NULL && pfree = NULL)
 */
OS_API void
os_heapSetService(
    void *(* pmalloc)(os_size_t),
    void *(* prealloc)(void *, os_size_t),
    void (* pfree)(void *));

/** \brief Return amount of cummulative allocated memory
 */
OS_API os_uint64
os_heapAllocCum(void);

/** \brief Return amount of allocated memory since previous query
 */
OS_API os_uint64
os_heapAlloc(void);

/** \brief Reset counters for allocation interval statistics
 */
OS_API void
os_heapReset(void);

/** \brief Return count of allocated memory segments
 */
OS_API os_uint64
os_heapAllocCount(void);

/** \brief Return count of deallocated memory segments
 */
OS_API os_uint64
os_heapDeallocCount(void);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* OS_HEAP_H */
