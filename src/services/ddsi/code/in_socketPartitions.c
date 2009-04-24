
/* Interface */
#include "in_socketPartitions.h"

/* Implementation */
#include <assert.h>
#include "in_socket.h" /* For the types */

OS_CLASS(in_partition);
OS_STRUCT(in_partition) {
    in_partitionId id;
    in_address address;
    os_boolean connected;
    in_partition nextInHash;
};

in_partition
in_partitionNew(
    in_partitionId id,
    in_address address,
    os_boolean connected,
    in_partition nextInHash) {

    in_partition result = NULL;

    result = os_malloc(sizeof(*result));
    if (result != NULL) {
        result->id = id;
        result->address = address;
        result->connected = connected;
        result->nextInHash = nextInHash;
    }

    return result;
}


#define IN_PARTITIONS_HASHSIZE (256U)

OS_STRUCT(in_socketPartitions) {
    in_partition *hash;
    in_uint hashSize;
};


static void
in_socketPartitionsInitialize(
    in_socketPartitions socketPartitions,
    in_uint hashSize)
{
    size_t hashMemSize;

    hashMemSize = hashSize * sizeof(*socketPartitions->hash);
    socketPartitions->hash = os_malloc(hashMemSize);
    memset(socketPartitions->hash, 0, hashMemSize);
    socketPartitions->hashSize = hashSize;
}


in_socketPartitions
in_socketPartitionsNew() {

    in_socketPartitions result;

    result = os_malloc(sizeof(*result));

    if (result != NULL) {
        in_socketPartitionsInitialize(result, IN_PARTITIONS_HASHSIZE);
    }
    return result;
}


void in_socketPartitionsFinalize(
    in_socketPartitions socketPartitions)
{
    if (socketPartitions->hash != NULL) {
        os_free(socketPartitions->hash);
    }
}


void
in_socketPartitionsFree(
    in_socketPartitions socketPartitions)
{
    if (socketPartitions != NULL) {
        in_socketPartitionsFree(socketPartitions);
        os_free(socketPartitions);
    }
}

/* Returns TRUE if insertion succeeded, FALSE otherwise (item already
 * existed in list */
os_boolean
in_socketPartitionsAdd(
    in_socketPartitions socketPartitions,
    in_partitionId partitionId,
    in_address partitionAddress,
    os_boolean connected)
{
    os_boolean result = FALSE;
    os_boolean found = FALSE;
    in_partition *partitionPtr;
    in_partitionId hashIndex;

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
        *partitionPtr = in_partitionNew(partitionId, partitionAddress, connected, NULL);
        result = TRUE;
    } else if (result) {
        *partitionPtr = in_partitionNew(partitionId, partitionAddress, connected,
            (*partitionPtr)->nextInHash);
    }

    return result;
}


os_boolean
in_socketPartitionsLookup(
    in_socketPartitions socketPartitions,
    in_partitionId partitionId,
    in_address *partitionAddress)
{
    /* Not yet implemented */

    os_boolean result = FALSE;
    os_boolean done = FALSE;
    in_partition partition;
    in_partitionId hashIndex;

    assert(partitionAddress != NULL);

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
