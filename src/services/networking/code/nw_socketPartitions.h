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
#ifndef NW_SOCKETPARTITIONS_H
#define NW_SOCKETPARTITIONS_H

#include "nw__socket.h"

NW_CLASS(nw_socketPartitions);

nw_socketPartitions  nw_socketPartitionsNew();

void                 nw_socketPartitionsFree(
                         nw_socketPartitions socketPartitions);

nw_bool              nw_socketPartitionsAdd(
                         nw_socketPartitions socketPartitions,
                         sk_partitionId partitionId,
                         os_sockaddr_storage address,
                         sk_bool connected,
                         sk_bool compression,
                         c_ulong mTTL);

/* Forward declaration */
NW_CLASS(nw_addressList);

/* No need to free the received addressList */
nw_bool              nw_socketPartitionsLookup(
                         nw_socketPartitions socketPartitions,
                         sk_partitionId partitionId,
                         nw_addressList *addressList,
                         sk_bool *compression,
                         c_ulong *mTTL);


/* addressList methods */

os_sockaddr_storage           nw_addressListGetAddress(
                         nw_addressList addressList);

nw_addressList       nw_addressListGetNext(
                         nw_addressList addressList);


#endif /*NW_SOCKETPARTITIONS_H*/
