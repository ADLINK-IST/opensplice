
#ifndef D__GROUPREMOTELISTENER_H
#define D__GROUPREMOTELISTENER_H

#include "d__types.h"
#include "d__readerListener.h"

#if defined (__cplusplus)
extern "C" {
#endif

C_STRUCT(d_groupRemoteListener){
    C_EXTENDS(d_readerListener);
    d_groupCreationQueue groupCreationQueue;
};

void                d_groupRemoteListenerInit   (d_groupRemoteListener listener,
                                                 d_subscriber subscriber);

void                d_groupRemoteListenerDeinit (d_object object);

void                d_groupRemoteListenerAction (d_listener listener,
                                                 d_message message);

#if defined (__cplusplus)
}
#endif

#endif /* D__GROUPREMOTELISTENER_H */
