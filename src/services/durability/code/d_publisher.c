/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "d__publisher.h"
#include "d__durability.h"
#include "d__configuration.h"
#include "d__admin.h"
#include "d__misc.h"
#include "d__historicalDataRequestListener.h"
#include "d__historicalDataRequest.h"
#include "d__durabilityState.h"
#include "d__partitionTopicState.h"
#include "d__historicalData.h"
#include "d__group.h"
#include "d__capability.h"
#include "d_message.h"
#include "d_status.h"
#include "d_sampleRequest.h"
#include "d_groupsRequest.h"
#include "d_nameSpaces.h"
#include "d_nameSpacesRequest.h"
#include "d__mergeState.h"
#include "d_deleteData.h"
#include "d_qos.h"
#include "d_sampleChain.h"
#include "u_observable.h"
#include "u_publisher.h"
#include "u_participant.h"
#include "u_writer.h"
#include "u_writerQos.h"
#include "v_time.h"
#include "v_writer.h"
#include "v_group.h"
#include "v_state.h"
#include "v_partition.h"
#include "os_heap.h"
#include "os_report.h"



#define HISTORICAL_DATA_KIND_BEAD   0
#define HISTORICAL_DATA_KIND_LINK   1

/* Message type */
#define MESSAGE_FLAG_WRITE          0
#define MESSAGE_FLAG_DISPOSE        1
#define MESSAGE_FLAG_UNREGISTER     2
#define MESSAGE_FLAG_WRITE_DISPOSE  3


d_publisher
d_publisherNew(
    d_admin admin)
{
    d_publisher     publisher;
    d_durability    durability;
    d_configuration config;
    v_publisherQos  publisherQos = NULL;
    v_writerQos     writerQos = NULL;
    u_result result;
    d_thread self = d_threadLookupSelf();

    assert(d_adminIsValid(admin));

    /* Allocate publisher object */
    publisher = d_publisher(os_malloc(C_SIZEOF(d_publisher)));
    if (publisher) {
        /* zero fill structure so d_publisherDeinit can safely be invoked */
        memset(publisher, 0, sizeof(C_STRUCT(d_publisher)));
        /* Call super-init */
        d_objectInit(d_object(publisher), D_PUBLISHER,
                     (d_objectDeinitFunc)d_publisherDeinit);
        /* Initialize publisher */
        publisher->enabled   = TRUE;
        publisher->admin     = admin;
        durability           = d_adminGetDurability(admin);
        config               = d_durabilityGetConfiguration(durability);
        /* Create the publisher for the durability protocol */
        publisherQos         = d_publisherQosNew(config->partitionName);
        if (publisherQos == NULL) {
            goto cleanup_publisher;
        }
        publisher->publisher = u_publisherNew(
                                    u_participant(d_durabilityGetService(durability)),
                                    config->publisherName, publisherQos, TRUE);
        d_publisherQosFree(publisherQos);
        if (publisher->publisher == NULL) {
            goto cleanup_publisher;
        }
        /* Create the publisher for client durability, but only
         * if client durability is enabled.
         */
        if (config->clientDurabilityEnabled) {
            publisherQos         = d_publisherQosNew(config->clientDurabilityPartitionName);
            if (publisherQos == NULL) {
                goto cleanup_publisher2;
            }
            publisher->publisher2 = u_publisherNew(
                                        u_participant(d_durabilityGetService(durability)),
                                        D_CLIENT_DURABILITY_PUBLISHER_NAME, publisherQos, TRUE);
            d_publisherQosFree(publisherQos);
            if (publisher->publisher2 == NULL) {
                goto cleanup_publisher2;
            }
        } else {
            publisher->publisher2 = NULL;
        }
        /* Cache capabilitySupport in the publisher to enable
         * fast access in d_publisherInitMessage().
         */
        publisher->capabilitySupport = config->capabilitySupport;

        publisher->statusWriter                = NULL;
        publisher->newGroupWriter              = NULL;
        publisher->groupsRequestWriter         = NULL;
        publisher->sampleRequestWriter         = NULL;
        publisher->sampleChainWriter           = NULL;
        publisher->nameSpacesWriter            = NULL;
        publisher->nameSpacesRequestWriter     = NULL;
        publisher->deleteDataWriter            = NULL;
        publisher->durabilityStateWriter       = NULL;
        publisher->capabilityWriter            = NULL;

        /* writerQos for status writer */
        writerQos                          = d_writerQosNew(
                                                V_DURABILITY_VOLATILE,
                                                V_RELIABILITY_RELIABLE,
                                                V_ORDERBY_RECEPTIONTIME,
                                                config->heartbeatLatencyBudget,
                                                config->heartbeatTransportPriority);
        if (writerQos == NULL) {
             goto cleanup_publisher;
        }
        writerQos->history.v.kind = V_HISTORY_KEEPLAST;
        writerQos->history.v.depth = 1;

        /* status writer */
        publisher->statusNumber = 0;
        publisher->statusWriter = u_writerNew (publisher->publisher,
                                               "statusWriter",
                                               d_adminGetStatusTopic(admin),
                                               writerQos);
        if (publisher->statusWriter == NULL) {
            goto cleanup_writerQos;
        }
        result = u_entityEnable(u_entity(publisher->statusWriter));
        if (result != U_RESULT_OK) {
            goto cleanup_writerQos;
        }
        result = u_observableAction(u_observable(publisher->statusWriter),
                       d_publisherEnsureServicesAttached,
                       durability);
        if (result != U_RESULT_OK) {
            goto cleanup_writerQos;
        }
        /* Cleanup writerQos for statusWriter */
        d_writerQosFree(writerQos);

        /* writerQos for most durability protocol writers */
        writerQos                          = d_writerQosNew(
                                                V_DURABILITY_VOLATILE,
                                                V_RELIABILITY_RELIABLE,
                                                V_ORDERBY_RECEPTIONTIME,
                                                config->latencyBudget,
                                                config->transportPriority);
        if (writerQos == NULL) {
            goto cleanup_publisher;
        }

        /* Create the capability writer as one of the first writers because
         * the capability topic is written as one of the first topic.
         * This increases the chance that ddsi can connect to the writer before
         * the message is written.
         */
        if (config->capabilitySupport) {
            /* capability writer, only created if capabilitySupported=TRUE */
            publisher->capabilityNumber = 0;
            publisher->capabilityWriter = u_writerNew (publisher->publisher,
                                                       "capabilityWriter",
                                                       d_adminGetCapabilityTopic(admin),
                                                       writerQos);
            if (publisher->capabilityWriter == NULL) {
                goto cleanup_writerQos;
            }
            result = u_entityEnable(u_entity(publisher->capabilityWriter));
            if (result != U_RESULT_OK) {
                goto cleanup_writerQos;
            }
            result = u_observableAction(u_observable(publisher->capabilityWriter),
                           d_publisherEnsureServicesAttached,
                           durability);
            if (result != U_RESULT_OK) {
                goto cleanup_writerQos;
            }
       }

        /* newGroup writer */
        publisher->newGroupNumber = 0;
        publisher->newGroupWriter = u_writerNew (publisher->publisher,
                                                 "newGroupWriter",
                                                 d_adminGetNewGroupTopic(admin),
                                                 writerQos);
        if (publisher->newGroupWriter == NULL) {
            goto cleanup_writerQos;
        }
        result = u_entityEnable(u_entity(publisher->newGroupWriter));
        if (result != U_RESULT_OK) {
            goto cleanup_writerQos;
        }
        result = u_observableAction(u_observable(publisher->newGroupWriter),
                       d_publisherEnsureServicesAttached,
                       durability);
        if (result != U_RESULT_OK) {
            goto cleanup_writerQos;
        }

        /* groupsRequest writer */
        publisher->groupsRequestNumber = 0;
        publisher->groupsRequestWriter = u_writerNew (publisher->publisher,
                                                      "groupsRequestWriter",
                                                      d_adminGetGroupsRequestTopic(admin),
                                                      writerQos);
        if (publisher->groupsRequestWriter == NULL) {
            goto cleanup_writerQos;
        }
        result = u_entityEnable(u_entity(publisher->groupsRequestWriter));
        if (result != U_RESULT_OK) {
            goto cleanup_writerQos;
        }
        result = u_observableAction(u_observable(publisher->groupsRequestWriter),
                       d_publisherEnsureServicesAttached,
                       durability);
        if (result != U_RESULT_OK) {
            goto cleanup_writerQos;
        }

        /* sampleRequest writer */
        publisher->sampleRequestNumber = 0;
        publisher->sampleRequestWriter = u_writerNew (publisher->publisher,
                                                      "sampleRequestWriter",
                                                      d_adminGetSampleRequestTopic(admin),
                                                      writerQos);
        if (publisher->sampleRequestWriter == NULL) {
            d_writerQosFree(writerQos);
            goto cleanup_publisher;
        }
        result = u_entityEnable(u_entity(publisher->sampleRequestWriter));
        if (result != U_RESULT_OK) {
            goto cleanup_writerQos;
        }
        result = u_observableAction(u_observable(publisher->sampleRequestWriter),
                       d_publisherEnsureServicesAttached,
                       durability);
        if (result != U_RESULT_OK) {
            goto cleanup_writerQos;
        }

        /* nameSpaces writer */
        publisher->nameSpacesNumber = 0;
        publisher->nameSpacesWriter = u_writerNew (publisher->publisher,
                                                   "nameSpacesWriter",
                                                   d_adminGetNameSpacesTopic(admin),
                                                   writerQos);
        if (publisher->nameSpacesWriter == NULL) {
            goto cleanup_writerQos;
        }
        result = u_entityEnable(u_entity(publisher->nameSpacesWriter));
        if (result != U_RESULT_OK) {
            goto cleanup_writerQos;
        }
        result = u_observableAction(u_observable(publisher->nameSpacesWriter),
                       d_publisherEnsureServicesAttached,
                       durability);
        if (result != U_RESULT_OK) {
            goto cleanup_writerQos;
        }

        /* nameSpacesRequest writer */
        publisher->nameSpacesRequestNumber = 0;
        publisher->nameSpacesRequestWriter = u_writerNew (publisher->publisher,
                                                          "nameSpacesRequestWriter",
                                                          d_adminGetNameSpacesRequestTopic(admin),
                                                          writerQos);
        if (publisher->nameSpacesRequestWriter == NULL) {
            goto cleanup_writerQos;
        }
        result = u_entityEnable(u_entity(publisher->nameSpacesRequestWriter));
        if (result != U_RESULT_OK) {
            goto cleanup_writerQos;
        }
        result = u_observableAction(u_observable(publisher->nameSpacesRequestWriter),
                       d_publisherEnsureServicesAttached,
                       durability);
        if (result != U_RESULT_OK) {
            goto cleanup_writerQos;
        }

        /* deleteData writer */
        publisher->deleteDataNumber = 0;
        publisher->deleteDataWriter = u_writerNew (publisher->publisher,
                                                   "deleteDataWriter",
                                                   d_adminGetDeleteDataTopic(admin),
                                                   writerQos);
        if (publisher->deleteDataWriter == NULL) {
            goto cleanup_writerQos;
        }
        result = u_entityEnable(u_entity(publisher->deleteDataWriter));
        if (result != U_RESULT_OK) {
            goto cleanup_writerQos;
        }
        result = u_observableAction(u_observable(publisher->deleteDataWriter),
                       d_publisherEnsureServicesAttached,
                       durability);
        if (result != U_RESULT_OK) {
            goto cleanup_writerQos;
        }

        /* Cleanup writerQos for most durability protocol writers */
        d_writerQosFree(writerQos);

        if (config->clientDurabilityEnabled) {

            /* Set writerQos for durability state writer */
            writerQos                          = d_writerQosNew(
                                                    V_DURABILITY_VOLATILE,
                                                    V_RELIABILITY_RELIABLE,
                                                    V_ORDERBY_SOURCETIME,
                                                    config->heartbeatLatencyBudget,
                                                    config->heartbeatTransportPriority);
            if (writerQos == NULL) {
                 goto cleanup_publisher;
            }

            /* d_writerQosNew does not always return the proper settings.
             * The following settings ensure that the writer qos is correct
             */
            writerQos->reliability.v.max_blocking_time = OS_DURATION_INIT(0,100000000);
            /* durabilityState writer */
            assert(publisher->publisher2);
            publisher->durabilityStateNumber = 0;
            publisher->durabilityStateWriter = u_writerNew(publisher->publisher2,
                                                           "durabilityStateWriter",
                                                           d_adminGetDurabilityStateTopic(admin),
                                                           writerQos);
            if (publisher->durabilityStateWriter == NULL) {
                goto cleanup_writerQos;
            }
            result = u_entityEnable(u_entity(publisher->durabilityStateWriter));
            if (result != U_RESULT_OK) {
                goto cleanup_writerQos;
            }
            result = u_observableAction(u_observable(publisher->durabilityStateWriter),
                           d_publisherEnsureServicesAttached,
                           durability);
            if (result != U_RESULT_OK) {
                goto cleanup_writerQos;
            }

            /* Cleanup writerQos for set of topics */
            d_writerQosFree(writerQos);

        }

        /* writerQos for sampleChain */
        writerQos                          = d_writerQosNew(
                                                V_DURABILITY_VOLATILE,
                                                V_RELIABILITY_RELIABLE,
                                                V_ORDERBY_RECEPTIONTIME,
                                                config->alignerLatencyBudget,
                                                config->alignerTransportPriority);
        if (writerQos == NULL) {
            goto cleanup_publisher;
        }

        /* sampleChain writer */
        publisher->sampleChainNumber = 0;
        publisher->sampleChainWriter = u_writerNew (publisher->publisher,
                                                    "sampleChainWriter",
                                                    d_adminGetSampleChainTopic(admin),
                                                    writerQos);
        if (publisher->sampleChainWriter == NULL) {
            goto cleanup_writerQos;
        }
        result = u_entityEnable(u_entity(publisher->sampleChainWriter));
        if (result != U_RESULT_OK) {
            goto cleanup_writerQos;
        }
        result = u_observableAction(u_observable(publisher->sampleChainWriter),
                       d_publisherEnsureServicesAttached,
                       durability);
        if (result != U_RESULT_OK) {
            goto cleanup_writerQos;
        }

        /* Cleanup writerQos for sampleChain */
        d_writerQosFree(writerQos);

        /* WORKAROUND: Increase the possibility that ddsi discovers the writers.
         * OSPL-9939 is there to fix this
         */
        if (config->waitForRemoteReaders) {
            d_sleep(self, 200*OS_DURATION_MILLISECOND);
        }
    }
    return publisher;


cleanup_writerQos:
    d_writerQosFree(writerQos);
cleanup_publisher2:
cleanup_publisher:
    /* Free the publisher */
    d_publisherFree(publisher);

    return NULL;
}


