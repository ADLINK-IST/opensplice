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
#include "sac_structured.h"

#include "os_heap.h"

#include <string.h>
#include <assert.h>

#ifndef NULL
#define NULL 0
#endif

void
sac_copyGapiListener (
    const struct gapi_listener *gapiListener,
    struct DDS_Listener        *sacListener
    )
{
    sacListener->listener_data = gapiListener->listener_data;
}
    
void
sac_copySacListener (
    const struct DDS_Listener *sacListener,
    struct gapi_listener      *gapiListener
    )
{
    gapiListener->listener_data = sacListener->listener_data;
}
    
void
sac_copyGapiDataWriterListener (
    const struct gapi_dataWriterListener *gapiListener,
    struct DDS_DataWriterListener        *sacListener
    )
{
    sacListener->listener_data = gapiListener->listener_data;
    sacListener->on_offered_deadline_missed = 
	(DDS_DataWriterListener_OfferedDeadlineMissedListener)gapiListener->on_offered_deadline_missed;
    sacListener->on_offered_incompatible_qos = 
	(DDS_DataWriterListener_OfferedIncompatibleQosListener)gapiListener->on_offered_incompatible_qos;
    sacListener->on_liveliness_lost = 
	(DDS_DataWriterListener_LivelinessLostListener)gapiListener->on_liveliness_lost;
    sacListener->on_publication_matched = 
	(DDS_DataWriterListener_PublicationMatchedListener)gapiListener->on_publication_match;
}

void
sac_copySacDataWriterListener (
    const struct DDS_DataWriterListener *sacListener,
    struct gapi_dataWriterListener *gapiListener
    )
{
    gapiListener->listener_data = sacListener->listener_data;
    gapiListener->on_offered_deadline_missed = 
	(gapi_listener_OfferedDeadlineMissedListener)sacListener->on_offered_deadline_missed;
    gapiListener->on_offered_incompatible_qos = 
	(gapi_listener_OfferedIncompatibleQosListener)sacListener->on_offered_incompatible_qos;
    gapiListener->on_liveliness_lost = 
	(gapi_listener_LivelinessLostListener)sacListener->on_liveliness_lost;
    gapiListener->on_publication_match = 
	(gapi_listener_PublicationMatchedListener)sacListener->on_publication_matched;
}

void
sac_copyGapiDataReaderListener (
    const struct gapi_dataReaderListener *gapiListener,
    struct DDS_DataReaderListener        *sacListener
    )
{
    sacListener->listener_data = gapiListener->listener_data;
    sacListener->on_requested_deadline_missed = 
	(DDS_DataReaderListener_RequestedDeadlineMissedListener)gapiListener->on_requested_deadline_missed;
    sacListener->on_requested_incompatible_qos = 
	(DDS_DataReaderListener_RequestedIncompatibleQosListener)gapiListener->on_requested_incompatible_qos;
    sacListener->on_sample_rejected = 
	(DDS_DataReaderListener_SampleRejectedListener)gapiListener->on_sample_rejected;
    sacListener->on_liveliness_changed = 
	(DDS_DataReaderListener_LivelinessChangedListener)gapiListener->on_liveliness_changed;
    sacListener->on_data_available = 
	(DDS_DataReaderListener_DataAvailableListener)gapiListener->on_data_available;
    sacListener->on_subscription_matched = 
	(DDS_DataReaderListener_SubscriptionMatchedListener)gapiListener->on_subscription_match;
    sacListener->on_sample_lost = 
	(DDS_DataReaderListener_SampleLostListener)gapiListener->on_sample_lost;
}

void
sac_copySacDataReaderListener (
    const struct DDS_DataReaderListener *sacListener,
    struct gapi_dataReaderListener      *gapiListener
    )
{
    gapiListener->listener_data = sacListener->listener_data;
    gapiListener->on_requested_deadline_missed = 
	(gapi_listener_RequestedDeadlineMissedListener)sacListener->on_requested_deadline_missed;
    gapiListener->on_requested_incompatible_qos = 
	(gapi_listener_RequestedIncompatibleQosListener)sacListener->on_requested_incompatible_qos;
    gapiListener->on_sample_rejected = 
	(gapi_listener_SampleRejectedListener)sacListener->on_sample_rejected;
    gapiListener->on_liveliness_changed = 
	(gapi_listener_LivelinessChangedListener)sacListener->on_liveliness_changed;
    gapiListener->on_data_available = 
	(gapi_listener_DataAvailableListener)sacListener->on_data_available;
    gapiListener->on_subscription_match = 
	(gapi_listener_SubscriptionMatchedListener)sacListener->on_subscription_matched;
    gapiListener->on_sample_lost = 
	(gapi_listener_SampleLostListener)sacListener->on_sample_lost;
}

