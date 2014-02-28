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
#include "cmsoap.h"
#include "cms_service.h"
#include "cms_thread.h"
#include "cms_client.h"
#include "cms_soapThread.h"

#include "cmx_participant.h"
#include "cmx_entity.h"
#include "cmx_topic.h"
#include "cmx_reader.h"
#include "cmx_writer.h"
#include "cmx_service.h"
#include "cmx_snapshot.h"
#include "cmx_readerSnapshot.h"
#include "cmx_writerSnapshot.h"
#include "cmx_dataReader.h"
#include "cmx_domain.h"
#include "cmx_publisher.h"
#include "cmx_waitset.h"
#include "cmx_query.h"
#include "cmx_subscriber.h"
#include "cmx_storage.h"
#include "cmx_factory.h"

#include "u_entity.h"
#include "os_time.h"
#include "os_heap.h"
#include "os_report.h"
#include <soapH.h>

#include <string.h>
#include <stdio.h>

/*static cms_uri_cache uri_cache = NULL;*/


OPENSPLICE_ENTRYPOINT (ospl_cmsoap)
{
    cms_service cms;
    os_time sleepTime;
    cms_client client;
    c_bool success;
    char* name;
    char* config;
    c_long slave;
    struct soap* soap;

#ifdef INTEGRITY
    Error err;
    Semaphore soapSvcStartSem = SemaphoreObjectNumber(13);
#endif
    slave = -1;
    soap = NULL;

#ifdef INTEGRITY
       name = "cmsoap";
       config = "file:///ospl.xml";

       err = WaitForSemaphore(soapSvcStartSem);
       assert ( err == Success );
       argc = 3;
#endif

       if(argc == 3)
       {
#ifndef INTEGRITY
          name = argv[1];
          config = argv[2];
#endif
          cms = cms_serviceNew(name,config);

          if(cms != NULL){
             sleepTime.tv_sec=0;
             sleepTime.tv_nsec=10*1000L*1000L;

             while(cms->terminate == FALSE){
                while( (slave < 0) && (cms->terminate == FALSE)){
                   slave = soap_accept(cms->soap);
                }
                if (slave < 0) {
                   if(cms->configuration->verbosity > 0){
                      /* soap_accept is likely to return -1 when the soap service is shutdown
                       * so should be an INFO message here rather than a WARNING
                       */
                      OS_REPORT(OS_INFO, CMS_CONTEXT, 0,
                                "not accepting requests.");
                   }
                   cms->terminate = TRUE;
                } else {
                   if(cms->configuration->verbosity > 6){
                      OS_REPORT_5(OS_INFO, CMS_CONTEXT, 0,
                                  "Thread %d accepts connection from IP %d.%d.%d.%d\n",
                                  slave,
                                  (int)(cms->soap->ip>>24)&0xFF,
                                  (int)(cms->soap->ip>>16)&0xFF,
                                  (int)(cms->soap->ip>>8)&0xFF,
                                  (int)(cms->soap->ip&0xFF));
                   }
                   client = cms_serviceLookupClient(cms, cms->soap, config);

                   if(client != NULL){
                      soap = soap_copy(cms->soap);

                      if (soap == NULL) {
                         if(cms->configuration->verbosity > 0){
                            OS_REPORT(OS_ERROR, CMS_CONTEXT, 0,
                                      "Could not allocate SOAP environment.");
                         }
                         cms->terminate = TRUE;
                      } else {
                         soap->socket = slave;
                         slave = -1;
                         success = cms_clientHandleRequest(client, soap);

                         if(success == FALSE){
                            soap->error = soap_receiver_fault(soap,
                                                              "Could not handle request.", NULL);
                            soap_send_fault(soap);
                            soap_destroy(soap);
                            soap_end(soap);
                            soap_done(soap);
                            os_free(soap);
                            soap = NULL;
                         }
                      }
                   } else {
                      if(cms->configuration->verbosity > 3){
                         OS_REPORT(OS_INFO, CMS_CONTEXT, 0,
                                   "Maximum number of clients reached.");
                      }

                      if (soap == NULL) {
                         if(cms->configuration->verbosity > 0){
                            OS_REPORT(OS_ERROR, CMS_CONTEXT, 0,
                                      "Could not allocate SOAP environment.");
                         }
                         cms->terminate = TRUE;
                      } else {
                         soap->socket = slave;
                         slave = -1;
                         soap->error = soap_receiver_fault(soap,
                                                           "Maximum number of clients reached.",
                                                           NULL);
                         soap_send_fault(soap);
                         soap_destroy(soap);
                         soap_end(soap);
                         soap_done(soap);
                         soap = NULL;
                      }
                   }
                }
             }
             cms_serviceFree(cms);

          }
       } else {
          printf("Usage: %s <name> <uri>\n", argv[0]);
       }
    return 0;
}

