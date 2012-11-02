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
#ifndef C_ITERATOR_H
#define C_ITERATOR_H

#include "c_typebase.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_DB
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

C_CLASS(c_iter);

typedef void *c_iterResolveCompareArg;
typedef c_equality (*c_iterResolveCompare) ();

typedef void    *c_iterActionArg;
typedef c_bool (*c_iterAction)     (void *o, c_iterActionArg arg);
typedef void   (*c_iterWalkAction) (void *o, c_iterActionArg arg);

OS_API c_iter  c_iterNew        (void *object);
OS_API c_iter  c_iterInsert     (c_iter i, void *object);
OS_API c_iter  c_iterAppend     (c_iter i, void *object);
OS_API void   *c_iterTakeFirst  (c_iter i);
OS_API void   *c_iterTakeLast   (c_iter i);
OS_API void   *c_iterTake       (c_iter i, void *object);
OS_API void   *c_iterTakeAction (c_iter iter, c_iterAction condition, c_iterActionArg arg);
OS_API void   *c_iterReadAction (c_iter iter, c_iterAction condition, c_iterActionArg arg);
OS_API c_iter  c_iterConcat     (c_iter head, c_iter tail);
OS_API c_iter  c_iterCopy       (c_iter i);
OS_API c_long  c_iterLength     (c_iter i);
OS_API void   *c_iterResolve    (c_iter i, c_iterResolveCompare compare, c_iterResolveCompareArg arg);
OS_API void   *c_iterObject     (c_iter i, c_long index);
OS_API void    c_iterWalk       (c_iter i, c_iterWalkAction action, c_iterActionArg arg);
OS_API void    c_iterWalkUntil  (c_iter i, c_iterAction action, c_iterActionArg arg);
OS_API void    c_iterArray      (c_iter i, void *ar[]);
OS_API void    c_iterFree       (c_iter i);
OS_API c_bool  c_iterContains   (c_iter i, void *object);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