void
sac_copyGapiPublisherListener (
    const struct gapi_publisherListener *gapiListener,
    struct DDS_PublisherListener        *sacListener
    )
{
    sacListener->listener_data = gapiListener->listener_data;
    sacListener->on_offered_deadline_missed = 
	(DDS_PublisherListener_OfferedDeadlineMissedListener)gapiListener->on_offered_deadline_missed;
    sacListener->on_offered_incompatible_qos = 
	(DDS_PublisherListener_OfferedIncompatibleQosListener)gapiListener->on_offered_incompatible_qos;
    sacListener->on_liveliness_lost = 
	(DDS_PublisherListener_LivelinessLostListener)gapiListener->on_liveliness_lost;
    sacListener->on_publication_matched = 
	(DDS_PublisherListener_PublicationMatchedListener)gapiListener->on_publication_match;
}

void
sac_copySacPublisherListener (
    const struct DDS_PublisherListener *sacListener,
    struct gapi_publisherListener      *gapiListener
    )
{
    gapiListener->listener_data = sacListener->listener_data;
    gapiListener->on_offered_deadline_missed = 
	(gapi_listener_OfferedDeadlineMissedListener)sacListener->on_offered_deadline_missed;
    gapiListener->on_offered_incompatible_qos = 
	(gapi_listener_OfferedIncompatibleQosListener)sacListener->on_offered_incompatible_qos;
    gapiListener->on_liveliness_lost = 
	(gapi_listener_LivelinessLostListener)sacListener->on_liveliness_lost;
    gapiListener->on_publication_match = 
	(gapi_listener_PublicationMatchedListener)sacListener->on_publication_matched;
}

void
sac_copyGapiSubscriberListener (
    const struct gapi_subscriberListener *gapiListener,
    struct DDS_SubscriberListener        *sacListener
    )
{
    sacListener->listener_data = gapiListener->listener_data;
    sacListener->on_requested_deadline_missed = 
	(DDS_SubscriberListener_RequestedDeadlineMissedListener)gapiListener->on_requested_deadline_missed;
    sacListener->on_requested_incompatible_qos = 
	(DDS_SubscriberListener_RequestedIncompatibleQosListener)gapiListener->on_requested_incompatible_qos;
    sacListener->on_sample_rejected = 
	(DDS_SubscriberListener_SampleRejectedListener)gapiListener->on_sample_rejected;
    sacListener->on_liveliness_changed = 
	(DDS_SubscriberListener_LivelinessChangedListener)gapiListener->on_liveliness_changed;
    sacListener->on_data_available = 
	(DDS_SubscriberListener_DataAvailableListener)gapiListener->on_data_available;
    sacListener->on_subscription_matched = 
	(DDS_SubscriberListener_SubscriptionMatchedListener)gapiListener->on_subscription_match;
    sacListener->on_sample_lost = 
	(DDS_SubscriberListener_SampleLostListener)gapiListener->on_sample_lost;
    sacListener->on_data_on_readers = 
	(DDS_SubscriberListener_DataOnReadersListener)gapiListener->on_data_on_readers;
}

void
sac_copySacSubscriberListener (
    const struct DDS_SubscriberListener *sacListener,
    struct gapi_subscriberListener      *gapiListener
    )
{
    gapiListener->listener_data = sacListener->listener_data;
    gapiListener->on_requested_deadline_missed = 
	(gapi_listener_RequestedDeadlineMissedListener)sacListener->on_requested_deadline_missed;
    gapiListener->on_requested_incompatible_qos = 
	(gapi_listener_RequestedIncompatibleQosListener)sacListener->on_requested_incompatible_qos;
    gapiListener->on_sample_rejected = 
	(gapi_listener_SampleRejectedListener)sacListener->on_sample_rejected;
    gapiListener->on_liveliness_changed = 
	(gapi_listener_LivelinessChangedListener)sacListener->on_liveliness_changed;
    gapiListener->on_data_available = 
	(gapi_listener_DataAvailableListener)sacListener->on_data_available;
    gapiListener->on_subscription_match = 
	(gapi_listener_SubscriptionMatchedListener)sacListener->on_subscription_matched;
    gapiListener->on_sample_lost = 
	(gapi_listener_SampleLostListener)sacListener->on_sample_lost;
    gapiListener->on_data_on_readers = 
	(gapi_listener_DataOnReadersListener)sacListener->on_data_on_readers;
}

