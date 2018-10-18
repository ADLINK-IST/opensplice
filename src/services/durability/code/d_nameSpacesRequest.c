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

#include "d_nameSpacesRequest.h"
#include "d_message.h"
#include "os_heap.h"

d_nameSpacesRequest
d_nameSpacesRequestNew(
    d_admin admin)
{
    d_nameSpacesRequest request;

    request = NULL;

    if(admin){
        request = d_nameSpacesRequest(os_malloc(C_SIZEOF(d_nameSpacesRequest)));

        if(request){
            d_messageInit(d_message(request), admin);
        }
    }
    return request;
}

void
d_nameSpacesRequestFree(
    d_nameSpacesRequest request)
{
    if(request){
        d_messageDeinit(d_message(request));
        os_free(request);
    }
}
