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

#ifndef D_STATUSLISTENER_H
#define D_STATUSLISTENER_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_statusListener(l) ((d_statusListener)(l))

d_statusListener    d_statusListenerNew         (d_subscriber subscriber);

void                d_statusListenerFree        (d_statusListener listener);

c_bool              d_statusListenerStart       (d_statusListener listener);

c_bool              d_statusListenerStop        (d_statusListener listener);

#if defined (__cplusplus)
}
#endif

#endif /* D_STATUSLISTENER_H */
