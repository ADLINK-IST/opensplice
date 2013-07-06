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

#include "d_groupsRequest.h"
#include "d_message.h"
#include "os_heap.h"
#include "os.h"

d_groupsRequest
d_groupsRequestNew(
    d_admin admin,
    d_partition partition,
    d_topic topic)
{
    d_groupsRequest groupsRequest = NULL;
    
    if(admin){
        groupsRequest = d_groupsRequest(os_malloc(C_SIZEOF(d_groupsRequest)));
        d_messageInit(d_message(groupsRequest), admin);
        
        if(partition){
            groupsRequest->partition = os_strdup(partition);
        } else {
            groupsRequest->partition = NULL;
        }
        if(topic){
            groupsRequest->topic = os_strdup(topic);
        } else {
            groupsRequest->topic = NULL;
        }
        
    }
    return groupsRequest;
}

void
d_groupsRequestFree(
    d_groupsRequest groupsRequest)
{
    if(groupsRequest){
        d_messageDeinit(d_message(groupsRequest));
        
        if(groupsRequest->partition){
            os_free(groupsRequest->partition);
            groupsRequest->partition = NULL;
        }
        if(groupsRequest->topic){
            os_free(groupsRequest->topic);
            groupsRequest->topic = NULL;
        }
        os_free(groupsRequest);
    }
}
