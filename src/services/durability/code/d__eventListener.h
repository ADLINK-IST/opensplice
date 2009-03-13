
#ifndef D__EVENTLISTENER_H
#define D__EVENTLISTENER_H

#include "d__types.h"
#include "d_eventListener.h"

#if defined (__cplusplus)
extern "C" {
#endif

C_STRUCT(d_eventListener){
    C_EXTENDS(d_object);
    c_ulong interest;
    d_eventListenerFunc func;
    c_voidp args;
};

void
d_eventListenerDeinit(
    d_object object);

#if defined (__cplusplus)
}
#endif

#endif /*D__EVENTLISTENER_H*/
