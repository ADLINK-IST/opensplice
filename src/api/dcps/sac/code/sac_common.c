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

#include "dds_dcps.h"
#include "dds_dcps_private.h"
#include "sac_common.h"
#include "sac_entity.h"
#include "u_dataViewQos.h"
#include "u_instanceHandle.h"
#include "v_kernel.h"
#include "kernelModule.h"
#include "c_stringSupport.h"
#include "sac_report.h"
#include "os_atomics.h"

#define USE_OLD_ReaderDataLifecycleQosPolicy

/* Qos constant values. */

const DDS_OwnershipStrengthQosPolicy
     DDS_OwnershipStrengthQosPolicy_default =
         DDS_OwnershipStrengthQosPolicy_default_value;

const DDS_DurabilityQosPolicy
     DDS_DurabilityQosPolicy_default =
         DDS_DurabilityQosPolicy_default_value;

const DDS_DeadlineQosPolicy
     DDS_DeadlineQosPolicy_default =
         DDS_DeadlineQosPolicy_default_value;

const DDS_LatencyBudgetQosPolicy
     DDS_LatencyBudgetQosPolicy_default =
         DDS_LatencyBudgetQosPolicy_default_value;

const DDS_LivelinessQosPolicy
     DDS_LivelinessQosPolicy_default =
         DDS_LivelinessQosPolicy_default_value;

const DDS_ReliabilityQosPolicy
     DDS_ReliabilityQosPolicy_default =
         DDS_ReliabilityQosPolicy_default_value;

const DDS_ReliabilityQosPolicy
     DDS_ReliabilityQosPolicy_writer_default =
         DDS_ReliabilityQosPolicy_writer_default_value;

const DDS_DestinationOrderQosPolicy
     DDS_DestinationOrderQosPolicy_default =
         DDS_DestinationOrderQosPolicy_default_value;

const DDS_HistoryQosPolicy
     DDS_HistoryQosPolicy_default =
         DDS_HistoryQosPolicy_default_value;

const DDS_ResourceLimitsQosPolicy
     DDS_ResourceLimitsQosPolicy_default =
         DDS_ResourceLimitsQosPolicy_default_value;

const DDS_TransportPriorityQosPolicy
     DDS_TransportPriorityQosPolicy_default =
         DDS_TransportPriorityQosPolicy_default_value;

const DDS_LifespanQosPolicy
     DDS_LifespanQosPolicy_default =
         DDS_LifespanQosPolicy_default_value;

const DDS_OwnershipQosPolicy
     DDS_OwnershipQosPolicy_default =
         DDS_OwnershipQosPolicy_default_value;

const DDS_PresentationQosPolicy
     DDS_PresentationQosPolicy_default =
         DDS_PresentationQosPolicy_default_value;

const DDS_EntityFactoryQosPolicy
     DDS_EntityFactoryQosPolicy_default =
         DDS_EntityFactoryQosPolicy_default_value;

const DDS_WriterDataLifecycleQosPolicy
     DDS_WriterDataLifecycleQosPolicy_default =
         DDS_WriterDataLifecycleQosPolicy_default_value;

const DDS_SchedulingQosPolicy
     DDS_SchedulingQosPolicy_default =
         DDS_SchedulingQosPolicy_default_value;

const DDS_UserDataQosPolicy
     DDS_UserDataQosPolicy_default =
         DDS_UserDataQosPolicy_default_value;

const DDS_TopicDataQosPolicy
     DDS_TopicDataQosPolicy_default =
         DDS_TopicDataQosPolicy_default_value;

const DDS_GroupDataQosPolicy
     DDS_GroupDataQosPolicy_default =
         DDS_GroupDataQosPolicy_default_value;

const DDS_PartitionQosPolicy
     DDS_PartitionQosPolicy_default =
         DDS_PartitionQosPolicy_default_value;

const DDS_ReaderDataLifecycleQosPolicy
     DDS_ReaderDataLifecycleQosPolicy_default =
         DDS_ReaderDataLifecycleQosPolicy_default_value;

const DDS_TimeBasedFilterQosPolicy
     DDS_TimeBasedFilterQosPolicy_default =
         DDS_TimeBasedFilterQosPolicy_default_value;

const DDS_SubscriptionKeyQosPolicy
     DDS_SubscriptionKeyQosPolicy_default =
         DDS_SubscriptionKeyQosPolicy_default_value;

const DDS_ReaderLifespanQosPolicy
     DDS_ReaderLifespanQosPolicy_default =
         DDS_ReaderLifespanQosPolicy_default_value;

const DDS_ShareQosPolicy
     DDS_ShareQosPolicy_default =
         DDS_ShareQosPolicy_default_value;

const DDS_ViewKeyQosPolicy
     DDS_ViewKeyQosPolicy_default =
         DDS_ViewKeyQosPolicy_default_value;

const DDS_DurabilityServiceQosPolicy
     DDS_DurabilityServiceQosPolicy_default =
         DDS_DurabilityServiceQosPolicy_default_value;

const DDS_DomainParticipantFactoryQos
        DDS_DomainParticipantFactoryQos_default = {
            DDS_EntityFactoryQosPolicy_default_value
        };

const DDS_DomainParticipantFactoryQos *
    DDS_PARTICIPANTFACTORY_QOS_DEFAULT = &DDS_DomainParticipantFactoryQos_default;

const DDS_DomainParticipantQos
    DDS_DomainParticipantQos_default = {
        DDS_UserDataQosPolicy_default_value,
        DDS_EntityFactoryQosPolicy_default_value,
        DDS_SchedulingQosPolicy_default_value,
        DDS_SchedulingQosPolicy_default_value
    };

const DDS_DomainParticipantQos *
    DDS_PARTICIPANT_QOS_DEFAULT = &DDS_DomainParticipantQos_default;

const DDS_TopicQos
    DDS_TopicQos_default = {
        DDS_TopicDataQosPolicy_default_value,
        DDS_DurabilityQosPolicy_default_value,
        DDS_DurabilityServiceQosPolicy_default_value,
        DDS_DeadlineQosPolicy_default_value,
        DDS_LatencyBudgetQosPolicy_default_value,
        DDS_LivelinessQosPolicy_default_value,
        DDS_ReliabilityQosPolicy_default_value,
        DDS_DestinationOrderQosPolicy_default_value,
        DDS_HistoryQosPolicy_default_value,
        DDS_ResourceLimitsQosPolicy_default_value,
        DDS_TransportPriorityQosPolicy_default_value,
        DDS_LifespanQosPolicy_default_value,
        DDS_OwnershipQosPolicy_default_value
    };

const DDS_TopicQos *
    DDS_TOPIC_QOS_DEFAULT = &DDS_TopicQos_default;

const DDS_PublisherQos
    DDS_PublisherQos_default = {
        DDS_PresentationQosPolicy_default_value,
        DDS_PartitionQosPolicy_default_value,
        DDS_GroupDataQosPolicy_default_value,
        DDS_EntityFactoryQosPolicy_default_value
    };

const DDS_PublisherQos *
    DDS_PUBLISHER_QOS_DEFAULT = &DDS_PublisherQos_default;

const DDS_SubscriberQos
    DDS_SubscriberQos_default = {
        DDS_PresentationQosPolicy_default_value,
        DDS_PartitionQosPolicy_default_value,
        DDS_GroupDataQosPolicy_default_value,
        DDS_EntityFactoryQosPolicy_default_value,
        DDS_ShareQosPolicy_default_value
    };

const DDS_SubscriberQos *
    DDS_SUBSCRIBER_QOS_DEFAULT = &DDS_SubscriberQos_default;

#define DDS_DataReaderQos_default_value                 \
    {                                                   \
        DDS_DurabilityQosPolicy_default_value,          \
        DDS_DeadlineQosPolicy_default_value,            \
        DDS_LatencyBudgetQosPolicy_default_value,       \
        DDS_LivelinessQosPolicy_default_value,          \
        DDS_ReliabilityQosPolicy_default_value,         \
        DDS_DestinationOrderQosPolicy_default_value,    \
        DDS_HistoryQosPolicy_default_value,             \
        DDS_ResourceLimitsQosPolicy_default_value,      \
        DDS_UserDataQosPolicy_default_value,            \
        DDS_OwnershipQosPolicy_default_value,           \
        DDS_TimeBasedFilterQosPolicy_default_value,     \
        DDS_ReaderDataLifecycleQosPolicy_default_value, \
        DDS_SubscriptionKeyQosPolicy_default_value,     \
        DDS_ReaderLifespanQosPolicy_default_value,      \
        DDS_ShareQosPolicy_default_value                \
   }

const DDS_DataReaderQos
    DDS_DataReaderQos_default = DDS_DataReaderQos_default_value;

const DDS_DataReaderQos
    DDS_DataReaderQos_use_topic_qos = DDS_DataReaderQos_default_value;

#undef DDS_DataReaderQos_default_value

const DDS_DataReaderQos *
    DDS_DATAREADER_QOS_DEFAULT = &DDS_DataReaderQos_default;

const DDS_DataReaderQos *
    DDS_DATAREADER_QOS_USE_TOPIC_QOS = &DDS_DataReaderQos_use_topic_qos;

const DDS_DataReaderViewQos
    DDS_DataReaderViewQos_default = {
        DDS_ViewKeyQosPolicy_default_value
    };

const DDS_DataReaderViewQos *
    DDS_DATAREADERVIEW_QOS_DEFAULT = &DDS_DataReaderViewQos_default;

#define DDS_DataWriterQos_default_value                 \
    {                                                   \
        DDS_DurabilityQosPolicy_default_value,          \
        DDS_DeadlineQosPolicy_default_value,            \
        DDS_LatencyBudgetQosPolicy_default_value,       \
        DDS_LivelinessQosPolicy_default_value,          \
        DDS_ReliabilityQosPolicy_writer_default_value,  \
        DDS_DestinationOrderQosPolicy_default_value,    \
        DDS_HistoryQosPolicy_default_value,             \
        DDS_ResourceLimitsQosPolicy_default_value,      \
        DDS_TransportPriorityQosPolicy_default_value,   \
        DDS_LifespanQosPolicy_default_value,            \
        DDS_UserDataQosPolicy_default_value,            \
        DDS_OwnershipQosPolicy_default_value,           \
        DDS_OwnershipStrengthQosPolicy_default_value,   \
        DDS_WriterDataLifecycleQosPolicy_default_value  \
    }

const DDS_DataWriterQos
    DDS_DataWriterQos_default = DDS_DataWriterQos_default_value;

const DDS_DataWriterQos
    DDS_DataWriterQos_use_topic_qos = DDS_DataWriterQos_default_value;

#undef DDS_DataWriterQos_default_value

const DDS_DataWriterQos *
    DDS_DATAWRITER_QOS_DEFAULT = &DDS_DataWriterQos_default;

const DDS_DataWriterQos *
    DDS_DATAWRITER_QOS_USE_TOPIC_QOS = &DDS_DataWriterQos_use_topic_qos;

/* Sequence support routines */

DDS_string
DDS_StringSeq_to_string (
    const DDS_StringSeq *sequence,
    const DDS_string delimiter)
{
    unsigned long i;
    unsigned long size = 0;
    DDS_string   rstring = NULL;

    assert(sequence != NULL);
    assert(delimiter != NULL);

    for ( i = 0; i < sequence->_length; i++ ) {
        size += strlen(sequence->_buffer[i]);
    }
    if ( size > 0 ) {
        size += (sequence->_length * strlen(delimiter)) + 1;
        rstring = os_malloc(size);
        rstring[0] = '\0';
        for ( i = 0; i < sequence->_length; i++ ) {
            if ( sequence->_buffer[i] ) {
                if ( i != 0 ) {
                    os_strcat(rstring, delimiter);
                }
                os_strcat(rstring, sequence->_buffer[i]);
            }
        }
    } else {
        rstring = (DDS_string)os_malloc(1);
        rstring[0] = '\0';
    }
    return rstring;
}

DDS_ReturnCode_t
DDS_ReturnCode_get(
    u_result uResult)
{
    DDS_ReturnCode_t result;

    switch (uResult) {
    case U_RESULT_UNDEFINED:
        result = DDS_RETCODE_ERROR;
    break;
    case U_RESULT_OK:
        result = DDS_RETCODE_OK;
    break;
    case U_RESULT_INTERRUPTED:
        result = DDS_RETCODE_ERROR;
    break;
    case U_RESULT_NOT_INITIALISED:
        result = DDS_RETCODE_ERROR;
    break;
    case U_RESULT_OUT_OF_MEMORY:
        result = DDS_RETCODE_OUT_OF_RESOURCES;
    break;
    case U_RESULT_INTERNAL_ERROR:
        result = DDS_RETCODE_ERROR;
    break;
    case U_RESULT_ILL_PARAM:
        result = DDS_RETCODE_BAD_PARAMETER;
    break;
    case U_RESULT_CLASS_MISMATCH:
        result = DDS_RETCODE_PRECONDITION_NOT_MET;
    break;
    case U_RESULT_DETACHING:
        result = DDS_RETCODE_ALREADY_DELETED;
    break;
    case U_RESULT_TIMEOUT:
        result = DDS_RETCODE_TIMEOUT;
    break;
    case U_RESULT_OUT_OF_RESOURCES:
        result = DDS_RETCODE_OUT_OF_RESOURCES;
    break;
    case U_RESULT_INCONSISTENT_QOS:
        result = DDS_RETCODE_INCONSISTENT_POLICY;
    break;
    case U_RESULT_IMMUTABLE_POLICY:
        result = DDS_RETCODE_IMMUTABLE_POLICY;
    break;
    case U_RESULT_PRECONDITION_NOT_MET:
        result = DDS_RETCODE_PRECONDITION_NOT_MET;
    break;
    case U_RESULT_ALREADY_DELETED:
        result = DDS_RETCODE_ALREADY_DELETED;
    break;
    case U_RESULT_HANDLE_EXPIRED:
        result = DDS_RETCODE_BAD_PARAMETER;
    break;
    case U_RESULT_NO_DATA:
        result = DDS_RETCODE_NO_DATA;
    break;
    case U_RESULT_UNSUPPORTED:
        result = DDS_RETCODE_UNSUPPORTED;
    break;
    default:
        result = DDS_RETCODE_ERROR;
    break;
    }
    return result;
}

