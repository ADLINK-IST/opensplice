
#ifndef NW_DISCOVERY_H
#define NW_DISCOVERY_H

#include "nw_commonTypes.h" /* for NW_CLASS */
#include "kernelModule.h"   /* for v_networkId */

NW_CLASS(nw_discoveryWriter);

nw_discoveryWriter nw_discoveryWriterNew(
                       v_networkId networkId,
                       const char *name);
                       
void               nw_discoveryWriterRespondToStartingNode(
                       nw_discoveryWriter discoveryWriter);

/* Note: Extend the onNodeAlive event to include more information about node */
typedef void *nw_discoveryMsgArg;
typedef void (*nw_discoveryAction)(
                 v_networkId networkId,
                 nw_address adress,
                 c_time detectedTime,
                 unsigned int aliveCount,
                 nw_discoveryMsgArg arg);

NW_CLASS(nw_discoveryReader);

nw_discoveryReader nw_discoveryReaderNew(
                       v_networkId networkId,
                       const char *name,
                       nw_discoveryAction startedAction,
                       nw_discoveryAction stoppedAction,
                       nw_discoveryAction diedAction,
                       nw_discoveryMsgArg arg);

void               nw_discoveryReaderInitiateCheck(
                       nw_discoveryReader discoveryReader);

#endif /* #ifndef NW_DISCOVERY_H */
