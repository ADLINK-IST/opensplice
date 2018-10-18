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
#include "dds.h"
#include "dds_report.h"
#include "dds__qos.h"
#include "dds__time.h"
#include "cmn_samplesList.h"
#include "cmn_reader.h"
#include "dds_loanRegistry.h"
#include "dds__subscriber.h"


/* Both SAC and C99 have the same defines for the state flags.
 * However, the flags have different bits. The application
 * uses the C99 ones. Due to the includes, we have the SAC
 * ones. Be sure we can use the C99 ones as well.
 */
static const uint32_t  C99_DDS_READ_SAMPLE_STATE                   =  1;
static const uint32_t  C99_DDS_NOT_READ_SAMPLE_STATE               =  2;
static const uint32_t  C99_DDS_NEW_VIEW_STATE                      =  4;
static const uint32_t  C99_DDS_NOT_NEW_VIEW_STATE                  =  8;
static const uint32_t  C99_DDS_ALIVE_INSTANCE_STATE                = 16;
static const uint32_t  C99_DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE   = 32;
static const uint32_t  C99_DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE = 64;


struct DataReaderInfo {
    struct DDS_EntityUserData_s _parent;
    dds_readerlistener_t *listener;
    bool ownSubscriber;
    dds_loanRegistry_t registry;
};


static void
on_requested_deadline_missed (
    void *listener_data,
    DDS_DataReader reader,
    const DDS_RequestedDeadlineMissedStatus *status)
{
    struct DataReaderInfo *info = listener_data;
    dds_readerlistener_t *listener = NULL;

    if (info) {
        listener = info->listener;
    }

    if (listener && status && listener->on_requested_deadline_missed) {
        dds_requested_deadline_missed_status_t s;
        s.total_count = status->total_count;
        s.total_count_change = status->total_count_change;
        s.last_instance_handle = status->last_instance_handle;
        listener->on_requested_deadline_missed(reader, &s);
    }
}

static void
on_requested_incompatible_qos (
    void *listener_data,
    DDS_DataReader reader,
    const DDS_RequestedIncompatibleQosStatus *status)
{
    struct DataReaderInfo *info = listener_data;
    dds_readerlistener_t *listener = NULL;

    if (info) {
        listener = info->listener;
    }

    if (listener && status && listener->on_requested_incompatible_qos) {
        dds_requested_incompatible_qos_status_t s;
        s.total_count = status->total_count;
        s.total_count_change = status->total_count_change;
        s.last_policy_id = status->last_policy_id;
        listener->on_requested_incompatible_qos(reader, &s);
    }
}

static void
on_sample_rejected (
    void *listener_data,
    DDS_DataReader reader,
    const DDS_SampleRejectedStatus *status)
{
    struct DataReaderInfo *info = listener_data;
    dds_readerlistener_t *listener = NULL;

    if (info) {
        listener = info->listener;
    }

    if (listener && status && listener->on_sample_rejected) {
        dds_sample_rejected_status_t s;
        s.total_count = status->total_count;
        s.total_count_change = status->total_count_change;
        s.last_reason = (dds_sample_rejected_status_kind)status->last_reason;
        s.last_instance_handle = status->last_instance_handle;
        listener->on_sample_rejected(reader, &s);
    }
}

static void
on_liveliness_changed (
    void *listener_data,
    DDS_DataReader reader,
    const DDS_LivelinessChangedStatus *status)
{
    struct DataReaderInfo *info = listener_data;
    dds_readerlistener_t *listener = NULL;

    if (info) {
        listener = info->listener;
    }

    if (listener && status && listener->on_liveliness_changed) {
        dds_liveliness_changed_status_t s;
        s.alive_count = status->alive_count;
        s.alive_count_change = status->alive_count_change;
        s.last_publication_handle = status->last_publication_handle;
        s.not_alive_count = status->not_alive_count;
        s.not_alive_count_change = status->not_alive_count_change;
        listener->on_liveliness_changed(reader, &s);
    }
}

static void
on_data_available (
    void *listener_data,
    DDS_DataReader reader)
{
    struct DataReaderInfo *info = listener_data;
    dds_readerlistener_t *listener = NULL;

    if (info) {
        listener = info->listener;
    }

    if (listener && listener->on_data_available) {
        listener->on_data_available(reader);
    }
}

static void
on_subscription_match (
    void *listener_data,
    DDS_DataReader reader,
    const DDS_SubscriptionMatchedStatus *status)
{
    struct DataReaderInfo *info = listener_data;
    dds_readerlistener_t *listener = NULL;

    if (info) {
        listener = info->listener;
    }

    if (listener && status && listener->on_subscription_matched) {
        dds_subscription_matched_status_t s;
        s.total_count = status->total_count;
        s.total_count_change = status->total_count_change;
        s.current_count = status->current_count;
        s.current_count_change = status->current_count_change;
        s.last_publication_handle = status->last_publication_handle;
        listener->on_subscription_matched(reader, &s);
    }
}

