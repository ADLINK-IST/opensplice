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

#include "saj_DomainParticipantFactory.h"
#include "saj_utilities.h"
#include "u_user.h"
#include "saj__report.h"
#include "os_signalHandler.h"
#include "os_stdlib.h"

#define SAJ_FUNCTION(name) Java_org_opensplice_dds_dcps_DomainParticipantFactoryImpl_##name

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniUserInitialize)(
  JNIEnv *env,
  jobject this)
{
    jint result;
    char* ldPreload;
    os_libraryAttr attr;

    OS_UNUSED_ARG(this);
    /* Ignore the SIGHUP, SIGINT and SIGTERM on POSIX platforms
     * and ConsoleCtrlHandler on Windows platforms for JAVA see OSPL-6588
     */
    os_signalHandlerIgnoreJavaSignals();

    ldPreload = os_getenv("LD_PRELOAD");

    if(ldPreload){
        if(strstr(ldPreload, "jsig") == NULL){
            os_signalHandlerSetEnabled(0);
        }
    } else {
        os_signalHandlerSetEnabled(0);
    }
    /* Do not open a reportStack yet, since that is only possible
     * AFTER the UserLayer has been initialized!!
     */
    os_libraryAttrInit(&attr);
    os_libraryOpen("ddskernel", &attr);    
    
    result = (jint)saj_InitializeSAJ(env);
    return result;
}

JNIEXPORT jstring JNICALL
SAJ_FUNCTION(jniGetProcessName) (
  JNIEnv  *env,
  jobject this)
{
    jstring jname = NULL;
    os_char *name;

    OS_UNUSED_ARG(this);

    name = u_userGetProcessName();
    if (name != NULL) {
        jname = NEW_STRING_UTF(env, name);
    }

CATCH_EXCEPTION:
    os_free(name);
    return jname;
}

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetDomainIdFromEnvUri) (
    JNIEnv  *env,
    jobject this)
{
    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(this);

    /* No REPORT_STACK needed: u_userGetDomainIdFromEnvUri returns
     * either U_DOMAIN_ID_DEFAULT, or else the configured domainId.
     * Both outcomes are OK, so no REPORT_STACK will ever be used.
     */
    return (jint) u_userGetDomainIdFromEnvUri();
}

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniUserDetach)(
  JNIEnv *env,
  jobject this,
  jboolean blockOperations,
  jboolean deleteEntities)
{
    os_uint32 flags = 0;
    u_result ures;

    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(this);

    if (blockOperations) {
        flags |= U_USER_BLOCK_OPERATIONS;
    }
    if (deleteEntities) {
        flags |= U_USER_DELETE_ENTITIES;
    }

    ures = u_userDetach(flags);
    return saj_retcode_from_user_result(ures);
}


#undef SAJ_FUNCTION
