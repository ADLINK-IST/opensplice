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
#ifndef D_READERREQUEST_H_
#define D_READERREQUEST_H_

#include "d__types.h"
#include "v_kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_readerRequest(r) ((d_readerRequest)(r))

d_readerRequest
d_readerRequestNew(
    d_admin admin,
    v_handle source,
    c_char* filter,
    c_char** filterParams,
    c_long filterParamsCount,
    struct v_resourcePolicy resourceLimits,
    c_time minSourceTimestamp,
    c_time maxSourceTimestamp);

d_readerRequest
d_readerRequestProxyNew(
    v_handle source);

d_admin
d_readerRequestGetAdmin(
    d_readerRequest request);

v_handle
d_readerRequestGetHandle(
    d_readerRequest request);

c_long
d_readerRequestCompare(
    d_readerRequest request1,
    d_readerRequest request2);

void
d_readerRequestFree(
    d_readerRequest request);

c_bool
d_readerRequestAddChain(
    d_readerRequest request,
    d_chain chain);

c_bool
d_readerRequestRemoveChain(
    d_readerRequest request,
    d_chain chain);

c_bool
d_readerRequestHasChains(
    d_readerRequest request);

c_bool
d_readerRequestAreGroupsComplete(
    d_readerRequest request);

c_bool
d_readerRequestAddGroup(
    d_readerRequest request,
    d_group group);

d_table
d_readerRequestGetGroups(
    d_readerRequest request);

void
d_readerRequestRemoveGroup(
    d_readerRequest request,
    d_group group);

void
d_readerRequestPrint(
    d_readerRequest request);

#if defined (__cplusplus)
}
#endif

#endif /* D_READERREQUEST_H_ */
