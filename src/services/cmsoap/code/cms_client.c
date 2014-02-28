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
#include "cms_client.h"
#include "cms_service.h"
#include "cms__thread.h"
#include "cms_thread.h"
#include "cms_soapThread.h"
#include "os.h"
#include "os_report.h"
#include "u_entity.h"
#include "v_participant.h"
#include "v_statistics.h"
#include "v_cmsoapStatistics.h"
#include "../code/v__statisticsInterface.h"


static c_bool   cms_clientLeaseIsValid  (cms_client client);
static void*    cms_clientRun           (void *client);

struct Namespace namespaces[] =
{
        {"SOAP-ENV", "http://schemas.xmlsoap.org/soap/envelope/", "http://www.w3.org/*/soap-envelope", NULL},
        {"SOAP-ENC", "http://schemas.xmlsoap.org/soap/encoding/", "http://www.w3.org/*/soap-encoding", NULL},
        {"xsi", "http://www.w3.org/2001/XMLSchema-instance", "http://www.w3.org/*/XMLSchema-instance", NULL},
        {"xsd", "http://www.w3.org/2001/XMLSchema", "http://www.w3.org/*/XMLSchema", NULL},
        {"cms", "http://127.0.0.1/cms.wsdl", NULL, NULL},
        {NULL, NULL, NULL, NULL}
};

static void
cms_clientStatisticsThreadAdd(
    v_entity entity,
    c_voidp args)
{
    v_statisticsULongValueInc(v_cmsoap, clientThreads, entity);
    v_statisticsMaxValueSetValue(v_cmsoap, maxClientThreads, entity,
        *(v_statisticsGetRef(v_cmsoap, clientThreads, entity)));
}

static void
cms_clientStatisticsThreadRemove(
    v_entity entity,
    c_voidp args)
{
    v_statisticsULongValueDec(v_cmsoap, clientThreads, entity);
}

cms_client
cms_clientNew(
    unsigned long ip,
    cms_service service,
    const c_char* uri)
{
    cms_client client;
    os_result osr;
    os_mutexAttr attr;
    os_condAttr condAttr;

    client = cms_client(os_malloc(C_SIZEOF(cms_client)));
    cms_threadInit(cms_thread(client), "cms_client", &service->configuration->clientScheduling);
    if(client != NULL){
        cms_object(client)->kind = CMS_CLIENT;
        cms_thread(client)->uri = os_strdup(uri);
        client->ip = ip;
        client->initCount = 0;
        client->service = service;
        client->internalFree = FALSE;
        osr = os_mutexAttrInit(&attr);

        if(osr == os_resultSuccess){
            attr.scopeAttr = OS_SCOPE_PRIVATE;
            osr = os_mutexInit(&client->soapMutex, &attr);
            client->soapEnvs = c_iterNew(NULL);

            if(osr == os_resultSuccess){
                osr = os_condAttrInit(&condAttr);

                if(osr == os_resultSuccess){
                    osr = os_mutexInit(&client->conditionMutex, &attr);

                    if(osr == os_resultSuccess){
                        condAttr.scopeAttr = OS_SCOPE_PRIVATE;
                        osr = os_condInit(&client->condition, &client->conditionMutex, &condAttr );
                        if(osr == os_resultSuccess){
                            osr = os_mutexInit(&client->threadMutex, &attr);

                            if(osr == os_resultSuccess){
                                client->threads = c_iterNew(NULL);
                            } else {
                                cms_clientFree(client);
                            }
                        }
                    } else {
                        cms_clientFree(client);
                    }
                } else {
                    cms_clientFree(client);
                }
            } else {
                cms_clientFree(client);
            }
        } else {
            cms_clientFree(client);
        }
    }

    if(client == NULL){
        if(service->configuration->verbosity >= 1){
            OS_REPORT(OS_ERROR, CMS_CONTEXT, 0,
            "cms_clientNew: client could not be initialized.");
        }
    }
    return client;
}

