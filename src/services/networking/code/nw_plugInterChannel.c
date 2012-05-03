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
#include "nw_plugInterChannel.h"

/* Implementation */
#include "os_heap.h"
#include "nw_plugTypes.h"
#include "nw__confidence.h"
#include "nw_report.h"

#if 0
#define DPRINT_DATA_POST(packetNr) printf("Posting reception of packet %u\n", packetNr)
#define DPRINT_DATA_PROCESS(valid, packetNr) if (valid) printf("Processing reception of packet %u\n", packetNr)
#define DPRINT_ACK_POST(startingNr, closingNr) printf("Posting reception of ack (%u,%u)\n", startingNr, closingNr)
#define DPRINT_ACK_PROCESS(valid, startingNr, closingNr) if (valid) printf("Processing reception of ack (%u,%u)\n", startingNr, closingNr)
#else
#define DPRINT_DATA_POST(packetNr)
#define DPRINT_DATA_PROCESS(valid, packetNr)
#define DPRINT_ACK_POST(startingNr, closingNr)
#define DPRINT_ACK_PROCESS(valid, startingNr, closingNr)
#endif

/* --------------------------------- Ringbuffer ----------------------------- */

typedef enum nw_plugInterCommMessageKind_e {
    NW_PLUGREL_UNDEFINED,
    NW_PLUGREL_DATA_RECEIVED,
    NW_PLUGREL_ACK_RECEIVED,
    NW_PLUGREL_BACKUP_RECEIVED,
    NW_PLUGREL_DATA_ANNOUNCE,
    NW_PLUGREL_DATA_REQ
} nw_plugInterCommMessageKind;



NW_STRUCT(nw_plugInterCommDataMessage) {
    /* The nodeId that has sent the data */
    nw_seqNr sendingNodeId;
    /* The networkig partition via which this message has been sent */
    nw_partitionId sendingPartitionId;
    /* The node address that has sent the data */
    os_sockaddr_storage sendingNodeAddress;
    /* The packetnumber this message concerns */
    nw_seqNr packetNr;
    /* The current number of defrag buffers in use by the receive thread */
    nw_length currentRecvBuffer;

};
NW_CLASS(nw_plugInterCommDataMessage);

NW_STRUCT(nw_plugInterCommAckMessage) {
    /* The nodeId that has sent the ack */
    nw_seqNr sendingNodeId;
    /* The networkig partition via which this ack has been sent */
    nw_partitionId sendingPartitionId;
    /* The node address that has sent the data */
    os_sockaddr_storage sendingNodeAddress;
    /* The ack data for this message */
    nw_seqNr startingNr;
    nw_seqNr closingNr;
    /* The current number of defrag buffers in use by the receive thread of the remote node*/
    nw_length remoteRecvBuffer;
};
NW_CLASS(nw_plugInterCommAckMessage);

NW_STRUCT(nw_plugInterCommBackupMessage) {
    /* The nodeId that has sent the data */
   nw_seqNr sendingNodeId;
   /* The networkig partition via which this message has been sent */
   nw_partitionId sendingPartitionId;
   /* The node address that has sent the data */
   os_sockaddr_storage sendingNodeAddress;
   /* The complete message buffer */
   nw_plugDataBuffer backupBuffer;
   /* The current number of defrag buffers in use by the receive thread */
   nw_length currentRecvBuffer;

};
NW_CLASS(nw_plugInterCommBackupMessage);

NW_STRUCT(nw_plugInterCommAnnounceMessage) {
    /* The nodeId which died and we have data from */
    nw_seqNr diedNodeId;
    /* The networking partition this data belongs to */
    nw_partitionId partitionId;
    /* The range of messages still available from the died node */
    nw_seqNr firstNr;
    nw_seqNr lastNr;
};
NW_CLASS(nw_plugInterCommAnnounceMessage);

NW_STRUCT(nw_plugInterCommRequestMessage) {
    /* The nodeId that we request the data from */
    nw_seqNr servingNodeId;
    /* The node address that we request the data from */
    os_sockaddr_storage servingNodeAddress;
    /* The nodeId which died and we request data for */
    nw_seqNr diedNodeId;
    /* The networking partition this data belongs to */
    nw_partitionId partitionId;
    /* The range of messages that we request */
    nw_seqNr firstNr;
    nw_seqNr lastNr;
};
NW_CLASS(nw_plugInterCommRequestMessage);

typedef void *nw_ringBufferEntry;

NW_STRUCT(nw_ringBuffer) {
    nw_plugInterCommMessageKind kind;
    os_uint32 nofEntries;
    nw_ringBufferEntry *entries /* [nofEntries] */;
    os_uint32 head;
    os_uint32 tail;
};
NW_CLASS(nw_ringBuffer);


