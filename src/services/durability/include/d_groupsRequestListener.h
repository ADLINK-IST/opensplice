
#ifndef D_GROUPSREQUESTLISTENER_H
#define D_GROUPSREQUESTLISTENER_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_groupsRequestListener(l) ((d_groupsRequestListener)(l))

d_groupsRequestListener d_groupsRequestListenerNew   (d_subscriber subscriber);

void                    d_groupsRequestListenerFree  (d_groupsRequestListener listener);

c_bool                  d_groupsRequestListenerStart (d_groupsRequestListener listener);

c_bool                  d_groupsRequestListenerStop  (d_groupsRequestListener listener);

#if defined (__cplusplus)
}
#endif

#endif /* D_GROUPSREQUESTLISTENER_H */
