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
#ifndef NW_BRIDGE_H
#define NW_BRIDGE_H

#include "os_time.h"
#include "c_metabase.h"    /* For c_type */
#include "nw_plugTypes.h"
#include "nw_channel.h"
#include "kernelModule.h"  /* For v_message and v_networkHashValue */

NW_CLASS(nw_bridge);

typedef enum nw_channelType_e {
    NW_CT_INVALID,     /* avoids uninitialized values */
    NW_CT_BEST_EFFORT /* not reliable at all */
    /* In the near future:
     * NW_CT_RELIABLE,
     * NW_CT_RELIABLE_P2P,
     * NW_CT_GUARDED_P2P */
} nw_channelType;


/* Bridge class operations */
nw_bridge         nw_bridgeNew(v_networkId nodeId);

void              nw_bridgeFree(
                      nw_bridge bridge);

/* Returns the number of bytes written */
c_ulong           nw_bridgeWrite(
                      nw_bridge bridge,
                      nw_seqNr channelId,
                      v_networkPartitionId partitionId,
                      v_message message,
                      v_networkHashValue hashValue,
                      const char *partitionName,
                      const char *topicName,
                      nw_signedLength *bytesLeft,
                      plugSendStatistics pss);

nw_bool           nw_bridgeFlush(
                      nw_bridge bridge,
                      nw_seqNr channelId,
                      nw_bool all,
                      nw_signedLength *bytesLeft,
                      plugSendStatistics pss);

void              nw_bridgePeriodicAction(
                      nw_bridge bridge,
                      nw_seqNr channelId,
                      nw_signedLength *bytesLeft,
                      plugSendStatistics pss);

typedef c_voidp nw_typeLookupArg;

/* Note: can not use the normal notation c_type here because of the
 *       parentheses that follow; this will be expanded by the preprocessor
 *       since c_type(o) is a macro... */
typedef NW_STRUCT(c_type)*   (*nw_typeLookupAction) (
                      v_networkHashValue hashValue,
                      const char *partitionName,
                      const char *topicName,
                      nw_typeLookupArg arg);

void              nw_bridgeRead(
                      nw_bridge bridge,
                      nw_seqNr channelId,
                      v_message *message,
                      const nw_typeLookupAction typeLookupAction,
                      nw_typeLookupArg typeLookupArg,
                      plugReceiveStatistics prs)
;

void              nw_bridgeTrigger(
                      nw_bridge bridge,
                      nw_seqNr channelId);

nw_receiveChannel nw_bridgeNewReceiveChannel(
                      nw_bridge bridge,
                      const char *pathName,
                      nw_onFatalCallBack onFatal,
                      c_voidp onFatalUsrData);

nw_sendChannel    nw_bridgeNewSendChannel(
                      nw_bridge bridge,
                      const char *pathName,
                      nw_onFatalCallBack onFatal,
                      c_voidp onFatalUsrData);

void              nw_bridgeFreeChannel(
                      nw_bridge bridge,
                      nw_seqNr channelId);


/* Notification of node states */

void              nw_bridgeNotifyNodeStarted(
                      nw_bridge bridge,
                      v_networkId networkId,
                      os_sockaddr_storage address);

void              nw_bridgeNotifyNodeStopped(
                      nw_bridge bridge,
                      v_networkId networkId,
                      os_sockaddr_storage address);

void              nw_bridgeNotifyNodeDied(
                      nw_bridge bridge,
                      v_networkId networkId,
                      os_sockaddr_storage address);

void              nw_bridgeNotifyGpAdd(
                      nw_bridge bridge,
                      v_networkId networkId,
                      os_sockaddr_storage address);

void              nw_bridgeNotifyGpRemove(
                      nw_bridge bridge,
                      v_networkId networkId,
                      os_sockaddr_storage address);


/* Convenience */

nw_channelType    nw_channelTypeFromString(
                      const char *string);

nw_globalId       v_gidToGlobalId(v_gid gid);
v_gid             v_gidFromGlobalId(nw_globalId globalId);

#endif /* NW_BRIDGE_H */