#define NW_RINGBUFFER_INDEX_IS_VALID(ringBuffer, index) \
        (ringBuffer->nofEntries > index)
#define NW_RINGBUFFER_ENTRY_BY_INDEX(ringBuffer, index) \
            (ringBuffer->entries[index])
#define NW_RINGBUFFER_INDEX_INC(ringBuffer, index)     \
        index++;                                       \
        if (index >= ringBuffer->nofEntries) {         \
            index = 0;                                 \
        }
#define NW_RINGBUFFER_IS_FULL(ringBuffer) \
        (((ringBuffer->head + 1) % ringBuffer->nofEntries) == ringBuffer->tail)
#define NW_RINGBUFFER_IS_EMPTY(ringBuffer) \
        (ringBuffer->head == ringBuffer->tail)

static void
nw_ringBufferPostDataEntry(
    nw_ringBuffer ringBuffer,
    nw_seqNr sendingNodeId,
    nw_partitionId sendingPartitionId,
    os_sockaddr_storage sendingNodeAddress,
    nw_seqNr packetNr,
    nw_length currentRecvBuffer)
{
    nw_plugInterCommDataMessage message;

    NW_CONFIDENCE(ringBuffer != NULL);
    NW_CONFIDENCE(ringBuffer->kind == NW_PLUGREL_DATA_RECEIVED);
    NW_CONFIDENCE(NW_RINGBUFFER_INDEX_IS_VALID(ringBuffer, ringBuffer->head));

    if (!NW_RINGBUFFER_IS_FULL(ringBuffer)) {
        message = NW_RINGBUFFER_ENTRY_BY_INDEX(ringBuffer, ringBuffer->head);
        message->sendingNodeAddress = sendingNodeAddress;
        message->sendingPartitionId = sendingPartitionId;
        message->sendingNodeId = sendingNodeId;
        message->packetNr = packetNr;
        message->currentRecvBuffer = currentRecvBuffer;
        NW_RINGBUFFER_INDEX_INC(ringBuffer, ringBuffer->head);
    } else {
       NW_REPORT_WARNING("Posting reliability control message",
           "Interchannel communication administration for reliability is full");
    }
}

static nw_bool
nw_ringBufferProcessDataEntry(
    nw_ringBuffer ringBuffer,
    nw_seqNr *sendingNodeId,
    nw_partitionId *sendingPartitionId,
    os_sockaddr_storage *sendingNodeAddress,
    nw_seqNr *packetNr,
    nw_length *currentRecvBuffer)
{
    nw_plugInterCommDataMessage message;
    nw_bool result;

    NW_CONFIDENCE(ringBuffer != NULL);
    NW_CONFIDENCE(ringBuffer->kind == NW_PLUGREL_DATA_RECEIVED);
    NW_CONFIDENCE(NW_RINGBUFFER_INDEX_IS_VALID(ringBuffer, ringBuffer->tail));
    NW_CONFIDENCE(sendingNodeId != NULL);
    NW_CONFIDENCE(packetNr != NULL);
    NW_CONFIDENCE(currentRecvBuffer != NULL);

    if (!NW_RINGBUFFER_IS_EMPTY(ringBuffer)) {
        message = NW_RINGBUFFER_ENTRY_BY_INDEX(ringBuffer, ringBuffer->tail);
        *sendingNodeAddress = message->sendingNodeAddress;
        *sendingPartitionId = message->sendingPartitionId;
        *sendingNodeId = message->sendingNodeId;
        *packetNr = message->packetNr;
        *currentRecvBuffer = message->currentRecvBuffer;
        NW_RINGBUFFER_INDEX_INC(ringBuffer, ringBuffer->tail);
        result = TRUE;
    } else {
        result = FALSE;
    }
    return result;
}


static void
nw_ringBufferPostAckEntry(
    nw_ringBuffer ringBuffer,
    nw_seqNr sendingNodeId,
    nw_partitionId sendingPartitionId,
    os_sockaddr_storage sendingNodeAddress,
    nw_seqNr startingNr,
    nw_seqNr closingNr,
    nw_length remoteRecvBuffer)
{
    nw_plugInterCommAckMessage message;

    NW_CONFIDENCE(ringBuffer != NULL);
    NW_CONFIDENCE(ringBuffer->kind == NW_PLUGREL_ACK_RECEIVED);
    NW_CONFIDENCE(NW_RINGBUFFER_INDEX_IS_VALID(ringBuffer, ringBuffer->head));

    if (!NW_RINGBUFFER_IS_FULL(ringBuffer)) {
        message = NW_RINGBUFFER_ENTRY_BY_INDEX(ringBuffer, ringBuffer->head);
        message->sendingNodeId = sendingNodeId;
        message->sendingPartitionId = sendingPartitionId;
        message->sendingNodeAddress = sendingNodeAddress;
        message->startingNr = startingNr;
        message->closingNr = closingNr;
        message->remoteRecvBuffer = remoteRecvBuffer;
        NW_RINGBUFFER_INDEX_INC(ringBuffer, ringBuffer->head);
    } else {
       NW_REPORT_WARNING("Posting reliability control message",
           "Interchannel communication administration for reliability is full");
    }

}

