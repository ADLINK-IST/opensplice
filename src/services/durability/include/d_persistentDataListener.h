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

#ifndef D_PERSISTENTDATALISTENER_H
#define D_PERSISTENTDATALISTENER_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_persistentDataListener(p) ((d_persistentDataListener)(p))

d_persistentDataListener    d_persistentDataListenerNew     (d_subscriber subscriber);

void                        d_persistentDataListenerFree    (d_persistentDataListener listener);

c_bool                      d_persistentDataListenerStart   (d_persistentDataListener listener);

c_bool                      d_persistentDataListenerStop    (d_persistentDataListener listener);


#if defined (__cplusplus)
}
#endif

#endif /*D_PERSISTENTDATALISTENER_H*/
