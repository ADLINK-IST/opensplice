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
#ifndef U_QUERY_H
#define U_QUERY_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "u_types.h"
#include "u_reader.h"
#include "os_if.h"

#ifdef OSPL_BUILD_USER
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define u_query(o) ((u_query)(o))

OS_API u_query
u_queryNew(
    u_reader source,
    const c_char *name,
    q_expr predicate,
    c_value params[]);

OS_API u_result    
u_queryInit(
    u_query _this);

OS_API u_result    
u_queryFree(
    u_query _this);

OS_API u_result    
u_queryDeinit(
    u_query _this);

OS_API u_result    
u_queryRead(
    u_query _this,
    u_readerAction action,
    c_voidp actionArg);
                             
OS_API u_result    
u_queryTake(
    u_query _this,
    u_readerAction action,
    c_voidp actionArg);

OS_API void *
u_queryReadList(
    u_query _this,
    c_ulong max,
    u_readerCopyList copy,
    c_voidp copyArg);

OS_API void *
u_queryTakeList(
    u_query _this,
    c_ulong max,
    u_readerCopyList copy,
    c_voidp copyArg);

OS_API u_result    
u_queryReadInstance(
    u_query _this,
    u_instanceHandle h,
    u_readerAction action,
    c_voidp actionArg);

OS_API u_result    
u_queryTakeInstance(
    u_query _this,
    u_instanceHandle h,
    u_readerAction action,
    c_voidp actionArg);

OS_API u_result   
u_queryReadNextInstance(
    u_query _this,
    u_instanceHandle h,
    u_readerAction action,
    c_voidp actionArg);

OS_API u_result    
u_queryTakeNextInstance(
    u_query _this,
    u_instanceHandle h,
    u_readerAction action,
    c_voidp actionArg);


OS_API c_bool      
u_queryTest(
    u_query _this);

OS_API c_bool      
u_queryTriggerTest(
    u_query _this);

OS_API u_result    
u_querySet(
    u_query _this,
    c_value params[]);

OS_API u_reader    
u_querySource(
    u_query _this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif

