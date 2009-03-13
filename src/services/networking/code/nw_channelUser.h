
#ifndef NW_CHANNELUSER_H
#define NW_CHANNELUSER_H

#include "kernelModule.h"      /* for v_group */
#include "nw_commonTypes.h"

NW_CLASS(nw_channelUser); /* extends from nw_runnable */

void        nw_channelUserNotifyNewGroup(
                nw_channelUser channelUser,
                v_networkReaderEntry entry);


#endif /* NW_CHANNELUSER_H */

