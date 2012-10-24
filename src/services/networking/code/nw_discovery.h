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

#ifndef NW_DISCOVERY_H
#define NW_DISCOVERY_H

#include "nw_commonTypes.h" /* for NW_CLASS */
#include "nw_plugTypes.h" /* for NW_CLASS */
#include "kernelModule.h"   /* for v_networkId */

NW_CLASS(nw_discoveryWriter);

nw_discoveryWriter nw_discoveryWriterNew(
                       v_networkId networkId,
                       const char *name);

void               nw_discoveryWriterRespondToStartingNode(
                       nw_discoveryWriter discoveryWriter,
                       nw_networkId networkId);


/* Tage definitions used in the dynamic part of the discovery message */
#define nw_disctag_role     (1) /* contant is a string containing the role of the originating node */
#define nw_disctag_dyn_req  (2) /* content is a string representing the scope-expression of requested roles */
#define nw_disctag_dyn_list (3) /* content is an array of os_sockaddr_storage */

/* Note: Extend the onNodeAlive event to include more information about node */
typedef void *nw_discoveryMsgArg;
typedef void (*nw_discoveryAction)(
                 v_networkId networkId,
                 os_sockaddr_storage adress,
                 c_time detectedTime,
                 os_uint32 aliveCount,
                 nw_discoveryMsgArg arg);

NW_CLASS(nw_discoveryReader);

nw_discoveryReader nw_discoveryReaderNew(
                        v_networkId networkId,
                        const char *name,
                        nw_discoveryAction startedAction,
                        nw_discoveryAction stoppedAction,
                        nw_discoveryAction diedAction,
                        nw_discoveryAction gpAddAction,
                        nw_discoveryAction gpRemoveAction,
                        nw_discoveryWriter discoveryWriter,
                        nw_discoveryMsgArg arg);

void               nw_discoveryReaderInitiateCheck(
                       nw_discoveryReader discoveryReader);

#endif /* #ifndef NW_DISCOVERY_H */
