/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
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

typedef c_bool (*u_subscriberAction)(u_subscriber subscriber, c_voidp arg);

#include "u_reader.h"
#include "os_if.h"

#ifdef OSPL_BUILD_USER
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define u_subscriber(o) \
        ((u_subscriber)u_entityCheckType(u_entity(o),U_SUBSCRIBER))

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
    const c_char *partitionExpr);

OS_API u_result
u_subscriberUnSubscribe (
    u_subscriber _this,
    const c_char *partitionExpr);

OS_API u_result
u_subscriberAddReader(
    u_subscriber _this,
    u_reader reader);

OS_API u_result
u_subscriberRemoveReader(
    u_subscriber _this,
    u_reader reader);

OS_API c_bool
u_subscriberContainsReader(
    u_subscriber _this,
    u_reader reader);

OS_API c_long
u_subscriberReaderCount(
    u_subscriber _this);

OS_API c_iter
u_subscriberLookupReaders(
    u_subscriber _this,
    const c_char *topic_name);

OS_API u_result
u_subscriberWalkReaders(
    u_subscriber _this,
    u_readerAction action,
    c_voidp actionArg);

OS_API u_dataReader
u_subscriberCreateDataReader (
    u_subscriber _this,
    const c_char *name,
    const c_char *expression,
    c_value params[],
    v_readerQos qos,
    c_bool enable);

OS_API u_result
u_subscriberDeleteContainedEntities (
    u_subscriber _this);

#undef OS_API 

#if defined (__cplusplus)
}
#endif

#endif
