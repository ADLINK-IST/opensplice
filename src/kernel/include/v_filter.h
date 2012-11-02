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
#ifndef V_FILTER_H
#define V_FILTER_H

#include "v_kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */
#define v_filter(f) (C_CAST(f,v_filter))

OS_API v_filter
v_filterNew (
    v_topic t,
    q_expr e,
    c_value params[]);

OS_API c_bool
v_filterEval (
    v_filter _this,
    c_object o);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* V_FILTER_H */

