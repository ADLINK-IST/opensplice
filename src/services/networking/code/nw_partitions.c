
/* Interface */
#include "nw_partitions.h"

/* Implementation */
#include "nw__confidence.h"
#include "nw_report.h"
#include "nw_misc.h"

/* No quick lookups required because this list is supposed to remain short
 * and the lookups only called a few times */

#define NW_IS_LOCALPARTITION(partitionName) \
    (strcmp(partitionName, NW_LOCALPARTITION_NAME) == 0)
#define NW_IS_GLOBALPARTITION(partitionName) \
    (strcmp(partitionName, NW_GLOBALPARTITION_NAME) == 0)

/* ------------------------------- nw_networkPartition ---------------------- */


NW_CLASS(nw_networkPartition);
NW_STRUCT(nw_networkPartition) {
    v_networkPartitionId id;
    nw_networkPartitionName name;
    nw_networkPartitionAddress address; /* Not really needed here */
    nw_bool connected;
    nw_networkPartition next;
};

nw_networkPartition
nw_networkPartitionNew(
    v_networkPartitionId id,
    const nw_networkPartitionName name,
    const nw_networkPartitionAddress address,
    nw_bool connected,
    nw_networkPartition *prevNext)
{
    nw_networkPartition result = NULL;
    
    NW_CONFIDENCE(prevNext != NULL);
    
    result = os_malloc(sizeof(*result));
    
    if (result != NULL) {
        result->id = id;
        result->name = nw_stringDup(name);
        result->address = nw_stringDup(address);
        result->connected = connected;
        result->next = NULL;
        *prevNext = result;
    }
    
    return result;
}

void
nw_networkPartitionFree(
    nw_networkPartition networkPartition,
    nw_networkPartition *prevNext)
{
    NW_CONFIDENCE(prevNext != NULL);
    
    if (networkPartition != NULL) {
        *prevNext = networkPartition->next;
        os_free(networkPartition->name);
        os_free(networkPartition->address);
        os_free(networkPartition);
    }
}
    

/* -------------------------- nw_partitionMapping --------------------------- */


NW_CLASS(nw_partitionMapping);
NW_STRUCT(nw_partitionMapping) {
    nw_dcpsPartitionExpression dcpsPartitionExpression;
    nw_dcpsTopicExpression dcpsTopicExpression;
    nw_networkPartition networkPartition;
    nw_partitionMapping next;
};


nw_partitionMapping
nw_partitionMappingNew(
    nw_dcpsPartitionExpression dcpsPartitionExpression,
    nw_dcpsTopicExpression dcpsTopicExpression,
    nw_networkPartition networkPartition,
    nw_partitionMapping *prevNext)
{
    nw_partitionMapping result = NULL;
    
    NW_CONFIDENCE(prevNext != NULL);
    
    result = os_malloc(sizeof(*result));
    
    if (result != NULL) {
        result->dcpsPartitionExpression = nw_stringDup(dcpsPartitionExpression);
        result->dcpsTopicExpression = nw_stringDup(dcpsTopicExpression);
        result->networkPartition = networkPartition;
        result->next = NULL;
        *prevNext = result;
    }
    
    return result;
}

void
nw_partitionMappingFree(
    nw_partitionMapping partitionMapping,
    nw_partitionMapping *prevNext)
{
    NW_CONFIDENCE(prevNext != NULL);
    
    if (partitionMapping != NULL) {
        *prevNext = partitionMapping->next;
        os_free(partitionMapping->dcpsPartitionExpression);
        os_free(partitionMapping->dcpsTopicExpression);
        os_free(partitionMapping);
    }
}

typedef enum nw_mappingMatch_e {
    NW_MATCH_NONE     = 0,
    NW_MATCH_WILDCARD = 1,
    NW_MATCH_PARTIAL  = 2,
    NW_MATCH_EXACT    = 3
} nw_mappingMatch;

