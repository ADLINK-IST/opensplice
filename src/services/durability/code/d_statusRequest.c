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

#include "d_statusRequest.h"
#include "d_message.h"
#include "os_heap.h"

d_statusRequest
d_statusRequestNew(
    d_admin admin)
{
    d_statusRequest request = NULL;
    
    if(admin){
        request = d_statusRequest(os_malloc(C_SIZEOF(d_statusRequest)));
        d_messageInit(d_message(request), admin);
    }
    return request;
}

void
d_statusRequestFree(
    d_statusRequest request)
{
    if(request){
        d_messageDeinit(d_message(request));
        os_free(request);
    }
}
