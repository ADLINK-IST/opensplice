/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#ifndef D__NAMESPACESREQUESTLISTENER_H
#define D__NAMESPACESREQUESTLISTENER_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

d_nameSpacesRequestListener d_nameSpacesRequestListenerNew              (d_subscriber subscriber);
void                        d_nameSpacesRequestListenerFree             (d_nameSpacesRequestListener listener);
c_bool                      d_nameSpacesRequestListenerStart            (d_nameSpacesRequestListener listener);
c_bool                      d_nameSpacesRequestListenerStop             (d_nameSpacesRequestListener listener);
void                        d_nameSpacesRequestListenerReportNameSpaces (d_nameSpacesRequestListener listener);

#if defined (__cplusplus)
}
#endif

#endif /* D__NAMESPACESREQUESTLISTENER_H */
