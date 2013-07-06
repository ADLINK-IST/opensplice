/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

#ifndef D_WAITSET_H
#define D_WAITSET_H

#include "u_dispatcher.h"
#include "u_waitsetEvent.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_waitset(w)       ((d_waitset)(w))
#define d_waitsetEntity(w) ((d_waitsetEntity)(w))

typedef c_ulong (*d_waitsetAction)(u_dispatcher dispatcher, u_waitsetEvent event, c_voidp usrData);

d_waitset           d_waitsetNew            (d_subscriber subscriber,
                                             c_bool runToCompletion,
                                             c_bool timedWait);

void                d_waitsetFree           (d_waitset waitset);

c_bool              d_waitsetAttach         (d_waitset waitset,
                                             d_waitsetEntity we);

c_bool              d_waitsetDetach         (d_waitset waitset,
                                             d_waitsetEntity we);

d_subscriber        d_waitsetGetSubscriber  (d_waitset waitset);

d_waitsetEntity     d_waitsetEntityNew      (const c_char* name,
                                             u_dispatcher dispatcher,
                                             d_waitsetAction action,
                                             c_ulong mask,
                                             os_threadAttr attr,
                                             c_voidp usrData);
                                         
void                d_waitsetEntityFree     (d_waitsetEntity we);

#if defined (__cplusplus)
}
#endif

#endif /*D_WAITSET_H*/
