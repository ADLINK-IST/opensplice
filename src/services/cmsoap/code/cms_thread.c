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

    thread = cms_thread(os_malloc(C_SIZEOF(cms_thread)));
    if (thread != NULL) {
        cms_object(thread)->kind = CMS_THREAD;
    }

    cms_threadInit(thread, name, attr);

    return thread;
}

void
cms_threadInit(
    cms_thread thread,
    const c_char* name,
    os_threadAttr * attr)
{
    os_result osr;

    if(thread != NULL){
        if(name != NULL){
            thread->ready = TRUE;
            thread->terminate = FALSE;
            thread->results = NULL;
            thread->name = name;
            thread->uri = NULL;

            if (attr != NULL) {
                thread->attr = *attr;
            } else {
                osr = os_threadAttrInit(&thread->attr);
                if(osr != os_resultSuccess){
                    OS_REPORT(OS_ERROR, CMS_CONTEXT, 0, "cms_threadInit failed.");
                    os_free(thread);
                    thread = NULL;
                }
            }
        } else {
            os_free(thread);
            thread = NULL;
            OS_REPORT(OS_ERROR, CMS_CONTEXT, 0, "cms_threadInit failed. (no name supplied)");
        }
    } else {
        OS_REPORT(OS_ERROR, CMS_CONTEXT, 0, "cms_threadInit failed.");
    }
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
                OS_REPORT_1(OS_ERROR, CMS_CONTEXT, 0,
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
    osr = os_threadCreate(&thread->id, thread->name, &thread->attr, start_routine, arg);

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
        if (thread->uri != NULL) {
            os_free(thread->uri);
        }
        cms_threadDeinit(thread);
        os_free(thread);
        thread = NULL;
    }
}
