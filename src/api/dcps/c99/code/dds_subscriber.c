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

struct SubscriberInfo {
    struct DDS_EntityUserData_s _parent;
    dds_subscriberlistener_t *listener;
};


static void
on_requested_deadline_missed (
    void *listener_data,
    DDS_DataReader reader,
    const DDS_RequestedDeadlineMissedStatus *status)
{
    struct SubscriberInfo *info = listener_data;
    dds_subscriberlistener_t *listener = NULL;

    if (info) {
        listener = info->listener;
    }

    if (listener && status && listener->readerlistener.on_requested_deadline_missed) {
        dds_requested_deadline_missed_status_t s;
        s.total_count = status->total_count;
        s.total_count_change = status->total_count_change;
        s.last_instance_handle = status->last_instance_handle;
        listener->readerlistener.on_requested_deadline_missed(reader, &s);
    }
}

static void
on_requested_incompatible_qos (
    void *listener_data,
    DDS_DataReader reader,
    const DDS_RequestedIncompatibleQosStatus *status)
{
    struct SubscriberInfo *info = listener_data;
    dds_subscriberlistener_t *listener = NULL;

    if (info) {
        listener = info->listener;
    }

    if (listener && status && listener->readerlistener.on_requested_incompatible_qos) {
        dds_requested_incompatible_qos_status_t s;
        s.total_count = status->total_count;
        s.total_count_change = status->total_count_change;
        s.last_policy_id = status->last_policy_id;
        listener->readerlistener.on_requested_incompatible_qos(reader, &s);
    }
}

static void
on_sample_rejected (
    void *listener_data,
    DDS_DataReader reader,
    const DDS_SampleRejectedStatus *status)
{
    struct SubscriberInfo *info = listener_data;
    dds_subscriberlistener_t *listener = NULL;

    if (info) {
        listener = info->listener;
    }

    if (listener && status && listener->readerlistener.on_sample_rejected) {
        dds_sample_rejected_status_t s;
        s.total_count = status->total_count;
        s.total_count_change = status->total_count_change;
        s.last_reason = status->last_reason;
        s.last_instance_handle = status->last_instance_handle;
        listener->readerlistener.on_sample_rejected(reader, &s);
    }
}

static void
on_liveliness_changed (
    void *listener_data,
    DDS_DataReader reader,
    const DDS_LivelinessChangedStatus *status)
{
    struct SubscriberInfo *info = listener_data;
    dds_subscriberlistener_t *listener = NULL;

    if (info) {
        listener = info->listener;
    }

    if (listener && status && listener->readerlistener.on_liveliness_changed) {
        dds_liveliness_changed_status_t s;
        s.alive_count = status->alive_count;
        s.alive_count_change = status->alive_count_change;
        s.last_publication_handle = status->last_publication_handle;
        s.not_alive_count = status->not_alive_count;
        s.not_alive_count_change = status->not_alive_count_change;
        listener->readerlistener.on_liveliness_changed(reader, &s);
    }
}

static void
on_data_available (
    void *listener_data,
    DDS_DataReader reader)
{
    struct SubscriberInfo *info = listener_data;
    dds_subscriberlistener_t *listener = NULL;

    if (info) {
        listener = info->listener;
    }

    if (listener && listener->readerlistener.on_data_available) {
        listener->readerlistener.on_data_available(reader);
    }
}

static void
on_subscription_match (
    void *listener_data,
    DDS_DataReader reader,
    const DDS_SubscriptionMatchedStatus *status)
{
    struct SubscriberInfo *info = listener_data;
    dds_subscriberlistener_t *listener = NULL;

    if (info) {
        listener = info->listener;
    }

    if (listener && status && listener->readerlistener.on_subscription_matched) {
        dds_subscription_matched_status_t s;
        s.total_count = status->total_count;
        s.total_count_change = status->total_count_change;
        s.current_count = status->current_count;
        s.current_count_change = status->current_count_change;
        s.last_publication_handle = status->last_publication_handle;
        listener->readerlistener.on_subscription_matched(reader, &s);
    }
}

static void
on_sample_lost (
    void *listener_data,
    DDS_DataReader reader,
    const DDS_SampleLostStatus *status)
{
    struct SubscriberInfo *info = listener_data;
    dds_subscriberlistener_t *listener = NULL;

    if (info) {
        listener = info->listener;
    }

    if (listener && status && listener->readerlistener.on_sample_lost) {
        dds_sample_lost_status_t s;
        s.total_count = status->total_count;
        s.total_count_change = status->total_count_change;
        listener->readerlistener.on_sample_lost(reader, &s);
    }
}

static void
on_data_on_readers (
    void *listener_data,
    DDS_Subscriber subscriber)
{
    struct SubscriberInfo *info = listener_data;
    dds_subscriberlistener_t *listener = NULL;

    if (info) {
        listener = info->listener;
    }

    if (listener && listener->on_data_readers) {
        listener->on_data_readers(subscriber);
    }
}


static void
dds_subscriber_info_free(
    DDS_EntityUserData data)
{
    struct SubscriberInfo *info = (struct SubscriberInfo *)data;
    if (info) {
        if (info->listener) {
            os_free(info->listener);
        }
        os_free(info);
    }
}

