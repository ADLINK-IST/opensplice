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
#include "dds__time.h"
#include "dds__publisher.h"

struct DataWriterInfo {
    struct DDS_EntityUserData_s _parent;
    dds_writerlistener_t *listener;
    bool ownPublisher;
};

static void
on_offered_deadline_missed (
    void *listener_data,
    DDS_DataWriter writer,
    const DDS_OfferedDeadlineMissedStatus *status)
{
    struct DataWriterInfo *info = listener_data;
    dds_writerlistener_t *listener = NULL;

    if (info) {
        listener = info->listener;
    }

    if (listener && status && listener->on_offered_deadline_missed) {
        dds_offered_deadline_missed_status_t s;
        s.total_count = status->total_count;
        s.total_count_change = status->total_count_change;
        s.last_instance_handle = status->last_instance_handle;
        listener->on_offered_deadline_missed(writer, &s);
    }
}

static void
on_offered_incompatible_qos (
    void *listener_data,
    DDS_DataWriter writer,
    const DDS_OfferedIncompatibleQosStatus *status)
{
    struct DataWriterInfo *info = listener_data;
    dds_writerlistener_t *listener = NULL;

    if (info) {
        listener = info->listener;
    }

    if (listener && status && listener->on_offered_incompatible_qos) {
        dds_offered_incompatible_qos_status_t s;
        s.total_count = status->total_count;
        s.total_count_change = status->total_count_change;
        s.last_policy_id = status->last_policy_id;
        listener->on_offered_incompatible_qos(writer, &s);
    }
}

static void
on_liveliness_lost (
    void *listener_data,
    DDS_DataWriter writer,
    const DDS_LivelinessLostStatus *status)
{
    struct DataWriterInfo *info = listener_data;
    dds_writerlistener_t *listener = NULL;

    if (info) {
        listener = info->listener;
    }

    if (listener && status && listener->on_liveliness_lost) {
        dds_liveliness_lost_status_t s;
        s.total_count = status->total_count;
        s.total_count_change = status->total_count_change;
        listener->on_liveliness_lost(writer, &s);
    }
}

static void
on_publication_match (
    void *listener_data,
    DDS_DataWriter writer,
    const DDS_PublicationMatchedStatus *status)
{
    struct DataWriterInfo *info = listener_data;
    dds_writerlistener_t *listener = NULL;

    if (info) {
        listener = info->listener;
    }

    if (listener && status && listener->on_publication_matched) {
        dds_publication_matched_status_t s;
        s.total_count = status->total_count;
        s.total_count_change = status->total_count_change;
        s.current_count = status->current_count;
        s.current_count_change = status->current_count_change;
        s.last_subscription_handle = status->last_subscription_handle;
        listener->on_publication_matched(writer, &s);
    }
}

static void
dds_datawriter_info_free(
    DDS_EntityUserData data)
{
    struct DataWriterInfo *info = (struct DataWriterInfo *)data;
    if (info) {
        if (info->listener) {
            os_free(info->listener);
        }
        os_free(info);
    }
}

static struct DataWriterInfo *
dds_datawriter_info_new(
    void)
{
    struct DataWriterInfo *info;

    info = os_malloc(sizeof(*info));
    DDS_Entity_user_data_init((DDS_EntityUserData)info, dds_datawriter_info_free);
    info->listener = NULL;
    info->ownPublisher = false;

    return info;
}

static void
dds_datawriter_listener_init(
    struct DDS_DataWriterListener *listener,
    void *data)
{
    listener->listener_data = data;
    listener->on_offered_deadline_missed = on_offered_deadline_missed;
    listener->on_offered_incompatible_qos = on_offered_incompatible_qos;
    listener->on_liveliness_lost = on_liveliness_lost;
    listener->on_publication_matched = on_publication_match;
}