void
d_publisherDeinit(
    d_publisher publisher)
{
    d_durability durability;
    d_configuration config;

    assert(d_publisherIsValid(publisher));

    durability = d_adminGetDurability(publisher->admin);
    config = d_durabilityGetConfiguration(durability);
    if (publisher->statusWriter) {
        d_printTimedEvent(durability, D_LEVEL_FINEST, "destroying status writer\n");
        u_objectFree(u_object(publisher->statusWriter));
        d_printTimedEvent(durability, D_LEVEL_FINEST, "status writer destroyed\n");
        publisher->statusWriter = NULL;
    }
    if (publisher->newGroupWriter) {
        d_printTimedEvent(durability, D_LEVEL_FINEST, "destroying newGroup writer\n");
        u_objectFree(u_object(publisher->newGroupWriter));
        d_printTimedEvent(durability, D_LEVEL_FINEST, "newGroup writer destroyed\n");
        publisher->newGroupWriter = NULL;
    }
    if (publisher->groupsRequestWriter) {
        d_printTimedEvent(durability, D_LEVEL_FINEST, "destroying groupsRequest writer\n");
        u_objectFree(u_object(publisher->groupsRequestWriter));
        d_printTimedEvent(durability, D_LEVEL_FINEST, "groupsRequest writer destroyed\n");
        publisher->groupsRequestWriter = NULL;
    }
    if (publisher->sampleChainWriter) {
        d_printTimedEvent(durability, D_LEVEL_FINEST, "destroying sampleChain writer\n");
        u_objectFree(u_object(publisher->sampleChainWriter));
        d_printTimedEvent(durability, D_LEVEL_FINEST, "sampleChain writer destroyed\n");
        publisher->sampleChainWriter = NULL;
    }
    if (publisher->nameSpacesWriter) {
        d_printTimedEvent(durability, D_LEVEL_FINEST, "destroying nameSpaces writer\n");
        u_objectFree (u_object(publisher->nameSpacesWriter));
        d_printTimedEvent(durability, D_LEVEL_FINEST, "nameSpaces writer destroyed\n");
        publisher->nameSpacesWriter = NULL;
    }
    if (publisher->nameSpacesRequestWriter) {
        d_printTimedEvent(durability, D_LEVEL_FINEST, "destroying nameSpacesRequest writer\n");
        u_objectFree(u_object(publisher->nameSpacesRequestWriter));
        d_printTimedEvent(durability, D_LEVEL_FINEST, "nameSpacesRequest writer destroyed\n");
        publisher->nameSpacesRequestWriter = NULL;
    }
    if (publisher->deleteDataWriter) {
        d_printTimedEvent(durability, D_LEVEL_FINEST, "destroying deleteData writer\n");
        u_objectFree(u_object(publisher->deleteDataWriter));
        d_printTimedEvent(durability, D_LEVEL_FINEST, "deleteData writer destroyed\n");
        publisher->deleteDataWriter = NULL;
    }
    if (publisher->sampleRequestWriter) {
        d_printTimedEvent(durability, D_LEVEL_FINEST, "destroying sampleRequest writer\n");
        u_objectFree (u_object(publisher->sampleRequestWriter));
        d_printTimedEvent(durability, D_LEVEL_FINEST, "sampleRequest writer destroyed\n");
        publisher->sampleRequestWriter = NULL;
    }
    if (publisher->durabilityStateWriter) {
        d_printTimedEvent(durability, D_LEVEL_FINEST, "destroying durabilityState writer\n");
        u_objectFree (u_object(publisher->durabilityStateWriter));
        d_printTimedEvent(durability, D_LEVEL_FINEST, "durabilityState writer destroyed\n");
        publisher->durabilityStateWriter = NULL;
    }
    if (config->capabilitySupport) {
        if (publisher->capabilityWriter) {
            d_printTimedEvent(durability, D_LEVEL_FINEST, "destroying capability writer\n");
            u_objectFree (u_object(publisher->capabilityWriter));
            d_printTimedEvent(durability, D_LEVEL_FINEST, "capability writer destroyed\n");
            publisher->capabilityWriter = NULL;
        }
    }
    if (config->clientDurabilityEnabled) {
        if (publisher->publisher2) {
            d_printTimedEvent(durability, D_LEVEL_FINEST, "destroying user publisher for client durability\n");
            u_objectFree(u_object(publisher->publisher2));
            d_printTimedEvent(durability, D_LEVEL_FINEST, "user publisher for client durability destroyed\n");
            publisher->publisher2 = NULL;
        }
    }
    if (publisher->publisher) {
        d_printTimedEvent(durability, D_LEVEL_FINEST, "destroying user publisher\n");
        u_objectFree(u_object(publisher->publisher));
        d_printTimedEvent(durability, D_LEVEL_FINEST, "user publisher destroyed\n");
        publisher->publisher = NULL;
    }
    /* Call super-deinit */
    d_objectDeinit(d_object(publisher));
}

