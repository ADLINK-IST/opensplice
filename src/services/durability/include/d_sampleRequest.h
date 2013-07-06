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

#ifndef D_SAMPLEREQUEST_H
#define D_SAMPLEREQUEST_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_sampleRequest(s) ((d_sampleRequest)(s))

d_sampleRequest     d_sampleRequestNew          (d_admin admin,
                                                 const c_char* partition,
                                                 const c_char* topic,
                                                 d_durabilityKind kind,
                                                 d_timestamp requestTime,
                                                 c_bool withTimeRange,
                                                 d_timestamp beginTime,
                                                 d_timestamp endTime);

d_sampleRequest     d_sampleRequestCopy         (d_sampleRequest request);

void                d_sampleRequestFree         (d_sampleRequest sampleRequest);

int                 d_sampleRequestCompare      (d_sampleRequest request1,
                                                 d_sampleRequest request2);

void                d_sampleRequestSetCondition (d_sampleRequest request,
                                                 d_readerRequest condition);

c_bool              d_sampleRequestHasCondition (d_sampleRequest request);

void                d_sampleRequestSetSource    (d_sampleRequest request,
                                                 d_networkAddress source);

#if defined (__cplusplus)
}
#endif

#endif /* D_SAMPLEREQUEST_H */
