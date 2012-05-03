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

/* interface */
#include "nw_channel.h"
#include "nw__channel.h"

/* implementation */
#include "os_heap.h"
#include "nw__confidence.h"
#include "nw_commonTypes.h"
#include "nw_bridge.h"
#include "nw_misc.h"       /* for nw_stringDup */
#include "nw_report.h"
#include "v_entity.h"      /* for v_entity() */
#include "v_group.h"       /* for v_group() */
#include "v_topic.h"
#include "v_partition.h"
#include "nw_report.h"
#include "nw_security.h"

#define NW_EXTENDS(type) NW_STRUCT(type) _parent


/* ---------------------------------- channel ------------------------------- */

/* Baseclass */

NW_STRUCT(nw_channel){
    nw_bridge owningBridge;
    nw_seqNr channelId;
};


static void
nw_channelInitialize(
    nw_channel channel,
    nw_bridge owningBridge,
    nw_seqNr channelId)
{
    NW_CONFIDENCE(channel != NULL);

    if (channel != NULL) {
        channel->channelId = channelId;
        channel->owningBridge = owningBridge;
    }
}


static void
nw_channelFinalize(
    nw_channel channel)
{
    NW_CONFIDENCE(channel != NULL);

    if (channel != NULL) {
        nw_bridgeFreeChannel(channel->owningBridge, channel->channelId);
    }
}



/* --------------------------------- hashTable ------------------------------ */

/* Helper hashTable */

NW_CLASS(nw_entryHashItem);
NW_STRUCT(nw_entryHashItem) {
    v_networkHashValue hashValue;
    const c_char *partitionName;
    const c_char *topicName;
    v_networkReaderEntry entry;
    nw_entryHashItem next;
};

static void
nw_entryHashItemInsert(
    v_networkHashValue hashValue,
    const c_char *partitionName,
    const c_char *topicName,
    v_networkReaderEntry entry,
    nw_entryHashItem *prevNextPtr)
{
    nw_entryHashItem newItem;

    NW_CONFIDENCE(prevNextPtr != NULL);
    newItem = (nw_entryHashItem)os_malloc(sizeof(*newItem));
    if (newItem != NULL) {
        newItem->hashValue = hashValue;
        newItem->partitionName = (const char *)nw_stringDup(partitionName);
        newItem->topicName = (const char *)nw_stringDup(topicName);
        newItem->entry = entry;
        newItem->next = *prevNextPtr;
        *prevNextPtr = newItem;
    }
}

static void
nw_entryHashItemRemove(
    nw_entryHashItem item,
    nw_entryHashItem *prevNext)
{
    NW_CONFIDENCE(item != NULL);
    NW_CONFIDENCE(prevNext != NULL);

    *prevNext = item->next;
    os_free((char *)item->partitionName);
    os_free((char *)item->topicName);
    os_free(item);
}

static nw_eq
nw_entryHashItemCompare(
    nw_entryHashItem item1,
    nw_entryHashItem item2)
{
    int cmpRes;

    cmpRes = strcmp(item1->topicName, item2->topicName);
    if (cmpRes < 0) return NW_LT;
    if (cmpRes > 0) return NW_GT;
    cmpRes = strcmp(item1->partitionName, item2->partitionName);
    if (cmpRes < 0) return NW_LT;
    if (cmpRes > 0) return NW_GT;

    return NW_EQ;
}

#define NW_INDEX_FROM_HASHVALUE(hashValue, hashSize)       \
    ((hashValue.h1 +  (hashValue.h2 << 2) +                \
     (hashValue.h3 << 4) + (hashValue.h4 << 6)) % hashSize)
#define NW_ITEM_FROM_HASHVALUE(entryHash, hashValue)       \
    (entryHash->hashItems[NW_INDEX_FROM_HASHVALUE(hashValue, entryHash->hashSize)])
#define NW_ITEM_FROM_INDEX(entryHash, index)               \
    (entryHash->hashItems[index])

NW_CLASS(nw_entryHash);
NW_STRUCT(nw_entryHash) {
    nw_seqNr hashSize;
    nw_entryHashItem hashItems[1]; /*actually: [hashSize] */
};