void
d_publisherFree(
    d_publisher publisher)
{
    assert(d_publisherIsValid(publisher));

    d_objectFree(d_object(publisher));
}

static c_bool
d__publisherStatusWrite(
    d_publisher publisher,
    d_status message,
    c_bool resend)
{
    c_bool result = TRUE;
    u_result ur;
    d_durability durability;
    d_thread const self = d_threadLookupSelf ();

    if(publisher && publisher->enabled){
        /* Status messages with TERMINATING or TERMINATED must
         * be published, so checking needed on the durability
         * state. */
        durability = d_adminGetDurability(publisher->admin);
        d_publisherInitMessage(publisher, d_message(message));

        if (!resend) {
            d_message(message)->sequenceNumber = publisher->statusNumber++;
        }
                
        d_threadAwake(self);
        ur = u_writerWrite(publisher->statusWriter,
                d_publisherStatusWriterCopy,
                message,
                os_timeWGet(),
                U_INSTANCEHANDLE_NIL);

        if (ur == U_RESULT_TIMEOUT) {
            result = FALSE;
        } else if (ur != U_RESULT_OK) {
            d_printTimedEvent(durability, D_LEVEL_SEVERE,
                    "Write of d_status message FAILED with result %s.\n", u_resultImage(ur));
            OS_REPORT(OS_ERROR, D_CONTEXT_DURABILITY, 0,
                    "Write of d_status message FAILED with result %s.", u_resultImage(ur));
            d_durabilityTerminate(durability, TRUE);
        }
    }
    return result;
}

c_bool
d_publisherStatusWrite(
    d_publisher publisher,
    d_status message,
    d_networkAddress addressee)
{
    OS_UNUSED_ARG(addressee);

    assert(d_objectIsValid(d_object(publisher), D_PUBLISHER) == TRUE);

    return d__publisherStatusWrite(publisher, message, FALSE);
}

c_bool
d_publisherStatusResend(
    d_publisher publisher,
    d_status message)
{
    assert(d_objectIsValid(d_object(publisher), D_PUBLISHER) == TRUE);

    return d__publisherStatusWrite(publisher, message, TRUE);
}

c_bool
d_publisherNewGroupWrite(
    d_publisher publisher,
    d_newGroup message,
    d_networkAddress addressee)
{
    c_bool result;
    u_result ur;
    int resendCount;
    c_bool terminate;
    d_durability durability;
    d_serviceState state;
    d_thread const self = d_threadLookupSelf ();

    OS_UNUSED_ARG(addressee);
    result = FALSE;
    assert(d_objectIsValid(d_object(publisher), D_PUBLISHER) == TRUE);

    if(publisher){
        if(publisher->enabled == TRUE){
            durability = d_adminGetDurability(publisher->admin);
            state = d_durabilityGetState(durability);
            if ((state != D_STATE_TERMINATING) && (state != D_STATE_TERMINATED)) {
                /* The service is not about to terminate, so it is OK to
                 * publish newGroup messages.
                 */
                terminate = FALSE;
                resendCount = 0;
                d_publisherInitMessage(publisher, d_message(message));
                d_message(message)->sequenceNumber = publisher->newGroupNumber++;

                while((!result) && (!terminate)){
                    d_threadAwake(self);
                    ur = u_writerWrite(publisher->newGroupWriter,
                                       d_publisherNewGroupWriterCopy,
                                       message,
                                       os_timeWGet(),
                                       U_INSTANCEHANDLE_NIL);

                    if(ur == U_RESULT_OK){
                        result = TRUE;
                    } else if(ur == U_RESULT_TIMEOUT) {
                        terminate = d_durabilityMustTerminate(durability);
                        resendCount++;

                        if(terminate){
                            d_printTimedEvent(durability, D_LEVEL_FINEST,
                                "Failed to resend d_newGroup message, because durability is terminating.\n");
                        } else if((resendCount == 1) || ((resendCount % 5) == 0)){
                            d_printTimedEvent(durability, D_LEVEL_WARNING,
                                "Already tried to resend d_newGroup message '%d' times.\n", resendCount);

                            if(resendCount != 1){
                                OS_REPORT(OS_WARNING, D_CONTEXT_DURABILITY, 0,
                                        "Already tried to resend d_newGroup message '%d' times.",
                                        resendCount);
                            }
                        }
                    } else {
                        d_printTimedEvent(durability, D_LEVEL_SEVERE,
                                "Write of d_newGroup message FAILED with result %d.\n", ur);
                        OS_REPORT(OS_ERROR, D_CONTEXT_DURABILITY, 0,
                                "Write of d_newGroup message FAILED with result %d.", ur);
                        d_durabilityTerminate(durability, TRUE);
                        terminate = d_durabilityMustTerminate(durability);
                    }
                }
            }
        }
    }
    return result;
}

c_bool
d_publisherGroupsRequestWrite(
    d_publisher publisher,
    d_groupsRequest message,
    d_networkAddress addressee)
{
    c_bool result;
    u_result ur;
    int resendCount;
    c_bool terminate;
    d_durability durability;
    d_serviceState state;
    d_thread const self = d_threadLookupSelf ();

    OS_UNUSED_ARG(addressee);
    result = FALSE;
    assert(d_objectIsValid(d_object(publisher), D_PUBLISHER) == TRUE);

    if(publisher){
        if(publisher->enabled == TRUE){
            durability = d_adminGetDurability(publisher->admin);
            state = d_durabilityGetState(durability);
            if ((state != D_STATE_TERMINATING) && (state != D_STATE_TERMINATED)) {
                /* The service is not about to terminate, so it is OK to
                 * publish groupsRequest messages.
                 */
                terminate = FALSE;
                resendCount = 0;
                d_publisherInitMessage(publisher, d_message(message));
                d_message(message)->sequenceNumber = publisher->groupsRequestNumber++;

                while((!result) && (!terminate)){
                    d_threadAwake(self);
                    ur = u_writerWrite(publisher->groupsRequestWriter,
                                       d_publisherGroupsRequestWriterCopy,
                                       message, os_timeWGet(), U_INSTANCEHANDLE_NIL);

                    if(ur == U_RESULT_OK){
                        result = TRUE;
                    } else if(ur == U_RESULT_TIMEOUT) {
                        terminate = d_durabilityMustTerminate(durability);
                        resendCount++;

                        if(terminate){
                            d_printTimedEvent(durability, D_LEVEL_FINEST,
                                "Failed to resend d_groupsRequest message, because durability is terminating.\n");
                        } else if((resendCount == 1) || ((resendCount % 5) == 0)){
                            d_printTimedEvent(durability, D_LEVEL_WARNING,
                                "Already tried to resend d_groupsRequest message '%d' times.\n", resendCount);

                            if(resendCount != 1){
                                OS_REPORT(OS_WARNING, D_CONTEXT_DURABILITY, 0,
                                        "Already tried to resend d_groupsRequest message '%d' times.",
                                        resendCount);
                            }
                        }
                    } else {
                        d_printTimedEvent(durability, D_LEVEL_SEVERE,
                                "Write of d_groupsRequest message FAILED with result %d.\n", ur);
                        OS_REPORT(OS_ERROR, D_CONTEXT_DURABILITY, 0,
                                "Write of d_groupsRequest message FAILED with result %d.", ur);
                        d_durabilityTerminate(durability, TRUE);
                        terminate = d_durabilityMustTerminate(durability);
                    }
                }
            }
        }
    }
    return result;
}


c_bool
d_publisherSampleRequestWrite(
    d_publisher publisher,
    d_sampleRequest message,
    d_networkAddress addressee)
{
    c_bool result;
    u_result ur;
    int resendCount;
    c_bool terminate;
    d_durability durability;
    d_serviceState state;
    d_thread const self = d_threadLookupSelf ();

    OS_UNUSED_ARG(addressee);
    result = FALSE;
    assert(d_objectIsValid(d_object(publisher), D_PUBLISHER) == TRUE);

    if(publisher){
        if(publisher->enabled == TRUE){
            durability = d_adminGetDurability(publisher->admin);
            state = d_durabilityGetState(durability);
            if ((state != D_STATE_TERMINATING) && (state != D_STATE_TERMINATED)) {
                /* The service is not about to terminate, so it is OK to
                 * publish sampleRequest messages.
                 */
                terminate = FALSE;
                resendCount = 0;
                d_publisherInitMessage(publisher, d_message(message));
                d_message(message)->sequenceNumber = publisher->sampleRequestNumber++;

                while((!result) && (!terminate)){
                    d_threadAwake(self);
                    ur = u_writerWrite(publisher->sampleRequestWriter,
                                       d_publisherSampleRequestWriterCopy,
                                       message, os_timeWGet(), U_INSTANCEHANDLE_NIL);

                    if(ur == U_RESULT_OK){
                        result = TRUE;
                    } else if(ur == U_RESULT_TIMEOUT) {
                        terminate = d_durabilityMustTerminate(durability);
                        resendCount++;

                        if(terminate){
                            d_printTimedEvent(durability, D_LEVEL_FINEST,
                                "Failed to resend d_sampleRequest message, because durability is terminating.\n");
                        } else if((resendCount == 1) || ((resendCount % 5) == 0)){
                            d_printTimedEvent(durability, D_LEVEL_WARNING,
                                "Already tried to resend d_sampleRequest message '%d' times.\n", resendCount);

                            if(resendCount != 1){
                                OS_REPORT(OS_WARNING, D_CONTEXT_DURABILITY, 0,
                                        "Already tried to resend d_sampleRequest message '%d' times.",
                                        resendCount);
                            }
                        }
                    } else {
                        d_printTimedEvent(durability, D_LEVEL_SEVERE,
                                "Write of d_sampleRequest message FAILED with result %d.\n", ur);
                        OS_REPORT(OS_ERROR, D_CONTEXT_DURABILITY, 0,
                                "Write of d_sampleRequest message FAILED with result %d.", ur);
                        d_durabilityTerminate(durability, TRUE);
                        terminate = d_durabilityMustTerminate(durability);
                    }
                }
            }
        }
    }
    return result;
}

