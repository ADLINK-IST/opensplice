/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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

#include "saj_utilities.h"
#include "saj__exception.h"
#include "os_stdlib.h"

void
saj_exceptionThrow (
    JNIEnv *env,
    int errorCode,
    const char *format,
    ...)
{
    char memo[512];
    va_list valist;
    jstring jmemo;
    jthrowable jexception;

    va_start (valist, format);
    os_vsnprintf (memo, sizeof(memo), format, valist);
    va_end (valist);
    jmemo = NEW_STRING_UTF(env, memo);
    if (jmemo) {
        jexception = CALL_STATIC_OBJECT_METHOD(
            env,
            GET_CACHED(utilities_class),
            GET_CACHED(utilities_throwException_mid),
            (jint)errorCode,
            jmemo);
        DELETE_LOCAL_REF(env, jmemo);
        /* Following JNI calls cannot check for exceptions because this
         * will always be true as one will be thrown.
         */
        (*env)->Throw(env, jexception);
        (*env)->DeleteLocalRef(env, jexception);
    }
    CATCH_EXCEPTION:;
}
