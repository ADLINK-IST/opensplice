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
#ifndef V_SUBSCRIBER_H
#define V_SUBSCRIBER_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "v_kernel.h"
#include "v_entity.h"
#include "v_partition.h"
#include "v_reader.h"
#include "v_participant.h"
#include "v_subscriberQos.h"
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief The <code>v_subscriber</code> cast methods.
 *
 * This method casts an object to a <code>v_subscriber</code> object.
 * Before the cast is performed, if the NDEBUG flag is not set,
 * the type of the object is checked to be <code>v_subscriber</code> or
 * one of its subclasses.
 */
#define v_subscriber(o) (C_CAST(o,v_subscriber))

OS_API v_subscriber
v_subscriberNew(
    v_participant participant,
    const c_char *name,
    v_subscriberQos qos,
    c_bool enable);

OS_API void
v_subscriberFree(
    v_subscriber _this);

OS_API v_result
v_subscriberEnable(
    v_subscriber _this);

OS_API void
v_subscriberDeinit(
   v_subscriber _this);

OS_API void
v_subscriberSubscribe(
    v_subscriber _this,
    const c_char *partitionExpr);

OS_API void
v_subscriberUnSubscribe(
    v_subscriber _this,
    const c_char *partitionExpr);

OS_API c_iter
v_subscriberLookupPartitions(
    v_subscriber _this,
    const c_char *partitionExpr);

OS_API c_bool
v_subscriberCheckPartitionInterest(
    v_subscriber _this,
    v_partition partition);

OS_API void
v_subscriberAddReader(
    v_subscriber _this,
    v_reader reader);

OS_API void
v_subscriberRemoveReader(
    v_subscriber _this,
    v_reader reader);

OS_API c_iter
v_subscriberLookupReaders(
    v_subscriber _this);

OS_API c_iter
v_subscriberLookupReadersByTopic(
    v_subscriber _this,
    const c_char *topicName);

OS_API v_subscriberQos
v_subscriberGetQos(
    v_subscriber _this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
