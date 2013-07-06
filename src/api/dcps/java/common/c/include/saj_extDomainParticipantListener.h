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
#ifndef SAJ_EXTDOMAINPARTICIPANTLISTENER_H
#define SAJ_EXTDOMAINPARTICIPANTLISTENER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "saj_listener.h"

struct gapi_domainParticipantListener*
saj_extDomainParticipantListenerNew(
    JNIEnv *env,
    jobject listener);

#ifdef __cplusplus
}
#endif
#endif