static void
on_sample_lost (
    void *listener_data,
    DDS_DataReader reader,
    const DDS_SampleLostStatus *status)
{
    struct DataReaderInfo *info = listener_data;
    dds_readerlistener_t *listener = NULL;

    if (info) {
        listener = info->listener;
    }

    if (listener && status && listener->on_sample_lost) {
        dds_sample_lost_status_t s;
        s.total_count = status->total_count;
        s.total_count_change = status->total_count_change;
        listener->on_sample_lost(reader, &s);
    }
}

static void
dds_datareader_info_free(
    DDS_EntityUserData data)
{
    struct DataReaderInfo *info = (struct DataReaderInfo *)data;
    if (info) {
        if (info->listener) {
            os_free(info->listener);
        }
        if (info->registry) {
            dds_loanRegistry_free(info->registry);
        }
        os_free(info);
    }
}


static struct DataReaderInfo *
dds_data_reader_info_new(
    DDS_TypeSupport ts)
{
    struct DataReaderInfo *info;

    info = os_malloc(sizeof(*info));
    DDS_Entity_user_data_init((DDS_EntityUserData)info, dds_datareader_info_free);
    info->listener = NULL;
    info->ownSubscriber = false;
    info->registry = dds_loanRegistry_new(ts);

    return info;
}


static void
dds_datareader_listener_init(
    struct DDS_DataReaderListener *listener,
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
}

static void
dds_mask_to_state_masks(
    uint32_t mask,
    DDS_SampleStateMask *sampleMask,
    DDS_ViewStateMask *viewMask,
    DDS_InstanceStateMask *instanceMask)
{
    uint32_t sMask = (mask & C99_DDS_READ_SAMPLE_STATE) |
                     (mask & C99_DDS_NOT_READ_SAMPLE_STATE);
    uint32_t vMask = (mask & C99_DDS_NEW_VIEW_STATE) |
                     (mask & C99_DDS_NOT_NEW_VIEW_STATE);
    uint32_t iMask = (mask & C99_DDS_ALIVE_INSTANCE_STATE) |
                     (mask & C99_DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE) |
                     (mask & C99_DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE);

    if (sMask == 0) {
        *sampleMask = DDS_ANY_SAMPLE_STATE;
    } else {
        *sampleMask = 0;
        if (sMask & C99_DDS_READ_SAMPLE_STATE) {
            *sampleMask |= DDS_READ_SAMPLE_STATE;
        }
        if (sMask & C99_DDS_NOT_READ_SAMPLE_STATE) {
            *sampleMask |= DDS_NOT_READ_SAMPLE_STATE;
        }
    }

    if (vMask == 0) {
        *viewMask = DDS_ANY_VIEW_STATE;
    } else {
        *viewMask = 0;
        if (vMask & C99_DDS_NEW_VIEW_STATE) {
            *viewMask |= DDS_NEW_VIEW_STATE;
        }
        if (vMask & C99_DDS_NOT_NEW_VIEW_STATE) {
            *viewMask |= DDS_NOT_NEW_VIEW_STATE;
        }
    }

    if (iMask == 0) {
        *instanceMask = DDS_ANY_INSTANCE_STATE;
    } else {
        *instanceMask = 0;
        if (iMask & C99_DDS_ALIVE_INSTANCE_STATE) {
            *instanceMask |= DDS_ALIVE_INSTANCE_STATE;
        }
        if (iMask & C99_DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE) {
            *instanceMask |= DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE;
        }
        if (iMask & C99_DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE) {
            *instanceMask |= DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE;
        }
    }
}


static u_sampleMask
dds_mask_to_u_mask(
    uint32_t mask)
{
    u_sampleMask ret = 0;

    uint32_t sMask = (mask & C99_DDS_READ_SAMPLE_STATE) |
                     (mask & C99_DDS_NOT_READ_SAMPLE_STATE);
    uint32_t vMask = (mask & C99_DDS_NEW_VIEW_STATE) |
                     (mask & C99_DDS_NOT_NEW_VIEW_STATE);
    uint32_t iMask = (mask & C99_DDS_ALIVE_INSTANCE_STATE) |
                     (mask & C99_DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE) |
                     (mask & C99_DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE);

    if (sMask == 0) {
        ret |= U_STATE_ANY_SAMPLE;
    } else {
        if (sMask & C99_DDS_READ_SAMPLE_STATE) {
            ret |= U_STATE_READ_SAMPLE;
        }
        if (sMask & C99_DDS_NOT_READ_SAMPLE_STATE) {
            ret |= U_STATE_NOT_READ_SAMPLE;
        }
    }

    if (vMask == 0) {
        ret |= U_STATE_ANY_VIEW;
    } else {
        if (vMask & C99_DDS_NEW_VIEW_STATE) {
            ret |= U_STATE_NEW_VIEW;
        }
        if (vMask & C99_DDS_NOT_NEW_VIEW_STATE) {
            ret |= U_STATE_NOT_NEW_VIEW;
        }
    }

    if (iMask == 0) {
        ret |= U_STATE_ANY_INSTANCE;
    } else {
        if (iMask & C99_DDS_ALIVE_INSTANCE_STATE) {
            ret |= U_STATE_ALIVE_INSTANCE;
        }
        if (iMask & C99_DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE) {
            ret |= U_STATE_DISPOSED_INSTANCE;
        }
        if (iMask & C99_DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE) {
            ret |= U_STATE_NOWRITERS_INSTANCE;
        }
    }

    return ret;
}