static nw_entryHash
nw_entryHashNew(
    nw_seqNr hashSize)
{
    nw_entryHash result;
    unsigned int totalSize;

    totalSize = sizeof(*result) + (hashSize-1)*sizeof(result->hashItems[0]);
    result = (nw_entryHash)os_malloc(totalSize);
    if (result != NULL) {
        memset(result, 0, totalSize);
        result->hashSize = hashSize;
    }
    return result;
}

static void
nw_entryHashFree(
    nw_entryHash entryHash)
{
    os_uint32 index;
    nw_entryHashItem *itemPtr;

    NW_CONFIDENCE(entryHash != NULL);

    if (entryHash != NULL) {
        for (index=0; index< entryHash->hashSize; index++) {
            itemPtr = &(entryHash->hashItems[index]);
            while (*itemPtr != NULL) {
                nw_entryHashItemRemove(*itemPtr, itemPtr);
            }
        }
        os_free(entryHash);
    }
}

#ifdef NW_LOOPBACK
static v_networkReaderEntry
nw_entryHashLookupByNamesOnly(
    nw_entryHash entryHash,
    const c_char *partitionName,
    const c_char *topicName)
{
    v_networkReaderEntry result = NULL;
    nw_entryHashItem *itemPtr;
    nw_bool ready = FALSE;
    os_uint32 index = 0;
    int cmpRes;

    while ((index < entryHash->hashSize) && !ready) {
        itemPtr = &NW_ITEM_FROM_INDEX(entryHash, index);
        while ((*itemPtr != NULL) && !ready) {
            cmpRes = strcmp((*itemPtr)->topicName, topicName);
            if (cmpRes == 0) {
                cmpRes = strcmp((*itemPtr)->partitionName, partitionName);
                if (cmpRes == 0) {
                    result = (*itemPtr)->entry;
                    ready = TRUE;
                } else if (cmpRes < 0) {
                    itemPtr = &((*itemPtr)->next);
                } else {
                    /* No entry found, leave inner loop */
                    ready = TRUE;
                }
            } else if (cmpRes < 0) {
                itemPtr = &((*itemPtr)->next);
            } else {
                /* No entry found, leave inner loop */
                ready = TRUE;
            }
        }
        if (result == NULL) {
            /* We are walking over all hash-entries, so if no result, we need to
             * check the other indices. */
            index++;
            ready = FALSE;
        }
    }

    return result;
}
#endif


static v_networkReaderEntry
nw_entryHashLookup(
    nw_entryHash entryHash,
    v_networkHashValue hashValue,
    const c_char *partitionName,
    const c_char *topicName)
{
    v_networkReaderEntry result = NULL;
    nw_entryHashItem *itemPtr;
    nw_bool ready = FALSE;
    int cmpRes;

    itemPtr = &NW_ITEM_FROM_HASHVALUE(entryHash, hashValue);
    while ((*itemPtr != NULL) && !ready) {
        cmpRes = strcmp((*itemPtr)->topicName, topicName);
        if (cmpRes == 0) {
            cmpRes = strcmp((*itemPtr)->partitionName, partitionName);
            if (cmpRes == 0) {
                result = (*itemPtr)->entry;
                ready = TRUE;
            } else if (cmpRes < 0) {
                itemPtr = &((*itemPtr)->next);
            } else {
                /* No entry found, and will not be found */
                ready = TRUE;
            }
        } else if (cmpRes < 0) {
            itemPtr = &((*itemPtr)->next);
        } else {
            /* No entry found, and will not be found */
            ready = TRUE;
        }
    }
    return result;
}

