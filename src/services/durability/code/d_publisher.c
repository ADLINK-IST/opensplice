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

#include "d__publisher.h"
#include "d_publisher.h"
#include "d_durability.h"
#include "d_message.h"
#include "d_misc.h"
#include "d_status.h"
#include "d_statusRequest.h"
#include "d_sampleRequest.h"
#include "d_groupsRequest.h"
#include "d_nameSpaces.h"
#include "d_nameSpacesRequest.h"
#include "d__mergeState.h"
#include "d_deleteData.h"
#include "d_configuration.h"
#include "d_qos.h"
#include "d_sampleChain.h"
#include "d_admin.h"
#include "d_durability.h"
#include "u_publisher.h"
#include "u_participant.h"
#include "u_writer.h"
#include "u_writerQos.h"
#include "v_time.h"
#include "v_writer.h"
#include "v_group.h"
#include "os_heap.h"
#include "os_report.h"

d_publisher
d_publisherNew(
    d_admin admin)
{
    d_publisher     publisher;
    d_durability    durability;
    d_configuration config;
    v_publisherQos  publisherQos;
    v_writerQos     writerQos;

    publisher = NULL;

    if(admin){
        publisher            = d_publisher(os_malloc(C_SIZEOF(d_publisher)));
        d_objectInit(d_object(publisher), D_PUBLISHER, d_publisherDeinit);
        publisher->enabled   = TRUE;
        publisher->admin     = admin;
        durability           = d_adminGetDurability(admin);
        config               = d_durabilityGetConfiguration(durability);
        publisherQos         = d_publisherQosNew(config->partitionName);
        publisher->publisher = u_publisherNew(
                                    u_participant(d_durabilityGetService(durability)),
                                    config->publisherName, publisherQos, TRUE);

        d_publisherQosFree(publisherQos);

        if(publisher->publisher){


            publisher->statusWriter            = NULL;
            publisher->newGroupWriter          = NULL;
            publisher->groupsRequestWriter     = NULL;
            publisher->statusRequestWriter     = NULL;
            publisher->sampleRequestWriter     = NULL;
            publisher->sampleChainWriter       = NULL;
            publisher->nameSpacesWriter        = NULL;
            publisher->nameSpacesRequestWriter = NULL;
            publisher->deleteDataWriter        = NULL;
            writerQos                          = d_writerQosNew(
                                                    V_DURABILITY_VOLATILE,
                                                    V_RELIABILITY_RELIABLE,
                                                    config->heartbeatLatencyBudget,
                                                    config->heartbeatTransportPriority);

            publisher->statusNumber = 0;
            publisher->statusWriter = u_writerNew (publisher->publisher,
                                                   "statusWriter",
                                                   d_adminGetStatusTopic(admin),
                                                   d_publisherStatusWriterCopy,
                                                   writerQos,
                                                   TRUE);
            assert(publisher->statusWriter);

            d_writerQosFree(writerQos);
            writerQos = d_writerQosNew (V_DURABILITY_VOLATILE,
                                        V_RELIABILITY_RELIABLE,
                                        config->latencyBudget,
                                        config->transportPriority);

            u_entityAction(u_entity(publisher->statusWriter), d_publisherEnsureServicesAttached, durability);

            publisher->newGroupNumber = 0;
            publisher->newGroupWriter = u_writerNew (publisher->publisher,
                                                     "newGroupWriter",
                                                     d_adminGetNewGroupTopic(admin),
                                                     d_publisherNewGroupWriterCopy,
                                                     writerQos,
                                                     TRUE);
            assert(publisher->newGroupWriter);
            u_entityAction(u_entity(publisher->newGroupWriter), d_publisherEnsureServicesAttached, durability);


            publisher->groupsRequestNumber = 0;
            publisher->groupsRequestWriter = u_writerNew (publisher->publisher,
                                                          "groupsRequestWriter",
                                                          d_adminGetGroupsRequestTopic(admin),
                                                          d_publisherGroupsRequestWriterCopy,
                                                          writerQos,
                                                          TRUE);
            assert(publisher->groupsRequestWriter);
            u_entityAction(u_entity(publisher->groupsRequestWriter), d_publisherEnsureServicesAttached, durability);

            publisher->statusRequestNumber = 0;
            publisher->statusRequestWriter = u_writerNew (publisher->publisher,
                                                          "statusRequestWriter",
                                                          d_adminGetStatusRequestTopic(admin),
                                                          d_publisherStatusRequestWriterCopy,
                                                          writerQos,
                                                          TRUE);
            assert(publisher->statusRequestWriter);
            u_entityAction(u_entity(publisher->statusRequestWriter), d_publisherEnsureServicesAttached, durability);

            publisher->sampleRequestNumber = 0;
            publisher->sampleRequestWriter = u_writerNew (publisher->publisher,
                                                          "sampleRequestWriter",
                                                          d_adminGetSampleRequestTopic(admin),
                                                          d_publisherSampleRequestWriterCopy,
                                                          writerQos,
                                                          TRUE);
            assert(publisher->sampleRequestWriter);
            u_entityAction(u_entity(publisher->sampleRequestWriter), d_publisherEnsureServicesAttached, durability);

            publisher->nameSpacesNumber = 0;
            publisher->nameSpacesWriter = u_writerNew (publisher->publisher,
                                                       "nameSpacesWriter",
                                                       d_adminGetNameSpacesTopic(admin),
                                                       d_publisherNameSpacesWriterCopy,
                                                       writerQos,
                                                       TRUE);
            assert(publisher->nameSpacesWriter);
            u_entityAction(u_entity(publisher->nameSpacesWriter), d_publisherEnsureServicesAttached, durability);

            publisher->nameSpacesRequestNumber = 0;
            publisher->nameSpacesRequestWriter = u_writerNew (publisher->publisher,
                                                              "nameSpacesRequestWriter",
                                                              d_adminGetNameSpacesRequestTopic(admin),
                                                              d_publisherNameSpacesRequestWriterCopy,
                                                              writerQos,
                                                              TRUE);
            assert(publisher->nameSpacesRequestWriter);
            u_entityAction(u_entity(publisher->nameSpacesRequestWriter), d_publisherEnsureServicesAttached, durability);

            publisher->deleteDataNumber = 0;
            publisher->deleteDataWriter = u_writerNew (publisher->publisher,
                                                       "deleteDataWriter",
                                                       d_adminGetDeleteDataTopic(admin),
                                                       d_publisherDeleteDataWriterCopy,
                                                       writerQos,
                                                       TRUE);
            assert(publisher->deleteDataWriter);
            u_entityAction(u_entity(publisher->deleteDataWriter), d_publisherEnsureServicesAttached, durability);

            d_writerQosFree(writerQos);
            writerQos = d_writerQosNew (V_DURABILITY_VOLATILE,
                                        V_RELIABILITY_RELIABLE,
                                        config->alignerLatencyBudget,
                                        config->alignerTransportPriority);
            publisher->sampleChainNumber = 0;
            publisher->sampleChainWriter = u_writerNew (publisher->publisher,
                                                        "sampleChainWriter",
                                                        d_adminGetSampleChainTopic(admin),
                                                        d_publisherSampleChainWriterCopy,
                                                        writerQos,
                                                        TRUE);
            assert(publisher->sampleChainWriter);
            u_entityAction(u_entity(publisher->sampleChainWriter), d_publisherEnsureServicesAttached, durability);

            d_writerQosFree(writerQos);
        } else {
            d_publisherFree(publisher);
            publisher = NULL;
        }
    }
    return publisher;
}

