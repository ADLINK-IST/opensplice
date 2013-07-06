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

#ifndef D_READERLISTENER_H
#define D_READERLISTENER_H

#include "d_listener.h"
#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_readerListener(l) ((d_readerListener)(l))

void                d_readerListenerInit        (d_readerListener listener,
                                                 d_listenerAction action,
                                                 d_subscriber subscriber,
                                                 const c_char* topicName,
                                                 const c_char* fieldName,
                                                 v_reliabilityKind reliability,
                                                 v_historyQosKind historyKind,
                                                 c_long historyDepth,
                                                 os_threadAttr attr,
                                                 d_objectDeinitFunc deinit);

void                d_readerListenerFree        (d_readerListener listener);

#if defined (__cplusplus)
}
#endif

#endif /* D_READERLISTENER_H */
