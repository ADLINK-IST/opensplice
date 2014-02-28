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
#ifndef UT_STACK_H
#define UT_STACK_H

#include "os_defs.h"
#include "os_classbase.h"
#include "ut_result.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define UT_STACK_DEFAULT_INC (8)

OS_CLASS(ut_stack);

typedef ut_result (*ut_stackWalkAction)(void *o, void *arg);

OS_API ut_stack
ut_stackNew(
    os_uint32 increment);
    
OS_API ut_result
ut_stackFree(
    ut_stack stack);

OS_API ut_result
ut_stackPush(
    ut_stack stack,
    void *o);

OS_API void *
ut_stackPop(
    ut_stack stack);
    
OS_API os_int32
ut_stackIsEmpty(
    ut_stack stack);

OS_API ut_result
ut_stackWalk(
    ut_stack stack,
    ut_stackWalkAction action,
    void *arg);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* UT_STACK_H */
