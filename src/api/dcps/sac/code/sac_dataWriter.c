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
#include "sac_common.h"
#include "sac_objManag.h"
#include "sac_object.h"
#include "sac_entity.h"
#include "sac_dataWriter.h"
#include "sac_typeSupport.h"
#include "sac_topicDescription.h"
#include "dds_builtinTopicsSplDcps.h"
#include "sac_domainParticipant.h"
#include "sac_genericCopyIn.h"
#include "u_writer.h"
#include "v_status.h"
#include "sac_report.h"

#define DDS_DataWriterClaim(_this, writer) \
        DDS_Object_claim(DDS_Object(_this), DDS_DATAWRITER, (_Object *)writer)

#define DDS_DataWriterClaimRead(_this, writer) \
        DDS_Object_claim(DDS_Object(_this), DDS_DATAWRITER, (_Object *)writer)

#define DDS_DataWriterRelease(_this) \
        DDS_Object_release(DDS_Object(_this))

#define DDS_DataWriterCheck(_this, writer) \
        DDS_Object_check_and_assign(DDS_Object(_this), DDS_DATAWRITER, (_Object *)writer)

#define _DataWriter_get_user_entity(_this) \
        u_writer(_Entity_get_user_entity(_Entity(_DataWriter(_this))))

#define WRITER_STATUS_MASK (DDS_LIVELINESS_LOST_STATUS            |\
                            DDS_OFFERED_DEADLINE_MISSED_STATUS    |\
                            DDS_OFFERED_INCOMPATIBLE_QOS_STATUS   |\
                            DDS_PUBLICATION_MATCHED_STATUS)

typedef struct writerInfo_s {
    _DataWriter writer;
    void *data;
} writerInfo;


static v_copyin_result
_DataWriterCopy (
    c_type type,
    const void *data,
    void *to)
{
    v_copyin_result result;
    c_base base = c_getBase(c_object(type));
    writerInfo *info = (writerInfo *)data;

    if (info->writer->copy_cache) {
        C_STRUCT(DDS_srcInfo) dataInfo;
        dataInfo.copyProgram = info->writer->copy_cache;
        dataInfo.src = info->data;
        result = info->writer->copy_in (base, &dataInfo, to);
    } else {
        result = info->writer->copy_in (base, info->data, to);
    }

    return result;
}

static DDS_ReturnCode_t
_DataWriter_deinit (
    _Object _this)
{
    DDS_ReturnCode_t result;
    _DataWriter w;

    w = _DataWriter(_this);
    if (w == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "DataWriter = NULL");
    } else {
        DDS_Entity_set_listener_interest(DDS_Entity(_this), 0);
        DDS_Entity_disable_callbacks(DDS_Entity(_this));
        DDS_TopicDescription_free(DDS_TopicDescription(w->topic));
        result = _Entity_deinit(_this);
    }
    return result;
}

DDS_ReturnCode_t
DDS_DataWriterNew (
    u_writer uWriter,
    const DDS_Publisher publisher,
    const DDS_Topic topic,
    DDS_DataWriter *writer)
{
    _DataWriter _this;
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    DDS_TypeSupport typeSupport;

    if (uWriter == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "uWriter = NULL");
    }
    if (publisher == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "Publisher = NULL");
    }
    if (topic == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "Topic = NULL");
    }
    if (writer == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "DataWriter holder = NULL");
    }
    if (result == DDS_RETCODE_OK) {
        result = DDS_TopicDescription_get_typeSupport(topic, &typeSupport);
    }
    if (result == DDS_RETCODE_OK) {
        result = DDS_Object_new(DDS_DATAWRITER, _DataWriter_deinit, (_Object *)&_this);
    }
    if (result == DDS_RETCODE_OK) {
        result = DDS_Entity_init(_this, u_entity(uWriter));
        DDS_Object_set_domain_id(_Object(_this), DDS_Object_get_domain_id(publisher));
    }
    if (result == DDS_RETCODE_OK) {
        _this->publisher = publisher;
        _this->topic      = DDS_Topic(DDS_TopicDescription_keep(DDS_TopicDescription(topic)));
        _this->copy_in    = DDS_TypeSupportCopyIn(typeSupport);
        _this->copy_out   = DDS_TypeSupportCopyOut(typeSupport);
        _this->copy_cache = DDS_TypeSupportCopyCache(typeSupport);
/* TODO       copy_action = DDS_TypeSupportGetWriterCopy(typeSupport);*/

        *writer = (DDS_DataWriter)_this;
    }
    return result;
}

DDS_ReturnCode_t
DDS_DataWriterFree (
    DDS_DataWriter _this)
{
    DDS_ReturnCode_t result;
    result = DDS__free(_this);
    return result;
}

/*     ReturnCode_t
 *     set_qos(
 *         in DataWriterQos qos);
 */
DDS_ReturnCode_t
DDS_DataWriter_set_qos (
    DDS_DataWriter _this,
    const DDS_DataWriterQos *qos)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    _DataWriter w;
    u_writerQos wQos;
    u_result uResult;
    DDS_DataWriterQos writerQos;
    DDS_TopicQos topicQos;

    SAC_REPORT_STACK();

    memset(&writerQos, 0, sizeof(DDS_DataWriterQos));
    (void)DDS_DataWriterQos_init(&writerQos, DDS_DATAWRITER_QOS_DEFAULT);

    result = DDS_DataWriterQos_is_consistent(qos);
    if (result == DDS_RETCODE_OK) {
        result = DDS_DataWriterClaim(_this, &w);
    }
    if (result == DDS_RETCODE_OK) {
        if (qos == DDS_DATAWRITER_QOS_DEFAULT ||
            qos == DDS_DATAWRITER_QOS_USE_TOPIC_QOS)
        {
            result = DDS_Publisher_get_default_datawriter_qos(
                w->publisher, &writerQos);
            qos = &writerQos;
        }
        if (qos == DDS_DATAWRITER_QOS_USE_TOPIC_QOS) {
            memset(&topicQos, 0, sizeof(DDS_TopicQos));
            (void)DDS_TopicQos_init(&topicQos, DDS_TOPIC_QOS_DEFAULT);
            result = DDS_Topic_get_qos(w->topic, &topicQos);
            if (result == DDS_RETCODE_OK) {
                result = DDS_Publisher_copy_from_topic_qos(
                    w->publisher, &writerQos, &topicQos);
            }
            if (result == DDS_RETCODE_OK) {
                result = DDS_DataWriterQos_is_consistent(&writerQos);
            }
            (void)DDS_TopicQos_deinit(&topicQos);
        }
        if (result == DDS_RETCODE_OK) {
            wQos = DDS_DataWriterQos_copyIn(qos);
            if (wQos == NULL) {
                result = DDS_RETCODE_OUT_OF_RESOURCES;
                SAC_REPORT(result, "Failed to copy DDS_DataWriterQos");
            }
        }
        if (result == DDS_RETCODE_OK) {
            uResult = u_writerSetQos(_DataWriter_get_user_entity(w), wQos);
            result = DDS_ReturnCode_get(uResult);
            u_writerQosFree(wQos);
        }
        DDS_DataWriterRelease(_this);
    }

    (void)DDS_DataWriterQos_deinit(&writerQos);

    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     ReturnCode_t
 *     get_qos(
 *         inout DataWriterQos qos);
 */
