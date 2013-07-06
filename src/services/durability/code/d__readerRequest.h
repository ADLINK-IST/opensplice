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
#ifndef D__READERREQUEST_H_
#define D__READERREQUEST_H_

#include "d__types.h"
#include "d_lock.h"
#include "d_readerRequest.h"

#if defined (__cplusplus)
extern "C" {
#endif

C_STRUCT(d_readerRequest){
    C_EXTENDS(d_lock);
    d_admin admin;
    v_handle readerHandle;
    c_char* filter;
    c_char** filterParams;
    c_ulong filterParamsCount;
    struct v_resourcePolicy resourceLimits;
    c_time minSourceTimestamp;
    c_time maxSourceTimestamp;
    d_table requests;
    d_table groups;
};

void
d_readerRequestDeinit(
    d_object object);

#if defined (__cplusplus)
}
#endif

#endif /* D__READERREQUEST_H_ */
