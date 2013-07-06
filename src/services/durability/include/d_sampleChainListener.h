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

#ifndef D_SAMPLECHAINLISTENER_H
#define D_SAMPLECHAINLISTENER_H

#include "d__types.h"
#include "sd_serializer.h"

#if defined (__cplusplus)
extern "C" {
#endif


C_STRUCT(d_chainBead){
    d_networkAddress sender;
    v_message message;
    c_value keyValues[32];
    c_ulong nrOfKeys;
    c_ulong refCount;
};

C_STRUCT(d_chainLink){
    d_networkAddress sender;
    c_ulong sampleCount;
    d_admin admin;
};

C_STRUCT(d_chain){
    C_EXTENDS(d_object);
    d_sampleRequest request;
    d_table         beads;
    d_table         links;
    d_table         fellows;
    c_long          samplesExpect;
    c_ulong         receivedSize;
    sd_serializer   serializer;
    v_group         vgroup;
};

#define d_chain(c)                ((d_chain)(c))
#define d_chainBead(c)            ((d_chainBead)(c))
#define d_chainLink(c)            ((d_chainLink)(c))
#define d_sampleChainListener(l)  ((d_sampleChainListener)(l))

d_sampleChainListener   d_sampleChainListenerNew                (d_subscriber subscriber);

void                    d_sampleChainListenerFree               (d_sampleChainListener listener);

c_bool                  d_sampleChainListenerStart              (d_sampleChainListener listener);

c_bool                  d_sampleChainListenerStop               (d_sampleChainListener listener);

void                    d_sampleChainListenerInsertRequest      (d_sampleChainListener listener,
                                                                 d_chain chain,
                                                                 c_bool reportGroupWhenUnfullfilled);

c_bool                  d_sampleChainListenerInsertMergeAction  (d_sampleChainListener listener,
                                                                 d_mergeAction action);

void                    d_sampleChainListenerTryFulfillChains   (d_sampleChainListener listener,
                                                                 d_group group);

void                    d_sampleChainListenerReportStatus       (d_sampleChainListener listener);

void                    d_sampleChainListenerCheckUnfulfilled   (d_sampleChainListener listener,
                                                                 d_nameSpace nameSpace,
                                                                 d_networkAddress fellowAddress);

void                    d_chainFellowFree                       (d_fellow fellow);

int                     d_chainCompare                          (d_chain chain1,
                                                                 d_chain chain2);

d_chain                 d_chainNew                              (d_admin admin,
                                                                 d_sampleRequest request);

c_bool                  d_chainReportStatus                     (d_chain chain,
                                                                 d_durability durability);

void                    d_chainFree                             (d_chain chain);

d_chainBead             d_chainBeadNew                          (d_networkAddress sender,
                                                                 v_message message,
                                                                 d_chain chain);

void                    d_chainBeadFree                         (d_chainBead chainBead);

int                     d_chainBeadCompare                      (d_chainBead bead1,
                                                                 d_chainBead bead2);

int                     d_chainBeadContentCompare               (d_chainBead bead1,
                                                                 d_chainBead bead2);

c_bool                  d_chainBeadCorrect                      (d_chainBead bead,
                                                                 c_voidp args);

c_bool                  d_chainBeadInject                       (d_chainBead bead,
                                                                 c_voidp args);

d_chainLink             d_chainLinkNew                          (d_networkAddress sender,
                                                                 c_ulong sampleCount,
                                                                 d_admin admin);

void                    d_chainLinkFree                         (d_chainLink chainLink);

void                    d_chainLinkDummyFree                    (d_chainLink link);

int                     d_chainLinkCompare                      (d_chainLink link1,
                                                                 d_chainLink link2);

#if defined (__cplusplus)
}
#endif

#endif /* D_SAMPLECHAINLISTENER_H */
