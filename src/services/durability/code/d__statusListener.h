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

#ifndef D__STATUSLISTENER_H
#define D__STATUSLISTENER_H

#include "d__types.h"
#include "d__readerListener.h"

#if defined (__cplusplus)
extern "C" {
#endif

C_STRUCT(d_statusListener){
    C_EXTENDS(d_readerListener);
    d_action cleanupAction;
};

void                d_statusListenerInit            (d_statusListener listener,
                                                     d_subscriber subscriber);

void                d_statusListenerDeinit          (d_object object);

void                d_statusListenerAction          (d_listener listener,
                                                     d_message message);

void*               d_statusListenerFellowCleanup   (void* userData);

#if defined (__cplusplus)
}
#endif

#endif /* D__STATUSLISTENER_H */