static nw_bool
nw_ringBufferProcessAckEntry(
    nw_ringBuffer ringBuffer,
    nw_seqNr *sendingNodeId,
    nw_partitionId *sendingPartitionId,
    os_sockaddr_storage *sendingNodeAddress,
    nw_seqNr *startingNr,
    nw_seqNr *closingNr,
    nw_length *remoteRecvBuffer)
{
    nw_plugInterCommAckMessage message;
    nw_bool result;

    NW_CONFIDENCE(ringBuffer != NULL);
    NW_CONFIDENCE(ringBuffer->kind == NW_PLUGREL_ACK_RECEIVED);
    NW_CONFIDENCE(NW_RINGBUFFER_INDEX_IS_VALID(ringBuffer, ringBuffer->tail));
    NW_CONFIDENCE(sendingNodeId != NULL);
    NW_CONFIDENCE(startingNr != NULL);
    NW_CONFIDENCE(closingNr != NULL);
    NW_CONFIDENCE(remoteRecvBuffer != NULL);

    if (!NW_RINGBUFFER_IS_EMPTY(ringBuffer)) {
        message = NW_RINGBUFFER_ENTRY_BY_INDEX(ringBuffer, ringBuffer->tail);
        *sendingNodeId = message->sendingNodeId;
        *sendingPartitionId = message->sendingPartitionId;
        *sendingNodeAddress = message->sendingNodeAddress;
        *startingNr = message->startingNr;
        *closingNr = message->closingNr;
        *remoteRecvBuffer = message->remoteRecvBuffer;
        NW_RINGBUFFER_INDEX_INC(ringBuffer, ringBuffer->tail);
        result = TRUE;
    } else {
        result = FALSE;
    }
    return result;
}

static void
nw_ringBufferPostBackupEntry(
    nw_ringBuffer ringBuffer,
    nw_seqNr sendingNodeId,
    nw_partitionId sendingPartitionId,
    os_sockaddr_storage sendingNodeAddress,
    nw_plugDataBuffer backupBuffer,
    nw_length currentRecvBuffer)
{
    nw_plugInterCommBackupMessage message;

    NW_CONFIDENCE(ringBuffer != NULL);
    NW_CONFIDENCE(ringBuffer->kind == NW_PLUGREL_BACKUP_RECEIVED);
    NW_CONFIDENCE(NW_RINGBUFFER_INDEX_IS_VALID(ringBuffer, ringBuffer->head));

    if (!NW_RINGBUFFER_IS_FULL(ringBuffer)) {
        message = NW_RINGBUFFER_ENTRY_BY_INDEX(ringBuffer, ringBuffer->head);
        message->sendingNodeAddress = sendingNodeAddress;
        message->sendingPartitionId = sendingPartitionId;
        message->sendingNodeId = sendingNodeId;
        message->backupBuffer = backupBuffer;
        message->currentRecvBuffer = currentRecvBuffer;
        NW_RINGBUFFER_INDEX_INC(ringBuffer, ringBuffer->head);
    } else {
       NW_REPORT_WARNING("Posting reliability backupbuffer message",
           "Interchannel communication administration for reliability is full");
    }
}

static nw_bool
nw_ringBufferProcessBackupEntry(
    nw_ringBuffer ringBuffer,
    nw_seqNr *sendingNodeId,
    nw_partitionId *sendingPartitionId,
    os_sockaddr_storage *sendingNodeAddress,
    nw_plugDataBuffer *backupBuffer,
    nw_length *currentRecvBuffer)
{
    nw_plugInterCommBackupMessage message;
    nw_bool result;

    NW_CONFIDENCE(ringBuffer != NULL);
    NW_CONFIDENCE(ringBuffer->kind == NW_PLUGREL_BACKUP_RECEIVED);
    NW_CONFIDENCE(NW_RINGBUFFER_INDEX_IS_VALID(ringBuffer, ringBuffer->tail));
    NW_CONFIDENCE(sendingNodeId != NULL);
    NW_CONFIDENCE(backupBuffer != NULL);
    NW_CONFIDENCE(currentRecvBuffer != NULL);

    if (!NW_RINGBUFFER_IS_EMPTY(ringBuffer)) {
       message = NW_RINGBUFFER_ENTRY_BY_INDEX(ringBuffer, ringBuffer->tail);
       *sendingNodeAddress = message->sendingNodeAddress;
       *sendingPartitionId = message->sendingPartitionId;
       *sendingNodeId = message->sendingNodeId;
       *backupBuffer = message->backupBuffer;
       *currentRecvBuffer = message->currentRecvBuffer;
       NW_RINGBUFFER_INDEX_INC(ringBuffer, ringBuffer->tail);
       result = TRUE;
    } else {
       result = FALSE;
    }
    return result;
}

