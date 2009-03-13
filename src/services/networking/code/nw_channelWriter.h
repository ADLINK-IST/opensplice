
#ifndef NW_CHANNELWRITER_H
#define NW_CHANNELWRITER_H

#include "nw_commonTypes.h"
#include "nw_channel.h"   /* for nw_sendChannel */
#include "u_networkReader.h" /* for u_networkReader */

NW_CLASS(nw_channelWriter);  /* extends from nw_channelUser */

nw_channelWriter  nw_channelWriterNew(const char *serviceName,
                                      const char *pathName,
                                      nw_sendChannel sendChannel,
                                      u_networkReader reader);
                                      
#endif /* NW_CHANNELWRITER_H */

