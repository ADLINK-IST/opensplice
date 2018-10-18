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
#include "d_networkAddress.h"
#include "os_heap.h"

d_networkAddress
d_networkAddressNew(
    c_ulong systemId,
    c_ulong localId,
    c_ulong lifecycleId)
{
    d_networkAddress addr;

    addr = os_malloc(sizeof *addr);
    addr->systemId    = systemId;
    addr->localId     = localId;

    /* Lifecycle is explicitly not used. A scan on where lifecycleId is used reveals that there is no
     * logic depending on the lifecycle, other than comparision operations. It could therefore be
     * removed as attribute, although this would mean a change in protocol. */
    OS_UNUSED_ARG(lifecycleId);
    addr->lifecycleId = 0;

    return addr;
}

void
d_networkAddressFree(
    d_networkAddress addr)
{
    os_free(addr);
}

c_bool
d_networkAddressEquals(
    d_networkAddress addr1,
    d_networkAddress addr2)
{
    c_bool result = FALSE;

    if( (addr1->systemId    == addr2->systemId) &&
        (addr1->localId     == addr2->localId) &&
        (addr1->lifecycleId == addr2->lifecycleId))
    {
        result = TRUE;
    }
    return result;
}

d_networkAddress
d_networkAddressUnaddressed()
{
    return d_networkAddressNew(0, 0, 0);
}

c_bool
d_networkAddressIsUnaddressed(
    d_networkAddress address)
{
    d_networkAddress addr;
    c_bool result;

    addr   = d_networkAddressUnaddressed();
    result = d_networkAddressEquals(address, addr);
    d_networkAddressFree(addr);

    return result;

}

int
d_networkAddressCompare(
    d_networkAddress addr1,
    d_networkAddress addr2)
{
    int result;

    if(addr1 == addr2) {
        result = 0;
    } else if(!addr1) {
        result = 1;
    } else if(!addr2) {
        result = -1;
    } else if(addr1->systemId > addr2->systemId){
        result = 1;
    } else if(addr1->systemId < addr2->systemId){
        result = -1;
    } else if(addr1->localId > addr2->localId){
        result = 1;
    } else if(addr1->localId < addr2->localId){
        result = -1;
    } else if(addr1->lifecycleId > addr2->lifecycleId){
        result = 1;
    } else if(addr1->lifecycleId < addr2->lifecycleId){
        result = -1;
    } else {
        result = 0;
    }
    return result;
}
