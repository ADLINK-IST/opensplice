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

#ifndef D__GROUPSREQUESTLISTENER_H
#define D__GROUPSREQUESTLISTENER_H

#include "d__types.h"
#include "d__readerListener.h"

#if defined (__cplusplus)
extern "C" {
#endif

d_groupsRequestListener d_groupsRequestListenerNew   (d_subscriber subscriber);

void                    d_groupsRequestListenerFree  (d_groupsRequestListener listener);

c_bool                  d_groupsRequestListenerStart (d_groupsRequestListener listener);

c_bool                  d_groupsRequestListenerStop  (d_groupsRequestListener listener);

#if defined (__cplusplus)
}
#endif

#endif /* D__GROUPREQUESTLISTENER_H */