DDS_ReturnCode_t
DDS_DataWriter_get_qos (
    DDS_DataWriter _this,
    DDS_DataWriterQos *qos)
{
    DDS_ReturnCode_t result;
    _DataWriter w;
    u_writerQos uQos;
    u_result uResult;

    SAC_REPORT_STACK();

    result = DDS_DataWriterCheck(_this, &w);
    if (result == DDS_RETCODE_OK) {
        if (qos == NULL) {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "DataWriterQos = NULL");
        } else if (qos == DDS_DATAWRITER_QOS_DEFAULT) {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "QoS 'DATAWRITER_QOS_DEFAULT' is read-only.");
        } else if (qos == DDS_DATAWRITER_QOS_USE_TOPIC_QOS) {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "QoS 'DATAWRITER_QOS_USE_TOPIC_QOS' is read-only.");
        }
    }
    if (result == DDS_RETCODE_OK) {
        uResult = u_writerGetQos(_DataWriter_get_user_entity(w), &uQos);
        if (uResult == U_RESULT_OK) {
            result = DDS_DataWriterQos_copyOut(uQos, qos);
            u_writerQosFree(uQos);
        } else {
            result = DDS_ReturnCode_get(uResult);
        }
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     ReturnCode_t
 *     set_listener(
 *         in DataWriterListener a_listener,
 *         in StatusKindMask mask);
 */
DDS_ReturnCode_t
DDS_DataWriter_set_listener (
    DDS_DataWriter _this,
    const struct DDS_DataWriterListener *a_listener,
    const DDS_StatusMask mask)
{
    DDS_ReturnCode_t result;
    _DataWriter w;

    SAC_REPORT_STACK();

    result = DDS_DataWriterClaim(_this, &w);
    if (result == DDS_RETCODE_OK) {
        if (a_listener != NULL) {
            w->listener = *a_listener;
            result = DDS_Entity_set_listener_interest(DDS_Entity(w), mask);
        } else {
            memset(&w->listener, 0, sizeof(struct DDS_DataWriterListener));
            result = DDS_Entity_set_listener_interest(DDS_Entity(w), mask);
        }
        DDS_DataWriterRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     DataWriterListener
 *     get_listener();
 */
struct DDS_DataWriterListener
DDS_DataWriter_get_listener (
    DDS_DataWriter _this)
{
    DDS_ReturnCode_t result;
    _DataWriter w;
    struct DDS_DataWriterListener *listener, noListener;

    SAC_REPORT_STACK();

    result = DDS_DataWriterCheck(_this, &w);
    if (result == DDS_RETCODE_OK) {
        listener = &w->listener;
    } else {
        memset(&noListener, 0, sizeof(struct DDS_DataWriterListener));
        listener = &noListener;
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return *listener;
}

/*     ReturnCode_t
 *     set_listener_mask(
 *         in StatusKindMask mask);
 */
DDS_ReturnCode_t
DDS_DataWriter_set_listener_mask (
    _DataWriter _this,
    const DDS_StatusMask mask)
{
    DDS_ReturnCode_t result;

    assert(_this);

    result = DDS_Entity_set_listener_interest(DDS_Entity(_this), mask);

    return result;
}


/*     Topic
 *     get_topic();
 */
DDS_Topic
DDS_DataWriter_get_topic (
    DDS_DataWriter _this)
{
    DDS_ReturnCode_t result;
    _DataWriter w;
    DDS_Topic topic = NULL;

    SAC_REPORT_STACK();

    result = DDS_DataWriterCheck(_this, &w);
    if (result == DDS_RETCODE_OK) {
        topic = w->topic;
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return topic;
}

/*     Publisher
 *     get_publisher();
 */
DDS_Publisher
DDS_DataWriter_get_publisher (
    DDS_DataWriter _this)
{
    DDS_ReturnCode_t result;
    _DataWriter w;
    DDS_Publisher publisher = NULL;

    SAC_REPORT_STACK();

    result = DDS_DataWriterCheck(_this, &w);
    if (result == DDS_RETCODE_OK) {
        publisher = w->publisher;
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return publisher;
}

/* ReturnCode_t
 *   wait_for_acknowledgments(
 *      in Duration_t max_wait);
 */
DDS_ReturnCode_t
DDS_DataWriter_wait_for_acknowledgments (
    DDS_DataWriter _this,
    const DDS_Duration_t *max_wait)
{
    DDS_ReturnCode_t result;
    _DataWriter w;
    os_duration timeout;
    u_result uResult;

    SAC_REPORT_STACK();

    result = DDS_DataWriterCheck(_this, &w);
    if (result == DDS_RETCODE_OK) {
        result = DDS_Duration_copyIn(max_wait, &timeout);
    }
    if (result == DDS_RETCODE_OK) {
        uResult = u_writerWaitForAcknowledgments(_DataWriter_get_user_entity(w), timeout);
        result = DDS_ReturnCode_get(uResult);
    }
    SAC_REPORT_FLUSH(_this, (result != DDS_RETCODE_OK) && (result != DDS_RETCODE_TIMEOUT));
    return result;
}

static v_result
copy_liveliness_lost_status(
    c_voidp info,
    c_voidp arg)
{
    struct v_livelinessLostInfo *from;
    DDS_LivelinessLostStatus *to;

    from = (struct v_livelinessLostInfo *)info;
    to = (DDS_LivelinessLostStatus *)arg;

    to->total_count = from->totalCount;
    to->total_count_change = from->totalChanged;
    return V_RESULT_OK;
}

/*     // Access the status
 * ReturnCode_t
 * get_liveliness_lost_status(
 *       inout LivelinessLostStatus a_status);
 */
DDS_ReturnCode_t
DDS_DataWriter_get_liveliness_lost_status (
    DDS_DataWriter _this,
    DDS_LivelinessLostStatus *status)
{
    DDS_ReturnCode_t result;
    _DataWriter w;
    u_result uResult;

    SAC_REPORT_STACK();

    if (status != NULL) {
        result = DDS_DataWriterCheck(_this, &w);
        if (result == DDS_RETCODE_OK) {
            uResult = u_writerGetLivelinessLostStatus(
                          _DataWriter_get_user_entity(w),
                          TRUE,
                          copy_liveliness_lost_status,
                          status);
            result = DDS_ReturnCode_get(uResult);
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "LivelinessLostStatus holder = NULL");
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

static v_result
copy_deadline_missed_status(
    c_voidp info,
    c_voidp arg)
{
    struct v_deadlineMissedInfo *from;
    DDS_OfferedDeadlineMissedStatus *to;
    v_result result;
    v_object instance;

    result = V_RESULT_INTERNAL_ERROR;
    from = (struct v_deadlineMissedInfo *)info;
    to = (DDS_OfferedDeadlineMissedStatus *)arg;

    to->total_count = from->totalCount;
    to->total_count_change = from->totalChanged;

    if (!v_handleIsNil(from->instanceHandle)) {
        if (v_handleClaim(from->instanceHandle, &instance) == V_HANDLE_OK) {
            to->last_instance_handle = u_instanceHandleNew(v_public(instance));
            if (v_handleRelease(from->instanceHandle) == V_HANDLE_OK) {
                result = V_RESULT_OK;
            }
        }
    } else {
        result = V_RESULT_OK;
    }
    return result;
}

/*     // Access the status
 * ReturnCode_t
 * get_offered_deadline_missed_status(
 *       inout OfferedDeadlineMissedStatus a_status);
 */
DDS_ReturnCode_t
DDS_DataWriter_get_offered_deadline_missed_status (
    DDS_DataWriter _this,
    DDS_OfferedDeadlineMissedStatus *status)
{
    DDS_ReturnCode_t result;
    _DataWriter w;
    u_result uResult;

    SAC_REPORT_STACK();

    if (status != NULL) {
        result = DDS_DataWriterCheck(_this, &w);
        if (result == DDS_RETCODE_OK) {
            uResult = u_writerGetDeadlineMissedStatus(
                          _DataWriter_get_user_entity(w),
                          TRUE,
                          copy_deadline_missed_status,
                          status);
            result = DDS_ReturnCode_get(uResult);
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "OfferedDeadlineMissedStatus holder = NULL");
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

static v_result
copy_IncompatibleQosStatus(
    c_voidp info,
    c_voidp arg)
{
    DDS_unsigned_long i, j, len;
    v_result result = V_RESULT_OK;
    struct v_incompatibleQosInfo *from;
    DDS_OfferedIncompatibleQosStatus *to;

    from = (struct v_incompatibleQosInfo *)info;
    to = (DDS_OfferedIncompatibleQosStatus *)arg;

    to->total_count = from->totalCount;
    to->total_count_change = from->totalChanged;
    to->last_policy_id = from->lastPolicyId;

    len = 0;
    for (i=0; i<V_POLICY_ID_COUNT; i++) {
        if (from->policyCount[i] > 0) len++;
    }
    if ((to->policies._buffer != NULL) && (len > to->policies._maximum)) {
        DDS_free(to->policies._buffer);
        to->policies._buffer = NULL;
    }
    if ((to->policies._buffer == NULL) && (len > 0)) {
        to->policies._buffer = DDS_QosPolicyCountSeq_allocbuf(len);
        if (to->policies._buffer == NULL) {
            result = V_RESULT_OUT_OF_RESOURCES;
        } else {
            to->policies._maximum = len;
        }
    }
    if ( to->policies._buffer ) {
        to->policies._length = len;
        j=0;
        for ( i = 0; i < V_POLICY_ID_COUNT; i++ ) {
            if (from->policyCount[i] > 0) {
                to->policies._buffer[j].policy_id = (DDS_QosPolicyId_t) i;
                to->policies._buffer[j++].count = from->policyCount[i];
            }
        }
    } else {
        to->policies._maximum = 0;
        to->policies._length = 0;
    }
    return result;
}

/*     // Access the status
 * ReturnCode_t
 * get_offered_incompatible_qos_status(
 *       inout OfferedIncompatibleQosStatus a_status);
 */
DDS_ReturnCode_t
DDS_DataWriter_get_offered_incompatible_qos_status (
    DDS_DataWriter _this,
    DDS_OfferedIncompatibleQosStatus *status)
{
    DDS_ReturnCode_t result;
    _DataWriter w;
    u_result uResult;

    SAC_REPORT_STACK();

    if (status != NULL) {
        result = DDS_DataWriterCheck(_this, &w);
        if (result == DDS_RETCODE_OK) {
            uResult = u_writerGetIncompatibleQosStatus(
                          _DataWriter_get_user_entity(w),
                          TRUE,
                          copy_IncompatibleQosStatus,
                          status);
            result = DDS_ReturnCode_get(uResult);
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "OfferedIncompatibleQosStatus holder = NULL");
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

static v_result
copy_publication_matched_status(
    c_voidp info,
    c_voidp arg)
{
    struct v_topicMatchInfo *from;
    DDS_PublicationMatchedStatus *to;

    from = (struct v_topicMatchInfo *)info;
    to = (DDS_PublicationMatchedStatus *)arg;

    to->total_count = from->totalCount;
    to->total_count_change = from->totalChanged;
    to->current_count = from->currentCount;
    to->current_count_change = from->currentChanged;
    to->last_subscription_handle = u_instanceHandleFromGID(from->instanceHandle);
    return V_RESULT_OK;
}

/*     // Access the status
 * ReturnCode_t
 * get_publication_matched_status(
 *       inout PublicationMatchedStatus a_status);
 */
DDS_ReturnCode_t
DDS_DataWriter_get_publication_matched_status (
    DDS_DataWriter _this,
    DDS_PublicationMatchedStatus *status)
{
    DDS_ReturnCode_t result;
    _DataWriter w;
    u_result uResult;

    SAC_REPORT_STACK();

    if (status != NULL) {
        result = DDS_DataWriterCheck(_this, &w);
        if (result == DDS_RETCODE_OK) {
            uResult = u_writerGetPublicationMatchStatus (
                          _DataWriter_get_user_entity(w),
                          TRUE,
                          copy_publication_matched_status,
                          status);
            result = DDS_ReturnCode_get(uResult);
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "PublicationMatchedStatus holder = NULL");
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     ReturnCode_t
 *     assert_liveliness();
 */
DDS_ReturnCode_t
DDS_DataWriter_assert_liveliness (
    DDS_DataWriter _this)
{
    DDS_ReturnCode_t result;
    _DataWriter w;
    u_result uResult;

    SAC_REPORT_STACK();

    result = DDS_DataWriterCheck(_this, &w);
    if (result == DDS_RETCODE_OK) {
        uResult = u_writerAssertLiveliness(_DataWriter_get_user_entity(w));
        result = DDS_ReturnCode_get(uResult);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

static v_result
copy_matched_subscription(
    u_subscriptionInfo *info,
    void *arg)
{
    DDS_InstanceHandleSeq *to;
    DDS_InstanceHandle_t *tmp_buffer;

    to = (DDS_InstanceHandleSeq *)arg;

    if (to->_maximum <= to->_length) {
        tmp_buffer = to->_buffer;
        to->_buffer = DDS_InstanceHandleSeq_allocbuf(to->_length + 10);
        to->_maximum = to->_length + 10;
        if (tmp_buffer) {
            memcpy(to->_buffer, tmp_buffer, to->_length * sizeof(*to->_buffer));
            DDS_free(tmp_buffer);
        }
    }
    to->_buffer[to->_length] = u_instanceHandleFromGID(info->key);
    ++to->_length;
    return V_RESULT_OK;
}

/*     ReturnCode_t
 *     get_matched_subscriptions(
 *         inout InstanceHandleSeq subscription_handles);
 */
DDS_ReturnCode_t
DDS_DataWriter_get_matched_subscriptions (
    DDS_DataWriter _this,
    DDS_InstanceHandleSeq *subscription_handles)
{
    DDS_ReturnCode_t result;
    _DataWriter w;
    u_result uResult;

    SAC_REPORT_STACK();

    result = DDS_DataWriterCheck(_this, &w);
    if (result == DDS_RETCODE_OK) {
        subscription_handles->_length = 0;
        uResult = u_writerGetMatchedSubscriptions(
                      _DataWriter_get_user_entity(w),
                      copy_matched_subscription,
                      subscription_handles);
        result = DDS_ReturnCode_get(uResult);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/* TODO:
 * Temporary wrapper to fix the missing result value.
 * The preprocessor must be changes to generate copy operation with a result value.
 */
static v_result
___DDS_SubscriptionBuiltinTopicData__copyOut(
    u_subscriptionInfo *info,
    void *arg)
{
    __DDS_SubscriptionBuiltinTopicData__copyOut(info, arg);
    return V_RESULT_OK;
}

/*     ReturnCode_t
 *     get_matched_subscription_data(
 *         inout SubscriptionBuiltinTopicData subscription_data,
 *         in InstanceHandle_t subscription_handle);
 */
DDS_ReturnCode_t
DDS_DataWriter_get_matched_subscription_data (
    DDS_DataWriter _this,
    DDS_SubscriptionBuiltinTopicData *subscription_data,
    const DDS_InstanceHandle_t subscription_handle)
{
    DDS_ReturnCode_t result;
    _DataWriter w;
    u_result uResult;

    SAC_REPORT_STACK();

    result = DDS_DataWriterCheck(_this, &w);
    if (result == DDS_RETCODE_OK) {
        uResult = u_writerGetMatchedSubscriptionData(
                      _DataWriter_get_user_entity(w),
                      subscription_handle,
                      ___DDS_SubscriptionBuiltinTopicData__copyOut,
                      subscription_data);
        result = DDS_ReturnCode_get(uResult);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}



/*
 * Typeless Foo API operations
 */

/*
 * Alternative function for DDS_Time_copyIn, which can also handle
 * the proprietary DDS_TIMESTAMP_CURRENT.
 */
static DDS_ReturnCode_t
time_copyIn (
    const DDS_Time_t *from,
    os_timeW *to,
    os_int64 maxSupportedSeconds)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;

    assert(to);

    if (from) {
        if ((from->sec == DDS_TIMESTAMP_CURRENT_SEC) &&
            (from->nanosec == DDS_TIMESTAMP_CURRENT_NSEC))
        {
            /* Proprietary time extension, used to indicate that current time
             * needs to be used. Kernel translates invalid to current.
             */
            *to = OS_TIMEW_INVALID;
        } else if (DDS_Time_is_valid(from, maxSupportedSeconds)) {
            result = DDS_Time_copyIn(from, to, maxSupportedSeconds);
        } else {
            result = DDS_RETCODE_BAD_PARAMETER;
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "Time = NULL");
    }

    return result;
}

DDS_InstanceHandle_t
DDS_DataWriter_register_instance (
    DDS_DataWriter _this,
    const DDS_Sample instance_data)
{
    DDS_InstanceHandle_t handle;
    DDS_Time_t now = DDS_TIMESTAMP_CURRENT;

    SAC_REPORT_STACK();

    handle = DDS_DataWriter_register_instance_w_timestamp(_this, instance_data, &now);
    SAC_REPORT_FLUSH(_this, handle == DDS_HANDLE_NIL);
    return handle;
}

DDS_InstanceHandle_t
DDS_DataWriter_register_instance_w_timestamp (
    DDS_DataWriter _this,
    const DDS_Sample instance_data,
    const DDS_Time_t *source_timestamp)
{
    DDS_ReturnCode_t result;
    DDS_InstanceHandle_t handle = DDS_HANDLE_NIL;
    writerInfo data;
    _DataWriter w;
    u_result uResult;
    os_timeW timestamp;

    SAC_REPORT_STACK();

    if (instance_data != NULL) {
        result = DDS_DataWriterCheck(_this, &w);
        if (result == DDS_RETCODE_OK) {
            result = time_copyIn(source_timestamp, &timestamp, SAC_ENTITY_MAX_SUPPORTED_SECONDS(_this));
            if (result == DDS_RETCODE_OK) {
                data.writer = _this;
                data.data = (void *)instance_data;
                uResult = u_writerRegisterInstance(
                                  _DataWriter_get_user_entity(w),
                                  _DataWriterCopy,
                                  (void *)&data,
                                  timestamp,
                                  (u_instanceHandle *) &handle);
                result = DDS_ReturnCode_get(uResult);
            }
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "Sample = NULL");
    }
    SAC_REPORT_FLUSH(_this, (result != DDS_RETCODE_OK) && (result != DDS_RETCODE_TIMEOUT));
    return handle;
}

DDS_ReturnCode_t
DDS_DataWriter_unregister_instance (
    DDS_DataWriter _this,
    const DDS_Sample instance_data,
    const DDS_InstanceHandle_t handle)
{
    DDS_ReturnCode_t result;
    DDS_Time_t now = DDS_TIMESTAMP_CURRENT;

    SAC_REPORT_STACK();

    result = DDS_DataWriter_unregister_instance_w_timestamp(_this, instance_data, handle, &now);
    SAC_REPORT_FLUSH(_this, (result != DDS_RETCODE_OK) && (result != DDS_RETCODE_TIMEOUT));
    return result;
}

DDS_ReturnCode_t
DDS_DataWriter_unregister_instance_w_timestamp (
    DDS_DataWriter _this,
    const DDS_Sample instance_data,
    const DDS_InstanceHandle_t handle,
    const DDS_Time_t *source_timestamp)
{
    DDS_ReturnCode_t result;
    _DataWriter w;
    u_result uResult;
    writerInfo data, *_data;
    os_timeW timestamp;

    SAC_REPORT_STACK();

    result = DDS_DataWriterCheck(_this, &w);
    if (result == DDS_RETCODE_OK) {
        result = time_copyIn(source_timestamp, &timestamp, SAC_ENTITY_MAX_SUPPORTED_SECONDS(_this));
    }
    if (result == DDS_RETCODE_OK) {
        if (instance_data != NULL) {
            data.writer = _this;
            data.data = (void *)instance_data;
            _data = &data;
        } else {
            _data = NULL;
        }
        uResult = u_writerUnregisterInstance(
                              _DataWriter_get_user_entity(w),
                              _DataWriterCopy,
                              (void *)_data,
                              timestamp,
                              (u_instanceHandle)handle);
        result = DDS_ReturnCode_get(uResult);
    }
    SAC_REPORT_FLUSH(_this, (result != DDS_RETCODE_OK) && (result != DDS_RETCODE_TIMEOUT));
    return result;
}

DDS_ReturnCode_t
DDS_DataWriter_write (
    DDS_DataWriter _this,
    const DDS_Sample instance_data,
    const DDS_InstanceHandle_t handle)
{
    DDS_ReturnCode_t result;
    DDS_Time_t now = DDS_TIMESTAMP_CURRENT;

    SAC_REPORT_STACK();

    result = DDS_DataWriter_write_w_timestamp(_this, instance_data, handle, &now);
    SAC_REPORT_FLUSH(_this, (result != DDS_RETCODE_OK) && (result != DDS_RETCODE_TIMEOUT));
    return result;
}

DDS_ReturnCode_t
DDS_DataWriter_write_w_timestamp (
    DDS_DataWriter _this,
    const DDS_Sample instance_data,
    const DDS_InstanceHandle_t handle,
    const DDS_Time_t *source_timestamp)
{
    DDS_ReturnCode_t result;
    _DataWriter w;
    u_result uResult;
    os_timeW timestamp;
    writerInfo data;

    SAC_REPORT_STACK();

    if (instance_data != NULL) {
        result = DDS_DataWriterCheck(_this, &w);
        if (result == DDS_RETCODE_OK) {
            result = time_copyIn(source_timestamp, &timestamp, SAC_ENTITY_MAX_SUPPORTED_SECONDS(_this));
        }
        if (result == DDS_RETCODE_OK) {
            data.writer = _this;
            data.data = (void *)instance_data;
            uResult = u_writerWrite(_DataWriter_get_user_entity(w),
                                    _DataWriterCopy,
                                    (void *)&data,
                                    timestamp,
                                    (u_instanceHandle)handle);
            result = DDS_ReturnCode_get(uResult);
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "InstanceHandle = NULL");
    }
    SAC_REPORT_FLUSH(_this, (result != DDS_RETCODE_OK) && (result != DDS_RETCODE_TIMEOUT));
    return result;
}

DDS_ReturnCode_t
DDS_DataWriter_dispose (
    DDS_DataWriter _this,
    const DDS_Sample instance_data,
    const DDS_InstanceHandle_t instance_handle)
{
    DDS_ReturnCode_t result;
    DDS_Time_t now = DDS_TIMESTAMP_CURRENT;

    SAC_REPORT_STACK();

    result = DDS_DataWriter_dispose_w_timestamp(_this, instance_data, instance_handle, &now);
    SAC_REPORT_FLUSH(_this, (result != DDS_RETCODE_OK) && (result != DDS_RETCODE_TIMEOUT));
    return result;
}

DDS_ReturnCode_t
DDS_DataWriter_dispose_w_timestamp (
    DDS_DataWriter _this,
    const DDS_Sample instance_data,
    const DDS_InstanceHandle_t instance_handle,
    const DDS_Time_t *source_timestamp)
{
    DDS_ReturnCode_t result;
    _DataWriter w;
    u_result uResult;
    os_timeW timestamp;
    writerInfo data;

    SAC_REPORT_STACK();

    if (instance_data != NULL) {
        result = DDS_DataWriterCheck(_this, &w);
        if (result == DDS_RETCODE_OK) {
            result = time_copyIn(source_timestamp, &timestamp, SAC_ENTITY_MAX_SUPPORTED_SECONDS(_this));
        }
        if (result == DDS_RETCODE_OK) {
            data.writer = _this;
            data.data = (void *)instance_data;
            uResult = u_writerDispose(_DataWriter_get_user_entity(w),
                                      _DataWriterCopy,
                                      (void *)&data,
                                      timestamp,
                                      (u_instanceHandle)instance_handle);
            result = DDS_ReturnCode_get(uResult);
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "InstanceHandle = NULL");
    }
    SAC_REPORT_FLUSH(_this, (result != DDS_RETCODE_OK) && (result != DDS_RETCODE_TIMEOUT));
    return result;
}

DDS_ReturnCode_t
DDS_DataWriter_writedispose (
    DDS_DataWriter _this,
    const DDS_Sample instance_data,
    const DDS_InstanceHandle_t instance_handle)
{
    DDS_ReturnCode_t result;
    DDS_Time_t now = DDS_TIMESTAMP_CURRENT;

    SAC_REPORT_STACK();

    result = DDS_DataWriter_writedispose_w_timestamp(_this, instance_data, instance_handle, &now);
    SAC_REPORT_FLUSH(_this, (result != DDS_RETCODE_OK) && (result != DDS_RETCODE_TIMEOUT));
    return result;
}

DDS_ReturnCode_t
DDS_DataWriter_writedispose_w_timestamp (
    DDS_DataWriter _this,
    const DDS_Sample instance_data,
    const DDS_InstanceHandle_t instance_handle,
    const DDS_Time_t *source_timestamp)
{
    DDS_ReturnCode_t result;
    _DataWriter w;
    u_result uResult;
    os_timeW timestamp;
    writerInfo data;

    SAC_REPORT_STACK();

    if (instance_data != NULL) {
        result = DDS_DataWriterCheck(_this, &w);
        if (result == DDS_RETCODE_OK) {
            result = time_copyIn(source_timestamp, &timestamp, SAC_ENTITY_MAX_SUPPORTED_SECONDS(_this));
        }
        if (result == DDS_RETCODE_OK) {
            data.writer = _this;
            data.data = (void *)instance_data;
            uResult = u_writerWriteDispose(_DataWriter_get_user_entity(w),
                                           _DataWriterCopy,
                                           (void *)&data,
                                           timestamp,
                                           (u_instanceHandle)instance_handle);
            result = DDS_ReturnCode_get(uResult);
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "InstanceHandle = NULL");
    }
    SAC_REPORT_FLUSH(_this, (result != DDS_RETCODE_OK) && (result != DDS_RETCODE_TIMEOUT));
    return result;
}

DDS_ReturnCode_t
DDS_DataWriter_get_key_value (
    DDS_DataWriter _this,
    DDS_Sample key_holder,
    const DDS_InstanceHandle_t handle)
{
    DDS_ReturnCode_t result;
    _DataWriter w;
    u_result uResult;

    SAC_REPORT_STACK();

    if (key_holder != NULL) {
        result = DDS_DataWriterCheck(_this, &w);
        if (result == DDS_RETCODE_OK) {
            uResult = u_writerCopyKeysFromInstanceHandle(
                                           _DataWriter_get_user_entity(w),
                                           (u_instanceHandle)handle,
                                           (u_writerCopyKeyAction)_DataWriter(_this)->copy_out,
                                           key_holder);
            result = DDS_ReturnCode_get(uResult);
            if (result != DDS_RETCODE_OK) {
                SAC_REPORT(result, "Could not copy keys from Instance Handle");
            }
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "Sample key_holder = NULL");
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

DDS_InstanceHandle_t
DDS_DataWriter_lookup_instance(
    DDS_DataWriter _this,
    const DDS_Sample instance_data)
{
    DDS_ReturnCode_t result;
    _DataWriter w;
    u_result uResult;
    DDS_InstanceHandle_t handle = DDS_HANDLE_NIL;
    writerInfo data;

    SAC_REPORT_STACK();

    if (instance_data != NULL) {
        result = DDS_DataWriterCheck(_this, &w);
        if (result == DDS_RETCODE_OK) {
            data.writer = _this;
            data.data = (void *)instance_data;
            uResult = u_writerLookupInstance(_DataWriter_get_user_entity(w),
                                             _DataWriterCopy,
                                             &data,
                                             (u_instanceHandle *)&handle);
            result = DDS_ReturnCode_get(uResult);
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "Sample = NULL");
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return handle;
}


DDS_ReturnCode_t
DDS_DataWriter_notify_listener (
    DDS_DataWriter _this,
    v_listenerEvent event)
{
    DDS_ReturnCode_t result;
    u_eventMask triggerMask;
    struct DDS_DataWriterListener cb;

    cb = _DataWriter(_this)->listener;
    triggerMask = event->kind;
    result = DDS_RETCODE_OK;

    if ((triggerMask & V_EVENT_LIVELINESS_LOST) &&
            (cb.on_liveliness_lost != NULL))
    {
        DDS_LivelinessLostStatus status;
        DDS_LivelinessLostStatus_init(&status, &((v_writerStatus)event->eventData)->livelinessLost);
        cb.on_liveliness_lost(cb.listener_data, _this, &status);
    }
    if ((triggerMask & V_EVENT_OFFERED_DEADLINE_MISSED) &&
            (cb.on_offered_deadline_missed != NULL))
    {
        DDS_OfferedDeadlineMissedStatus status;
        DDS_OfferedDeadlineMissedStatus_init(&status, &((v_writerStatus)event->eventData)->deadlineMissed);
        cb.on_offered_deadline_missed(cb.listener_data, _this, &status);
    }
    if ((triggerMask & V_EVENT_OFFERED_INCOMPATIBLE_QOS) &&
            (cb.on_offered_incompatible_qos != NULL))
    {
        DDS_OfferedIncompatibleQosStatus status;
        DDS_OfferedIncompatibleQosStatus_init(&status, &((v_writerStatus)event->eventData)->incompatibleQos);
        cb.on_offered_incompatible_qos(cb.listener_data, _this, &status);
    }
    if ((triggerMask & V_EVENT_PUBLICATION_MATCHED) &&
            (cb.on_publication_matched != NULL))
    {
        DDS_PublicationMatchedStatus status;
        DDS_PublicationMatchedStatus_init(&status, &((v_writerStatus)event->eventData)->publicationMatch);
        cb.on_publication_matched(cb.listener_data, _this, &status);
    }

    return result;
}
