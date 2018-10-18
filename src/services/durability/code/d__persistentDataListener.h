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

#ifndef D__PERSISTENTDATALISTENER_H
#define D__PERSISTENTDATALISTENER_H

#include "d__types.h"
#include "d__listener.h"
#include "u_groupQueue.h"

#if defined (__cplusplus)
extern "C" {
#endif

d_persistentDataListener d_persistentDataListenerNew (d_subscriber subscriber);

void                     d_persistentDataListenerFree (d_persistentDataListener listener);

c_bool                   d_persistentDataListenerStart (d_persistentDataListener listener);

c_bool                   d_persistentDataListenerStop (d_persistentDataListener listener);

#if defined (__cplusplus)
}
#endif

#endif /* D__PERSISTENTDATALISTENER_H */