const DDS_char *
DDS_ReturnCode_image(
    DDS_ReturnCode_t code)
{
    const DDS_char *image;

#define _CASE_(code) case code: image = #code; break
    switch (code) {
    _CASE_(DDS_RETCODE_OK);
    _CASE_(DDS_RETCODE_ERROR);
    _CASE_(DDS_RETCODE_UNSUPPORTED);
    _CASE_(DDS_RETCODE_BAD_PARAMETER);
    _CASE_(DDS_RETCODE_PRECONDITION_NOT_MET);
    _CASE_(DDS_RETCODE_OUT_OF_RESOURCES);
    _CASE_(DDS_RETCODE_NOT_ENABLED);
    _CASE_(DDS_RETCODE_IMMUTABLE_POLICY);
    _CASE_(DDS_RETCODE_INCONSISTENT_POLICY);
    _CASE_(DDS_RETCODE_ALREADY_DELETED);
    _CASE_(DDS_RETCODE_TIMEOUT);
    _CASE_(DDS_RETCODE_NO_DATA);
    _CASE_(DDS_RETCODE_ILLEGAL_OPERATION);
    break;
    default:
        image = "Illegal return code value";
    break;
    }
#undef _CASE_
    return image;
}

const DDS_char *
DDS_ObjectKind_image(
    DDS_ObjectKind kind)
{
    const DDS_char *image;

#define _CASE_(kind) case kind: image = #kind; break

    switch(kind) {
    _CASE_(DDS_UNDEFINED);

    /* Objects: */
    _CASE_(DDS_ERRORINFO);
    _CASE_(DDS_DOMAINFACTORY);
    _CASE_(DDS_TYPESUPPORT);
    _CASE_(DDS_WAITSET);
    _CASE_(DDS_DOMAIN);

    /* Conditions: */
    _CASE_(DDS_CONDITION);
    _CASE_(DDS_STATUSCONDITION);
    _CASE_(DDS_GUARDCONDITION);
    _CASE_(DDS_READCONDITION);
    _CASE_(DDS_QUERYCONDITION);

    /* Entities: */
    _CASE_(DDS_ENTITY);
    _CASE_(DDS_DOMAINPARTICIPANT);
    _CASE_(DDS_PUBLISHER);
    _CASE_(DDS_SUBSCRIBER);
    _CASE_(DDS_DATAWRITER);
    _CASE_(DDS_DATAREADER);
    _CASE_(DDS_DATAREADERVIEW);

    /* Topic Descriptions: */
    _CASE_(DDS_TOPICDESCRIPTION);
    _CASE_(DDS_TOPIC);
    _CASE_(DDS_CONTENTFILTEREDTOPIC);
    _CASE_(DDS_MULTITOPIC);

    _CASE_(DDS_QOSPROVIDER);
    _CASE_(DDS_OBJECT_COUNT);

    default:
        image = "Invalid ObjectKind";
    break;
    }
#undef _CASE_
    return image;
}


v_eventMask
DDS_StatusMask_get_eventMask(
    DDS_StatusMask statusMask)
{
    v_eventMask mask = 0;
    if (statusMask & DDS_INCONSISTENT_TOPIC_STATUS) {
        mask |= V_EVENT_INCONSISTENT_TOPIC;
    }
    if (statusMask & DDS_LIVELINESS_LOST_STATUS) {
        mask |= V_EVENT_LIVELINESS_LOST;
    }
    if (statusMask & DDS_OFFERED_DEADLINE_MISSED_STATUS) {
        mask |= V_EVENT_OFFERED_DEADLINE_MISSED;
    }
    if (statusMask & DDS_OFFERED_INCOMPATIBLE_QOS_STATUS) {
        mask |= V_EVENT_OFFERED_INCOMPATIBLE_QOS;
    }
    if (statusMask & DDS_DATA_ON_READERS_STATUS) {
        mask |= V_EVENT_ON_DATA_ON_READERS;
    }
    if (statusMask & DDS_SAMPLE_LOST_STATUS) {
        mask |= V_EVENT_SAMPLE_LOST;
    }
    if (statusMask & DDS_DATA_AVAILABLE_STATUS) {
        mask |= V_EVENT_DATA_AVAILABLE;
    }
    if (statusMask & DDS_SAMPLE_REJECTED_STATUS) {
        mask |= V_EVENT_SAMPLE_REJECTED;
    }
    if (statusMask & DDS_LIVELINESS_CHANGED_STATUS) {
        mask |= V_EVENT_LIVELINESS_CHANGED;
    }
    if (statusMask & DDS_REQUESTED_DEADLINE_MISSED_STATUS) {
        mask |= V_EVENT_REQUESTED_DEADLINE_MISSED;
    }
    if (statusMask & DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS) {
        mask |= V_EVENT_REQUESTED_INCOMPATIBLE_QOS;
    }
    if (statusMask & DDS_PUBLICATION_MATCHED_STATUS) {
        mask |= V_EVENT_PUBLICATION_MATCHED;
    }
    if (statusMask & DDS_SUBSCRIPTION_MATCHED_STATUS) {
        mask |= V_EVENT_SUBSCRIPTION_MATCHED;
    }
    if (statusMask & DDS_ALL_DATA_DISPOSED_TOPIC_STATUS) {
        mask |= V_EVENT_ALL_DATA_DISPOSED;
    }
    return mask;
}

DDS_boolean
DDS_sequence_is_valid (
    const _DDS_sequence seq)
{
    DDS_boolean  valid = TRUE;

    if ( seq ) {
        if (seq->_maximum > 0 && seq->_buffer == NULL ) {
            valid = FALSE;
            SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Sequence _maximum > 0 but _buffer = NULL");
        }
        if (seq->_maximum == 0 && seq->_buffer != NULL ) {
            valid = FALSE;
            SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Sequence _maximum = 0 but _buffer != NULL");
        }
        if (seq->_length > seq->_maximum ) {
            valid = FALSE;
            SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Sequence _length > _maximum");
        }
    } else {
        valid = FALSE;
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Sequence = NULL");
    }
    return valid;
}

DDS_boolean
DDS_Duration_is_valid(
    const DDS_Duration_t *duration)
{
    DDS_boolean valid;

    /* Duration is valid, only when range below 1 billion,
     * or when both fields are equal to DURATION_INFINITE.
     */
    if ( duration ) {
        if ( (duration->sec == DDS_DURATION_INFINITE_SEC &&
              duration->nanosec == DDS_DURATION_INFINITE_NSEC) ||
             (duration->nanosec < 1000000000ULL) ) {
            valid = TRUE;
        } else {
            valid = FALSE;
            SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Invalid duration, seconds = %d, nanoseconds = %d",
                        duration->sec, duration->nanosec);
        }
    } else {
        valid = FALSE;
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Bad parameter: duration = NULL");
    }

    return valid;
}

DDS_boolean
DDS_StringSeq_is_valid (
    const DDS_StringSeq *seq)
{
    DDS_boolean valid = TRUE;

    if ( seq != NULL ) {
        if ( DDS_sequence_is_valid((void *)seq) ) {
            DDS_unsigned_long i;

            for ( i = 0; valid && (i < seq->_length); i++ ) {
                if ( !seq->_buffer[i] ) {
                    valid = FALSE;
                    SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "String sequence element %d = NULL", i);
                }
            }
        } else {
            valid = FALSE;
        }
    } else {
        valid = FALSE;
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "String sequence = NULL");
    }

    return valid;
}

DDS_StringSeq *
DDS_StringSeq_dup (
    const DDS_StringSeq *src)
{
    DDS_StringSeq *dst = NULL;
    DDS_unsigned_long i;

    if (src) {
        dst = DDS_StringSeq__alloc();

        if ( dst != NULL ) {
            dst->_maximum = src->_length;
            dst->_length  = src->_length;
            dst->_release = TRUE;
            if ( src->_length > 0 ) {
                assert(src->_buffer);
                dst->_buffer  = DDS_StringSeq_allocbuf (src->_length);
                for (i = 0; i < src->_length; i++) {
                    dst->_buffer[i] = DDS_string_dup(src->_buffer[i]);
                }
            } else {
                dst->_buffer  = NULL;
            }
        }
    }
    return dst;
}

DDS_boolean
DDS_StringSeq_set_length (
    DDS_StringSeq *seq,
    DDS_unsigned_long len)
{
    DDS_boolean result = TRUE;
    DDS_string *buffer = NULL;

    if ( seq->_maximum > 0UL ) {
        assert(seq->_buffer);
        if ( len != seq->_maximum ) {
            buffer = DDS_StringSeq_allocbuf(len);

            if ( buffer ) {
                if ( seq->_release ) {
                    DDS_free(seq->_buffer);
                }
                seq->_release = TRUE;
                seq->_maximum = len;
            } else {
                result = FALSE;
            }
        } else {
            buffer = seq->_buffer;
        }
    } else {
        buffer = DDS_StringSeq_allocbuf(len);
        if ( buffer ) {
            seq->_release = TRUE;
            seq->_maximum = len;
        } else {
            result = FALSE;
        }
    }

    if ( result ) {
        seq->_length = len;
        seq->_buffer = buffer;
    }

    return result;
}

DDS_string
DDS_StringSeq_to_String (
    const DDS_StringSeq *sequence,
    const DDS_string delimiter)
{
    unsigned long i;
    unsigned long size = 0;
    DDS_string   rstring = NULL;

    assert(sequence != NULL);
    assert(delimiter != NULL);

    for ( i = 0; i < sequence->_length; i++ ) {
        size += strlen(sequence->_buffer[i]);
    }

    if ( size > 0 ) {
        size += (sequence->_length * strlen(delimiter)) + 1;
        rstring = (DDS_string) os_malloc(size);
        rstring[0] = '\0';
        for ( i = 0; i < sequence->_length; i++ ) {
            if ( sequence->_buffer[i] ) {
                if ( i != 0 ) {
                    os_strcat(rstring, delimiter);
                }
                os_strcat(rstring, sequence->_buffer[i]);
            }
        }
    } else {
        rstring = os_malloc(1);
        rstring[0] = '\0';
    }

    return rstring;
}

DDS_boolean
DDS_string_to_StringSeq (
    const DDS_string  string,
    const DDS_string  delimiter,
    DDS_StringSeq *sequence)
{
    DDS_boolean result = TRUE;
    DDS_unsigned_long size  = 0UL;
    DDS_unsigned_long i;
    DDS_string str;
    c_iter iter;

    assert(delimiter);
    assert(sequence);

    if ( string ) {
        iter = c_splitString(string, delimiter);
        if ( iter ) {
            size = c_iterLength(iter);
            if ( DDS_StringSeq_set_length(sequence, size) ) {
                for ( i = 0UL; i < size; i++ ) {
                    str = (DDS_string)c_iterTakeFirst(iter);
                    DDS_string_replace(str, &sequence->_buffer[i]);
                    os_free(str);
                    if ( !sequence->_buffer[i] ) {
                         result = FALSE;
                    }
                }
            }
            c_iterFree(iter);
        } else {
            result = FALSE;
        }
    } else {
        result = DDS_StringSeq_set_length(sequence, size);
    }
    return result;
}

DDS_ReturnCode_t
DDS_Time_copyIn (
    const DDS_Time_t *from,
    os_timeW *to,
    os_int64 maxSupportedSeconds)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;

    assert(to);

    if (from) {
        os_int64 sec = from->sec;
        if ((from->sec == DDS_TIMESTAMP_INVALID_SEC) && (from->nanosec == DDS_TIMESTAMP_INVALID_NSEC)) {
            *to = OS_TIMEW_INVALID;
        } else if (sec > maxSupportedSeconds) {
            result = DDS_RETCODE_UNSUPPORTED;
            if (maxSupportedSeconds < OS_TIME_MAX_VALID_SECONDS) {
                SAC_REPORT(result, "Time value [%"PA_PRId64".%u] is not supported, support for time beyond year 2038 is not enabled",
                          sec, from->nanosec);
            } else {
                SAC_REPORT(result, "Time value [%"PA_PRId64".%u] is not supported the time is too large", sec, from->nanosec);
            }
        } else if ((from->sec >= 0) && (from->nanosec < 1000000000)) {
            *to = OS_TIMEW_INIT(from->sec, from->nanosec);
        } else {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "Time is invalid (seconds=%"PA_PRId64", nanoseconds=%u)", sec, from->nanosec);
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "Time = NULL");
    }

    return result;
}

DDS_ReturnCode_t
DDS_Time_copyOut (
    const os_timeW *from,
    DDS_Time_t *to)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;

    assert(from);
    assert(!OS_TIMEW_ISINFINITE(*from));

    if (to) {
        if (OS_TIMEW_ISINVALID(*from)) {
            to->sec = DDS_TIMESTAMP_INVALID_SEC;
            to->nanosec = DDS_TIMESTAMP_INVALID_NSEC;
        } else {
            assert(OS_TIMEW_ISNORMALIZED(*from));
            to->sec = OS_TIMEW_GET_SECONDS(*from);
            to->nanosec = OS_TIMEW_GET_NANOSECONDS(*from);
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "Time = NULL");
    }

    return result;
}

DDS_boolean
DDS_Time_is_valid (
    const DDS_Time_t *time,
    os_int64 maxSupportedSeconds)
{
    DDS_boolean  valid = TRUE;
    os_int64 sec = time->sec;

    if ((time->sec == DDS_TIMESTAMP_INVALID_SEC) && (time->nanosec == DDS_TIMESTAMP_INVALID_NSEC)) {
        valid = FALSE;
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Time is invalid");

    } else if ((time->sec < 0) || (time->nanosec >= 1000000000)) {
        valid = FALSE;
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Time is invalid (seconds=%"PA_PRId64", nanoseconds=%u)",
                   sec, time->nanosec);
    } else if (sec > maxSupportedSeconds) {
         valid = FALSE;
         if (maxSupportedSeconds < OS_TIME_MAX_VALID_SECONDS) {
             SAC_REPORT(DDS_RETCODE_BAD_PARAMETER,
                        "Time value [%"PA_PRId64".%u] is not supported, support for time beyond year 2038 is not enabled",
                        sec, time->nanosec);
         } else {
             SAC_REPORT(DDS_RETCODE_BAD_PARAMETER,
                        "Time value [%"PA_PRId64".%u] is not supported the time is too large", sec, time->nanosec);
         }
    }

    return valid;
}

static DDS_boolean
validBoolean(
    DDS_boolean value)
{
    return ( (value == FALSE) || (value == TRUE) );
}

static DDS_boolean
validUserDataQosPolicy (
    const DDS_UserDataQosPolicy *policy)
{
    DDS_boolean valid;
    valid = DDS_sequence_is_valid((void *)&policy->value);
    if (!valid) {
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Invalid UserData policy");
    }
    return valid;
}

static DDS_boolean
validTopicDataQosPolicy (
    const DDS_TopicDataQosPolicy *policy)
{
    DDS_boolean valid;
    valid = DDS_sequence_is_valid((void *)&policy->value);
    if (!valid) {
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Invalid TopicData policy");
    }
    return valid;
}

