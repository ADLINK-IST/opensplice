#ifndef D_DELETEDATALISTENER_H
#define D_DELETEDATALISTENER_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_deleteDataListener(l) ((d_deleteDataListener)(l))

d_deleteDataListener    d_deleteDataListenerNew     (d_subscriber subscriber);

void                    d_deleteDataListenerFree    (d_deleteDataListener listener);

c_bool                  d_deleteDataListenerStart   (d_deleteDataListener listener);

c_bool                  d_deleteDataListenerStop    (d_deleteDataListener listener);

#if defined (__cplusplus)
}
#endif

#endif /*D_DELETEDATALISTENER_H*/
