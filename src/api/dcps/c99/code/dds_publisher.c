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
#include "dds_dcps.h"
#include "dds_dcps_private.h"
#include "dds.h"
#include "dds_report.h"
#include "dds__qos.h"


struct PublisherInfo {
    struct DDS_EntityUserData_s _parent;
    dds_publisherlistener_t *listener;
};

static void
on_offered_deadline_missed (
    void *listener_data,
    DDS_DataWriter writer,
    const DDS_OfferedDeadlineMissedStatus *status)
{
    struct PublisherInfo *info = listener_data;
    dds_publisherlistener_t *listener = NULL;

    if (info) {
        listener = info->listener;
    }

    if (listener && status && listener->writerlistener.on_offered_deadline_missed) {
        dds_offered_deadline_missed_status_t s;
        s.total_count = status->total_count;
        s.total_count_change = status->total_count_change;
        s.last_instance_handle = status->last_instance_handle;
        listener->writerlistener.on_offered_deadline_missed(writer, &s);
    }
}

static void
on_offered_incompatible_qos (
    void *listener_data,
    DDS_DataWriter writer,
    const DDS_OfferedIncompatibleQosStatus *status)
{
    struct PublisherInfo *info = listener_data;
    dds_publisherlistener_t *listener = NULL;

    if (info) {
        listener = info->listener;
    }

    if (listener && status && listener->writerlistener.on_offered_incompatible_qos) {
        dds_offered_incompatible_qos_status_t s;
        s.total_count = status->total_count;
        s.total_count_change = status->total_count_change;
        s.last_policy_id = status->last_policy_id;
        listener->writerlistener.on_offered_incompatible_qos(writer, &s);
    }
}

static void
on_liveliness_lost (
    void *listener_data,
    DDS_DataWriter writer,
    const DDS_LivelinessLostStatus *status)
{
    struct PublisherInfo *info = listener_data;
    dds_publisherlistener_t *listener = NULL;

    if (info) {
        listener = info->listener;
    }

    if (listener && status && listener->writerlistener.on_liveliness_lost) {
        dds_liveliness_lost_status_t s;
        s.total_count = status->total_count;
        s.total_count_change = status->total_count_change;
        listener->writerlistener.on_liveliness_lost(writer, &s);
    }
}

static void
on_publication_match (
    void *listener_data,
    DDS_DataWriter writer,
    const DDS_PublicationMatchedStatus *status)
{
    struct PublisherInfo *info = listener_data;
    dds_publisherlistener_t *listener = NULL;

    if (info) {
        listener = info->listener;
    }

    if (listener && status && listener->writerlistener.on_publication_matched) {
        dds_publication_matched_status_t s;
        s.total_count = status->total_count;
        s.total_count_change = status->total_count_change;
        s.current_count = status->current_count;
        s.current_count_change = status->current_count_change;
        s.last_subscription_handle = status->last_subscription_handle;
        listener->writerlistener.on_publication_matched(writer, &s);
    }
}

static void
dds_publisher_info_free(
    DDS_EntityUserData data)
{
    struct PublisherInfo *info = (struct PublisherInfo *)data;
    if (info) {
        if (info->listener) {
            os_free(info->listener);
        }
        os_free(info);
    }
}

static struct PublisherInfo *
dds_publisher_info_new(
    void)
{
    struct PublisherInfo *info;

    info = os_malloc(sizeof(*info));
    DDS_Entity_user_data_init((DDS_EntityUserData)info, dds_publisher_info_free);
    info->listener = NULL;

    return info;
}

static void
dds_publisher_listener_init(
    struct DDS_PublisherListener *listener,
    void *data)
{
    listener->listener_data = data;
    listener->on_offered_deadline_missed = on_offered_deadline_missed;
    listener->on_offered_incompatible_qos = on_offered_incompatible_qos;
    listener->on_liveliness_lost = on_liveliness_lost;
    listener->on_publication_matched = on_publication_match;
}



