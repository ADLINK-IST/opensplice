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

#include "cms_thread.h"
#include "cms__thread.h"
#include "os_heap.h"
#include "os_report.h"
#include "os_thread.h"

cms_thread
cms_threadNew(
    const c_char* name,
    os_threadAttr * attr)
{
    cms_thread thread;
    thread = os_malloc(sizeof *thread);
    cms_object(thread)->kind = CMS_THREAD;
    if (!cms_threadInit(thread, name, attr)){
        os_free(thread);
        thread = NULL;
    }
    return thread;
}

c_bool
cms_threadInit(
    cms_thread thread,
    const c_char* name,
    os_threadAttr * attr)
{
    c_bool retValue = FALSE;
    if(thread != NULL){
        if(name != NULL){
            thread->ready = TRUE;
            thread->terminate = FALSE;
            thread->results = NULL;
            thread->name = name;
            thread->did.id[0] = '\0';
            thread->uri = NULL;
            retValue = TRUE;
            if (attr != NULL) {
                thread->attr = *attr;
            } else {
                os_threadAttrInit(&thread->attr);
            }
        } else {
            OS_REPORT(OS_ERROR, CMS_CONTEXT, 0, "cms_threadInit failed. (no name supplied)");
        }
    } else {
        OS_REPORT(OS_ERROR, CMS_CONTEXT, 0, "cms_threadInit failed.");
    }
    return (retValue);
}

void
cms_threadDeinit(
    cms_thread thread)
{
    os_result osr;
    os_threadId self;

    if(thread != NULL){
        thread->terminate = TRUE;

        self = os_threadIdSelf();

        /* Only wait on exit when the thread that calls this routine does not
         * wait for itself.
         */
        if(os_threadIdToInteger(self) != os_threadIdToInteger(thread->id)){
            osr = os_threadWaitExit(thread->id, NULL);

            if(osr != os_resultSuccess){
                OS_REPORT(OS_ERROR, CMS_CONTEXT, 0,
                            "Thread '%s' waitExit failed.", thread->name);
            }
        }
    }
}

c_bool
cms_threadStart(
    cms_thread thread,
    void *(* start_routine)(void *),
    void *arg)
{
    os_result osr;
    c_bool success;

    success = TRUE;
    osr = u_serviceThreadCreate(&thread->id, thread->name, &thread->attr, start_routine, arg);

    if(osr != os_resultSuccess){
        OS_REPORT(OS_ERROR, CMS_CONTEXT, 0, "cms_threadStart failed.");
        success = FALSE;
    }
    return success;
}


void
cms_threadFree(
    cms_thread thread)
{
    if(thread != NULL){
        cms_threadDeinit(thread);

        if (thread->uri != NULL) {
            os_free(thread->uri);
        }
        os_free(thread);
        thread = NULL;
    }
}
