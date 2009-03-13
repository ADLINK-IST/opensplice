#ifndef SAJ_DOMAINPARTICIPANTLISTENER_H
#define SAJ_DOMAINPARTICIPANTLISTENER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "saj_listener.h"

struct gapi_domainParticipantListener*
saj_domainParticipantListenerNew(
    JNIEnv *env,
    jobject listener);

#ifdef __cplusplus
}
#endif
#endif