void
d_publisherDeinit(
    d_object object)
{
    d_publisher publisher;
    d_durability durability;

    assert(d_objectIsValid(object, D_PUBLISHER) == TRUE);

    if(object){
        publisher = d_publisher(object);
        durability = d_adminGetDurability(publisher->admin);

        if(publisher->statusWriter){
            d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_MAIN, "destroying status writer\n");
            u_writerFree(publisher->statusWriter);
            d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_MAIN, "status writer destroyed\n");
            publisher->statusWriter = NULL;
        }
        if(publisher->newGroupWriter){
            d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_MAIN, "destroying newGroup writer\n");
            u_writerFree(publisher->newGroupWriter);
            d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_MAIN, "newGroup writer destroyed\n");
            publisher->newGroupWriter = NULL;
        }
        if(publisher->groupsRequestWriter){
            d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_MAIN, "destroying groupsRequest writer\n");
            u_writerFree(publisher->groupsRequestWriter);
            d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_MAIN, "groupsRequest writer destroyed\n");
            publisher->groupsRequestWriter = NULL;
        }
        if(publisher->statusRequestWriter){
            d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_MAIN, "destroying statusRequest writer\n");
            u_writerFree(publisher->statusRequestWriter);
            d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_MAIN, "statusRequest writer destroyed\n");
            publisher->statusRequestWriter = NULL;
        }
        if(publisher->sampleChainWriter){
            d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_MAIN, "destroying sampleChain writer\n");
            u_writerFree(publisher->sampleChainWriter);
            d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_MAIN, "sampleChain writer destroyed\n");
            publisher->sampleChainWriter = NULL;
        }
        if(publisher->nameSpacesWriter){
            d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_MAIN, "destroying nameSpaces writer\n");
            u_writerFree(publisher->nameSpacesWriter);
            d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_MAIN, "nameSpaces writer destroyed\n");
            publisher->nameSpacesWriter = NULL;
        }
        if(publisher->nameSpacesRequestWriter){
            d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_MAIN, "destroying nameSpacesRequest writer\n");
            u_writerFree(publisher->nameSpacesRequestWriter);
            d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_MAIN, "nameSpacesRequest writer destroyed\n");
            publisher->nameSpacesRequestWriter = NULL;
        }
        if(publisher->deleteDataWriter){
            d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_MAIN, "destroying deleteData writer\n");
            u_writerFree(publisher->deleteDataWriter);
            d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_MAIN, "deleteData writer destroyed\n");
            publisher->deleteDataWriter = NULL;
        }
        if(publisher->sampleRequestWriter){
            d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_MAIN, "destroying sampleRequest writer\n");
            u_writerFree(publisher->sampleRequestWriter);
            d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_MAIN, "sampleRequest writer destroyed\n");
            publisher->sampleRequestWriter = NULL;
        }
        if(publisher->publisher){
            d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_MAIN, "destroying user publisher\n");
            u_publisherFree(publisher->publisher);
            d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_MAIN, "user publisher destroyed\n");
            publisher->publisher = NULL;
        }
    }
}

void
d_publisherFree(
    d_publisher publisher)
{
    assert(d_objectIsValid(d_object(publisher), D_PUBLISHER) == TRUE);

    if(publisher){
        d_objectFree(d_object(publisher), D_PUBLISHER);
    }
}

