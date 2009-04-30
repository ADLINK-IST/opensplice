/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifndef U_SUBSCRIBER_H
#define U_SUBSCRIBER_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "u_types.h"
#include "os_if.h"

#ifdef OSPL_BUILD_USER
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define u_subscriber(o) ((u_subscriber)(o))

OS_API u_subscriber
u_subscriberNew (
    u_participant _scope,
    const c_char *name,
    v_subscriberQos qos,
    c_bool enable);

OS_API u_result
u_subscriberFree (
    u_subscriber _this);

OS_API u_result
u_subscriberSubscribe (
    u_subscriber _this,
    const c_char *domainExpr);

OS_API u_result
u_subscriberUnSubscribe (
    u_subscriber _this,
    const c_char *domainExpr);

OS_API c_iter
u_subscriberLookupReaders (
    u_subscriber _this,
    const c_char *topicName);

#undef OS_API 

#if defined (__cplusplus)
}
#endif

#endif
