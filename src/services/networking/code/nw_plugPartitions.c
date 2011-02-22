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

/* Interface */
#include "nw__plugPartitions.h"
#include "nw_plugPartitions.h"

/* Implementation */
#include "string.h"
#include "os_heap.h"
#include "nw_misc.h"
#include "nw_socketMisc.h"
#include "nw_commonTypes.h"
#include "nw__confidence.h"
#include "nw_report.h"
#include "nw_partitions.h"

NW_CLASS(nw_plugPartition);
NW_STRUCT(nw_plugPartition) {
    nw_partitionId id;
    nw_partitionAddress address;
    nw_networkSecurityPolicy securityPolicy;
    nw_bool connected;
    nw_bool compression;
    os_uint32 hash;
    c_ulong mTTL;
};

NW_STRUCT(nw_plugPartitions) {
    nw_seqNr nofPartitions;
    nw_plugPartition *partitions;
};


nw_plugPartitions
nw_plugPartitionsNew(
    nw_seqNr nofPartitions)
{
    nw_plugPartitions result;
    os_uint32 size;
    
    result = os_malloc(sizeof(*result));
    if (result != NULL) {
        NW_CONFIDENCE(nofPartitions > 0);
        result->nofPartitions = nofPartitions;
        size = nofPartitions * sizeof(*result->partitions);
        result->partitions = os_malloc(size);
        if (result->partitions != NULL) {
            memset(result->partitions, 0, size);
        }
    }
    return result;
}

void
nw_plugPartitionsFree(
    nw_plugPartitions plugPartitions)
{
    nw_seqNr iPartition;
    
    if (plugPartitions != NULL) {
        if (plugPartitions->partitions != NULL) {
            for (iPartition = 0; iPartition < plugPartitions->nofPartitions; iPartition++) {
                NW_CONFIDENCE(plugPartitions->partitions[iPartition] != NULL);
                if (plugPartitions->partitions[iPartition] != NULL) {
                    NW_CONFIDENCE(plugPartitions->partitions[iPartition]->address != NULL);
                    os_free(plugPartitions->partitions[iPartition]->address);
		    if (plugPartitions->partitions[iPartition]->securityPolicy)
			os_free(plugPartitions->partitions[iPartition]->securityPolicy);
                    os_free(plugPartitions->partitions[iPartition]);
                }
            }
            os_free(plugPartitions->partitions);
        }
        os_free(plugPartitions);
    }
}

void
nw_plugPartitionsSetPartition(
    nw_plugPartitions plugPartitions,
    nw_partitionId partitionId,
    nw_partitionAddress partitionAddress,
    nw_networkSecurityPolicy securityPolicy,
    os_uint32 hash,
    nw_bool connected,
    nw_bool compression,
    c_ulong mTTL)
{
    nw_plugPartition newPartition;
    
    NW_CONFIDENCE(plugPartitions != NULL);
    NW_CONFIDENCE(partitionId <= plugPartitions->nofPartitions);
    NW_CONFIDENCE(plugPartitions->partitions[partitionId] == NULL);
    
    if (plugPartitions != NULL) {
        newPartition = os_malloc(sizeof(*newPartition));
        if (newPartition != NULL) {
            newPartition->id = partitionId;
	    /* FIXME: stringDup return values must be checked for NULL !!, but
	     * question is what should be done in these cases... */ 
            newPartition->address = nw_stringDup(partitionAddress);
	    /* security profile might be undeclared */ 
	    newPartition->securityPolicy = securityPolicy ? nw_stringDup(securityPolicy) : NULL;  
            newPartition->connected = connected;
            newPartition->compression = compression;
            plugPartitions->partitions[partitionId] = newPartition;
            newPartition->hash = hash;
            newPartition->mTTL = mTTL;
        }
    }
}

void
nw_plugPartitionsSetDefaultPartition(
    nw_plugPartitions plugPartitions,
    nw_partitionAddress partitionAddress,
    nw_networkSecurityPolicy securityPolicy,
    os_uint32 hash,
    c_ulong mTTL)
{
    nw_plugPartitionsSetPartition(plugPartitions, 0, partitionAddress, securityPolicy, hash, TRUE, FALSE, mTTL);

}