void
sac_copyGapiTopicListener (
    const struct gapi_topicListener *gapiListener,
    struct DDS_TopicListener        *sacListener
    )
{
    sacListener->listener_data = gapiListener->listener_data;
    sacListener->on_inconsistent_topic = 
	(DDS_TopicListener_InconsistentTopicListener)gapiListener->on_inconsistent_topic;
#ifdef _DDS1631API_
    sacListener->on_all_data_disposed = 
	(DDS_TopicListener_AllDataDisposedListener)gapiListener->on_all_data_disposed;
#endif
}

void
sac_copySacTopicListener (
    const struct DDS_TopicListener *sacListener,
    struct gapi_topicListener *gapiListener
    )
{
    gapiListener->listener_data = sacListener->listener_data;
    gapiListener->on_inconsistent_topic = 
	(gapi_listener_InconsistentTopicListener)sacListener->on_inconsistent_topic;
#ifdef _DDS1631API_
    gapiListener->on_all_data_disposed = 
	(gapi_listener_AllDataDisposedListener)sacListener->on_all_data_disposed;
#endif
}

void
sac_copyGapiDomainParticipantListener (
    const struct gapi_domainParticipantListener *gapiListener,
    struct DDS_DomainParticipantListener        *sacListener
    )
{
    sacListener->listener_data = gapiListener->listener_data;
    sacListener->on_inconsistent_topic = 
	(DDS_DomainParticipantListener_InconsistentTopicListener)gapiListener->on_inconsistent_topic;
    sacListener->on_requested_deadline_missed = 
	(DDS_DomainParticipantListener_RequestedDeadlineMissedListener)gapiListener->on_requested_deadline_missed;
    sacListener->on_requested_incompatible_qos = 
	(DDS_DomainParticipantListener_RequestedIncompatibleQosListener)gapiListener->on_requested_incompatible_qos;
    sacListener->on_sample_rejected = 
	(DDS_DomainParticipantListener_SampleRejectedListener)gapiListener->on_sample_rejected;
    sacListener->on_liveliness_changed = 
	(DDS_DomainParticipantListener_LivelinessChangedListener)gapiListener->on_liveliness_changed;
    sacListener->on_data_available = 
	(DDS_DomainParticipantListener_DataAvailableListener)gapiListener->on_data_available;
    sacListener->on_subscription_matched = 
	(DDS_DomainParticipantListener_SubscriptionMatchedListener)gapiListener->on_subscription_match;
    sacListener->on_sample_lost = 
	(DDS_DomainParticipantListener_SampleLostListener)gapiListener->on_sample_lost;
    sacListener->on_data_on_readers = 
	(DDS_DomainParticipantListener_DataOnReadersListener)gapiListener->on_data_on_readers;
    sacListener->on_offered_deadline_missed = 
	(DDS_DomainParticipantListener_OfferedDeadlineMissedListener)gapiListener->on_offered_deadline_missed;
    sacListener->on_offered_incompatible_qos = 
	(DDS_DomainParticipantListener_OfferedIncompatibleQosListener)gapiListener->on_offered_incompatible_qos;
    sacListener->on_liveliness_lost = 
	(DDS_DomainParticipantListener_LivelinessLostListener)gapiListener->on_liveliness_lost;
    sacListener->on_publication_matched = 
	(DDS_DomainParticipantListener_PublicationMatchedListener)gapiListener->on_publication_match;
}