static DDS_boolean
validGroupDataQosPolicy (
    const DDS_GroupDataQosPolicy *policy)
{
    DDS_boolean valid;
    valid = DDS_sequence_is_valid((void *)&policy->value);
    if (!valid) {
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Invalid GroupData policy");
    }
    return valid;
}

static DDS_boolean
validPartitionQosPolicy (
    const DDS_PartitionQosPolicy *policy)
{
    DDS_boolean valid;
    valid = DDS_sequence_is_valid((void *)&policy->name);
    if (!valid) {
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Invalid Partition policy");
    }
    return valid;
}

static DDS_boolean
validDurabilityQosPolicy (
    const DDS_DurabilityQosPolicy *policy)
{
    DDS_boolean valid;

    switch (policy->kind) {
    case DDS_VOLATILE_DURABILITY_QOS:
    case DDS_TRANSIENT_DURABILITY_QOS:
    case DDS_TRANSIENT_LOCAL_DURABILITY_QOS:
    case DDS_PERSISTENT_DURABILITY_QOS:
        valid = TRUE;
    break;
    default:
        valid = FALSE;
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Invalid durability kind (%d)", policy->kind);
    }
    return valid;
}

static DDS_boolean
validDurabilityServiceQosPolicy (
    const DDS_DurabilityServiceQosPolicy *policy)
{
    DDS_boolean valid = TRUE;

    if ( (policy->history_kind != DDS_KEEP_LAST_HISTORY_QOS)  &&
         (policy->history_kind != DDS_KEEP_ALL_HISTORY_QOS) ) {
        valid = FALSE;
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Invalid DurabilityService.history_kind (%d)", policy->history_kind);
    } else {
        if ( policy->history_kind == DDS_KEEP_LAST_HISTORY_QOS ) {
            if ( policy->history_depth <= 0 ) {
                valid = FALSE;
                SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Invalid DurabilityService.history_kind (%d)", policy->history_kind);
            }
        }
    }
    if (( policy->max_samples < -1  ) ||
        ( policy->max_instances < -1 ) ||
        ( policy->max_samples_per_instance < -1 ))
    {
        valid = FALSE;
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Invalid DurabilityService history limits, "
                    "max_samples (%d), max_instances (%d), max_samples_per_instance (%d)",
                     policy->max_samples, policy->max_instances, policy->max_samples_per_instance);
    }
    if (!DDS_Duration_is_valid(&policy->service_cleanup_delay)) {
        valid = FALSE;
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Invalid DurabilityService service_cleanup_delay (%d)",
                     policy->service_cleanup_delay);
    }
    return valid;
}

static DDS_boolean
validLifespanQosPolicy (
    const DDS_LifespanQosPolicy *policy)
{
    DDS_boolean valid;
    valid = DDS_Duration_is_valid(&policy->duration);
    if (!valid) {
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Invalid Lifespan duration (%d)", policy->duration);
    }
    return valid;
}

static DDS_boolean
validDeadlineQosPolicy (
    const DDS_DeadlineQosPolicy *policy)
{
    DDS_boolean valid;
    valid = DDS_Duration_is_valid(&policy->period);
    if (!valid) {
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Invalid Deadline period (%d)", policy->period);
    }
    return valid;
}

static DDS_boolean
validPresentationQosPolicy (
    const DDS_PresentationQosPolicy *policy)
{
    DDS_boolean valid = TRUE;

    if ( ( !validBoolean(policy->coherent_access) ) ||
         ( !validBoolean(policy->ordered_access) ))
    {
        valid = FALSE;
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Invalid boolean value for coherent_access = %d "
                    "or for ordered_access = %d",
                     policy->coherent_access, policy->ordered_access);
    }
    switch (policy->access_scope) {
    case DDS_INSTANCE_PRESENTATION_QOS:
    case DDS_TOPIC_PRESENTATION_QOS:
    case DDS_GROUP_PRESENTATION_QOS:
    break;
    default:
        valid = FALSE;
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Invalid access_scope = %d ", policy->access_scope);
    }

    return valid;
}

static DDS_boolean
validOwnershipQosPolicy (
    const DDS_OwnershipQosPolicy *policy)
{
    DDS_boolean valid = TRUE;

    if ( (policy->kind != DDS_SHARED_OWNERSHIP_QOS) &&
         (policy->kind != DDS_EXCLUSIVE_OWNERSHIP_QOS) )
    {
        valid = FALSE;
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Invalid ownership kind = %d ", policy->kind);
    }
    return valid;
}

static DDS_boolean
validLivelinessQosPolicy (
    const DDS_LivelinessQosPolicy *policy)
{
    DDS_boolean valid = TRUE;

    switch (policy->kind) {
    case DDS_AUTOMATIC_LIVELINESS_QOS:
    case DDS_MANUAL_BY_PARTICIPANT_LIVELINESS_QOS:
    case DDS_MANUAL_BY_TOPIC_LIVELINESS_QOS:
    break;
    default:
        valid = FALSE;
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Invalid Liveliness kind = %d ", policy->kind);
    }
    if ( !DDS_Duration_is_valid(&policy->lease_duration) ) {
        valid = FALSE;
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Invalid Liveliness lease_duration = %d ", policy->lease_duration);
    }
    return valid;
}

static DDS_boolean
validTimeBasedFilterQosPolicy (
    const DDS_TimeBasedFilterQosPolicy *policy)
{
    DDS_boolean valid;
    valid = DDS_Duration_is_valid(&policy->minimum_separation);
    if (!valid) {
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Invalid Deadline period (%d)", policy->minimum_separation);
    }
    return valid;
}


static DDS_boolean
validReliabilityQosPolicy (
    const DDS_ReliabilityQosPolicy *policy)
{
    DDS_boolean valid = TRUE;

    if ( (policy->kind != DDS_BEST_EFFORT_RELIABILITY_QOS) &&
         (policy->kind != DDS_RELIABLE_RELIABILITY_QOS) )
    {
        valid = FALSE;
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Invalid Reliability kind = %d ", policy->kind);
    }
    return valid;
}

static DDS_boolean
validDestinationOrderQosPolicy (
    const DDS_DestinationOrderQosPolicy *policy)
{
    DDS_boolean valid = TRUE;

    if ( (policy->kind != DDS_BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS) &&
         (policy->kind != DDS_BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS) )
    {
        valid = FALSE;
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Invalid DestinationOrder kind = %d ", policy->kind);
    }
    return valid;
}

static DDS_boolean
validHistoryQosPolicy (
    const DDS_HistoryQosPolicy *policy)
{
    DDS_boolean valid = TRUE;

    if ( (policy->kind != DDS_KEEP_LAST_HISTORY_QOS) &&
         (policy->kind != DDS_KEEP_ALL_HISTORY_QOS) ) {
        valid = FALSE;
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Invalid History kind = %d ", policy->kind);
    } else {
        if ( policy->kind == DDS_KEEP_LAST_HISTORY_QOS ) {
            if ( policy->depth <= 0 ) {
                valid = FALSE;
                SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Invalid History depth = %d ", policy->depth);
            }
        }
    }
    return valid;
}

static DDS_boolean
validInvalidSampleVisibilityQosPolicy(
    const DDS_InvalidSampleVisibilityQosPolicy *policy)
{
    DDS_boolean valid = TRUE;
    switch (policy->kind) {
    case DDS_ALL_INVALID_SAMPLES:
    case DDS_MINIMUM_INVALID_SAMPLES:
    case DDS_NO_INVALID_SAMPLES:
    break;
    default:
        valid = FALSE;
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Invalid InvalidSampleVisibility kind = %d ", policy->kind);
    }
    return valid;
}

static DDS_boolean
validWriterDataLifecycleQosPolicy (
    const DDS_WriterDataLifecycleQosPolicy *policy)
{
    return validBoolean(policy->autodispose_unregistered_instances);
}

static DDS_boolean
validReaderDataLifecycleQosPolicy (
    const DDS_ReaderDataLifecycleQosPolicy *policy)
{
    DDS_boolean valid = TRUE;

    if (!DDS_Duration_is_valid(&policy->autopurge_nowriter_samples_delay)) {
        valid = FALSE;
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Invalid autopurge_nowriter_samples_delay = %d",
                     policy->autopurge_nowriter_samples_delay);
    }
    if (!DDS_Duration_is_valid(&policy->autopurge_disposed_samples_delay)) {
        valid = FALSE;
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Invalid autopurge_disposed_samples_delay = %d",
                     policy->autopurge_disposed_samples_delay);
    }
    if (!validInvalidSampleVisibilityQosPolicy(&policy->invalid_sample_visibility)) {
        valid = FALSE;
    }
#ifdef USE_OLD_ReaderDataLifecycleQosPolicy
    if ( !validBoolean(policy->enable_invalid_samples) ) {
        valid = FALSE;
    }
#endif
    if ( !validBoolean(policy->autopurge_dispose_all) ) {
        valid = FALSE;
    }

    return valid;
}

static DDS_boolean
validEntityFactoryQosPolicy (
    const DDS_EntityFactoryQosPolicy *policy)
{
    return validBoolean(policy->autoenable_created_entities);
}

static DDS_boolean
validLatencyBudgetQosPolicy (
    const DDS_LatencyBudgetQosPolicy *policy)
{
    DDS_boolean valid;
    valid = DDS_Duration_is_valid(&policy->duration);
    if (!valid) {
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Invalid Latency duration (%d)", policy->duration);
    }
    return valid;
}

static DDS_boolean
validResourceLimitsQosPolicy (
    const DDS_ResourceLimitsQosPolicy *policy)
{
    DDS_boolean valid = TRUE;

    if ( (policy->max_samples <= 0) &&
         (policy->max_samples  != DDS_LENGTH_UNLIMITED) ) {
        valid = FALSE;
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Invalid max_samples = %d",
                     policy->max_samples);
    }
    if ( (policy->max_instances <= 0) &&
         (policy->max_instances  != DDS_LENGTH_UNLIMITED) ) {
        valid = FALSE;
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Invalid max_instances = %d",
                     policy->max_instances);
    }
    if ( (policy->max_samples_per_instance <= 0) &&
         (policy->max_samples_per_instance  != DDS_LENGTH_UNLIMITED) ) {
        valid = FALSE;
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Invalid max_samples_per_instance = %d",
                     policy->max_samples_per_instance);
    }
    if ( (policy->max_samples != DDS_LENGTH_UNLIMITED) &&
         (policy->max_samples_per_instance != DDS_LENGTH_UNLIMITED) &&
         (policy->max_samples < policy->max_samples_per_instance) ) {
        valid = FALSE;
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Invalid max_samples(%d) < max_samples_per_instance(%d)",
                policy->max_samples, policy->max_samples_per_instance);
    }
    return valid;
}

static DDS_boolean
validTransportPriorityQosPolicy (
    const DDS_TransportPriorityQosPolicy *policy)
{
    DDS_boolean valid = TRUE;

    OS_UNUSED_ARG(policy);

    return valid;
}

static DDS_boolean
validOwnershipStrengthQosPolicy (
    const DDS_OwnershipStrengthQosPolicy *policy)
{
    DDS_boolean valid = TRUE;

    OS_UNUSED_ARG(policy);

    return valid;
}

static DDS_boolean
validSubscriptionKeyQosPolicy (
    const DDS_SubscriptionKeyQosPolicy *policy)
{
    DDS_boolean valid = TRUE;

    if ( !validBoolean(policy->use_key_list) ) {
        valid = FALSE;
    }
    if ( valid && policy->use_key_list ) {
        if ( !DDS_sequence_is_valid((void *)&policy->key_list) ) {
            valid = FALSE;
            SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Invalid SubscriptionKey key_list");
        }
    }
    return valid;
}

static DDS_boolean
validViewKeyQosPolicy (
    const DDS_ViewKeyQosPolicy *policy)
{
    DDS_boolean valid = TRUE;

    if ( !validBoolean(policy->use_key_list) ) {
        valid = FALSE;
    }
    if ( valid && policy->use_key_list ) {
        if ( !DDS_sequence_is_valid((void *)&policy->key_list) ) {
            valid = FALSE;
            SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Invalid ViewKey key_list");
        }
    }
    return valid;
}

static DDS_boolean
validReaderLifespanQosPolicy (
    const DDS_ReaderLifespanQosPolicy *policy)
{
    DDS_boolean valid = TRUE;

    if ( !validBoolean(policy->use_lifespan) ) {
        valid = FALSE;
    }
    if ( valid && policy->use_lifespan ) {
        if ( !DDS_Duration_is_valid(&policy->duration) ) {
            valid = FALSE;
            SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Invalid ReaderLifespan duration = %d",
                         policy->duration);
        }
    }
    return valid;
}


static DDS_boolean
validShareQosPolicy (
    const DDS_ShareQosPolicy *policy)
{
    DDS_boolean valid = TRUE;

    if ( !validBoolean(policy->enable) ) {
        valid = FALSE;
    }
    if ( valid && policy->enable ) {
        if ( !policy->name ) {
            valid = FALSE;
            SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Invalid Share name = NULL");
        }
    }
    return valid;
}

static DDS_boolean
validSchedulingClassQosPolicy (
    const DDS_SchedulingClassQosPolicy *policy)
{
    DDS_boolean valid = TRUE;

    switch (policy->kind) {
    case DDS_SCHEDULE_REALTIME:
    case DDS_SCHEDULE_TIMESHARING:
    case DDS_SCHEDULE_DEFAULT:
    break;
    default:
        valid = FALSE;
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Invalid SchedulingClass kind = %d",
                     policy->kind);
    }
    return valid;
}

static DDS_boolean
validSchedulingPriorityQosPolicy (
    const DDS_SchedulingPriorityQosPolicy *policy)
{
    DDS_boolean valid = TRUE;

    if (policy->kind != DDS_PRIORITY_ABSOLUTE &&
        policy->kind != DDS_PRIORITY_RELATIVE) {
        valid = FALSE;
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Invalid SchedulingPriority kind = %d",
                     policy->kind);
    }
    return valid;
}

static DDS_boolean
validSchedulingQosPolicy (
    const DDS_SchedulingQosPolicy *policy)
{
    DDS_boolean valid = TRUE;

    if ( !validSchedulingClassQosPolicy(&policy->scheduling_class) ) {
        valid = FALSE;
    }
    if ( !validSchedulingPriorityQosPolicy(&policy->scheduling_priority_kind) ) {
        valid = FALSE;
    }
    return valid;
}

