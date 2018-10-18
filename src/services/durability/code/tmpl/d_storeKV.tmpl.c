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

#include "os_defs.h"
#include "d__store.h"
#include "d_storeKV.h"
#include "u_user.h"


d_storeKV d_storeNewKV (u_participant participant)
{
    OS_UNUSED_ARG(participant);
    return NULL;
}

d_storeResult d_storeFreeKV (d_storeKV store)
{
    OS_UNUSED_ARG(store);
    return D_STORE_RESULT_OK;
}
