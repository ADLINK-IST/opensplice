
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
                         nw_address address);

void                 nw_plugChannelNotifyNodeStopped(
                         nw_plugChannel channel,
                         nw_networkId networkId,
                         nw_address address);

void                 nw_plugChannelNotifyNodeDied(
                         nw_plugChannel channel,
                         nw_networkId networkId,
                         nw_address address);

#endif /* NW_PLUGCHANNEL_H */

