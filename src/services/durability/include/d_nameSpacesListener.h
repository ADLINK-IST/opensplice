
#ifndef D_NAMESPACESLISTENER_H
#define D_NAMESPACESLISTENER_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_nameSpacesListener(l) ((d_nameSpacesListener)(l))

d_nameSpacesListener    d_nameSpacesListenerNew     (d_subscriber subscriber);

void                    d_nameSpacesListenerFree    (d_nameSpacesListener listener);

c_bool                  d_nameSpacesListenerStart   (d_nameSpacesListener listener);

c_bool                  d_nameSpacesListenerStop    (d_nameSpacesListener listener);

#if defined (__cplusplus)
}
#endif


#endif /*D_NAMESPACESLISTENER_H*/
