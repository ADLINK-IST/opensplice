
#ifndef D__NAMESPACESREQUESTLISTENER_H
#define D__NAMESPACESREQUESTLISTENER_H

#include "d__types.h"
#include "d__readerListener.h"

#if defined (__cplusplus)
extern "C" {
#endif

C_STRUCT(d_nameSpacesRequestListener){
    C_EXTENDS(d_readerListener);
    c_iter nameSpaces;
};

void                d_nameSpacesRequestListenerInit     (d_nameSpacesRequestListener listener,
                                                         d_subscriber subscriber);

void                d_nameSpacesRequestListenerDeinit   (d_object object);

void                d_nameSpacesRequestListenerAction   (d_listener listener,
                                                         d_message message);

#if defined (__cplusplus)
}
#endif

#endif /*D__NAMESPACESREQUESTLISTENER_H*/