static void
nw_entryHashInsert(
    nw_entryHash entryHash,
    v_networkReaderEntry entry)
{
    nw_entryHashItem *itemPtr;
    nw_bool ready = FALSE;
    int cmpRes;
    const c_char *partitionName;
    const c_char *topicName;

    partitionName = v_partitionName(v_groupPartition(entry->group));
    topicName = v_topicName(v_groupTopic(entry->group));
    itemPtr = &NW_ITEM_FROM_HASHVALUE(entryHash, entry->hashValue);
    while ((*itemPtr != NULL) && !ready) {
        cmpRes = strcmp((*itemPtr)->topicName, topicName);
        if (cmpRes == 0) {
            cmpRes = strcmp((*itemPtr)->partitionName, partitionName);
            if (cmpRes == 0) {
                ready = TRUE;
            } else if (cmpRes < 0) {
                itemPtr = &((*itemPtr)->next);
            } else {
                /* No entry was found and no entry will be found, insert new */
                nw_entryHashItemInsert(entry->hashValue, partitionName, topicName,
                                       entry, itemPtr);
                ready = TRUE;
            }
        } else if (cmpRes < 0) {
            itemPtr = &((*itemPtr)->next);
        } else {
            /* No entry found, and will not be found */
            nw_entryHashItemInsert(entry->hashValue, partitionName, topicName,
                                   entry, itemPtr);
            ready = TRUE;
        }
    }

    if (!ready) {
        /* No proper place found, we are at the tail now */
        nw_entryHashItemInsert(entry->hashValue, partitionName, topicName,
            entry, itemPtr);
    }
}


/* ---------------------------- receiveChannel ------------------------------ */

/**
* @extends nw_channel_s
*/
NW_STRUCT(nw_receiveChannel){
    NW_EXTENDS(nw_channel);
    /* Currently found entry */
    v_networkReaderEntry currentEntry;
    /* hashTable for quick lookup */
    nw_entryHash hash;
};

#define NW_ENTRYHASH_SIZE (256)

nw_receiveChannel
nw_receiveChannelNew(
    nw_bridge owningBridge,
    nw_seqNr channelId)
{
    nw_receiveChannel result = NULL;

    result = (nw_receiveChannel)os_malloc(sizeof(*result));

    if (result != NULL) {
        nw_channelInitialize((nw_channel)result, owningBridge, channelId);
        result->hash = nw_entryHashNew(NW_ENTRYHASH_SIZE);
    }

    return result;
}

#undef NW_ENTRYHASH_SIZE

void
nw_receiveChannelAddGroup(
    nw_receiveChannel channel,
    v_networkReaderEntry entry)
{
    NW_CONFIDENCE(channel != NULL);

    if (channel != NULL) {
        /* Add entry to hashtable */
        nw_entryHashInsert(channel->hash, entry);
        NW_TRACE_3(Groups, 2,
                   "Channel %u added new group with partition %s and topic %s",
                   ((nw_channel)(channel))->channelId,
                   v_partitionName(v_groupPartition(entry->group)),
                   v_topicName(v_groupTopic(entry->group)));
    }
}

void
nw_receiveChannelTrigger(
    nw_receiveChannel channel)
{
    nw_bridgeTrigger(((nw_channel)channel)->owningBridge,
                     ((nw_channel)channel)->channelId);
}

void
nw_receiveChannelFree(
    nw_receiveChannel receiveChannel)
{
    NW_CONFIDENCE(receiveChannel != NULL);

    if (receiveChannel != NULL) {
        nw_entryHashFree(receiveChannel->hash);
        nw_channelFinalize((nw_channel)receiveChannel);
        os_free(receiveChannel);
    }
}

NW_CLASS(nw_lookupArg);
NW_STRUCT(nw_lookupArg) {
    nw_receiveChannel receiveChannel;
    nw_entryLookupAction entryLookupAction;
    nw_entryLookupArg entryLookupArg;
    v_networkReaderEntry entryFound;
};

static c_type
onTypeLookup(
    v_networkHashValue hashValue,
    const c_char *partitionName,
    const c_char *topicName,
    nw_typeLookupArg arg)
{
    c_type result = NULL;
    nw_lookupArg lookupArg = (nw_lookupArg)arg;
    nw_receiveChannel receiveChannel = lookupArg->receiveChannel;
    v_networkReaderEntry entry;

    NW_CONFIDENCE(receiveChannel != NULL);

#ifdef NW_LOOPBACK
    if (nw_configurationUseComplementPartitions()) {
        entry = nw_entryHashLookupByNamesOnly(receiveChannel->hash,
                                              partitionName, topicName);
    } else {
        entry = nw_entryHashLookup(receiveChannel->hash, hashValue,
                                   partitionName, topicName);
    }
#else
    entry = nw_entryHashLookup(receiveChannel->hash, hashValue,
                               partitionName, topicName);
#endif
    if ((entry == NULL) && (lookupArg->entryLookupAction != NULL)) {
        entry = lookupArg->entryLookupAction(hashValue, partitionName,
            topicName, lookupArg->entryLookupArg);
    }
    lookupArg->entryFound = entry;

    if (entry != NULL) {
        result = v_topicMessageType(v_group(entry->group)->topic);
    }

    return result;
}