c_bool
d_publisherSampleChainWrite(
    d_publisher publisher,
    d_sampleChain message,
    d_networkAddress addressee)
{
    c_bool result;
    u_result ur;
    int resendCount;
    c_bool terminate;
    d_durability durability;
    d_serviceState state;
    d_thread const self = d_threadLookupSelf ();

    OS_UNUSED_ARG(addressee);

    assert(d_publisherIsValid(publisher));

    result = FALSE;
    if (publisher) {
        if (publisher->enabled == TRUE) {
            durability = d_adminGetDurability(publisher->admin);
            state = d_durabilityGetState(durability);
            if ((state != D_STATE_TERMINATING) && (state != D_STATE_TERMINATED)) {
                /* The service is not about to terminate, so it is OK to
                 * publish sampleChain messages.
                 */
                terminate = FALSE;
                resendCount = 0;
                d_publisherInitMessage(publisher, d_message(message));
                d_message(message)->sequenceNumber = publisher->sampleChainNumber++;

                while((!result) && (!terminate)){
                    d_threadAwake(self);
                    ur = u_writerWrite(publisher->sampleChainWriter,
                                       d_publisherSampleChainWriterCopy,
                                       message, os_timeWGet(), U_INSTANCEHANDLE_NIL);

                    if (ur == U_RESULT_OK) {
                        result = TRUE;
                    } else {
                        terminate = d_durabilityMustTerminate(durability);
                        /* Print a message in the durability log every 100th attempt */
                        if ((resendCount % 100) == 0) {
                            d_printTimedEvent(durability, D_LEVEL_FINEST,
                                "Failed to send d_sampleChain message (result: %s, terminate: %d, resendCount: %d).\n",
                                 u_resultImage(ur), terminate, resendCount);
                            /* To prevent pollution of the ospl-info log a single entry is logged,
                             * even if multiple retries occur. You can always inspect the durability
                             * log to get an indication how long the retry lasts.
                             */
                            if (resendCount == 0) {
                                OS_REPORT(OS_WARNING, D_CONTEXT_DURABILITY, 0,
                                "Failed to send d_sampleChain message with result '%s'.\n", u_resultImage(ur));
                            }
                        }

                        if (ur == U_RESULT_TIMEOUT) {
                            /* Retry in case of TIMEOUT */
                            resendCount++;
                        } else {
                            /* Something bad happened, terminate */
                            d_printTimedEvent(durability, D_LEVEL_SEVERE,
                                "I am going to terminate (result: %s, terminate: %d, resendCount: %d).\n",
                                 u_resultImage(ur), terminate, resendCount);
                            d_durabilityTerminate(durability, TRUE);
                            terminate = d_durabilityMustTerminate(durability);
                        }
                    }
                } /* while */
                if ((!terminate) && (resendCount > 0)) {

                    /* Apparently a message has been successfully published after a
                     * number of retries
                     */
                    d_printTimedEvent(durability, D_LEVEL_FINEST,
                        "Managed to publish d_sampleChain message after %d resends.\n",
                         resendCount);
                }
            }
        }
    }
    return result;
}

c_bool
d_publisherNameSpacesRequestWrite(
    d_publisher publisher,
    d_nameSpacesRequest message,
    d_networkAddress addressee,
    d_serviceState state)
{
    c_bool result;
    u_result ur;
    int resendCount;
    c_bool terminate;
    d_durability durability;
    d_serviceState myState;
    d_thread const self = d_threadLookupSelf ();

    OS_UNUSED_ARG(addressee);
    result = FALSE;
    assert(d_objectIsValid(d_object(publisher), D_PUBLISHER) == TRUE);

    if(publisher){
        if(publisher->enabled == TRUE){
            durability = d_adminGetDurability(publisher->admin);
            myState = d_durabilityGetState(durability);
            if ((myState != D_STATE_TERMINATING) && (myState != D_STATE_TERMINATED)) {
                /* The service is not about to terminate, so it is OK to
                 * publish nameSpacesRequest messages.
                 */
                terminate = FALSE;
                resendCount = 0;
                d_publisherInitMessage(publisher, d_message(message));
                /* Set the sender state. For normal nameSpacesRequests this is
                 * usually the state of the current durability service. For
                 * namespacesRequests that have been generated on behalf of a fellow
                 * it typically is the fellow's state.
                 * See e.g., d_fellowCheckInitialResponsiveness() */
                d_message(message)->senderState = state;
                d_message(message)->sequenceNumber = publisher->nameSpacesRequestNumber++;

                while((!result) && (!terminate)){
                    d_threadAwake(self);
                    ur = u_writerWrite(publisher->nameSpacesRequestWriter,
                                       d_publisherNameSpacesRequestWriterCopy,
                                       message, os_timeWGet(), U_INSTANCEHANDLE_NIL);

                    if(ur == U_RESULT_OK){
                        result = TRUE;
                    } else if(ur == U_RESULT_TIMEOUT) {
                        terminate = d_durabilityMustTerminate(durability);
                        resendCount++;

                        if (terminate) {
                            d_printTimedEvent(durability, D_LEVEL_FINEST,
                                "Failed to resend d_nameSpaceRequest message, because durability is terminating.\n");
                        } else if ((resendCount == 1) || ((resendCount % 5) == 0)) {
                            d_printTimedEvent(durability, D_LEVEL_WARNING,
                                "Already tried to resend d_nameSpaceRequest message '%d' times.\n", resendCount);

                            if(resendCount != 1){
                                OS_REPORT(OS_WARNING, D_CONTEXT_DURABILITY, 0,
                                        "Already tried to resend d_nameSpaceRequest message '%d' times",
                                        resendCount);
                            }
                        }
                    } else {
                        d_printTimedEvent(durability, D_LEVEL_SEVERE,
                                "Write of d_nameSpaceRequest message FAILED with result %d.\n", ur);
                        OS_REPORT(OS_ERROR, D_CONTEXT_DURABILITY, 0,
                                "Write of d_nameSpaceRequest message FAILED with result %d.", ur);
                        d_durabilityTerminate(durability, TRUE);
                        terminate = d_durabilityMustTerminate(durability);
                    }
                }
            }
        }
    }
    return result;
}

c_bool
d_publisherNameSpacesWrite(
    d_publisher publisher,
    d_nameSpaces message,
    d_networkAddress addressee)
{
    c_bool result;
    u_result ur;
    int resendCount;
    c_bool terminate;
    d_durability durability;
    d_serviceState state;
    d_thread const self = d_threadLookupSelf ();

    OS_UNUSED_ARG(addressee);
    result = FALSE;
    assert(d_objectIsValid(d_object(publisher), D_PUBLISHER) == TRUE);

    if(publisher){
        if(publisher->enabled == TRUE){
            durability = d_adminGetDurability(publisher->admin);
            state = d_durabilityGetState(durability);
            if ((state != D_STATE_TERMINATING) && (state != D_STATE_TERMINATED)) {
                /* The service is not about to terminate, so it is OK to
                 * publish nameSpaces messages.
                 */
                terminate = FALSE;
                resendCount = 0;
                d_publisherInitMessage(publisher, d_message(message));
                d_message(message)->sequenceNumber = publisher->nameSpacesNumber++;

                while((!result) && (!terminate)){
                    d_threadAwake(self);
                     ur = u_writerWrite(publisher->nameSpacesWriter,
                                        d_publisherNameSpacesWriterCopy,
                                       message, os_timeWGet(), U_INSTANCEHANDLE_NIL);

                    if(ur == U_RESULT_OK){
                        result = TRUE;
                    } else if(ur == U_RESULT_TIMEOUT) {
                        terminate = d_durabilityMustTerminate(durability);
                        resendCount++;

                        if(terminate){
                            d_printTimedEvent(durability, D_LEVEL_FINEST,
                                "Failed to resend d_nameSpaces message, because durability is terminating.\n");
                        } else if((resendCount == 1) || ((resendCount % 5) == 0)){
                            d_printTimedEvent(durability, D_LEVEL_WARNING,
                                "Already tried to resend d_nameSpaces message '%d' times.\n", resendCount);

                            if(resendCount != 1){
                                OS_REPORT(OS_WARNING, D_CONTEXT_DURABILITY, 0,
                                        "Already tried to resend d_nameSpaces message '%d' times.",
                                        resendCount);
                            }
                        }
                    } else {
                        d_printTimedEvent(durability, D_LEVEL_SEVERE,
                                "Write of d_nameSpaces message FAILED with result %d.\n", ur);
                        OS_REPORT(OS_ERROR, D_CONTEXT_DURABILITY, 0,
                                "Write of d_nameSpaces message FAILED with result %d.", ur);
                        d_durabilityTerminate(durability, TRUE);
                        terminate = d_durabilityMustTerminate(durability);
                    }
                }
            }
        }
    }
    return result;
}