static struct SubscriberInfo *
dds_subscriber_info_new(
    void)
{
    struct SubscriberInfo *info;

    info = os_malloc(sizeof(*info));
    DDS_Entity_user_data_init((DDS_EntityUserData)info, dds_subscriber_info_free);
    info->listener = NULL;

    return info;
}

static void
dds_subscriber_listener_init(
    struct DDS_SubscriberListener *listener,
    void *data)
{
    listener->listener_data = data;
    listener->on_requested_deadline_missed = on_requested_deadline_missed;
    listener->on_requested_incompatible_qos = on_requested_incompatible_qos;
    listener->on_sample_rejected = on_sample_rejected;
    listener->on_liveliness_changed = on_liveliness_changed;
    listener->on_data_available = on_data_available;
    listener->on_subscription_matched = on_subscription_match;
    listener->on_sample_lost = on_sample_lost;
    listener->on_data_on_readers = on_data_on_readers;
}





int
dds_subscriber_create(
    dds_entity_t pp,
    dds_entity_t * subscriber,
    const dds_qos_t * qos,
    const dds_subscriberlistener_t * listener)
{
    DDS_ReturnCode_t result;
    struct SubscriberInfo *info;
    struct DDS_SubscriberListener dpl;
    struct DDS_SubscriberListener *lp = NULL;
    DDS_StatusMask mask = (listener) ? DDS_STATUS_MASK_ANY : 0;
    DDS_SubscriberQos *sQos;

    DDS_REPORT_STACK();

    if (pp && subscriber) {
        *subscriber = NULL;

        info = dds_subscriber_info_new();

        if (listener) {
            info->listener = os_malloc(sizeof(dds_subscriberlistener_t));
            *info->listener = *listener;
            lp = &dpl;
            dds_subscriber_listener_init(&dpl, info);
        }

        if (qos) {
            sQos = DDS_SubscriberQos__alloc();
            result = DDS_DomainParticipant_get_default_subscriber_qos(pp, sQos);
            if (result == DDS_RETCODE_OK) {
                dds_qos_to_subscriber_qos(sQos, qos);
                *subscriber = DDS_DomainParticipant_create_subscriber(pp, sQos, lp, mask);
            }
            DDS_free(sQos);
        } else {
            *subscriber = DDS_DomainParticipant_create_subscriber(pp, DDS_SUBSCRIBER_QOS_DEFAULT, lp, mask);
        }
        if (*subscriber) {
            result = DDS_Entity_set_user_data(*subscriber, (DDS_EntityUserData)info);
        } else {
            result = dds_report_get_error_code();
        }
        DDS_Entity_release_user_data((DDS_EntityUserData)info);
    } else {
        return DDS_RETCODE_BAD_PARAMETER;
        DDS_REPORT(result, "DomainParticipant or subscriber parameter is NULL.");
    }

    DDS_REPORT_FLUSH(pp, result != DDS_RETCODE_OK);

    return DDS_ERRNO(result, DDS_MOD_KERNEL, DDS_ERR_Mx);
}

int
dds_subscriber_get_listener(
    dds_entity_t e,
    dds_subscriberlistener_t *listener)
{
    DDS_ReturnCode_t result;
    struct SubscriberInfo *info = NULL;

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
dds_subscriber_set_listener(
    dds_entity_t e,
    const dds_subscriberlistener_t *listener)
{
    DDS_ReturnCode_t result               = DDS_RETCODE_ERROR;
    struct SubscriberInfo *info           = NULL;
    struct DDS_SubscriberListener dpl     = {0};
    dds_subscriberlistener_t *newListener = NULL;
    dds_subscriberlistener_t *oldListener = NULL;
    DDS_StatusMask mask                   = 0;

    DDS_REPORT_STACK();

    result = DDS_Entity_claim_user_data(e, (DDS_EntityUserData *)&info);
    if (result == DDS_RETCODE_OK) {
        oldListener = info->listener;
        if (listener) {
            newListener = os_malloc(sizeof(dds_subscriberlistener_t));
            *newListener = *listener;
            mask = dds_status_get_enabled(e);
            info->listener = newListener;
            dds_subscriber_listener_init(&dpl, info);
        } else {
            mask = 0;
            info->listener = NULL;
        }
        result = DDS_Subscriber_set_listener(e, &dpl, mask);
        DDS_Entity_release_user_data((DDS_EntityUserData)info);
        os_free(oldListener);
    }

    DDS_REPORT_FLUSH(e, result != DDS_RETCODE_OK);

    return DDS_ERRNO(result, DDS_MOD_KERNEL, DDS_ERR_Mx);
}

int
dds_subscriber_delete(
    dds_entity_t e)
{
    int result;
    DDS_DomainParticipant participant;

    result = DDS_Subscriber_delete_contained_entities(e);
    if (result == DDS_RETCODE_OK) {
        participant = DDS_Subscriber_get_participant(e);
        if (participant) {
            result = DDS_DomainParticipant_delete_subscriber(participant, e);
        } else {
            result = DDS_RETCODE_ALREADY_DELETED;
        }
    }

    return DDS_ERRNO(result, DDS_MOD_KERNEL, DDS_ERR_Mx);
}