int
result_from_u_result(
    u_result uResult)
{
    int result;

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



struct flushCopyArg {
    DDS_TypeSupportCopyInfo copyInfo;
    void **data;
    dds_sample_info_t *info;
    uint32_t seqIndex;
};

#define C99_GET_SAMPLE_STATE(x)   ((dds_sample_state_t)((x) & 0x3))
#define C99_GET_VIEW_STATE(x)     ((dds_view_state_t)(((x) & 0x3))<<2)
#define C99_GET_INSTANCE_STATE(x) ((dds_instance_state_t)(((x) & 0x7))<<4)


static void
copy_sample_info(
    cmn_sampleInfo srcInfo,
    dds_sample_info_t *dst)
{
    dst->sample_state                = C99_GET_SAMPLE_STATE(srcInfo->sample_state);
    dst->view_state                  = C99_GET_VIEW_STATE(srcInfo->view_state);
    dst->instance_state              = C99_GET_INSTANCE_STATE(srcInfo->instance_state);
    dst->valid_data                  = (bool)                  srcInfo->valid_data;
    dst->instance_handle             = (dds_instance_handle_t) srcInfo->instance_handle;
    dst->publication_handle          = (dds_instance_handle_t) srcInfo->publication_handle;
    dst->disposed_generation_count   = (uint32_t)              srcInfo->disposed_generation_count;
    dst->no_writers_generation_count = (uint32_t)              srcInfo->no_writers_generation_count;
    dst->sample_rank                 = (uint32_t)              srcInfo->sample_rank;
    dst->generation_rank             = (uint32_t)              srcInfo->generation_rank;
    dst->absolute_generation_rank    = (uint32_t)              srcInfo->absolute_generation_rank;
    dst->source_timestamp            = (dds_time_t)            OS_TIMEW_GET_VALUE(srcInfo->source_timestamp);
    dst->reception_timestamp         = (dds_time_t)            OS_TIMEW_GET_VALUE(srcInfo->reception_timestamp);
}

static void
sample_flush_copy(
    void * sample,
    cmn_sampleInfo sampleInfo,
    void *arg)
{
    struct flushCopyArg *a = (struct flushCopyArg *)arg;
    void *dst;

    dst = a->data[a->seqIndex];
    DDS_TypeSupportCopyInfo_copy_out(a->copyInfo, a->data, dst, sample);
    copy_sample_info(sampleInfo, &(a->info[a->seqIndex]));
    a->seqIndex++;
}


static int
samples_flush_copy(
    dds_entity_t reader,
    cmn_samplesList samplesList,
    uint32_t length,
    void **data,
    dds_sample_info_t *info,
    dds_loanRegistry_t registry)
{
    struct flushCopyArg arg;
    int result = DDS_RETCODE_OK;
    int32_t testlength;
    u_entity uEntity = DDS_Entity_get_user_entity_for_test(reader);

    /* Prepare the buffers. */
    if (length > 0) {
        arg.copyInfo       = dds_loanRegistry_copyInfo(registry);
        arg.data           = data;
        arg.info           = info;
        arg.seqIndex       = 0;
        result = result_from_u_result(u_readerProtectCopyOutEnter(uEntity));
        if (result == DDS_RETCODE_OK) {
            testlength = cmn_samplesList_flush(samplesList, sample_flush_copy, &arg);
            if (testlength < 0) {
                result = DDS_RETCODE_ALREADY_DELETED;
            }
            u_readerProtectCopyOutExit(uEntity);
            assert((result != DDS_RETCODE_OK) || ((int32_t)length == testlength));
        }
    } else {
        result = DDS_RETCODE_NO_DATA;
    }

    return result;
}



int
dds_reader_create (
  dds_entity_t pp_or_sub,
  dds_entity_t * reader,
  dds_entity_t topic,
  const dds_qos_t * qos,
  const dds_readerlistener_t * listener)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    dds_entity_t subscriber = NULL;
    DDS_TypeSupport ts;
    struct DataReaderInfo *info;
    struct DDS_DataReaderListener dpl;
    struct DDS_DataReaderListener *lp = NULL;
    DDS_StatusMask mask = (listener) ? DDS_STATUS_MASK_ANY : DDS_STATUS_MASK_NONE;
    DDS_DataReaderQos *rQos;
    bool ownSubscriber = false;

    DDS_REPORT_STACK();

    if (!reader) {
        result = DDS_RETCODE_BAD_PARAMETER;
        DDS_REPORT(result, "Reader parameter is NULL.");
    }
    if (!topic) {
        result = DDS_RETCODE_BAD_PARAMETER;
        DDS_REPORT(result, "Topic parameter is NULL.");
    }
    if (result == DDS_RETCODE_OK) {
        if (!pp_or_sub) {
            result = dds_subscriber_create(NULL, &subscriber, qos, NULL);
            ownSubscriber = true;
        } else {
            if (DDS_Entity_get_kind(pp_or_sub) == DDS_ENTITY_KIND_DOMAINPARTICIPANT) {
                result = dds_subscriber_create(pp_or_sub, &subscriber, qos, NULL);
                ownSubscriber = true;
            } else {
                subscriber = pp_or_sub;
            }
        }
    }
    if (result == DDS_RETCODE_OK) {
        result = DDS_Topic_get_typeSupport(topic, &ts);
        if (result != DDS_RETCODE_OK) {
          DDS_REPORT(result, "Failed to get type support.");
        }
    }
    if (result == DDS_RETCODE_OK) {
        info = dds_data_reader_info_new(ts);
        if (!info) {
            result = DDS_RETCODE_ERROR;
            DDS_REPORT(result, "Failed to create reader info.");
        }
    }
    if (result == DDS_RETCODE_OK) {

        *reader = NULL;
        info->ownSubscriber = ownSubscriber;

        if (listener) {
            info->listener = os_malloc(sizeof(dds_readerlistener_t));
            *info->listener = *listener;
            lp = &dpl;
            dds_datareader_listener_init(&dpl, info);
        }

        if (qos) {
            rQos = DDS_DataReaderQos__alloc();
            result = DDS_Subscriber_get_default_datareader_qos(subscriber, rQos);
            if (result == DDS_RETCODE_OK) {
                dds_qos_to_reader_qos(rQos, qos);
                *reader = DDS_Subscriber_create_datareader(subscriber, topic, rQos, lp, mask);
            }
            DDS_free(rQos);
        } else {
            *reader = DDS_Subscriber_create_datareader(subscriber, topic, DDS_DATAREADER_QOS_USE_TOPIC_QOS, lp, mask);
        }
        if (*reader) {
            result = DDS_Entity_set_user_data(*reader, (DDS_EntityUserData)info);
        } else {
            result = dds_report_get_error_code();
        }
        DDS_Entity_release_user_data((DDS_EntityUserData)info);
    }

    DDS_REPORT_FLUSH(pp_or_sub, result != DDS_RETCODE_OK);

    return DDS_ERRNO(result, DDS_MOD_READER, DDS_ERR_Mx);
}