c_bool
d_publisherDeleteDataWrite(
    d_publisher publisher,
    d_deleteData message,
    d_networkAddress addressee)
{
    c_bool result;
    u_result ur;
    int resendCount;
    c_bool terminate;
    d_durability durability;
    d_serviceState state;
    d_thread const self = d_threadLookupSelf ();

    OS_UNUSED_ARG(addressee);
    result = FALSE;
    assert(d_objectIsValid(d_object(publisher), D_PUBLISHER) == TRUE);

    if(publisher){
        if(publisher->enabled == TRUE){
            durability = d_adminGetDurability(publisher->admin);
            state = d_durabilityGetState(durability);
            if ((state != D_STATE_TERMINATING) && (state != D_STATE_TERMINATED)) {
                /* The service is not about to terminate, so it is OK to
                 * publish deleteData messages.
                 */
                terminate = FALSE;
                resendCount = 0;
                d_publisherInitMessage(publisher, d_message(message));
                d_message(message)->sequenceNumber = publisher->deleteDataNumber++;

                while((!result) && (!terminate)){
                    d_threadAwake(self);
                    ur = u_writerWrite(publisher->deleteDataWriter,
                                       d_publisherDeleteDataWriterCopy,
                                       message, os_timeWGet(), U_INSTANCEHANDLE_NIL);

                    if(ur == U_RESULT_OK){
                        result = TRUE;
                    } else if(ur == U_RESULT_TIMEOUT) {
                        terminate = d_durabilityMustTerminate(durability);
                        resendCount++;

                        if (terminate) {
                            d_printTimedEvent(durability, D_LEVEL_FINEST,
                                "Failed to resend d_deleteData message, because durability is terminating.\n");
                        } else if((resendCount == 1) || ((resendCount % 5) == 0)){
                            d_printTimedEvent(durability, D_LEVEL_WARNING,
                                "Already tried to resend d_deleteData message '%d' times.\n", resendCount);

                            if(resendCount != 1){
                                OS_REPORT(OS_WARNING, D_CONTEXT_DURABILITY, 0,
                                   "Already tried to resend d_deleteData message '%d' times.",resendCount);
                            }
                        }
                    } else {
                        d_printTimedEvent(durability, D_LEVEL_SEVERE,
                                "Write of d_deleteData message FAILED with result %d.\n", ur);
                        OS_REPORT(OS_ERROR, D_CONTEXT_DURABILITY, 0,
                                "Write of d_deleteData message FAILED with result %d.", ur);
                        d_durabilityTerminate(durability, TRUE);
                        terminate = d_durabilityMustTerminate(durability);
                    }
                }
            }
        }
    }
    return result;
}


c_bool
d_publisherCapabilityWrite(
    d_publisher publisher,
    d_capability message,
    d_networkAddress addressee)
{
    c_bool result = FALSE;
    u_result ur;
    int resendCount;
    c_bool terminate;
    d_durability durability;
    d_serviceState state;
    d_thread const self = d_threadLookupSelf ();

    OS_UNUSED_ARG(addressee);
    assert(d_publisherIsValid(publisher));

    if (publisher) {
        if (publisher->enabled == TRUE) {
            durability = d_adminGetDurability(publisher->admin);
            state = d_durabilityGetState(durability);
            if ((state != D_STATE_TERMINATING) && (state != D_STATE_TERMINATED)) {
                /* The service is not about to terminate, so it is OK to
                 * publish capability messages.
                 */
                terminate = FALSE;
                resendCount = 0;
                d_publisherInitMessage(publisher, d_message(message));
                d_message(message)->sequenceNumber = publisher->capabilityNumber++;
                while ((!result) && (!terminate)) {
                    d_threadAwake(self);
                    ur = u_writerWrite(publisher->capabilityWriter,
                                       d_publisherCapabilityWriterCopy,
                                       message, os_timeWGet(), U_INSTANCEHANDLE_NIL);
                    if (ur == U_RESULT_OK) {
                        result = TRUE;
                    } else if (ur == U_RESULT_TIMEOUT) {
                        terminate = d_durabilityMustTerminate(durability);
                        resendCount++;

                        if (terminate) {
                            d_printTimedEvent(durability, D_LEVEL_FINEST,
                                "Failed to resend capability message, because durability is terminating.\n");
                        } else if ((resendCount == 1) || ((resendCount % 5) == 0)) {
                            d_printTimedEvent(durability, D_LEVEL_WARNING,
                                "Already tried to resend capability message '%d' times.\n", resendCount);

                            if (resendCount != 1) {
                                OS_REPORT(OS_WARNING, D_CONTEXT_DURABILITY, 0,
                                        "Already tried to resend capability message '%d' times.",
                                        resendCount);
                            }
                        }
                    } else {
                        d_printTimedEvent(durability, D_LEVEL_SEVERE,
                              "Write of capability message FAILED with result %d.\n", ur);
                        OS_REPORT(OS_ERROR, D_CONTEXT_DURABILITY, 0,
                              "Write of capability message FAILED with result %d.", ur);
                        d_durabilityTerminate(durability, TRUE);
                        terminate = d_durabilityMustTerminate(durability);
                    }
                }
            }
        }
    }
    return result;
}


static v_copyin_result
d_publisherDurabilityStateWriterCopy(
    c_type type,
    const void *from,
    void *to)
{
    d_durabilityState durabilityState = d_durabilityState(from);
    struct _DDS_DurabilityState *msgTo = (struct _DDS_DurabilityState *)to;
    c_base base = c_getBase(type);

    /* Copy the version */
    msgTo->version = durabilityState->version;
    /* Copy my server gid */
    msgTo->serverId = durabilityState->serverId;
    /* Copy the list of requestIds.
     * Note that the list of requestIds may not be modified while in this function!
     */
    {
        c_ulong len = c_iterLength(durabilityState->requestIds);
        c_type subtype0;
        struct _DDS_RequestId_t *dst0;

        subtype0 = c_type(c_metaResolve (c_metaObject(base), "DDS::RequestId_t"));
        dst0 = (struct _DDS_RequestId_t *)c_sequenceNew_s(subtype0, 0, len);
        c_free(subtype0);
        if (!dst0) {
            OS_REPORT (OS_ERROR, "copyIn", 0, "Member 'DurabilityState.requestIds' could not be allocated.");
            goto alloc_requestIds;
        }
        {
            c_iterIter iter = c_iterIterGet(durabilityState->requestIds);
            struct _DDS_RequestId_t *requestId;
            c_ulong i = 0;

            while ((requestId = c_iterNext(&iter)) != NULL) {
                dst0[i].clientId = requestId->clientId;
                dst0[i].requestId = requestId->requestId;
                i++;
            }
        }
        msgTo->requestIds = (c_sequence)dst0;
    }
    /* Copy the dataState.
     * Note that the list of dataState may not be modified while in this function!
     */
    {
        c_ulong len = c_iterLength(durabilityState->dataState);
        c_type subtype0;
        struct _DDS_PartitionTopicState_t *dst0;

        subtype0 = c_type(c_metaResolve (c_metaObject(base), "DDS::PartitionTopicState_t"));
        dst0 = (struct _DDS_PartitionTopicState_t *)c_sequenceNew_s(subtype0, 0, len);
        c_free(subtype0);
        if (!dst0) {
            OS_REPORT (OS_ERROR, "copyIn", 0, "Member 'DurabilityState.dataState' could not be allocated.");
            goto alloc_dataState;
        }
        {
            c_iterIter iter = c_iterIterGet(durabilityState->dataState);
            d_partitionTopicState partitionTopicState;
            c_ulong i = 0;

             while ((partitionTopicState = d_partitionTopicState(c_iterNext(&iter))) != NULL) {
                dst0[i].topic = c_stringNew_s(base, partitionTopicState->topic);
                if (!dst0[i].topic) {
                    OS_REPORT (OS_ERROR, "copyIn", 0, "Member 'DurabilityState.dataState.topic' could not be allocated.");
                    goto alloc_partitionTopicStateTopic;
                }
                dst0[i].partition = c_stringNew_s(base, partitionTopicState->partition);
                if (!dst0[i].partition) {
                    OS_REPORT (OS_ERROR, "copyIn", 0, "Member 'DurabilityState.dataState.partition' could not be allocated.");
                    goto alloc_partitionTopicStatePartition;
                }
                dst0[i].completeness = partitionTopicState->completeness;

                /* Copy the extensions */
                {
                    c_type type1;
                    c_type subtype1;
                    c_ulong length1;
                    struct _DDS_NameValue_t *dst1;

                    subtype1 = c_type(c_metaResolve (c_metaObject(base), "DDS::NameValue_t"));
                    type1 = c_metaSequenceTypeNew_s(c_metaObject(base),"C_SEQUENCE<DDS::NameValue_t>",subtype1, 0);
                    c_free(subtype1);
                    if (!type1) {
                        OS_REPORT (OS_ERROR, "copyIn", 0, "Type for 'C_SEQUENCE<DDS::NameValue_t>' could not be allocated.");
                        goto alloc_partitionTopicStateExtensionsType;
                    }

                    /* TODO: currently always a zero-length sequence is generated. */
                    length1 = 0;
                    dst1 = (struct _DDS_NameValue_t *)c_newSequence_s(c_collectionType(type1), length1);

                    if (!dst1) {
                        OS_REPORT (OS_ERROR, "copyIn", 0, "Member 'DurabilityState.dataState.extensions' could not be allocated.");
                        goto alloc_partitionTopicStateExtensions;
                    }
                    dst0[i].extensions = (c_sequence)dst1;
                }
                i++;
            }
        }
        msgTo->dataState = (c_sequence)dst0;
    }
    /* Copy the extensions */
    {
        static c_type type0 = NULL;
        c_type subtype0;
        c_ulong length0;
        struct _DDS_NameValue_t *dst0;

        subtype0 = c_type(c_metaResolve (c_metaObject(base), "DDS::NameValue_t"));
        type0 = c_metaSequenceTypeNew_s(c_metaObject(base),"C_SEQUENCE<DDS::NameValue_t>",subtype0,0);
        c_free(subtype0);
        if (!type0) {
            OS_REPORT (OS_ERROR, "copyIn", 0, "Type for 'C_SEQUENCE<DDS::NameValue_t>' could not be allocated.");
            goto alloc_extensionsType;
        }
        /* TODO: currently always a zero-length sequence is generated.
         * That means that effectivly always a zero-size list of extensions is used
         * in the historicalData.
         */
        length0 = 0;
        dst0 = (struct _DDS_NameValue_t *)c_newSequence_s(c_collectionType(type0), length0);
        if (!dst0) {
            OS_REPORT (OS_ERROR, "copyIn", 0,"Member 'DurabilityState.extensions' could not be allocated.");
            goto alloc_extensions;
        }
        msgTo->extensions = (c_sequence)dst0;
    }
    return V_COPYIN_RESULT_OK;

alloc_extensions:
alloc_extensionsType:
alloc_partitionTopicStateExtensions:
alloc_partitionTopicStateExtensionsType:
alloc_partitionTopicStatePartition:
alloc_partitionTopicStateTopic:
alloc_dataState:
alloc_requestIds:
    /* To check if c_free() iteratively frees all subtypes */
    c_free(msgTo);

    return V_COPYIN_RESULT_OUT_OF_MEMORY;
}


