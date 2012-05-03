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
#ifndef NW__PLUGCONTROLBUFFER_H
#define NW__PLUGCONTROLBUFFER_H

#include "nw__plugNetwork.h" /* for HtoN and NtoH */
#include "nw__plugBuffer.h"

/* To be used by nw_plugChannel descendants only.
 * These structs are privately public for quick access, to avoid
 * the overhead of extra function calls */

#define nw_plugControlBuffer(o) ((nw_plugControlBuffer)(o))

#define NW_PLUGCONTROLBUFFER_ALIGNMENT      (4)
#define NW_PLUGCONTROLBUFFER_DATA_ALIGNMENT (4)
#define NW_PLUGCONTROLBUFFER_HEADERSIZE     (sizeof(NW_STRUCT(nw_plugControlBuffer)))
#define UI(val) ((os_address)(val))

#define NW_PLUGCONTROLBUFFER_FIRSTMESSAGE(buffer) \
    ((nw_plugControlMessage)(UI(buffer) + NW_PLUGCONTROLBUFFER_HEADERSIZE))
#define NW_PLUGCONTROLBUFFER_FIRSTALTMESSAGE(buffer) \
        ((nw_plugControlAltMessage)(UI(buffer) + NW_PLUGCONTROLBUFFER_HEADERSIZE))

/* Important note: make sure that this message has the same memory layout on all
 *                 supported platforms. endianness may differ but alignment
 *                 needs to match */
NW_CLASS(nw_plugControlAltMessage);
NW_STRUCT(nw_plugControlAltMessage) {
    /* this layout is used for tthe DataAnounce and DataRequest message,
     * indicated by special values for the RecvBufferInUse field (see below)
     */
    nw_seqNr diedNodeId;
    nw_partitionId partitionId;
    nw_seqNr firstNr;
    nw_seqNr lastNr;
};

/* Important note: make sure that this message has the same memory layout on all
 *                 supported platforms. endianness may differ but alignment
 *                 needs to match */
NW_CLASS(nw_plugControlMessage);
NW_STRUCT(nw_plugControlMessage) {
    /* For now, only ack messages are known */
    nw_partitionId partitionId;
    nw_seqNr startingNr;
    nw_seqNr closingNr;
};

/* Important note: make sure that this message has the same memory layout on all
 *                 supported platforms. endianness may differ but alignment
 *                 needs to match */
/* Data announce messages and data request messages, as used un the mutli-node reliability functionality
 * are indicated by special values of the RecvBufferInUse - field:
 * -1 indicated a Data Announce message
 * -2 indicates a Data Request message
 * In both cases the contained messages use the nw_plugControlAltMessage layout
 */
NW_CLASS(nw_plugControlBuffer);

/**
* @extends nw_plugBuffer_s
*/
NW_STRUCT(nw_plugControlBuffer) {
    NW_EXTENDS(nw_plugBuffer);
    /* The number of messages within this fragment  */
    nw_seqNr nrOfMessages;
    /* channelstatus: number of fragmentbuffers in use by the receive thread */
    nw_length RecvBufferInUse;
    /* Stretchy array of nw_plugControlMessage */
    /* NW_STRUCT(nw_plugControlMessage) messages[] */
};

#define NW_CONTROL_MESSAGE_SIZE (sizeof(NW_STRUCT(nw_plugControlMessage)))
#define NW_CONTROL_ALTMESSAGE_SIZE (sizeof(NW_STRUCT(nw_plugControlAltMessage)))

#define NW_CONTROL_TAG_DATA_ANNOUNCE (-1)
#define NW_CONTROL_TAG_DATA_REQUEST (-2)

/* Getters/Setters */

#define nw_plugControlBufferGetNrOfMessages(buffer)                  nw_plugNetworkToHost(buffer->nrOfMessages)
#define nw_plugControlBufferSetNrOfMessages(buffer, value)           buffer->nrOfMessages = nw_plugHostToNetwork(value)

#define nw_plugControlBufferGetRecvBufferInUse(buffer)               nw_plugNetworkToHost(buffer->RecvBufferInUse)
#define nw_plugControlBufferSetRecvBufferInUse(buffer, value)        buffer->RecvBufferInUse = nw_plugHostToNetwork(value)

#define nw_plugControlMessageGetPartitionId(message)                 nw_plugNetworkToHost(message->partitionId)
#define nw_plugControlMessageSetPartitionId(message, value)          message->partitionId = nw_plugHostToNetwork(value)

#define nw_plugControlMessageGetStartingNr(message)                  nw_plugNetworkToHost(message->startingNr)
#define nw_plugControlMessageSetStartingNr(message, value)           message->startingNr = nw_plugHostToNetwork(value)

#define nw_plugControlMessageGetClosingNr(message)                   nw_plugNetworkToHost(message->closingNr)
#define nw_plugControlMessageSetClosingNr(message, value)            message->closingNr = nw_plugHostToNetwork(value)

/* for the alternative layout */
#define nw_plugControlAltMessageGetDiedNodeId(message)               nw_plugNetworkToHost(message->diedNodeId)
#define nw_plugControlAltMessageSetDiedNodeId(message, value)        message->diedNodeId = nw_plugHostToNetwork(value)

#define nw_plugControlAltMessageGetPartitionId(message)              nw_plugNetworkToHost(message->partitionId)
#define nw_plugControlAltMessageSetPartitionId(message, value)       message->partitionId = nw_plugHostToNetwork(value)

#define nw_plugControlAltMessageGetFirstNr(message)                  nw_plugNetworkToHost(message->firstNr)
#define nw_plugControlAltMessageSetFirstNr(message, value)           message->firstNr = nw_plugHostToNetwork(value)

#define nw_plugControlAltMessageGetLastNr(message)                   nw_plugNetworkToHost(message->lastNr)
#define nw_plugControlAltMessageSetLastNr(message, value)            message->lastNr = nw_plugHostToNetwork(value)

/* Buffer-analysis functions */

nw_plugControlMessage nw_plugControlBufferGetNextMessage(
                          nw_plugControlBuffer buffer,
                          nw_plugControlMessage prevMessage,
                          nw_bool *more);

nw_plugControlAltMessage nw_plugControlBufferGetNextAltMessage(
                          nw_plugControlBuffer buffer,
                          nw_plugControlAltMessage prevMessage,
                          nw_bool *more);

#endif /* NW__PLUGCONTROLBUFFER_H */