void
cms_clientFree(
    cms_client client)
{
    struct soap* soap;
    os_time ost;
    cms_soapThread soapThread;

    ost.tv_sec = 0;
    ost.tv_nsec = 1000000;

    cms_thread(client)->terminate = TRUE;

    os_mutexLock(&client->conditionMutex);
    os_condSignal(&client->condition);
    os_mutexUnlock(&client->conditionMutex);

    cms_threadDeinit(cms_thread(client));

    if(client->soapEnvs){
        os_mutexLock(&client->soapMutex);
        soap = (struct soap*)(c_iterTakeFirst(client->soapEnvs));

        while(soap){
            soap->error = soap_receiver_fault(soap, "Service is terminating.", NULL);
            soap_send_fault(soap);
            soap_destroy(soap);
            soap_end(soap);
            soap_done(soap);
            os_free(soap);
            soap = (struct soap*)(c_iterTakeFirst(client->soapEnvs));
        }
        c_iterFree(client->soapEnvs);
        client->soapEnvs = NULL;
        os_mutexUnlock(&client->soapMutex);
    }

    if(client->threads){
        soapThread = cms_soapThread(c_iterTakeFirst(client->threads));

        while(soapThread){
            cms_soapThreadFree(soapThread);
            u_entityAction(u_entity(client->service->uservice), cms_clientStatisticsThreadRemove, client->service);
            soapThread = cms_soapThread(c_iterTakeFirst(client->threads));
        }
        c_iterFree(client->threads);
        client->threads = NULL;
    }
    os_mutexDestroy(&client->soapMutex);
    os_mutexDestroy(&client->threadMutex);
    os_mutexDestroy(&client->conditionMutex);
    os_condDestroy(&client->condition);
    client->initCount = 0;

    if(client->service->configuration->verbosity >= 5){
        OS_REPORT_4(OS_INFO, CMS_CONTEXT, 0,
                        "Client thread stopped for IP: %d.%d.%d.%d",
                        (int)(client->ip>>24)&0xFF,
                        (int)(client->ip>>16)&0xFF,
                        (int)(client->ip>>8)&0xFF,
                        (int)(client->ip&0xFF));
    }
}

c_bool
cms_clientStart(
    cms_client client)
{
    return cms_threadStart(cms_thread(client), cms_clientRun, (void*)client);
}

c_bool
cms_clientHandleRequest(
    cms_client client,
    struct soap* request)
{
    c_bool result;

    if(cms_thread(client)->terminate == FALSE){
        os_mutexLock(&client->soapMutex);
        client->soapEnvs = c_iterAppend(client->soapEnvs, request);
        os_mutexUnlock(&client->soapMutex);

        os_mutexLock(&client->conditionMutex);
        os_condSignal(&client->condition);
        os_mutexUnlock(&client->conditionMutex);
        result = TRUE;
    } else {
        result = FALSE;
    }
    return result;
}

void
cms_clientRemove(
    cms_client client)
{
    os_mutexLock(&client->threadMutex);
    client->initCount--;

    if(client->initCount == 0){
        if (cms_thread(client)->terminate == FALSE) {
            client->internalFree = TRUE;
            cms_thread(client)->terminate = TRUE;
            cms_serviceRemoveClient(client->service, client);
        }
    }
    os_mutexUnlock(&client->threadMutex);

    os_mutexLock(&client->conditionMutex);
    os_condSignal(&client->condition);
    os_mutexUnlock(&client->conditionMutex);
}

void
cms_clientAdd(
    cms_client client)
{
    os_mutexLock(&client->threadMutex);
    client->initCount++;
    os_mutexUnlock(&client->threadMutex);
}

c_bool
cms_clientInitialized(
    cms_client client)
{
    c_bool result;

    result = FALSE;

    if(client->initCount > 0){
        result = TRUE;
    }
    return result;
}

