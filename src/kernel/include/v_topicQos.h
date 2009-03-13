#ifndef V_TOPICQOS_H
#define V_TOPICQOS_H

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
 * \brief The <code>v_topicQos</code> cast methods.
 *
 * This method casts an object to a <code>v_topicQos</code> object.
 * Before the cast is performed, if the NDEBUG flag is not set,
 * the type of the object is checked to be <code>v_topicQos</code> or
 * one of its subclasses.
 */
#define v_topicQos(o) (C_CAST(o,v_topicQos))

OS_API v_topicQos
v_topicQosNew (
    v_kernel kernel,
    v_topicQos template);

OS_API void
v_topicQosFree (
    v_topicQos q);

OS_API c_bool
v_topicQosEqual (
    v_topicQos qos1,
    v_topicQos qos2,
    int reportType);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
