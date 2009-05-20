/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifndef NW__PLUGPARTITIONS_H
#define NW__PLUGPARTITIONS_H

#include "nw_plugPartitions.h"

nw_plugPartitions
nw_plugPartitionsNew(
    nw_seqNr nofPartitions);

void
nw_plugPartitionsFree(
    nw_plugPartitions plugPartitions);

void
nw_plugPartitionsSetPartition(
    nw_plugPartitions plugPartitions,
    nw_partitionId partitionId,
    nw_partitionAddress partitionAddress,
    nw_bool connected);

void
nw_plugPartitionsSetDefaultPartition(
    nw_plugPartitions plugPartitions,
    nw_partitionAddress partitionAddress);

#endif /* NW__PLUGPARTITIONS_H */
