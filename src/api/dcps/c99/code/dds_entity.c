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
#include <dds__domainParticipant.h>
#include <dds__topic.h>
#include <dds__publisher.h>
#include <dds__subscriber.h>
#include <dds__datawriter.h>
#include <dds__datareader.h>
#include <dds__qos.h>


void
dds_entity_delete (
    dds_entity_t e)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;


    DDS_REPORT_STACK();

    if (e) {
        switch (DDS_Entity_get_kind(e)) {
        case DDS_ENTITY_KIND_DOMAINPARTICIPANT:
            result = dds_domainparticipant_delete(e);
            break;
        case DDS_ENTITY_KIND_TOPIC:
            result = dds_topic_delete(e);
            break;
        case DDS_ENTITY_KIND_PUBLISHER:
            result = dds_publisher_delete(e);
            break;
        case DDS_ENTITY_KIND_SUBSCRIBER:
            result = dds_subscriber_delete(e);
            break;
        case DDS_ENTITY_KIND_DATAWRITER:
            result = dds_datawriter_delete(e);
            break;
        case DDS_ENTITY_KIND_DATAREADER:
            result = dds_datareader_delete(e);
            break;
        default:
            result = DDS_RETCODE_BAD_PARAMETER;
            DDS_REPORT(result, "Entity parameter is not a valid dds entity.");
            break;
        }
    }

    DDS_REPORT_FLUSH(NULL, result != DDS_RETCODE_OK);
}



static int
dds_status_get(
        dds_entity_t e,
        uint32_t * status,
        uint32_t mask,
        DDS_boolean clear)
{
    DDS_ReturnCode_t result = DDS_RETCODE_BAD_PARAMETER;
    if (status != NULL) {
        DDS_StatusMask s;
        result = DDS_Entity_read_status(e, &s, (DDS_StatusMask)mask, clear);
        *status = (uint32_t)s;
    }
    return result;
}

int
dds_status_read (
    dds_entity_t e,
    uint32_t * status,
    uint32_t mask)
{
    DDS_ReturnCode_t result;

    DDS_REPORT_STACK();

    result = dds_status_get(e, status, mask, FALSE);

    DDS_REPORT_FLUSH(e, result != DDS_RETCODE_OK);

    return DDS_ERRNO(result, DDS_MOD_KERNEL, DDS_ERR_Mx);
}

int
dds_status_take (
    dds_entity_t e,
    uint32_t * status,
    uint32_t mask)
{
    DDS_ReturnCode_t result;

    DDS_REPORT_STACK();

    result = dds_status_get(e, status, mask, TRUE);

    DDS_REPORT_FLUSH(e, result != DDS_RETCODE_OK);

    return DDS_ERRNO(result, DDS_MOD_KERNEL, DDS_ERR_Mx);
}

uint32_t
dds_status_changes (
    dds_entity_t e)
{
    uint32_t status;

    status = DDS_Entity_get_status_changes(e);

    return status;
}

uint32_t
dds_status_get_enabled (
     dds_entity_t e)
{
    uint32_t mask = 0;
    DDS_StatusCondition cond;

    DDS_REPORT_STACK();

    cond = DDS_Entity_get_statuscondition(e);

    if (cond != NULL) {
        mask = DDS_StatusCondition_get_enabled_statuses(cond);
    }

    DDS_REPORT_FLUSH(e, cond == NULL);

    return mask;
}

int
dds_status_set_enabled (
    dds_entity_t e,
    uint32_t mask)
{
    DDS_ReturnCode_t result;
    DDS_StatusCondition cond;

    DDS_REPORT_STACK();

    if ((cond = DDS_Entity_get_statuscondition(e)) != NULL) {
        result = DDS_StatusCondition_set_enabled_statuses(cond, mask);
        if (result == DDS_RETCODE_OK) {
            result = DDS_Entity_set_listener_mask(e, mask);
        }
    } else {
        result = DDS_RETCODE_ALREADY_DELETED;
    }

    DDS_REPORT_FLUSH(e, result != DDS_RETCODE_OK);

    return DDS_ERRNO(result, DDS_MOD_KERNEL, DDS_ERR_Mx);
}

void
dds_qos_get (
    dds_entity_t e,
    struct nn_xqos * qos)
{
    DDS_ReturnCode_t result;

    DDS_REPORT_STACK();

