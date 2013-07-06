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
#ifndef SAJ_COPYIN_H
#define SAJ_COPYIN_H

#include <jni.h>

#include "c_typebase.h"

#include "saj_copyCache.h"
#include "os_if.h"

#ifdef OSPL_BUILD_DCPSSAJ
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */


C_CLASS(saj_srcInfo);

C_STRUCT(saj_srcInfo) {
    JNIEnv *javaEnv;
    jobject javaObject;
    saj_copyCache copyProgram;
};

OS_API c_bool
saj_copyInStruct (
    c_base base,
    void *src,
    void *dst);
    
#undef OS_API

#endif /* SAJ_COPYIN_H */