int
dds_reader_wait_for_historical_data (
  dds_entity_t reader,
  dds_duration_t max_wait)
{
    DDS_ReturnCode_t result;
    DDS_Duration_t ts = dds_duration_to_sac(max_wait);

    DDS_REPORT_STACK();

    result = DDS_DataReader_wait_for_historical_data(reader, &ts);

    DDS_REPORT_FLUSH(reader, result != DDS_RETCODE_OK);

    return DDS_ERRNO(result, DDS_MOD_READER, DDS_ERR_Mx);
}

dds_condition_t
dds_readcondition_create (
    dds_entity_t rd,
    uint32_t mask)
{
    dds_condition_t condition;
    DDS_SampleStateMask sampleMask;
    DDS_ViewStateMask viewMask;
    DDS_InstanceStateMask instanceMask;

    DDS_REPORT_STACK();

    dds_mask_to_state_masks(mask, &sampleMask, &viewMask, &instanceMask);
    condition = DDS_DataReader_create_readcondition(rd, sampleMask, viewMask, instanceMask);

    DDS_REPORT_FLUSH(rd, !condition);

    return condition;
}

dds_condition_t
dds_querycondition_create (
  dds_entity_t reader,
  uint32_t mask,
  dds_querycondition_filter_fn filter)
{
    DDS_ReturnCode_t result = DDS_RETCODE_UNSUPPORTED;

    OS_UNUSED_ARG(reader);
    OS_UNUSED_ARG(mask);
    OS_UNUSED_ARG(filter);

    DDS_REPORT_STACK();

    DDS_REPORT(result, "The operation dds_querycondition_create is unsupported.");

    DDS_REPORT_FLUSH(reader, result != DDS_RETCODE_OK);

    return NULL;
}

