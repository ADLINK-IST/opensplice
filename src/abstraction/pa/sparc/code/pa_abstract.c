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
/*
 * Implements PA abstraction for sparc
 */

#ifdef pa_increment
#undef pa_increment
#endif

#ifdef pa_decrement
#undef pa_decrement
#endif

static os_uint32 os_atomic_add(os_uint32 *count, os_int32 difference )
{
   volatile os_uint32 __result;
   uint32_t __tmp1;
   __asm__ __volatile__("\tld [%3], %0\n"
			"1:\tadd %0, %2, %1\n"
			"\tcas [%3], %0, %1\n"
			"\tcmp %0, %1\n"
			"\tbne 1b\n"
			"\tmov %1, %0\n"
			: "=&r" (__result), "+r" (__tmp1)
			: "r" (difference), "r" (count)
			: "memory", "cc");
   return( __result );
}

os_uint32 pa_increment(os_uint32 *count)
{
  return( os_atomic_add( count, 1 )+1 );
}

os_uint32 pa_decrement(os_uint32 *count)
{
    return( os_atomic_add( count, -1 )-1 );
}
