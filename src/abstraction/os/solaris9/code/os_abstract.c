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

#include "../common/code/os_abstract.c"
#include "../../pa/code/pa_abstract.c"

os_uint32
pa_increment(
    os_uint32 *count)
{
    int result;

    result = __exchange_and_add (count, 1);
    result++;

    return (os_uint32)result;
}

os_uint32
pa_decrement(
    os_uint32 *count)
{
    int result;

    result = __exchange_and_add (count, -1);
    result--;

    return (os_uint32)result;
}
