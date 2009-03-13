/*
 * Implements PA abstraction for x86
 */

os_uint32
pa_increment(
    os_uint32 *count)
{
    os_uint32 value;

    __asm__ __volatile__ (
            "movl $1, %0\n\t"
            "lock\n\t"
            "xaddl %0, %2\n\t"
            "incl %0"
    :       "=&r" (value), "=m" (*count)
    :       "m" (*count)
    :       "memory");

    return value;
}

os_uint32
pa_decrement(
    os_uint32 *count)
{
    os_uint32 value;
#if 1
    __asm__ __volatile__ (
            "movl $-1, %0\n\t"
            "lock\n\t"
            "xaddl %0, %2\n\t"
            "decl %0"
    :       "=&r" (value), "=m" (*count)
    :       "m" (*count)
    :       "memory");
#else
    *count--;
    value = *count;
#endif
    return value;
}
