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
