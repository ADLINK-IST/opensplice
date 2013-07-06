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
