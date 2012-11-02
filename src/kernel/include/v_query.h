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
#ifndef V_QUERY_H
#define V_QUERY_H

#include "v_kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief The <code>v_query</code> cast method.
 *
 * This method casts an object to a <code>v_query</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_query</code> or
 * one of its subclasses.
 */
#define v_query(o) (C_CAST(o,v_query))

OS_API v_query
v_queryNew(
    v_collection source,
    const c_char *name,
    q_expr predicate,
    c_value params[]);

OS_API void
v_queryFree(
    v_query q);

OS_API void
v_queryInit(
    v_query q,
    const c_char *name,
    v_statistics qs,
    v_collection src,
    q_expr predicate,
    c_value params[]);

OS_API void
v_queryDeinit(
    v_query q);

OS_API c_bool
v_queryRead(
    v_query q,
    c_action action,
    c_voidp arg);

OS_API c_bool
v_queryTake(
    v_query q,
    c_action action,
    c_voidp arg);

OS_API c_bool
v_queryReadInstance(
    v_query q,
    v_dataReaderInstance instance,
    c_action action,
    c_voidp arg);

OS_API c_bool
v_queryTakeInstance(
    v_query q,
    v_dataReaderInstance instance,
    c_action action,
    c_voidp arg);

OS_API c_bool
v_queryReadNextInstance(
    v_query q,
    v_dataReaderInstance instance,
    c_action action,
    c_voidp arg);

OS_API c_bool
v_queryTakeNextInstance(
    v_query q,
    v_dataReaderInstance instance,
    c_action action,
    c_voidp arg);

OS_API c_bool
v_queryTest(
    v_query q);

OS_API c_bool
v_queryTriggerTest(
    v_query q);

OS_API v_collection
v_querySource(
    v_query q);

OS_API c_bool
v_querySetParams(
    v_query q,
    q_expr predicate,
    c_value params[]);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* V_QUERY_H */
