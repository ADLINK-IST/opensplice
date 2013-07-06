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
#ifndef D__DELETEDATALISTENER_H
#define D__DELETEDATALISTENER_H

#include "d__types.h"
#include "d__readerListener.h"

#if defined (__cplusplus)
extern "C" {
#endif

C_STRUCT(d_deleteDataListener){
    C_EXTENDS(d_readerListener);
};

void                d_deleteDataListenerInit    (d_deleteDataListener listener,
                                                 d_subscriber subscriber);

void                d_deleteDataListenerDeinit  (d_object object);

void                d_deleteDataListenerAction  (d_listener listener,
                                                 d_message message);

#if defined (__cplusplus)
}
#endif

#endif /*D__DELETEDATALISTENER_H*/
