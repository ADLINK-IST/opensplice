
#ifndef D_STATUSREQUESTLISTENER_H
#define D_STATUSREQUESTLISTENER_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_statusRequestListener(l) ((d_statusRequestListener)(l))

d_statusRequestListener d_statusRequestListenerNew  (d_subscriber subscriber);

void                    d_statusRequestListenerFree (d_statusRequestListener listener);

c_bool                  d_statusRequestListenerStart(d_statusRequestListener listener);

c_bool                  d_statusRequestListenerStop (d_statusRequestListener listener);

#if defined (__cplusplus)
}
#endif

#endif /* D_STATUSREQUESTLISTENER_H */
