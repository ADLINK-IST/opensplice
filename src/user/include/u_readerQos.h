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
#ifndef U_READERQOS_H
#define U_READERQOS_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "u_types.h"
#include "v_kernel.h"
#include "os_if.h"

#ifdef OSPL_BUILD_USER
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

OS_API typedef v_readerQos u_readerQos;

/*
 * creates a copy of the given qos.
 * if no qos is passed (NULL) then the default value is applied.
 */
OS_API u_readerQos
u_readerQosNew (
    u_readerQos qos);

/*
 * initializes the given qos to the default value.
 */
OS_API u_result
u_readerQosInit (
    u_readerQos _this);

OS_API void
u_readerQosDeinit (
    u_readerQos _this);

/*
 * Frees all resources claimed by the given object.
 */
OS_API void
u_readerQosFree (
    u_readerQos _this);

#undef OS_API 

#if defined (__cplusplus)
}
#endif

#endif /* U_READERQOS_H */
