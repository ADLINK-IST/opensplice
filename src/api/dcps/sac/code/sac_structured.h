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
#ifndef SAC_STRUCTURED_H
#define SAC_STRUCTURED_H

#include "gapi.h"
#include "gapi_structured.h"
#include "dds_dcps.h"

#define DDS_SampleLostStatusCopyin(src,dst) \
	gapi_sampleLostStatusCopyin(src, (gapi_sampleLostStatus *)dst)
#define DDS_SampleLostStatusCopyout(src,dst) \
	gapi_sampleLostStatusCopyout((gapi_sampleLostStatus *)src, dst)

#define DDS_SampleRejectedStatusCopyin(src,dst) \
	gapi_sampleRejectedStatusCopyin(src, (gapi_sampleRejectedStatus *)dst)
#define DDS_SampleRejectedStatusCopyout(src,dst) \
	gapi_sampleRejectedStatusCopyout((gapi_sampleRejectedStatus *)src, dst)

#define DDS_LivelinessLostStatusCopyin(src,dst) \
	gapi_livelinessLostStatusCopyin(src, (gapi_livelinessLostStatus *)dst)
#define DDS_LivelinessLostStatusCopyout(src,dst) \
	gapi_livelinessLostStatusCopyout((gapi_livelinessLostStatus *)src, dst)

#define DDS_LivelinessChangedStatusCopyin(src,dst) \
	gapi_livelinessChangedStatusCopyin(src, (gapi_livelinessChangedStatus *)dst)
#define DDS_LivelinessChangedStatusCopyout(src,dst) \
	gapi_livelinessChangedStatusCopyout((gapi_livelinessChangedStatus *)src, dst)

#define DDS_OfferedDeadlineMissedStatusCopyin(src,dst) \
	gapi_offeredDeadlineMissedStatusCopyin(src, (gapi_offeredDeadlineMissedStatus *)dst)
#define DDS_OfferedDeadlineMissedStatusCopyout(src,dst) \
	gapi_offeredDeadlineMissedStatusCopyout((gapi_offeredDeadlineMissedStatus *)src, dst)

#define DDS_RequestedDeadlineMissedStatusCopyin(src,dst) \
	gapi_requestedDeadlineMissedStatusCopyin(src, (gapi_requestedDeadlineMissedStatus *)dst)
#define DDS_RequestedDeadlineMissedStatusCopyout(src,dst) \
	gapi_requestedDeadlineMissedStatusCopyout((gapi_requestedDeadlineMissedStatus *)src, dst)

#define DDS_QosPolicyCountCopyin(src,dst) \
	gapi_qosPolicyCountCopyin(src, (gapi_qosPolicyCount *)dst)
#define DDS_QosPolicyCountCopyout(src,dst) \
	gapi_qosPolicyCountCopyout((gapi_qosPolicyCount *)src, dst)

#define DDS_QosPolicyCountSeqCopyin(src,dst) \
	gapi_qosPolicyCountSeqCopyin(src, (gapi_qosPolicyCountSeq *)dst)
#define DDS_QosPolicyCountSeqCopyout(src,dst) \
	gapi_qosPolicyCountSeqCopyout((gapi_qosPolicyCountSeq *)src, dst)

#define DDS_OfferedIncompatibleQosStatusCopyin(src,dst) \
	gapi_offeredIncompatibleQosStatusCopyin(src, (gapi_offeredIncompatibleQosStatus *)dst)
#define DDS_OfferedIncompatibleQosStatusCopyout(src,dst) \
	gapi_offeredIncompatibleQosStatusCopyout((gapi_offeredIncompatibleQosStatus *)src, dst)

#define DDS_RequestedIncompatibleQosStatusCopyin(src,dst) \
	gapi_requestedIncompatibleQosStatusCopyin(src, (gapi_requestedIncompatibleQosStatus *)dst)
#define DDS_RequestedIncompatibleQosStatusCopyout(src,dst) \
	gapi_requestedIncompatibleQosStatusCopyout((gapi_requestedIncompatibleQosStatus *)src, dst)

#define DDS_PublicationMatchedStatusCopyin(src,dst) \
	gapi_publicationMatchedStatusCopyin(src, (gapi_publicationMatchedStatus *)dst)
#define DDSgapi_publicationMatchedStatusCopyout(src,dst) \
	gapi_publicationMatchedStatus((gapi_publicationMatchedStatus *)src, dst)

#define DDS_SubscriptionMatchedStatusCopyin(src,dst) \
	gapi_subscriptionMatchedStatusCopyin(src, (gapi_subscriptionMatchedStatus *)dst)
#define DDS_SubscriptionMatchedStatusCopyout(src,dst) \
	gapi_subscriptionMatchedStatusCopyout((gapi_subscriptionMatchedStatus *)src, dst)

void
sac_copyGapiListener (
    const struct gapi_listener *gapiListener,
    struct DDS_Listener        *sacListener
    );

void
sac_copySacListener (
    const struct DDS_Listener *sacListener,
    struct gapi_listener      *gapiListener
    );

void
sac_copyGapiDataWriterListener (
    const struct gapi_dataWriterListener *gapiListener,
    struct DDS_DataWriterListener        *sacListener
    );

