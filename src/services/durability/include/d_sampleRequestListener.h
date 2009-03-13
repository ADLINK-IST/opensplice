
#ifndef D_SAMPLEREQUESTLISTENER_H
#define D_SAMPLEREQUESTLISTENER_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_sampleRequestListener(l) ((d_sampleRequestListener)(l))

d_sampleRequestListener d_sampleRequestListenerNew  (d_subscriber subscriber);

void                    d_sampleRequestListenerFree (d_sampleRequestListener listener);

c_bool                  d_sampleRequestListenerStart(d_sampleRequestListener listener);

c_bool                  d_sampleRequestListenerStop (d_sampleRequestListener listener);

#if defined (__cplusplus)
}
#endif

#endif /* D_SAMPLEREQUESTLISTENER_H */