/* Mapping result:
 * 
 *    Partition match |   Topic match  |   result
 * ---------------------------------------------------
 * 1) EXACT           |   EXACT        |   EXACT
 * 2) EXACT           |   WILDCARD     |   PARTIAL
 * 3) EXACT           |   NONE         |   NONE
 * 4) WILDCARD        |   EXACT        |   PARTIAL
 * 5) WILDCARD        |   WILDCARD     |   WILDCARD
 * 6) WILDCARD        |   NONE         |   NONE
 * 7) NONE            |   EXACT        |   NONE
 * 8) NONE            |   WILDCARD     |   NONE
 * 9) NONE            |   NONE         |   NONE
 * 
 */

static nw_mappingMatch
nw_partitionMappingMatch(
    nw_partitionMapping partitionMapping,
    const nw_dcpsPartitionName dcpsPartitionName,
    const nw_dcpsTopicName dcpsTopicName)
{
    nw_mappingMatch result = NW_MATCH_NONE;
    nw_mappingMatch partitionMatch;
    
    if (partitionMapping != NULL) {
        if (strcmp(NW_PARTITIONS_WILDCARD, partitionMapping->dcpsPartitionExpression) == 0) {
            partitionMatch = NW_MATCH_WILDCARD;
        } else if (strcmp(dcpsPartitionName, partitionMapping->dcpsPartitionExpression) == 0) {
            partitionMatch = NW_MATCH_EXACT;
        } else {
            partitionMatch = NW_MATCH_NONE;
        }
        
        if (partitionMatch != NW_MATCH_NONE) {
            if (strcmp(NW_PARTITIONS_WILDCARD, partitionMapping->dcpsTopicExpression) == 0) {
                if (partitionMatch == NW_MATCH_WILDCARD) {
                    result = NW_MATCH_WILDCARD; /* 5 */
                } else {
                    result = NW_MATCH_PARTIAL;  /* 4 */
                }
            } else if (strcmp(dcpsTopicName, partitionMapping->dcpsTopicExpression) == 0) {
                if (partitionMatch == NW_MATCH_WILDCARD) {
                    result = NW_MATCH_PARTIAL;  /* 2 */
                } else {
                    result = NW_MATCH_EXACT;     /* 1 */
                }
            } else {
                result = NW_MATCH_NONE; /* 3 or 6 */
            }
        } else {
            result = NW_MATCH_NONE; /* 7 or 8 or 9 */
        }
    }
    
    return result;
}

/* --------------------------------- nw_partitions -------------------------- */


NW_STRUCT(nw_partitions) {
    nw_networkPartition partitionsHead;
    nw_networkPartition partitionsTail;
    nw_partitionMapping mappingsHead;
    nw_partitionMapping mappingsTail;
};

nw_partitions
nw_partitionsNew() {
    
    nw_partitions result = NULL;
    
    result = os_malloc(sizeof(*result));
    
    if (result != NULL) {
        result->partitionsHead = NULL;
        result->partitionsTail = NULL;
        result->mappingsHead = NULL;
        result->mappingsTail = NULL;
        nw_partitionsAddPartition(result, V_NETWORKPARTITIONID_LOCALHOST,
            NW_LOCALPARTITION_NAME, NULL, TRUE);
    }
    
    return result;
}

void
nw_partitionsFree(
    nw_partitions partitions)
{
    if (partitions != NULL) {
        while (partitions->partitionsHead != NULL) {
            nw_networkPartitionFree(partitions->partitionsHead,
                &partitions->partitionsHead);
        }
        while (partitions->mappingsHead != NULL) {
            nw_partitionMappingFree(partitions->mappingsHead,
                &partitions->mappingsHead);
        }
        os_free(partitions);
    }
}


/* Creation of networkPartitions and mappings */

static nw_networkPartition
nw_partitionsLookupPartitionByName(
    nw_partitions partitions,
    const nw_networkPartitionName partitionName)
{
    nw_networkPartition result = NULL;
    nw_networkPartition currentPartition;
    
    if (partitions != NULL) {
        currentPartition = partitions->partitionsHead;
        while ((result == NULL) && (currentPartition != NULL)) {
            if (strcmp(currentPartition->name, partitionName) == 0) {
                result = currentPartition;
            } else {
                currentPartition = currentPartition->next;
            }
        }
    }
    
    return result;
}

