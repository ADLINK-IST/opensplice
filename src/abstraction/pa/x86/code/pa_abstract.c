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
 * Implements PA abstraction for x86
 */

os_uint32
pa_increment(
    os_uint32 *count)
{
    os_uint32 value;

#ifdef __GNUC__
    __asm__ __volatile__ (
            "movl $1, %0\n\t"
            "lock\n\t"
            "xaddl %0, %2\n\t"
            "incl %0"
    :       "=&r" (value), "=m" (*count)
    :       "m" (*count)
    :       "memory");

    return value;
#else
    AtomicModify( count, &value, 0, 1 );
    return( value + 1 );
#endif
}

os_uint32
pa_decrement(
    os_uint32 *count)
{
    os_uint32 value;
#ifdef __GNUC__
    __asm__ __volatile__ (
            "movl $-1, %0\n\t"
            "lock\n\t"
            "xaddl %0, %2\n\t"
            "decl %0"
    :       "=&r" (value), "=m" (*count)
    :       "m" (*count)
    :       "memory");

    return value;
#else
    AtomicModify( count, &value, 0, -1 );
    return( value - 1 );
#endif
}

