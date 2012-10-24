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
#ifndef NW_PLUGNETWORK_H
#define NW_PLUGNETWORK_H

#include "nw_plugTypes.h"
#include "nw_plugChannel.h"

NW_CLASS(nw_plugNetwork);

/* NetworkPlug class operations */

nw_plugNetwork nw_plugNetworkIncarnate(
                   nw_networkId networkId);

void           nw_plugNetworkExcarnate(
                   nw_plugNetwork network);

nw_plugChannel nw_plugNetworkNewReceiveChannel(
                   nw_plugNetwork network,
                   const char *pathName,
                   nw_onFatalCallBack onFatal,
                   c_voidp onFatalUsrData);

nw_plugChannel nw_plugNetworkNewSendChannel(
                   nw_plugNetwork network,
                   const char *pathName,
                   nw_onFatalCallBack onFatal,
                   c_voidp onFatalUsrData);

nw_seqNr       nw_plugNetworkGetNetworkId(
                   nw_plugNetwork network);

nw_seqNr       nw_plugNetworkGetMaxChannelId(
                   nw_plugNetwork network);


#endif /* NW_PLUGNETWORK_H */

