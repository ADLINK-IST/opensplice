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
#ifndef U_PUBLISHERQOS_H
#define U_PUBLISHERQOS_H

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

OS_API v_publisherQos u_publisherQosNew    (v_publisherQos tmpl);
OS_API u_result       u_publisherQosInit   (v_publisherQos q);
OS_API void           u_publisherQosDeinit (v_publisherQos q);
OS_API void           u_publisherQosFree   (v_publisherQos q);

#undef OS_API 

#if defined (__cplusplus)
}
#endif

#endif /* U_PUBLISHERQOS_H */