c_bool
d_publisherDurabilityStateWrite(
    d_publisher publisher,
    d_durabilityState durabilityState)
{
    c_bool result = FALSE;
    u_result ur;
    int resendCount;
    c_bool terminate;
    d_durability durability;
    d_serviceState state;
    d_thread self = d_threadLookupSelf ();

    assert(d_publisherIsValid(publisher));
    assert(d_durabilityStateIsValid(durabilityState));

    durability = d_adminGetDurability(publisher->admin);
    if (publisher->enabled) {
        state = d_durabilityGetState(durability);
        if ((state != D_STATE_TERMINATING) && (state != D_STATE_TERMINATED)) {
            /* The service is not about to terminate, so it is OK to
             * publish durabilityState messages.
             */
            d_printTimedEvent(durability, D_LEVEL_FINE,
                    "Publish durabilityState (#requestIds: %ld, #states: %ld)\n",
                    c_iterLength(durabilityState->requestIds), c_iterLength(durabilityState->dataState));
            terminate = FALSE;
            resendCount = 0;
            while ((!result) && (!terminate)) {
                d_threadAwake(self);
                ur = u_writerWrite(publisher->durabilityStateWriter,
                                   d_publisherDurabilityStateWriterCopy,
                                   durabilityState, os_timeWGet(), U_INSTANCEHANDLE_NIL);
                if (ur == U_RESULT_OK) {
                    result = TRUE;
                } else if (ur == U_RESULT_TIMEOUT) {
                    terminate = d_durabilityMustTerminate(durability);
                    resendCount++;
                    if (terminate) {
                        d_printTimedEvent(durability, D_LEVEL_FINEST,
                            "Failed to send durabilityState message because durability is terminating.\n");
                    } else if (resendCount == 1) {
                       /* Only a single log line in case of resends to prevent log pollution */
                        d_printTimedEvent(durability, D_LEVEL_WARNING,
                            "Failed to publish durabilityState message with result %s, try to resend.\n", u_resultImage(ur));
                        OS_REPORT(OS_WARNING, D_CONTEXT_DURABILITY, 0,
                            "Failed to publish durabilityState message with result %s, try to resend.", u_resultImage(ur));
                    }
                } else {
                    d_printTimedEvent(durability, D_LEVEL_SEVERE,
                            "Publication of durabilityState message FAILED with result %s, I am going to terminate\n", u_resultImage(ur));
                    OS_REPORT(OS_ERROR, D_CONTEXT_DURABILITY, 0,
                            "Publication of durabilityState message FAILED with result %s, I am going to terminate", u_resultImage(ur));
                    d_durabilityTerminate(durability, TRUE);
                    terminate = d_durabilityMustTerminate(durability);
                }
            } /* while */
            if ((result) && (resendCount > 0) && (!terminate)) {
                    d_printTimedEvent(durability, D_LEVEL_FINEST,
                            "Publication of durabilityState message succeeded after %d resends\n", resendCount);
            }
        }
    }
    return result;
}


v_copyin_result
d_publisherSampleChainWriterCopy(
    c_type type,
    const void *data,
    void *to)
{
    d_sampleChain msgFrom = d_sampleChain(data);
    d_sampleChain msgTo   = d_sampleChain(to);
    c_base           base = c_getBase(type);
    c_type           subtype;
    c_long           valueSize;

    d_publisherMessageWriterCopy(&msgFrom->parentMsg, &msgTo->parentMsg);

    msgTo->partition       = c_stringNew_s(base, msgFrom->partition);
    if (!msgTo->partition) {
        OS_REPORT (OS_ERROR, "copyIn", 0, "Member 'partition' could not be allocated.");
        goto err_allocPartition;
    }
    msgTo->topic           = c_stringNew_s(base, msgFrom->topic);
    if (!msgTo->topic) {
        OS_REPORT (OS_ERROR, "copyIn", 0, "Member 'topic' could not be allocated.");
        goto err_allocTopic;
    }
    msgTo->durabilityKind  = msgFrom->durabilityKind;
    msgTo->msgBody._d      = msgFrom->msgBody._d;
    msgTo->addresseesCount = msgFrom->addresseesCount;

    assert(msgTo->addresseesCount > 0);
    subtype = c_resolve(base, "durabilityModule2::d_networkAddress_s");
    msgTo->addressees = c_arrayNew_s(subtype, msgTo->addresseesCount);
    c_free(subtype);
    if (!msgTo->addressees) {
        OS_REPORT (OS_ERROR, "copyIn", 0, "Member 'addressees' could not be allocated.");
        goto err_allocAddressees;
    }
    memcpy(msgTo->addressees, msgFrom->addressees, msgTo->addresseesCount*C_SIZEOF(d_networkAddress));

    msgTo->source.systemId    = msgFrom->source.systemId;
    msgTo->source.localId     = msgFrom->source.localId;
    msgTo->source.lifecycleId = msgFrom->source.lifecycleId;

    switch(msgTo->msgBody._d) {
    case BEAD:
        valueSize = msgFrom->msgBody._u.bead.size;
        msgTo->msgBody._u.bead.size  = valueSize;
        assert(valueSize >= 0);
        msgTo->msgBody._u.bead.value = c_arrayNew_s(c_octet_t(base), (c_ulong)valueSize);
        if (!msgTo->msgBody._u.bead.value) {
            OS_REPORT (OS_ERROR, "copyIn", 0, "Member 'msgBody._u.bead.value' could not be allocated.");
            goto err_allocBeadValue;
        }
        memcpy(msgTo->msgBody._u.bead.value, msgFrom->msgBody._u.bead.value, (c_ulong)valueSize);
    break;
    case LINK:
        msgTo->msgBody._u.link.nrSamples      = msgFrom->msgBody._u.link.nrSamples;
        msgTo->msgBody._u.link.completeness   = msgFrom->msgBody._u.link.completeness;
    break;
    default:
        OS_REPORT(OS_ERROR, "d_publisherSampleChainWriterCopy", 0,
                    "Illegal message body discriminant value (%d) detected.",
                    msgTo->msgBody._d);
        assert(FALSE);
    }

    return V_COPYIN_RESULT_OK;

err_allocBeadValue:
err_allocAddressees:
err_allocTopic:
err_allocPartition:
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
}

v_copyin_result
d_publisherMessageWriterCopy(
    d_message msgFrom,
    d_message msgTo)
{
    msgTo->senderAddress.systemId = msgFrom->senderAddress.systemId;
    msgTo->senderAddress.localId = msgFrom->senderAddress.localId;
    msgTo->senderAddress.lifecycleId = msgFrom->senderAddress.lifecycleId;

    msgTo->addressee.systemId = msgFrom->addressee.systemId;
    msgTo->addressee.localId = msgFrom->addressee.localId;
    msgTo->addressee.lifecycleId = msgFrom->addressee.lifecycleId;

    msgTo->senderState = msgFrom->senderState;
    msgTo->productionTimestamp = msgFrom->productionTimestamp;
    msgTo->sequenceNumber = msgFrom->sequenceNumber;

    return V_COPYIN_RESULT_OK;
}

v_copyin_result
d_publisherStatusWriterCopy(
    c_type type,
    const void *data,
    void *to)
{
    OS_UNUSED_ARG(type);
    assert(type);
    return d_publisherMessageWriterCopy(&d_status(data)->parentMsg, &d_status(to)->parentMsg);
}

v_copyin_result
d_publisherNewGroupWriterCopy(
    c_type type,
    const void *data,
    void *to)
{
    v_copyin_result result;
    d_newGroup msgFrom    = (d_newGroup)data;
    d_newGroup msgTo      = (d_newGroup)to;
    c_base base           = c_getBase(type);

    result = d_publisherMessageWriterCopy(&msgFrom->parentMsg, &msgTo->parentMsg);

    if(msgFrom->partition){
        msgTo->partition = c_stringNew_s(base, msgFrom->partition);
        if (!msgTo->partition) {
            OS_REPORT (OS_ERROR, "copyIn", 0, "Member 'partition' could not be allocated.");
            goto err_allocPartition;
        }
    } else {
        msgTo->partition = NULL;
    }

    if(msgFrom->topic){
        msgTo->topic = c_stringNew_s(base, msgFrom->topic);
        if (!msgTo->topic) {
            OS_REPORT (OS_ERROR, "copyIn", 0, "Member 'topic' could not be allocated.");
            goto err_allocTopic;
        }
    } else {
        msgTo->topic = NULL;
    }
    msgTo->completeness   = msgFrom->completeness;
    msgTo->durabilityKind = msgFrom->durabilityKind;
    msgTo->quality        = msgFrom->quality;
    msgTo->alignerCount   = msgFrom->alignerCount;

    return result;

err_allocTopic:
err_allocPartition:
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
}

