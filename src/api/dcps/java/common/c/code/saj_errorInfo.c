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
#include "saj_ErrorInfo.h"
#include "saj_utilities.h"
#include "os_report.h"
#include "cmn_errorInfo.h"

#define SAJ_FUNCTION(name) Java_org_opensplice_dds_dcps_ErrorInfoImpl_##name

/**
 * Class:     DDS_ErrorInfo
 * Method:    jniErrorInfoUpdate
 * Signature: (LDDS/ReturnCodeHolder;LDDS/StringHolder;LDDS/StringHolder;LDDS/StringHolder;LDDS/StringHolder;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniUpdate)(
    JNIEnv *env,
    jobject jerrorInfo)
{
    saj_returnCode retcode = SAJ_RETCODE_OK;
    os_reportInfo *osInfo;
    jint jcode;
    jstring jmessage;
    jstring jlocation;
    jstring jsourceLine;
    jstring jstackTrace;

    osInfo = os_reportGetApiInfo();
    if (osInfo != NULL) {
        jcode = cmn_errorInfo_reportCodeToCode (osInfo->reportCode);
        SET_INT_FIELD(env, jerrorInfo, errorInfoImpl_code, jcode);

        if (osInfo->description != NULL) {
            jmessage = NEW_STRING_UTF(env, osInfo->description);
            if (jmessage != NULL) {
                SET_OBJECT_FIELD(env, jerrorInfo, errorInfoImpl_message, jmessage);
            }
        }

        if (osInfo->reportContext != NULL) {
            jlocation = NEW_STRING_UTF(env, osInfo->reportContext);
            if (jlocation != NULL) {
                SET_OBJECT_FIELD(env, jerrorInfo, errorInfoImpl_location, jlocation);
            }
        }

        if (osInfo->sourceLine != NULL) {
            jsourceLine = NEW_STRING_UTF(env, osInfo->sourceLine);
            if (jsourceLine != NULL) {
                SET_OBJECT_FIELD(env, jerrorInfo, errorInfoImpl_sourceLine, jsourceLine);
            }
         }

        if (osInfo->callStack != NULL) {
            jstackTrace = NEW_STRING_UTF(env, osInfo->callStack);
            if (jstackTrace != NULL) {
                SET_OBJECT_FIELD(env, jerrorInfo, errorInfoImpl_stackTrace, jstackTrace);
            }
        }
    } else {
        retcode = SAJ_RETCODE_NO_DATA;
    }

    return (jint)retcode;

    CATCH_EXCEPTION:
    return SAJ_RETCODE_ERROR;
}
