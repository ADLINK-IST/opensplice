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
#ifndef D__READERREQUEST_H
#define D__READERREQUEST_H

#include "d__types.h"
#include "d__lock.h"

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * Macro that checks the d_readerRequest validity.
 * Because d_readerRequest is a concrete class typechecking is required.
 */
#define             d_readerRequestIsValid(_this)   \
    d_objectIsValid(d_object(_this), D_READER_REQUEST)

/**
 * \brief The d_readerRequest cast macro.
 *
 * This macro casts an object to a d_readerRequest object.
 */
#define d_readerRequest(_this) ((d_readerRequest)(_this))

C_STRUCT(d_readerRequest){
    C_EXTENDS(d_lock);
    d_admin admin;
    v_handle readerHandle;
    c_char* filter;
    c_char** filterParams;
    c_ulong filterParamsCount;
    v_resourcePolicyI resourceLimits;
    c_time minSourceTimestamp;
    c_time maxSourceTimestamp;
    d_table requests;
    d_table groups;
    c_bool groupsIgnored;
};

void                   d_readerRequestDeinit                  (d_readerRequest request);

d_readerRequest        d_readerRequestNew                     (d_admin admin,
                                                               v_waitsetEvent event);

d_readerRequest        d_readerRequestProxyNew                (v_handle source);

d_admin                d_readerRequestGetAdmin                (d_readerRequest request);

v_handle               d_readerRequestGetHandle               (d_readerRequest request);

c_long                 d_readerRequestCompare                 (d_readerRequest request1,
                                                               d_readerRequest request2);

void                   d_readerRequestFree                    (d_readerRequest request);

c_bool                 d_readerRequestAddChain                (d_readerRequest request,
                                                               d_chain chain);

c_bool                 d_readerRequestRemoveChain             (d_readerRequest request,
                                                               d_chain chain);

c_bool                 d_readerRequestHasChains               (d_readerRequest request);

c_bool                 d_readerRequestAreGroupsComplete       (d_readerRequest request);

c_bool                 d_readerRequestAddGroup                (d_readerRequest request,
                                                               d_group group);

d_table                d_readerRequestGetGroups               (d_readerRequest request);

void                   d_readerRequestRemoveGroup             (d_readerRequest request,
                                                               d_group group);

void                   d_readerRequestPrint                   (d_readerRequest request);

void                   d_readerRequestSetGroupIgnored         (d_readerRequest request);

c_bool                 d_readerRequestGetGroupIgnored         (d_readerRequest request);

#if defined (__cplusplus)
}
#endif

#endif /* D__READERREQUEST_H */
