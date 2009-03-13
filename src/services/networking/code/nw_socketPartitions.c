
/* Interface */
#include "nw_socketPartitions.h"

/* Implementation */
#include "nw_socket.h" /* For the types */
#include "nw__confidence.h"

NW_CLASS(nw_partition);
NW_STRUCT(nw_partition) {
    sk_partitionId id;
    sk_address address;
    sk_bool connected;
    nw_partition nextInHash;
};

nw_partition
nw_partitionNew(
    sk_partitionId id,
    sk_address address,
    sk_bool connected,
    nw_partition nextInHash) {

    nw_partition result = NULL;
    
    result = os_malloc(sizeof(*result));
    if (result != NULL) {
        result->id = id;
        result->address = address;
        result->connected = connected;
        result->nextInHash = nextInHash;
    }
    
    return result;
}


#define NW_PARTITIONS_HASHSIZE (256U)

NW_STRUCT(nw_socketPartitions) {
    nw_partition *hash;
    nw_seqNr hashSize;
};


static void
nw_socketPartitionsInitialize(
    nw_socketPartitions socketPartitions,
    nw_seqNr hashSize)
{
    size_t hashMemSize;
    
    hashMemSize = hashSize * sizeof(*socketPartitions->hash);
    socketPartitions->hash = os_malloc(hashMemSize);
    memset(socketPartitions->hash, 0, hashMemSize);
    socketPartitions->hashSize = hashSize;
}


nw_socketPartitions
nw_socketPartitionsNew() {

    nw_socketPartitions result;
    
    result = os_malloc(sizeof(*result));
    
    if (result != NULL) {
        nw_socketPartitionsInitialize(result, NW_PARTITIONS_HASHSIZE);
    }
    return result;
}


void nw_socketPartitionsFinalize(
    nw_socketPartitions socketPartitions)
{
    if (socketPartitions->hash != NULL) {
        os_free(socketPartitions->hash);
    }
}


void
nw_socketPartitionsFree(
    nw_socketPartitions socketPartitions)
{
    if (socketPartitions != NULL) {
        nw_socketPartitionsFree(socketPartitions);
        os_free(socketPartitions);
    }
}

/* Returns TRUE if insertion succeeded, FALSE otherwise (item already
 * existed in list */
nw_bool
nw_socketPartitionsAdd(
    nw_socketPartitions socketPartitions,
    sk_partitionId partitionId,
    sk_address partitionAddress,
    sk_bool connected)
{
    nw_bool result = FALSE;
    nw_bool found = FALSE;
    nw_partition *partitionPtr;
    sk_partitionId hashIndex;

    hashIndex = partitionId % socketPartitions->hashSize;
    partitionPtr = &(socketPartitions->hash[hashIndex]);
    while (*partitionPtr != NULL && !found) {
        if ((*partitionPtr)->id < partitionId) {
            partitionPtr = &((*partitionPtr)->nextInHash);
        } else if ((*partitionPtr)->id > partitionId) {
            found = TRUE;
            /* Insertion is needed so result = TRUE */
            result = TRUE;
        } else {
            /* Exact match already found in the partitionsHash */
            found = TRUE;
        }
    }
    if (!found) {
        /* Item has to be appended to the end */
        *partitionPtr = nw_partitionNew(partitionId, partitionAddress, connected, NULL);
        result = TRUE;
    } else if (result) {
        *partitionPtr = nw_partitionNew(partitionId, partitionAddress, connected,
            (*partitionPtr)->nextInHash);
    }
    
    return result;
}


nw_bool
nw_socketPartitionsLookup(
    nw_socketPartitions socketPartitions,
    sk_partitionId partitionId,
    sk_address *partitionAddress)
{
    /* Not yet implemented */
    
    nw_bool result = FALSE;
    nw_bool done = FALSE;
    nw_partition partition;
    sk_partitionId hashIndex;
    
    NW_CONFIDENCE(partitionAddress != NULL);

    hashIndex = partitionId % socketPartitions->hashSize;
    partition = socketPartitions->hash[hashIndex];
    while (partition != NULL && !done) {
        if (partition->id < partitionId) {
            /* Not found yet, keep on searching in ordered list */
            partition = partition->nextInHash;
        } else if (partition->id == partitionId) {
            /* Exact match found, set result to TRUE */
            *partitionAddress = partition->address;
            done = TRUE;
            result = TRUE;
        } else {
            /* Item not in ordered list */
            done = TRUE;
        }
    }

    return result;
}