static DDS_boolean
consistentDurabilityServiceQosPolicy(
    const DDS_DurabilityServiceQosPolicy *policy)
{
    DDS_boolean valid = TRUE;

    if ( policy->history_kind == DDS_KEEP_LAST_HISTORY_QOS ) {
        if ( policy->max_samples_per_instance != DDS_LENGTH_UNLIMITED ) {
            if ( policy->history_depth >
                    policy->max_samples_per_instance ) {
                valid = FALSE;
                SAC_REPORT(DDS_RETCODE_INCONSISTENT_POLICY, "Resource_limits.max_samples_per_instance = %d"
                        "while history.depth = %d", policy->max_samples_per_instance, policy->history_depth);
            }
        }
    }
    return valid;
}

static DDS_boolean
consistentResourceDepthCombination(
    const DDS_ResourceLimitsQosPolicy *resourcePolicy,
    const DDS_HistoryQosPolicy *historyPolicy)
{
    DDS_boolean valid = TRUE;

    if ( historyPolicy->kind == DDS_KEEP_LAST_HISTORY_QOS ) {
        if ( resourcePolicy->max_samples_per_instance != DDS_LENGTH_UNLIMITED ) {
            if ( historyPolicy->depth >
                    resourcePolicy->max_samples_per_instance ) {
                valid = FALSE;
                SAC_REPORT(DDS_RETCODE_INCONSISTENT_POLICY, "Resource_limits.max_samples_per_instance = %d"
                        "while history.depth = %d", resourcePolicy->max_samples_per_instance, historyPolicy->depth);
            }
        }
    }
    return valid;
}

static DDS_boolean
consistentDeadlineTimeBasedFilterCombination(
    const DDS_DeadlineQosPolicy *deadlinePolicy,
    const DDS_TimeBasedFilterQosPolicy *filterPolicy)
{
    DDS_boolean valid = TRUE;

    if ((deadlinePolicy->period.sec < filterPolicy->minimum_separation.sec) ||
            ((deadlinePolicy->period.sec == filterPolicy->minimum_separation.sec) &&
             (deadlinePolicy->period.nanosec < filterPolicy->minimum_separation.nanosec))) {
        valid = FALSE;
        SAC_REPORT(DDS_RETCODE_INCONSISTENT_POLICY, "Deadline period (%d, %d) < TimeBasedFilterPeriod (%d, %d)",
                deadlinePolicy->period.sec, deadlinePolicy->period.nanosec,
                filterPolicy->minimum_separation.sec, filterPolicy->minimum_separation.nanosec);
    }
    return valid;
}

static DDS_boolean
consistentReaderDataLifecycleQosPolicy(
    const DDS_ReaderDataLifecycleQosPolicy *policy)
{
    DDS_boolean valid = TRUE;

#ifdef USE_OLD_ReaderDataLifecycleQosPolicy
    if (policy->enable_invalid_samples != TRUE) {
        /* TODO: improve deprecated policy reporting */
        SAC_REPORT_DEPRECATED("%s.%s.%s is deprecated and will be replaced by %s.%s.%s\n"
                    "Mixed usage of a deprecated policy and its replacement counterpart will trigger an inconsistent QoS error!",
                    "DataReaderQos",
                    DDS_READERDATALIFECYCLE_QOS_POLICY_NAME,
                    "enable_invalid_samples",
                    "DataReaderQos",
                    DDS_READERDATALIFECYCLE_QOS_POLICY_NAME,
                    "invalid_sample_visibility");
        if (policy->invalid_sample_visibility.kind != DDS_MINIMUM_INVALID_SAMPLES) {
            /* TODO: improve inconsistent policy report */
            SAC_REPORT(DDS_RETCODE_INCONSISTENT_POLICY, "%s %s.%s inconsistent with %s.%s",
                        "DataReaderQos",
                        DDS_READERDATALIFECYCLE_QOS_POLICY_NAME,
                        "enable_invalid_samples",
                        DDS_READERDATALIFECYCLE_QOS_POLICY_NAME,
                        "invalid_sample_visibility");
            valid = FALSE;
        }
    }
#endif
    return valid;
}

static DDS_boolean
supportedReaderDataLifecycleQosPolicy(
    const DDS_ReaderDataLifecycleQosPolicy *policy)
{
    DDS_boolean valid = TRUE;

    if (policy->invalid_sample_visibility.kind == DDS_ALL_INVALID_SAMPLES) {
        valid = FALSE;
        SAC_REPORT(DDS_RETCODE_UNSUPPORTED, "Used unsupported \"AllInvalidSamples\" sample visibility scope");
    }
    return valid;
}

DDS_ReturnCode_t
DDS_DomainParticipantFactoryQos_is_consistent (
    const DDS_DomainParticipantFactoryQos *qos)
{
    DDS_ReturnCode_t result;

    result = DDS_RETCODE_OK;
    if (qos == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "DomainParticipantFactoryQos = NULL");
    } else if (qos != DDS_PARTICIPANTFACTORY_QOS_DEFAULT) {
        if ( !validEntityFactoryQosPolicy(&qos->entity_factory) )
        {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "Detected invalid DomainParticipantFactoryQos");
        }
    }
    return result;
}

DDS_ReturnCode_t
DDS_DomainParticipantQos_is_consistent (
    const DDS_DomainParticipantQos *qos)
{
    DDS_ReturnCode_t result;

    result = DDS_RETCODE_OK;
    if (qos == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "DomainParticipantQos = NULL");
    } else if (qos != DDS_PARTICIPANT_QOS_DEFAULT) {
        if (!validUserDataQosPolicy(&qos->user_data) ||
            !validEntityFactoryQosPolicy(&qos->entity_factory) ||
            !validSchedulingQosPolicy(&qos->watchdog_scheduling) ||
            !validSchedulingQosPolicy(&qos->listener_scheduling) )
        {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "Detected invalid DomainParticipantQos");
        }
    }
    return result;
}


DDS_ReturnCode_t
DDS_TopicQos_is_consistent (
    const DDS_TopicQos *qos)
{
    DDS_ReturnCode_t result;

    result = DDS_RETCODE_OK;
    if (qos == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "TopicQos = NULL");
    } else if (qos != DDS_TOPIC_QOS_DEFAULT) {
        if ( !validTopicDataQosPolicy(&qos->topic_data)                 ||
             !validDurabilityQosPolicy(&qos->durability)                ||
             !validDurabilityServiceQosPolicy(&qos->durability_service) ||
             !validDeadlineQosPolicy(&qos->deadline)                    ||
             !validLatencyBudgetQosPolicy(&qos->latency_budget)         ||
             !validLivelinessQosPolicy(&qos->liveliness)                ||
             !validReliabilityQosPolicy(&qos->reliability)              ||
             !validDestinationOrderQosPolicy(&qos->destination_order)   ||
             !validHistoryQosPolicy(&qos->history)                      ||
             !validResourceLimitsQosPolicy(&qos->resource_limits)       ||
             !validTransportPriorityQosPolicy(&qos->transport_priority) ||
             !validLifespanQosPolicy(&qos->lifespan)                    ||
             !validOwnershipQosPolicy(&qos->ownership) )
        {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "Detected invalid TopicQos");
        } else if (!consistentResourceDepthCombination(&qos->resource_limits, &qos->history) ||
                   !consistentDurabilityServiceQosPolicy(&qos->durability_service)) {
            result = DDS_RETCODE_INCONSISTENT_POLICY;
            SAC_REPORT(result, "Detected inconsistent TopicQos");
        }
    }
    return result;
}

DDS_ReturnCode_t
DDS_PublisherQos_is_consistent (
    const DDS_PublisherQos *qos)
{
    DDS_ReturnCode_t result;

    result = DDS_RETCODE_OK;
    if (qos == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "PublisherQos = NULL");
    } else if (qos != DDS_PUBLISHER_QOS_DEFAULT) {
        if ( !validPresentationQosPolicy(&qos->presentation)    ||
             !validPartitionQosPolicy(&qos->partition)          ||
             !validGroupDataQosPolicy(&qos->group_data)         ||
             !validEntityFactoryQosPolicy(&qos->entity_factory) )
        {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "Detected invalid PublisherQos");
        } else {
            result = DDS_RETCODE_OK;
        }
    }
    return result;
}

DDS_ReturnCode_t
DDS_SubscriberQos_is_consistent (
    const DDS_SubscriberQos *qos)
{
    DDS_ReturnCode_t result;

    result = DDS_RETCODE_OK;
    if (qos == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "SubscriberQos = NULL");
    } else if (qos != DDS_SUBSCRIBER_QOS_DEFAULT) {
        if ( !validPresentationQosPolicy(&qos->presentation)    ||
             !validPartitionQosPolicy(&qos->partition)          ||
             !validGroupDataQosPolicy(&qos->group_data)         ||
             !validEntityFactoryQosPolicy(&qos->entity_factory) ||
             !validShareQosPolicy(&qos->share) )
        {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "Detected invalid SubscriberQos");
        } else {
            result = DDS_RETCODE_OK;
        }
    }
    return result;
}

DDS_ReturnCode_t
DDS_DataReaderQos_is_consistent (
    const DDS_DataReaderQos *qos)
{
    DDS_ReturnCode_t result;

    result = DDS_RETCODE_OK;
    if (qos == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "DataReaderQos = NULL");
    } else if ((qos != DDS_DATAREADER_QOS_DEFAULT) &&
               (qos != DDS_DATAREADER_QOS_USE_TOPIC_QOS))
    {
        if (!validDurabilityQosPolicy(&qos->durability)                     ||
            !validDeadlineQosPolicy(&qos->deadline)                         ||
            !validLatencyBudgetQosPolicy(&qos->latency_budget)              ||
            !validLivelinessQosPolicy(&qos->liveliness)                     ||
            !validReliabilityQosPolicy(&qos->reliability)                   ||
            !validDestinationOrderQosPolicy(&qos->destination_order)        ||
            !validHistoryQosPolicy(&qos->history)                           ||
            !validResourceLimitsQosPolicy(&qos->resource_limits)            ||
            !validUserDataQosPolicy(&qos->user_data)                        ||
            !validTimeBasedFilterQosPolicy(&qos->time_based_filter)         ||
            !validOwnershipQosPolicy(&qos->ownership)                       ||
            !validReaderDataLifecycleQosPolicy(&qos->reader_data_lifecycle) ||
            !validSubscriptionKeyQosPolicy(&qos->subscription_keys)         ||
            !validReaderLifespanQosPolicy(&qos->reader_lifespan)            ||
            !validShareQosPolicy(&qos->share) )
        {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "Detected invalid DataReaderQos");
        } else {
            if ( !consistentResourceDepthCombination(&qos->resource_limits, &qos->history) ||
                 !consistentDeadlineTimeBasedFilterCombination(&qos->deadline, &qos->time_based_filter) ||
                 !consistentReaderDataLifecycleQosPolicy(&qos->reader_data_lifecycle)) {
                result = DDS_RETCODE_INCONSISTENT_POLICY;
                SAC_REPORT(result, "Detected inconsistent DataReaderQos");
            }
            if (!supportedReaderDataLifecycleQosPolicy(&qos->reader_data_lifecycle)) {
                result = DDS_RETCODE_UNSUPPORTED; /* See OSPL-433 */
                SAC_REPORT(result, "Detected unsupported DataReaderQos");
            }
        }
    }
    return result;
}

DDS_ReturnCode_t
DDS_DataReaderViewQos_is_consistent (
    const DDS_DataReaderViewQos *qos)
{
    DDS_ReturnCode_t result;

    result = DDS_RETCODE_OK;
    if (qos == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "DataReaderViewQos = NULL");
    } else if (qos != DDS_DATAREADERVIEW_QOS_DEFAULT) {
        if ( !validViewKeyQosPolicy(&qos->view_keys) ) {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "Detected invalid DataReaderViewQos");
        }
    }
    return result;
}


DDS_ReturnCode_t
DDS_DataWriterQos_is_consistent (
    const DDS_DataWriterQos *qos)
{
    DDS_ReturnCode_t result;

    result = DDS_RETCODE_OK;
    if (qos == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "DataWriterQos = NULL");
    } else if ((qos != DDS_DATAWRITER_QOS_DEFAULT) &&
        (qos != DDS_DATAWRITER_QOS_USE_TOPIC_QOS))
    {
        if ( !validDurabilityQosPolicy(&qos->durability)                ||
             !validDeadlineQosPolicy(&qos->deadline)                    ||
             !validLatencyBudgetQosPolicy(&qos->latency_budget)         ||
             !validLivelinessQosPolicy(&qos->liveliness)                ||
             !validReliabilityQosPolicy(&qos->reliability)              ||
             !validDestinationOrderQosPolicy(&qos->destination_order)   ||
             !validHistoryQosPolicy(&qos->history)                      ||
             !validResourceLimitsQosPolicy(&qos->resource_limits)       ||
             !validTransportPriorityQosPolicy(&qos->transport_priority) ||
             !validLifespanQosPolicy(&qos->lifespan)                    ||
             !validUserDataQosPolicy(&qos->user_data)                   ||
             !validOwnershipQosPolicy(&qos->ownership)                  ||
             !validOwnershipStrengthQosPolicy(&qos->ownership_strength) ||
             !validWriterDataLifecycleQosPolicy(&qos->writer_data_lifecycle) )
        {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "Detected invalid DataWriterQos");
        } else if ( !consistentResourceDepthCombination(&qos->resource_limits, &qos->history)) {
            result = DDS_RETCODE_INCONSISTENT_POLICY;
            SAC_REPORT(result, "Detected inconsistent DataWriterQos");
        }
    }
    return result;
}

DDS_ReturnCode_t
DDS_sequence_init (
    _DDS_sequence seq)
{
    DDS_ReturnCode_t result = DDS_RETCODE_BAD_PARAMETER;

    if (seq != NULL) {
        seq->_buffer = NULL;
        seq->_maximum = 0;
        seq->_length = 0;
        seq->_release = FALSE;
        result = DDS_RETCODE_OK;
    }
    return result;
}

DDS_ReturnCode_t
DDS_sequence_deinit(
    _DDS_sequence seq)
{
    DDS_ReturnCode_t result;

    if (seq != NULL) {
        if (seq->_buffer != NULL) {
            DDS_free(seq->_buffer);
        }
        seq->_buffer = NULL;
        seq->_maximum = 0;
        seq->_length = 0;
        seq->_release = FALSE;
        result = DDS_RETCODE_OK;
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
    }
    return result;
}

DDS_ReturnCode_t
DDS_StringSeq_init(
    DDS_StringSeq *seq,
    const DDS_StringSeq *template)
{
    DDS_ReturnCode_t result;
    DDS_unsigned_long i;

    if (seq != NULL) {
        if (template != NULL) {
            seq->_maximum = template->_length;
            seq->_length = template->_length;
            seq->_release = (template->_length > 0);
            seq->_buffer = DDS_StringSeq_allocbuf(template->_length);
            for (i=0; i<template->_length; i++) {
                seq->_buffer[i] = DDS_string_dup(template->_buffer[i]);
            }
            result = DDS_RETCODE_OK;
        } else {
            result = DDS_sequence_init((_DDS_sequence)seq);
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "DDS_StringSeq is null");
    }
    return result;
}

