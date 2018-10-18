/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "d_groupsRequest.h"
#include "d_message.h"
#include "os_heap.h"
#include "vortex_os.h"

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
