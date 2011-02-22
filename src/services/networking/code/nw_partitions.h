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

#ifndef NW_PARTITIONS_H
#define NW_PARTITIONS_H

#include "nw_commonTypes.h"
#include "kernelModule.h"

NW_CLASS(nw_partitions);

typedef char  *nw_networkPartitionName;
typedef char  *nw_networkPartitionAddress;
typedef char  *nw_dcpsPartitionExpression;
typedef char  *nw_dcpsTopicExpression;
typedef char  *nw_dcpsPartitionName;
typedef char  *nw_dcpsTopicName;
typedef char  *nw_networkSecurityPolicy;

#define NW_PARTITIONS_WILDCARD    "*"
#define NW_PARTITIONS_DEFAULT     ""

#define NW_LOCALPARTITION_NAME    "@local"
#define NW_GLOBALPARTITION_NAME   "@global"



nw_partitions         nw_partitionsNew();

void                  nw_partitionsFree(
                          nw_partitions partitions);
                          
/* Creation of networkPartitions and mappings */

void                  nw_partitionsAddPartition(
                          nw_partitions partitions,
                          v_networkPartitionId partitionId,
                          const nw_networkPartitionName partitionName,
                          const nw_networkPartitionAddress partitionAddress,
                          const nw_networkSecurityPolicy securityPolicy,
                          nw_bool connected);

void                  nw_partitionsSetGlobalPartition(
                          nw_partitions partitions,
                          const nw_networkPartitionAddress partitionAddress,
                          const nw_networkSecurityPolicy securityPolicy);

void                  nw_partitionsAddMapping(
                          nw_partitions partitions,
                          const nw_dcpsPartitionExpression dcpsPartitionExpression,
                          const nw_dcpsTopicExpression dcpsTopicExpression,
                          const nw_networkPartitionName networkPartitionName);
                          
/* Looking up of networkPartitions */


v_networkPartitionId  nw_partitionsLookupBestFit(
                          nw_partitions partitions,
                          const nw_dcpsPartitionName dcpsPartitionName,
                          const nw_dcpsTopicName dcpsTopicName);

#endif /*NW_PARTITIONS_H*/
