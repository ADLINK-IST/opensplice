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
#ifndef OS_CLASSBASE_H
#define OS_CLASSBASE_H

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_OS
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define OS_STRUCT(name)  struct name##_s
#define OS_EXTENDS(type) OS_STRUCT(type) _parent
#define OS_CLASS(name)   typedef OS_STRUCT(name) *name
#define OS_SIZEOF(name)  sizeof(OS_STRUCT(name))
#define OS_SUPER(obj)    (&((obj)->_parent))

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* OS_CLASSBASE_H */
