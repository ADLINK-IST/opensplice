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
#ifndef U_PARTICIPANTQOS_H
#define U_PARTICIPANTQOS_H

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

OS_API v_participantQos u_participantQosNew    (v_participantQos tmpl);
OS_API u_result         u_participantQosInit   (v_participantQos q);
OS_API void             u_participantQosDeinit (v_participantQos q);
OS_API void             u_participantQosFree   (v_participantQos q);

#undef OS_API 

#if defined (__cplusplus)
}
#endif

#endif /* U_PARTICIPANTQOS_H */
