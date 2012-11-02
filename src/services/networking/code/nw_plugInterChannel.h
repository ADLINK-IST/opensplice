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

#ifndef NW_PLUGINTERCHANNEL_H
#define NW_PLUGINTERCHANNEL_H

#include "nw_plugTypes.h"
#include "nw_misc.h"
#include "nw__plugDataBuffer.h"

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
            os_sockaddr_storage sendingNodeAddress,
            nw_seqNr packetNr,
            nw_length currentRecvBuffer);

nw_bool nw_plugInterChannelProcessDataReceivedMessage(
            nw_plugInterChannel plugInterChannel,
            nw_seqNr *sendingNodeId,
            nw_partitionId *sendingPartitionId,
            os_sockaddr_storage *sendingNodeAddress,
            nw_seqNr *packetNr,
            nw_length *currentRecvBuffer);

void    nw_plugInterChannelPostBackupReceivedMessage(
            nw_plugInterChannel plugInterChannel,
            nw_seqNr sendingNodeId,
            nw_partitionId sendingPartitionId,
            os_sockaddr_storage sendingNodeAddress,
            nw_plugDataBuffer backupBuffer,
            nw_length currentRecvBuffer);

nw_bool nw_plugInterChannelProcessBackupReceivedMessage(
            nw_plugInterChannel plugInterChannel,
            nw_seqNr *sendingNodeId,
            nw_partitionId *sendingPartitionId,
            os_sockaddr_storage *sendingNodeAddress,
            nw_plugDataBuffer *backupBuffer,
            nw_length *currentRecvBuffer);

void    nw_plugInterChannelPostAckReceivedMessage(
            nw_plugInterChannel plugInterChannel,
            nw_seqNr sendingNodeId,
            nw_partitionId sendingPartitionId,
            os_sockaddr_storage sendingNodeAddress,
            nw_seqNr startingNr,
            nw_seqNr closingNr,
            nw_length remoteRecvBuffer);

nw_bool nw_plugInterChannelProcessAckReceivedMessage(
            nw_plugInterChannel plugInterChannel,
            nw_seqNr *sendingNodeId,
            nw_partitionId *sendingPartitionId,
            os_sockaddr_storage *sendingNodeAddress,
            nw_seqNr *startingNr,
            nw_seqNr *closingNr,
            nw_length *remoteRecvBuffer);

void nw_plugInterChannelPostDataAnnounceMessage(
            nw_plugInterChannel plugInterChannel,
            nw_seqNr diedNodeId,
            nw_partitionId partitionId,
            nw_seqNr firstNr,
            nw_seqNr lastNr);

nw_bool nw_plugInterChannelProcessDataAnnounceMessage(
            nw_plugInterChannel plugInterChannel,
            nw_seqNr *diedNodeId,
            nw_partitionId *partitionId,
            nw_seqNr *firstNr,
            nw_seqNr *lastNr);

void nw_plugInterChannelPostDataRequestMessage(
            nw_plugInterChannel plugInterChannel,
            nw_seqNr servingNodeId,
            os_sockaddr_storage servingNodeAddress,
            nw_seqNr diedNodeId,
            nw_partitionId partitionId,
            nw_seqNr firstNr,
            nw_seqNr lastNr);

nw_bool nw_plugInterChannelProcessDataRequestMessage(
            nw_plugInterChannel plugInterChannel,
            nw_seqNr *servingNodeId,
            os_sockaddr_storage *servingNodeAddress,
            nw_seqNr *diedNodeId,
            nw_partitionId *partitionId,
            nw_seqNr *firstNr,
            nw_seqNr *lastNr);

void nw_plugInterChannelSetTrigger(
            nw_plugInterChannel plugInterChannel);

nw_bool nw_plugInterChannelGetTrigger(
            nw_plugInterChannel plugInterChannel);



#endif /*NW_PLUGINTERCHANNEL_H*/
