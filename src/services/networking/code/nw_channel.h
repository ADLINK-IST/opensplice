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

#ifndef NW_CHANNEL_H
#define NW_CHANNEL_H

#include "os_time.h"          /* For os_time */
#include "nw_commonTypes.h"   /* For nw_name */
#include "nw_plugTypes.h"     /* For policy-kinds */
#include "kernelModule.h"     /* For v_message and v_topic */
#include "nw_plugSendChannel.h"
#include "nw_plugReceiveChannel.h"

/*
 * All types have protected constructors
 * called by networkingBridge functions
 */


/**
* @class nw_channel
*/
NW_CLASS(nw_channel);


/* receiveChannel */

NW_CLASS(nw_receiveChannel);

void nw_receiveChannelAddGroup(
         nw_receiveChannel channel,
         v_networkReaderEntry entry);

void nw_receiveChannelTrigger(
         nw_receiveChannel channel);

void nw_receiveChannelFree(
         nw_receiveChannel channel);


/* Reader/Write info for p2p communication */
struct nw_endpointInfo {
   c_ulong messageId;
   v_gid   sender;
   c_bool  sendTo;
   v_gid   receiver; /* Valid only if sendTo == TRUE */
};

typedef c_voidp nw_entryLookupArg;

typedef NW_STRUCT(v_networkReaderEntry)* (*nw_entryLookupAction) (
    v_networkHashValue hashValue,
    const char *partitionName,
    const char *topicName,
    nw_entryLookupArg arg);

/* The entry lookupaction is called at the moment that a message for an
 * unknown partition/topic has been received */
void nw_receiveChannelRead(
         nw_receiveChannel receiveChannel,
         v_message *messagePtr,
         v_networkReaderEntry *entryPtr,
         const nw_entryLookupAction entryLookupAction,
         nw_entryLookupArg entryLookupArg,
         plugReceiveStatistics prs);

/* sendChannel */

NW_CLASS(nw_sendChannel);

void nw_sendChannelFree(
         nw_sendChannel channel);

/* Function returns number of bytes sent */
c_ulong nw_sendChannelWrite(
         nw_sendChannel sendChannel,
         v_networkReaderEntry entry,
         v_message message,
         nw_signedLength *maxBytes,
         plugSendStatistics pss);

nw_bool nw_sendChannelFlush(
            nw_sendChannel sendChannel,
            nw_bool all,
            nw_signedLength *maxBytes,
            plugSendStatistics pss);

void nw_sendChannelPeriodicAction(
         nw_sendChannel sendChannel,
         nw_signedLength *maxBytes,
         plugSendStatistics pss);

#endif /* NW_CHANNEL_H */