DDS_ReturnCode_t
DDS_StringSeq_deinit(
    DDS_StringSeq *seq)
{
    return DDS_sequence_deinit((_DDS_sequence)seq);
}

DDS_ReturnCode_t
DDS_sequence_octet_init(
    DDS_sequence_octet *seq,
    const DDS_sequence_octet *template)
{
    DDS_ReturnCode_t result;

    if (seq != NULL) {
        if (template != NULL) {
            seq->_maximum = template->_length;
            seq->_length = template->_length;
            seq->_release = (template->_length > 0);
            seq->_buffer = DDS_sequence_octet_allocbuf(template->_length);
            memcpy(&seq->_buffer, &template->_buffer, template->_length);
            result = DDS_RETCODE_OK;
        } else {
            result = DDS_sequence_init((_DDS_sequence)seq);
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "DDS_sequence_octet is null");
    }
    return result;
}

DDS_ReturnCode_t
DDS_sequence_octet_deinit(
    DDS_sequence_octet *seq)
{
    return DDS_sequence_deinit((_DDS_sequence)seq);
}

DDS_ReturnCode_t
DDS_DomainParticipantFactoryQos_init (
    DDS_DomainParticipantFactoryQos *qos,
    const DDS_DomainParticipantFactoryQos *template)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;

    assert (template != NULL);

    if (qos == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "DomainParticipantFactoryQos = NULL");
    } else if (qos == DDS_PARTICIPANTFACTORY_QOS_DEFAULT) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "QoS 'PARTICIPANTFACTORY_QOS_DEFAULT' is read-only.");
    } else if (template != DDS_PARTICIPANTFACTORY_QOS_DEFAULT) {
        result = DDS_DomainParticipantFactoryQos_is_consistent(template);
    }

    if (result == DDS_RETCODE_OK) {
        qos->entity_factory = template->entity_factory;
    }

    return result;
}

DDS_ReturnCode_t
DDS_DomainParticipantFactoryQos_deinit (
    DDS_DomainParticipantFactoryQos *qos)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;

    if (qos == NULL || qos == &DDS_DomainParticipantFactoryQos_default) {
        result = DDS_RETCODE_BAD_PARAMETER;
    } else {
        qos->entity_factory = DDS_EntityFactoryQosPolicy_default;
    }

    return result;
}

DDS_ReturnCode_t
DDS_DomainParticipantQos_init (
    DDS_DomainParticipantQos *qos,
    const DDS_DomainParticipantQos *template)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;

    assert (template != NULL);

    if (qos == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "DomainParticipantQos = NULL");
    } else if (qos == DDS_PARTICIPANT_QOS_DEFAULT) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "QoS 'PARTICIPANT_QOS_DEFAULT' is read-only.");
    } else if (template != DDS_PARTICIPANT_QOS_DEFAULT) {
        result = DDS_DomainParticipantQos_is_consistent(template);
    }

    if (result == DDS_RETCODE_OK) {
        DDS_sequence_clean((_DDS_sequence)&qos->user_data.value);
        result = DDS_sequence_octet_init(&qos->user_data.value,
                                         &template->user_data.value);
        if (result == DDS_RETCODE_OK) {
            qos->entity_factory = template->entity_factory;
            qos->watchdog_scheduling = template->watchdog_scheduling;
            qos->listener_scheduling = template->listener_scheduling;
        }
    }

    return result;
}

DDS_ReturnCode_t
DDS_DomainParticipantQos_deinit (
    DDS_DomainParticipantQos *qos)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;

    if (qos == NULL || qos == DDS_PARTICIPANT_QOS_DEFAULT) {
        result = DDS_RETCODE_BAD_PARAMETER;
    } else {
        result = DDS_sequence_octet_deinit(&qos->user_data.value);
        if (result == DDS_RETCODE_OK) {
            qos->entity_factory = DDS_EntityFactoryQosPolicy_default;
            qos->watchdog_scheduling = DDS_SchedulingQosPolicy_default;
            qos->listener_scheduling = DDS_SchedulingQosPolicy_default;
        } else {
            result = DDS_RETCODE_ERROR;
        }
    }

    return result;
}

DDS_ReturnCode_t
DDS_TopicQos_init (
    DDS_TopicQos *qos,
    const DDS_TopicQos *template)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;

    assert (template != NULL);

    if (qos == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "TopicQos = NULL");
    } else if (qos == DDS_TOPIC_QOS_DEFAULT) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "QoS 'TOPIC_QOS_DEFAULT' is read-only.");
    } else if (template != DDS_TOPIC_QOS_DEFAULT) {
        result = DDS_TopicQos_is_consistent(template);
    }

    if (result == DDS_RETCODE_OK) {
        DDS_sequence_clean((_DDS_sequence)&qos->topic_data.value);
        result = DDS_sequence_octet_init(&qos->topic_data.value,
                                         &template->topic_data.value);
        if (result == DDS_RETCODE_OK) {
            qos->durability = template->durability;
            qos->durability_service = template->durability_service;
            qos->deadline = template->deadline;
            qos->latency_budget = template->latency_budget;
            qos->liveliness = template->liveliness;
            qos->reliability = template->reliability;
            qos->destination_order = template->destination_order;
            qos->history = template->history;
            qos->resource_limits = template->resource_limits;
            qos->transport_priority = template->transport_priority;
            qos->lifespan = template->lifespan;
            qos->ownership = template->ownership;
        }
    }

    return result;
}

DDS_ReturnCode_t
DDS_TopicQos_deinit (
    DDS_TopicQos *qos)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;

    if (qos == NULL || qos == DDS_TOPIC_QOS_DEFAULT) {
        result = DDS_RETCODE_BAD_PARAMETER;
    } else {
        result = DDS_sequence_octet_deinit(&qos->topic_data.value);
        if (result == DDS_RETCODE_OK) {
            qos->durability = DDS_DurabilityQosPolicy_default;
            qos->durability_service = DDS_DurabilityServiceQosPolicy_default;
            qos->deadline = DDS_DeadlineQosPolicy_default;
            qos->latency_budget = DDS_LatencyBudgetQosPolicy_default;
            qos->liveliness = DDS_LivelinessQosPolicy_default;
            qos->reliability = DDS_ReliabilityQosPolicy_default;
            qos->destination_order = DDS_DestinationOrderQosPolicy_default;
            qos->history = DDS_HistoryQosPolicy_default;
            qos->resource_limits = DDS_ResourceLimitsQosPolicy_default;
            qos->transport_priority = DDS_TransportPriorityQosPolicy_default;
            qos->lifespan = DDS_LifespanQosPolicy_default;
            qos->ownership = DDS_OwnershipQosPolicy_default;
        } else {
            result = DDS_RETCODE_ERROR;
        }
    }

    return result;
}

DDS_ReturnCode_t
DDS_PublisherQos_init (
    DDS_PublisherQos *qos,
    const DDS_PublisherQos *template)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;

    assert (template != NULL);

    if (qos == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "PublisherQos = NULL");
    } else if (qos == DDS_PUBLISHER_QOS_DEFAULT) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "QoS 'PUBLISHER_QOS_DEFAULT' is read-only.");
    } else {
        result = DDS_PublisherQos_is_consistent(template);
    }

    if (result == DDS_RETCODE_OK) {
        DDS_sequence_clean((_DDS_sequence)&qos->partition.name);
        result = DDS_StringSeq_init(&qos->partition.name,
                                    &template->partition.name);
        if (result == DDS_RETCODE_OK) {
            DDS_sequence_clean((_DDS_sequence)&qos->group_data.value);
            result = DDS_sequence_octet_init(&qos->group_data.value,
                                             &template->group_data.value);
        }
        if (result == DDS_RETCODE_OK) {
            qos->presentation = template->presentation;
            qos->entity_factory = template->entity_factory;
        }
    }

    return result;
}

DDS_ReturnCode_t
DDS_PublisherQos_deinit (
    DDS_PublisherQos *qos)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;

    if (qos == NULL || qos == DDS_PUBLISHER_QOS_DEFAULT) {
        result = DDS_RETCODE_BAD_PARAMETER;
    } else {
        result = DDS_StringSeq_deinit(&qos->partition.name);
        if (result == DDS_RETCODE_OK) {
            result = DDS_sequence_octet_deinit(&qos->group_data.value);
        }
        if (result == DDS_RETCODE_OK) {
            qos->presentation = DDS_PresentationQosPolicy_default;
            qos->entity_factory = DDS_EntityFactoryQosPolicy_default;
        } else {
            result = DDS_RETCODE_ERROR;
        }
    }

    return result;
}

DDS_ReturnCode_t
DDS_DataWriterQos_init (
    DDS_DataWriterQos *qos,
    const DDS_DataWriterQos *template)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;

    assert (template != NULL);

    if (qos == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "DataWriterQos = NULL");
    } else if (qos == DDS_DATAWRITER_QOS_DEFAULT) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "QoS 'DATAWRITER_QOS_DEFAULT' is read-only.");
    } else if (qos == DDS_DATAWRITER_QOS_USE_TOPIC_QOS) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "QoS 'DATAWRITER_QOS_USE_TOPIC_QOS' is read-only.");
    } else if (template == DDS_DATAWRITER_QOS_USE_TOPIC_QOS) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "QoS 'DATAWRITER_QOS_USE_TOPIC_QOS' is invalid in this context.");
    } else if (template != DDS_DATAWRITER_QOS_DEFAULT) {
        result = DDS_DataWriterQos_is_consistent (template);
    }

    if (result == DDS_RETCODE_OK) {
        if (result == DDS_RETCODE_OK) {
            DDS_sequence_clean((_DDS_sequence)&qos->user_data.value);
            result = DDS_sequence_octet_init(&qos->user_data.value,
                                             &template->user_data.value);
        }
        if (result == DDS_RETCODE_OK) {
            qos->durability = template->durability;
            qos->deadline = template->deadline;
            qos->latency_budget = template->latency_budget;
            qos->liveliness = template->liveliness;
            qos->reliability = template->reliability;
            qos->destination_order = template->destination_order;
            qos->history = template->history;
            qos->resource_limits = template->resource_limits;
            qos->transport_priority = template->transport_priority;
            qos->lifespan = template->lifespan;
            qos->ownership = template->ownership;
            qos->ownership_strength = template->ownership_strength;
            qos->writer_data_lifecycle = template->writer_data_lifecycle;
        }
    }

    return result;
}

DDS_ReturnCode_t
DDS_DataWriterQos_deinit (
    DDS_DataWriterQos *qos)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;

    if (qos == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "DataWriterQos = NULL");
    } else if (qos == DDS_DATAWRITER_QOS_DEFAULT ||
               qos == DDS_DATAWRITER_QOS_USE_TOPIC_QOS)
    {
        result = DDS_RETCODE_BAD_PARAMETER;
    } else {
        result = DDS_sequence_octet_deinit(&qos->user_data.value);
        if (result == DDS_RETCODE_OK) {
            qos->durability = DDS_DurabilityQosPolicy_default;
            qos->deadline = DDS_DeadlineQosPolicy_default;
            qos->latency_budget = DDS_LatencyBudgetQosPolicy_default;
            qos->liveliness = DDS_LivelinessQosPolicy_default;
            qos->reliability = DDS_ReliabilityQosPolicy_default;
            qos->destination_order = DDS_DestinationOrderQosPolicy_default;
            qos->history = DDS_HistoryQosPolicy_default;
            qos->resource_limits = DDS_ResourceLimitsQosPolicy_default;
            qos->transport_priority = DDS_TransportPriorityQosPolicy_default;
            qos->lifespan = DDS_LifespanQosPolicy_default;
            qos->ownership = DDS_OwnershipQosPolicy_default;
            qos->ownership_strength = DDS_OwnershipStrengthQosPolicy_default;
            qos->writer_data_lifecycle = DDS_WriterDataLifecycleQosPolicy_default;
        } else {
            result = DDS_RETCODE_ERROR;
        }
    }

    return result;
}

DDS_ReturnCode_t
DDS_SubscriberQos_init (
    DDS_SubscriberQos *qos,
    const DDS_SubscriberQos *template)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;

    assert (template != NULL);

    if (qos == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "SubscriberQos = NULL");
    } else if (qos == DDS_SUBSCRIBER_QOS_DEFAULT) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "QoS 'SUBSCRIBER_QOS_DEFAULT' is read-only.");
    } else if (template != DDS_SUBSCRIBER_QOS_DEFAULT) {
        result = DDS_SubscriberQos_is_consistent(template);
    }

    if (result == DDS_RETCODE_OK) {
        DDS_sequence_clean((_DDS_sequence)&qos->partition.name);
        result = DDS_StringSeq_init(&qos->partition.name,
                                    &template->partition.name);
        if (result == DDS_RETCODE_OK) {
            DDS_sequence_clean((_DDS_sequence)&qos->group_data.value);
            result = DDS_sequence_octet_init(&qos->group_data.value,
                                             &template->group_data.value);
        }
        if (result == DDS_RETCODE_OK) {
            DDS_string_replace(template->share.name, &qos->share.name);
            qos->share.enable = template->share.enable;
            qos->presentation = template->presentation;
            qos->entity_factory = template->entity_factory;
        }
    }

    return result;
}

DDS_ReturnCode_t
DDS_SubscriberQos_deinit (
    DDS_SubscriberQos *qos)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;

    if (qos == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "SubscriberQos = NULL");
    } else if (qos == DDS_SUBSCRIBER_QOS_DEFAULT) {
        result = DDS_RETCODE_BAD_PARAMETER;
    } else {
        result = DDS_StringSeq_deinit(&qos->partition.name);
        if (result == DDS_RETCODE_OK) {
            result = DDS_sequence_octet_deinit(&qos->group_data.value);
        }
        if (result == DDS_RETCODE_OK) {
            DDS_free(qos->share.name);
            qos->share = DDS_ShareQosPolicy_default;
            qos->presentation = DDS_PresentationQosPolicy_default;
            qos->entity_factory = DDS_EntityFactoryQosPolicy_default;
        } else {
            result = DDS_RETCODE_ERROR;
        }
    }

    return result;
}