int
dds_writer_create (
  dds_entity_t pp_or_pub,
  dds_entity_t * writer,
  dds_entity_t topic,
  const dds_qos_t * qos,
  const dds_writerlistener_t * listener)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    dds_entity_t publisher = NULL;
    struct DataWriterInfo *info;
    struct DDS_DataWriterListener dpl;
    struct DDS_DataWriterListener *lp = NULL;
    DDS_StatusMask mask = DDS_STATUS_MASK_ANY | DDS_PUBLICATION_MATCHED_STATUS;
    DDS_DataWriterQos *wQos;
    bool ownPublisher = false;

    DDS_REPORT_STACK();

    if (!pp_or_pub) {
        result = DDS_RETCODE_BAD_PARAMETER;
        DDS_REPORT(result, "Participant or publisher parameter is NULL.");
    } else if (!writer) {
        result = DDS_RETCODE_BAD_PARAMETER;
        DDS_REPORT(result, "Writer parameter is NULL.");
    } else if (!topic) {
        result = DDS_RETCODE_BAD_PARAMETER;
        DDS_REPORT(result, "Topic parameter is NULL.");
    } else {
        if (DDS_Entity_get_kind(pp_or_pub) == DDS_ENTITY_KIND_DOMAINPARTICIPANT) {
            result = dds_publisher_create(pp_or_pub, &publisher, qos, NULL);
            ownPublisher = true;
        } else {
            publisher = pp_or_pub;
        }
    }

    if (result == DDS_RETCODE_OK) {
        *writer = NULL;

        info = dds_datawriter_info_new();
        info->ownPublisher = ownPublisher;

        if (listener) {
            info->listener = os_malloc(sizeof(dds_writerlistener_t));
            *info->listener = *listener;
            lp = &dpl;
            dds_datawriter_listener_init(&dpl, info);
        }

        if (qos) {
            wQos = DDS_DataWriterQos__alloc();
            result = DDS_Publisher_get_default_datawriter_qos(publisher, wQos);
            if (result == DDS_RETCODE_OK) {
                dds_qos_to_writer_qos(wQos, qos);
                *writer = DDS_Publisher_create_datawriter(publisher, topic, wQos, lp, lp ? mask : 0);
            }
            DDS_free(wQos);
        } else {
            *writer = DDS_Publisher_create_datawriter(publisher, topic, DDS_DATAWRITER_QOS_DEFAULT, lp,  lp ? mask : 0);
        }
        if (*writer) {
            result = DDS_Entity_set_user_data(*writer, (DDS_EntityUserData)info);
        } else {
            result = dds_report_get_error_code();
        }
        if (result == DDS_RETCODE_OK) {
            /* Because of OSPL-9383, we have to set the mask with
             * DDS_PUBLICATION_MATCHED_STATUS enabled. */
            dds_status_set_enabled(*writer, mask);
        }
        DDS_Entity_release_user_data((DDS_EntityUserData)info);
    }

    DDS_REPORT_FLUSH(pp_or_pub, result != DDS_RETCODE_OK);

    return DDS_ERRNO(result, DDS_MOD_WRITER, DDS_ERR_Mx);
}

dds_instance_handle_t
dds_instance_register (
    dds_entity_t wr,
    const void *data)
{
    dds_instance_handle_t handle;

    DDS_REPORT_STACK();

    handle = DDS_DataWriter_register_instance(wr, (const DDS_Sample)data);

    DDS_REPORT_FLUSH(wr, handle == 0);

    return handle;
}

int
dds_instance_unregister (
    dds_entity_t wr,
    const void * data,
    dds_instance_handle_t handle)
{
    DDS_ReturnCode_t result;

    DDS_REPORT_STACK();

    result = DDS_DataWriter_unregister_instance(wr, (const DDS_Sample)data, handle);

    DDS_REPORT_FLUSH(wr, result != DDS_RETCODE_OK);

    return DDS_ERRNO(result, DDS_MOD_WRITER, DDS_ERR_Mx);
}

int
dds_instance_unregister_ts (
    dds_entity_t wr,
    const void * data,
    dds_instance_handle_t handle,
    dds_time_t timestamp)
{
    DDS_ReturnCode_t result;
    DDS_Time_t ts = dds_time_to_sac(timestamp);

    DDS_REPORT_STACK();

    result = DDS_DataWriter_unregister_instance_w_timestamp(wr, (const DDS_Sample)data, handle, &ts);

    DDS_REPORT_FLUSH(wr, result != DDS_RETCODE_OK);

    return DDS_ERRNO(result, DDS_MOD_WRITER, DDS_ERR_Mx);
}

int
dds_instance_writedispose (
    dds_entity_t wr,
    const void *data)
{
    DDS_ReturnCode_t result;

    DDS_REPORT_STACK();

    result = DDS_DataWriter_writedispose(wr, (const DDS_Sample)data, DDS_HANDLE_NIL);

    DDS_REPORT_FLUSH(wr, result != DDS_RETCODE_OK);

    return DDS_ERRNO(result, DDS_MOD_WRITER, DDS_ERR_Mx);
}

int dds_instance_writedispose_ts (
    dds_entity_t wr,
    const void *data,
    dds_time_t timestamp)
{
    DDS_ReturnCode_t result;
    DDS_Time_t ts = dds_time_to_sac(timestamp);

    DDS_REPORT_STACK();

    result = DDS_DataWriter_writedispose_w_timestamp(wr, (const DDS_Sample)data, DDS_HANDLE_NIL, &ts);

    DDS_REPORT_FLUSH(wr, result != DDS_RETCODE_OK);

    return DDS_ERRNO(result, DDS_MOD_WRITER, DDS_ERR_Mx);
}

