/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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

#include <dds_dcps.h>
#include <dds_dcps_private.h>
#include <dds.h>
#include <dds_report.h>
#include <dds__qos.h>
#include <os_atomics.h>

struct DomainParticipantInfo {
    struct DDS_EntityUserData_s _parent;
    dds_participantlistener_t *listener;
};


static void
on_inconsistent_topic (
    void *listener_data,
    DDS_Topic topic,
    const DDS_InconsistentTopicStatus *status)
{
    struct DomainParticipantInfo *info = listener_data;
    dds_participantlistener_t *listener = NULL;

    if (info) {
        listener = info->listener;
    }

    if (listener && status && listener->topiclistener.on_inconsistent_topic) {
        dds_inconsistent_topic_status_t s;
        s.total_count = status->total_count;
        s.total_count_change = status->total_count_change;
        listener->topiclistener.on_inconsistent_topic(topic, &s);
    }
}

static void
on_offered_deadline_missed (
    void *listener_data,
    DDS_DataWriter writer,
    const DDS_OfferedDeadlineMissedStatus *status)
{
    struct DomainParticipantInfo *info = listener_data;
    dds_participantlistener_t *listener = NULL;

    if (info) {
        listener = info->listener;
    }

    if (listener && status && listener->publisherlistener.writerlistener.on_offered_deadline_missed) {
        dds_offered_deadline_missed_status_t s;
        s.total_count = status->total_count;
        s.total_count_change = status->total_count_change;
        s.last_instance_handle = status->last_instance_handle;
        listener->publisherlistener.writerlistener.on_offered_deadline_missed(writer, &s);
    }
}

static void
on_offered_incompatible_qos (
    void *listener_data,
    DDS_DataWriter writer,
    const DDS_OfferedIncompatibleQosStatus *status)
{
    struct DomainParticipantInfo *info = listener_data;
    dds_participantlistener_t *listener = NULL;

    if (info) {
        listener = info->listener;
    }

    if (listener && status && listener->publisherlistener.writerlistener.on_offered_incompatible_qos) {
        dds_offered_incompatible_qos_status_t s;
        s.total_count = status->total_count;
        s.total_count_change = status->total_count_change;
        s.last_policy_id = status->last_policy_id;
        listener->publisherlistener.writerlistener.on_offered_incompatible_qos(writer, &s);
    }
}

static void
on_liveliness_lost (
    void *listener_data,
    DDS_DataWriter writer,
    const DDS_LivelinessLostStatus *status)
{
    struct DomainParticipantInfo *info = listener_data;
    dds_participantlistener_t *listener = NULL;

    if (info) {
        listener = info->listener;
    }

    if (listener && status && listener->publisherlistener.writerlistener.on_liveliness_lost) {
        dds_liveliness_lost_status_t s;
        s.total_count = status->total_count;
        s.total_count_change = status->total_count_change;
        listener->publisherlistener.writerlistener.on_liveliness_lost(writer, &s);
    }
}

static void
on_publication_match (
    void *listener_data,
    DDS_DataWriter writer,
    const DDS_PublicationMatchedStatus *status)
{
    struct DomainParticipantInfo *info = listener_data;
    dds_participantlistener_t *listener = NULL;

    if (info) {
        listener = info->listener;
    }

    if (listener && status && listener->publisherlistener.writerlistener.on_publication_matched) {
        dds_publication_matched_status_t s;
        s.total_count = status->total_count;
        s.total_count_change = status->total_count_change;
        s.current_count = status->current_count;
        s.current_count_change = status->current_count_change;
        s.last_subscription_handle = status->last_subscription_handle;
        listener->publisherlistener.writerlistener.on_publication_matched(writer, &s);
    }
}

static void
on_requested_deadline_missed (
    void *listener_data,
    DDS_DataReader reader,
    const DDS_RequestedDeadlineMissedStatus *status)
{
    struct DomainParticipantInfo *info = listener_data;
    dds_participantlistener_t *listener = NULL;

    if (info) {
        listener = info->listener;
    }

    if (listener && status && listener->subscriberlistener.readerlistener.on_requested_deadline_missed) {
        dds_requested_deadline_missed_status_t s;
        s.total_count = status->total_count;
        s.total_count_change = status->total_count_change;
        s.last_instance_handle = status->last_instance_handle;
        listener->subscriberlistener.readerlistener.on_requested_deadline_missed(reader, &s);
    }
}

