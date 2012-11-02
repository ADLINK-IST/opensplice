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
#ifndef NW__PLUGDATABUFFER_H
#define NW__PLUGDATABUFFER_H

#include "nw__plugNetwork.h" /* for HtoN and NtoH */
#include "nw__plugBuffer.h"
#include "nw_configuration.h" /* for NW_DEBUGGING */

/* To be used by nw_plugChannel descendants only.
 * These structs are privately public for quick access, to avoid
 * the overhead of extra function calls */

#define nw_plugDataBuffer(o) ((nw_plugDataBuffer)(o))

#define NW_PLUGDATABUFFER_ALIGNMENT      (4)
#define NW_PLUGDATABUFFER_DATA_ALIGNMENT (4)
#define NW_PLUGDATABUFFER_HEADERSIZE \
        (sizeof(NW_STRUCT(nw_plugDataBuffer)))

#define UI(val) ((os_address)(val))

#define NW_PLUGDATABUFFER_FIRSTMESSAGE(buffer) \
        ((nw_messageHolder)(UI(buffer) + NW_PLUGDATABUFFER_HEADERSIZE))

#define NW_PLUGDATABUFFER_DIFF(p1, p2) \
        (UI(p2) - UI(p1))

#define NW_MESSAGEHOLDER_SIZE \
        (sizeof(NW_STRUCT(nw_messageHolder)))

#define NW_MESSAGEHOLDER_DATA(holder) \
        ((nw_data)(UI(holder) + NW_MESSAGEHOLDER_SIZE))

#ifdef NW_DEBUGGING
#define NW_VALUE_DONT_CARE (0xD0BE900D)
#endif /* NW_DEBUGGING */

/*#define NW_TIMESTAMP*/

#ifdef NW_TIMESTAMP
/* timestamp names*/
typedef enum {
    NW_MSG_TIMESTAMP_SERIALIZED, /* After serialize. */
    NW_MSG_TIMESTAMP_MAX
} NW_MSG_TIMESTAMP_ID;
typedef enum {
    NW_BUF_TIMESTAMP_FILLED, /* nw buffer is full, ready to be send. */
    NW_BUF_TIMESTAMP_FLUSH,  /* start of sending full buffers. */
    NW_BUF_TIMESTAMP_SEND,   /* write buffer into socket. */
    NW_BUF_TIMESTAMP_RECEIVE, /* read from socket into defrag buffers and add to wait queue. */
    NW_BUF_TIMESTAMP_HANDLE, /* take from wait queue and insert or place into out of order list. */
    NW_BUF_TIMESTAMP_MAX
} NW_BUF_TIMESTAMP_ID;

/* fill a named timestampslot of item with the current time */
/* can be used for both messageHolder as plugDataBuffers */
#define NW_STAMP(item,id) \
    (item)->timestamp[id] = os_hrtimeGet()
#else
#define NW_STAMP(item,id)
#endif /* NW_TIMESTAMP */


NW_CLASS(nw_messageHolder);
NW_STRUCT(nw_messageHolder) {
    nw_seqNr      length;
#ifdef NW_TIMESTAMP
/*    os_time timestamp[NW_MSG_TIMESTAMP_MAX];*/
#endif /* NW_TIMESTAMP */
    /* unsigned char data[]  Note: this is a stretchy array */
};

/* Important note: make sure that this message has the same memory layout on all
 *                 supported platforms. endianness may differ but alignment
 *                 needs to match */
NW_CLASS(nw_plugDataBuffer);
/**
* @extends nw_plugBuffer_s
*/
NW_STRUCT(nw_plugDataBuffer) {
    NW_EXTENDS(nw_plugBuffer);
    /* The partition this message is meant to be sent to */
    nw_partitionId partitionId;
    /* The number of messages within this fragment */
    nw_seqNr nrOfMessages;
    /* The packetnumber used for reliability */
    nw_seqNr packetNr;
    /* The number of fragmented message, in case it has been fragmented */
    nw_seqNr fragmentedMsgNr;
    /* The number of fragment within this message */
    nw_seqNr fragmentNr;
    /* The number of fragmented message if this packet contains the last fragment */
    nw_seqNr terminatedMsgNr;
    /* the number of the terminating fragment */
    nw_seqNr terminatingFragmentNr;
#ifdef NW_TIMESTAMP
    os_time timestamp[NW_BUF_TIMESTAMP_MAX];
#endif /* NW_TIMESTAMP */
    /* The data itself */
    /* NW_STRUCT(nw_messageHolder) firstMessage; */
};

/* Getters/Setters */

#define nw_plugDataBufferGetPartitionId(buffer) \
        nw_plugNetworkToHost(buffer->partitionId)

#define nw_plugDataBufferSetPartitionId(buffer, value) \
        buffer->partitionId = nw_plugHostToNetwork(value)

#define nw_plugDataBufferGetNrOfMessages(buffer) \
        nw_plugNetworkToHost(buffer->nrOfMessages)

#define nw_plugDataBufferSetNrOfMessages(buffer, value) \
        buffer->nrOfMessages = nw_plugHostToNetwork(value)

#define nw_plugDataBufferGetPacketNr(buffer) \
        nw_plugNetworkToHost(buffer->packetNr)

#define nw_plugDataBufferSetPacketNr(buffer, value) \
        buffer->packetNr = nw_plugHostToNetwork(value)

#define nw_plugDataBufferGetFragmentedMsgNr(buffer) \
        nw_plugNetworkToHost(buffer->fragmentedMsgNr)

#define nw_plugDataBufferSetFragmentedMsgNr(buffer, value) \
        buffer->fragmentedMsgNr = nw_plugHostToNetwork(value)

#define nw_plugDataBufferGetFragmentNr(buffer) \
        nw_plugNetworkToHost(buffer->fragmentNr)

#define nw_plugDataBufferSetFragmentNr(buffer, value) \
        buffer->fragmentNr = nw_plugHostToNetwork(value)

#define nw_plugDataBufferGetTerminatedMsgNr(buffer) \
        nw_plugNetworkToHost(buffer->terminatedMsgNr)

#define nw_plugDataBufferSetTerminatedMsgNr(buffer, value) \
        buffer->terminatedMsgNr = nw_plugHostToNetwork(value)

#define nw_plugDataBufferGetTerminatingFragmentNr(buffer) \
        nw_plugNetworkToHost(buffer->terminatingFragmentNr)

#define nw_plugDataBufferSetTerminatingFragmentNr(buffer, value) \
        buffer->terminatingFragmentNr = nw_plugHostToNetwork(value)

#define nw_messageHolderGetLength(holder) \
        nw_plugNetworkToHost(holder->length)

#define nw_messageHolderSetLength(holder, value) \
        holder->length = nw_plugHostToNetwork(value)



/* Buffer-analysis functions */

nw_bool          nw_plugDataBufferContainsWholeMessages(
                     nw_plugDataBuffer buffer);

nw_messageHolder nw_plugDataBufferGetNextMessageHolder(
                     nw_plugDataBuffer buffer,
                     nw_messageHolder prevMessageHolder,
                     nw_bool wholeMessagesOnly);

nw_bool          nw_plugDataBufferIsLastMessageHolder(
                     nw_plugDataBuffer buffer,
                     nw_messageHolder messageHolder);

char *
nw_plugDataBufferToString(
    nw_plugDataBuffer buffer);

#endif /* NW__PLUGDATABUFFER_H */
