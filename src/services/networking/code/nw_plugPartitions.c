
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

NW_CLASS(nw_plugPartition);
NW_STRUCT(nw_plugPartition) {
    nw_partitionId id;
    nw_partitionAddress address;
    nw_bool connected;
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
    nw_bool connected)
{
    nw_plugPartition newPartition;
    
    NW_CONFIDENCE(plugPartitions != NULL);
    NW_CONFIDENCE(partitionId <= plugPartitions->nofPartitions);
    NW_CONFIDENCE(plugPartitions->partitions[partitionId] == NULL);
    
    if (plugPartitions != NULL) {
        newPartition = os_malloc(sizeof(*newPartition));
        if (newPartition != NULL) {
            newPartition->id = partitionId;
            newPartition->address = nw_stringDup(partitionAddress);
            newPartition->connected = connected;
            plugPartitions->partitions[partitionId] = newPartition;
        }
    }
}

void
nw_plugPartitionsSetDefaultPartition(
    nw_plugPartitions plugPartitions,
    nw_partitionAddress partitionAddress)
{
    nw_plugPartitionsSetPartition(plugPartitions, 0, partitionAddress, TRUE);
}



nw_seqNr
nw_plugPartitionsGetNofPartitions(
    nw_plugPartitions plugPartitions)
{
    NW_CONFIDENCE(plugPartitions != NULL);
    
    return plugPartitions->nofPartitions;
}

void
nw_plugPartitionsGetPartition(
    nw_plugPartitions plugPartitions,
    nw_partitionId partitionId,
    nw_bool *found,
    nw_partitionAddress *partitionAddress,
    nw_bool *connected)
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
            NW_CONFIDENCE(plugPartitions->partitions[partitionId]->id == partitionId);
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
    nw_partitionAddress *partitionAddress)
{
    *partitionAddress = plugPartitions->partitions[0]->address;
    
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
