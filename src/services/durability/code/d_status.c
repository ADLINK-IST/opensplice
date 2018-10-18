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