int
dds_instance_dispose (
    dds_entity_t wr,
    const void *data)
{
    DDS_ReturnCode_t result;

    DDS_REPORT_STACK();

    result = DDS_DataWriter_dispose(wr, (const DDS_Sample)data, DDS_HANDLE_NIL);

    DDS_REPORT_FLUSH(wr, result != DDS_RETCODE_OK);

    return DDS_ERRNO(result, DDS_MOD_WRITER, DDS_ERR_Mx);
}

int
dds_instance_dispose_ts (
    dds_entity_t wr,
    const void *data,
    dds_time_t timestamp)
{
    DDS_ReturnCode_t result;
    DDS_Time_t ts = dds_time_to_sac(timestamp);

    DDS_REPORT_STACK();

    result = DDS_DataWriter_dispose_w_timestamp(wr, (const DDS_Sample)data, DDS_HANDLE_NIL, &ts);

    DDS_REPORT_FLUSH(wr, result != DDS_RETCODE_OK);

    return DDS_ERRNO(result, DDS_MOD_WRITER, DDS_ERR_Mx);
}

int
dds_write (
    dds_entity_t wr,
    const void *data)
{
    DDS_ReturnCode_t result;

    DDS_REPORT_STACK();

    result = DDS_DataWriter_write(wr, (const DDS_Sample)data, DDS_HANDLE_NIL);

    DDS_REPORT_FLUSH(wr, result != DDS_RETCODE_OK);

    return DDS_ERRNO(result, DDS_MOD_WRITER, DDS_ERR_Mx);
}

int dds_write_ts (
    dds_entity_t wr,
    const void *data,
    dds_time_t timestamp)
{
    DDS_ReturnCode_t result;
    DDS_Time_t ts = dds_time_to_sac(timestamp);

    DDS_REPORT_STACK();

    result = DDS_DataWriter_write_w_timestamp(wr, (const DDS_Sample)data, DDS_HANDLE_NIL, &ts);

    DDS_REPORT_FLUSH(wr, result != DDS_RETCODE_OK);

    return DDS_ERRNO(result, DDS_MOD_WRITER, DDS_ERR_Mx);
}

void
dds_write_flush (
     dds_entity_t wr)
{
    OS_UNUSED_ARG(wr);
}

int
dds_datawriter_get_listener(
    dds_entity_t e,
    dds_writerlistener_t *listener)
{
    DDS_ReturnCode_t result;
    struct DataWriterInfo *info = NULL;

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

    return DDS_ERRNO(result, DDS_MOD_WRITER, DDS_ERR_Mx);
}

int
dds_datawriter_set_listener(
    dds_entity_t e,
    const dds_writerlistener_t *listener)
{
    DDS_ReturnCode_t result           = DDS_RETCODE_ERROR;
    struct DataWriterInfo *info       = NULL;
    struct DDS_DataWriterListener dpl = {0};
    dds_writerlistener_t *newListener = NULL;
    dds_writerlistener_t *oldListener = NULL;
    DDS_StatusMask mask               = 0;

    DDS_REPORT_STACK();

    result = DDS_Entity_claim_user_data(e, (DDS_EntityUserData *)&info);
    if (result == DDS_RETCODE_OK) {
        oldListener = info->listener;
        if (listener) {
            newListener = os_malloc(sizeof(dds_writerlistener_t));
            *newListener = *listener;
            mask = dds_status_get_enabled(e);
            info->listener = newListener;
            dds_datawriter_listener_init(&dpl, info);
        } else {
            mask = 0;
            info->listener = NULL;
        }
        result = DDS_DataWriter_set_listener(e, &dpl, mask);
        DDS_Entity_release_user_data((DDS_EntityUserData)info);
        os_free(oldListener);
    }

    DDS_REPORT_FLUSH(e, result != DDS_RETCODE_OK);

    return DDS_ERRNO(result, DDS_MOD_WRITER, DDS_ERR_Mx);
}

int
dds_datawriter_delete(
    dds_entity_t wr)
{
    int result;
    struct DataWriterInfo *info = NULL;
    DDS_Publisher publisher;

    result = DDS_Entity_claim_user_data(wr, (DDS_EntityUserData *)&info);
    if (result == DDS_RETCODE_OK) {
        publisher = DDS_DataWriter_get_publisher(wr);
        if (publisher) {
            if (info->ownPublisher) {
                result = dds_publisher_delete(publisher);
            } else {
                result = DDS_Publisher_delete_datawriter(publisher, wr);
            }
        } else {
            result = DDS_RETCODE_ALREADY_DELETED;
        }
        DDS_Entity_release_user_data((DDS_EntityUserData)info);
    }

    return DDS_ERRNO(result, DDS_MOD_WRITER, DDS_ERR_Mx);
}