nw_seqNr
nw_plugPartitionsGetNofPartitions(
    nw_plugPartitions plugPartitions)
{
    NW_CONFIDENCE(plugPartitions != NULL);
    
    return plugPartitions->nofPartitions;
}

nw_partitionId
nw_plugPartitionsGetPartitionIdByHash(
        nw_plugPartitions plugPartitions,
        nw_partitionId hash)
{
    nw_bool foundHash = FALSE;
    nw_partitionId partitionId = 0;
    nw_seqNr counter = 0;
    nw_seqNr maxsize = plugPartitions->nofPartitions;

    while(!foundHash) {

        if (counter >= maxsize) {
            NW_REPORT_WARNING_1("receive data",
                                "PartitionId with hash 0x%x not found, using globalPartition",
                                hash);
            foundHash = TRUE;
        } else {
            if (plugPartitions->partitions[counter] != NULL) {
               if (plugPartitions->partitions[counter]->hash == hash) {
                    partitionId = plugPartitions->partitions[counter]->id;
                    foundHash =TRUE;
                }
            }
        }
        counter++;
    }
    return partitionId;
}

void
nw_plugPartitionsGetPartition(
    nw_plugPartitions plugPartitions,
    nw_partitionId partitionId,
    nw_bool *found,
    nw_partitionAddress *partitionAddress,
    nw_networkSecurityPolicy *securityPolicy, /* may be NULL */
    nw_bool *connected,
    nw_bool *compression,
    os_uint32 *hash,
    c_ulong *mTTL)
{
    *found = FALSE;
    
    NW_CONFIDENCE(plugPartitions != NULL);
    NW_CONFIDENCE(partitionId < plugPartitions->nofPartitions);
    NW_CONFIDENCE(found != NULL);
    NW_CONFIDENCE(partitionAddress != NULL);

    if (partitionId < plugPartitions->nofPartitions) {    
        if (plugPartitions->partitions[partitionId] != NULL) {
            *partitionAddress = plugPartitions->partitions[partitionId]->address;
            *connected = plugPartitions->partitions[partitionId]->connected;
            *compression = plugPartitions->partitions[partitionId]->compression;
            *hash = plugPartitions->partitions[partitionId]->hash;
            *mTTL =  plugPartitions->partitions[partitionId]->mTTL;
            NW_CONFIDENCE(plugPartitions->partitions[partitionId]->id == partitionId);
            if (securityPolicy) { /* may be NULL */
               *securityPolicy = plugPartitions->partitions[partitionId]->securityPolicy;
            }
            *found = TRUE;
        }
    } else {
        NW_REPORT_ERROR_2("nw_plugPartitionsGetPartition",
                         "Illegal Partition Id (%d), Max Id = %d",
                          partitionId, plugPartitions->nofPartitions);
    }
}

nw_bool
nw_plugPartitionsGetDefaultPartition(
    nw_plugPartitions plugPartitions,
    nw_partitionAddress *partitionAddress,
    nw_networkSecurityPolicy *securityPolicy)
{
    *partitionAddress = plugPartitions->partitions[0]->address;
    if (securityPolicy)
	*securityPolicy = plugPartitions->partitions[0]->securityPolicy; 
    
    return TRUE;
}

static nw_bool
nwplugPartitionsContainsAddressType(
    nw_plugPartitions plugPartitions,
    sk_addressType addressType)
{
    nw_bool result = FALSE;
    nw_seqNr iPartition;
    sk_addressType thisAddressType;
    
    if (plugPartitions != NULL) {
        if (plugPartitions->partitions != NULL) {
            for (iPartition = 0;
                 (iPartition < plugPartitions->nofPartitions) && !result;
                 iPartition++) {
                NW_CONFIDENCE(plugPartitions->partitions[iPartition] != NULL);
                if (plugPartitions->partitions[iPartition] != NULL) {
                    NW_CONFIDENCE(plugPartitions->partitions[iPartition]->address != NULL);
                    if (plugPartitions->partitions[iPartition]->connected) {
                        thisAddressType = sk_getAddressType(
                            plugPartitions->partitions[iPartition]->address);
                        result = (addressType == thisAddressType);
                    }
                }
            }
        }
    }
    
    return result;
}
