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

#ifndef D__SAMPLEREQUESTLISTENER_H
#define D__SAMPLEREQUESTLISTENER_H

#include "d__types.h"
#include "d__readerListener.h"
#include "v_group.h"
#include "sd_serializer.h"

#if defined (__cplusplus)
extern "C" {
#endif

C_STRUCT(d_sampleRequestListener){
    C_EXTENDS(d_readerListener);
    c_bool mayProceed;
    d_action actor;
    d_actionQueue actionQueue;
    c_iter requests;
};

struct writeBeadHelper{
    c_iter list;
    c_iter instances;

    d_sampleRequest request;
    c_ulong count;
    c_ulong writeCount;
    c_ulong disposeCount;
    c_ulong writeDisposeCount;
    c_ulong registerCount;
    c_ulong unregisterCount;
    c_ulong skipCount;
    c_ulong size;
    sd_serializer serializer;
    d_sampleChain sampleChain;
    d_publisher publisher;
    d_networkAddress addressee;
    c_bool checkTimeRange;
};

C_CLASS(d_sampleRequestHelper);

C_STRUCT(d_sampleRequestHelper){
    d_sampleRequestListener listener;
    d_sampleRequest request;
    c_iter addressees;
    os_time timeToAct;
};

#define d_sampleRequestHelper(h) ((d_sampleRequestHelper)(h))

void                d_sampleRequestListenerInit         (d_sampleRequestListener listener,
                                                         d_subscriber subscriber);

void                d_sampleRequestListenerDeinit       (d_object object);

void                d_sampleRequestListenerAction       (d_listener listener,
                                                         d_message message);

c_bool              d_sampleRequestListenerWriteBead    (c_object object,
                                                         c_voidp userData);

c_bool              d_sampleRequestListenerAddList      (c_object object,
                                                         v_groupInstance instance,
                                                         v_groupFlushType flushType,
                                                         c_voidp userData);

/*************HELPER FUNCTIONS******************/

d_sampleRequestHelper   d_sampleRequestHelperNew          (d_sampleRequestListener listener,
                                                           const d_sampleRequest request,
                                                           os_time timeToAct);

void                    d_sampleRequestHelperFree         (d_sampleRequestHelper helper);


#if defined (__cplusplus)
}
#endif

#endif /* D__SAMPLEREQUESTLISTENER_H */
