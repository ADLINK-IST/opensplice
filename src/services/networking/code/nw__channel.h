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

#ifndef NW__CHANNEL_H
#define NW__CHANNEL_H

#include "nw_channel.h"
#include "nw_bridge.h"

/* This file contains the protected constructors for receive and send channels */



/* receiveChannel */

nw_receiveChannel nw_receiveChannelNew(
                      nw_bridge owningBridge,
                      nw_seqNr channelID);

         
/* sendChannel */

nw_sendChannel    nw_sendChannelNew(
                      nw_bridge owningBridge,
                      nw_seqNr channelID);


#endif /* NW__CHANNEL_H */

