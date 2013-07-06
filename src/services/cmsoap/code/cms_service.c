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
#include "cms_service.h"
#include "cms_configuration.h"
#include "cms_thread.h"
#include "cms_client.h"
#include "cmx_factory.h"
#include "cmx__factory.h"
#include <soapH.h>
#include "u_service.h"
#include "u_participant.h"
#include "u_dispatcher.h"
#include "v_service.h"
#include "v_participant.h"
#include "kernelModule.h"
#include "v_participantQos.h"
#include "v_cmsoapStatistics.h"
#include "../code/v__statisticsInterface.h"
#include "v_group.h"
#include "os_heap.h"
#include "os_report.h"
#include "os_socket.h"
#include "v_qos.h"
#include "os_stdlib.h"

#define CMSERVICE_ATTACH_TIMEOUT 30

static void
cms_serviceStatisticsAction(
    v_entity entity,
    c_voidp args)
{
    OS_UNUSED_ARG(args);
    v_statisticsULongValueInc(v_cmsoap, connectedClients, entity);
    v_statisticsMaxValueSetValue(v_cmsoap, maxConnectedClients, entity,
        *(v_statisticsGetRef(v_cmsoap, connectedClients, entity)));
}

static void
cms_serviceUpdateStatistics(
    cms_service service)
{
    if(service != NULL){
        u_entityAction(u_entity(service->uservice), cms_serviceStatisticsAction, service);
    }
}

static void*
cms_serviceCollectGarbage(
    void* arg)
{
    cms_service cms;
    os_time update;
    cms_thread client;
    c_bool garbagePresent;

    cms = cms_service(arg);
    update.tv_sec = 2;
    update.tv_nsec = 0;

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


        if(cms->terminate == FALSE){
            os_nanoSleep(update);
        }
    }
    c_iterFree(cms->clientGarbage);

    return NULL;
}