v_copyin_result
d_publisherGroupsRequestWriterCopy(
    c_type type,
    const void *data,
    void *to)
{
    v_copyin_result result  = V_COPYIN_RESULT_OK;
    d_groupsRequest msgFrom = (d_groupsRequest)data;
    d_groupsRequest msgTo   = (d_groupsRequest)to;
    c_base base             = c_getBase(type);

    result = d_publisherMessageWriterCopy(&d_groupsRequest(data)->parentMsg, &d_groupsRequest(to)->parentMsg);

    if(msgFrom->partition){
        msgTo->partition = c_stringNew_s(base, msgFrom->partition);
        if (!msgTo->partition) {
            OS_REPORT (OS_ERROR, "copyIn", 0, "Member 'partition' could not be allocated.");
            goto err_allocPartition;
        }
    } else {
        msgTo->partition = NULL;
    }

    if(msgFrom->topic){
        msgTo->topic = c_stringNew_s(base, msgFrom->topic);
        if (!msgTo->topic) {
            OS_REPORT (OS_ERROR, "copyIn", 0, "Member 'topic' could not be allocated.");
            goto err_allocTopic;
        }
    } else {
        msgTo->topic = NULL;
    }

    return result;

err_allocTopic:
err_allocPartition:
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
}

v_copyin_result
d_publisherSampleRequestWriterCopy(
    c_type type,
    const void *data,
    void *to)
{
    v_copyin_result result;
    c_ulong i;
    d_sampleRequest msgFrom = d_sampleRequest(data);
    d_sampleRequest msgTo   = d_sampleRequest(to);
    c_base base             = c_getBase(type);

    result = d_publisherMessageWriterCopy(&msgFrom->parentMsg, &msgTo->parentMsg);

    msgTo->partition = c_stringNew_s(base, msgFrom->partition);
    if (!msgTo->partition) {
        OS_REPORT (OS_ERROR, "copyIn", 0, "Member 'partition' could not be allocated.");
        goto err_allocPartition;
    }

    msgTo->topic = c_stringNew_s(base, msgFrom->topic);
    if (!msgTo->topic) {
        OS_REPORT (OS_ERROR, "copyIn", 0, "Member 'topic' could not be allocated.");
        goto err_allocTopic;
    }

    msgTo->durabilityKind = msgFrom->durabilityKind;
    msgTo->requestTime    = msgFrom->requestTime;
    msgTo->withTimeRange  = msgFrom->withTimeRange;
    msgTo->beginTime      = msgFrom->beginTime;
    msgTo->endTime        = msgFrom->endTime;

    msgTo->source.systemId    = msgFrom->source.systemId;
    msgTo->source.localId     = msgFrom->source.localId;
    msgTo->source.lifecycleId = msgFrom->source.lifecycleId;

    if(msgFrom->filter){
        msgTo->filter = c_stringNew_s(base, msgFrom->filter);
        if (!msgTo->filter) {
            OS_REPORT (OS_ERROR, "copyIn", 0, "Member 'filter' could not be allocated.");
            goto err_allocFilter;
        }
    } else {
        msgTo->filter = NULL;
    }
    if(msgFrom->filterParams){
        msgTo->filterParamsCount = msgFrom->filterParamsCount;
        /* Create array for filters and equalityCheck hash string (+1) */
        msgTo->filterParams = c_arrayNew_s(c_string_t(base), msgFrom->filterParamsCount + 1);
        if (!msgTo->filterParams) {
            OS_REPORT (OS_ERROR, "copyIn", 0, "Member 'filterParams' could not be allocated.");
            goto err_allocFilterParams;
        }

        for(i=0; i<msgFrom->filterParamsCount; i++){
            msgTo->filterParams[i] = c_stringNew_s(base, msgFrom->filterParams[i]);
            if (!msgTo->filterParams[i]) {
                OS_REPORT (OS_ERROR, "copyIn", 0, "Member 'filterParams[%d]' could not be allocated.", i);
                goto err_allocFilterParam;
            }
        }
        /* Add equalityCheck hash string as last entry to filterParams sequence */
        msgTo->filterParams[i] = c_stringNew_s(base, msgFrom->filterParams[i]);
        if (!msgTo->filterParams[i]) {
            OS_REPORT (OS_ERROR, "copyIn", 0, "Member 'filterParams[%d]' could not be allocated.", i);
            goto err_allocFilterParam;
        }
    }
    msgTo->maxSamples            = msgFrom->maxSamples;
    msgTo->maxInstances          = msgFrom->maxInstances;
    msgTo->maxSamplesPerInstance = msgFrom->maxSamplesPerInstance;

    return result;

err_allocFilterParam:
err_allocFilterParams:
err_allocFilter:
err_allocTopic:
err_allocPartition:
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
}

v_copyin_result
d_publisherNameSpacesWriterCopy(
    c_type type,
    const void *data,
    void *to)
{
    v_copyin_result result;

    d_nameSpaces msgFrom = d_nameSpaces(data);
    d_nameSpaces msgTo   = d_nameSpaces(to);
    c_base base          = c_getBase(type);
    c_ulong count        = 0;
    OS_UNUSED_ARG(type);

    result = d_publisherMessageWriterCopy(&msgFrom->parentMsg, &msgTo->parentMsg);

    msgTo->name = c_stringNew_s(base, msgFrom->name);
    if (!msgTo->name) {
        OS_REPORT (OS_ERROR, "copyIn", 0, "Member 'name' could not be allocated.");
        goto err_allocName;
    }

    msgTo->durabilityKind             = msgFrom->durabilityKind;
    msgTo->alignmentKind              = msgFrom->alignmentKind;
    msgTo->aligner                    = msgFrom->aligner;
    msgTo->total                      = msgFrom->total;
    msgTo->initialQuality             = msgFrom->initialQuality;
    msgTo->master.systemId            = msgFrom->master.systemId;
    msgTo->master.localId             = msgFrom->master.localId;
    msgTo->master.lifecycleId         = msgFrom->master.lifecycleId;
    msgTo->isComplete                 = msgFrom->isComplete;
    msgTo->masterConfirmed            = msgFrom->masterConfirmed;
    msgTo->state.value                = msgFrom->state.value;

    if(msgFrom->partitions) {
        msgTo->partitions = c_stringNew_s(base, msgFrom->partitions);
        if (!msgTo->partitions) {
            OS_REPORT (OS_ERROR, "copyIn", 0, "Member 'partitions' could not be allocated.");
            goto err_allocPartitions;
        }
    } else {
        msgTo->partitions = NULL;
    }
    if(msgFrom->state.role) {
        msgTo->state.role    = c_stringNew_s(base, msgFrom->state.role);
        if (!msgTo->state.role) {
            OS_REPORT (OS_ERROR, "copyIn", 0, "Member 'state.role' could not be allocated.");
            goto err_allocStateRole;
        }
    } else {
        msgTo->state.role    = NULL;
    }
    msgTo->state.value       = msgFrom->state.value;
    msgTo->mergedStatesCount = msgFrom->mergedStatesCount;

    count = msgTo->mergedStatesCount;
    if (msgFrom->aligner == TRUE) {
        count++;
    }

    if (count > 0) {
        c_type mergeStateType = c_resolve(base, "durabilityModule2::d_mergeState_s");
        msgTo->mergedStates = c_arrayNew_s(mergeStateType, count);
        c_free(mergeStateType);
        if (!msgTo->mergedStates) {
            OS_REPORT (OS_ERROR, "copyIn", 0, "Member 'mergedStates' could not be allocated.");
            goto err_allocMergedStates;
        }
    } else {
        msgTo->mergedStates = NULL;
    }

    {
        const struct d_mergeState_s *from = (const struct d_mergeState_s *) msgFrom->mergedStates;
        struct d_mergeState_s *to = (struct d_mergeState_s *) msgTo->mergedStates;
        c_ulong i;
        for(i=0; i<count; i++){
            to[i].value = from[i].value;
            to[i].role = c_stringNew_s(base, from[i].role);
            if (to[i].role == NULL) {
                OS_REPORT (OS_ERROR, "copyIn", 0, "Member 'mergedStates[%i].role' could not be allocated.", i);
                goto err_allocRole;
            }
        }
    }

    return result;

err_allocRole:
err_allocMergedStates:
err_allocStateRole:
err_allocPartitions:
err_allocName:
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
}

v_copyin_result
d_publisherNameSpacesRequestWriterCopy(
    c_type type,
    const void *data,
    void *to)
{
    OS_UNUSED_ARG(type);
    assert(type);

    return d_publisherMessageWriterCopy(&d_nameSpacesRequest(data)->parentMsg, &d_nameSpacesRequest(to)->parentMsg);
}

v_copyin_result
d_publisherDeleteDataWriterCopy(
    c_type type,
    const void *data,
    void *to)
{
    v_copyin_result result;
    d_deleteData msgFrom = d_deleteData(data);
    d_deleteData msgTo   = d_deleteData(to);
    c_base base          = c_getBase(type);

    result = d_publisherMessageWriterCopy(&msgFrom->parentMsg, &msgTo->parentMsg);

    msgTo->actionTime = msgFrom->actionTime;

    if(msgFrom->partitionExpr) {
        msgTo->partitionExpr = c_stringNew_s(base, msgFrom->partitionExpr);
        if (!msgTo->partitionExpr) {
            OS_REPORT (OS_ERROR, "copyIn", 0, "Member 'partitionExpr' could not be allocated.");
            goto err_allocPartitionExpr;
        }
    } else {
        msgTo->partitionExpr = NULL;
    }
    if(msgFrom->topicExpr) {
        msgTo->topicExpr = c_stringNew_s(base, msgFrom->topicExpr);
        if (!msgTo->topicExpr) {
            OS_REPORT (OS_ERROR, "copyIn", 0, "Member 'topicExpr' could not be allocated.");
            goto err_allocTopicExpr;
        }
    } else {
        msgTo->topicExpr = NULL;
    }
    return result;

err_allocTopicExpr:
err_allocPartitionExpr:
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
}