static void*
cms_clientRun(
    void* client)
{
    cms_client cmsc;
    cms_thread thread;
    cms_soapThread soapThread;
    struct soap* soap;
    os_time ost;
    os_time ost2;
    c_bool found;
    int i;
    c_long maxThreads;

    ost.tv_sec = 10;
    ost.tv_nsec = 0;

    ost2.tv_sec = 0;
    ost2.tv_nsec = 1000;

    cmsc = cms_client(client);
    thread = cms_thread(client);
    thread->ready = FALSE;
    maxThreads = cmsc->service->configuration->maxThreadsPerClient;
    cmsc->leaseTime = os_timeGet();

    while(thread->terminate == FALSE){
        if(!(cms_clientLeaseIsValid(cmsc))){ /*lease has expired.*/
            thread->terminate = TRUE;
            cms_serviceRemoveClient(cmsc->service, cmsc);
            cmsc->internalFree = TRUE;    /*if lease expires, free the client.*/
        } else {
            soapThread = NULL;
            os_mutexLock(&cmsc->soapMutex);
            soap = (struct soap*)(c_iterTakeFirst(cmsc->soapEnvs));
            os_mutexUnlock(&cmsc->soapMutex);

            /*new request arrived and client is not terminating.*/
            while((soap != NULL) && (cms_thread(thread)->terminate == FALSE)){
                found = FALSE;
                cmsc->leaseTime = os_timeGet();
                os_mutexLock(&cmsc->threadMutex);

                while((found == FALSE) && (cms_thread(thread)->terminate == FALSE)){
                    for(i=0; i<c_iterLength(cmsc->threads) && (found == FALSE); i++){
                        soapThread = c_iterObject(cmsc->threads, i);

                        if(cms_thread(soapThread)->ready == TRUE){
                            found = TRUE;
                        } else {
                            if(cmsc->service->configuration->verbosity >= 7){
                                OS_REPORT_1(OS_INFO, CMS_CONTEXT, 0,
                                    "Thread %s not ready yet.",
                                    cms_thread(thread)->name);
                            }
                        }
                    }
                    if(found == FALSE){
                        if(maxThreads > c_iterLength(cmsc->threads)){
                            soapThread = cms_soapThreadNew("cms_clientSoapThread", cmsc);
                            cmsc->threads = c_iterAppend(cmsc->threads, soapThread);
                            cms_soapThreadHandleRequest(soapThread, soap);
                            cms_soapThreadStart(soapThread);
                            u_entityAction(u_entity(cmsc->service->uservice), cms_clientStatisticsThreadAdd, cmsc->service);

                            if(cmsc->service->configuration->verbosity >= 6){
                                OS_REPORT_5(OS_INFO, CMS_CONTEXT, 0,
                                    "New thread started for client: %d.%d.%d.%d. Total #threads: %d",
                                    (int)(cmsc->ip>>24)&0xFF,
                                    (int)(cmsc->ip>>16)&0xFF,
                                    (int)(cmsc->ip>>8)&0xFF,
                                    (int)(cmsc->ip&0xFF),
                                    c_iterLength(cmsc->threads));
                            }
                            found = TRUE;
                        } else {
                            os_nanoSleep(ost2);
                        }
                    } else {
                        cms_soapThreadHandleRequest(soapThread, soap);
                    }
                }
                os_mutexUnlock(&cmsc->threadMutex);

                os_mutexLock(&cmsc->soapMutex);
                soap = (struct soap*)(c_iterTakeFirst(cmsc->soapEnvs));
                os_mutexUnlock(&cmsc->soapMutex);
            }
        }
        os_mutexLock(&cmsc->conditionMutex);

        if(thread->terminate == FALSE){
            os_condTimedWait(&cmsc->condition, &cmsc->conditionMutex, &ost);

            if(cmsc->service->configuration->verbosity >= 7){
                OS_REPORT_1(OS_INFO, CMS_CONTEXT, 0,  "soapClient '%s' condition triggered.", thread->name);
            }
        }
        os_mutexUnlock(&cmsc->conditionMutex);
    }
    thread->ready = TRUE;

    if(cmsc->internalFree == TRUE){
        cms_clientFree(cmsc);
    }
    return NULL;
}

static c_bool
cms_clientLeaseIsValid(
    cms_client client)
{
    v_duration clientLeasePeriod;
    os_time curTime;
    os_time valTime;
    os_compare cmp;
    c_bool result;

    clientLeasePeriod = client->service->configuration->clientLeasePeriod;

    curTime = os_timeGet();
    valTime.tv_sec = client->leaseTime.tv_sec + clientLeasePeriod.seconds;
    valTime.tv_nsec = client->leaseTime.tv_nsec + clientLeasePeriod.nanoseconds;

    cmp = os_timeCompare(valTime, curTime);

    if(cmp == OS_LESS){
        result = FALSE;
        if(client->service->configuration->verbosity >= 2){
            OS_REPORT_4(OS_INFO, CMS_CONTEXT, 0,
                            "Client lease expired: IP %d.%d.%d.%d",
                            (int)(client->ip>>24)&0xFF,
                            (int)(client->ip>>16)&0xFF,
                            (int)(client->ip>>8)&0xFF,
                            (int)(client->ip&0xFF));
        }
    } else {
       result = TRUE;
    }
    return result;
}