    if (e && qos) {
        switch (DDS_Entity_get_kind(e)) {
        case DDS_ENTITY_KIND_DOMAINPARTICIPANT:
            {
                DDS_DomainParticipantQos *pqos = DDS_DomainParticipantQos__alloc();
                result = DDS_DomainParticipant_get_qos(e, pqos);
                if (result == DDS_RETCODE_OK) {
                    dds_qos_from_participant_qos(qos, pqos);
                }
                DDS_free(pqos);
            }
            break;
        case DDS_ENTITY_KIND_PUBLISHER:
            {
                DDS_PublisherQos *pqos = DDS_PublisherQos__alloc();
                result = DDS_Publisher_get_qos(e, pqos);
                if (result == DDS_RETCODE_OK) {
                    dds_qos_from_publisher_qos(qos, pqos);
                }
                DDS_free(pqos);
            }
            break;
        case DDS_ENTITY_KIND_SUBSCRIBER:
            {
                DDS_SubscriberQos *sqos = DDS_SubscriberQos__alloc();
                result = DDS_Subscriber_get_qos(e, sqos);
                if (result == DDS_RETCODE_OK) {
                    dds_qos_from_subscriber_qos(qos, sqos);
                }
                DDS_free(sqos);
            }
            break;
        case DDS_ENTITY_KIND_DATAWRITER:
            {
                DDS_DataWriterQos *wqos = DDS_DataWriterQos__alloc();
                result = DDS_DataWriter_get_qos(e, wqos);
                if (result == DDS_RETCODE_OK) {
                    dds_qos_from_writer_qos(qos, wqos);
                }
                DDS_free(wqos);
            }
            break;
        case DDS_ENTITY_KIND_DATAREADER:
            {
                DDS_DataReaderQos *rqos = DDS_DataReaderQos__alloc();
                result = DDS_DataReader_get_qos(e, rqos);
                if (result == DDS_RETCODE_OK) {
                    dds_qos_from_reader_qos(qos, rqos);
                }
                DDS_free(rqos);
            }
            break;
        case DDS_ENTITY_KIND_TOPIC:
            {
                DDS_TopicQos *tqos = DDS_TopicQos__alloc();
                result = DDS_Topic_get_qos(e, tqos);
                if (result == DDS_RETCODE_OK) {
                    dds_qos_from_topic_qos(qos, tqos);
                }
                DDS_free(tqos);
            }
            break;
        default:
            result = DDS_RETCODE_BAD_PARAMETER;
            DDS_REPORT(result, "Entity parameter is not a valid dds entity.");
            break;
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        DDS_REPORT(result, "Entity or qos parameter is NULL.");
    }

    DDS_REPORT_FLUSH(e, result != DDS_RETCODE_OK);
}

int
dds_qos_set (
     dds_entity_t e,
     const struct nn_xqos * qos)
{
    DDS_ReturnCode_t result;

    DDS_REPORT_STACK();

    if (e && qos) {
        switch (DDS_Entity_get_kind(e)) {
        case DDS_ENTITY_KIND_DOMAINPARTICIPANT:
            {
                DDS_DomainParticipantQos *pqos = DDS_DomainParticipantQos__alloc();
                dds_qos_to_participant_qos(pqos, qos);
                result = DDS_DomainParticipant_set_qos(e, pqos);
                DDS_free(pqos);
            }
            break;
        case DDS_ENTITY_KIND_PUBLISHER:
            {
                DDS_PublisherQos *pqos = DDS_PublisherQos__alloc();
                dds_qos_to_publisher_qos(pqos, qos);
                result = DDS_Publisher_get_qos(e, pqos);
                DDS_free(pqos);
            }
            break;
        case DDS_ENTITY_KIND_SUBSCRIBER:
            {
                DDS_SubscriberQos *sqos = DDS_SubscriberQos__alloc();
                dds_qos_to_subscriber_qos(sqos, qos);
                result = DDS_Subscriber_get_qos(e, sqos);
                DDS_free(sqos);
            }
            break;
        case DDS_ENTITY_KIND_DATAWRITER:
            {
                DDS_DataWriterQos *wqos = DDS_DataWriterQos__alloc();
                dds_qos_to_writer_qos(wqos, qos);
                result = DDS_DataWriter_get_qos(e, wqos);
                DDS_free(wqos);
            }
            break;
        case DDS_ENTITY_KIND_DATAREADER:
            {
                DDS_DataReaderQos *rqos = DDS_DataReaderQos__alloc();
                dds_qos_to_reader_qos(rqos, qos);
                result = DDS_DataReader_get_qos(e, rqos);
                DDS_free(rqos);
            }
            break;
        case DDS_ENTITY_KIND_TOPIC:
            {
                DDS_TopicQos *tqos = DDS_TopicQos__alloc();
                dds_qos_to_topic_qos(tqos, qos);
                result = DDS_Topic_get_qos(e, tqos);
                DDS_free(tqos);
            }
            break;
        default:
            result = DDS_RETCODE_BAD_PARAMETER;
            DDS_REPORT(result, "Entity parameter is not a valid dds entity.");
            break;
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        DDS_REPORT(result, "Entity or qos parameter is NULL.");
    }

    DDS_REPORT_FLUSH(e, result != DDS_RETCODE_OK);

    return DDS_ERRNO(result, DDS_MOD_KERNEL, DDS_ERR_Mx);
}


void
dds_listener_get (
    dds_entity_t e,
    dds_listener_t listener)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;

    DDS_REPORT_STACK();

    if (e && listener) {
        switch (DDS_Entity_get_kind(e)) {
        case DDS_ENTITY_KIND_DOMAINPARTICIPANT:
            result = dds_domainparticipant_get_listener(e, (dds_participantlistener_t *)listener);
            break;
        case DDS_ENTITY_KIND_PUBLISHER:
            result = dds_publisher_get_listener(e, (dds_publisherlistener_t *) listener);
            break;
        case DDS_ENTITY_KIND_SUBSCRIBER:
            result = dds_subscriber_get_listener(e, (dds_subscriberlistener_t *) listener);
            break;
        case DDS_ENTITY_KIND_DATAWRITER:
            result = dds_datawriter_get_listener(e, (dds_writerlistener_t *) listener);
            break;
        case DDS_ENTITY_KIND_DATAREADER:
            result = dds_datareader_get_listener(e, (dds_readerlistener_t *) listener);
            break;
        case DDS_ENTITY_KIND_TOPIC:
            result = dds_topic_get_listener(e, (dds_topiclistener_t *) listener);
            break;
        default:
            result = DDS_RETCODE_BAD_PARAMETER;
            DDS_REPORT(result, "Entity parameter is not a valid dds entity.");
            break;
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        DDS_REPORT(result, "Entity or listener parameter is NULL.");
    }

    DDS_REPORT_FLUSH(e, result != DDS_RETCODE_OK);
}

void
dds_listener_set (
     dds_entity_t e,
     const dds_listener_t listener)
{
    int result = DDS_RETCODE_OK;

    DDS_REPORT_STACK();

    if (e && listener) {
        switch (DDS_Entity_get_kind(e)) {
        case DDS_ENTITY_KIND_DOMAINPARTICIPANT:
            result = dds_domainparticipant_set_listener(e, (const dds_participantlistener_t *) listener);
            break;
        case DDS_ENTITY_KIND_PUBLISHER:
            result = dds_publisher_set_listener(e, (const dds_publisherlistener_t *) listener);
            break;
        case DDS_ENTITY_KIND_SUBSCRIBER:
            result = dds_subscriber_set_listener(e, (const dds_subscriberlistener_t *) listener);
            break;
        case DDS_ENTITY_KIND_DATAWRITER:
            result = dds_datawriter_set_listener(e, (const dds_writerlistener_t *) listener);
            break;
        case DDS_ENTITY_KIND_DATAREADER:
            result = dds_datareader_set_listener(e, (const dds_readerlistener_t *) listener);
            break;
        case DDS_ENTITY_KIND_TOPIC:
            result = dds_topic_set_listener(e, (const dds_topiclistener_t *) listener);
            break;
        default:
            result = DDS_RETCODE_BAD_PARAMETER;
            DDS_REPORT(result, "Entity parameter is not a valid dds entity.");
            break;
        }

    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        DDS_REPORT(result, "Entity or listener parameter is NULL.");
    }

    DDS_REPORT_FLUSH(e, result != DDS_RETCODE_OK);
}

dds_condition_t
dds_statuscondition_get (
    dds_entity_t pp)
{
    DDS_StatusCondition cond;

    DDS_REPORT_STACK();

    cond = DDS_Entity_get_statuscondition(pp);

    DDS_REPORT_FLUSH(pp, cond == NULL);

    return cond;
}

dds_instance_handle_t
dds_instance_lookup (
    dds_entity_t e,
    const void * data)
{
    dds_instance_handle_t handle = DDS_HANDLE_NIL;

    DDS_REPORT_STACK();

    if (e && data) {
        switch (DDS_Entity_get_kind(e)) {
        case DDS_ENTITY_KIND_DATAWRITER:
            handle = (dds_instance_handle_t)DDS_DataWriter_lookup_instance(e, (const DDS_Sample)data);
            break;
        case DDS_ENTITY_KIND_DATAREADER:
            handle = (dds_instance_handle_t)DDS_DataReader_lookup_instance(e, (const DDS_Sample)data);
            break;
        default:
            DDS_REPORT(DDS_RETCODE_BAD_PARAMETER, "Entity parameter is not a valid writer or reader.");
            break;
        }
    } else {
        DDS_REPORT(DDS_RETCODE_BAD_PARAMETER, "Entity or data = NULL.");
    }

    DDS_REPORT_FLUSH(e, handle == DDS_HANDLE_NIL);

    return handle;
}

int
dds_instance_get_key (
    dds_entity_t e,
    dds_instance_handle_t inst,
    void * data)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;

    DDS_REPORT_STACK();

    if (e && data) {
        switch (DDS_Entity_get_kind(e)) {
        case DDS_ENTITY_KIND_DATAWRITER:
            result = DDS_DataWriter_get_key_value(e, data, inst);
            break;
        case DDS_ENTITY_KIND_DATAREADER:
            result = DDS_DataReader_get_key_value(e, data, inst);
            break;
        default:
            result = DDS_RETCODE_BAD_PARAMETER;
            DDS_REPORT(result, "Entity parameter is not a valid writer or reader.");
            break;
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        DDS_REPORT(result, "Entity or data = NULL.");
    }

    DDS_REPORT_FLUSH(e, result != DDS_RETCODE_OK);

    return DDS_ERRNO(result, DDS_MOD_KERNEL, DDS_ERR_Mx);
}
