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

#include "d_status.h"
#include "d_message.h"
#include "os_heap.h"

d_status
d_statusNew(
    d_admin admin)
{
    d_status status = NULL;
    
    if(admin){
        status = d_status(os_malloc(C_SIZEOF(d_status)));
        d_messageInit(d_message(status), admin);
    }
    return status;
}

void
d_statusFree(
    d_status status)
{
    if(status){
        d_messageDeinit(d_message(status));
        os_free(status);
    }
}
