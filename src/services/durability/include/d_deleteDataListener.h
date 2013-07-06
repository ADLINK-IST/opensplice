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
#ifndef D_DELETEDATALISTENER_H
#define D_DELETEDATALISTENER_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_deleteDataListener(l) ((d_deleteDataListener)(l))

d_deleteDataListener    d_deleteDataListenerNew     (d_subscriber subscriber);

void                    d_deleteDataListenerFree    (d_deleteDataListener listener);

c_bool                  d_deleteDataListenerStart   (d_deleteDataListener listener);

c_bool                  d_deleteDataListenerStop    (d_deleteDataListener listener);

#if defined (__cplusplus)
}
#endif

#endif /*D_DELETEDATALISTENER_H*/