static void
nw_ringBufferPostAnnounceEntry(
    nw_ringBuffer ringBuffer,
    nw_seqNr diedNodeId,
    nw_partitionId partitionId,
    nw_seqNr firstNr,
    nw_seqNr lastNr)
{
    nw_plugInterCommAnnounceMessage message;

    NW_CONFIDENCE(ringBuffer != NULL);
    NW_CONFIDENCE(ringBuffer->kind == NW_PLUGREL_DATA_ANNOUNCE);
    NW_CONFIDENCE(NW_RINGBUFFER_INDEX_IS_VALID(ringBuffer, ringBuffer->head));

    if (!NW_RINGBUFFER_IS_FULL(ringBuffer)) {
        message = NW_RINGBUFFER_ENTRY_BY_INDEX(ringBuffer, ringBuffer->head);
        message->diedNodeId = diedNodeId;
        message->partitionId = partitionId;
        message->firstNr = firstNr;
        message->lastNr = lastNr;
        NW_RINGBUFFER_INDEX_INC(ringBuffer, ringBuffer->head);
    } else {
       NW_REPORT_WARNING("Posting reliability control message",
           "Interchannel communication administration for reliability is full");
    }
}

static nw_bool
nw_ringBufferProcessAnnounceEntry(
    nw_ringBuffer ringBuffer,
    nw_seqNr *diedNodeId,
    nw_partitionId *partitionId,
    nw_seqNr *firstNr,
    nw_seqNr *lastNr)
{
    nw_plugInterCommAnnounceMessage message;
    nw_bool result;

    NW_CONFIDENCE(ringBuffer != NULL);
    NW_CONFIDENCE(ringBuffer->kind == NW_PLUGREL_DATA_ANNOUNCE);
    NW_CONFIDENCE(NW_RINGBUFFER_INDEX_IS_VALID(ringBuffer, ringBuffer->tail));
    NW_CONFIDENCE(diedNodeId != NULL);
    NW_CONFIDENCE(firstNr != NULL);
    NW_CONFIDENCE(lastNr != NULL);

    if (!NW_RINGBUFFER_IS_EMPTY(ringBuffer)) {
        message = NW_RINGBUFFER_ENTRY_BY_INDEX(ringBuffer, ringBuffer->tail);
        *diedNodeId = message->diedNodeId;
        *partitionId = message->partitionId;
        *firstNr = message->firstNr;
        *lastNr = message->lastNr;
        NW_RINGBUFFER_INDEX_INC(ringBuffer, ringBuffer->tail);
        result = TRUE;
    } else {
        result = FALSE;
    }
    return result;
}

static void
nw_ringBufferPostRequestEntry(
    nw_ringBuffer ringBuffer,
    nw_seqNr servingNodeId,
    os_sockaddr_storage servingNodeAddress,
    nw_seqNr diedNodeId,
    nw_partitionId partitionId,
    nw_seqNr firstNr,
    nw_seqNr lastNr)
{
    nw_plugInterCommRequestMessage message;

    NW_CONFIDENCE(ringBuffer != NULL);
    NW_CONFIDENCE(ringBuffer->kind == NW_PLUGREL_DATA_REQ);
    NW_CONFIDENCE(NW_RINGBUFFER_INDEX_IS_VALID(ringBuffer, ringBuffer->head));

    if (!NW_RINGBUFFER_IS_FULL(ringBuffer)) {
        message = NW_RINGBUFFER_ENTRY_BY_INDEX(ringBuffer, ringBuffer->head);
        message->servingNodeId = servingNodeId;
        message->servingNodeAddress = servingNodeAddress;
        message->diedNodeId = diedNodeId;
        message->partitionId = partitionId;
        message->firstNr = firstNr;
        message->lastNr = lastNr;
        NW_RINGBUFFER_INDEX_INC(ringBuffer, ringBuffer->head);
    } else {
       NW_REPORT_WARNING("Posting reliability control message",
           "Interchannel communication administration for reliability is full");
    }

}

