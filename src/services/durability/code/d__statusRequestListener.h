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

#ifndef D__STATUSREQUESTLISTENER_H
#define D__STATUSREQUESTLISTENER_H

#include "d__types.h"
#include "d__readerListener.h"

#if defined (__cplusplus)
extern "C" {
#endif

C_STRUCT(d_statusRequestListener){
    C_EXTENDS(d_readerListener);
};

void                d_statusRequestListenerInit     (d_statusRequestListener listener,
                                                     d_subscriber subscriber);

void                d_statusRequestListenerDeinit   (d_object object);

void                d_statusRequestListenerAction   (d_listener listener,
                                                     d_message message);

#if defined (__cplusplus)
}
#endif

#endif /* D__STATUSREQUESTLISTENER_H */
