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

int             d_sampleChainPrintAddressees    (c_char *buffer,
                                                 c_size length,
                                                 d_sampleChain chain);

#if defined (__cplusplus)
}
#endif

#endif /* D_SAMPLECHAIN_H */