static nw_bool
nw_ringBufferProcessRequestEntry(
    nw_ringBuffer ringBuffer,
    nw_seqNr *servingNodeId,
    os_sockaddr_storage *servingNodeAddress,
    nw_seqNr *diedNodeId,
    nw_partitionId *partitionId,
    nw_seqNr *firstNr,
    nw_seqNr *lastNr)
{
    nw_plugInterCommRequestMessage message;
    nw_bool result;

    NW_CONFIDENCE(ringBuffer != NULL);
    NW_CONFIDENCE(ringBuffer->kind == NW_PLUGREL_DATA_REQ);
    NW_CONFIDENCE(NW_RINGBUFFER_INDEX_IS_VALID(ringBuffer, ringBuffer->tail));
    NW_CONFIDENCE(servingNodeId != NULL);
    NW_CONFIDENCE(servingNodeAddress != NULL);
    NW_CONFIDENCE(diedNodeId != NULL);
    NW_CONFIDENCE(firstNr != NULL);
    NW_CONFIDENCE(lastNr != NULL);

    if (!NW_RINGBUFFER_IS_EMPTY(ringBuffer)) {
        message = NW_RINGBUFFER_ENTRY_BY_INDEX(ringBuffer, ringBuffer->tail);
        *servingNodeId = message->servingNodeId;
        *servingNodeAddress = message->servingNodeAddress;
        *diedNodeId = message->diedNodeId;
        *partitionId = message->partitionId;
        *firstNr = message->firstNr;
        *lastNr = message->lastNr;
        NW_RINGBUFFER_INDEX_INC(ringBuffer, ringBuffer->tail);
        result = TRUE;
    } else {
        result = FALSE;
    }
    return result;
}


static void
nw_plugInterCommDataMessageNew(
    nw_ringBufferEntry ringBufEntries[],
    os_uint32 nofEntries
    )
{
    nw_plugInterCommDataMessage array;
    os_uint32 i;

    array = os_malloc(sizeof(NW_STRUCT(nw_plugInterCommDataMessage)) * nofEntries);
    if (array) {
        for (i=0; i<nofEntries; i++) {
             ringBufEntries[i] = (nw_ringBufferEntry)&array[i];
        }
    }
}

static void
nw_plugInterCommDataMessageFree(
    nw_ringBufferEntry ringBufEntries[]
    )
{
    /* The ringbuffer elements are allocated in one block. */
    /* Element 0 is the start of the block, thus freeing   */
    /* element 0 frees the whole block.                    */
    os_free (ringBufEntries[0]);
}

static void
nw_plugInterCommAckMessageNew(
    nw_ringBufferEntry ringBufEntries[],
    os_uint32 nofEntries
    )
{
    nw_plugInterCommAckMessage array;
    os_uint32 i;

    array = os_malloc(sizeof(NW_STRUCT(nw_plugInterCommAckMessage)) * nofEntries);
    if (array) {
        for (i=0; i<nofEntries; i++) {
             ringBufEntries[i] = (nw_ringBufferEntry)&array[i];
        }
    }
}

static void
nw_plugInterCommAckMessageFree(
    nw_ringBufferEntry ringBufEntries[]
    )
{
    /* The ringbuffer elements are allocated in one block. */
    /* Element 0 is the start of the block, thus freeing   */
    /* element 0 frees the whole block.                    */
    os_free (ringBufEntries[0]);
}


static void
nw_plugInterCommBackupMessageNew(
        nw_ringBufferEntry ringBufEntries[],
        os_uint32 nofEntries
        )
{
    nw_plugInterCommBackupMessage array;
    os_uint32 i;

    array = os_malloc(sizeof(NW_STRUCT(nw_plugInterCommBackupMessage)) * nofEntries);
    if (array) {
        for (i=0; i<nofEntries; i++) {
             ringBufEntries[i] = (nw_ringBufferEntry)&array[i];
        }
    }
}


static void
nw_plugInterCommBackupMessageFree(
        nw_ringBufferEntry ringBufEntries[]
        )
{
    /* The ringbuffer elements are allocated in one block. */
    /* Element 0 is the start of the block, thus freeing   */
    /* element 0 frees the whole block.                    */
    os_free (ringBufEntries[0]);
}

static void
nw_plugInterCommAnnounceMessageNew(
    nw_ringBufferEntry ringBufEntries[],
    os_uint32 nofEntries
    )
{
    nw_plugInterCommAnnounceMessage array;
    os_uint32 i;

    array = os_malloc(sizeof(NW_STRUCT(nw_plugInterCommAnnounceMessage)) * nofEntries);
    if (array) {
        for (i=0; i<nofEntries; i++) {
             ringBufEntries[i] = (nw_ringBufferEntry)&array[i];
        }
    }
}

static void
nw_plugInterCommAnnounceMessageFree(
    nw_ringBufferEntry ringBufEntries[]
    )
{
    /* The ringbuffer elements are allocated in one block. */
    /* Element 0 is the start of the block, thus freeing   */
    /* element 0 frees the whole block.                    */
    os_free (ringBufEntries[0]);
}