void
sac_copySacDomainParticipantListener (
    const struct DDS_DomainParticipantListener *sacListener,
    struct gapi_domainParticipantListener      *gapiListener
    )
{
    gapiListener->listener_data = sacListener->listener_data;
    gapiListener->on_inconsistent_topic = 
	(gapi_listener_InconsistentTopicListener)sacListener->on_inconsistent_topic;
    gapiListener->on_requested_deadline_missed = 
	(gapi_listener_RequestedDeadlineMissedListener)sacListener->on_requested_deadline_missed;
    gapiListener->on_requested_incompatible_qos = 
	(gapi_listener_RequestedIncompatibleQosListener)sacListener->on_requested_incompatible_qos;
    gapiListener->on_sample_rejected = 
	(gapi_listener_SampleRejectedListener)sacListener->on_sample_rejected;
    gapiListener->on_liveliness_changed = 
	(gapi_listener_LivelinessChangedListener)sacListener->on_liveliness_changed;
    gapiListener->on_data_available = 
	(gapi_listener_DataAvailableListener)sacListener->on_data_available;
    gapiListener->on_subscription_match = 
	(gapi_listener_SubscriptionMatchedListener)sacListener->on_subscription_matched;
    gapiListener->on_sample_lost = 
	(gapi_listener_SampleLostListener)sacListener->on_sample_lost;
    gapiListener->on_data_on_readers = 
	(gapi_listener_DataOnReadersListener)sacListener->on_data_on_readers;
    gapiListener->on_offered_deadline_missed = 
	(gapi_listener_OfferedDeadlineMissedListener)sacListener->on_offered_deadline_missed;
    gapiListener->on_offered_incompatible_qos = 
	(gapi_listener_OfferedIncompatibleQosListener)sacListener->on_offered_incompatible_qos;
    gapiListener->on_liveliness_lost = 
	(gapi_listener_LivelinessLostListener)sacListener->on_liveliness_lost;
    gapiListener->on_publication_match = 
	(gapi_listener_PublicationMatchedListener)sacListener->on_publication_matched;
}

void
sac_copyGapiInconsistentTopicStatus (
    gapi_inconsistentTopicStatus *s,
    DDS_InconsistentTopicStatus *d
    )
{
    d->total_count = (DDS_long)s->total_count;
    d->total_count_change = (DDS_long)s->total_count_change;
}

void
sac_copySacInconsistentTopicStatus (
    DDS_InconsistentTopicStatus *s,
    gapi_inconsistentTopicStatus *d
    )
{
    d->total_count = (gapi_long)s->total_count;
    d->total_count_change = (gapi_long)s->total_count_change;
}

void
sac_copyGapiSampleLostStatus (
    gapi_sampleLostStatus *s,
    DDS_SampleLostStatus *d
    )
{
    d->total_count = (DDS_long)s->total_count;
    d->total_count_change = (DDS_long)s->total_count_change;
}

void
sac_copySacSampleLostStatus (
    DDS_SampleLostStatus *s,
    gapi_sampleLostStatus *d
    )
{
    d->total_count = (gapi_long)s->total_count;
    d->total_count_change = (gapi_long)s->total_count_change;
}

void
sac_copyGapiSampleRejectedStatus (
    gapi_sampleRejectedStatus *s,
    DDS_SampleRejectedStatus *d
    )
{
    d->total_count = (DDS_long)s->total_count;
    d->total_count_change = (DDS_long)s->total_count_change;
    d->last_reason = (DDS_SampleRejectedStatusKind)s->last_reason;
    d->last_instance_handle = (DDS_InstanceHandle_t)s->last_instance_handle;
}

void
sac_copySacSampleRejectedStatus (
    DDS_SampleRejectedStatus *s,
    gapi_sampleRejectedStatus *d
    )
{
    d->total_count = (gapi_long)s->total_count;
    d->total_count_change = (gapi_long)s->total_count_change;
    d->last_reason = (gapi_sampleRejectedStatusKind)s->last_reason;
    d->last_instance_handle = (gapi_instanceHandle_t)s->last_instance_handle;
}

void
sac_copyGapiLivelinessLostStatus (
    gapi_livelinessLostStatus *s,
    DDS_LivelinessLostStatus *d
    )
{
    d->total_count = (DDS_long)s->total_count;
    d->total_count_change = (DDS_long)s->total_count_change;
}

void
sac_copySacLivelinessLostStatus (
    DDS_LivelinessLostStatus *s,
    gapi_livelinessLostStatus *d
    )
{
    d->total_count = (gapi_long)s->total_count;
    d->total_count_change = (gapi_long)s->total_count_change;
}

void
sac_copyGapiLivelinessChangedStatus (
    gapi_livelinessChangedStatus *s,
    DDS_LivelinessChangedStatus *d
    )
{
    d->alive_count = (DDS_long)s->alive_count;
    d->not_alive_count = (DDS_long)s->not_alive_count;
    d->alive_count_change = (DDS_long)s->alive_count_change;
    d->not_alive_count_change = (DDS_long)s->not_alive_count_change;
    d->last_publication_handle = (DDS_InstanceHandle_t)s->last_publication_handle;
}

void
sac_copySacLivelinessChangedStatus (
    DDS_LivelinessChangedStatus *s,
    gapi_livelinessChangedStatus *d
    )
{
    d->alive_count = (gapi_long)s->alive_count;
    d->not_alive_count = (gapi_long)s->not_alive_count;
    d->alive_count_change = (gapi_long)s->alive_count_change;
    d->not_alive_count_change = (gapi_long)s->not_alive_count_change;
    d->last_publication_handle = (gapi_instanceHandle_t)s->last_publication_handle;
}