static void
on_requested_incompatible_qos (
    void *listener_data,
    DDS_DataReader reader,
    const DDS_RequestedIncompatibleQosStatus *status)
{
    struct DomainParticipantInfo *info = listener_data;
    dds_participantlistener_t *listener = NULL;

    if (info) {
        listener = info->listener;
    }

    if (listener && status && listener->subscriberlistener.readerlistener.on_requested_incompatible_qos) {
        dds_requested_incompatible_qos_status_t s;
        s.total_count = status->total_count;
        s.total_count_change = status->total_count_change;
        s.last_policy_id = status->last_policy_id;
        listener->subscriberlistener.readerlistener.on_requested_incompatible_qos(reader, &s);
    }
}

static void
on_sample_rejected (
    void *listener_data,
    DDS_DataReader reader,
    const DDS_SampleRejectedStatus *status)
{
    struct DomainParticipantInfo *info = listener_data;
    dds_participantlistener_t *listener = NULL;

    if (info) {
        listener = info->listener;
    }

    if (listener && status && listener->subscriberlistener.readerlistener.on_sample_rejected) {
        dds_sample_rejected_status_t s;
        s.total_count = status->total_count;
        s.total_count_change = status->total_count_change;
        s.last_reason = status->last_reason;
        s.last_instance_handle = status->last_instance_handle;
        listener->subscriberlistener.readerlistener.on_sample_rejected(reader, &s);
    }
}

static void
on_liveliness_changed (
    void *listener_data,
    DDS_DataReader reader,
    const DDS_LivelinessChangedStatus *status)
{
    struct DomainParticipantInfo *info = listener_data;
    dds_participantlistener_t *listener = NULL;

    if (info) {
        listener = info->listener;
    }

    if (listener && status && listener->subscriberlistener.readerlistener.on_liveliness_changed) {
        dds_liveliness_changed_status_t s;
        s.alive_count = status->alive_count;
        s.alive_count_change = status->alive_count_change;
        s.last_publication_handle = status->last_publication_handle;
        s.not_alive_count = status->not_alive_count;
        s.not_alive_count_change = status->not_alive_count_change;
        listener->subscriberlistener.readerlistener.on_liveliness_changed(reader, &s);
    }
}

static void
on_data_available (
    void *listener_data,
    DDS_DataReader reader)
{
    struct DomainParticipantInfo *info = listener_data;
    dds_participantlistener_t *listener = NULL;

    if (info) {
        listener = info->listener;
    }

    if (listener && listener->subscriberlistener.readerlistener.on_data_available) {
        listener->subscriberlistener.readerlistener.on_data_available(reader);
    }
}

static void
on_subscription_match (
    void *listener_data,
    DDS_DataReader reader,
    const DDS_SubscriptionMatchedStatus *status)
{
    struct DomainParticipantInfo *info = listener_data;
    dds_participantlistener_t *listener = NULL;

    if (info) {
        listener = info->listener;
    }

    if (listener && status && listener->subscriberlistener.readerlistener.on_subscription_matched) {
        dds_subscription_matched_status_t s;
        s.total_count = status->total_count;
        s.total_count_change = status->total_count_change;
        s.current_count = status->current_count;
        s.current_count_change = status->current_count_change;
        s.last_publication_handle = status->last_publication_handle;
        listener->subscriberlistener.readerlistener.on_subscription_matched(reader, &s);
    }
}

static void
on_sample_lost (
    void *listener_data,
    DDS_DataReader reader,
    const DDS_SampleLostStatus *status)
{
    struct DomainParticipantInfo *info = listener_data;
    dds_participantlistener_t *listener = NULL;

    if (info) {
        listener = info->listener;
    }

    if (listener && status && listener->subscriberlistener.readerlistener.on_sample_lost) {
        dds_sample_lost_status_t s;
        s.total_count = status->total_count;
        s.total_count_change = status->total_count_change;
        listener->subscriberlistener.readerlistener.on_sample_lost(reader, &s);
    }
}

static void
on_data_on_readers (
    void *listener_data,
    DDS_Subscriber subscriber)
{
    struct DomainParticipantInfo *info = listener_data;
    dds_participantlistener_t *listener = NULL;

    if (info) {
        listener = info->listener;
    }

    if (listener && listener->subscriberlistener.on_data_readers) {
        listener->subscriberlistener.on_data_readers(subscriber);
    }
}


static void
dds_participant_info_free(
    DDS_EntityUserData data)
{
    struct DomainParticipantInfo *info = (struct DomainParticipantInfo *)data;
    if (info) {
        if (info->listener) {
            os_free(info->listener);
        }
        os_free(info);
    }
}