void
sac_copySacDataWriterListener (
    const struct DDS_DataWriterListener *sacListener,
    struct gapi_dataWriterListener *gapiListener
    );

void
sac_copyGapiDataReaderListener (
    const struct gapi_dataReaderListener *gapiListener,
    struct DDS_DataReaderListener        *sacListener
    );

void
sac_copySacDataReaderListener (
    const struct DDS_DataReaderListener *sacListener,
    struct gapi_dataReaderListener      *gapiListener
    );

void
sac_copyGapiPublisherListener (
    const struct gapi_publisherListener *gapiListener,
    struct DDS_PublisherListener        *sacListener
    );

void
sac_copySacPublisherListener (
    const struct DDS_PublisherListener *sacListener,
    struct gapi_publisherListener      *gapiListener
    );

void
sac_copyGapiSubscriberListener (
    const struct gapi_subscriberListener *gapiListener,
    struct DDS_SubscriberListener        *sacListener
    );

void
sac_copySacSubscriberListener (
    const struct DDS_SubscriberListener *sacListener,
    struct gapi_subscriberListener      *gapiListener
    );

void
sac_copyGapiTopicListener (
    const struct gapi_topicListener *gapiListener,
    struct DDS_TopicListener        *sacListener
    );

void
sac_copySacTopicListener (
    const struct DDS_TopicListener *sacListener,
    struct gapi_topicListener      *gapiListener
    );

void
sac_copyGapiDomainParticipantListener (
    const struct gapi_domainParticipantListener *gapiListener,
    struct DDS_DomainParticipantListener        *sacListener
    );

void
sac_copySacDomainParticipantListener (
    const struct DDS_DomainParticipantListener *sacListener,
    struct gapi_domainParticipantListener      *gapiListener
    );

void
sac_copyGapiInconsistentTopicStatus (
    gapi_inconsistentTopicStatus *s,
    DDS_InconsistentTopicStatus *d
    );

void
sac_copySacInconsistentTopicStatus (
    DDS_InconsistentTopicStatus *s,
    gapi_inconsistentTopicStatus *d
    );

void
sac_copyGapiSampleLostStatus (
    gapi_sampleLostStatus *s,
    DDS_SampleLostStatus *d
    );

void
sac_copySacSampleLostStatus (
    DDS_SampleLostStatus *s,
    gapi_sampleLostStatus *d
    );

void
sac_copyGapiSampleRejectedStatus (
    gapi_sampleRejectedStatus *s,
    DDS_SampleRejectedStatus *d
    );

void
sac_copySacSampleRejectedStatus (
    DDS_SampleRejectedStatus *s,
    gapi_sampleRejectedStatus *d
    );

void
sac_copyGapiLivelinessLostStatus (
    gapi_livelinessLostStatus *s,
    DDS_LivelinessLostStatus *d
    );

void
sac_copySacLivelinessLostStatus (
    DDS_LivelinessLostStatus *s,
    gapi_livelinessLostStatus *d
    );

void
sac_copyGapiLivelinessChangedStatus (
    gapi_livelinessChangedStatus *s,
    DDS_LivelinessChangedStatus *d
    );

void
sac_copySacLivelinessChangedStatus (
    DDS_LivelinessChangedStatus *s,
    gapi_livelinessChangedStatus *d
    );

void
sac_copyGapiRequestedDeadlineMissedStatus (
    gapi_requestedDeadlineMissedStatus *s,
    DDS_RequestedDeadlineMissedStatus *d
    );

void
sac_copySacRequestedDeadlineMissedStatus (
    DDS_RequestedDeadlineMissedStatus *s,
    gapi_requestedDeadlineMissedStatus *d
    );

void
sac_copyGapiOfferedIncompatibleQosStatus (
    gapi_offeredIncompatibleQosStatus *s,
    DDS_OfferedIncompatibleQosStatus *d
    );

void
sac_copySacOfferedIncompatibleQosStatus (
    DDS_OfferedIncompatibleQosStatus *s,
    gapi_offeredIncompatibleQosStatus *d
    );

void
sac_copyGapiRequestedIncompatibleQosStatus (
    gapi_requestedIncompatibleQosStatus *s,
    DDS_RequestedIncompatibleQosStatus *d
    );

void
sac_copySacRequestedIncompatibleQosStatus (
    DDS_RequestedIncompatibleQosStatus *s,
    gapi_requestedIncompatibleQosStatus *d
    );

void
sac_copyGapiPublicationMatchedStatus (
    gapi_publicationMatchedStatus *s,
    DDS_PublicationMatchedStatus *d
    );

void
sac_copySacPublicationMatchedStatus (
    DDS_PublicationMatchedStatus *s,
    gapi_publicationMatchedStatus *d
    );

void
sac_copyGapiSubscriptionMatchedStatus (
    gapi_subscriptionMatchedStatus *s,
    DDS_SubscriptionMatchedStatus *d
    );

void
sac_copySacSubscriptionMatchedStatus (
    DDS_SubscriptionMatchedStatus *s,
    gapi_subscriptionMatchedStatus *d
    );

#endif /* SAC_STRUCTURED_H */
