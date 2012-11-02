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

#ifndef NW_CHANNELUSER_H
#define NW_CHANNELUSER_H

#include "kernelModule.h"      /* for v_group */
#include "nw_commonTypes.h"

NW_CLASS(nw_channelUser); /* extends from nw_runnable */

void        nw_channelUserNotifyNewGroup(
                nw_channelUser channelUser,
                v_networkReaderEntry entry);


#endif /* NW_CHANNELUSER_H */

