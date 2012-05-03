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
#include "nw_socketPartitions.h"

/* Implementation */
#include "nw_socket.h" /* For the types */
#include "nw__confidence.h"

/* Helper class for storing a list of addresses */

NW_STRUCT(nw_addressList) {
    nw_addressList next;
    os_sockaddr_storage address;
};

/* addressList methods */

/* private */

static nw_addressList
nw_addressListNew(
        os_sockaddr_storage address)
{
    nw_addressList result = NULL;

    result = (nw_addressList)os_malloc(sizeof(*result));
    if (result) {
        result->next = NULL;
        result->address = address;
    }
    return result;
}
 /* protected */
static void
nw_addressListFree(
    nw_addressList addressList)
{
    nw_addressList currentItem, nextItem;
    NW_CONFIDENCE(addressList != NULL);

    currentItem = addressList;
    while (currentItem != NULL) {
        nextItem = currentItem->next;
        os_free(currentItem);
        currentItem = nextItem;
    }
}

/* private */
static nw_bool
nw_addressListAppend(
    nw_addressList addressList,
    os_sockaddr_storage address)
{
    nw_bool result = FALSE;
    nw_bool found;
    nw_addressList currentItem, nextItem;

    currentItem = addressList;
    NW_CONFIDENCE(currentItem != NULL);

    if (currentItem) {
        /* Walk to the last element */
        /* This is not in the main loop and not many items are expected in this list */
        nextItem = currentItem->next;
        found = os_sockaddrIPAddressEqual((os_sockaddr*) &currentItem->address,
                                         (os_sockaddr*) &address);
        while (nextItem && !found) {
            currentItem = nextItem;
            nextItem = currentItem->next;
            found = os_sockaddrIPAddressEqual((os_sockaddr*) &currentItem->address,
                                             (os_sockaddr*) &address);
        }
        if (!found) {
            NW_CONFIDENCE(nextItem == NULL);
            currentItem->next = nw_addressListNew(address);
        }
        result = TRUE;
    }
    return result;
}

/* public */

os_sockaddr_storage
nw_addressListGetAddress(
    nw_addressList addressList)
{
    os_sockaddr_storage result;
    memset(&result, 0, sizeof(result));
    if (addressList) {
        result = addressList->address;
    }
    return result;
}


nw_addressList
nw_addressListGetNext(
    nw_addressList addressList)
{
    nw_addressList result = NULL;

    if (addressList) {
        result = addressList->next;
    }
    return result;
}

/* protected */

NW_CLASS(nw_partition);
NW_STRUCT(nw_partition) {
    sk_partitionId id;
    nw_addressList addressList;
    sk_bool connected;
    sk_bool compression;
    nw_partition nextInHash;
    c_ulong mTTL;
};


nw_partition
nw_partitionNew(
    sk_partitionId id,
    os_sockaddr_storage address,
    sk_bool connected,
    sk_bool compression,
    nw_partition nextInHash,
    c_ulong mTTL) {

    nw_partition result = NULL;

    result = os_malloc(sizeof(*result));
    if (result != NULL) {
        result->id = id;
        result->addressList = nw_addressListNew(address);
        result->connected = connected;
        result->compression = compression;
        result->nextInHash = nextInHash;
        result->mTTL = mTTL;
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
    os_sockaddr_storage address,
    sk_bool connected,
    sk_bool compression,
    c_ulong mTTL)
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
        *partitionPtr = nw_partitionNew(partitionId, address, connected, compression, NULL, mTTL);
        result = TRUE;
    } else if (result) {
        *partitionPtr = nw_partitionNew(partitionId, address, connected, compression, (*partitionPtr)->nextInHash, mTTL);
    } else {
        result = nw_addressListAppend((*partitionPtr)->addressList, address);
    }

    return result;
}


nw_bool
nw_socketPartitionsLookup(
    nw_socketPartitions socketPartitions,
    sk_partitionId partitionId,
    nw_addressList *addressList,
    sk_bool *compression,
    c_ulong *mTTL)
{
    /* Not yet implemented */

    nw_bool result = FALSE;
    nw_bool done = FALSE;
    nw_partition partition;
    sk_partitionId hashIndex;

    NW_CONFIDENCE(addressList != NULL);

    hashIndex = partitionId % socketPartitions->hashSize;
    partition = socketPartitions->hash[hashIndex];
    while (partition != NULL && !done) {
        if (partition->id < partitionId) {
            /* Not found yet, keep on searching in ordered list */
            partition = partition->nextInHash;
        } else if (partition->id == partitionId) {
            /* Exact match found, set result to TRUE */
            *addressList = partition->addressList;
            *compression = partition->compression;
            *mTTL = partition->mTTL;
            done = TRUE;
            result = TRUE;
        } else {
            /* Item not in ordered list */
            done = TRUE;
        }
    }

    return result;
}
