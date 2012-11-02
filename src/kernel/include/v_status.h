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
#ifndef V_STATUS_H
#define V_STATUS_H

#include "v_kernel.h"
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief The <code>v_....Status</code> cast methods.
 *
 * This method casts an object to a <code>v_....Status</code> object.
 * Before the cast is performed, if the NDEBUG flag is not set,
 * the type of the object is checked to be <code>v_....Status</code> or
 * one of its subclasses.
 */
#define v_status(s)            (C_CAST(s,v_status))
#define v_kernelStatus(s)      (C_CAST(s,v_kernelStatus))
#define v_partitionStatus(s)      (C_CAST(s,v_partitionStatus))
#define v_topicStatus(s)       (C_CAST(s,v_topicStatus))
#define v_writerStatus(s)      (C_CAST(s,v_writerStatus))
#define v_readerStatus(s)      (C_CAST(s,v_readerStatus))
#define v_subscriberStatus(s)  (C_CAST(s,v_subscriberStatus))

typedef enum v_statusResult {
    STATUS_RESULT_SUCCESS,
    STATUS_RESULT_FAIL
} v_statusResult;

typedef v_result
(*v_statusAction) (
    c_voidp info, c_voidp arg);

OS_API v_status       v_statusNew       (v_entity e);
OS_API void           v_statusFree      (v_status s);

OS_API void           v_statusInit      (v_status s, const c_char *name);
OS_API void           v_statusDeinit    (v_status s);

OS_API v_statusResult v_statusReset     (v_status s, c_ulong mask);
OS_API c_ulong        v_statusGetMask   (v_status s);

#undef OS_API

#endif