DDS_ReturnCode_t
DDS_DataReaderQos_init (
    DDS_DataReaderQos *qos,
    const DDS_DataReaderQos *template)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;

    assert (template != NULL);

    if (qos == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "DataReaderQos = NULL");
    } else if (qos == DDS_DATAREADER_QOS_DEFAULT) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "QoS 'DATAREADER_QOS_DEFAULT' is read-only.");
    } else if (qos == DDS_DATAREADER_QOS_USE_TOPIC_QOS) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "QoS 'DATAREADER_QOS_USE_TOPIC_QOS' is read-only.");
    } else if (template == DDS_DATAREADER_QOS_USE_TOPIC_QOS) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "QoS 'DATAREADER_QOS_USE_TOPIC_QOS' is invalid in this context.");
    } else if (template != DDS_DATAREADER_QOS_DEFAULT) {
        result = DDS_DataReaderQos_is_consistent(template);
    }

    if (result == DDS_RETCODE_OK) {
            DDS_sequence_clean((_DDS_sequence)&qos->user_data.value);
        result = DDS_sequence_octet_init(&qos->user_data.value,
                                         &template->user_data.value);
        if (result == DDS_RETCODE_OK) {
            DDS_sequence_clean((_DDS_sequence)&qos->subscription_keys.key_list);
            result = DDS_StringSeq_init(&qos->subscription_keys.key_list,
                                        &template->subscription_keys.key_list);
        }
        if (result == DDS_RETCODE_OK) {
            qos->durability = template->durability;
            qos->deadline = template->deadline;
            qos->latency_budget = template->latency_budget;
            qos->liveliness = template->liveliness;
            qos->reliability = template->reliability;
            qos->destination_order = template->destination_order;
            qos->history = template->history;
            qos->resource_limits = template->resource_limits;
            qos->ownership = template->ownership;
            qos->time_based_filter = template->time_based_filter;
            qos->reader_data_lifecycle = template->reader_data_lifecycle;
            qos->subscription_keys.use_key_list = template->subscription_keys.use_key_list;
            qos->reader_lifespan = template->reader_lifespan;
            qos->share.enable = template->share.enable;
            DDS_string_replace(template->share.name, &qos->share.name);
        }
    }

    return result;
}

DDS_ReturnCode_t
DDS_DataReaderQos_deinit (
    DDS_DataReaderQos *qos)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;

    if (qos == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "DataReaderQos = NULL");
    } else if (qos == DDS_DATAREADER_QOS_DEFAULT ||
               qos == DDS_DATAREADER_QOS_USE_TOPIC_QOS)
    {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "DataReaderQos = DDS_DATAREADER_QOS_DEFAULT or DDS_DATAREADER_QOS_USE_TOPIC_QOS which is not allowed.");
    }

    if (result == DDS_RETCODE_OK) {
        result = DDS_sequence_octet_deinit(&qos->user_data.value);
        if (result == DDS_RETCODE_OK) {
            result = DDS_StringSeq_deinit(&qos->subscription_keys.key_list);
        }
        if (result == DDS_RETCODE_OK) {
            qos->durability = DDS_DurabilityQosPolicy_default;
            qos->deadline = DDS_DeadlineQosPolicy_default;
            qos->latency_budget = DDS_LatencyBudgetQosPolicy_default;
            qos->liveliness = DDS_LivelinessQosPolicy_default;
            qos->reliability = DDS_ReliabilityQosPolicy_default;
            qos->destination_order = DDS_DestinationOrderQosPolicy_default;
            qos->history = DDS_HistoryQosPolicy_default;
            qos->resource_limits = DDS_ResourceLimitsQosPolicy_default;
            qos->ownership = DDS_OwnershipQosPolicy_default;
            qos->time_based_filter = DDS_TimeBasedFilterQosPolicy_default;
            qos->reader_data_lifecycle = DDS_ReaderDataLifecycleQosPolicy_default;
            qos->subscription_keys = DDS_SubscriptionKeyQosPolicy_default;
            qos->reader_lifespan = DDS_ReaderLifespanQosPolicy_default;
            DDS_free(qos->share.name);
            qos->share = DDS_ShareQosPolicy_default;
        } else {
            result = DDS_RETCODE_ERROR;
        }
    }

    return result;
}

DDS_ReturnCode_t
DDS_DataReaderViewQos_init (
    DDS_DataReaderViewQos *qos,
    const DDS_DataReaderViewQos *template)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;

    assert (template != NULL);

    if (qos == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "DataReaderViewQos = NULL");
    } else if (qos == DDS_DATAREADERVIEW_QOS_DEFAULT) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "QoS 'DATAREADERVIEW_QOS_DEFAULT' is read-only.");
    } else if (template != DDS_DATAVIEW_QOS_DEFAULT) {
        result = DDS_DataReaderViewQos_is_consistent(template);
    }

    if (result == DDS_RETCODE_OK) {
        DDS_sequence_clean((_DDS_sequence)&qos->view_keys.key_list);
        result = DDS_StringSeq_init(&qos->view_keys.key_list,
                                    &template->view_keys.key_list);
        if (result == DDS_RETCODE_OK) {
            qos->view_keys.use_key_list = template->view_keys.use_key_list;
        }
    }

    return result;
}

DDS_ReturnCode_t
DDS_DataReaderViewQos_deinit (
    DDS_DataReaderViewQos *qos)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;

    if (qos == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "DataReaderViewQos = NULL");
    } else if (qos == DDS_DATAVIEW_QOS_DEFAULT) {
        result = DDS_RETCODE_BAD_PARAMETER;
    }

    if (result == DDS_RETCODE_OK) {
        result = DDS_StringSeq_deinit(&qos->view_keys.key_list);
        if (result == DDS_RETCODE_OK) {
            qos->view_keys = DDS_ViewKeyQosPolicy_default;
        } else {
            result = DDS_RETCODE_ERROR;
        }
    }

    return result;
}

DDS_ReturnCode_t
DDS_Duration_init_mapping (
    const DDS_Duration_t *from,
    os_duration *to)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;

    if ( (from->sec     == DDS_DURATION_INFINITE_SEC) &&
         (from->nanosec == DDS_DURATION_INFINITE_NSEC) ) {
         *to = OS_DURATION_INFINITE;
    } else {
        *to = OS_DURATION_INIT(from->sec, from->nanosec);
    }
    return result;
}

DDS_ReturnCode_t
DDS_Duration_copyIn (
    const DDS_Duration_t *from,
    os_duration *to)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;

    assert(to);

    if (from) {
        if ((from->sec == DDS_DURATION_INFINITE_SEC) &&
            (from->nanosec == DDS_DURATION_INFINITE_NSEC)) {
            *to = OS_DURATION_INFINITE;
        } else if ((from->sec >= 0) && (from->nanosec < 1000000000)) {
            *to = OS_DURATION_INIT(from->sec, from->nanosec);
        } else {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "Duration is invalid (seconds=%d, nanoseconds=%u)",
                    from->sec, from->nanosec);
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "Duration = NULL");
    }

    return result;
}

DDS_ReturnCode_t
DDS_Duration_copyOut (
    const os_duration *from,
    DDS_Duration_t *to)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;

    assert(from);
    assert(!OS_DURATION_ISINVALID(*from));

    if (to) {
        if (OS_DURATION_ISINFINITE(*from)) {
            to->sec = DDS_DURATION_INFINITE_SEC;
            to->nanosec = DDS_DURATION_INFINITE_NSEC;
        } else {
            assert(OS_DURATION_ISPOSITIVE(*from));
            to->sec = OS_DURATION_GET_SECONDS(*from);
            to->nanosec = OS_DURATION_GET_NANOSECONDS(*from);
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "Duration = NULL");
    }

    return result;
}

static void
DDS_SchedulingQosPolicy_copy (
    const DDS_SchedulingQosPolicy *_this,
    v_schedulePolicyI *policy)
{
    if (_this && policy) {
        switch (_this->scheduling_class.kind) {
        case DDS_SCHEDULE_DEFAULT:
            policy->v.kind = V_SCHED_DEFAULT;
        break;
        case DDS_SCHEDULE_TIMESHARING:
            policy->v.kind = V_SCHED_TIMESHARING;
        break;
        case DDS_SCHEDULE_REALTIME:
            policy->v.kind = V_SCHED_REALTIME;
        break;
        }
        switch (_this->scheduling_priority_kind.kind) {
        case V_SCHED_PRIO_RELATIVE:
            policy->v.priorityKind = V_SCHED_PRIO_RELATIVE;
        break;
        case V_SCHED_PRIO_ABSOLUTE:
            policy->v.priorityKind = V_SCHED_PRIO_ABSOLUTE;
        break;
        }
        policy->v.priority = (c_long)_this->scheduling_priority;
    }
}

u_participantQos
DDS_DomainParticipantQos_copyIn (
    const DDS_DomainParticipantQos *qos)
{
    u_participantQos pQos;

    pQos = u_participantQosNew(NULL);
    if (pQos != NULL) {
        if (qos->user_data.value._length) {
            pQos->userData.v.size = qos->user_data.value._length;
            pQos->userData.v.value = os_malloc(qos->user_data.value._length);
            memcpy (pQos->userData.v.value,
                    qos->user_data.value._buffer,
                    qos->user_data.value._length);
        } else {
            pQos->userData.v.size = 0;
            pQos->userData.v.value = NULL;
        }
        pQos->entityFactory.v.autoenable_created_entities =
            qos->entity_factory.autoenable_created_entities;
        DDS_SchedulingQosPolicy_copy(&qos->watchdog_scheduling,
                                     &pQos->watchdogScheduling);
    }
    return pQos;
}

u_topicQos
DDS_TopicQos_copyIn (
    const DDS_TopicQos *qos)
{
    u_topicQos tQos;

    tQos = u_topicQosNew(NULL);
    if (tQos != NULL) {
        if (qos->topic_data.value._length > 0) {
            tQos->topicData.v.size = qos->topic_data.value._length;
            tQos->topicData.v.value = os_malloc(qos->topic_data.value._length);
            memcpy (tQos->topicData.v.value,
                    qos->topic_data.value._buffer,
                    qos->topic_data.value._length);
        } else {
            tQos->topicData.v.size = 0;
            tQos->topicData.v.value = NULL;
        }

        tQos->liveliness.v.kind = (v_livelinessKind) qos->liveliness.kind;
        tQos->reliability.v.kind = (v_reliabilityKind) qos->reliability.kind;
        tQos->reliability.v.synchronous = qos->reliability.synchronous;
        tQos->orderby.v.kind = (v_orderbyKind) qos->destination_order.kind;
        tQos->history.v.kind = (v_historyQosKind) qos->history.kind;
        tQos->history.v.depth = qos->history.depth;
        tQos->ownership.v.kind = (v_ownershipKind) qos->ownership.kind;

        tQos->durability.v.kind = (v_durabilityKind) qos->durability.kind;
        tQos->durabilityService.v.history_kind = (v_historyQosKind) qos->durability_service.history_kind;
        tQos->durabilityService.v.history_depth = qos->durability_service.history_depth;
        tQos->durabilityService.v.max_samples = qos->durability_service.max_samples;
        tQos->durabilityService.v.max_instances = qos->durability_service.max_instances;
        tQos->durabilityService.v.max_samples_per_instance = qos->durability_service.max_samples_per_instance;
        tQos->resource.v.max_samples = qos->resource_limits.max_samples;
        tQos->resource.v.max_instances = qos->resource_limits.max_instances;
        tQos->resource.v.max_samples_per_instance = qos->resource_limits.max_samples_per_instance;
        tQos->transport.v.value = qos->transport_priority.value;

        DDS_Duration_init_mapping(&qos->lifespan.duration, &tQos->lifespan.v.duration);
        DDS_Duration_init_mapping(&qos->deadline.period, &tQos->deadline.v.period);
        DDS_Duration_init_mapping(&qos->latency_budget.duration, &tQos->latency.v.duration);
        DDS_Duration_init_mapping(&qos->liveliness.lease_duration, &tQos->liveliness.v.lease_duration);
        DDS_Duration_init_mapping(&qos->durability_service.service_cleanup_delay, &tQos->durabilityService.v.service_cleanup_delay);
        DDS_Duration_init_mapping(&qos->reliability.max_blocking_time, &tQos->reliability.v.max_blocking_time);
    }
    return tQos;
}

u_publisherQos
DDS_PublisherQos_copyIn (
    const DDS_PublisherQos *qos)
{
    u_publisherQos pQos;

    pQos = u_publisherQosNew(NULL);
    if (pQos != NULL) {
        if (qos->group_data.value._length) {
            pQos->groupData.v.size = qos->group_data.value._length;
            pQos->groupData.v.value = os_malloc(qos->group_data.value._length);
            memcpy (pQos->groupData.v.value,
                    qos->group_data.value._buffer,
                    qos->group_data.value._length);
        } else {
            pQos->groupData.v.size = 0;
            pQos->groupData.v.value = NULL;
        }
        pQos->partition.v = DDS_StringSeq_to_string(&qos->partition.name,",");
        pQos->presentation.v.access_scope = (v_presentationKind) qos->presentation.access_scope;
        pQos->presentation.v.coherent_access = qos->presentation.coherent_access;
        pQos->presentation.v.ordered_access = qos->presentation.ordered_access;
        pQos->entityFactory.v.autoenable_created_entities = qos->entity_factory.autoenable_created_entities;
    }
    return pQos;
}

u_subscriberQos
DDS_SubscriberQos_copyIn (
    const DDS_SubscriberQos *qos)
{
    u_subscriberQos sQos;

    sQos = u_subscriberQosNew(NULL);
    if (sQos != NULL) {
        if (qos->group_data.value._length) {
            sQos->groupData.v.size = qos->group_data.value._length;
            sQos->groupData.v.value = os_malloc(qos->group_data.value._length);
            memcpy(sQos->groupData.v.value,
                   qos->group_data.value._buffer,
                   qos->group_data.value._length);
        } else {
            sQos->groupData.v.size = 0;
            sQos->groupData.v.value = NULL;
        }
        sQos->partition.v = DDS_StringSeq_to_string(&qos->partition.name,",");
        sQos->presentation.v.access_scope = (v_presentationKind) qos->presentation.access_scope;
        sQos->presentation.v.coherent_access = qos->presentation.coherent_access;
        sQos->presentation.v.ordered_access = qos->presentation.ordered_access;
        sQos->entityFactory.v.autoenable_created_entities = qos->entity_factory.autoenable_created_entities;
        if ( qos->share.enable ) {
            sQos->share.v.enable = TRUE;
            if (qos->share.name) {
                sQos->share.v.name = os_strdup(qos->share.name);
            } else {
                sQos->share.v.name = NULL;
            }
        } else {
            sQos->share.v.enable = FALSE;
            sQos->share.v.name = NULL;
        }
    }
    return sQos;
}

