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
#include "cms_soapThread.h"
#include "cms__thread.h"
#include "cms_thread.h"
#include "cms_service.h"
#include "cms_client.h"
#include "u_entity.h"
#include "v_participant.h"
#include "../code/v__statisticsInterface.h"
#include "v_cmsoapStatistics.h"
#include "os.h"
#include "os_report.h"
#include <soapH.h>

static void*    cms_soapThreadRun   (void *thr);

void
cms_soapThreadStatisticsRequestHandledAdd(
    v_entity entity,
    c_voidp args)
{
    v_statisticsULongValueInc(v_cmsoap, requestsHandled, entity);
}

cms_soapThread
cms_soapThreadNew(
    const c_char* name,
    cms_client client)
{
    cms_soapThread thread;
    os_mutexAttr attr;
    os_condAttr condAttr;
    os_result osr;

    thread = NULL;
    osr = os_resultInvalid;

    if(client != NULL){
        thread = cms_soapThread(os_malloc(C_SIZEOF(cms_soapThread)));
        if (thread != NULL) {
            cms_object(thread)->kind = CMS_SOAPTHREAD;
            cms_threadInit(cms_thread(thread), name, &client->service->configuration->clientScheduling);
            cms_thread(thread)->uri =  os_strdup(cms_thread(client)->uri);
            thread->client = client;
            thread->soap = NULL;

            osr = os_mutexAttrInit(&attr);
            if(osr == os_resultSuccess){
                attr.scopeAttr = OS_SCOPE_PRIVATE;
                osr = os_mutexInit(&thread->soapMutex, &attr);
                if(osr == os_resultSuccess){
                    osr = os_condAttrInit(&condAttr);
                    if(osr == os_resultSuccess){
                        condAttr.scopeAttr = OS_SCOPE_PRIVATE;
                        osr = os_condInit(&thread->condition, &thread->soapMutex, &condAttr );
                    }
                }
            }
        }
    }

    if (osr != os_resultSuccess) {
        cms_soapThreadFree(thread);
        return NULL;
    }

    return thread;
}

c_bool
cms_soapThreadStart(
    cms_soapThread thread)
{
    return cms_threadStart(cms_thread(thread), cms_soapThreadRun, (void*)thread);
}

void
cms_soapThreadFree(
    cms_soapThread thread)
{
    if(thread->client->service->configuration->verbosity >= 6){
        OS_REPORT_1(OS_INFO, CMS_CONTEXT, 0,  "Stopping soapThread '%s'...",
                        cms_thread(thread)->name);
    }
    os_mutexLock(&thread->soapMutex);
    cms_thread(thread)->terminate = TRUE;
    os_condSignal(&thread->condition);
    os_mutexUnlock(&thread->soapMutex);

    cms_threadDeinit(cms_thread(thread));

    if(thread->client->service->configuration->verbosity >= 6){
        OS_REPORT_1(OS_INFO, CMS_CONTEXT, 0,  "soapThread '%s' stopped.",
                            cms_thread(thread)->name);
    }
    os_condDestroy(&thread->condition);
    os_mutexDestroy(&thread->soapMutex);
    os_free(cms_thread(thread)->uri);
    os_free(thread);
}

c_bool
cms_soapThreadHandleRequest(
    cms_soapThread thread,
    struct soap* soap)
{
    c_bool result;

    if(cms_thread(thread)->terminate == FALSE){
        os_mutexLock(&thread->soapMutex);
        cms_thread(thread)->ready = FALSE;
        thread->soap = soap;
        os_condSignal(&thread->condition);
        os_mutexUnlock(&thread->soapMutex);
        result = TRUE;
    } else {
        result = FALSE;
    }
    return result;
}

static void*
cms_soapThreadRun(
    void *thr)
{
    cms_soapThread thread;
    struct soap* soap;
    c_char* result;

    thread = cms_soapThread(thr);
    os_mutexLock(&thread->soapMutex);

    while(cms_thread(thread)->terminate == FALSE){

        if(thread->soap != NULL){
            soap = thread->soap;
            thread->soap = NULL;

            cms_thread(thread)->results = NULL;
            soap->user = thr;
            soap_serve(soap);
            soap_destroy(soap);
            soap_end(soap);
            soap_done(soap);
            free(soap);
            u_entityAction( u_entity(thread->client->service->uservice),
                            cms_soapThreadStatisticsRequestHandledAdd,
                            thread->client->service);

            if(cms_thread(thread)->results != NULL){
                result = (c_char*)(c_iterTakeFirst(cms_thread(thread)->results));

                while(result){
                    os_free(result);
                    result = (c_char*)(c_iterTakeFirst(cms_thread(thread)->results));
                }
                c_iterFree(cms_thread(thread)->results);
                cms_thread(thread)->results = NULL;
            }
        }

        if(cms_thread(thread)->terminate == FALSE){
            cms_thread(thread)->ready = TRUE;

            if(thread->client->service->configuration->verbosity >= 7){
                OS_REPORT_1(OS_INFO, CMS_CONTEXT, 0,  "soapThread '%s' ready.", cms_thread(thread)->name);
            }
            os_condWait(&thread->condition, &thread->soapMutex);

            if(thread->client->service->configuration->verbosity >= 7){
                OS_REPORT_1(OS_INFO, CMS_CONTEXT, 0,  "soapThread '%s' condition triggered.", cms_thread(thread)->name);
            }
        }
    }
    os_mutexUnlock(&thread->soapMutex);

    if(thread->client->service->configuration->verbosity >= 6){
        OS_REPORT_1(OS_INFO, CMS_CONTEXT, 0,  "soapThread '%s' ends.", cms_thread(thread)->name);
    }
    return NULL;
}
