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
    c_base base;
};

OS_API os_int32
saj_copyInStruct (
    c_base base,
    const void *src,
    void *dst);
    
#undef OS_API

#endif /* SAJ_COPYIN_H */
