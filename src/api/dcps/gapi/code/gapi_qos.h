/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifndef GAPI_QOS_H
#define GAPI_QOS_H

#include "gapi.h"
#include "gapi_common.h"

#define GAPI_DURATION_INFINITE          { GAPI_DURATION_INFINITE_SEC, GAPI_DURATION_INFINITE_NSEC }
#define GAPI_DURATION_ZERO              { GAPI_DURATION_ZERO_SEC, GAPI_DURATION_ZERO_NSEC }

extern gapi_domainParticipantQos        gapi_domainParticipantQosDefault;
extern gapi_topicQos                    gapi_topicQosDefault;
extern gapi_publisherQos                gapi_publisherQosDefault;
extern gapi_subscriberQos               gapi_subscriberQosDefault;
extern gapi_dataReaderQos               gapi_dataReaderQosDefault;
extern gapi_dataReaderViewQos           gapi_dataReaderViewQosDefault;
extern gapi_dataWriterQos               gapi_dataWriterQosDefault;

gapi_returnCode_t
gapi_domainParticipantFactoryQosIsConsistent (
    const gapi_domainParticipantFactoryQos *qos,
    const gapi_context              *context);

gapi_returnCode_t
gapi_domainParticipantQosIsConsistent (
    const gapi_domainParticipantQos *qos,
    const gapi_context              *context);

gapi_returnCode_t
gapi_topicQosIsConsistent (
    const gapi_topicQos *qos,
    const gapi_context  *context);

gapi_boolean
gapi_topicQosEqual (
    const gapi_topicQos *qos1,
    const gapi_topicQos *qos2);

gapi_returnCode_t
gapi_publisherQosIsConsistent (
    const gapi_publisherQos *qos,
    const gapi_context      *context);

gapi_returnCode_t
gapi_subscriberQosIsConsistent (
    const gapi_subscriberQos *qos,
    const gapi_context       *context);

gapi_returnCode_t
gapi_dataReaderQosIsConsistent (
    const gapi_dataReaderQos *qos,
    const gapi_context       *context);


gapi_returnCode_t
gapi_dataReaderViewQosIsConsistent (
    const gapi_dataReaderViewQos *qos,
    const gapi_context           *context);

gapi_returnCode_t
gapi_dataWriterQosIsConsistent (
    const gapi_dataWriterQos *qos,
    const gapi_context       *context);

gapi_returnCode_t
gapi_domainParticipantFactoryQosCheckMutability (
    const gapi_domainParticipantFactoryQos *new_qos,
    const gapi_domainParticipantFactoryQos *old_qos,
    const gapi_context              *context);

gapi_returnCode_t
gapi_domainParticipantQosCheckMutability (
    const gapi_domainParticipantQos *new_qos,
    const gapi_domainParticipantQos *old_qos,
    const gapi_context              *context);

gapi_returnCode_t
gapi_topicQosCheckMutability (
    const gapi_topicQos *new_qos,
    const gapi_topicQos *old_qos,
    const gapi_context  *context);

gapi_returnCode_t
gapi_publisherQosCheckMutability (
    const gapi_publisherQos *new_qos,
    const gapi_publisherQos *old_qos,
    const gapi_context      *context);

gapi_returnCode_t
gapi_subscriberQosCheckMutability (
    const gapi_subscriberQos *new_qos,
    const gapi_subscriberQos *old_qos,
    const gapi_context       *context);

gapi_returnCode_t
gapi_dataReaderQosCheckMutability (
    const gapi_dataReaderQos *new_qos,
    const gapi_dataReaderQos *old_qos,
    const gapi_context       *context);

gapi_returnCode_t
gapi_dataReaderViewQosCheckMutability (
    const gapi_dataReaderViewQos *new_qos,
    const gapi_dataReaderViewQos *old_qos,
    const gapi_context           *context);


gapi_returnCode_t
gapi_dataWriterQosCheckMutability (
    const gapi_dataWriterQos *new_qos,
    const gapi_dataWriterQos *old_qos,
    const gapi_context       *context);

#endif
