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
#include "os_socket.h"
#include "cms_service.h"
#include "cms_configuration.h"
#include "cms_thread.h"
#include "cms_client.h"
#include "cmx_factory.h"
#include "cmx__factory.h"
#include <soapH.h>
#include "u_observable.h"
#include "u_service.h"
#include "u_participant.h"
#include "u_cmsoap.h"
#include "v_service.h"
#include "v_participant.h"
#include "kernelModule.h"
#include "v_cmsoap.h"
#include "v_participantQos.h"
#include "v_cmsoapStatistics.h"
#include "v_group.h"
#include "v_maxValue.h"
#include "os_heap.h"
#include "os_report.h"
#include "v_qos.h"
#include "os_stdlib.h"
#include "u_domain.h"
#include "u_participantQos.h"

#define CMSERVICE_ATTACH_TIMEOUT 30

static void
cms_serviceStatisticsAction(
    v_public entity,
    c_voidp args)
{
    v_cmsoap cmsoap = v_cmsoap(entity);

    OS_UNUSED_ARG(args);

    if (cmsoap->statistics) {
        cmsoap->statistics->connectedClients++;
        v_maxValueSetValue(&cmsoap->statistics->maxConnectedClients,
                           cmsoap->statistics->connectedClients);
    }
}

static void
cms_serviceUpdateStatistics(
    cms_service service)
{
    if(service != NULL){
        (void)u_observableAction(u_observable(service->uservice), cms_serviceStatisticsAction, service);
    }
}

static void*
cms_serviceCollectGarbage(
    void* arg)
{
    cms_service cms;
    os_duration update;
    cms_thread client;
    c_bool garbagePresent;

    cms = cms_service(arg);
    update = OS_DURATION_INIT(2,0);

    garbagePresent = FALSE;

    /*
     * Keep going until service terminates AND all garbage has been collected.
     */
    while((cms->terminate == FALSE) || (c_iterLength(cms->clientGarbage) != 0)){
        os_mutexLock(&cms->clientMutex);
        client = cms_thread(c_iterTakeFirst(cms->clientGarbage));
        os_mutexUnlock(&cms->clientMutex);

        while(client != NULL){
            /*
             * Call threadFree and NOT clientFree on purpose.
             */
            cms_threadFree(client);
            os_mutexLock(&cms->clientMutex);
            client = cms_thread(c_iterTakeFirst(cms->clientGarbage));
            os_mutexUnlock(&cms->clientMutex);
            garbagePresent = TRUE;
        }

        if((c_iterLength(cms->clients) == 0) && (garbagePresent == TRUE)){
            if(cms->configuration->verbosity >= 3){
                OS_REPORT(OS_INFO, CMS_CONTEXT, 0,
                                "No clients connected. Performing some garbage collecting...");
            }
            cmx_deregisterAllEntities();
            garbagePresent = FALSE;
        }
        /* Terminate flag is also accessed outside the mutex lock. This is no problem,
         * protection guarantees no wait entry while setting the flag. */
        os_mutexLock(&cms->terminateMutex);
        if(cms->terminate == FALSE){
            os_condTimedWait(&cms->terminateCondition, &cms->terminateMutex, update);
        }
        os_mutexUnlock(&cms->terminateMutex);
    }
    c_iterFree(cms->clientGarbage);

    return NULL;
}

static void
cms_serviceActionGroups(
    v_public p,
    c_voidp args)
{
    v_service s;
    c_iter groups;
    v_group group;

    OS_UNUSED_ARG(args);

    s = v_service(p);
    groups = v_serviceTakeNewGroups(s);
    group = v_group(c_iterTakeFirst(groups));

    while(group != NULL){
        c_free(group);
        group = v_group(c_iterTakeFirst(groups));
    }
    c_iterFree(groups);
}

