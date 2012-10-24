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

#ifndef NW_CHANNELWRITER_H
#define NW_CHANNELWRITER_H

#include "nw_commonTypes.h"
#include "nw_channel.h"   /* for nw_sendChannel */
#include "u_networkReader.h" /* for u_networkReader */
#include "nw_plugSendChannel.h"

NW_CLASS(nw_channelWriter);  /* extends from nw_channelUser */


nw_channelWriter  nw_channelWriterNew(const char *serviceName,
                                      const char *pathName,
                                      nw_sendChannel sendChannel,
                                      u_networkReader reader,
                                      c_ulong stat_channel_id);
#endif /* NW_CHANNELWRITER_H */

