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

#ifndef D_EVENTLISTENER_H
#define D_EVENTLISTENER_H

#if defined (__cplusplus)
extern "C" {
#endif

#define D_NONE                          (0)
#define D_GROUP_LOCAL_NEW               (0x0001U << 0)
#define D_FELLOW_NEW                    (0x0001U << 1)
#define D_FELLOW_REMOVED                (0x0001U << 2)
#define D_FELLOW_LOST                   (0x0001U << 3)
#define D_NAMESPACE_NEW                 (0x0001U << 4)
#define D_NAMESPACE_STATE_CONFLICT      (0x0001U << 5)
#define D_NAMESPACE_MASTER_CONFLICT     (0x0001U << 6)
#define D_NAMESPACE_DELAYED_INITIAL     (0x0001U << 7)

#define d_eventListener(l) ((d_eventListener)(l))

typedef c_bool      (*d_eventListenerFunc)      (c_ulong event, 
                                                 d_fellow fellow, 
                                                 d_nameSpace nameSpace,
                                                 d_group group, 
                                                 c_voidp userData,
                                                 c_voidp args);

d_eventListener     d_eventListenerNew          (c_ulong interest,
                                                 d_eventListenerFunc func,
                                                 c_voidp args);

c_voidp             d_eventListenerGetUserData  (d_eventListener listener);

void                d_eventListenerFree         (d_eventListener listener);

#if defined (__cplusplus)
}
#endif

#endif /*D_EVENTLISTENER_H*/