int
dds_publisher_create(
    dds_entity_t pp,
    dds_entity_t *  publisher,
    const dds_qos_t * qos,
    const dds_publisherlistener_t * listener)
{
    DDS_ReturnCode_t result;
    struct PublisherInfo *info;
    struct DDS_PublisherListener dpl;
    struct DDS_PublisherListener *lp = NULL;
    DDS_StatusMask mask = (listener) ? DDS_STATUS_MASK_ANY : 0;
    DDS_PublisherQos *pQos;

    DDS_REPORT_STACK();

    if (pp && publisher) {
        *publisher = NULL;

        info = dds_publisher_info_new();

        if (listener) {
            info->listener = os_malloc(sizeof(dds_publisherlistener_t));
            *info->listener = *listener;
            lp = &dpl;
            dds_publisher_listener_init(&dpl, info);
        }

        if (qos) {
            pQos = DDS_PublisherQos__alloc();
            result = DDS_DomainParticipant_get_default_publisher_qos(pp, pQos);
            if (result == DDS_RETCODE_OK) {
                dds_qos_to_publisher_qos(pQos, qos);
                *publisher = DDS_DomainParticipant_create_publisher(pp, pQos, lp, mask);
            }
            DDS_free(pQos);
        } else {
            *publisher = DDS_DomainParticipant_create_publisher(pp, DDS_PUBLISHER_QOS_DEFAULT, lp, mask);
        }
        if (*publisher) {
            result = DDS_Entity_set_user_data(*publisher, (DDS_EntityUserData)info);
        } else {
            result = dds_report_get_error_code();
        }
        DDS_Entity_release_user_data((DDS_EntityUserData)info);
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        DDS_REPORT(result, "DomainParticipant or publisher parameter is NULL.");
    }

    DDS_REPORT_FLUSH(pp, result != DDS_RETCODE_OK);

    return DDS_ERRNO(result, DDS_MOD_KERNEL, DDS_ERR_Mx);
}

int
dds_publisher_get_listener(
    dds_entity_t e,
    dds_publisherlistener_t *listener)
{
    DDS_ReturnCode_t result;
    struct PublisherInfo *info = NULL;

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
dds_publisher_set_listener(
    dds_entity_t e,
    const dds_publisherlistener_t *listener)
{
    DDS_ReturnCode_t result              = DDS_RETCODE_ERROR;
    struct PublisherInfo *info           = NULL;
    struct DDS_PublisherListener dpl     = {0};
    dds_publisherlistener_t *newListener = NULL;
    dds_publisherlistener_t *oldListener = NULL;
    DDS_StatusMask mask                  = 0;

    DDS_REPORT_STACK();

    result = DDS_Entity_claim_user_data(e, (DDS_EntityUserData *)&info);
    if (result == DDS_RETCODE_OK) {
        oldListener = info->listener;
        if (listener) {
            newListener = os_malloc(sizeof(dds_publisherlistener_t));
            *newListener = *listener;
            mask = dds_status_get_enabled(e);
            info->listener = newListener;
            dds_publisher_listener_init(&dpl, info);
        } else {
            mask = 0;
            info->listener = NULL;
        }
        result = DDS_Publisher_set_listener(e, &dpl, mask);
        DDS_Entity_release_user_data((DDS_EntityUserData)info);
        os_free(oldListener);
    }

    DDS_REPORT_FLUSH(e, result != DDS_RETCODE_OK);

    return DDS_ERRNO(result, DDS_MOD_KERNEL, DDS_ERR_Mx);
}

int
dds_publisher_delete(
    dds_entity_t e)
{
    int result;
    DDS_DomainParticipant participant;

    result = DDS_Publisher_delete_contained_entities(e);
    if (result == DDS_RETCODE_OK) {
        participant = DDS_Publisher_get_participant(e);
        if (participant) {
            result = DDS_DomainParticipant_delete_publisher(participant, e);
        } else {
            result = DDS_RETCODE_ALREADY_DELETED;
        }
    }

    return DDS_ERRNO(result, DDS_MOD_KERNEL, DDS_ERR_Mx);
}
