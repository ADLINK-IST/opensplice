/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#ifndef SAJ_COPYOUT_H
#define SAJ_COPYOUT_H

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

C_CLASS(saj_dstInfo);

C_STRUCT(saj_dstInfo) {
    JNIEnv *javaEnv;
    jobject javaObject;
    jclass javaClass;
    saj_copyCache copyProgram;
    jobject jreader;
};

OS_API void
saj_copyOutStruct (
    void *src,
    void *dst);

OS_API void
saj_CDROutStruct (
    void *src,
    void *dst);

#undef OS_API

#endif /* SAJ_COPYOUT_H */