static void
nw_plugInterCommRequestMessageNew(
    nw_ringBufferEntry ringBufEntries[],
    os_uint32 nofEntries
    )
{
    nw_plugInterCommRequestMessage array;
    os_uint32 i;

    array = os_malloc(sizeof(NW_STRUCT(nw_plugInterCommRequestMessage)) * nofEntries);
    if (array) {
        for (i=0; i<nofEntries; i++) {
             ringBufEntries[i] = (nw_ringBufferEntry)&array[i];
        }
    }
}


static void
nw_plugInterCommRequestMessageFree(
        nw_ringBufferEntry ringBufEntries[]
        )
{
    /* The ringbuffer elements are allocated in one block. */
    /* Element 0 is the start of the block, thus freeing   */
    /* element 0 frees the whole block.                    */
    os_free (ringBufEntries[0]);
}

typedef nw_ringBufferEntry (*nw_ringBufferEntryNewFunc)(nw_ringBufferEntry ringBufEntries[], os_uint32 nofEntries);
typedef void               (*nw_ringBufferEntryFreeFunc)(nw_ringBufferEntry ringBufEntries[]);


static nw_ringBuffer
nw_ringBufferNew(
    nw_plugInterCommMessageKind kind,
    os_uint32 nofEntries)
{
    nw_ringBuffer result = NULL;

    result = (nw_ringBuffer)os_malloc((os_uint32)sizeof(*result));

    if (result != NULL) {
        result->kind = kind;
        result->entries = (nw_ringBufferEntry *)os_malloc(
                              nofEntries * (os_uint32)sizeof(*result->entries));
        if (result->entries) {
            result->nofEntries = nofEntries;

            switch (kind) {
                case NW_PLUGREL_DATA_RECEIVED:
                    nw_plugInterCommDataMessageNew (result->entries, nofEntries);
                    break;
                case NW_PLUGREL_ACK_RECEIVED:
                    nw_plugInterCommAckMessageNew (result->entries, nofEntries);
                    break;
                case NW_PLUGREL_BACKUP_RECEIVED:
                    nw_plugInterCommBackupMessageNew (result->entries, nofEntries);
                    break;
                case NW_PLUGREL_DATA_ANNOUNCE:
                    nw_plugInterCommAnnounceMessageNew (result->entries, nofEntries);
                    break;
                case NW_PLUGREL_DATA_REQ:
                    nw_plugInterCommRequestMessageNew (result->entries, nofEntries);
                    break;
                default:
                    NW_CONFIDENCE(FALSE);
                    break;
            }
        } else {
            result->nofEntries = 0;
        }
        result->head = 0;
        result->tail = 0;
    }

    return result;
}


static void
nw_ringBufferFree(
    nw_ringBuffer ringBuffer)
{
    if (ringBuffer != NULL) {
        if (ringBuffer->entries != NULL) {
            switch (ringBuffer->kind) {
                case NW_PLUGREL_DATA_RECEIVED:
                    nw_plugInterCommDataMessageFree (ringBuffer->entries);
                    break;
                case NW_PLUGREL_ACK_RECEIVED:
                    nw_plugInterCommAckMessageFree (ringBuffer->entries);
                    break;
                case NW_PLUGREL_BACKUP_RECEIVED:
                    nw_plugInterCommBackupMessageFree (ringBuffer->entries);
                    break;
                case NW_PLUGREL_DATA_ANNOUNCE:
                    nw_plugInterCommAnnounceMessageFree (ringBuffer->entries);
                    break;
                case NW_PLUGREL_DATA_REQ:
                    nw_plugInterCommRequestMessageFree (ringBuffer->entries);
                    break;
                default:
                    NW_CONFIDENCE(FALSE);
                    break;
            }
            os_free(ringBuffer->entries);
        }
        os_free(ringBuffer);
    }
}


/* ------------------------ InterChannel communication ---------------------- */


NW_STRUCT(nw_plugInterChannel) {
    nw_ringBuffer ringBufferDataReceived;
    nw_ringBuffer ringBufferAckReceived;
    nw_ringBuffer ringBufferBackupReceived;
    nw_ringBuffer ringBufferDataAnnounce;
    nw_ringBuffer ringBufferDataRequest;
    nw_bool       trigger;
    os_uint32 refCount;
};

static nw_plugInterChannel
nw_plugInterChannelNew(
    os_uint32 queueSize)
{
    nw_plugInterChannel result;

    result = (nw_plugInterChannel)os_malloc(sizeof(*result));
    if (result != NULL) {
        result->ringBufferDataReceived = nw_ringBufferNew(NW_PLUGREL_DATA_RECEIVED, queueSize);
        result->ringBufferAckReceived = nw_ringBufferNew(NW_PLUGREL_ACK_RECEIVED, queueSize);
        result->ringBufferBackupReceived = nw_ringBufferNew(NW_PLUGREL_BACKUP_RECEIVED, queueSize);
        result->ringBufferDataAnnounce = nw_ringBufferNew(NW_PLUGREL_DATA_ANNOUNCE, queueSize);
        result->ringBufferDataRequest = nw_ringBufferNew(NW_PLUGREL_DATA_REQ, queueSize);
        result->trigger = FALSE;
        result->refCount = 1;
    }
    return result;
}

