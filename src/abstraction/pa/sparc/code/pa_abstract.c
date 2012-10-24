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
/*
 * Implements PA abstraction for sparc
 */

typedef unsigned int _Atomic_word;

unsigned char _S_atomicity_lock = 0;

static unsigned int
__attribute__ ((__unused__))
__exchange_and_add (volatile _Atomic_word* __mem, int __val)
{
  _Atomic_word __result, __tmp;

  __asm__ __volatile__("1:      ldstub  [%1], %0\n\t"
                       "        cmp     %0, 0\n\t"
                       "        bne     1b\n\t"
                       "         nop"
                       : "=&r" (__tmp)
                       : "r" (&_S_atomicity_lock)
                       : "memory");
  __result = *__mem;
  *__mem += __val;
  __asm__ __volatile__("stb     %%g0, [%0]"
                       : /* no outputs */
                       : "r" (&_S_atomicity_lock)
                       : "memory");
  return __result;
}