static void*
cms_serviceLeaseUpdate(
    void* arg)
{
    cms_service cms;
    u_result result;
    cms = cms_service(arg);

    result = U_RESULT_OK;
    while((cms->terminate == FALSE) && (result == U_RESULT_OK)) {
        result = u_serviceRenewLease(cms->uservice, cms->configuration->leasePeriod);
        if (result != U_RESULT_OK){
            OS_REPORT(OS_ERROR, CMS_CONTEXT, 0,
                "Failed to update service lease.");
        } else if(cms->configuration->verbosity >= 7){
            OS_REPORT(OS_INFO, CMS_CONTEXT, 0,
                "Service lease updated.");
        }
        os_mutexLock(&cms->terminateMutex);
        if(cms->terminate == FALSE) {
            os_condTimedWait(&cms->terminateCondition, &cms->terminateMutex, cms->configuration->leaseRenewalPeriod);
        }
        os_mutexUnlock(&cms->terminateMutex);
    }
    result = u_serviceRenewLease(cms->uservice, 20*OS_DURATION_SECOND);
    if (result != U_RESULT_OK) {
        OS_REPORT(OS_ERROR, CMS_CONTEXT, 0,
            "Failed to update service lease.");
    }
    return NULL;
}

static void
cms_serviceSplicedaemonListener(
    v_serviceStateKind spliceDaemonState,
    c_voidp usrData)
{
    cms_service cms = (cms_service)usrData;

    switch (spliceDaemonState) {
    case STATE_TERMINATING:
    case STATE_TERMINATED:
    case STATE_DIED:
        if(cms->terminate == FALSE){
            if(cms->configuration->verbosity >= 2){
                OS_REPORT(OS_WARNING, CMS_CONTEXT, 0,
                    "Splicedaemon is terminating. Terminating CMSOAP service now...");
            }
            os_mutexLock(&cms->terminateMutex);
            cms->terminate = TRUE;
            os_condBroadcast(&cms->terminateCondition);
            os_mutexUnlock(&cms->terminateMutex);

        }
    break;
    default:
    break;
    }
}

static c_bool
cms_serviceInitLease(
    cms_service cms)
{
    c_bool success;

    success = FALSE;
    cms->leaseThread = cms_threadNew("cms_serviceLease", &cms->configuration->leaseScheduling);

    if(cms->leaseThread != NULL){
        success = cms_threadStart(cms->leaseThread,
                                  (void*(*)(void*))cms_serviceLeaseUpdate,
                                  (void*)cms);
        if(success != TRUE){
            if(cms->configuration->verbosity >= 1){
                OS_REPORT(OS_ERROR, CMS_CONTEXT, 0,
                    "cms_serviceNew: lease update thread could not be started.");
            }
        }
    }
    return success;
}

static c_bool
cms_serviceInitSOAP(
    cms_service cms)
{
    struct soap* soap;
    c_bool success;
    c_long master;

    success = FALSE;
    soap = (struct soap*)(malloc(sizeof(struct soap)));

    soap_init2(soap, SOAP_IO_KEEPALIVE | SOAP_IO_FLUSH, SOAP_IO_KEEPALIVE | SOAP_IO_FLUSH);
    cms->soap = soap;
    cms->soap->accept_timeout = -1000000;
#ifdef PIKEOS_POSIX
    cms->soap->bind_flags = 0;
#else
    cms->soap->bind_flags = SO_REUSEADDR;
#endif

    master = soap_bind(cms->soap, NULL, (int) cms->configuration->port, (int) cms->configuration->backlog);

    if(master < 0){
        if(cms->configuration->verbosity >= 1){
            OS_REPORT(OS_ERROR, CMS_CONTEXT, 0, "Could not bind to port %d",
                    cms->configuration->port);
        }
    } else {
        cms->clients = c_iterNew(NULL);
        success = TRUE;
    }
    return success;
}


static c_bool
cms_serviceInitConfiguration(
    const c_char* name,
    cms_service cms )
{
    c_bool success;

    success = cms_configurationNew(name, cms);

    return success;
}

