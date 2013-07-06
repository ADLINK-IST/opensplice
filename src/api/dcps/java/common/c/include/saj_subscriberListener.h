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
#ifndef SAJ_SUBSCRIBERLISTENER_H
#define SAJ_SUBSCRIBERLISTENER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "saj_listener.h"

struct gapi_subscriberListener*
saj_subscriberListenerNew(
    JNIEnv *env,
    jobject listener);

void
saj_subscriberListenerOnDataOnReaders(
    void* listenerData, 
    gapi_subscriber subscriber);

#ifdef __cplusplus
}
#endif
#endif
