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

#ifndef D__SUBSCRIBER_H
#define D__SUBSCRIBER_H

#include "d__types.h"
#include "u_user.h"

#if defined (__cplusplus)
extern "C" {
#endif

C_STRUCT(d_subscriber){
    C_EXTENDS(d_object);
    d_admin                     admin;
    u_subscriber                subscriber;
    u_subscriber                persistentSubscriber;
    d_waitset                   waitset;

    d_statusListener            statusListener;
    d_groupRemoteListener       groupRemoteListener;
    d_groupLocalListener        groupLocalListener;
    d_groupsRequestListener     groupsRequestListener;
    d_sampleRequestListener     sampleRequestListener;
    d_sampleChainListener       sampleChainListener;
    d_nameSpacesRequestListener nameSpacesRequestListener;
    d_nameSpacesListener        nameSpacesListener;
    d_persistentDataListener    persistentDataListener;
    d_deleteDataListener        deleteDataListener;

    d_store                     persistentStore;
};

void        d_subscriberDeinit      (d_object object);

#if defined (__cplusplus)
}
#endif

#endif /* D__SUBSCRIBER_H */
