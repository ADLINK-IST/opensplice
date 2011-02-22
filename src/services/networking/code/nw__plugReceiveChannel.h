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
#ifndef NW__PLUGRECEIVECHANNEL_H_
#define NW__PLUGRECEIVECHANNEL_H_

#include "nw_plugReceiveChannel.h"
#include "nw_plugChannel.h"
#include "nw_plugPartitions.h"

/* Constructor, to be called by plugNetwork only */

nw_plugChannel nw_plugReceiveChannelNew(
                   nw_seqNr seqNr,
                   nw_networkId nodeId,
                   nw_plugPartitions plugPartitions,
                   nw_userData *userDataPtr,
                   const char *pathName,
                   nw_onFatalCallBack onFatal,
                   c_voidp onFatalUsrData);                   

void nw_plugReceiveChannelFree(
                   nw_plugChannel channel);

nw_partitionId
nw_plugReceiveChannelLookUpPartitionHash(
        nw_plugReceiveChannel channel,
        nw_partitionId partitionHash);

#ifdef _PROFILE_
os_time *
nw_plugReceiveChannelLastHolderTimestamps (
    nw_plugChannel channel);
os_time *
nw_plugReceiveChannelLastBufferTimestamps (
    nw_plugChannel channel);
#endif

#endif /*NW__PLUGRECEIVECHANNEL_H_*/