static c_bool
cms_serviceInitGarbageCollector(
    cms_service cms)
{
    c_bool success;

    success = FALSE;

    cms->clientGarbage = c_iterNew(NULL);
    cms->garbageCollector = cms_threadNew("cms_garbageCollector", &cms->configuration->garbageScheduling);

    if(cms->garbageCollector != NULL){
        success = cms_threadStart(cms->garbageCollector,
                                  (void*(*)(void*))cms_serviceCollectGarbage,
                                  (void*)cms);
        if(success != TRUE){
            if(cms->configuration->verbosity >= 1){
                OS_REPORT(OS_ERROR, CMS_CONTEXT, 0,
                    "cms_serviceNew: garbage collector could not be started.");
            }
        }
    }
    return success;
}

static c_bool
cms_serviceInit(
    const c_char* name,
    cms_service cms)
{
    c_bool       success = FALSE;
    c_char*      config;
    os_result    osr;

    if(cms != NULL){
        success = cms_serviceInitConfiguration(name, cms);

        if(success == TRUE){
            if(cms->configuration->verbosity >= 3){
                config = cms_configurationFormat(cms->configuration);
                OS_REPORT(OS_INFO, CMS_CONTEXT, 0, "%s", config);
                os_free(config);
            }
            osr = os_mutexInit(&cms->terminateMutex, NULL);
            if(osr == os_resultSuccess){
                osr = os_condInit(&cms->terminateCondition, &cms->terminateMutex, NULL );
                if(osr != os_resultSuccess){
                    if(cms->configuration->verbosity >= 1){
                        OS_REPORT(OS_INFO, CMS_CONTEXT, 0,
                        "cms_serviceInit: Terminate condition could not be initialized.");
                    }
                }
                else {
                    success = cms_serviceInitLease(cms);
                }
            } else {
                if(cms->configuration->verbosity >= 1){
                    OS_REPORT(OS_INFO, CMS_CONTEXT, 0,
                    "cms_serviceInit: Terminate condition mutex could not be initialized.");
                }
            }

            if(success == TRUE){
                if(cms->configuration->verbosity >= 3){
                    OS_REPORT(OS_INFO, CMS_CONTEXT, 0,
                    "cms_serviceInit: lease updater initialized.");
                }
                success = cms_serviceInitSOAP(cms);

                if(success == TRUE){
                    if(cms->configuration->verbosity >= 3){
                        OS_REPORT(OS_INFO, CMS_CONTEXT, 0,
                        "cms_serviceInit: SOAP environment initialized.");
                    }
                    osr = os_mutexInit(&cms->clientMutex, NULL);

                    if(osr == os_resultSuccess){
                        if(cms->configuration->verbosity >= 4){
                            OS_REPORT(OS_INFO, CMS_CONTEXT, 0,
                            "CMSOAPService initialized.");
                        }
                        success = cms_serviceInitGarbageCollector(cms);

                        if(success == TRUE){
                            if(cms->configuration->verbosity >= 4){
                                OS_REPORT(OS_INFO, CMS_CONTEXT, 0,
                                "cms_serviceInit: Garbage collector initialized.");
                            }
                        }
                    } else {
                        if(cms->configuration->verbosity >= 1){
                            OS_REPORT(OS_INFO, CMS_CONTEXT, 0,
                            "cms_serviceInit: Client mutex could not be initialized.");
                        }
                    }
                }
            }
        }
    }
    return success;
}

