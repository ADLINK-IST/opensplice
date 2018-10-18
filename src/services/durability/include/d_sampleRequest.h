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

#ifndef D_SAMPLEREQUEST_H
#define D_SAMPLEREQUEST_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif


/**
 * \brief The d_sampleRequest cast macro.
 *
 * This macro casts an object to a d_sampleRequest object.
 */
#define d_sampleRequest(s) ((d_sampleRequest)(s))

d_sampleRequest     d_sampleRequestNew          (d_admin admin,
                                                 c_char* partition,
                                                 c_char* topic,
                                                 d_durabilityKind kind,
                                                 os_timeW requestTime,
                                                 c_bool withTimeRange,
                                                 os_timeW beginTime,
                                                 os_timeW endTime);

d_sampleRequest     d_sampleRequestCopy         (d_sampleRequest request,
                                                 c_bool inSharedMemory);

void                d_sampleRequestFree         (d_sampleRequest sampleRequest);

void                d_sampleRequestSetCondition (d_sampleRequest request,
                                                 d_readerRequest condition);

c_bool              d_sampleRequestSpecificReader (d_sampleRequest request);

c_bool              d_sampleRequestHasCondition (d_sampleRequest request);

void                d_sampleRequestSetSource    (d_sampleRequest request,
                                                 d_networkAddress source);

void                d_sampleRequestSetHash      (d_sampleRequest request,
                                                 c_string hash);

c_bool              d_sampleRequestHasHash      (d_sampleRequest request,
                                                 d_fellow fellow);

c_bool              d_sampleRequestCanCombine   (d_sampleRequest request1,
                                                 d_sampleRequest request2);

#if defined (__cplusplus)
}
#endif

#endif /* D_SAMPLEREQUEST_H */