dds_condition_t
dds_querycondition_create_sql (
  dds_entity_t reader,
  uint32_t mask,
  const char *expression,
  const char **parameters,
  uint32_t maxp)
{
    dds_condition_t condition = NULL;
    DDS_SampleStateMask sampleMask;
    DDS_ViewStateMask viewMask;
    DDS_InstanceStateMask instanceMask;
    DDS_StringSeq *pseq;

    DDS_REPORT_STACK();

    if (!expression) {
        DDS_REPORT(DDS_RETCODE_BAD_PARAMETER, "The expression parameter is NULL.");
    } else {
        pseq = DDS_StringSeq__alloc();

        if (parameters && (maxp > 0)) {
            uint32_t i;

            pseq->_buffer = DDS_StringSeq_allocbuf(maxp);
            pseq->_length = maxp;
            pseq->_maximum = maxp;
            pseq->_release = TRUE;
            for (i = 0; i < maxp; i++) {
                pseq->_buffer[i] = DDS_string_dup(parameters[i]);
            }
        }
        dds_mask_to_state_masks(mask, &sampleMask, &viewMask, &instanceMask);

        condition = DDS_DataReader_create_querycondition(reader, sampleMask, viewMask, instanceMask, expression, pseq);
        DDS_free(pseq);
    }

    DDS_REPORT_FLUSH(reader, !condition);

    return condition;
}

int
dds_read (
  dds_entity_t rd,
  void ** buf,
  uint32_t maxs,
  dds_sample_info_t * si,
  uint32_t mask)
{
    DDS_ReturnCode_t result;
    struct DataReaderInfo *info = NULL;
    cmn_samplesList sampleList;
    u_dataReader uReader;
    u_result uResult;
    int length = 0;

    DDS_REPORT_STACK();

    if (!rd) {
        result = DDS_RETCODE_BAD_PARAMETER;
        DDS_REPORT(result, "The reader parameter is NULL.");
    } else if (!buf) {
        result = DDS_RETCODE_BAD_PARAMETER;
        DDS_REPORT(result, "The data buffer parameter is NULL.");
    } else if (!si) {
        result = DDS_RETCODE_BAD_PARAMETER;
        DDS_REPORT(result, "The sample info parameter is NULL.");
    } else {
        result = DDS_Entity_claim_user_data(rd, (DDS_EntityUserData *)&info);
    }

    if (result == DDS_RETCODE_OK) {
        sampleList = cmn_samplesList_new(FALSE);
        uReader = u_dataReader(DDS_Entity_get_user_entity_for_test(rd));
        cmn_samplesList_reset(sampleList, maxs);
        uResult = u_dataReaderRead(uReader, dds_mask_to_u_mask(mask), cmn_reader_action, sampleList, 0);
        if (uResult == U_RESULT_OK) {
            length = (int) cmn_samplesList_length(sampleList);
            if (length > 0) {
                if (buf[0] == 0) {
                    result = dds_loanRegistry_register(info->registry, buf, length);
                }
                if (result == DDS_RETCODE_OK) {
                    result = samples_flush_copy(rd, sampleList, length, buf, si, info->registry);
                }
            }
        } else {
            result = result_from_u_result(uResult);
        }
        cmn_samplesList_free(sampleList);
        DDS_Entity_release_user_data((DDS_EntityUserData)info);
    }
    DDS_REPORT_FLUSH(rd, (result != DDS_RETCODE_OK) && (result != DDS_RETCODE_NO_DATA));

    return ((result == DDS_RETCODE_OK) || (result == DDS_RETCODE_NO_DATA)) ?
                            length                                         :
                            DDS_ERRNO(result, DDS_MOD_READER, DDS_ERR_Mx);
}

int dds_read_instance (
  dds_entity_t rd,
  void ** buf,
  uint32_t maxs,
  dds_sample_info_t * si,
  dds_instance_handle_t handle,
  uint32_t mask)
{
    DDS_ReturnCode_t result;
    struct DataReaderInfo *info = NULL;
    cmn_samplesList sampleList;
    u_dataReader uReader;
    u_result uResult;
    bool noReport = false;
    int length = 0;

    DDS_REPORT_STACK();

    if (!rd) {
        result = DDS_RETCODE_BAD_PARAMETER;
        DDS_REPORT(result, "The reader parameter is NULL.");
    } else if (!buf) {
        result = DDS_RETCODE_BAD_PARAMETER;
        DDS_REPORT(result, "The data buffer parameter is NULL.");
    } else if (!si) {
        result = DDS_RETCODE_BAD_PARAMETER;
        DDS_REPORT(result, "The sample info parameter is NULL.");
    } else {
        result = DDS_Entity_claim_user_data(rd, (DDS_EntityUserData *)&info);
    }

    if (result == DDS_RETCODE_OK) {
        sampleList = cmn_samplesList_new(FALSE);
        uReader = u_dataReader(DDS_Entity_get_user_entity_for_test(rd));
        cmn_samplesList_reset(sampleList, maxs);
        uResult = u_dataReaderReadInstance(uReader, handle, dds_mask_to_u_mask(mask), cmn_reader_action, sampleList, OS_DURATION_ZERO);
        noReport = (uResult == U_RESULT_HANDLE_EXPIRED);
        if (uResult == U_RESULT_OK) {
            length = (int) cmn_samplesList_length(sampleList);
            if (length > 0) {
                if (buf[0] == 0) {
                    result = dds_loanRegistry_register(info->registry, buf, length);
                }
                if (result == DDS_RETCODE_OK) {
                    result = samples_flush_copy(rd, sampleList, length, buf, si, info->registry);
                }
            }
        } else {
            result = result_from_u_result(uResult);
        }
        cmn_samplesList_free(sampleList);
        DDS_Entity_release_user_data((DDS_EntityUserData)info);
    }
    DDS_REPORT_FLUSH(rd, (result != DDS_RETCODE_OK) && (result != DDS_RETCODE_NO_DATA) && !noReport);

    return ((result == DDS_RETCODE_OK) || (result == DDS_RETCODE_NO_DATA)) ?
                            length                                         :
                            DDS_ERRNO(result, DDS_MOD_READER, DDS_ERR_Mx);
}

