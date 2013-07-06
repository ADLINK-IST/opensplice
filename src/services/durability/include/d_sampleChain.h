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

#ifndef D_SAMPLECHAIN_H
#define D_SAMPLECHAIN_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_sampleChain(s) ((d_sampleChain)(s))

d_sampleChain   d_sampleChainNew                (d_admin admin,
                                                 const c_char* partition,
                                                 const c_char* topic,
                                                 d_durabilityKind kind,
                                                 d_networkAddress source);

void            d_sampleChainFree               (d_sampleChain sampleChain);

int             d_sampleChainCompare            (d_sampleChain sampleChain1,
                                                 d_sampleChain sampleChain2);

c_bool          d_sampleChainContainsAddressee  (d_sampleChain sampleChain,
                                                 d_networkAddress addressee);

#if defined (__cplusplus)
}
#endif

#endif /* D_SAMPLECHAIN_H */