u_writerQos
DDS_DataWriterQos_copyIn (
    const DDS_DataWriterQos *qos)
{
    u_writerQos wQos;

    assert(qos != DDS_DATAWRITER_QOS_USE_TOPIC_QOS);

    wQos = u_writerQosNew(NULL);
    if (wQos != NULL) {
        if (qos->user_data.value._length) {
            wQos->userData.v.size = qos->user_data.value._length;
            wQos->userData.v.value = os_malloc(qos->user_data.value._length);
            memcpy (wQos->userData.v.value,
                    qos->user_data.value._buffer,
                    qos->user_data.value._length);
        } else {
            wQos->userData.v.size = 0;
            wQos->userData.v.value = NULL;
        }
        wQos->durability.v.kind = (v_durabilityKind) qos->durability.kind;
        wQos->history.v.kind = (v_historyQosKind) qos->history.kind;
        wQos->history.v.depth = qos->history.depth;
        wQos->liveliness.v.kind = (v_livelinessKind) qos->liveliness.kind;
        wQos->orderby.v.kind = (v_orderbyKind) qos->destination_order.kind;
        wQos->reliability.v.kind = (v_reliabilityKind) qos->reliability.kind;
        wQos->reliability.v.synchronous = qos->reliability.synchronous;
        wQos->resource.v.max_samples = qos->resource_limits.max_samples;
        wQos->resource.v.max_instances = qos->resource_limits.max_instances;
        wQos->resource.v.max_samples_per_instance = qos->resource_limits.max_samples_per_instance;
        wQos->ownership.v.kind = (v_ownershipKind) qos->ownership.kind;
        wQos->strength.v.value = qos->ownership_strength.value;
        wQos->transport.v.value = qos->transport_priority.value;
        wQos->lifecycle.v.autodispose_unregistered_instances = qos->writer_data_lifecycle.autodispose_unregistered_instances;
        DDS_Duration_init_mapping(&qos->deadline.period, &wQos->deadline.v.period);
        DDS_Duration_init_mapping(&qos->latency_budget.duration, &wQos->latency.v.duration);
        DDS_Duration_init_mapping(&qos->liveliness.lease_duration, &wQos->liveliness.v.lease_duration);
        DDS_Duration_init_mapping(&qos->reliability.max_blocking_time, &wQos->reliability.v.max_blocking_time);
        DDS_Duration_init_mapping(&qos->lifespan.duration, &wQos->lifespan.v.duration);
        DDS_Duration_init_mapping(&qos->writer_data_lifecycle.autopurge_suspended_samples_delay, &wQos->lifecycle.v.autopurge_suspended_samples_delay);
        DDS_Duration_init_mapping(&qos->writer_data_lifecycle.autounregister_instance_delay, &wQos->lifecycle.v.autounregister_instance_delay);
    }
    return wQos;
}

u_readerQos
DDS_DataReaderQos_copyIn (
    const DDS_DataReaderQos *qos)
{
    u_readerQos rQos;

    assert(qos != DDS_DATAREADER_QOS_USE_TOPIC_QOS);

    rQos = u_readerQosNew(NULL);
    if (rQos != NULL) {
        if (qos->user_data.value._length) {
            rQos->userData.v.size = qos->user_data.value._length;
            rQos->userData.v.value = os_malloc(qos->user_data.value._length);
            memcpy (rQos->userData.v.value,
                    qos->user_data.value._buffer,
                    qos->user_data.value._length);
        } else {
            rQos->userData.v.size = 0;
            rQos->userData.v.value = NULL;
        }

        rQos->durability.v.kind = (v_durabilityKind) qos->durability.kind;
        rQos->liveliness.v.kind = (v_livelinessKind) qos->liveliness.kind;
        rQos->reliability.v.kind = (v_reliabilityKind) qos->reliability.kind;
        rQos->reliability.v.synchronous = qos->reliability.synchronous;
        rQos->orderby.v.kind = (v_orderbyKind) qos->destination_order.kind;
        rQos->history.v.kind = (v_historyQosKind) qos->history.kind;
        rQos->history.v.depth = qos->history.depth;
        rQos->resource.v.max_samples = qos->resource_limits.max_samples;
        rQos->resource.v.max_instances = qos->resource_limits.max_instances;
        rQos->resource.v.max_samples_per_instance = qos->resource_limits.max_samples_per_instance;
        rQos->ownership.v.kind = (v_ownershipKind) qos->ownership.kind;
        rQos->lifespan.v.used = qos->reader_lifespan.use_lifespan;
        rQos->lifecycle.v.autopurge_dispose_all = qos->reader_data_lifecycle.autopurge_dispose_all;

        DDS_Duration_init_mapping(&qos->deadline.period,  &rQos->deadline.v.period);
        DDS_Duration_init_mapping(&qos->latency_budget.duration, &rQos->latency.v.duration);
        DDS_Duration_init_mapping(&qos->liveliness.lease_duration, &rQos->liveliness.v.lease_duration);
        DDS_Duration_init_mapping(&qos->reliability.max_blocking_time, &rQos->reliability.v.max_blocking_time);
        DDS_Duration_init_mapping(&qos->time_based_filter.minimum_separation, &rQos->pacing.v.minSeperation);
        DDS_Duration_init_mapping(&qos->reader_data_lifecycle.autopurge_nowriter_samples_delay, &rQos->lifecycle.v.autopurge_nowriter_samples_delay);
        DDS_Duration_init_mapping(&qos->reader_data_lifecycle.autopurge_disposed_samples_delay, &rQos->lifecycle.v.autopurge_disposed_samples_delay);
        DDS_Duration_init_mapping(&qos->reader_lifespan.duration, &rQos->lifespan.v.duration);

        rQos->share.v.enable = qos->share.enable;
        if ( qos->share.enable ) {
            if (qos->share.name) {
                rQos->share.v.name = os_strdup(qos->share.name);
            } else {
                rQos->share.v.name = NULL;
            }
        } else {
            rQos->share.v.name = NULL;
        }
    }
    rQos->userKey.v.enable = qos->subscription_keys.use_key_list;
    if ( qos->subscription_keys.use_key_list ) {
        rQos->userKey.v.expression =
        DDS_StringSeq_to_string(&qos->subscription_keys.key_list, ",");
    } else {
        rQos->userKey.v.expression = NULL;
    }

#ifdef USE_OLD_ReaderDataLifecycleQosPolicy
    if (qos->reader_data_lifecycle.enable_invalid_samples == FALSE) {
        rQos->lifecycle.v.enable_invalid_samples = qos->reader_data_lifecycle.enable_invalid_samples;
    } else {
#endif
        switch(qos->reader_data_lifecycle.invalid_sample_visibility.kind) {
            case DDS_NO_INVALID_SAMPLES:
                rQos->lifecycle.v.enable_invalid_samples = FALSE;
                break;
            case DDS_MINIMUM_INVALID_SAMPLES:
                rQos->lifecycle.v.enable_invalid_samples = TRUE;
                break;
            default:
                rQos->lifecycle.v.enable_invalid_samples = TRUE;
                break;
        }
#ifdef USE_OLD_ReaderDataLifecycleQosPolicy
    }
#endif

    return rQos;
}

u_dataViewQos
DDS_DataReaderViewQos_copyIn (
    const DDS_DataReaderViewQos *qos)
{
    u_dataViewQos vQos = NULL;

    vQos = u_dataViewQosNew(NULL);
    if (vQos != NULL) {
        vQos->userKey.v.enable = qos->view_keys.use_key_list;
        if ( qos->view_keys.use_key_list ) {
            vQos->userKey.v.expression =
                DDS_StringSeq_to_string(&qos->view_keys.key_list, ",");
        } else {
            vQos->userKey.v.expression = NULL;
        }
    }
    return vQos;
}

DDS_ReturnCode_t
DDS_UserDataQosPolicy_from_mapping(
    const v_userDataPolicyI *mapping,
    DDS_UserDataQosPolicy *policy)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;

    assert(mapping != NULL);
    assert(policy != NULL);

    if ( (mapping->v.size > 0) && mapping->v.value ) {
        DDS_sequence_replacebuf((_DDS_sequence)&policy->value,
                                (bufferAllocatorType)DDS_sequence_octet_allocbuf,
                                mapping->v.size);
        if ( policy->value._buffer ) {
            policy->value._maximum = mapping->v.size;
            policy->value._length  = mapping->v.size;
            policy->value._release = TRUE;
            memcpy(policy->value._buffer, mapping->v.value, mapping->v.size);
        } else {
            result = DDS_RETCODE_OUT_OF_RESOURCES;
            SAC_REPORT(result, "Failed to allocate heap memory of size %s",
                        mapping->v.size);
        }
    } else {
        policy->value._maximum = 0;
        policy->value._length  = 0;
        policy->value._release = FALSE;
        policy->value._buffer = NULL;
    }
    return result;
}

DDS_ReturnCode_t
DDS_GroupDataQosPolicy_from_mapping(
    const v_groupDataPolicyI *mapping,
    DDS_GroupDataQosPolicy *policy)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;

    assert(mapping != NULL);
    assert(policy != NULL);

    if ( (mapping->v.size > 0) && mapping->v.value ) {
        assert(policy->value._buffer == NULL); /* or reuse existing TBI */
        policy->value._buffer = DDS_sequence_octet_allocbuf(mapping->v.size);
        if ( policy->value._buffer ) {
            policy->value._maximum = mapping->v.size;
            policy->value._length  = mapping->v.size;
            policy->value._release = TRUE;
            memcpy(policy->value._buffer, mapping->v.value, mapping->v.size);
        } else {
            result = DDS_RETCODE_OUT_OF_RESOURCES;
            SAC_REPORT(result, "Failed to allocate heap memory of size %s",
                        mapping->v.size);
        }
    } else {
        policy->value._maximum = 0;
        policy->value._length  = 0;
        policy->value._release = FALSE;
        policy->value._buffer = NULL;
    }
    return result;
}

DDS_ReturnCode_t
DDS_TopicDataQosPolicy_from_mapping(
    const v_topicDataPolicyI *mapping,
    DDS_TopicDataQosPolicy *policy)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;

    assert(mapping != NULL);
    assert(policy != NULL);

    if ( (mapping->v.size > 0) && mapping->v.value ) {
        assert(policy->value._buffer == NULL); /* or reuse existing TBI */
        policy->value._buffer = DDS_sequence_octet_allocbuf(mapping->v.size);
        if ( policy->value._buffer ) {
            policy->value._maximum = mapping->v.size;
            policy->value._length  = mapping->v.size;
            policy->value._release = TRUE;
            memcpy(policy->value._buffer, mapping->v.value, mapping->v.size);
        } else {
            result = DDS_RETCODE_OUT_OF_RESOURCES;
            SAC_REPORT(result, "Failed to allocate heap memory of size %s",
                        mapping->v.size);
        }
    } else {
        policy->value._maximum = 0;
        policy->value._length  = 0;
        policy->value._release = FALSE;
        policy->value._buffer = NULL;
    }
    return result;
}

DDS_ReturnCode_t
DDS_DomainParticipantQos_copyOut (
    const u_participantQos uQos,
    DDS_DomainParticipantQos *qos)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;

    assert(uQos != NULL);
    assert(qos != NULL);

    DDS_UserDataQosPolicy_from_mapping(&uQos->userData, &qos->user_data);
    qos->entity_factory.autoenable_created_entities = uQos->entityFactory.v.autoenable_created_entities;
    qos->watchdog_scheduling.scheduling_class.kind = (DDS_SchedulingClassQosPolicyKind) uQos->watchdogScheduling.v.kind;
    qos->watchdog_scheduling.scheduling_priority_kind.kind = (DDS_SchedulingPriorityQosPolicyKind) uQos->watchdogScheduling.v.priorityKind;
    qos->watchdog_scheduling.scheduling_priority = uQos->watchdogScheduling.v.priority;
    qos->listener_scheduling = DDS_DomainParticipantQos_default.listener_scheduling;

    return result;
}

DDS_ReturnCode_t
DDS_TopicQos_copyOut (
    const u_topicQos uQos,
    DDS_TopicQos *qos)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;

    assert(uQos != NULL);
    assert(qos != NULL);

    DDS_TopicDataQosPolicy_from_mapping(&uQos->topicData, &qos->topic_data);

    qos->durability.kind = (DDS_DurabilityQosPolicyKind) uQos->durability.v.kind;

    qos->durability_service.history_kind = (DDS_HistoryQosPolicyKind) uQos->durabilityService.v.history_kind;
    qos->durability_service.history_depth = uQos->durabilityService.v.history_depth;
    qos->durability_service.max_samples = uQos->durabilityService.v.max_samples;
    qos->durability_service.max_instances = uQos->durabilityService.v.max_instances;
    qos->durability_service.max_samples_per_instance = uQos->durabilityService.v.max_samples_per_instance;

    qos->liveliness.kind = (DDS_LivelinessQosPolicyKind) uQos->liveliness.v.kind;
    qos->reliability.kind = (DDS_ReliabilityQosPolicyKind) uQos->reliability.v.kind;
    qos->reliability.synchronous = uQos->reliability.v.synchronous;
    qos->destination_order.kind = (DDS_DestinationOrderQosPolicyKind) uQos->orderby.v.kind;
    qos->history.kind = (DDS_HistoryQosPolicyKind) uQos->history.v.kind;
    qos->history.depth = uQos->history.v.depth;
    qos->ownership.kind = (DDS_OwnershipQosPolicyKind) uQos->ownership.v.kind;

    qos->resource_limits.max_samples = uQos->resource.v.max_samples;
    qos->resource_limits.max_instances = uQos->resource.v.max_instances;
    qos->resource_limits.max_samples_per_instance = uQos->resource.v.max_samples_per_instance;

    qos->transport_priority.value = uQos->transport.v.value;

    DDS_Duration_from_mapping(&uQos->lifespan.v.duration, &qos->lifespan.duration);
    DDS_Duration_from_mapping(&uQos->deadline.v.period, &qos->deadline.period);
    DDS_Duration_from_mapping(&uQos->latency.v.duration, &qos->latency_budget.duration);
    DDS_Duration_from_mapping(&uQos->liveliness.v.lease_duration, &qos->liveliness.lease_duration);
    DDS_Duration_from_mapping(&uQos->durabilityService.v.service_cleanup_delay, &qos->durability_service.service_cleanup_delay);
    DDS_Duration_from_mapping(&uQos->reliability.v.max_blocking_time, &qos->reliability.max_blocking_time);

    return result;
}

