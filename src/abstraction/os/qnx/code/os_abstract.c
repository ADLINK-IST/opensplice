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

/** \file os/qnx/code/os_abstract.c
 *  \brief PA abstraction
 *
 * Implements PA abstraction for QNX
 * by including the common services.
 */

#include "../common/code/os_abstract.c"

#if defined (__arm__)

unsigned int __sync_fetch_and_add_4 (volatile void *ptr, unsigned int value)
{
   register unsigned int result;
   __asm__ volatile
   (
      "1: ldrex  %0,  [%1]               \n\t"
      "add       r1,   %0,  %2           \n\t"
      "strex     r2,   r1,  [%1]         \n\t"
      "cmp       r2,   #0                \n\t"
      "bne       1b"
      : "=&r" (result)
      : "r"(ptr), "r"(value)
      : "r1","r2"
   );
   return result;
}

unsigned int __sync_fetch_and_sub_4 (volatile void *ptr, unsigned int value)
{
   register unsigned int result;
   __asm__ volatile
   (
      "1: ldrex  %0,  [%1]               \n\t"
      "sub       r1,   %0,  %2           \n\t"
      "strex     r2,   r1,  [%1]         \n\t"
      "cmp       r2,   #0                \n\t"
      "bne       1b"
      : "=&r" (result)
      : "r"(ptr), "r"(value)
      : "r1","r2"
   );
   return result;
}

unsigned int __sync_fetch_and_or_4 (volatile void *ptr, unsigned int value)
{
   register unsigned int result;
   __asm__ volatile
   (
      "1: ldrex  %0,  [%1]               \n\t"
      "orr       r1,   %0,  %2           \n\t"
      "strex     r2,   r1,  [%1]         \n\t"
      "cmp       r2,   #0                \n\t"
      "bne       1b"
      : "=&r" (result)
      : "r"(ptr), "r"(value)
      : "r1","r2"
   );
   return result;
}

unsigned int __sync_fetch_and_and_4 (volatile void *ptr, unsigned int value)
{
   register unsigned int result;
   __asm__ volatile
   (
      "1: ldrex  %0,  [%1]               \n\t"
      "and       r1,   %0,  %2           \n\t"
      "strex     r2,   r1,  [%1]         \n\t"
      "cmp       r2,   #0                \n\t"
      "bne       1b"   
      : "=&r" (result) 
      : "r"(ptr), "r"(value)
      : "r1","r2"
   );
   return result;
}  

unsigned int __sync_add_and_fetch_4 (volatile void *ptr, unsigned int value)
{
   register unsigned int result;
   __asm__ volatile
   (
      "1: ldrex  %0,  [%1]               \n\t"
      "add       %0,   %0,  %2           \n\t"
      "strex     r1,   %0,  [%1]         \n\t"
      "cmp       r1,   #0                \n\t"
      "bne       1b"
      : "=&r" (result)
      : "r"(ptr), "r"(value)
      : "r1"
   );
   return result;
}

unsigned int __sync_sub_and_fetch_4 (volatile void *ptr, unsigned int value)
{
   register unsigned int result;
   __asm__ volatile
   (
      "1: ldrex  %0,  [%1]               \n\t"
      "sub       %0,   %0,  %2           \n\t"
      "strex     r1,   %0,  [%1]         \n\t"
      "cmp       r1,   #0                \n\t"
      "bne       1b"
      : "=&r" (result)
      : "r"(ptr), "r"(value)
      : "r1"
   );
   return result;
}

unsigned int __sync_or_and_fetch_4 (volatile void *ptr, unsigned int value)
{
   register unsigned int result;
   __asm__ volatile
   (
      "1: ldrex  %0,  [%1]               \n\t"
      "orr       %0,   %0,  %2           \n\t"
      "strex     r1,   %0,  [%1]         \n\t"
      "cmp       r1,   #0                \n\t"
      "bne       1b"
      : "=&r" (result)
      : "r"(ptr), "r"(value)
      : "r1"
   );
   return result;
}

unsigned int __sync_and_and_fetch_4 (volatile void *ptr, unsigned int value)
{
   register unsigned int result;
   __asm__ volatile
   (
      "1: ldrex  %0,  [%1]               \n\t"
      "and       %0,   %0,  %2           \n\t"
      "strex     r1,   %0,  [%1]         \n\t"
      "cmp       r1,   #0                \n\t"
      "bne       1b" 
      : "=&r" (result)
      : "r"(ptr), "r"(value)
      : "r1"
   );
   return result;
}

char __sync_bool_compare_and_swap_4 (volatile void *ptr, uint32_t oldval, uint32_t newval)
{
   register int result;
   __asm__ volatile
   (
      "ldrex    r0, [%1]         \n\t"       /*exclusive load of ptr */
      "cmp      r0,  %2          \n\t"       /*compare the oldval ==  *ptr */
#if defined (__thumb__)
      "ite eq\n\t"
#endif
      "strexeq  %0,  %3, [%1]    \n\t"       /*store if eq, strex+eq*/
#if defined (__thumb__)
      "clrexne"
#endif
      : "=&r" (result)
      : "r"(ptr), "r"(oldval),"r"(newval)
      : "r0"
   );
   return result == 0;
}

#endif
