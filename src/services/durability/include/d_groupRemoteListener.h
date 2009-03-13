
#ifndef D_GROUPREMOTELISTENER_H
#define D_GROUPREMOTELISTENER_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_groupRemoteListener(l) ((d_groupRemoteListener)(l))

d_groupRemoteListener   d_groupRemoteListenerNew                    (d_subscriber subscriber);

void                    d_groupRemoteListenerFree                   (d_groupRemoteListener listener);

c_bool                  d_groupRemoteListenerStart                  (d_groupRemoteListener listener);

c_bool                  d_groupRemoteListenerStop                   (d_groupRemoteListener listener);

c_bool                  d_groupRemoteListenerAreRemoteGroupsHandled (d_groupRemoteListener listener);

#if defined (__cplusplus)
}
#endif

#endif /* D_GROUPREMOTELISTENER_H */