static void
cms_serviceActionGroups(
    v_entity e,
    c_voidp args)
{
    v_service s;
    c_iter groups;
    v_group group;

    OS_UNUSED_ARG(args);
    s = v_service(e);
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
    os_time update;
    v_duration duration;
    cms = cms_service(arg);

    update.tv_sec = cms->configuration->leaseRenewalPeriod.seconds;
    update.tv_nsec = cms->configuration->leaseRenewalPeriod.nanoseconds;

    while(cms->terminate == FALSE){
        u_serviceRenewLease(cms->uservice, cms->configuration->leasePeriod);

        if(cms->configuration->verbosity >= 7){
            OS_REPORT(OS_INFO, CMS_CONTEXT, 0,
                "CMSOAP service lease updated.");
        }
        if(cms->terminate == FALSE){
            os_nanoSleep(update);
        }
    }
    duration.seconds = 20;
    duration.nanoseconds = 0;
    u_serviceRenewLease(cms->uservice, duration);
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
            cms->terminate = TRUE;
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
    cms->soap->accept_timeout = 3;
#ifdef PIKEOS_POSIX
    cms->soap->bind_flags = 0;
#else
    cms->soap->bind_flags = SO_REUSEADDR;
#endif

    master = soap_bind(cms->soap, NULL, cms->configuration->port,
                       cms->configuration->backlog);

    if(master < 0){
        if(cms->configuration->verbosity >= 1){
            OS_REPORT_1(OS_ERROR, CMS_CONTEXT, 0, "Could not bind to port %d",
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
    c_bool success;
    c_char* config;
    os_mutexAttr attr;
    os_result osr;

    success = FALSE;


    if(cms != NULL){
        success = cms_serviceInitConfiguration(name, cms);

        if(success == TRUE){
            if(cms->configuration->verbosity >= 3){
                config = cms_configurationFormat(cms->configuration);
                OS_REPORT(OS_INFO, CMS_CONTEXT, 0, config);
                os_free(config);
            }
            success = cms_serviceInitLease(cms);

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
                    osr = os_mutexAttrInit(&attr);

                    if(osr == os_resultSuccess){
                        attr.scopeAttr = OS_SCOPE_PRIVATE;
                        osr = os_mutexInit(&cms->clientMutex, &attr);

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

    C_STRUCT(v_participantQos) q;
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

            service->uservice = u_serviceNew(uri, CMSERVICE_ATTACH_TIMEOUT, name, NULL, U_SERVICE_CMSOAP, NULL);

            if(service->uservice != NULL) {
                /*disable all events.*/
                u_dispatcherSetEventMask(u_dispatcher(service->uservice), 0);

                u_entityAction(u_entity(service->uservice), cms_serviceActionGroups, NULL);

                u_serviceChangeState(service->uservice, STATE_INITIALISING);
                success = cms_serviceInit(name, service);

                result = u_participantQosInit((v_participantQos)&q);
                if (result == U_RESULT_OK) {
                    /* Insert service information in userData QoS */
                    len = sizeof(struct sockaddr);
                    errcode = os_sockGetsockname (service->soap->master, (struct sockaddr*)&addr, len);
                    if(errcode == os_resultSuccess) {
                        OS_REPORT_1(OS_INFO, CMS_CONTEXT, 0, "SOAP service is reachable via port %d",ntohs(addr.sin_port));

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
                                                ipTagStr = os_realloc(ipTagStr, strlen(ipTagStr) + chars + 1);
                                            } else {
                                                ipTagStr = os_malloc(chars + 1);
                                                *ipTagStr = '\0';
                                            }
                                            ipTagStr = os_strcat(ipTagStr, tmp);
                                        }
                                    }
                                }
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

                        q.userData.size = strlen(xmlStr);
                        q.userData.value = os_malloc(q.userData.size);
                        memcpy(q.userData.value, xmlStr, q.userData.size);

                        if(service->configuration->verbosity >= 5){
                            OS_REPORT_1(OS_INFO, CMS_CONTEXT, 0, "SOAP userData: %s", xmlStr);
                        }
                        os_free(xmlStr);
                    } else {
                        q.userData.size = 0;
                        q.userData.value = NULL;
                        if(service->configuration->verbosity >= 1){
                            OS_REPORT(OS_WARNING, CMS_CONTEXT, 0, "Could not get SOAP port.");
                        }
                    }

                    result = u_entitySetQoS(u_entity(service->uservice), (v_qos)&q);
                    if (result != U_RESULT_OK) {
                        if(service->configuration->verbosity >= 1){
                            OS_REPORT(OS_WARNING, CMS_CONTEXT, 0,"Could not update the participantQos for publication of the SOAP ip address and port.");
                        }
                    }
                    os_free(q.userData.value);
                } else {
                    if(service->configuration->verbosity >= 1){
                        OS_REPORT(OS_WARNING, CMS_CONTEXT, 0,"Could not initiate participantQos for SOAP service ip address and port publication.");
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
        cms->terminate = TRUE;

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
                    OS_REPORT_1(OS_WARNING, CMS_CONTEXT, 0,
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
            u_serviceFree(cms->uservice);
        }
        cmx_detach();


        os_free(cms);
    }
}

cms_client
cms_serviceLookupClient(
    cms_service cms,
    struct soap* soap,
    const c_char* uri)
{
    int i;
    cms_client client;
    cms_client result;

    result = NULL;
    os_mutexLock(&cms->clientMutex);

    for(i=0; i<c_iterLength(cms->clients) && result == NULL; i++){
        client = cms_client(c_iterObject(cms->clients, i));

        if(client->ip == soap->ip){
            if(cms_thread(client)->terminate == FALSE){
                result = client;
            }
        }
    }

    if( (result == NULL) &&
        (((c_ulong)c_iterLength(cms->clients)) < cms->configuration->maxClients))
    {
        result                  = cms_clientNew(soap->ip, cms, uri);
        cms->clients            = c_iterInsert(cms->clients, result);
        cms_clientStart(result);
        cms_serviceUpdateStatistics(cms);

        if(cms->configuration->verbosity >= 4){
            OS_REPORT_4(OS_INFO, CMS_CONTEXT, 0,
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

