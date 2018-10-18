/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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

/* SAC */
#include <dds_dcps.h>

/* C99 */
#include <dds/impl.h>
#include <dds/status.h>
#include <dds_report.h>


int dds_get_inconsistent_topic_status (dds_entity_t topic, dds_inconsistent_topic_status_t * status)
{
    DDS_ReturnCode_t result = DDS_RETCODE_BAD_PARAMETER;
    DDS_InconsistentTopicStatus s = {0,0};

    DDS_REPORT_STACK();
    result = DDS_Topic_get_inconsistent_topic_status(topic, &s);
    if (result == DDS_RETCODE_OK) {
        if (status) {
            status->total_count = (uint32_t)s.total_count;
            status->total_count_change = (int32_t)s.total_count_change;
        }
    }
    DDS_REPORT_FLUSH(topic, result != DDS_RETCODE_OK);
    return DDS_ERRNO(result, DDS_MOD_STATUS, DDS_ERR_Mx);
}

int dds_get_publication_matched_status (dds_entity_t writer, dds_publication_matched_status_t * status)
{
    DDS_ReturnCode_t result = DDS_RETCODE_BAD_PARAMETER;
    DDS_PublicationMatchedStatus s = {0,0,0,0,0};

    DDS_REPORT_STACK();
    result = DDS_DataWriter_get_publication_matched_status(writer, &s);
    if (result == DDS_RETCODE_OK) {
        if (status) {
            status->total_count = (uint32_t)s.total_count;
            status->total_count_change = (int32_t)s.total_count_change;
            status->current_count = (uint32_t)s.current_count;
            status->current_count_change = (int32_t)s.current_count_change;
            status->last_subscription_handle = (dds_instance_handle_t)s.last_subscription_handle;
        }
    }
    DDS_REPORT_FLUSH(writer, result != DDS_RETCODE_OK);
    return DDS_ERRNO(result, DDS_MOD_STATUS, DDS_ERR_Mx);
}

int dds_get_liveliness_lost_status (dds_entity_t writer, dds_liveliness_lost_status_t * status)
{
    DDS_ReturnCode_t result = DDS_RETCODE_BAD_PARAMETER;
    DDS_LivelinessLostStatus s = {0,0};

    DDS_REPORT_STACK();
    result = DDS_DataWriter_get_liveliness_lost_status(writer, &s);
    if (result == DDS_RETCODE_OK) {
        if (status) {
            status->total_count = (uint32_t)s.total_count;
            status->total_count_change = (int32_t)s.total_count_change;
        }
    }
    DDS_REPORT_FLUSH(writer, result != DDS_RETCODE_OK);
    return DDS_ERRNO(result, DDS_MOD_STATUS, DDS_ERR_Mx);
}

int dds_get_offered_deadline_missed_status (dds_entity_t writer, dds_offered_deadline_missed_status_t * status)
{
    DDS_ReturnCode_t result = DDS_RETCODE_BAD_PARAMETER;
    DDS_OfferedDeadlineMissedStatus s = {0,0,0};

    DDS_REPORT_STACK();
    result = DDS_DataWriter_get_offered_deadline_missed_status(writer, &s);
    if (result == DDS_RETCODE_OK) {
        if (status) {
            status->total_count = (uint32_t)s.total_count;
            status->total_count_change = (int32_t)s.total_count_change;
            status->last_instance_handle = (dds_instance_handle_t)s.last_instance_handle;
        }
    }
    DDS_REPORT_FLUSH(writer, result != DDS_RETCODE_OK);
    return DDS_ERRNO(result, DDS_MOD_STATUS, DDS_ERR_Mx);
}

int dds_get_offered_incompatible_qos_status (dds_entity_t writer, dds_offered_incompatible_qos_status_t * status)
{
    DDS_ReturnCode_t result = DDS_RETCODE_BAD_PARAMETER;
    DDS_OfferedIncompatibleQosStatus s = {0,0,0,{0,0,NULL,FALSE}};

    DDS_REPORT_STACK();
    result = DDS_DataWriter_get_offered_incompatible_qos_status(writer, &s);
    if (result == DDS_RETCODE_OK) {
        if (status) {
            status->total_count = (uint32_t)s.total_count;
            status->total_count_change = (int32_t)s.total_count_change;
            status->last_policy_id = (uint32_t)s.last_policy_id;
        }
    }
    DDS_REPORT_FLUSH(writer, result != DDS_RETCODE_OK);
    return DDS_ERRNO(result, DDS_MOD_STATUS, DDS_ERR_Mx);
}

int dds_get_subscription_matched_status (dds_entity_t reader, dds_subscription_matched_status_t * status)
{
    DDS_ReturnCode_t result = DDS_RETCODE_BAD_PARAMETER;
    DDS_SubscriptionMatchedStatus s = {0,0,0,0,0};

    DDS_REPORT_STACK();
    result = DDS_DataReader_get_subscription_matched_status(reader, &s);
    if (result == DDS_RETCODE_OK) {
        if (status) {
            status->total_count = (uint32_t)s.total_count;
            status->total_count_change = (int32_t)s.total_count_change;
            status->current_count = (uint32_t)s.current_count;
            status->current_count_change = (int32_t)s.current_count_change;
            status->last_publication_handle = (dds_instance_handle_t)s.last_publication_handle;
        }
    }
    DDS_REPORT_FLUSH(reader, result != DDS_RETCODE_OK);
    return DDS_ERRNO(result, DDS_MOD_STATUS, DDS_ERR_Mx);
}

