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