static c_bool
validateInitialization(
    struct soap *soap)
{
    c_bool result;
    cms_client client;

    client = cms_client(cms_soapThread(soap->user)->client);
    result = cms_clientInitialized(client);

    if(result == FALSE){
        soap->error = soap_receiver_fault(soap,
                    "Connection lost.",
                    "Client lease has probably expired.");
        soap_send_fault(soap);

        client->internalFree = TRUE;
        cms_thread(client)->terminate = TRUE;
        os_mutexLock(&client->conditionMutex);
        os_condSignal(&client->condition);
        os_mutexUnlock(&client->conditionMutex);
    } else {
        client->leaseTime = os_timeGet();
        u_entityAction( u_entity(client->service->uservice),
                            cms_soapThreadStatisticsRequestHandledAdd,
                            client->service);
    }
    return result;
}

static void
storeResultInThread(
    cms_thread thread,
    c_char* result)
{
    thread->results = c_iterInsert(thread->results, result);
}

int cms__updateLease(
    struct soap *soap,
    char* dummy,
    char** result)
{
    cms_thread it;
    cms_client client;

    it = cms_thread(soap->user);
    client = cms_soapThread(it)->client;
    client->leaseTime = os_timeGet();
    *result = os_strdup("<result>OK</result>");

    storeResultInThread(it, (c_char*)(*result));

    return SOAP_OK;
}

int
cms__initialise(
    struct soap *soap,
    char* dummy,
    char** result)
{
    cms_thread it;
    cms_client client;

    it = cms_thread(soap->user);

    client = cms_soapThread(it)->client;
    *result = os_strdup("<result>OK</result>");
    storeResultInThread(it, *result);
    cms_clientAdd(client);

    return SOAP_OK;
}

