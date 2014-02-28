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

/** \file os/solaris/code/os_abstract.c
 *  \brief PA abstraction
 *
 * Implements PA abstraction for Solaris
 * by including the common services.
 */

#include "../common/code/os_abstract.c"
/*#include "../../pa/code/pa_abstract.c"*/
#include <atomic.h>

/* pa_increment and pa_decrement are implemented at os_level */
#include <atomic.h>

#ifdef pa_increment
#undef pa_increment
#endif

#ifdef pa_decrement
#undef pa_decrement
#endif

os_uint32
pa_increment(
    os_uint32 *count)
{
    return atomic_add_32_nv(count, 1);
}

os_uint32
pa_decrement(
    os_uint32 *count)
{
    return atomic_add_32_nv(count, -1);
}


