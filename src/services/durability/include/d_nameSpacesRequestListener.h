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

#ifndef D_NAMESPACESREQUESTLISTENER_H
#define D_NAMESPACESREQUESTLISTENER_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_nameSpacesRequestListener(l) ((d_nameSpacesRequestListener)(l))

d_nameSpacesRequestListener d_nameSpacesRequestListenerNew              (d_subscriber subscriber);

void                        d_nameSpacesRequestListenerFree             (d_nameSpacesRequestListener listener);

c_bool                      d_nameSpacesRequestListenerStart            (d_nameSpacesRequestListener listener);

c_bool                      d_nameSpacesRequestListenerStop             (d_nameSpacesRequestListener listener);

void                        d_nameSpacesRequestListenerReportNameSpaces (d_nameSpacesRequestListener listener);

#if defined (__cplusplus)
}
#endif

#endif /*D_NAMESPACESREQUESTLISTENER_H*/
