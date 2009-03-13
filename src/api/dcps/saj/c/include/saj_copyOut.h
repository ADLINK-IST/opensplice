#ifndef SAJ_COPYOUT_H
#define SAJ_COPYOUT_H

#include <jni.h>

#include <c_typebase.h>

#include "saj_copyCache.h"
#include <os_if.h>

#ifdef OSPL_BUILD_DCPSSAJ
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

C_CLASS(saj_dstInfo);

C_STRUCT(saj_dstInfo) {
    JNIEnv *javaEnv;
    jobject javaObject;
    jclass javaClass;
    saj_copyCache copyProgram;
};

OS_API void
saj_copyOutStruct (
    void *src,
    void *dst);

#undef OS_API

#endif /* SAJ_COPYOUT_H */
