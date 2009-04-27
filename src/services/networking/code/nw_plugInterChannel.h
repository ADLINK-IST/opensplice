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

#ifndef NW_PLUGINTERCHANNEL_H
#define NW_PLUGINTERCHANNEL_H

#include "nw_plugTypes.h"
#include "nw_misc.h"

NW_CLASS(nw_plugInterChannel);

void    nw_plugInterChannelIncarnate(
            nw_plugInterChannel *interChannel,
            const char *pathName);
                        
void    nw_plugInterChannelExcarnate(
            nw_plugInterChannel *interChannel);

void    nw_plugInterChannelPostDataReceivedMessage(
            nw_plugInterChannel plugInterChannel,
            nw_seqNr sendingNodeId,
            nw_partitionId sendingPartitionId,
            nw_address sendingNodeAddress,
            nw_seqNr packetNr,
            nw_length currentRecvBuffer);

nw_bool nw_plugInterChannelProcessDataReceivedMessage(
            nw_plugInterChannel plugInterChannel,
            nw_seqNr *sendingNodeId,
            nw_partitionId *sendingPartitionId,
            nw_address *sendingNodeAddress,
            nw_seqNr *packetNr,
            nw_length *currentRecvBuffer);
            
void    nw_plugInterChannelPostAckReceivedMessage(
            nw_plugInterChannel plugInterChannel,
            nw_seqNr sendingNodeId,
            nw_partitionId sendingPartitionId,
            nw_address sendingNodeAddress,
            nw_seqNr startingNr,
            nw_seqNr closingNr,
            nw_length remoteRecvBuffer);

nw_bool nw_plugInterChannelProcessAckReceivedMessage(
            nw_plugInterChannel plugInterChannel,
            nw_seqNr *sendingNodeId,
            nw_partitionId *sendingPartitionId,
            nw_address *sendingNodeAddress,
            nw_seqNr *startingNr,
            nw_seqNr *closingNr,
            nw_length *remoteRecvBuffer);

#endif /*NW_PLUGINTERCHANNEL_H*/