int dds_read_cond (
  dds_entity_t rd,
  void ** buf,
  uint32_t maxs,
  dds_sample_info_t * si,
  dds_condition_t cond)
{
    DDS_ReturnCode_t result;
    struct DataReaderInfo *info = NULL;
    cmn_samplesList sampleList;
    u_object uObject;
    uint32_t mask;
    u_result uResult;
    int length = 0;

    DDS_REPORT_STACK();

    if (!rd) {
        result = DDS_RETCODE_BAD_PARAMETER;
        DDS_REPORT(result, "The reader parameter is NULL.");
    } else if (!buf) {
        result = DDS_RETCODE_BAD_PARAMETER;
        DDS_REPORT(result, "The data buffer parameter is NULL.");
    } else if (!si) {
        result = DDS_RETCODE_BAD_PARAMETER;
        DDS_REPORT(result, "The sample info parameter is NULL.");
    } else {
        result = DDS_ReadCondition_get_settings(cond, rd, &uObject, &mask);
    }

    if (result == DDS_RETCODE_OK) {
        if (u_objectKind(uObject) == U_QUERY) {
            result = DDS_Entity_claim_user_data(rd, (DDS_EntityUserData *)&info);
            if (result == DDS_RETCODE_OK) {
                sampleList = cmn_samplesList_new(FALSE);
                cmn_samplesList_reset(sampleList, maxs);
                uResult = u_queryRead(u_query(uObject), cmn_reader_action, sampleList, OS_DURATION_ZERO);
                if (uResult == U_RESULT_OK) {
                    length = (int) cmn_samplesList_length(sampleList);
                    if (length > 0) {
                        if (buf[0] == 0) {
                            result = dds_loanRegistry_register(info->registry, buf, length);
                        }
                        if (result == DDS_RETCODE_OK) {
                            result = samples_flush_copy(rd, sampleList, length, buf, si, info->registry);
                        }
                    }
                } else {
                    result = result_from_u_result(uResult);
                }
                cmn_samplesList_free(sampleList);
                DDS_Entity_release_user_data((DDS_EntityUserData)info);
            }
        } else {
            return dds_read(rd, buf, maxs, si, mask);
        }
    }

    DDS_REPORT_FLUSH(rd, (result != DDS_RETCODE_OK) && (result != DDS_RETCODE_NO_DATA));

    return ((result == DDS_RETCODE_OK) || (result == DDS_RETCODE_NO_DATA)) ?
                            length                                         :
                            DDS_ERRNO(result, DDS_MOD_READER, DDS_ERR_Mx);
}

int
dds_take (
  dds_entity_t rd,
  void ** buf,
  uint32_t maxs,
  dds_sample_info_t * si,
  uint32_t mask)
{
    DDS_ReturnCode_t result;
    struct DataReaderInfo *info = NULL;
    cmn_samplesList sampleList;
    u_dataReader uReader;
    u_result uResult;
    int length = 0;

    DDS_REPORT_STACK();

    if (!rd) {
        result = DDS_RETCODE_BAD_PARAMETER;
        DDS_REPORT(result, "The reader parameter is NULL.");
    } else if (!buf) {
        result = DDS_RETCODE_BAD_PARAMETER;
        DDS_REPORT(result, "The data buffer parameter is NULL.");
    } else if (!si) {
        result = DDS_RETCODE_BAD_PARAMETER;
        DDS_REPORT(result, "The sample info parameter is NULL.");
    } else {
        result = DDS_Entity_claim_user_data(rd, (DDS_EntityUserData *)&info);
    }

    if (result == DDS_RETCODE_OK) {
        sampleList = cmn_samplesList_new(FALSE);
        uReader = u_dataReader(DDS_Entity_get_user_entity_for_test(rd));
        cmn_samplesList_reset(sampleList, maxs);
        uResult = u_dataReaderTake(uReader, dds_mask_to_u_mask(mask), cmn_reader_action, sampleList, 0);
        if (uResult == U_RESULT_OK) {
            length = (int) cmn_samplesList_length(sampleList);
            if (length > 0) {
                if (buf[0] == 0) {
                    result = dds_loanRegistry_register(info->registry, buf, length);
                }
                if (result == DDS_RETCODE_OK) {
                    result = samples_flush_copy(rd, sampleList, length, buf, si, info->registry);
                }
            }
        } else {
            result = result_from_u_result(uResult);
        }
        cmn_samplesList_free(sampleList);
        DDS_Entity_release_user_data((DDS_EntityUserData)info);
    }

    DDS_REPORT_FLUSH(rd, (result != DDS_RETCODE_OK) && (result != DDS_RETCODE_NO_DATA));

    return ((result == DDS_RETCODE_OK) || (result == DDS_RETCODE_NO_DATA)) ?
                            length                                         :
                            DDS_ERRNO(result, DDS_MOD_READER, DDS_ERR_Mx);
}