#define MAX_INTERFACES 64
#define SOAP_TAG        "<TunerService>%s</TunerService>"
#define IP_TAG          "<Ip>%s:%d</Ip>"
cms_service
cms_serviceNew(
    const c_char* name,
    const c_char* uri)
{
    cms_service service;
    c_bool success;
    const c_char* init;
    u_participantQos qos;
    struct sockaddr_in addr;
    socklen_t len;
    os_result errcode;
    char* ipTagStr = NULL;
    char* xmlStr = NULL;
    u_result result;
    os_result res;
    os_ifAttributes *ifList;
    os_uint32 nofIf, i;

    service = NULL;
    success = FALSE;

    if(uri != NULL) {
        init = cmx_initialise();

        if(strcmp(init, CMS_RESULT_OK) == 0) {
            service = cms_service(os_malloc(C_SIZEOF(cms_service)));
            cms_object(service)->kind   = CMS_SERVICE;
            service->terminate          = FALSE;
            service->leaseThread        = NULL;
            service->clients            = NULL;
            service->soap               = NULL;
            service->configuration      = NULL;
            service->garbageCollector   = NULL;
            service->uri                = os_strdup(uri);

            service->uservice = u_cmsoapNew(uri, U_DOMAIN_ID_ANY, CMSERVICE_ATTACH_TIMEOUT, name, NULL, FALSE);

            if(service->uservice != NULL) {
                (void)sprintf(service->did.id, "%d", u_participantGetDomainId(u_participant(service->uservice)));
                /*disable all events.*/
                result = u_observableSetListenerMask(u_observable(service->uservice), 0);
                if (result == U_RESULT_OK) {
                    result = u_observableAction(u_observable(service->uservice), cms_serviceActionGroups, NULL);
                }

                if (result == U_RESULT_OK) {
                    if (u_serviceChangeState(service->uservice, STATE_INITIALISING)) {
                        success = cms_serviceInit(name, service);

                        result = u_participantGetQos(u_participant(service->uservice), &qos);
                    } else {
                        result = U_RESULT_INTERNAL_ERROR;
                    }
                }

                if ((result == U_RESULT_OK) && success) {
                    /* Insert service information in userData QoS */
                    len = sizeof(addr);
                    memset(&addr, 0, len);
                    errcode = os_sockGetsockname (service->soap->master, (struct sockaddr*)&addr, len);
                    if(errcode == os_resultSuccess) {
                        OS_REPORT(OS_INFO, CMS_CONTEXT, 0, "SOAP service is reachable via port %d",ntohs(addr.sin_port));

                        ifList = os_malloc(MAX_INTERFACES * sizeof(*ifList));

#ifdef WITH_IPV6
                        res = os_sockQueryIPv6Interfaces(ifList, (os_uint32)MAX_INTERFACES, &nofIf);
#else
                        res = os_sockQueryInterfaces(ifList, (os_uint32)MAX_INTERFACES, &nofIf);
#endif
                        /* SOAP userdata layout:
                         * <TunerService>
                         * <Ip>x.x.x.x:port</Ip> [<Ip>x.x.x.x</Ip>]...
                         * </TunerService>
                         */
                        if (res == os_resultSuccess) {
                            os_char tmp[64];
                            int chars;
                            for (i = 0; i < nofIf; i++) {
                                /* ignore the local loopback interface */
                                if (!os_sockaddrIsLoopback((os_sockaddr*)&ifList[i].address)) {
                                    os_sprintf(tmp,"%s",inet_ntoa(((os_sockaddr_in*)&ifList[i].address)->sin_addr));
                                    if (strcmp(tmp,"0.0.0.0") != 0) {
                                        chars = os_sprintf(tmp, IP_TAG,
                                            inet_ntoa(((os_sockaddr_in*)&ifList[i].address)->sin_addr),
                                            ntohs(addr.sin_port));
                                        if (chars > 0) {
                                            if (ipTagStr) {
                                                ipTagStr = os_realloc(ipTagStr, strlen(ipTagStr) + (size_t) chars + 1);
                                            } else {
                                                ipTagStr = os_malloc((size_t) chars + 1);
                                                *ipTagStr = '\0';
                                            }
                                            ipTagStr = os_strcat(ipTagStr, tmp);
                                        }
                                    }
                                }
                            }
                            if (ipTagStr == NULL) {
                                if(service->configuration->verbosity >= 1) {
                                    OS_REPORT(OS_WARNING, CMS_CONTEXT, 0, "Could not find a network interface ip address");
                                }
                                ipTagStr = os_malloc((strlen(IP_TAG) + INET6_ADDRSTRLEN_EXTENDED));
                                os_sprintf (ipTagStr, IP_TAG, "127.0.0.1", ntohs(addr.sin_port));
                            }
                        } else {
                            if(service->configuration->verbosity >= 1) {
                                OS_REPORT(OS_WARNING, CMS_CONTEXT, 0,"Could not get SOAP ip address.");
                            }
                            ipTagStr = os_malloc((strlen(IP_TAG) + INET6_ADDRSTRLEN_EXTENDED));
                            os_sprintf (ipTagStr, IP_TAG, "127.0.0.1", ntohs(addr.sin_port));
                        }
                        os_free(ifList);

                        xmlStr = os_malloc(strlen(ipTagStr) + strlen(SOAP_TAG)+1);
                        os_sprintf (xmlStr, SOAP_TAG, ipTagStr);
                        os_free(ipTagStr);

                        qos->userData.v.size = (c_long) strlen(xmlStr);
                        qos->userData.v.value = os_malloc((size_t) qos->userData.v.size);
                        memcpy(qos->userData.v.value, xmlStr, (size_t) qos->userData.v.size);

                        if(service->configuration->verbosity >= 5){
                            OS_REPORT(OS_INFO, CMS_CONTEXT, 0, "SOAP userData: %s", xmlStr);
                        }
                        os_free(xmlStr);
                    } else {
                        qos->userData.v.size = 0;
                        qos->userData.v.value = NULL;
                        if(service->configuration->verbosity >= 1){
                            OS_REPORT(OS_WARNING, CMS_CONTEXT, 0, "Could not get SOAP port.");
                        }
                    }

                    result = u_participantSetQos(u_participant(service->uservice), qos);
                    if (result != U_RESULT_OK) {
                        if(service->configuration->verbosity >= 1){
                            OS_REPORT(OS_WARNING, CMS_CONTEXT, 0,"Could not update the participantQos for publication of the SOAP ip address and port.");
                        }
                    }
                    u_participantQosFree(qos);
                } else {
                    if(service->configuration == NULL || service->configuration->verbosity >= 1){
                       OS_REPORT(OS_WARNING, CMS_CONTEXT, 0,"Could not get participantQos for SOAP service port publication (%s).", u_resultImage(result));
                    }
                }

                if(success) {
                    result = u_entityEnable(u_entity(service->uservice));
                    if (result != U_RESULT_OK) {
                        if(service->configuration == NULL || service->configuration->verbosity >= 1){
                            OS_REPORT(OS_WARNING, CMS_CONTEXT, 0,"Could not enable service participant.");
                            success = FALSE;
                        }
                    }
                }

                if(success == FALSE){
                    cms_serviceFree(service);
                    service = NULL;
                } else {
                    u_serviceWatchSpliceDaemon(service->uservice,
                                                cms_serviceSplicedaemonListener,
                                                service);
                    u_serviceChangeState(service->uservice, STATE_OPERATIONAL);
                }
            } else {
                OS_REPORT(OS_ERROR, CMS_CONTEXT, 0,
                "cms_serviceNew: user layer service could not be created.");

                cms_serviceFree(service);
                service = NULL;
            }
        } else {
            if(service && service->configuration->verbosity >= 1){
                OS_REPORT(OS_ERROR, CMS_CONTEXT, 0,
                      "cms_serviceNew: C&M API could not be initialized.");
            }
        }
    } else {
        if(service && service->configuration->verbosity > 0){
            OS_REPORT(OS_ERROR, CMS_CONTEXT, 0, "cms_serviceNew: no uri supplied.");
        }
    }
    return service;
}
#undef SOAP_TAG
#undef IP_TAG

