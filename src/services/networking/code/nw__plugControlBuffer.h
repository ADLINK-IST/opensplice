/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
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
#define UI(val) ((nw_length)(val))

#define NW_PLUGCONTROLBUFFER_FIRSTMESSAGE(buffer) \
    ((nw_plugControlMessage)(UI(buffer) + NW_PLUGCONTROLBUFFER_HEADERSIZE))


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
NW_CLASS(nw_plugControlBuffer);
NW_STRUCT(nw_plugControlBuffer) {
    NW_EXTENDS(nw_plugBuffer);
    /* The number of messages within this fragment */
    nw_seqNr nrOfMessages;
    /* channelstatus: number of fragmentbuffers in use by the receive thread */
    nw_length RecvBufferInUse;
    /* Stretchy array of nw_plugControlMessage */
    /* NW_STRUCT(nw_plugControlMessage) messages[] */
};

#define NW_CONTROL_MESSAGE_SIZE (sizeof(NW_STRUCT(nw_plugControlMessage)))

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


/* Buffer-analysis functions */

nw_plugControlMessage nw_plugControlBufferGetNextMessage(
                          nw_plugControlBuffer buffer,
                          nw_plugControlMessage prevMessage,
                          nw_bool *more);

#endif /* NW__PLUGCONTROLBUFFER_H */