static void
nw_plugInterChannelFree(
    nw_plugInterChannel plugInterChannel)
{
    NW_CONFIDENCE(plugInterChannel != NULL);

    if (plugInterChannel != NULL) {
        nw_ringBufferFree(plugInterChannel->ringBufferDataReceived);
        nw_ringBufferFree(plugInterChannel->ringBufferAckReceived);
        nw_ringBufferFree(plugInterChannel->ringBufferBackupReceived);
        nw_ringBufferFree(plugInterChannel->ringBufferDataAnnounce);
        nw_ringBufferFree(plugInterChannel->ringBufferDataRequest);
        os_free(plugInterChannel);
    }
}


void
nw_plugInterChannelIncarnate(
    nw_plugInterChannel *interChannel,
    const char *pathName)
{
    c_ulong ringbufferSize;

    NW_CONFIDENCE(interChannel != NULL);

    if (*interChannel == NULL) {
        ringbufferSize = NWCF_SIMPLE_PARAM(ULong, pathName, AdminQueueSize);
        if (ringbufferSize < NWCF_MIN(AdminQueueSize)) {
            NW_REPORT_WARNING_3("initializing network",
                "Reliable channel \"%s\": requested value %u for admin queuesize is too small, "
                "using %u instead",
                pathName, ringbufferSize, NWCF_MIN(AdminQueueSize));
            ringbufferSize = NWCF_MIN(AdminQueueSize);
        }
        *interChannel = nw_plugInterChannelNew(ringbufferSize);
    } else {
        (*interChannel)->refCount++;
    }
}


void
nw_plugInterChannelExcarnate(
    nw_plugInterChannel *interChannel)
{
    NW_CONFIDENCE(interChannel != NULL);

    if (*interChannel != NULL) {
        (*interChannel)->refCount--;
        if ((*interChannel)->refCount == 0) {
            nw_plugInterChannelFree(*interChannel);
            *interChannel = NULL;
        }
    } else {
        /* Error */
        NW_CONFIDENCE(*interChannel != NULL);
    }
}


void
nw_plugInterChannelPostDataReceivedMessage(
    nw_plugInterChannel plugInterChannel,
    nw_seqNr sendingNodeId,
    nw_partitionId sendingPartitionId,
    os_sockaddr_storage sendingNodeAddress,
    nw_seqNr packetNr,
    nw_length currentRecvBuffer)
{
    NW_CONFIDENCE(plugInterChannel != NULL);

DPRINT_DATA_POST(packetNr);
    nw_ringBufferPostDataEntry(plugInterChannel->ringBufferDataReceived,
        sendingNodeId, sendingPartitionId, sendingNodeAddress, packetNr,currentRecvBuffer);
}

nw_bool
nw_plugInterChannelProcessDataReceivedMessage(
    nw_plugInterChannel plugInterChannel,
    nw_seqNr *sendingNodeId,
    nw_partitionId *sendingPartitionId,
    os_sockaddr_storage *sendingNodeAddress,
    nw_seqNr *packetNr,
    nw_length *currentRecvBuffer)
{
    nw_bool result;

    NW_CONFIDENCE(plugInterChannel != NULL);

    result = nw_ringBufferProcessDataEntry(plugInterChannel->ringBufferDataReceived,
                 sendingNodeId, sendingPartitionId, sendingNodeAddress, packetNr,currentRecvBuffer);
DPRINT_DATA_PROCESS(result, *packetNr);

    return result;
}


void
nw_plugInterChannelPostAckReceivedMessage(
    nw_plugInterChannel plugInterChannel,
    nw_seqNr sendingNodeId,
    nw_partitionId sendingPartitionId,
    os_sockaddr_storage sendingNodeAddress,
    nw_seqNr startingNr,
    nw_seqNr closingNr,
    nw_length remoteRecvBuffer)
{
    NW_CONFIDENCE(plugInterChannel != NULL);

DPRINT_ACK_POST(startingNr, closingNr);
    nw_ringBufferPostAckEntry(plugInterChannel->ringBufferAckReceived,
        sendingNodeId, sendingPartitionId, sendingNodeAddress,
        startingNr, closingNr, remoteRecvBuffer );
}