void
cms_serviceFree(
    cms_service cms)
{
    cms_client client;
    c_iter clientCopy;
    c_ulong i, size;

    if(cms != NULL){
        if(cms->configuration != NULL){
            if(cms->configuration->verbosity >= 2){
                OS_REPORT(OS_INFO, CMS_CONTEXT, 0, "Terminating CMSOAP service...");
            }
        }
        if(cms->uservice != NULL){
            u_serviceChangeState(cms->uservice, STATE_TERMINATING);
        }
        os_mutexLock(&cms->terminateMutex);
        cms->terminate = TRUE;
        os_condBroadcast(&cms->terminateCondition);
        os_mutexUnlock(&cms->terminateMutex);

        if(cms->leaseThread != NULL){
            cms_threadFree(cms->leaseThread);
        }
        if(cms->garbageCollector != NULL){
            cms_threadFree(cms->garbageCollector);
        }
        if(cms->soap != NULL){
            cms->soap->attributes = NULL;
            soap_destroy(cms->soap);
            soap_end(cms->soap);
            soap_done(cms->soap);
            free(cms->soap);
        }
        if(cms->clients != NULL){
            os_mutexLock(&cms->clientMutex);
            clientCopy = c_iterCopy(cms->clients);
            os_mutexUnlock(&cms->clientMutex);

            if(c_iterLength(cms->clients) > 0){
                if(cms->configuration->verbosity >= 2){
                    OS_REPORT(OS_WARNING, CMS_CONTEXT, 0,
                    "Terminating CMSOAPService, but %d client(s) is/are still connected.",
                    c_iterLength(cms->clients));
                }
            }
            size = c_iterLength(clientCopy);

            for(i=0; i<size; i++){
                client = cms_client(c_iterObject(clientCopy, i));
                cms_thread(client)->terminate = TRUE;
            }
            for(i=0; i<size; i++){
                client = cms_client(c_iterObject(clientCopy, i));
                cms_clientFree(client);
            }

            c_iterFree(clientCopy);

            os_mutexLock(&cms->clientMutex);
            c_iterFree(cms->clients);
            os_mutexUnlock(&cms->clientMutex);

            os_mutexDestroy(&cms->clientMutex);
        }
        if(cms->configuration != NULL){
            if(cms->configuration->verbosity >= 4){
                OS_REPORT(OS_INFO, CMS_CONTEXT, 0, "CMSOAP service terminated.");
            }
        }
        cms_configurationFree(cms->configuration);

        if(cms->uservice != NULL){
            cmx_deregisterAllEntities();
            u_serviceChangeState(cms->uservice, STATE_TERMINATED);
            u_objectFree (u_object (cms->uservice));
        }
        if(cms->uri != NULL){
            os_free(cms->uri);
        }
        os_condDestroy(&cms->terminateCondition);
        os_mutexDestroy(&cms->terminateMutex);
        cmx_detach();

        os_free(cms);
    }
}