void
nw_partitionsAddPartition(
    nw_partitions partitions,
    v_networkPartitionId networkPartitionId,
    const nw_networkPartitionName partitionName,
    const nw_networkPartitionAddress partitionAddress,
    nw_bool connected)
{
    nw_networkPartitionName usedName = NULL;
    nw_networkPartition networkPartition;
    nw_networkPartition *prevNext;
    
    if (partitions != NULL) {
        
        /* Take default name partitionName is empty or NULL */
        
        if (partitionName != NULL) {
            if (*partitionName != '\0') {
                usedName = partitionName;
            }
        }
        if (usedName == NULL) {
            usedName = partitionAddress;
        }
        
        /* First see if the partition is already available */
        networkPartition = nw_partitionsLookupPartitionByName(partitions, usedName);
        if (networkPartition == NULL) {
            if (partitions->partitionsTail != NULL) {
                prevNext = &(partitions->partitionsTail->next);
            } else {
                prevNext = &(partitions->partitionsHead);
            }
            networkPartition = nw_networkPartitionNew(networkPartitionId,
                usedName, partitionAddress, connected, prevNext);
            partitions->partitionsTail = networkPartition;
        }
    }
}


void
nw_partitionsAddMapping(
    nw_partitions partitions,
    const nw_dcpsPartitionExpression dcpsPartitionExpression,
    const nw_dcpsTopicExpression dcpsTopicExpression,
    const nw_networkPartitionName partitionName)
{
    nw_networkPartition networkPartition;
    nw_partitionMapping partitionMapping;
    nw_partitionMapping *prevNext;
    
    if (partitions != NULL) {
        networkPartition = nw_partitionsLookupPartitionByName(partitions,
            partitionName);
        if (networkPartition == NULL) {
            NW_REPORT_WARNING_1("creating partition mapping",
                "Mapping onto unknown network partition '%s' did not succeed, "
                "using the default partition",
                partitionName);
            networkPartition = nw_partitionsLookupPartitionByName(partitions,
                NW_GLOBALPARTITION_NAME);
            NW_CONFIDENCE(networkPartition != NULL);
        }
        
        if (partitions->mappingsTail != NULL) {
            prevNext = &(partitions->mappingsTail->next);
        } else {
            prevNext = &(partitions->mappingsHead);
        }
        partitionMapping = nw_partitionMappingNew(dcpsPartitionExpression,
            dcpsTopicExpression, networkPartition, prevNext);
        partitions->mappingsTail = partitionMapping;
    }
}


void
nw_partitionsSetGlobalPartition(
    nw_partitions partitions,
    const nw_networkPartitionAddress partitionAddress)
{
    nw_partitionsAddPartition(partitions, 0, NW_GLOBALPARTITION_NAME, partitionAddress, TRUE);
    nw_partitionsAddMapping(partitions, NW_PARTITIONS_WILDCARD,
        NW_PARTITIONS_WILDCARD, NW_GLOBALPARTITION_NAME);
}


                          
/* Looking up of networkPartitions */

v_networkPartitionId
nw_partitionsLookupBestFit(
    nw_partitions partitions,
    const nw_dcpsPartitionName dcpsPartitionName,
    const nw_dcpsTopicName dcpsTopicName)
{
    v_networkPartitionId result = 0;
    nw_partitionMapping currentMapping;
    nw_mappingMatch bestMatch = NW_MATCH_NONE;
    nw_mappingMatch currentMatch;
    
    if (partitions != NULL) {
        currentMapping = partitions->mappingsHead;
        while ((currentMapping != NULL) && (bestMatch != NW_MATCH_EXACT)) {
            currentMatch = nw_partitionMappingMatch(currentMapping,
                dcpsPartitionName, dcpsTopicName);
            if (currentMatch > bestMatch) {
                result = currentMapping->networkPartition->id;
                bestMatch = currentMatch;
            }
            currentMapping = currentMapping->next;
        }
    }
    
    /* We should have found at least a wildcard match, for the default partition */
    NW_CONFIDENCE(bestMatch != NW_MATCH_NONE);
    
    return result;
}
