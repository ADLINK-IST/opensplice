
#ifndef D_LISTENER_H
#define D_LISTENER_H

#include "d__types.h"
#include "u_dispatcher.h"

#if defined (__cplusplus)
extern "C" {
#endif

typedef void (*d_listenerAction)(d_listener listener, d_message message);

#define d_listener(l) ((d_listener)(l))


d_listenerAction    d_listenerGetAction     (d_listener listener);

c_bool              d_listenerIsAttached    (d_listener listener);

d_admin             d_listenerGetAdmin      (d_listener listener);

void                d_listenerLock          (d_listener listener);

void                d_listenerUnlock        (d_listener listener);

#if defined (__cplusplus)
}
#endif

#endif /* D_LISTENER_H */
