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
#ifndef U_WRITERQOS_H
#define U_WRITERQOS_H

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

typedef v_writerQos u_writerQos;

OS_API u_writerQos
u_writerQosNew (
    u_writerQos _template);

OS_API u_result
u_writerQosInit (
    u_writerQos _this);

OS_API void
u_writerQosDeinit (
    u_writerQos _this);

OS_API void
u_writerQosFree (
    u_writerQos _this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* U_WRITERQOS_H */