DDS_ReturnCode_t
DDS_PublisherQos_copyOut (
    const u_publisherQos uQos,
    DDS_PublisherQos *qos)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;

    assert(uQos != NULL);
    assert(qos != NULL);

    DDS_GroupDataQosPolicy_from_mapping(&uQos->groupData, &qos->group_data);

    DDS_string_to_StringSeq(uQos->partition.v,",",&qos->partition.name);

    qos->presentation.access_scope = (DDS_PresentationQosPolicyAccessScopeKind) uQos->presentation.v.access_scope;
    qos->presentation.coherent_access = uQos->presentation.v.coherent_access;
    qos->presentation.ordered_access = uQos->presentation.v.ordered_access;
    qos->entity_factory.autoenable_created_entities = uQos->entityFactory.v.autoenable_created_entities;

    return result;
}

DDS_ReturnCode_t
DDS_SubscriberQos_copyOut (
    const u_subscriberQos uQos,
    DDS_SubscriberQos *qos)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;

    assert(uQos != NULL);
    assert(qos != NULL);

    DDS_GroupDataQosPolicy_from_mapping(&uQos->groupData, &qos->group_data);

    DDS_string_to_StringSeq(uQos->partition.v,",",&qos->partition.name);

    qos->presentation.access_scope = (DDS_PresentationQosPolicyAccessScopeKind) uQos->presentation.v.access_scope;
    qos->presentation.coherent_access = uQos->presentation.v.coherent_access;
    qos->presentation.ordered_access = uQos->presentation.v.ordered_access;
    qos->entity_factory.autoenable_created_entities = uQos->entityFactory.v.autoenable_created_entities;

    if ( uQos->share.v.enable ) {
        qos->share.enable = TRUE;
        DDS_string_replace(uQos->share.v.name, &qos->share.name);
    } else {
        qos->share.enable = FALSE;
        qos->share.name = DDS_string_dup("");
    }
    return result;
}

DDS_ReturnCode_t
DDS_DataWriterQos_copyOut (
    const u_writerQos uQos,
    DDS_DataWriterQos *qos)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;

    assert(uQos != NULL);
    assert(qos != NULL);

    qos->durability.kind = (DDS_DurabilityQosPolicyKind) uQos->durability.v.kind;
    qos->liveliness.kind = (DDS_LivelinessQosPolicyKind) uQos->liveliness.v.kind;
    qos->reliability.kind = (DDS_ReliabilityQosPolicyKind) uQos->reliability.v.kind;
    qos->reliability.synchronous = uQos->reliability.v.synchronous;
    qos->destination_order.kind = (DDS_DestinationOrderQosPolicyKind) uQos->orderby.v.kind;
    qos->history.kind = (DDS_HistoryQosPolicyKind) uQos->history.v.kind;
    qos->history.depth = uQos->history.v.depth;
    qos->ownership.kind = (DDS_OwnershipQosPolicyKind) uQos->ownership.v.kind;
    qos->ownership_strength.value = uQos->strength.v.value;
    qos->transport_priority.value = uQos->transport.v.value;

    qos->resource_limits.max_samples = uQos->resource.v.max_samples;
    qos->resource_limits.max_instances = uQos->resource.v.max_instances;
    qos->resource_limits.max_samples_per_instance = uQos->resource.v.max_samples_per_instance;

    qos->writer_data_lifecycle.autodispose_unregistered_instances = uQos->lifecycle.v.autodispose_unregistered_instances;

    DDS_UserDataQosPolicy_from_mapping(&uQos->userData, &qos->user_data);
    DDS_Duration_from_mapping(&uQos->deadline.v.period, &qos->deadline.period);
    DDS_Duration_from_mapping(&uQos->latency.v.duration, &qos->latency_budget.duration);
    DDS_Duration_from_mapping(&uQos->liveliness.v.lease_duration, &qos->liveliness.lease_duration);
    DDS_Duration_from_mapping(&uQos->reliability.v.max_blocking_time, &qos->reliability.max_blocking_time);
    DDS_Duration_from_mapping(&uQos->lifespan.v.duration, &qos->lifespan.duration);
    DDS_Duration_from_mapping(&uQos->lifecycle.v.autopurge_suspended_samples_delay, &qos->writer_data_lifecycle.autopurge_suspended_samples_delay);
    DDS_Duration_from_mapping(&uQos->lifecycle.v.autounregister_instance_delay, &qos->writer_data_lifecycle.autounregister_instance_delay);

    return result;
}

DDS_ReturnCode_t
DDS_DataReaderQos_copyOut (
    const u_readerQos uQos,
    DDS_DataReaderQos *qos)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;

    assert(uQos != NULL);
    assert(qos != NULL);

    qos->durability.kind = (DDS_DurabilityQosPolicyKind) uQos->durability.v.kind;
    qos->liveliness.kind = (DDS_LivelinessQosPolicyKind) uQos->liveliness.v.kind;
    qos->reliability.kind = (DDS_ReliabilityQosPolicyKind) uQos->reliability.v.kind;
    qos->reliability.synchronous = uQos->reliability.v.synchronous;
    qos->destination_order.kind = (DDS_DestinationOrderQosPolicyKind) uQos->orderby.v.kind;
    qos->history.kind = (DDS_HistoryQosPolicyKind) uQos->history.v.kind;
    qos->history.depth = uQos->history.v.depth;
    qos->ownership.kind = (DDS_OwnershipQosPolicyKind) uQos->ownership.v.kind;
    qos->subscription_keys.use_key_list = uQos->userKey.v.enable;
    qos->share.enable = uQos->share.v.enable;
    qos->reader_lifespan.use_lifespan = uQos->lifespan.v.used;
    qos->reader_data_lifecycle.autopurge_dispose_all = uQos->lifecycle.v.autopurge_dispose_all;

    qos->resource_limits.max_samples = uQos->resource.v.max_samples;
    qos->resource_limits.max_instances = uQos->resource.v.max_instances;
    qos->resource_limits.max_samples_per_instance = uQos->resource.v.max_samples_per_instance;

#ifdef USE_OLD_ReaderDataLifecycleQosPolicy
    qos->reader_data_lifecycle.enable_invalid_samples = uQos->lifecycle.v.enable_invalid_samples;
#endif
    qos->reader_data_lifecycle.invalid_sample_visibility.kind = uQos->lifecycle.v.enable_invalid_samples ? DDS_MINIMUM_INVALID_SAMPLES : DDS_NO_INVALID_SAMPLES;

    if ( uQos->share.v.enable ) {
        qos->share.enable = TRUE;
        DDS_string_replace(uQos->share.v.name, &qos->share.name);
    } else {
        qos->share.enable = FALSE;
        qos->share.name = DDS_string_dup("");
    }

    DDS_string_to_StringSeq(uQos->userKey.v.expression, ",", &qos->subscription_keys.key_list);

    DDS_UserDataQosPolicy_from_mapping(&uQos->userData, &qos->user_data);

    DDS_Duration_from_mapping(&uQos->lifecycle.v.autopurge_nowriter_samples_delay, &qos->reader_data_lifecycle.autopurge_nowriter_samples_delay);
    DDS_Duration_from_mapping(&uQos->lifecycle.v.autopurge_disposed_samples_delay, &qos->reader_data_lifecycle.autopurge_disposed_samples_delay);
    DDS_Duration_from_mapping(&uQos->pacing.v.minSeperation, &qos->time_based_filter.minimum_separation);
    DDS_Duration_from_mapping(&uQos->deadline.v.period, &qos->deadline.period);
    DDS_Duration_from_mapping(&uQos->latency.v.duration, &qos->latency_budget.duration);
    DDS_Duration_from_mapping(&uQos->liveliness.v.lease_duration, &qos->liveliness.lease_duration);
    DDS_Duration_from_mapping(&uQos->reliability.v.max_blocking_time, &qos->reliability.max_blocking_time);
    DDS_Duration_from_mapping(&uQos->lifespan.v.duration, &qos->reader_lifespan.duration);

    return result;
}

DDS_ReturnCode_t
DDS_DataReaderViewQos_copyOut (
    const u_dataViewQos uQos,
    DDS_DataReaderViewQos *qos)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;

    assert(uQos != NULL);
    assert(qos != NULL);

    qos->view_keys.use_key_list = uQos->userKey.v.enable;
    DDS_string_to_StringSeq(uQos->userKey.v.expression,",",&qos->view_keys.key_list);
    return result;
}

void
DDS_Duration_from_mapping (
    const os_duration *from,
    DDS_Duration_t *to)
{
    if (OS_DURATION_ISINFINITE(*from)) {
        to->sec     = DDS_DURATION_INFINITE_SEC;
        to->nanosec = DDS_DURATION_INFINITE_NSEC;
    } else {
        to->sec     = *from / OS_DURATION_SECOND;
        if (*from < 0) {
            to->nanosec = -(*from % OS_DURATION_SECOND);
        } else {
            to->nanosec = *from % OS_DURATION_SECOND;
        }
    }
}

void
DDS_InconsistentTopicStatus_init (
    DDS_InconsistentTopicStatus *status,
    struct v_inconsistentTopicInfo *info)
{
    status->total_count = info->totalCount;
    status->total_count_change = info->totalChanged;
}

void
DDS_LivelinessLostStatus_init (
    DDS_LivelinessLostStatus *status,
    struct v_livelinessLostInfo *info)
{
    status->total_count = info->totalCount;
    status->total_count_change = info->totalChanged;
}

void
DDS_RequestedDeadlineMissedStatus_init (
    DDS_RequestedDeadlineMissedStatus *status,
    struct v_deadlineMissedInfo *info)
{
    v_handleResult result;
    v_object instance;

    status->total_count = info->totalCount;
    status->total_count_change = info->totalChanged;
    result = v_handleClaim(info->instanceHandle, &instance);
    if (result == V_HANDLE_OK) {
        status->last_instance_handle = u_instanceHandleNew(v_public(instance));
        result = v_handleRelease(info->instanceHandle);
    }
}

void
DDS_OfferedDeadlineMissedStatus_init (
    DDS_OfferedDeadlineMissedStatus *status,
    struct v_deadlineMissedInfo *info)
{
    v_handleResult result;
    v_object instance;

    status->total_count = info->totalCount;
    status->total_count_change = info->totalChanged;
    result = v_handleClaim(info->instanceHandle, &instance);
    if (result == V_HANDLE_OK) {
        status->last_instance_handle = u_instanceHandleNew(v_public(instance));
        result = v_handleRelease(info->instanceHandle);
    }
}

void
DDS_SampleRejectedStatus_init (
    DDS_SampleRejectedStatus *status,
    struct v_sampleRejectedInfo *info)
{
    status->total_count = info->totalCount;
    status->total_count_change = info->totalChanged;
    status->last_instance_handle = u_instanceHandleFromGID(info->instanceHandle);;
    switch (info->lastReason) {
    case S_NOT_REJECTED:
        status->last_reason = DDS_NOT_REJECTED;
    break;
    case S_REJECTED_BY_INSTANCES_LIMIT:
        status->last_reason = DDS_REJECTED_BY_INSTANCES_LIMIT;
    break;
    case S_REJECTED_BY_SAMPLES_LIMIT:
        status->last_reason = DDS_REJECTED_BY_SAMPLES_LIMIT;
    break;
    case S_REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT:
        status->last_reason = DDS_REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT;
    break;
    }
}

void
DDS_LivelinessChangedStatus_init (
    DDS_LivelinessChangedStatus *status,
    struct v_livelinessChangedInfo *info)
{
    status->alive_count = info->activeCount;
    status->not_alive_count = info->inactiveCount;
    status->alive_count_change = info->activeChanged;
    status->not_alive_count_change = info->inactiveChanged;
    status->last_publication_handle = u_instanceHandleFromGID(info->instanceHandle);
}

void
DDS_RequestedIncompatibleQosStatus_init (
    DDS_RequestedIncompatibleQosStatus *status,
    struct v_incompatibleQosInfo *info)
{
    DDS_unsigned_long len, i, j;

    status->total_count = info->totalCount;
    status->total_count_change = info->totalChanged;
    status->last_policy_id = info->lastPolicyId;
    len = 0;
    for (i=0; i<V_POLICY_ID_COUNT; i++) {
        if (info->policyCount[i] > 0) len++;
    }

    status->policies._length  = len;
    status->policies._maximum = len;
    status->policies._buffer  = DDS_QosPolicyCountSeq_allocbuf(len);
    j=0;
    for ( i = 0; i < V_POLICY_ID_COUNT; i++ ) {
        if (info->policyCount[i] > 0) {
            status->policies._buffer[j].policy_id = (DDS_QosPolicyId_t) i;
            status->policies._buffer[j++].count = info->policyCount[i];
        }
    }
}

void
DDS_OfferedIncompatibleQosStatus_init (
    DDS_OfferedIncompatibleQosStatus *status,
    struct v_incompatibleQosInfo *info)
{
    DDS_unsigned_long len, i, j;

    status->total_count = info->totalCount;
    status->total_count_change = info->totalChanged;
    status->last_policy_id = info->lastPolicyId;
    len = 0;
    for (i=0; i<V_POLICY_ID_COUNT; i++) {
        if (info->policyCount[i] > 0) len++;
    }

    status->policies._length  = len;
    status->policies._maximum = len;
    status->policies._buffer  = DDS_QosPolicyCountSeq_allocbuf(len);
    j=0;
    for ( i = 0; i < V_POLICY_ID_COUNT; i++ ) {
        if (info->policyCount[i] > 0) {
            status->policies._buffer[j].policy_id = i;
            status->policies._buffer[j++].count = info->policyCount[i];
        }
    }
}

void
DDS_SampleLostStatus_init (
    DDS_SampleLostStatus *status,
    struct v_sampleLostInfo *info)
{

    status->total_count = info->totalCount;
    status->total_count_change = info->totalChanged;
}

void
DDS_SubscriptionMatchedStatus_init (
    DDS_SubscriptionMatchedStatus *status,
    struct v_topicMatchInfo *info)
{
    status->total_count = info->totalCount;
    status->total_count_change = info->totalChanged;
    status->current_count = info->currentCount;
    status->current_count_change = info->currentChanged;
    status->last_publication_handle = u_instanceHandleFromGID(info->instanceHandle);
}

void
DDS_PublicationMatchedStatus_init (
    DDS_PublicationMatchedStatus *status,
    struct v_topicMatchInfo *info)
{
    status->total_count = info->totalCount;
    status->total_count_change = info->totalChanged;
    status->current_count = info->currentCount;
    status->current_count_change = info->currentChanged;
    status->last_subscription_handle = u_instanceHandleFromGID(info->instanceHandle);
}

DDS_ReturnCode_t
DDS_InstanceHandle_set_userdata(
    DDS_InstanceHandle_t handle,
    void *userData)
{
    return DDS_ReturnCode_get(u_instanceHandleSetUserData((u_instanceHandle)handle, userData));
}

void *
DDS_InstanceHandle_get_userdata(
    DDS_InstanceHandle_t handle)
{
    return u_instanceHandleGetUserData((u_instanceHandle)handle);
}
