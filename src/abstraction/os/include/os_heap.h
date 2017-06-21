/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
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

#include "os_defs.h"
#include "os_alloca.h"

#if defined (__cplusplus)
extern "C" {
#endif

/* Define all types used in this interface                      */

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/** \brief Allocate memory from heap
 *
 * Allocate memory from heap with the identified size.
 *
 * Possible Results:
 * - assertion failure: size == 0
 * - abort() if memory exhaustion is detected
 * - returns pointer to allocated memory
 */
OS_API void *
os_malloc(
    os_size_t size) __attribute_malloc__
                    __attribute_returns_nonnull__
                    __attribute_warn_unused_result__
                    __attribute_alloc_size__((1));

/** \brief Reallocate memory from heap
 *
 * Reallocate memory from heap. If memblk is NULL
 * the function returns malloc(size). In contrast to
 * normal realloc, it is NOT supported to free memory
 * by passing 0. This way os_realloc() can be guaranteed
 * to never return NULL.
 * Possible Results:
 * - assertion failure: size == 0
 * - abort() if memory exhaustion is detected
 * - return pointer to reallocated memory otherwise.
 */
OS_API void *
os_realloc(
    void *memblk,
    os_size_t size) __attribute_returns_nonnull__
                    __attribute_warn_unused_result__
                    __attribute_alloc_size__((2));

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

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* OS_HEAP_H */