static struct DomainParticipantInfo *
dds_participant_info_new(
    void)
{
    struct DomainParticipantInfo *info;

    info = os_malloc(sizeof(*info));
    DDS_Entity_user_data_init((DDS_EntityUserData)info, dds_participant_info_free);
    info->listener = NULL;

    return info;
}

static void
dds_participant_listener_init(
    struct DDS_DomainParticipantListener *listener,
    void *data)
{
    listener->listener_data = data;
    listener->on_inconsistent_topic = on_inconsistent_topic;
    listener->on_requested_deadline_missed = on_requested_deadline_missed;
    listener->on_requested_incompatible_qos = on_requested_incompatible_qos;
    listener->on_sample_rejected = on_sample_rejected;
    listener->on_liveliness_changed = on_liveliness_changed;
    listener->on_data_available = on_data_available;
    listener->on_subscription_matched = on_subscription_match;
    listener->on_sample_lost = on_sample_lost;
    listener->on_data_on_readers = on_data_on_readers;
    listener->on_offered_deadline_missed = on_offered_deadline_missed;
    listener->on_offered_incompatible_qos = on_offered_incompatible_qos;
    listener->on_liveliness_lost = on_liveliness_lost;
    listener->on_publication_matched = on_publication_match;
}


dds_domainid_t
dds_domain_default (void)
{
    return DDS_DOMAIN_ID_DEFAULT;
}



int
dds_participant_create (
    dds_entity_t * pp,
    const dds_domainid_t domain,
    const dds_qos_t * qos,
    const dds_participantlistener_t * listener)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    DDS_DomainParticipantFactory factory;
    struct DomainParticipantInfo *info;
    struct DDS_DomainParticipantListener dpl;
    struct DDS_DomainParticipantListener *lp = NULL;
    DDS_StatusMask mask = (listener) ? DDS_STATUS_MASK_ANY : 0;
    DDS_DomainParticipantQos *pQos;

    DDS_REPORT_STACK();

    if (pp) {
        info = dds_participant_info_new();

        if (listener) {
            info->listener = os_malloc(sizeof(dds_participantlistener_t));
            *info->listener = *listener;
            lp = &dpl;
            dds_participant_listener_init(&dpl, info);
        }

        factory = DDS_DomainParticipantFactory_get_instance();
        if (factory) {
            if (qos) {
                pQos = DDS_DomainParticipantQos__alloc();
                result = DDS_DomainParticipantFactory_get_default_participant_qos(factory, pQos);
                if (result == DDS_RETCODE_OK) {
                    dds_qos_to_participant_qos(pQos, qos);
                    *pp = DDS_DomainParticipantFactory_create_participant(factory, domain, pQos, lp, mask);
                }
                DDS_free(pQos);
            } else {
                *pp = DDS_DomainParticipantFactory_create_participant(factory, domain, DDS_PARTICIPANT_QOS_DEFAULT, lp, mask);
            }
            if (*pp) {
                result = DDS_Entity_set_user_data(*pp, (DDS_EntityUserData)info);
            } else {
                result = dds_report_get_error_code();
            }
        } else {
            result = dds_report_get_error_code();
        }
        DDS_Entity_release_user_data((DDS_EntityUserData)info);
    } else {
        result =  DDS_RETCODE_BAD_PARAMETER;
        DDS_REPORT(result, "Entity parameter is NULL.");
    }

    DDS_REPORT_FLUSH(NULL, result != DDS_RETCODE_OK);

    return DDS_ERRNO(result, DDS_MOD_KERNEL, DDS_ERR_Mx);
}

