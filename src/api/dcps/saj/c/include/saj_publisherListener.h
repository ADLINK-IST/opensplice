#ifndef SAJ_PUBLISHERLISTENER_H
#define SAJ_PUBLISHERLISTENER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "saj_listener.h"

struct gapi_publisherListener*
saj_publisherListenerNew(
    JNIEnv *env,
    jobject listener);

#ifdef __cplusplus
}
#endif
#endif
