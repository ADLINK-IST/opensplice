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

#ifndef NW_PLUGCHANNEL_H
#define NW_PLUGCHANNEL_H

#include "nw_plugTypes.h"

NW_CLASS(nw_plugChannel);

void                 nw_plugChannelFree(
                         nw_plugChannel channel);

/* PlugChannel class operations */
nw_seqNr             nw_plugChannelGetId(
                         nw_plugChannel channel);

nw_reliabilityKind   nw_plugChannelGetReliabilityOffered(
                         nw_plugChannel channel);

nw_priorityKind      nw_plugChannelGetPriorityOffered(
                         nw_plugChannel channel);


void                 nw_plugChannelNotifyNodeStarted(
                         nw_plugChannel channel,
                         nw_networkId networkId,
                         os_sockaddr_storage address);

void                 nw_plugChannelNotifyNodeStopped(
                         nw_plugChannel channel,
                         nw_networkId networkId,
                         os_sockaddr_storage address);

void                 nw_plugChannelNotifyNodeDied(
                         nw_plugChannel channel,
                         nw_networkId networkId,
                         os_sockaddr_storage address);

void                 nw_plugChannelNotifyGpAdd(
                         nw_plugChannel channel,
                         nw_networkId networkId,
                         os_sockaddr_storage address);

void                 nw_plugChannelNotifyGpAddList(
                        nw_plugChannel channel,
                        c_string probelist);


void                 nw_plugChannelNotifyGpRemove(
                         nw_plugChannel channel,
                         nw_networkId networkId,
                         os_sockaddr_storage address);

#endif /* NW_PLUGCHANNEL_H */

