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

#ifndef D_GROUPREMOTELISTENER_H
#define D_GROUPREMOTELISTENER_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_groupRemoteListener(l) ((d_groupRemoteListener)(l))

d_groupRemoteListener   d_groupRemoteListenerNew                    (d_subscriber subscriber);

void                    d_groupRemoteListenerFree                   (d_groupRemoteListener listener);

c_bool                  d_groupRemoteListenerStart                  (d_groupRemoteListener listener);

c_bool                  d_groupRemoteListenerStop                   (d_groupRemoteListener listener);

c_bool                  d_groupRemoteListenerAreRemoteGroupsHandled (d_groupRemoteListener listener);

#if defined (__cplusplus)
}
#endif

#endif /* D_GROUPREMOTELISTENER_H */