int dds_take_instance (
  dds_entity_t rd,
  void ** buf,
  uint32_t maxs,
  dds_sample_info_t * si,
  dds_instance_handle_t handle,
  uint32_t mask)
{
    DDS_ReturnCode_t result;
    struct DataReaderInfo *info = NULL;
    cmn_samplesList sampleList;
    u_dataReader uReader;
    u_result uResult;
    bool noReport = false;
    int length = 0;

    DDS_REPORT_STACK();

    if (!rd) {
        result = DDS_RETCODE_BAD_PARAMETER;
        DDS_REPORT(result, "The reader parameter is NULL.");
    } else if (!buf) {
        result = DDS_RETCODE_BAD_PARAMETER;
        DDS_REPORT(result, "The data buffer parameter is NULL.");
    } else if (!si) {
        result = DDS_RETCODE_BAD_PARAMETER;
        DDS_REPORT(result, "The sample info parameter is NULL.");
    } else {
        result = DDS_Entity_claim_user_data(rd, (DDS_EntityUserData *)&info);
    }

    if (result == DDS_RETCODE_OK) {
        sampleList = cmn_samplesList_new(FALSE);
        uReader = u_dataReader(DDS_Entity_get_user_entity_for_test(rd));
        cmn_samplesList_reset(sampleList, maxs);
        uResult = u_dataReaderTakeInstance(uReader, handle, dds_mask_to_u_mask(mask), cmn_reader_action, sampleList, OS_DURATION_ZERO);
        noReport = (uResult == U_RESULT_HANDLE_EXPIRED);
        if (uResult == U_RESULT_OK) {
            length = (int) cmn_samplesList_length(sampleList);
            if (length > 0) {
                if (buf[0] == 0) {
                    result = dds_loanRegistry_register(info->registry, buf, length);
                }
                if (result == DDS_RETCODE_OK) {
                    result = samples_flush_copy(rd, sampleList, length, buf, si, info->registry);
                }
            }
        } else {
            result = result_from_u_result(uResult);
        }
        cmn_samplesList_free(sampleList);
        DDS_Entity_release_user_data((DDS_EntityUserData)info);
    }
    DDS_REPORT_FLUSH(rd, (result != DDS_RETCODE_OK) && (result != DDS_RETCODE_NO_DATA) && !noReport);

    return ((result == DDS_RETCODE_OK) || (result == DDS_RETCODE_NO_DATA)) ?
                            length                                         :
                            DDS_ERRNO(result, DDS_MOD_READER, DDS_ERR_Mx);
}

int dds_take_cond (
  dds_entity_t rd,
  void ** buf,
  uint32_t maxs,
  dds_sample_info_t * si,
  dds_condition_t cond)
{
    DDS_ReturnCode_t result;
    struct DataReaderInfo *info = NULL;
    cmn_samplesList sampleList;
    u_object uObject;
    uint32_t mask;
    u_result uResult;
    int length = 0;

    DDS_REPORT_STACK();

    if (!rd) {
        result = DDS_RETCODE_BAD_PARAMETER;
        DDS_REPORT(result, "The reader parameter is NULL.");
    } else if (!buf) {
        result = DDS_RETCODE_BAD_PARAMETER;
        DDS_REPORT(result, "The data buffer parameter is NULL.");
    } else if (!si) {
        result = DDS_RETCODE_BAD_PARAMETER;
        DDS_REPORT(result, "The sample info parameter is NULL.");
    } else {
        result = DDS_ReadCondition_get_settings(cond, rd, &uObject, &mask);
    }

    if (result == DDS_RETCODE_OK) {
        if (u_objectKind(uObject) == U_QUERY) {
            result = DDS_Entity_claim_user_data(rd, (DDS_EntityUserData *)&info);
            if (result == DDS_RETCODE_OK) {
                sampleList = cmn_samplesList_new(FALSE);
                cmn_samplesList_reset(sampleList, maxs);
                uResult = u_queryTake(u_query(uObject), cmn_reader_action, sampleList, OS_DURATION_ZERO);
                if (uResult == U_RESULT_OK) {
                    length = (int) cmn_samplesList_length(sampleList);
                    if (length > 0) {
                        if (buf[0] == 0) {
                            result = dds_loanRegistry_register(info->registry, buf, length);
                        }
                        if (result == DDS_RETCODE_OK) {
                            result = samples_flush_copy(rd, sampleList, length, buf, si, info->registry);
                        }
                    }
                } else {
                    result = result_from_u_result(uResult);
                }
                cmn_samplesList_free(sampleList);
                DDS_Entity_release_user_data((DDS_EntityUserData)info);
            }
        } else {
            return dds_take(rd, buf, maxs, si, mask);
        }
    }

    DDS_REPORT_FLUSH(rd, (result != DDS_RETCODE_OK) && (result != DDS_RETCODE_NO_DATA));

    return ((result == DDS_RETCODE_OK) || (result == DDS_RETCODE_NO_DATA)) ?
                            length                                         :
                            DDS_ERRNO(result, DDS_MOD_READER, DDS_ERR_Mx);
}