int
cms__detach(
    struct soap *soap,
    char* dummy,
    char** result)
{
    cms_thread it;
    cms_client client;

    it = cms_thread(soap->user);

    /*
     * Clearing the keep alive flags of input and output makes sure
     * the connection is closed after this call is handled. This is
     * necessary to prevent the client thread to be re-used for a future
     * client request, because re-use leads to a deadlock in this case.
     */
    soap_clr_imode(soap, SOAP_IO_KEEPALIVE);
    soap_clr_omode(soap, SOAP_IO_KEEPALIVE);

    client = cms_soapThread(it)->client;
    *result = os_strdup("<result>OK</result>");

    storeResultInThread(it, *result);

    if(client->service->terminate == FALSE){
        cms_clientRemove(client);
    }
    return SOAP_OK;
}
int
cms__participantNew(
    struct soap *soap,
    char* uri,
    int timeout,
    char* name,
    char* qos,
    c_char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = cmx_participantNew(it->uri, timeout, name, qos);
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__participantAllParticipants(
    struct soap *soap,
    char* participant,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = cmx_participantAllParticipants(participant);
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__participantAllTopics(
    struct soap *soap,
    char* participant,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = cmx_participantAllTopics(participant);
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__participantAllDomains(
    struct soap *soap,
    char* participant,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = cmx_participantAllDomains(participant);
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__participantFindTopic(
    struct soap *soap,
    char* participant,
    char* topicName,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = cmx_participantFindTopic(participant, topicName);
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__participantRegisterType(
    struct soap* soap,
    char* participant,
    char* type,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = os_strdup(cmx_participantRegisterType(participant, type));
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__entityFree(
    struct soap *soap,
    c_char* entity,
    char** dummy)
{
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        cmx_entityFree((c_char*)(os_strdup(entity)));

        *dummy = NULL;

        code = SOAP_OK;
    }
    return code;
}

int
cms__entityGetStatus(
    struct soap* soap,
    char* entity,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = cmx_entityStatus(entity);
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__entityGetQos(
    struct soap* soap,
    char* entity,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = cmx_entityQoS(entity);
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__entitySetQos(
    struct soap* soap,
    char* entity,
    char* qos,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = os_strdup(cmx_entitySetQoS(entity, qos));
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__entityStatistics(
    struct soap* soap,
    char* entity,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;
    it = cms_thread(soap->user);

    if(validateInitialization(soap) == TRUE){
        *result = cmx_entityStatistics(entity);
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__entityResetStatistics(
    struct soap* soap,
    char* entity,
    char* fieldName,
    char** result)
{
    cms_thread it;
    const char* field;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);

        field = fieldName;

        if(fieldName != NULL){
            if(strcmp("", fieldName) == 0){
                field = NULL;
            }
        }
        *result = os_strdup(cmx_entityResetStatistics(entity, field));
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__entityEnable(
    struct soap* soap,
    char* entity,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = os_strdup(cmx_entityEnable(entity));
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__entityOwnedEntities(
    struct soap *soap,
    char* entity,
    char* filter,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = cmx_entityOwnedEntities(entity, filter);
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__entityDependantEntities(
    struct soap *soap,
    char* entity,
    char* filter,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = cmx_entityDependantEntities(entity, filter);
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__topicDataType(
    struct soap* soap,
    char* topic,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = cmx_topicDataType(topic);
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__readerDataType(
    struct soap* soap,
    char* reader,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = cmx_readerDataType(reader);
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__writerDataType(
    struct soap* soap,
    char* writer,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = cmx_writerDataType(writer);
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__serviceGetState(
    struct soap* soap,
    char* service,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = cmx_serviceGetState(service);
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__readerRead(
    struct soap* soap,
    char* reader,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = cmx_readerRead(reader);
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__readerTake(
    struct soap* soap,
    char* reader,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = cmx_readerTake(reader);
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__readerReadNext(
    struct soap* soap,
    char* reader,
    char* localId,
    char* systemId,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = cmx_readerReadNext(reader, localId, systemId);
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__readerSnapshotNew(
    struct soap* soap,
    char* reader,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = cmx_readerSnapshotNew(reader);
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__writerSnapshotNew(
    struct soap* soap,
    char* writer,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = cmx_writerSnapshotNew(writer);
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__snapshotFree(
    struct soap* soap,
    char* snapshot,
    char** dummy)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        cmx_snapshotFree((c_char*)os_strdup(snapshot));
        *dummy = os_strdup(snapshot);
        storeResultInThread(it, *dummy);
        code = SOAP_OK;
    }
    return code;
}

int
cms__snapshotRead(
    struct soap* soap,
    char* snapshot,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = cmx_snapshotRead(snapshot);
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__snapshotTake(
    struct soap* soap,
    char* snapshot,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = cmx_snapshotTake(snapshot);
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__writerWrite(
    struct soap* soap,
    char* writer,
    char* userData,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = os_strdup(cmx_writerWrite(writer, userData));
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__writerDispose(
    struct soap* soap,
    char* writer,
    char* userData,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = os_strdup(cmx_writerDispose(writer, userData));
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__writerWriteDispose(
    struct soap* soap,
    char* writer,
    char* userData,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = os_strdup(cmx_writerWriteDispose(writer, userData));
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__writerRegister(
    struct soap* soap,
    char* writer,
    char* userData,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = os_strdup(cmx_writerRegister(writer, userData));
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__writerUnregister(
    struct soap* soap,
    char* writer,
    char* userData,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = os_strdup(cmx_writerUnregister(writer, userData));
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__publisherNew(
    struct soap* soap,
    char* participant,
    char* name,
    char* qos,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = cmx_publisherNew(participant, name, qos);
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__subscriberNew(
    struct soap* soap,
    char* participant,
    char* name,
    char* qos,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = cmx_subscriberNew(participant, name, qos);
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__domainNew(
    struct soap* soap,
    char* participant,
    char* name,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = cmx_domainNew(participant, name);
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__writerNew(
    struct soap* soap,
    char* publisher,
    char* name,
    char* topic,
    char* qos,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = cmx_writerNew(publisher, name, topic, qos);
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__dataReaderNew(
    struct soap* soap,
    char* subscriber,
    char* name,
    char* view,
    char* qos,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = cmx_dataReaderNew(subscriber, name, view, qos);
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__dataReaderWaitForHistoricalData(
    struct soap* soap,
    char* dataReader,
    int seconds,
    int nanoseconds,
    char** result)
{
    cms_thread it;
    int code;
    c_time waitTime;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        waitTime.seconds = (c_long)seconds;
        waitTime.nanoseconds = (c_ulong)nanoseconds;
        *result = os_strdup(cmx_dataReaderWaitForHistoricalData(dataReader, waitTime));
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__queryNew(
    struct soap* soap,
    char* reader,
    char* name,
    char* expression,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = cmx_queryNew(reader, name, expression);
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__topicNew(
    struct soap* soap,
    char* participant,
    char* name,
    char* typeName,
    char* keyList,
    char* qos,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = cmx_topicNew(participant, name, typeName, keyList, qos);
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__publisherPublish(
    struct soap* soap,
    char* publisher,
    char* expression,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = os_strdup(cmx_publisherPublish(publisher, expression));
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__subscriberSubscribe(
    struct soap* soap,
    char* subscriber,
    char* expression,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = os_strdup(cmx_subscriberSubscribe(subscriber, expression));
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__waitsetNew(
    struct soap* soap,
    char* participant,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = cmx_waitsetNew(participant);
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__waitsetAttach(
    struct soap* soap,
    char* waitset,
    char* entity,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = os_strdup(cmx_waitsetAttach(waitset, entity));
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__waitsetDetach(
    struct soap* soap,
    char* waitset,
    char* entity,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = os_strdup(cmx_waitsetDetach(waitset, entity));
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__waitsetWait(
    struct soap* soap,
    char* waitset,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = cmx_waitsetWait(waitset);
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__waitsetTimedWait(
    struct soap* soap,
    char* waitset,
    int seconds,
    int nanoseconds,
    char** result)
{
    cms_thread it;
    int code;
    c_time waitTime;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        waitTime.seconds = (c_long)seconds;
        waitTime.nanoseconds = (c_ulong)nanoseconds;
        *result = cmx_waitsetTimedWait(waitset, waitTime);
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__waitsetGetEventMask(
    struct soap* soap,
    char* waitset,
    unsigned int* result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = cmx_waitsetGetEventMask(waitset);
        code = SOAP_OK;
    }
    return code;
}

int
cms__waitsetSetEventMask(
    struct soap* soap,
    char* waitset,
    unsigned int mask,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = os_strdup(cmx_waitsetSetEventMask(waitset, (c_ulong)mask));
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__storageOpen(
    struct soap* soap,
    char* attrs,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = cmx_storageOpen(attrs);
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }

    return code;
}
int
cms__storageClose(
    struct soap* soap,
    char* storage,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = cmx_storageClose(storage);
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }

    return code;
}

int
cms__storageAppend(
    struct soap* soap,
    char* storage,
    char* metadata,
    char* data,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = cmx_storageAppend(storage, metadata, data);
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }

    return code;
}

int
cms__storageRead(
    struct soap* soap,
    char* storage,
    char** result)

{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = cmx_storageRead(storage);
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }

    return code;
}

int
cms__storageGetType(
    struct soap* soap,
    char* storage,
    char* typeName,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = cmx_storageGetType(storage, typeName);
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }

    return code;
}

int
cms__getVersion(
    struct soap* soap,
    char* dummy,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = cmx_getVersion();
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}

int
cms__entitiesStatistics(
    struct soap* soap,
    char* entities,
    char** result)
{
    cms_thread it;
    int code;

    code = SOAP_FAULT;

    if(validateInitialization(soap) == TRUE){
        it = cms_thread(soap->user);
        *result = cmx_entitiesStatistics(entities);
        storeResultInThread(it, *result);
        code = SOAP_OK;
    }
    return code;
}