int dds_get_liveliness_changed_status (dds_entity_t reader, dds_liveliness_changed_status_t * status)
{
    DDS_ReturnCode_t result = DDS_RETCODE_BAD_PARAMETER;
    DDS_LivelinessChangedStatus s = {0,0,0,0,0};

    DDS_REPORT_STACK();
    result = DDS_DataReader_get_liveliness_changed_status(reader, &s);
    if (result == DDS_RETCODE_OK) {
        if (status) {
            status->alive_count = (uint32_t)s.alive_count;
            status->not_alive_count = (uint32_t)s.not_alive_count;
            status->alive_count_change = (int32_t)s.alive_count_change;
            status->not_alive_count_change = (int32_t)s.not_alive_count_change;
            status->last_publication_handle = (dds_instance_handle_t)s.last_publication_handle;
        }
    }
    DDS_REPORT_FLUSH(reader, result != DDS_RETCODE_OK);
    return DDS_ERRNO(result, DDS_MOD_STATUS, DDS_ERR_Mx);
}

int dds_get_sample_rejected_status (dds_entity_t reader, dds_sample_rejected_status_t * status)
{
    DDS_ReturnCode_t result = DDS_RETCODE_BAD_PARAMETER;
    DDS_SampleRejectedStatus s = {0,0,0,0};

    DDS_REPORT_STACK();
    result = DDS_DataReader_get_sample_rejected_status(reader, &s);
    if (result == DDS_RETCODE_OK) {
        if (status) {
            status->total_count = (uint32_t)s.total_count;
            status->total_count_change = (int32_t)s.total_count_change;
            status->last_reason = (dds_sample_rejected_status_kind)s.last_reason;
            status->last_instance_handle = (dds_instance_handle_t)s.last_instance_handle;
        }
    }
    DDS_REPORT_FLUSH(reader, result != DDS_RETCODE_OK);
    return DDS_ERRNO(result, DDS_MOD_STATUS, DDS_ERR_Mx);
}

int dds_get_sample_lost_status (dds_entity_t reader, dds_sample_lost_status_t * status)
{
    DDS_ReturnCode_t result = DDS_RETCODE_BAD_PARAMETER;
    DDS_SampleLostStatus s = {0,0};

    DDS_REPORT_STACK();
    result = DDS_DataReader_get_sample_lost_status(reader, &s);
    if (result == DDS_RETCODE_OK) {
        if (status) {
            status->total_count = (uint32_t)s.total_count;
            status->total_count_change = (int32_t)s.total_count_change;
        }
    }
    DDS_REPORT_FLUSH(reader, result != DDS_RETCODE_OK);
    return DDS_ERRNO(result, DDS_MOD_STATUS, DDS_ERR_Mx);
}

int dds_get_requested_deadline_missed_status (dds_entity_t reader, dds_requested_deadline_missed_status_t * status)
{
    DDS_ReturnCode_t result = DDS_RETCODE_BAD_PARAMETER;
    DDS_RequestedDeadlineMissedStatus s = {0,0,0};

    DDS_REPORT_STACK();
    result = DDS_DataReader_get_requested_deadline_missed_status(reader, &s);
    if (result == DDS_RETCODE_OK) {
        if (status) {
            status->total_count = (uint32_t)s.total_count;
            status->total_count_change = (int32_t)s.total_count_change;
            status->last_instance_handle = (dds_instance_handle_t)s.last_instance_handle;
        }
    }
    DDS_REPORT_FLUSH(reader, result != DDS_RETCODE_OK);
    return DDS_ERRNO(result, DDS_MOD_STATUS, DDS_ERR_Mx);
}

int dds_get_requested_incompatible_qos_status (dds_entity_t reader, dds_requested_incompatible_qos_status_t * status)
{
    DDS_ReturnCode_t result = DDS_RETCODE_BAD_PARAMETER;
    DDS_RequestedIncompatibleQosStatus s = {0,0,0,{0,0,NULL,FALSE}};

    DDS_REPORT_STACK();
    result = DDS_DataReader_get_requested_incompatible_qos_status(reader, &s);
    if (result == DDS_RETCODE_OK) {
        if (status) {
            status->total_count = (uint32_t)s.total_count;
            status->total_count_change = (int32_t)s.total_count_change;
            status->last_policy_id = (uint32_t)s.last_policy_id;
            /* s.policies ignored */
        }
    }
    DDS_REPORT_FLUSH(reader, result != DDS_RETCODE_OK);
    return DDS_ERRNO(result, DDS_MOD_STATUS, DDS_ERR_Mx);
}
