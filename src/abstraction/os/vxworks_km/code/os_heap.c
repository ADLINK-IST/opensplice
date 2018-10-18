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

/** \file os/vxworks5.5/code/os_heap.c
 *  \brief vxWorks heap memory management
 *
 * Implements heap memory management for vxWorks
 * by including the common implementation
 */

#ifndef NDEBUG
#include <vxAtomicLib.h>
   atomic_t os__reallocdoublecopycount = 0;
#endif

#if ! defined(__PPC) && ! defined(__x86_64__)
static void *os__alignedmalloc(size_t size)
{
   void *ptr;
   void *origptr;

   origptr = malloc(size+12);
   ptr=origptr;
   if (!ptr)
   {
      return NULL;
   }
   assert ( ((char *)ptr - (char *)0) % 4 == 0 );

   if ( ((char *)ptr - (char *)0) % 8 != 0 )
   {
     /* malloc returned memory not 8 byte aligned */
     /* move pointer by 4 so that it will be aligned again */
     ptr=((uint32_t *)ptr) + 1;
   }

   /* Store requested size */
   /* 8 bytes before the pointer we return */
   *(((size_t *)ptr)) = size;
   ptr=((size_t *)ptr) + 1;

   /* Store pointer to result of malloc "before" the allocation */
   /* 4 bytes before the pointer we return */
   *((void **)ptr)= origptr;
   ptr=((uint32_t *)ptr) + 1;

   assert ( ((char *)ptr - (char *)0) % 8 == 0 );
   return ptr;
}

static void *os__alignedrealloc(void *ptr, size_t size)
{
   void *newptr;  /* Address returned from system realloc */
   void *origptr; /* Address returned by previous *alloc */
   void *olddata; /* Address of original user data immediately after realloc. */
   void *newdata; /* Address user data will be at on return. */
   size_t origsize; /* Size before realloc */

   if ( ptr == NULL )
   {
     return (os__alignedmalloc(size));
   }

   assert ( ((char *)ptr - (char *)0) % 8 == 0 );

   origptr = *(((void **)ptr)-1);
   if ( size == 0 )
   {
      /* really a free */
      realloc(origptr, size);
      return NULL;
   }

   origsize = *(((size_t *)ptr)-2);
   newptr = realloc(origptr, size+12);
   if ( newptr == NULL )
   {
      /* realloc failed, everything is left untouched */
      return NULL;
   }
   olddata = (char *)newptr + ((char *)ptr - (char *)origptr);

   assert ( ((char *)newptr - (char *)0) % 4 == 0 );

   if ( ((char *)newptr - (char *)0) % 8 == 0 )
   {
     /* Allow space for size and pointer */
      newdata = ((uint32_t *)newptr)+2;
   }
   else
   {
      /* malloc returned memory not 8 byte aligned */
      /* realign, and Allow space for size and pointer */
      newdata = ((uint32_t *)newptr)+3;
   }

   assert ( ((char *)newdata - (char *)0) % 8 == 0 );

   if ( (((char *)newptr - (char *)0) % 8) != (((char *)origptr - (char *)0) % 8) )
   {
      /* realloc returned memory with different alignment */
      assert (  ((char *)newdata)+4 == ((char *)olddata)
	      ||((char *)olddata)+4 == ((char *)newdata));
#ifndef NDEBUG
      vxAtomicInc( &os__reallocdoublecopycount);
#endif
      memmove(newdata, olddata, origsize < size ? origsize : size);
   }

   /* Store requested size */
   /* 8 bytes before the pointer we return */
   *(((size_t *)newdata)-2) = size;

   /* Store pointer to result of realloc "before" the allocation */
   /* 4 bytes before the pointer we return */
   *(((void **)newdata)-1) = newptr;

   return newdata;
}

static void os__alignedfree(void *ptr)
{
   assert ( ((char *)ptr - (char *)0) % 8 == 0 );
   free(*(((void **)ptr)-1));
}

static void *(* ptr_malloc)(size_t) = os__alignedmalloc;
static void *(* ptr_realloc)(void *,size_t) = os__alignedrealloc;
static void (* ptr_free)(void *) = os__alignedfree;
#else
/* For 64bit use the native ops align to 8 bytes */
static void *(* ptr_malloc)(size_t) = malloc;
static void (* ptr_free)(void *) = free;
static void *(* ptr_realloc)(void *,size_t) = realloc;
#endif


#include "../common/code/os_heap.c"
