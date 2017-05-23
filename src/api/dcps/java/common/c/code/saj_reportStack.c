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

#include "saj_ReportStack.h" /* JNI generated headers */
#include "saj__report.h"
#include "saj_utilities.h"
#include "os_report.h"

#define SAJ_FUNCTION(name) Java_org_opensplice_dds_dcps_ReportStack_##name

/*
 * Class:     org_opensplice_dds_dcps_ReportStack
 * Method:    reportStack
 * Signature: ()V
 */
JNIEXPORT void JNICALL
SAJ_FUNCTION(start) (
    JNIEnv *env,
    jclass jReportStack)
{
    OS_UNUSED_ARG (env);
    OS_UNUSED_ARG (jReportStack);

    SAJ_REPORT_STACK();
}

/*
 * Class:     org_opensplice_dds_dcps_ReportStack
 * Method:    jniReport
 * Signature: (Ljava/lang/String;ILjava/lang/String;ILjava/lang/String;)V
 */
JNIEXPORT void JNICALL
SAJ_FUNCTION(jniReport) (
    JNIEnv *env,
    jclass jReportStack,
    jstring jFile,
    jint jLine,
    jstring jMethod,
    jint jCode,
    jstring jMessage)
{
    const os_char *file = NULL;
    const os_char *method = NULL;
    const os_char *message = NULL;

    assert (env != NULL);
    assert (jFile != NULL);
    assert (jMethod != NULL);
    assert (jMessage != NULL);

    OS_UNUSED_ARG (jReportStack);

    if ((file = (*env)->GetStringUTFChars (env, jFile, NULL)) != NULL &&
        (method = (*env)->GetStringUTFChars (env, jMethod, NULL)) != NULL &&
        (message = (*env)->GetStringUTFChars (env, jMessage, NULL)) != NULL)
    {
        /* message is passed as a parameter on purpose, because it might
           contain sequences *printf treats as flags, resulting in segmentation
           faults. */
        saj_report(file, jLine, method, jCode, "%s", message);
    }

    /* ReleaseStringUTFChars may be invoked even in case of pending exceptions
       in the Java Virtual Machine. */
    if (message != NULL) {
        (*env)->ReleaseStringUTFChars (env, jMessage, message);
    }
    if (method != NULL) {
        (*env)->ReleaseStringUTFChars (env, jMethod, method);
    }
    if (file != NULL) {
        (*env)->ReleaseStringUTFChars (env, jFile, file);
    }
}

/*
 * Class:     org_opensplice_dds_dcps_ReportStack
 * Method:    jniReportDeprecated
 * Signature: (Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL
SAJ_FUNCTION(jniReportDeprecated) (
    JNIEnv *env,
    jclass jReportStack,
    jstring jFile,
    jint jLine,
    jstring jMethod,
    jstring jMessage)
{
    const os_char *file = NULL;
    const os_char *method = NULL;
    const os_char *message = NULL;

    assert (env != NULL);
    assert (jFile != NULL);
    assert (jMethod != NULL);
    assert (jMessage != NULL);

    OS_UNUSED_ARG (jReportStack);

    if ((file = (*env)->GetStringUTFChars (env, jFile, NULL)) != NULL &&
        (method = (*env)->GetStringUTFChars (env, jMethod, NULL)) != NULL &&
        (message = (*env)->GetStringUTFChars (env, jMessage, NULL)) != NULL)
    {
        /* message is passed as a parameter on purpose, because it might
           contain sequences *printf treats as flags, resulting in segmentation
           faults. */
        saj_report_deprecated(file, jLine, method, "%s", message);
    }

    /* ReleaseStringUTFChars may be invoked even in case of pending exceptions
       in the Java Virtual Machine. */
    if (message != NULL) {
        (*env)->ReleaseStringUTFChars (env, jMessage, message);
    }
    if (method != NULL) {
        (*env)->ReleaseStringUTFChars (env, jMethod, method);
    }
    if (file != NULL) {
        (*env)->ReleaseStringUTFChars (env, jFile, file);
    }
}

/*
 * Class:     org_opensplice_dds_dcps_ReportStack
 * Method:    jniReportFlushRequired
 * Signature: (Z)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniReportFlushRequired) (
    JNIEnv *env,
    jclass jReportStack,
    jboolean jFlush)
{
    OS_UNUSED_ARG (env);
    OS_UNUSED_ARG (jReportStack);

    return (jint)os_report_stack_flush_required(jFlush ? OS_TRUE : OS_FALSE);
}


/*
 * Class:     org_opensplice_dds_dcps_ReportStack
 * Method:    jniFlush
 * Signature: (Ljava/lang/String;ILjava/lang/String;ZI)V
 */
JNIEXPORT void JNICALL
SAJ_FUNCTION(jniFlush) (
    JNIEnv *env,
    jclass jReportStack,
    jstring jFile,
    jint jLine,
    jstring jMethod,
    jboolean jFlush,
    jint jDomainId)
{
    const os_char *file = NULL;
    os_int32 line = 0;
    os_int32 domainId = jDomainId;
    const os_char *method = NULL;
    os_boolean exception = OS_FALSE;

    OS_UNUSED_ARG (jReportStack);

    if (os_report_stack_flush_required(jFlush ? OS_TRUE : OS_FALSE)) {
        assert (jFile != NULL);
        assert (jMethod != NULL);

        line = jLine;
        if ((file = (*env)->GetStringUTFChars (env, jFile, NULL)) == NULL ||
            (method = (*env)->GetStringUTFChars (env, jMethod, NULL)) == NULL)
        {
            exception = OS_TRUE;
        }

        if (exception == OS_FALSE) {
            os_report_stack_unwind((os_boolean)jFlush, method, file, line, domainId);
        }

        /* ReleaseStringUTFChars may be invoked even in case of pending exceptions
       in the Java Virtual Machine. */
        if (method != NULL) {
            (*env)->ReleaseStringUTFChars (env, jMethod, method);
        }
        if (file != NULL) {
            (*env)->ReleaseStringUTFChars (env, jFile, file);
        }
    }
}
