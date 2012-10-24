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

#ifndef NW_CHANNELREADER_H
#define NW_CHANNELREADER_H

#include "nw_commonTypes.h" /* for NW_CLASS  */
#include "nw_channel.h" /* for sendChannel */
#include "u_networkReader.h" /* for u_networkReader */

NW_CLASS(nw_channelReader); /* extends from nw_channelUser */

nw_channelReader  nw_channelReaderNew(
                      const char *pathName,
                      nw_receiveChannel receiveChannel,
                      u_networkReader reader,
                      c_ulong stat_channel_id);

#endif /* NW_CHANNELREADER_H */

