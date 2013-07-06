/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

#ifndef D__SAMPLECHAINLISTENER_H
#define D__SAMPLECHAINLISTENER_H

#include "d__types.h"
#include "d__readerListener.h"
#include "os_thread.h"

#if defined (__cplusplus)
extern "C" {
#endif

C_STRUCT(d_sampleChainListener){
    C_EXTENDS(d_readerListener);
    d_table         chains;
    c_iter          chainsWaiting;
    c_ulong         id;
    d_eventListener fellowListener;
    d_actionQueue	resendQueue;
    c_iter          unfulfilledChains;
    d_table         mergeActions;
};

void                d_sampleChainListenerInit               (d_sampleChainListener listener,
                                                             d_subscriber subscriber);

void                d_sampleChainListenerDeinit             (d_object object);

void                d_sampleChainListenerAction             (d_listener listener,
                                                             d_message message);

void                d_sampleChainListenerReportGroup        (d_sampleChainListener listener,
                                                             d_group group);

d_chain             d_sampleChainListenerFindChain          (d_sampleChainListener listener,
                                                             d_sampleChain sampleChain);

c_bool              d_sampleChainListenerCleanupRequests    (d_chain chain,
                                                             c_voidp userData);

c_bool              d_sampleChainListenerCleanupBeads       (d_chainBead bead,
                                                             c_voidp userData);

c_bool              d_sampleChainRequestWrite               (d_fellow fellow,
                                                             c_voidp userData);

c_bool              d_sampleChainListenerNotifyFellowRemoved(c_ulong event,
                                                             d_fellow fellow,
                                                             d_nameSpace ns,
                                                             d_group group,
                                                             c_voidp eventUserData,
                                                             c_voidp userData);

c_bool              d_sampleChainListenerRemoveGroupWithFellows (d_fellow fellow,
                                                                 c_voidp args);

c_bool              d_sampleChainListenerCheckChainComplete (d_sampleChainListener listener,
                                                             d_chain chain);

c_bool              d_sampleChainFindAligner                (d_fellow fellow,
                                                             c_voidp args);

void                d_chainDeinit                           (d_object object);

struct chainCleanup{
    d_admin admin;
    d_sampleChainListener listener;
    d_networkAddress fellow;
    c_iter toRemove;
    c_iter beadsToRemove;
};



#if defined (__cplusplus)
}
#endif

#endif /* D__SAMPLECHAINLISTENER_H */