dds_entity_t
dds_participant_get (
    dds_entity_t entity)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    DDS_DomainParticipant participant = NULL;
    DDS_Publisher publisher;
    DDS_Subscriber subscriber;

    DDS_REPORT_STACK();

    if (entity) {
        switch (DDS_Entity_get_kind(entity)) {
        case DDS_ENTITY_KIND_DOMAINPARTICIPANT:
            participant = entity;
            break;
        case DDS_ENTITY_KIND_TOPIC:
            participant = DDS_Topic_get_participant(entity);
            break;
        case DDS_ENTITY_KIND_PUBLISHER:
            participant = DDS_Publisher_get_participant(entity);
            break;
        case DDS_ENTITY_KIND_SUBSCRIBER:
            participant = DDS_Subscriber_get_participant(entity);
            break;
        case DDS_ENTITY_KIND_DATAWRITER:
            publisher = DDS_DataWriter_get_publisher(entity);
            if (publisher) {
                participant = DDS_Publisher_get_participant(publisher);
            }
            break;
        case DDS_ENTITY_KIND_DATAREADER:
            subscriber = DDS_DataReader_get_subscriber(entity);
            if (subscriber) {
                participant = DDS_Subscriber_get_participant(subscriber);
            }
            break;
        default:
            result = DDS_RETCODE_BAD_PARAMETER;
            DDS_REPORT(result, "Entity parameter is not a valid dds entity.");
            break;
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        DDS_REPORT(result, "Entity parameter is NULL.");
    }

    if (!participant && result == DDS_RETCODE_OK) {
        result = dds_report_get_error_code();
    }

    DDS_REPORT_FLUSH(entity, result != DDS_RETCODE_OK);

    return participant;
}

dds_domainid_t
dds_participant_get_domain_id (
    dds_entity_t pp)
{
    return (dds_domainid_t)DDS_DomainParticipant_get_domain_id(pp);
}

dds_entity_t
dds_participant_lookup (
    dds_domainid_t domain_id)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    DDS_DomainParticipantFactory factory;
    DDS_DomainParticipant participant = NULL;

    DDS_REPORT_STACK();

    factory = DDS_DomainParticipantFactory_get_instance();
    if (factory) {
        participant = DDS_DomainParticipantFactory_lookup_participant(factory, domain_id);
    } else {
        result = DDS_RETCODE_ERROR;
        DDS_REPORT(result, "Failed to retrieve the DomainParticpantFactory.");
    }

    DDS_REPORT_FLUSH(NULL, result != DDS_RETCODE_OK);

    return participant;
}

int
dds_domainparticipant_get_listener(
    dds_entity_t e,
    dds_participantlistener_t *listener)
{
    DDS_ReturnCode_t result = DDS_RETCODE_BAD_PARAMETER;
    struct DomainParticipantInfo *info = NULL;

    DDS_REPORT_STACK();

    if (listener) {
        result = DDS_Entity_claim_user_data(e, (DDS_EntityUserData *)&info);
        if (result == DDS_RETCODE_OK) {
            if (info->listener) {
                *listener = *info->listener;
            }
            DDS_Entity_release_user_data((DDS_EntityUserData)info);
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        DDS_REPORT(result, "The listener parameter is NULL.");
    }

    DDS_REPORT_FLUSH(e, result != DDS_RETCODE_OK);

    return DDS_ERRNO(result, DDS_MOD_KERNEL, DDS_ERR_Mx);
}

int
dds_domainparticipant_set_listener(
    dds_entity_t e,
    const dds_participantlistener_t *listener)
{
    DDS_ReturnCode_t result;
    struct DomainParticipantInfo *info = NULL;
    struct DDS_DomainParticipantListener dpl;
    dds_participantlistener_t *newListener;
    dds_participantlistener_t *oldListener;
    DDS_StatusMask mask;

    DDS_REPORT_STACK();

    result = DDS_Entity_claim_user_data(e, (DDS_EntityUserData *)&info);
    if (result == DDS_RETCODE_OK) {
        oldListener = info->listener;
        if (listener) {
            newListener = os_malloc(sizeof(dds_participantlistener_t));
            *newListener = *listener;
            mask = dds_status_get_enabled(e);
            info->listener = newListener;
            dds_participant_listener_init(&dpl, info);
        } else {
            mask = 0;
            info->listener = NULL;
        }
        result = DDS_DomainParticipant_set_listener(e, &dpl, mask);
        DDS_Entity_release_user_data((DDS_EntityUserData)info);
        os_free(oldListener);
    }

    DDS_REPORT_FLUSH(e, result != DDS_RETCODE_OK);

    return DDS_ERRNO(result, DDS_MOD_KERNEL, DDS_ERR_Mx);
}

int
dds_domainparticipant_delete(
    dds_entity_t pp)
{
    int result;

    result = DDS_DomainParticipant_delete_contained_entities(pp);
    if (result == DDS_RETCODE_OK) {
        DDS_DomainParticipantFactory factory = DDS_DomainParticipantFactory_get_instance();
        result = DDS_DomainParticipantFactory_delete_participant(factory, pp);
    }

    return DDS_ERRNO(result, DDS_MOD_KERNEL, DDS_ERR_Mx);
}