void
sac_copyGapiRequestedDeadlineMissedStatus (
    gapi_requestedDeadlineMissedStatus *s,
    DDS_RequestedDeadlineMissedStatus *d
    )
{
    d->total_count = (DDS_long)s->total_count;
    d->total_count_change = (DDS_long)s->total_count_change;
    d->last_instance_handle = (DDS_InstanceHandle_t)s->last_instance_handle;
}

void
sac_copySacRequestedDeadlineMissedStatus (
    DDS_RequestedDeadlineMissedStatus *s,
    gapi_requestedDeadlineMissedStatus *d
    )
{
    d->total_count = (gapi_long)s->total_count;
    d->total_count_change = (gapi_long)s->total_count_change;
    d->last_instance_handle = (gapi_instanceHandle_t)s->last_instance_handle;
}

void
sac_copyGapiOfferedIncompatibleQosStatus (
    gapi_offeredIncompatibleQosStatus *s,
    DDS_OfferedIncompatibleQosStatus *d
    )
{
    d->total_count = (DDS_long)s->total_count;
    d->total_count_change = (DDS_long)s->total_count_change;
    d->last_policy_id = (DDS_QosPolicyId_t)s->last_policy_id;
    DDS_QosPolicyCountSeqCopyin (&s->policies, &d->policies);
}

void
sac_copySacOfferedIncompatibleQosStatus (
    DDS_OfferedIncompatibleQosStatus *s,
    gapi_offeredIncompatibleQosStatus *d
    )
{
    d->total_count = (gapi_long)s->total_count;
    d->total_count_change = (gapi_long)s->total_count_change;
    d->last_policy_id = (gapi_qosPolicyId_t)s->last_policy_id;
    DDS_QosPolicyCountSeqCopyout (&s->policies, &d->policies);
}

void
sac_copyGapiRequestedIncompatibleQosStatus (
    gapi_requestedIncompatibleQosStatus *s,
    DDS_RequestedIncompatibleQosStatus *d
    )
{
    d->total_count = (DDS_long)s->total_count;
    d->total_count_change = (DDS_long)s->total_count_change;
    d->last_policy_id = (DDS_QosPolicyId_t)s->last_policy_id;
    DDS_QosPolicyCountSeqCopyin (&s->policies, &d->policies);
}

void
sac_copySacRequestedIncompatibleQosStatus (
    DDS_RequestedIncompatibleQosStatus *s,
    gapi_requestedIncompatibleQosStatus *d
    )
{
    d->total_count = (gapi_long)s->total_count;
    d->total_count_change = (gapi_long)s->total_count_change;
    d->last_policy_id = (gapi_qosPolicyId_t)s->last_policy_id;
    DDS_QosPolicyCountSeqCopyout (&s->policies, &d->policies);
}

void
sac_copyGapiPublicationMatchStatus (
    gapi_publicationMatchedStatus *s,
    DDS_PublicationMatchedStatus *d
    )
{
    d->total_count = (DDS_long)s->total_count;
    d->total_count_change = (DDS_long)s->total_count_change;
    d->last_subscription_handle = (DDS_InstanceHandle_t)s->last_subscription_handle;
}

void
sac_copySacPublicationMatchStatus (
    DDS_PublicationMatchedStatus *s,
    gapi_publicationMatchedStatus *d
    )
{
    d->total_count = (gapi_long)s->total_count;
    d->total_count_change = (gapi_long)s->total_count_change;
    d->last_subscription_handle = (gapi_instanceHandle_t)s->last_subscription_handle;
}

void
sac_copyGapiSubscriptionMatchStatus (
    gapi_subscriptionMatchedStatus *s,
    DDS_SubscriptionMatchedStatus *d
    )
{
    d->total_count = (DDS_long)s->total_count;
    d->total_count_change = (DDS_long)s->total_count_change;
    d->last_publication_handle = (DDS_InstanceHandle_t)s->last_publication_handle;
}

void
sac_copySacSubscriptionMatchStatus (
    DDS_SubscriptionMatchedStatus *s,
    gapi_subscriptionMatchedStatus *d
    )
{
    d->total_count = (gapi_long)s->total_count;
    d->total_count_change = (gapi_long)s->total_count_change;
    d->last_publication_handle = (gapi_instanceHandle_t)s->last_publication_handle;
}
