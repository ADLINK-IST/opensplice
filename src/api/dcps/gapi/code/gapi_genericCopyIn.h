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
#ifndef GAPI_COPYIN_H
#define GAPI_COPYIN_H

#include "c_typebase.h"

#include "gapi_genericCopyCache.h"
#include "os_if.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

C_CLASS(gapi_srcInfo);

C_STRUCT(gapi_srcInfo) {
    void * src;
    gapi_copyCache copyProgram;
};

OS_API gapi_boolean
gapi_copyInStruct (
    c_base base,
    void *src,
    void *dst);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