cms_client
cms_serviceLookupClient(
    cms_service cms)
{
    c_ulong i;
    cms_client client;
    cms_client result;

    result = NULL;
    os_mutexLock(&cms->clientMutex);

    for(i=0; i<c_iterLength(cms->clients) && result == NULL; i++){
        client = cms_client(c_iterObject(cms->clients, i));

        if(client->ip == cms->soap->ip){
            if(cms_thread(client)->terminate == FALSE){
                result = client;
            }
        }
    }

    if( (result == NULL) && (c_iterLength(cms->clients) < cms->configuration->maxClients))
    {
        result                  = cms_clientNew(cms->soap->ip, cms);
        cms->clients            = c_iterInsert(cms->clients, result);
        cms_clientStart(result);
        cms_serviceUpdateStatistics(cms);

        if(cms->configuration->verbosity >= 4){
            OS_REPORT(OS_INFO, CMS_CONTEXT, 0,
                            "Client thread started for IP: %d.%d.%d.%d",
                            (int)(result->ip>>24)&0xFF,
                            (int)(result->ip>>16)&0xFF,
                            (int)(result->ip>>8)&0xFF,
                            (int)(result->ip&0xFF));
        }
    }
    os_mutexUnlock(&cms->clientMutex);

    return result;
}

void
cms_serviceRemoveClient(
    cms_service cms,
    cms_client client)
{
    if(cms != NULL){
        os_mutexLock(&cms->clientMutex);
        c_iterTake(cms->clients, client);
        cms->clientGarbage = c_iterInsert(cms->clientGarbage, client);
        cms_serviceUpdateStatistics(cms);
        os_mutexUnlock(&cms->clientMutex);
    }
}

