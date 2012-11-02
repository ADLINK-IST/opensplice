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
#ifndef U_DATAVIEWQOS_H
#define U_DATAVIEWQOS_H

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

OS_API v_dataViewQos u_dataViewQosNew    (v_dataViewQos tmpl);
OS_API u_result      u_dataViewQosInit   (v_dataViewQos q);
OS_API void          u_dataViewQosDeinit (v_dataViewQos q);
OS_API void          u_dataViewQosFree   (v_dataViewQos q);

#undef OS_API 

#if defined (__cplusplus)
}
#endif

#endif
