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
#include "v_kernel.h"
#include "v_message.h"
#include "v_time.h"

void
v_messageSetAllocTime(
    v_message _this)
{
#ifndef _NAT_
    if (_this) {
        _this->allocTime = v_timeGet();
    }
#endif    
}


