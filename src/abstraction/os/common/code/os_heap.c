/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
/** \file os/common/code/os_heap.c
 *  \brief Heap memory management service
 *
 * Implements functions for allocation and freeing
 * memory from and to heap respectively.
 */

#include <sys/types.h>
#include <stdlib.h>
#include <assert.h>
#include "os_signature.h"
#ifdef VXWORKS_RTP
#include <string.h>
#endif
#include "os_abstract.h"
#include "os_atomics.h"

#if defined LINUX && defined OSPL_STRICT_MEM
#include <stdint.h>
#endif

#if defined _WRS_KERNEL && defined OSPL_STRICT_MEM
#include <stdio.h>
#endif

#include <pthread.h>

#ifndef _WRS_KERNEL
static void *(* ptr_malloc)(size_t) = malloc;
static void (* ptr_free)(void *) = free;
static void *(* ptr_realloc)(void *,size_t) = realloc;
#endif

#ifdef OSPL_STRICT_MEM
static pa_uint32_t alloccnt = { 0ULL };
#endif

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
    char *ptr;

    assert(size > 0);

#ifdef OSPL_STRICT_MEM
    /* Allow 24 bytes so we can store the allocation size, magic number and malloc count, ( and keep alignement ) */
    ptr = ptr_malloc((size_t)size+24);
    if ( ptr != NULL )
    {
       *((size_t *)ptr) = size;
       ptr += 24;
       memset(ptr, 0, size);
       *(((uint64_t*)ptr)-1) = OS_MALLOC_MAGIC_SIG;
       *(((uint64_t*)ptr)-2) = pa_inc32_nv(&alloccnt);
    }
#else
    ptr = ptr_malloc(size);
#endif

    if(size == 0 && !ptr) {
        /* os_malloc() should never return NULL. Although it is not allowed to
         * pass size 0, this fallback assures code continues to run if the
         * os_malloc(0) isn't caught in a DEV-build. */
        ptr = ptr_malloc(1);
    }

    if(ptr == NULL) {
        /* Heap exhausted */
        abort();
    }

    return ptr;
}

void *
os_realloc(
    void *memblk,
    os_size_t size)
{
    unsigned char *ptr = (unsigned char *)memblk;

    assert(size > 0);

#ifdef OSPL_STRICT_MEM
    size_t origsize = 0;
    if ( ptr != NULL )
    {
       size_t i;
       origsize = *((size_t *)(ptr - 24));

       assert (*(((uint64_t*)ptr)-1) != OS_FREE_MAGIC_SIG);
       assert (*(((uint64_t*)ptr)-1) == OS_MALLOC_MAGIC_SIG);
       *(((uint64_t*)ptr)-1) = OS_FREE_MAGIC_SIG;

       for ( i = 0; i+7 < origsize; i++ )
       {
          assert( OS_MAGIC_SIG_CHECK( &ptr[i] ) && "Realloc of memory containing mutex or Condition variable" );
       }
       ptr -= 24;
    }

    if ( size > 0 )
    {
       size += 24;
    }
#endif

    ptr = ptr_realloc(ptr, size);

#ifdef OSPL_STRICT_MEM
    if ( size > 0 && ptr != NULL )
    {
       size -= 24;
       if ( size > origsize )
       {
          memset( ptr + 24 + origsize, 0, size - origsize );
       }
       *((size_t *)ptr) = size;
       ptr += 24;
       *(((uint64_t*)ptr)-1) = OS_MALLOC_MAGIC_SIG;
       *(((uint64_t*)ptr)-2) = pa_inc32_nv(&alloccnt);
    }
#endif

    if(ptr == NULL){
        abort();
    }

    return ptr;
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
    if (ptr != NULL)
    {
#ifdef OSPL_STRICT_MEM
        {
          size_t i;
          unsigned char *cptr = (unsigned char *)ptr;
          size_t memsize = *((size_t *)(cptr - 24));
          assert (*(((uint64_t*)ptr)-1) != OS_FREE_MAGIC_SIG);
          if (*(((uint64_t*)ptr)-1) != OS_MALLOC_MAGIC_SIG)
          {
             fprintf (stderr, "%s (%d): os_free error\n", __FILE__, __LINE__);
          }
          assert (*(((uint64_t*)ptr)-1) == OS_MALLOC_MAGIC_SIG);
          *(((uint64_t*)ptr)-1) = OS_FREE_MAGIC_SIG;
          for ( i = 0; i+7 < memsize; i++ )
          {
            assert( OS_MAGIC_SIG_CHECK( &cptr[i] ) && "Free of memory containing Mutex or Condition variable");
          }
          ptr = cptr - 24;
        }
#endif
        ptr_free (((char *)ptr));
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
