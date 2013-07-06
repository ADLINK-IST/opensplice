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
#include "cmx__groupqueue.h"
#include "v_groupQueue.h"
#include <stdio.h>
#include "os_stdlib.h"

c_char*
cmx_groupQueueInit(
    v_groupQueue entity)
{
    assert(C_TYPECHECK(entity, v_groupQueue));
    
    return (c_char*)(os_strdup("<kind>GROUPQUEUE</kind>"));
}