v_copyin_result
d_publisherCapabilityWriterCopy(
    c_type type,
    const void *data,
    void *to)
{
    v_copyin_result result;
    d_capability msgFrom = d_capability(data);
    d_capability msgTo   = d_capability(to);
    c_base base          = c_getBase(type);
    c_type nv_type;
    struct d_nameValue_s *nvFrom, *nvTo;
    c_ulong i,j;

    result = d_publisherMessageWriterCopy(&msgFrom->parentMsg, &msgTo->parentMsg);

    nv_type = c_resolve (base, "durabilityModule2::d_nameValue_s");
    assert(nv_type);
    msgTo->capabilities = c_sequenceNew_s(nv_type, D_NUMBER_OF_CAPABILITIES, D_NUMBER_OF_CAPABILITIES);
    c_free(nv_type);
    if (!msgTo->capabilities) {
        OS_REPORT (OS_ERROR, "copyIn", 0, "Member 'capabilities' could not be allocated.");
        goto err_allocCapabilities;
    }
    nvFrom = (struct d_nameValue_s*)msgFrom->capabilities;
    nvTo = (struct d_nameValue_s*)msgTo->capabilities;

    for (i=0; i<D_NUMBER_OF_CAPABILITIES; i++) {
        nvTo[i].name = c_stringNew_s(base, nvFrom[i].name);
        if (!nvTo[i].name) {
            OS_REPORT (OS_ERROR, "copyIn", 0, "Member 'name' could not be allocated.");
            goto err_allocName;
        }

        if ((strcmp(nvFrom[i].name, D_CAPABILITY_MASTER_SELECTION) == 0) || (strcmp(nvFrom[i].name, D_CAPABILITY_INCARNATION) == 0)) {
            unsigned dataBE;
            nvTo[i].value = c_sequenceNew_s(c_ulong_t(base), sizeof(c_ulong), sizeof(c_ulong));
            if (!nvTo[i].value) {
                OS_REPORT (OS_ERROR, "copyIn", 0, "Member 'value' could not be allocated.");
                c_free(nvTo[i].name);
                goto err_allocValue;
            }
            dataBE = d_swap4uToBE(((c_ulong*)nvFrom[i].value)[0]);
            memcpy(nvTo[i].value, &dataBE, sizeof(c_ulong));
        } else {
            nvTo[i].value = c_sequenceNew_s(c_octet_t(base), 1, 1);
            if (!nvTo[i].value) {
                OS_REPORT (OS_ERROR, "copyIn", 0, "Member 'value' could not be allocated.");
                c_free(nvTo[i].name);
                goto err_allocValue;
            }
            ((c_octet*)nvTo[i].value)[0] = ((c_octet*)nvFrom[i].value)[0];
        }
    }
    return result;

err_allocValue:
err_allocName:
    for (j=0; j<i; j++) {
        c_free(nvTo[j].name);
        c_free(nvTo[j].value);
    }
    c_free(msgTo->capabilities);
err_allocCapabilities:
    return V_COPYIN_RESULT_OUT_OF_MEMORY;
}


void
d_publisherInitMessage(
    d_publisher publisher,
    d_message message)
{
    os_uint32 seqnum;

    seqnum = d_adminGetNextSeqNum(publisher->admin);
    d_productionTimestampFromSeqnum(&d_message(message)->productionTimestamp, &seqnum);
    d_message(message)->senderState = d_durabilityGetState(d_adminGetDurability(publisher->admin));
}

void
d_publisherEnsureServicesAttached(
    v_public entity,
    c_voidp args)
{
    v_writer writer;
    v_group group;
    d_durability durability;

    writer = v_writer(entity);
    durability = d_durability(args);
    group = writer->groupSet.firstGroup->group;
    d_durabilityWaitForAttachToGroup(durability, group);
}

static void
d__publisherNameSpacesRequestUnregister(
    d_publisher publisher,
    d_networkAddress addressee)
{
    d_nameSpacesRequest request;
    u_instanceHandle handle = U_INSTANCEHANDLE_NIL;
    u_result ur;
    d_durability durability;
    int resendCount = 0;
    c_bool terminate;
    d_thread self = d_threadLookupSelf ();

    assert(publisher);
    assert(d_objectIsValid(d_object(publisher), D_PUBLISHER) == TRUE);

    request = d_nameSpacesRequestNew(publisher->admin);
    d_messageSetAddressee(d_message(request), addressee);
    /* Unregistration is only needed when the writer exists and it has been used before.
     * Therefore we do not care about the output of the lookupInstance as long as the handle
     * is set (or not set) */
    (void) u_writerLookupInstance(publisher->nameSpacesRequestWriter,
        d_publisherNameSpacesRequestWriterCopy, request, &handle);
    d_nameSpacesRequestFree(request);

    if (handle != U_INSTANCEHANDLE_NIL) {
        durability = d_adminGetDurability(publisher->admin);
        do {
            d_threadAwake(self);
            ur = u_writerUnregisterInstance(publisher->nameSpacesRequestWriter, NULL, NULL, os_timeWGet(), handle);
            if (ur == U_RESULT_TIMEOUT) {
                resendCount++;
                terminate = d_durabilityMustTerminate(durability);
                if (terminate) {
                    d_printTimedEvent(durability, D_LEVEL_FINEST,
                        "Failed to send d_nameSpacesRequest unregister message, because durability is terminating.\n");
                } else if ((resendCount == 1) || ((resendCount % 5) == 0)){
                    d_printTimedEvent(durability, D_LEVEL_WARNING,
                        "Already tried to send d_nameSpacesRequest unregister message '%d' times.\n", resendCount);

                    if(resendCount != 1){
                        OS_REPORT(OS_WARNING, D_CONTEXT_DURABILITY, 0,
                                "Already tried to send d_nameSpacesRequest unregister message '%d' times.",
                                resendCount);
                    }
                }
            } else if (ur != U_RESULT_OK) {
                d_printTimedEvent(durability, D_LEVEL_WARNING,
                        "Unregistration of d_nameSpacesRequest message FAILED with %s\n", u_resultImage(ur));
                OS_REPORT(OS_ERROR, D_CONTEXT_DURABILITY, 0,
                        "Unregistration of d_nameSpacesRequest message FAILED with %s", u_resultImage(ur));
            }
            terminate = d_durabilityMustTerminate(durability);
        } while (!terminate && ur == U_RESULT_TIMEOUT);
    }
}

static void
d__publisherStatusUnregister(
    d_publisher publisher,
    d_networkAddress addressee)
{
    d_status status;
    u_instanceHandle handle = U_INSTANCEHANDLE_NIL;
    u_result ur;
    d_durability durability;
    int resendCount = 0;
    c_bool terminate;
    d_thread self = d_threadLookupSelf ();

    assert(publisher);
    assert(d_objectIsValid(d_object(publisher), D_PUBLISHER) == TRUE);

    status = d_statusNew(publisher->admin);
    d_messageSetAddressee(d_message(status), addressee);
    /* Unregistration is only needed when the writer exists and it has been used before.
     * Therefore we do not care about the output of the lookupInstance as long as the handle
     * is set (or not set) */
    (void) u_writerLookupInstance(publisher->statusWriter,
        d_publisherStatusWriterCopy, status, &handle);
    d_statusFree(status);

    if (handle != U_INSTANCEHANDLE_NIL) {
        durability = d_adminGetDurability(publisher->admin);
        do {
            d_threadAwake(self);
            ur = u_writerUnregisterInstance(publisher->statusWriter, NULL, NULL, os_timeWGet(), handle);
            if (ur == U_RESULT_TIMEOUT) {
                resendCount++;
                terminate = d_durabilityMustTerminate(durability);
                if (terminate) {
                    d_printTimedEvent(durability, D_LEVEL_FINEST,
                        "Failed to send d_status unregister message, because durability is terminating.\n");
                } else if ((resendCount == 1) || ((resendCount % 5) == 0)){
                    d_printTimedEvent(durability, D_LEVEL_WARNING,
                        "Already tried to send d_status unregister message '%d' times.\n", resendCount);

                    if(resendCount != 1){
                        OS_REPORT(OS_WARNING, D_CONTEXT_DURABILITY, 0,
                                "Already tried to send d_status unregister message '%d' times.",
                                resendCount);
                    }
                }
            } else if (ur != U_RESULT_OK) {
                d_printTimedEvent(durability, D_LEVEL_WARNING,
                        "Unregistration of d_status message FAILED with %s\n", u_resultImage(ur));
                OS_REPORT(OS_ERROR, D_CONTEXT_DURABILITY, 0,
                        "Unregistration of d_status message FAILED with %s", u_resultImage(ur));
            }
            terminate = d_durabilityMustTerminate(durability);
        } while (!terminate && ur == U_RESULT_TIMEOUT);
    }
}

c_bool
d_publisherUnregisterInstances(
    d_publisher publisher,
    d_networkAddress addressee)
{
    c_bool result = FALSE;

    if (publisher && publisher->enabled == TRUE) {
        d__publisherNameSpacesRequestUnregister(publisher, addressee);
        d__publisherStatusUnregister(publisher, addressee);
        result = TRUE;
    }

    return result;
}
