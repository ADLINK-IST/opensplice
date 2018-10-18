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

#ifndef D_NETWORKADDRESS_H
#define D_NETWORKADDRESS_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_networkAddress(a) ((d_networkAddress)(a))

d_networkAddress    d_networkAddressNew             (c_ulong systemId,
                                                     c_ulong localId,
                                                     c_ulong lifecycleId);

void                d_networkAddressFree            (d_networkAddress addr);

c_bool              d_networkAddressIsUnaddressed   (d_networkAddress addr);

d_networkAddress    d_networkAddressUnaddressed     ();

c_bool              d_networkAddressEquals          (d_networkAddress addr1, 
                                                     d_networkAddress addr2);

int                 d_networkAddressCompare         (d_networkAddress addr1,
                                                     d_networkAddress addr2);

#if defined (__cplusplus)
}
#endif

#endif /*D_NETWORKADDRESS_H*/