int dds_take_next (
    dds_entity_t rd,
    void ** buf,
    dds_sample_info_t * si)
{
    int length;
    DDS_REPORT_STACK();
    length = dds_take(rd, buf, 1, si, C99_DDS_NOT_READ_SAMPLE_STATE);
    DDS_REPORT_FLUSH(rd, length < 0);
    return length;
}

int
dds_read_next (
    dds_entity_t rd,
    void ** buf,
    dds_sample_info_t * si)
{
    int length;
    DDS_REPORT_STACK();
    length = dds_read(rd, buf, 1, si, C99_DDS_NOT_READ_SAMPLE_STATE);
    DDS_REPORT_FLUSH(rd, length < 0);
    return length;
}

void
dds_return_loan (
    dds_entity_t rd,
    void ** buf,
    uint32_t maxs)
{
    DDS_ReturnCode_t result;
    struct DataReaderInfo *info = NULL;

    DDS_REPORT_STACK();

    result = DDS_Entity_claim_user_data(rd, (DDS_EntityUserData *)&info);
    if (result == DDS_RETCODE_OK) {
        result = dds_loanRegistry_deregister(info->registry, buf, maxs);
        DDS_Entity_release_user_data((DDS_EntityUserData)info);
    }

    DDS_REPORT_FLUSH(rd, result != DDS_RETCODE_OK);
}

int
dds_datareader_get_listener(
    dds_entity_t e,
    dds_readerlistener_t *listener)
{
    DDS_ReturnCode_t result;
    struct DataReaderInfo *info = NULL;

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

    return DDS_ERRNO(result, DDS_MOD_READER, DDS_ERR_Mx);
}

int
dds_datareader_set_listener(
    dds_entity_t e,
    const dds_readerlistener_t *listener)
{
    DDS_ReturnCode_t result           = DDS_RETCODE_ERROR;
    struct DataReaderInfo *info       = NULL;
    struct DDS_DataReaderListener dpl = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
    dds_readerlistener_t *newListener = NULL;
    dds_readerlistener_t *oldListener = NULL;
    DDS_StatusMask mask;

    DDS_REPORT_STACK();

    result = DDS_Entity_claim_user_data(e, (DDS_EntityUserData *)&info);
    if (result == DDS_RETCODE_OK) {
        oldListener = info->listener;
        if (listener) {
            newListener = os_malloc(sizeof(dds_readerlistener_t));
            *newListener = *listener;
            mask = dds_status_get_enabled(e);
            info->listener = newListener;
            dds_datareader_listener_init(&dpl, info);
        } else {
            mask = 0;
            info->listener = NULL;
        }
        result = DDS_DataReader_set_listener(e, &dpl, mask);
        DDS_Entity_release_user_data((DDS_EntityUserData)info);
        os_free(oldListener);
    }

    DDS_REPORT_FLUSH(e, result != DDS_RETCODE_OK);

    return DDS_ERRNO(result, DDS_MOD_READER, DDS_ERR_Mx);
}

int
dds_datareader_delete(
    dds_entity_t rd)
{
    int result;
    struct DataReaderInfo *info = NULL;
    DDS_Subscriber subscriber;

    result = DDS_Entity_claim_user_data(rd, (DDS_EntityUserData *)&info);
    if (result == DDS_RETCODE_OK) {
        subscriber = DDS_DataReader_get_subscriber(rd);
        if (subscriber) {
            if (info->ownSubscriber) {
                result = dds_subscriber_delete(subscriber);
            } else {
                result = DDS_DataReader_delete_contained_entities(rd);
                if (result == DDS_RETCODE_OK) {
                    result = DDS_Subscriber_delete_datareader(subscriber, rd);
                }
            }
        } else {
            result = DDS_RETCODE_ALREADY_DELETED;
        }
        DDS_Entity_release_user_data((DDS_EntityUserData)info);
    }

    return DDS_ERRNO(result, DDS_MOD_READER, DDS_ERR_Mx);
}
