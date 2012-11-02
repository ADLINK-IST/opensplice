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
#ifndef IN_SOCKETPARTITIONS_H
#define IN_SOCKETPARTITIONS_H

#include "in__socket.h"

in_socketPartitions  in_socketPartitionsNew();

void                 in_socketPartitionsFree(
                         in_socketPartitions socketPartitions);

os_boolean              in_socketPartitionsAdd(
                         in_socketPartitions socketPartitions,
                         in_partitionId partitionId,
                         in_address partitionAddress,
                         os_boolean connected);

os_boolean              in_socketPartitionsLookup(
                         in_socketPartitions socketPartitions,
                         in_partitionId partitionId,
                         in_address *partitionAddress);

#endif /*IN_SOCKETPARTITIONS_H*/
