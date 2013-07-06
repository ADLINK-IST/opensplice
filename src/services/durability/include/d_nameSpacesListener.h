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

#ifndef D_NAMESPACESLISTENER_H
#define D_NAMESPACESLISTENER_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_nameSpacesListener(l) ((d_nameSpacesListener)(l))

d_nameSpacesListener    d_nameSpacesListenerNew     (d_subscriber subscriber);

void                    d_nameSpacesListenerFree    (d_nameSpacesListener listener);

c_bool                  d_nameSpacesListenerStart   (d_nameSpacesListener listener);

c_bool                  d_nameSpacesListenerStop    (d_nameSpacesListener listener);

#if defined (__cplusplus)
}
#endif


#endif /*D_NAMESPACESLISTENER_H*/
