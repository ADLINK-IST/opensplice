/** \file os/win32/code/os_abstract.c
 *  \brief PA abstraction
 *
 * Implements PA abstraction for Windows
 */
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

os_uint
pa_increment(
    os_uint *count)
{
    return (os_uint)InterlockedIncrement((LONG *)count);
}

os_uint
pa_decrement(
    os_uint *count)
{
    return (os_uint)InterlockedDecrement((LONG *)count);
}

pa_endianNess
pa_getEndianNess (
    void)
{
   return pa_endianLittle;
}
