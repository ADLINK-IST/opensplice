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
#ifndef U_WAITSET_H
#define U_WAITSET_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "u_types.h"
#include "os_if.h"

#ifdef OSPL_BUILD_USER
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define u_waitset(o) \
        ((u_waitset)u_entityCheckType(u_entity(o), U_WAITSET))

typedef void (*u_waitsetAction)(v_waitsetEvent e, c_voidp arg);

OS_API u_waitset
u_waitsetNew(
    u_participant p);

OS_API u_result
u_waitsetInit(
    u_waitset _this);

OS_API u_result
u_waitsetFree(
    u_waitset _this);

OS_API u_result
u_waitsetDeinit(
    u_waitset _this);

OS_API u_result
u_waitsetWait(
    u_waitset _this,
    c_iter *list);

OS_API u_result
u_waitsetTimedWait(
    u_waitset _this,
    const c_time t,
    c_iter *list);

OS_API u_result
u_waitsetWaitAction(
    u_waitset _this,
    u_waitsetAction action,
    c_voidp arg);

OS_API u_result
u_waitsetTimedWaitAction(
    u_waitset _this,
    u_waitsetAction action,
    c_voidp arg,
    const c_time t);

OS_API u_result
u_waitsetWaitEvents(
    u_waitset _this,
    c_iter *list);

OS_API u_result
u_waitsetTimedWaitEvents(
    u_waitset _this,
    const c_time t,
    c_iter *list);

OS_API u_result
u_waitsetNotify(
    u_waitset _this,
    c_voidp eventArg);

OS_API u_result
u_waitsetAttach(
    u_waitset _this,
    u_entity c,
    c_voidp context);

OS_API u_result
u_waitsetDetach(
    u_waitset _this,
    u_entity c);

OS_API u_result
u_waitsetGetEventMask(
    u_waitset _this,
    c_ulong *eventMask);

OS_API u_result
u_waitsetSetEventMask(
    u_waitset _this,
    c_ulong eventMask);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
