/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

#ifndef U_SERVICETERMINATIONTHREAD_H_
#define U_SERVICETERMINATIONTHREAD_H_

#if defined (__cplusplus)
extern "C" {
#endif

#include "u_types.h"
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

C_CLASS(u_serviceTerminationThread);

OS_API u_serviceTerminationThread
u_serviceTerminationThreadNew();

OS_API u_result
u_serviceTerminationThreadFree(
    u_serviceTerminationThread this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* U_SERVICETERMINATIONTHREAD_H_ */
