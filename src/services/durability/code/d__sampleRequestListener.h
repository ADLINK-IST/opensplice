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

#ifndef D__SAMPLEREQUESTLISTENER_H
#define D__SAMPLEREQUESTLISTENER_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

C_CLASS(d_sampleRequestHelper);

d_sampleRequestListener d_sampleRequestListenerNew     (d_subscriber subscriber);
void                    d_sampleRequestListenerFree    (d_sampleRequestListener listener);
c_bool                  d_sampleRequestListenerStart   (d_sampleRequestListener listener);
c_bool                  d_sampleRequestListenerStop    (d_sampleRequestListener listener);

#if defined (__cplusplus)
}
#endif

#endif /* D__SAMPLEREQUESTLISTENER_H */
