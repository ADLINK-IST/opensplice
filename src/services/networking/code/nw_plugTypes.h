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
#ifndef NW_PLUGTYPES_H
#define NW_PLUGTYPES_H

#include "nw_commonTypes.h"

typedef unsigned char  *nw_userData;
typedef unsigned int    nw_networkId;
typedef unsigned int    nw_partitionId;
typedef char *          nw_partitionAddress;
typedef unsigned int    nw_latencyBudget;
typedef unsigned int    nw_flags;
typedef unsigned char  *nw_data;
typedef os_address      nw_length;
typedef int             nw_signedLength;
typedef struct nw_globalId_s {
    unsigned int nodeId;
    unsigned int localId1;
    unsigned int localId2;
} nw_globalId;


typedef enum nw_reliabilityKind_e {
    NW_REL_BEST_EFFORT,
    NW_REL_RELIABLE,
    NW_REL_GUARDED
} nw_reliabilityKind;

typedef enum nw_priorityKind_e {
    NW_PRI_LOW,
    NW_PRI_NORMAL,
    NW_PRI_HIGH,
    NW_PRI_VIP
} nw_priorityKind;

typedef enum nw_communicationKind_e {
    NW_COMM_SEND,
    NW_COMM_RECEIVE
} nw_communicationKind;


#define NW_RELIABILITY_UNDEFINED   (NW_REL_BEST_EFFORT)
#define NW_PRIORITY_UNDEFINED      (NW_PRI_NORMAL)
#define NW_LATENCYBUDGET_UNDEFINED (0U)

#endif /* NW_PLUGTYPES_H */
