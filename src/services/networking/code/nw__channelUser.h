/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

#ifndef NW__CHANNELUSER_H
#define NW__CHANNELUSER_H

#include "c_typebase.h"
#include "nw__runnable.h"
#include "nw_channelUser.h"
#include "kernelModule.h"
#include "u_networkReader.h"

NW_CLASS(nw_ringBuffer);


NW_STRUCT(nw_channelUser) {
    C_EXTENDS(nw_runnable);
    u_networkReader reader;  
    nw_ringBuffer messageBuffer;
};

/* Protected functions */

void        nw_channelUserInitialize(
                nw_channelUser channelUser,
                const char *name,
                const char *pathName,
                u_networkReader reader,
                const nw_runnableMainFunc runnableMainFunc,
                const nw_runnableTriggerFunc triggerFunc,
                const nw_runnableFinalizeFunc finalizeFunc);
                
void        nw_channelUserFinalize(
                nw_channelUser channelUser);

c_bool      nw_channelUserRetrieveNewGroup(
                nw_channelUser channelUser,
                v_networkReaderEntry *entry);

#endif /* NW__CHANNELUSER_H */