nw_bool
nw_plugInterChannelProcessAckReceivedMessage(
    nw_plugInterChannel plugInterChannel,
    nw_seqNr *sendingNodeId,
    nw_partitionId *sendingPartitionId,
    os_sockaddr_storage *sendingNodeAddress,
    nw_seqNr *startingNr,
    nw_seqNr *closingNr,
    nw_length *remoteRecvBuffer)
{
    nw_bool result;

    NW_CONFIDENCE(plugInterChannel != NULL);

    result = nw_ringBufferProcessAckEntry(plugInterChannel->ringBufferAckReceived,
                 sendingNodeId, sendingPartitionId, sendingNodeAddress,
                 startingNr, closingNr, remoteRecvBuffer);
DPRINT_ACK_PROCESS(result, *startingNr, *closingNr);

    return result;
}

void
nw_plugInterChannelPostBackupReceivedMessage(
    nw_plugInterChannel plugInterChannel,
    nw_seqNr sendingNodeId,
    nw_partitionId sendingPartitionId,
    os_sockaddr_storage sendingNodeAddress,
    nw_plugDataBuffer backupBuffer,
    nw_length currentRecvBuffer)
{
    NW_CONFIDENCE(plugInterChannel != NULL);

    nw_ringBufferPostBackupEntry(plugInterChannel->ringBufferBackupReceived,
        sendingNodeId, sendingPartitionId, sendingNodeAddress, backupBuffer,currentRecvBuffer);
}

nw_bool
nw_plugInterChannelProcessBackupReceivedMessage(
    nw_plugInterChannel plugInterChannel,
    nw_seqNr *sendingNodeId,
    nw_partitionId *sendingPartitionId,
    os_sockaddr_storage *sendingNodeAddress,
    nw_plugDataBuffer *backupBuffer,
    nw_length *currentRecvBuffer)
{
    nw_bool result;

    NW_CONFIDENCE(plugInterChannel != NULL);

    result = nw_ringBufferProcessBackupEntry(plugInterChannel->ringBufferBackupReceived,
                 sendingNodeId, sendingPartitionId, sendingNodeAddress, backupBuffer,currentRecvBuffer);

    return result;
}

void
nw_plugInterChannelPostDataAnnounceMessage(
    nw_plugInterChannel plugInterChannel,
    nw_seqNr diedNodeId,
    nw_partitionId partitionId,
    nw_seqNr firstNr,
    nw_seqNr lastNr)
{
    NW_CONFIDENCE(plugInterChannel != NULL);

    nw_ringBufferPostAnnounceEntry(plugInterChannel->ringBufferDataAnnounce,
        diedNodeId, partitionId, firstNr, lastNr);
}

nw_bool
nw_plugInterChannelProcessDataAnnounceMessage(
    nw_plugInterChannel plugInterChannel,
    nw_seqNr *diedNodeId,
    nw_partitionId *partitionId,
    nw_seqNr *firstNr,
    nw_seqNr *lastNr)
{
    nw_bool result;

    NW_CONFIDENCE(plugInterChannel != NULL);

    result = nw_ringBufferProcessAnnounceEntry(plugInterChannel->ringBufferDataAnnounce,
                 diedNodeId, partitionId, firstNr, lastNr);

    return result;
}

void
nw_plugInterChannelPostDataRequestMessage(
    nw_plugInterChannel plugInterChannel,
    nw_seqNr servingNodeId,
    os_sockaddr_storage servingNodeAddress,
    nw_seqNr diedNodeId,
    nw_partitionId partitionId,
    nw_seqNr firstNr,
    nw_seqNr lastNr)
{
    NW_CONFIDENCE(plugInterChannel != NULL);


    nw_ringBufferPostRequestEntry(plugInterChannel->ringBufferDataRequest,
        servingNodeId, servingNodeAddress, diedNodeId, partitionId, firstNr, lastNr);
}

nw_bool
nw_plugInterChannelProcessDataRequestMessage(
    nw_plugInterChannel plugInterChannel,
    nw_seqNr *servingNodeId,
    os_sockaddr_storage *servingNodeAddress,
    nw_seqNr *diedNodeId,
    nw_partitionId *partitionId,
    nw_seqNr *firstNr,
    nw_seqNr *lastNr)
{
    nw_bool result;

    NW_CONFIDENCE(plugInterChannel != NULL);

    result = nw_ringBufferProcessRequestEntry(plugInterChannel->ringBufferDataRequest,
                 servingNodeId, servingNodeAddress, diedNodeId, partitionId, firstNr, lastNr);

    return  result;
}

void
nw_plugInterChannelSetTrigger(
    nw_plugInterChannel plugInterChannel)
{
    plugInterChannel->trigger = TRUE;
}

nw_bool
nw_plugInterChannelGetTrigger(
    nw_plugInterChannel plugInterChannel)
{
    nw_bool result = plugInterChannel->trigger;
    plugInterChannel->trigger = FALSE;

    return result;
}



