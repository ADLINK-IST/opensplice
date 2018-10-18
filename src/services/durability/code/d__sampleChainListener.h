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

#ifndef D__SAMPLECHAINLISTENER_H
#define D__SAMPLECHAINLISTENER_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * \brief The d_chain cast macro.
 *
 * This macro casts an object to a d_chain object.
 */
#define d_chain(_this) ((d_chain)(_this))

/**
 * Macro that checks the d_chain validity.
 * Because d_chain is a concrete class typechecking is required.
 */
#define d_chainIsValid(_this)   \
        d_objectIsValid(d_object(_this), D_CHAIN)

/**
 * Macro that checks the d_sampleChainListener validity.
 * Because d_sampleChainListener is a concrete class typechecking is required.
 */
#define d_sampleChainListenerIsValid(_this)   \
        d_listenerIsValidKind(d_listener(_this), D_SAMPLE_CHAIN_LISTENER)

d_sampleChainListener   d_sampleChainListenerNew                (d_subscriber subscriber);
void                    d_sampleChainListenerFree               (d_sampleChainListener listener);
c_bool                  d_sampleChainListenerStart              (d_sampleChainListener listener);
c_bool                  d_sampleChainListenerStop               (d_sampleChainListener listener);
c_bool                  d_sampleChainListenerCheckChainComplete (d_sampleChainListener listener,
                                                                 d_chain chain);
void                    d_sampleChainListenerInsertRequest      (d_sampleChainListener listener,
                                                                 d_chain chain,
                                                                 c_bool reportGroupWhenUnfullfilled);
c_bool                  d_sampleChainListenerInsertMergeAction  (d_sampleChainListener listener,
                                                                 d_mergeAction action);
void                    d_sampleChainListenerTryFulfillChains   (d_sampleChainListener listener,
                                                                 d_group group);
void                    d_sampleChainListenerReportStatus       (d_sampleChainListener listener);
void                    d_sampleChainListenerReportGroup        (d_sampleChainListener listener,
                                                                 d_group group);
void                    d_sampleChainListenerCheckUnfulfilled   (d_sampleChainListener listener,
                                                                 d_nameSpace nameSpace,
                                                                 d_networkAddress fellowAddress);
void                    d_traceChain                            (d_chain chain);
int                     d_chainCompare                          (d_chain chain1,
                                                                 d_chain chain2);
d_chain                 d_chainNew                              (d_admin admin,
                                                                 d_sampleRequest request);
void                    d_chainFree                             (d_chain chain);

#if defined (__cplusplus)
}
#endif

#endif /* D__SAMPLECHAINLISTENER_H */