void
nw_receiveChannelRead(
    nw_receiveChannel receiveChannel,
    v_message *messagePtr,
    v_networkReaderEntry *entryPtr,
    const nw_entryLookupAction entryLookupAction,
    nw_entryLookupArg entryLookupArg,
    plugReceiveStatistics prs)
{
    nw_channel channel = (nw_channel)receiveChannel;
    NW_STRUCT(nw_lookupArg) lookupArg;

    NW_CONFIDENCE(channel != NULL);

    lookupArg.receiveChannel = receiveChannel;
    lookupArg.entryLookupAction = entryLookupAction;
    lookupArg.entryLookupArg = entryLookupArg;
    lookupArg.entryFound = NULL;

    nw_bridgeRead(channel->owningBridge, channel->channelId,
                  messagePtr, onTypeLookup, &lookupArg, prs);

    if (*messagePtr == NULL) {
    	/* If message has been dropped (NULL) for security reasons,
    	 * the entry still might be non-NULL, correct this */
    	lookupArg.entryFound = NULL;
    }

    /* Retrieve entry, but only if we still have the previous message */
    if (*entryPtr == NULL) {
      *entryPtr = lookupArg.entryFound;
    }
}


/* ------------------------------- sendChannel ------------------------------ */

/**
* @extends nw_channel_s
*/
NW_STRUCT(nw_sendChannel){
    NW_EXTENDS(nw_channel);
    /* No attributes introduced (yet) */
};


nw_sendChannel
nw_sendChannelNew(
    nw_bridge owningBridge,
    nw_seqNr channelId)
{
    nw_sendChannel result;

    result = (nw_sendChannel)os_malloc(sizeof(*result));;

    if (result != NULL) {
        nw_channelInitialize((nw_channel)result, owningBridge, channelId);
    }

    return result;
}

void
nw_sendChannelFree(
    nw_sendChannel sendChannel)
{
    NW_CONFIDENCE(sendChannel != NULL);

    if (sendChannel != NULL) {
        nw_channelFinalize((nw_channel)sendChannel);
        os_free(sendChannel);
    }
}


c_ulong
nw_sendChannelWrite(
    nw_sendChannel sendChannel,
    v_networkReaderEntry entry,
    v_message message,
    nw_signedLength *maxBytes,
    plugSendStatistics pss)
{
    c_ulong result = 0;
    nw_channel channel = (nw_channel)sendChannel;

    NW_CONFIDENCE(channel);

    result = nw_bridgeWrite(channel->owningBridge, channel->channelId,
                            entry->networkPartitionId, message, entry->hashValue,
                            v_partitionName(v_groupPartition(entry->group)),
                            v_topicName(v_groupTopic(entry->group)),maxBytes, pss);

    return result;
}

nw_bool
nw_sendChannelFlush(
    nw_sendChannel sendChannel,
    nw_bool all,
    nw_signedLength *maxBytes,
    plugSendStatistics pss)
{
    nw_bool result;
    nw_channel channel = (nw_channel)sendChannel;

    result = nw_bridgeFlush(channel->owningBridge, channel->channelId, all, maxBytes, pss);

    return result;
}

void
nw_sendChannelPeriodicAction(
    nw_sendChannel sendChannel,
    nw_signedLength *maxBytes,
    plugSendStatistics pss)
{
    nw_channel channel = (nw_channel)sendChannel;

    NW_CONFIDENCE(channel);

    if (channel) {
        nw_bridgePeriodicAction(channel->owningBridge, channel->channelId,maxBytes, pss);
    }
}
