
#ifndef D_EVENTLISTENER_H
#define D_EVENTLISTENER_H

#if defined (__cplusplus)
extern "C" {
#endif

#define D_GROUP_LOCAL_NEW      (0x0001U << 0)
#define D_FELLOW_NEW           (0x0001U << 1)
#define D_FELLOW_REMOVED       (0x0001U << 2)
#define D_FELLOW_LOST          (0x0001U << 3)

#define d_eventListener(l) ((d_eventListener)(l))

typedef c_bool      (*d_eventListenerFunc)      (c_ulong event, 
                                                 d_fellow fellow, 
                                                 d_group group, 
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
