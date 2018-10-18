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

#ifndef D__CAPABILITYLISTENER_H_
#define D__CAPABILITYLISTENER_H_

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

d_capabilityListener d_capabilityListenerNew   (d_subscriber subscriber);
void                 d_capabilityListenerFree  (d_capabilityListener listener);
c_bool               d_capabilityListenerStart (d_capabilityListener listener);
c_bool               d_capabilityListenerStop  (d_capabilityListener listener);

#if defined (__cplusplus)
}
#endif

#endif /* D__CAPABILITYLISTENER_H_ */