c_bool
d_publisherStatusWrite(
    d_publisher publisher,
    d_status message,
    d_networkAddress addressee)
{
    c_bool result;
    u_result ur;
    int resendCount;
    c_bool terminate;
    d_durability durability;

    OS_UNUSED_ARG(addressee);
    result = FALSE;
    assert(d_objectIsValid(d_object(publisher), D_PUBLISHER) == TRUE);

    if(publisher){
        if(publisher->enabled == TRUE){
            terminate = FALSE;
            resendCount = 0;
            durability = d_adminGetDurability(publisher->admin);
            d_publisherInitMessage(publisher, d_message(message));
            d_message(message)->sequenceNumber = publisher->statusNumber++;

            while((!result) && (!terminate)){
                ur = u_writerWrite(publisher->statusWriter,
                                   message,
                                   v_timeGet(),
                                   U_INSTANCEHANDLE_NIL);

                if(ur == U_RESULT_OK){
                    result = TRUE;
                } else if(ur == U_RESULT_TIMEOUT) {
                    terminate = d_durabilityMustTerminate(durability);
                    if(!terminate && d_message(message)->senderState == D_STATE_TERMINATING){
                        /* ES 21 - okt - 2011:
                         * if durability is not yet terminating, but the message being written
                         * does indicate that termination of durability is imminent, then we have
                         * to prevent durability from constantly retrying in the case of a TIMEOUT
                         * of the datawriter. The standard timeout provided to the write will ensure
                         * the write is tried during the period of the timeout. But once that timeout is
                         * exceeded then it is fruitless to continue try to write the terminating message
                         * Without this specific code, durability will hang when terminating is caused
                         * by splice daemon terminating.
                         */
                        terminate = TRUE;
                    }
                    resendCount++;

                    if(terminate){
                        d_printTimedEvent(durability, D_LEVEL_SEVERE, D_THREAD_UNSPECIFIED,
                            "Failed to resend d_status message, because durability is terminating.\n");
                        OS_REPORT(OS_WARNING, D_CONTEXT_DURABILITY, 0,
                            "Failed to send d_status message, because durability is terminating.");
                    }  else if((resendCount == 1) || ((resendCount % 5) == 0)){
                        d_printTimedEvent(durability, D_LEVEL_WARNING, D_THREAD_UNSPECIFIED,
                            "Already tried to resend d_status message '%d' times.\n", resendCount);

                        if(resendCount != 1){
                            OS_REPORT_1(OS_WARNING, D_CONTEXT_DURABILITY, 0,
                                    "Already tried to resend d_status message '%d' times.",
                                    resendCount);
                        }
                    }
                } else {
                    d_printTimedEvent(durability, D_LEVEL_SEVERE, D_THREAD_UNSPECIFIED,
                            "Write of d_status message FAILED with result %d.\n", ur);
                    OS_REPORT_1(OS_ERROR, D_CONTEXT_DURABILITY, 0,
                            "Write of d_status message FAILED with result %d.", ur);
                    d_durabilityTerminate(durability, TRUE);
                    terminate = d_durabilityMustTerminate(durability);
                }
            }
        }
    }
    return result;
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

    OS_UNUSED_ARG(addressee);
    result = FALSE;
    assert(d_objectIsValid(d_object(publisher), D_PUBLISHER) == TRUE);

    if(publisher){
        if(publisher->enabled == TRUE){
            terminate = FALSE;
            resendCount = 0;
            durability = d_adminGetDurability(publisher->admin);
            d_publisherInitMessage(publisher, d_message(message));
            d_message(message)->sequenceNumber = publisher->newGroupNumber++;

            while((!result) && (!terminate)){
                ur = u_writerWrite(publisher->newGroupWriter,
                                   message,
                                   v_timeGet(),
                                   U_INSTANCEHANDLE_NIL);

                if(ur == U_RESULT_OK){
                    result = TRUE;
                } else if(ur == U_RESULT_TIMEOUT) {
                    terminate = d_durabilityMustTerminate(durability);
                    resendCount++;

                    if(terminate){
                        d_printTimedEvent(durability, D_LEVEL_SEVERE, D_THREAD_UNSPECIFIED,
                            "Failed to resend d_newGroup message, because durability is terminating.\n");
                        OS_REPORT(OS_WARNING, D_CONTEXT_DURABILITY, 0,
                            "Failed to send d_newGroup message, because durability is terminating.");
                    } else if((resendCount == 1) || ((resendCount % 5) == 0)){
                        d_printTimedEvent(durability, D_LEVEL_WARNING, D_THREAD_UNSPECIFIED,
                            "Already tried to resend d_newGroup message '%d' times.\n", resendCount);

                        if(resendCount != 1){
                            OS_REPORT_1(OS_WARNING, D_CONTEXT_DURABILITY, 0,
                                    "Already tried to resend d_newGroup message '%d' times.",
                                    resendCount);
                        }
                    }
                } else {
                    d_printTimedEvent(durability, D_LEVEL_SEVERE, D_THREAD_UNSPECIFIED,
                            "Write of d_newGroup message FAILED with result %d.\n", ur);
                    OS_REPORT_1(OS_ERROR, D_CONTEXT_DURABILITY, 0,
                            "Write of d_newGroup message FAILED with result %d.", ur);
                    d_durabilityTerminate(durability, TRUE);
                    terminate = d_durabilityMustTerminate(durability);
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

    OS_UNUSED_ARG(addressee);
    result = FALSE;
    assert(d_objectIsValid(d_object(publisher), D_PUBLISHER) == TRUE);

    if(publisher){
        if(publisher->enabled == TRUE){
            terminate = FALSE;
            resendCount = 0;
            durability = d_adminGetDurability(publisher->admin);
            d_publisherInitMessage(publisher, d_message(message));
            d_message(message)->sequenceNumber = publisher->groupsRequestNumber++;

            while((!result) && (!terminate)){
                ur = u_writerWrite(publisher->groupsRequestWriter,
                                   message, v_timeGet(), U_INSTANCEHANDLE_NIL);

                if(ur == U_RESULT_OK){
                    result = TRUE;
                } else if(ur == U_RESULT_TIMEOUT) {
                    terminate = d_durabilityMustTerminate(durability);
                    resendCount++;

                    if(terminate){
                        d_printTimedEvent(durability, D_LEVEL_SEVERE, D_THREAD_UNSPECIFIED,
                            "Failed to resend d_groupsRequest message, because durability is terminating.\n");
                        OS_REPORT(OS_WARNING, D_CONTEXT_DURABILITY, 0,
                            "Failed to send d_groupsRequest message, because durability is terminating.");
                    } else if((resendCount == 1) || ((resendCount % 5) == 0)){
                        d_printTimedEvent(durability, D_LEVEL_WARNING, D_THREAD_UNSPECIFIED,
                            "Already tried to resend d_groupsRequest message '%d' times.\n", resendCount);

                        if(resendCount != 1){
                            OS_REPORT_1(OS_WARNING, D_CONTEXT_DURABILITY, 0,
                                    "Already tried to resend d_groupsRequest message '%d' times.",
                                    resendCount);
                        }
                    }
                } else {
                    d_printTimedEvent(durability, D_LEVEL_SEVERE, D_THREAD_UNSPECIFIED,
                            "Write of d_groupsRequest message FAILED with result %d.\n", ur);
                    OS_REPORT_1(OS_ERROR, D_CONTEXT_DURABILITY, 0,
                            "Write of d_groupsRequest message FAILED with result %d.", ur);
                    d_durabilityTerminate(durability, TRUE);
                    terminate = d_durabilityMustTerminate(durability);
                }
            }
        }
    }
    return result;
}

c_bool
d_publisherStatusRequestWrite(
    d_publisher publisher,
    d_statusRequest message,
    d_networkAddress addressee)
{
    c_bool result;
    u_result ur;
    int resendCount;
    c_bool terminate;
    d_durability durability;

    OS_UNUSED_ARG(addressee);
    result = FALSE;
    assert(d_objectIsValid(d_object(publisher), D_PUBLISHER) == TRUE);

    if(publisher){
        if(publisher->enabled == TRUE){
            terminate = FALSE;
            resendCount = 0;
            durability = d_adminGetDurability(publisher->admin);
            d_publisherInitMessage(publisher, d_message(message));
            d_message(message)->sequenceNumber = publisher->statusRequestNumber++;

            while((!result) && (!terminate)){
                ur = u_writerWrite(publisher->statusRequestWriter,
                                   message, v_timeGet(), U_INSTANCEHANDLE_NIL);

                if(ur == U_RESULT_OK){
                    result = TRUE;
                } else if(ur == U_RESULT_TIMEOUT) {
                    terminate = d_durabilityMustTerminate(durability);
                    resendCount++;

                    if(terminate){
                        d_printTimedEvent(durability, D_LEVEL_SEVERE, D_THREAD_UNSPECIFIED,
                            "Failed to resend d_statusRequest message, because durability is terminating.\n");
                        OS_REPORT(OS_WARNING, D_CONTEXT_DURABILITY, 0,
                            "Failed to send d_statusRequest message, because durability is terminating.");
                    } else if((resendCount == 1) || ((resendCount % 5) == 0)){
                        d_printTimedEvent(durability, D_LEVEL_WARNING, D_THREAD_UNSPECIFIED,
                            "Already tried to resend d_statusRequest message '%d' times.\n", resendCount);

                        if(resendCount != 1){
                            OS_REPORT_1(OS_WARNING, D_CONTEXT_DURABILITY, 0,
                                    "Already tried to resend d_statusRequest message '%d' times.",
                                    resendCount);
                        }
                    }
                } else {
                    d_printTimedEvent(durability, D_LEVEL_SEVERE, D_THREAD_UNSPECIFIED,
                            "Write of d_statusRequest message FAILED with result %d.\n", ur);
                    OS_REPORT_1(OS_ERROR, D_CONTEXT_DURABILITY, 0,
                            "Write of d_statusRequest message FAILED with result %d.", ur);
                    d_durabilityTerminate(durability, TRUE);
                    terminate = d_durabilityMustTerminate(durability);
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

    OS_UNUSED_ARG(addressee);
    result = FALSE;
    assert(d_objectIsValid(d_object(publisher), D_PUBLISHER) == TRUE);

    if(publisher){
        if(publisher->enabled == TRUE){
            terminate = FALSE;
            resendCount = 0;
            durability = d_adminGetDurability(publisher->admin);
            d_publisherInitMessage(publisher, d_message(message));
            d_message(message)->sequenceNumber = publisher->sampleRequestNumber++;

            while((!result) && (!terminate)){
                ur = u_writerWrite(publisher->sampleRequestWriter,
                                   message, v_timeGet(), U_INSTANCEHANDLE_NIL);

                if(ur == U_RESULT_OK){
                    result = TRUE;
                } else if(ur == U_RESULT_TIMEOUT) {
                    terminate = d_durabilityMustTerminate(durability);
                    resendCount++;

                    if(terminate){
                        d_printTimedEvent(durability, D_LEVEL_SEVERE, D_THREAD_UNSPECIFIED,
                            "Failed to resend d_sampleRequest message, because durability is terminating.\n");
                        OS_REPORT(OS_WARNING, D_CONTEXT_DURABILITY, 0,
                            "Failed to send d_sampleRequest message, because durability is terminating.");
                    } else if((resendCount == 1) || ((resendCount % 5) == 0)){
                        d_printTimedEvent(durability, D_LEVEL_WARNING, D_THREAD_UNSPECIFIED,
                            "Already tried to resend d_sampleRequest message '%d' times.\n", resendCount);

                        if(resendCount != 1){
                            OS_REPORT_1(OS_WARNING, D_CONTEXT_DURABILITY, 0,
                                    "Already tried to resend d_sampleRequest message '%d' times.",
                                    resendCount);
                        }
                    }
                } else {
                    d_printTimedEvent(durability, D_LEVEL_SEVERE, D_THREAD_UNSPECIFIED,
                            "Write of d_sampleRequest message FAILED with result %d.\n", ur);
                    OS_REPORT_1(OS_ERROR, D_CONTEXT_DURABILITY, 0,
                            "Write of d_sampleRequest message FAILED with result %d.", ur);
                    d_durabilityTerminate(durability, TRUE);
                    terminate = d_durabilityMustTerminate(durability);
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

    OS_UNUSED_ARG(addressee);
    result = FALSE;
    assert(d_objectIsValid(d_object(publisher), D_PUBLISHER) == TRUE);

    if(publisher){
        if(publisher->enabled == TRUE){
            terminate = FALSE;
            resendCount = 0;
            durability = d_adminGetDurability(publisher->admin);
            d_publisherInitMessage(publisher, d_message(message));
            d_message(message)->sequenceNumber = publisher->sampleChainNumber++;

            while((!result) && (!terminate)){
                ur = u_writerWrite(publisher->sampleChainWriter,
                                   message, v_timeGet(), U_INSTANCEHANDLE_NIL);

                if(ur == U_RESULT_OK){
                    result = TRUE;
                } else if(ur == U_RESULT_TIMEOUT) {
                    terminate = d_durabilityMustTerminate(durability);
                    resendCount++;

                    if(terminate){
                        d_printTimedEvent(durability, D_LEVEL_SEVERE, D_THREAD_UNSPECIFIED,
                            "Failed to resend d_sampleChain message, because durability is terminating.\n");
                        OS_REPORT(OS_WARNING, D_CONTEXT_DURABILITY, 0,
                            "Failed to send d_sampleChain message, because durability is terminating.");
                    } else if((resendCount == 1) || ((resendCount % 5) == 0)){
                        d_printTimedEvent(durability, D_LEVEL_WARNING, D_THREAD_UNSPECIFIED,
                            "Already tried to resend d_sampleChain message '%d' times.\n", resendCount);

                        if(resendCount != 1){
                            OS_REPORT_1(OS_WARNING, D_CONTEXT_DURABILITY, 0,
                                    "Already tried to resend d_sampleChain message '%d' times.",
                                    resendCount);
                        }
                    }
                } else {
                    d_printTimedEvent(durability, D_LEVEL_SEVERE, D_THREAD_UNSPECIFIED,
                            "Write of d_sampleChain message FAILED with result %d.\n", ur);
                    OS_REPORT_1(OS_ERROR, D_CONTEXT_DURABILITY, 0,
                            "Write of d_sampleChain message FAILED with result %d.\n", ur);
                    d_durabilityTerminate(durability, TRUE);
                    terminate = d_durabilityMustTerminate(durability);
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
    d_networkAddress addressee)
{
    c_bool result;
    u_result ur;
    int resendCount;
    c_bool terminate;
    d_durability durability;

    OS_UNUSED_ARG(addressee);
    result = FALSE;
    assert(d_objectIsValid(d_object(publisher), D_PUBLISHER) == TRUE);

    if(publisher){
        if(publisher->enabled == TRUE){
            terminate = FALSE;
            resendCount = 0;
            durability = d_adminGetDurability(publisher->admin);
            d_publisherInitMessage(publisher, d_message(message));
            d_message(message)->sequenceNumber = publisher->nameSpacesRequestNumber++;

            while((!result) && (!terminate)){
                ur = u_writerWrite(publisher->nameSpacesRequestWriter,
                                   message, v_timeGet(), U_INSTANCEHANDLE_NIL);

                if(ur == U_RESULT_OK){
                    result = TRUE;
                } else if(ur == U_RESULT_TIMEOUT) {
                    terminate = d_durabilityMustTerminate(durability);
                    resendCount++;

                    if(terminate){
                        d_printTimedEvent(durability, D_LEVEL_SEVERE, D_THREAD_UNSPECIFIED,
                            "Failed to resend d_nameSpaceRequest message, because durability is terminating.\n");
                        OS_REPORT(OS_WARNING, D_CONTEXT_DURABILITY, 0,
                            "Failed to send d_nameSpaceRequest message, because durability is terminating.");
                    } else if((resendCount == 1) || ((resendCount % 5) == 0)){
                        d_printTimedEvent(durability, D_LEVEL_WARNING, D_THREAD_UNSPECIFIED,
                            "Already tried to resend d_nameSpaceRequest message '%d' times.\n", resendCount);

                        if(resendCount != 1){
                            OS_REPORT_1(OS_WARNING, D_CONTEXT_DURABILITY, 0,
                                    "Already tried to resend d_nameSpaceRequest message '%d' times",
                                    resendCount);
                        }
                    }
                } else {
                    d_printTimedEvent(durability, D_LEVEL_SEVERE, D_THREAD_UNSPECIFIED,
                            "Write of d_nameSpaceRequest message FAILED with result %d.\n", ur);
                    OS_REPORT_1(OS_ERROR, D_CONTEXT_DURABILITY, 0,
                            "Write of d_nameSpaceRequest message FAILED with result %d.", ur);
                    d_durabilityTerminate(durability, TRUE);
                    terminate = d_durabilityMustTerminate(durability);
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

    OS_UNUSED_ARG(addressee);
    result = FALSE;
    assert(d_objectIsValid(d_object(publisher), D_PUBLISHER) == TRUE);

    if(publisher){
        if(publisher->enabled == TRUE){
            terminate = FALSE;
            resendCount = 0;
            durability = d_adminGetDurability(publisher->admin);
            d_publisherInitMessage(publisher, d_message(message));
            d_message(message)->sequenceNumber = publisher->nameSpacesNumber++;

            while((!result) && (!terminate)){
                 ur = u_writerWrite(publisher->nameSpacesWriter,
                                   message, v_timeGet(), U_INSTANCEHANDLE_NIL);

                if(ur == U_RESULT_OK){
                    result = TRUE;
                } else if(ur == U_RESULT_TIMEOUT) {
                    terminate = d_durabilityMustTerminate(durability);
                    resendCount++;

                    if(terminate){
                        d_printTimedEvent(durability, D_LEVEL_SEVERE, D_THREAD_UNSPECIFIED,
                            "Failed to resend d_nameSpaces message, because durability is terminating.\n");
                        OS_REPORT(OS_WARNING, D_CONTEXT_DURABILITY, 0,
                            "Failed to send d_nameSpaces message, because durability is terminating.");
                    } else if((resendCount == 1) || ((resendCount % 5) == 0)){
                        d_printTimedEvent(durability, D_LEVEL_WARNING, D_THREAD_UNSPECIFIED,
                            "Already tried to resend d_nameSpaces message '%d' times.\n", resendCount);

                        if(resendCount != 1){
                            OS_REPORT_1(OS_WARNING, D_CONTEXT_DURABILITY, 0,
                                    "Already tried to resend d_nameSpaces message '%d' times.",
                                    resendCount);
                        }
                    }
                } else {
                    d_printTimedEvent(durability, D_LEVEL_SEVERE, D_THREAD_UNSPECIFIED,
                            "Write of d_nameSpaces message FAILED with result %d.\n", ur);
                    OS_REPORT_1(OS_ERROR, D_CONTEXT_DURABILITY, 0,
                            "Write of d_nameSpaces message FAILED with result %d.", ur);
                    d_durabilityTerminate(durability, TRUE);
                    terminate = d_durabilityMustTerminate(durability);
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

    OS_UNUSED_ARG(addressee);
    result = FALSE;
    assert(d_objectIsValid(d_object(publisher), D_PUBLISHER) == TRUE);

    if(publisher){
        if(publisher->enabled == TRUE){
            terminate = FALSE;
            resendCount = 0;
            durability = d_adminGetDurability(publisher->admin);
            d_publisherInitMessage(publisher, d_message(message));
            d_message(message)->sequenceNumber = publisher->deleteDataNumber++;

            while((!result) && (!terminate)){
                ur = u_writerWrite(publisher->deleteDataWriter,
                                   message, v_timeGet(), U_INSTANCEHANDLE_NIL);

                if(ur == U_RESULT_OK){
                    result = TRUE;
                } else if(ur == U_RESULT_TIMEOUT) {
                    terminate = d_durabilityMustTerminate(durability);
                    resendCount++;

                    if(terminate){
                        d_printTimedEvent(durability, D_LEVEL_SEVERE, D_THREAD_UNSPECIFIED,
                            "Failed to resend d_deleteData message, because durability is terminating.\n");
                        OS_REPORT(OS_WARNING, D_CONTEXT_DURABILITY, 0,
                            "Failed to send d_deleteData message, because durability is terminating.");
                    } else if((resendCount == 1) || ((resendCount % 5) == 0)){
                        d_printTimedEvent(durability, D_LEVEL_WARNING, D_THREAD_UNSPECIFIED,
                            "Already tried to resend d_deleteData message '%d' times.\n", resendCount);

                        if(resendCount != 1){
                            OS_REPORT_1(OS_WARNING, D_CONTEXT_DURABILITY, 0,
                                    "Already tried to resend d_deleteData message '%d' times.",
                                    resendCount);
                        }
                    }
                } else {
                    d_printTimedEvent(durability, D_LEVEL_SEVERE, D_THREAD_UNSPECIFIED,
                            "Write of d_deleteData message FAILED with result %d.\n", ur);
                    OS_REPORT_1(OS_ERROR, D_CONTEXT_DURABILITY, 0,
                            "Write of d_deleteData message FAILED with result %d.", ur);
                    d_durabilityTerminate(durability, TRUE);
                    terminate = d_durabilityMustTerminate(durability);
                }
            }
        }
    }
    return result;
}

c_bool
d_publisherSampleChainWriterCopy(
    c_type type,
    void *data,
    void *to)
{
    d_sampleChain msgFrom = d_sampleChain(data);
    d_sampleChain msgTo   = d_sampleChain(to);
    c_base           base = c_getBase(type);
    static c_type    type0 = NULL;
    static c_type    type1 = NULL;
    unsigned int     valueSize;

    d_publisherMessageWriterCopy(&msgFrom->parentMsg, &msgTo->parentMsg);

    msgTo->partition       = c_stringNew(base, msgFrom->partition);
    msgTo->topic           = c_stringNew(base, msgFrom->topic);
    msgTo->durabilityKind  = msgFrom->durabilityKind;
    msgTo->msgBody._d      = msgFrom->msgBody._d;
    msgTo->addresseesCount = msgFrom->addresseesCount;

    assert(msgTo->addresseesCount > 0);
    if (type1 == NULL) {
        type1 = c_resolve(base, "durabilityModule2::d_networkAddress_s");
    }
    assert(type1 != NULL);
    msgTo->addressees = c_arrayNew(type1, msgTo->addresseesCount);
    assert(msgTo->addressees);
    memcpy(msgTo->addressees, msgFrom->addressees, msgTo->addresseesCount*C_SIZEOF(d_networkAddress));

    msgTo->source.systemId    = msgFrom->source.systemId;
    msgTo->source.localId     = msgFrom->source.localId;
    msgTo->source.lifecycleId = msgFrom->source.lifecycleId;

    switch(msgTo->msgBody._d) {
    case BEAD:
        if (type0 == NULL) {
            type0 = c_resolve(base, "c_octet");
        }
        assert(type0 != NULL);
        valueSize = msgFrom->msgBody._u.bead.size;
        msgTo->msgBody._u.bead.size  = valueSize;
        msgTo->msgBody._u.bead.value = c_arrayNew(type0, valueSize);
        memcpy(msgTo->msgBody._u.bead.value, msgFrom->msgBody._u.bead.value, valueSize);
    break;
    case LINK:
        msgTo->msgBody._u.link.nrSamples      = msgFrom->msgBody._u.link.nrSamples;
        msgTo->msgBody._u.link.completeness   = msgFrom->msgBody._u.link.completeness;
    break;
    default:
        OS_REPORT_1(OS_ERROR, "d_publisherSampleChainWriterCopy", 0,
                    "Illegal message body discriminant value (%d) detected.",
                    msgTo->msgBody._d);
        assert(FALSE);
    }

    return TRUE;
}

c_bool
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

    return TRUE;
}

c_bool
d_publisherStatusWriterCopy(
    c_type type,
    void *data,
    void *to)
{
    OS_UNUSED_ARG(type);
    assert(type);
    return d_publisherMessageWriterCopy(&d_status(data)->parentMsg, &d_status(to)->parentMsg);
}

c_bool
d_publisherNewGroupWriterCopy(
    c_type type,
    void *data,
    void *to)
{
    c_bool result;
    d_newGroup msgFrom    = (d_newGroup)data;
    d_newGroup msgTo      = (d_newGroup)to;
    c_base base           = c_getBase(type);

    result = d_publisherMessageWriterCopy(&msgFrom->parentMsg, &msgTo->parentMsg);

    if(msgFrom->partition){
        msgTo->partition = c_stringNew(base, msgFrom->partition);
    } else {
        msgTo->partition = NULL;
    }
    if(msgFrom->topic){
        msgTo->topic = c_stringNew(base, msgFrom->topic);
    } else {
        msgTo->topic = NULL;
    }
    msgTo->completeness   = msgFrom->completeness;
    msgTo->durabilityKind = msgFrom->durabilityKind;
    msgTo->quality        = msgFrom->quality;
    msgTo->alignerCount   = msgFrom->alignerCount;

    return result;
}

c_bool
d_publisherGroupsRequestWriterCopy(
    c_type type,
    void *data,
    void *to)
{
    c_bool result;
    d_groupsRequest msgFrom = (d_groupsRequest)data;
    d_groupsRequest msgTo   = (d_groupsRequest)to;
    c_base base             = c_getBase(type);

    result = d_publisherMessageWriterCopy(&d_groupsRequest(data)->parentMsg, &d_groupsRequest(to)->parentMsg);

    if(msgFrom->partition){
        msgTo->partition = c_stringNew(base, msgFrom->partition);
    } else {
        msgTo->partition = NULL;
    }
    if(msgFrom->topic){
        msgTo->topic = c_stringNew(base, msgFrom->topic);
    } else {
        msgTo->topic = NULL;
    }

    return result;
}

c_bool
d_publisherStatusRequestWriterCopy(
    c_type type,
    void *data,
    void *to)
{
    OS_UNUSED_ARG(type);
    assert(type);
    return d_publisherMessageWriterCopy(&d_statusRequest(data)->parentMsg, &d_status(to)->parentMsg);

}

c_bool
d_publisherSampleRequestWriterCopy(
    c_type type,
    void *data,
    void *to)
{
    c_bool result;
    c_ulong i;
    static c_type type3;
    d_sampleRequest msgFrom = d_sampleRequest(data);
    d_sampleRequest msgTo   = d_sampleRequest(to);
    c_base base             = c_getBase(type);

    result = d_publisherMessageWriterCopy(&msgFrom->parentMsg, &msgTo->parentMsg);

    msgTo->partition      = c_stringNew(base, msgFrom->partition);
    msgTo->topic          = c_stringNew(base, msgFrom->topic);
    msgTo->durabilityKind = msgFrom->durabilityKind;
    msgTo->requestTime    = msgFrom->requestTime;
    msgTo->withTimeRange  = msgFrom->withTimeRange;
    msgTo->beginTime      = msgFrom->beginTime;
    msgTo->endTime        = msgFrom->endTime;

    msgTo->source.systemId    = msgFrom->source.systemId;
    msgTo->source.localId     = msgFrom->source.localId;
    msgTo->source.lifecycleId = msgFrom->source.lifecycleId;

    if(msgFrom->filter){
        msgTo->filter = c_stringNew(base, msgFrom->filter);
    } else {
        msgTo->filter = NULL;
    }
    if(msgFrom->filterParams){
        if (type3 == NULL) {
            type3 = c_resolve(base, "c_string");
        }
        assert(type3 != NULL);
        msgTo->filterParamsCount = msgFrom->filterParamsCount;
        msgTo->filterParams = c_arrayNew(type3, msgFrom->filterParamsCount);


        for(i=0; i<msgFrom->filterParamsCount; i++){
            msgTo->filterParams[i] = c_stringNew(base, msgFrom->filterParams[i]);
        }
    }
    msgTo->maxSamples            = msgFrom->maxSamples;
    msgTo->maxInstances          = msgFrom->maxInstances;
    msgTo->maxSamplesPerInstance = msgFrom->maxSamplesPerInstance;

    return result;
}

c_bool
d_publisherNameSpacesWriterCopy(
    c_type type,
    void *data,
    void *to)
{
    c_bool result;
    c_ulong i;
    static c_type mergeStateType = NULL;

    d_nameSpaces msgFrom = d_nameSpaces(data);
    d_nameSpaces msgTo   = d_nameSpaces(to);
    c_base base          = c_getBase(type);
    OS_UNUSED_ARG(type);

    result = d_publisherMessageWriterCopy(&msgFrom->parentMsg, &msgTo->parentMsg);

    msgTo->name                       = c_stringNew(base, msgFrom->name);
    msgTo->durabilityKind             = msgFrom->durabilityKind;
    msgTo->alignmentKind              = msgFrom->alignmentKind;
    msgTo->aligner                    = msgFrom->aligner;
    msgTo->total                      = msgFrom->total;
    msgTo->initialQuality.seconds     = msgFrom->initialQuality.seconds;
    msgTo->initialQuality.nanoseconds = msgFrom->initialQuality.nanoseconds;
    msgTo->master.systemId            = msgFrom->master.systemId;
    msgTo->master.localId             = msgFrom->master.localId;
    msgTo->master.lifecycleId         = msgFrom->master.lifecycleId;
    msgTo->isComplete                 = msgFrom->isComplete;
    msgTo->masterConfirmed            = msgFrom->masterConfirmed;
    msgTo->state.value                = msgFrom->state.value;

    if(msgFrom->partitions) {
        msgTo->partitions = c_stringNew(base, msgFrom->partitions);
    } else {
        msgTo->partitions = NULL;
    }
    if(msgFrom->state.role) {
        msgTo->state.role    = c_stringNew(base, msgFrom->state.role);
    } else {
        msgTo->state.role    = NULL;
    }
    msgTo->state.value       = msgFrom->state.value;
    msgTo->mergedStatesCount = msgFrom->mergedStatesCount;

    if (mergeStateType == NULL) {
        mergeStateType = c_resolve(base, "durabilityModule2::d_mergeState_s");
    }
    assert(mergeStateType);

    if(msgTo->mergedStatesCount > 0){
        msgTo->mergedStates = c_arrayNew(mergeStateType, msgTo->mergedStatesCount);
        assert(msgTo->mergedStates);
    } else {
        msgTo->mergedStates = NULL;
    }

    for(i=0; i<msgTo->mergedStatesCount; i++){
        (((d_mergeState)(msgTo->mergedStates))[i]).value =
            (((d_mergeState)msgFrom->mergedStates)[i]).value;
        (((d_mergeState)msgTo->mergedStates)[i]).role  =
            c_stringNew(base, (((d_mergeState)msgFrom->mergedStates)[i]).role);
    }


    return result;
}

c_bool
d_publisherNameSpacesRequestWriterCopy(
    c_type type,
    void *data,
    void *to)
{
    OS_UNUSED_ARG(type);
    assert(type);

    return d_publisherMessageWriterCopy(&d_nameSpacesRequest(data)->parentMsg, &d_nameSpacesRequest(to)->parentMsg);
}

c_bool
d_publisherDeleteDataWriterCopy(
    c_type type,
    void *data,
    void *to)
{
    c_bool result;
    d_deleteData msgFrom = d_deleteData(data);
    d_deleteData msgTo   = d_deleteData(to);
    c_base base          = c_getBase(type);

    result = d_publisherMessageWriterCopy(&msgFrom->parentMsg, &msgTo->parentMsg);

    msgTo->actionTime.seconds       = msgFrom->actionTime.seconds;
    msgTo->actionTime.nanoseconds   = msgFrom->actionTime.nanoseconds;

    if(msgFrom->partitionExpr) {
        msgTo->partitionExpr = c_stringNew(base, msgFrom->partitionExpr);
    } else {
        msgTo->partitionExpr = NULL;
    }
    if(msgFrom->topicExpr) {
        msgTo->topicExpr = c_stringNew(base, msgFrom->topicExpr);
    } else {
        msgTo->topicExpr = NULL;
    }
    return result;
}

void
d_publisherInitMessage(
    d_publisher publisher,
    d_message message)
{
    d_message(message)->productionTimestamp = v_timeGet();
    d_message(message)->senderState = d_durabilityGetState(
                                            d_adminGetDurability(
                                                            publisher->admin));
}

void
d_publisherEnsureServicesAttached(
    v_entity entity,
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
