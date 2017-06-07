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

#ifndef SAC_COMMON_H
#define SAC_COMMON_H

#include "dds_dcps_private.h"
#include "sac_genericCopyCache.h"
#include "cmn_qosProvider.h"
#include "cmn_listenerDispatcher.h"
#include "u_user.h"
#include "v_kernel.h"
#include "c_base.h"
#include "cmn_samplesList.h"
#include "vortex_os.h"

extern const DDS_DomainParticipantFactoryQos *
    DDS_PARTICIPANTFACTORY_QOS_DEFAULT;

#define DDS_SAMPLE_STATE_FLAGS \
        (DDS_READ_SAMPLE_STATE | \
         DDS_NOT_READ_SAMPLE_STATE)

#define DDS_VIEW_STATE_FLAGS \
        (DDS_NEW_VIEW_STATE | \
         DDS_NOT_NEW_VIEW_STATE)

#define DDS_INSTANCE_STATE_FLAGS \
        (DDS_ALIVE_INSTANCE_STATE | \
         DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE | \
         DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE)

#define DDS_SAMPLE_MASK(sample_states, view_states, instance_states) \
        (sample_states & DDS_SAMPLE_STATE_FLAGS) | \
        ((view_states & DDS_VIEW_STATE_FLAGS) << 2) | \
        ((instance_states & DDS_INSTANCE_STATE_FLAGS) << 4)

#define DDS_SAMPLE_MASK_IS_VALID(sample_states, view_states, instance_states) \
    ((((sample_states == DDS_ANY_SAMPLE_STATE) || \
       ((sample_states & ~DDS_SAMPLE_STATE_FLAGS) == 0)) && \
      ((view_states == DDS_ANY_VIEW_STATE) ||  \
       ((view_states & ~DDS_VIEW_STATE_FLAGS) == 0)) && \
      ((instance_states == DDS_ANY_INSTANCE_STATE) ||  \
       ((instance_states & ~DDS_INSTANCE_STATE_FLAGS) == 0))) \
     ? DDS_RETCODE_OK : DDS_RETCODE_BAD_PARAMETER)

#define DDS_HISTORY_DEPTH_DEFAULT  (1)
#define DDS_RESOURCE_LIMIT_INFINITE (-1)

#define DDS_MAX_BLOCKING_TIME_DEFAULT {0, 100000000 } /* 100ms */
#define DDS_DEFAULT_LISTENER_STACKSIZE (0)

#define DDS_OwnershipStrengthQosPolicy_default_value { \
            0 \
        }

extern const DDS_OwnershipStrengthQosPolicy DDS_OwnershipStrengthQosPolicy_default;

#define DDS_DurabilityQosPolicy_default_value { \
           DDS_VOLATILE_DURABILITY_QOS \
        }

extern const DDS_DurabilityQosPolicy DDS_DurabilityQosPolicy_default;

#define DDS_DeadlineQosPolicy_default_value { \
           DDS_DURATION_INFINITE \
        }

extern const DDS_DeadlineQosPolicy DDS_DeadlineQosPolicy_default;

#define DDS_LatencyBudgetQosPolicy_default_value { \
           DDS_DURATION_ZERO \
        }

extern const DDS_LatencyBudgetQosPolicy DDS_LatencyBudgetQosPolicy_default;

#define DDS_LivelinessQosPolicy_default_value { \
           DDS_AUTOMATIC_LIVELINESS_QOS, \
           DDS_DURATION_INFINITE \
        }

extern const DDS_LivelinessQosPolicy DDS_LivelinessQosPolicy_default;

#define DDS_ReliabilityQosPolicy_default_value { \
           DDS_BEST_EFFORT_RELIABILITY_QOS, \
           DDS_MAX_BLOCKING_TIME_DEFAULT, \
           FALSE \
        }

extern const DDS_ReliabilityQosPolicy DDS_ReliabilityQosPolicy_default;

#define DDS_ReliabilityQosPolicy_writer_default_value { \
           DDS_RELIABLE_RELIABILITY_QOS, \
           DDS_MAX_BLOCKING_TIME_DEFAULT, \
           FALSE \
        }

extern const DDS_ReliabilityQosPolicy DDS_ReliabilityQosPolicy_writer_default;

#define DDS_DestinationOrderQosPolicy_default_value { \
           DDS_BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS \
        }

extern const DDS_DestinationOrderQosPolicy DDS_DestinationOrderQosPolicy_default;

#define DDS_HistoryQosPolicy_default_value { \
           DDS_KEEP_LAST_HISTORY_QOS, \
           DDS_HISTORY_DEPTH_DEFAULT \
        }

extern const DDS_HistoryQosPolicy DDS_HistoryQosPolicy_default;

#define DDS_ResourceLimitsQosPolicy_default_value { \
           DDS_RESOURCE_LIMIT_INFINITE, \
           DDS_RESOURCE_LIMIT_INFINITE, \
           DDS_RESOURCE_LIMIT_INFINITE \
        }

extern const DDS_ResourceLimitsQosPolicy DDS_ResourceLimitsQosPolicy_default;

#define DDS_TransportPriorityQosPolicy_default_value { \
           0 \
        }

extern const DDS_TransportPriorityQosPolicy DDS_TransportPriorityQosPolicy_default;

#define DDS_LifespanQosPolicy_default_value { \
           DDS_DURATION_INFINITE \
        }

extern const DDS_LifespanQosPolicy DDS_LifespanQosPolicy_default;

#define DDS_OwnershipQosPolicy_default_value { \
           DDS_SHARED_OWNERSHIP_QOS \
        }

extern const DDS_OwnershipQosPolicy DDS_OwnershipQosPolicy_default;

#define DDS_PresentationQosPolicy_default_value { \
           DDS_INSTANCE_PRESENTATION_QOS, \
           FALSE, \
           FALSE \
        }

extern const DDS_PresentationQosPolicy DDS_PresentationQosPolicy_default;

#define DDS_EntityFactoryQosPolicy_default_value { \
           TRUE \
        }

extern const DDS_EntityFactoryQosPolicy DDS_EntityFactoryQosPolicy_default;

#define DDS_WriterDataLifecycleQosPolicy_default_value { \
            TRUE, \
            DDS_DURATION_INFINITE, \
            DDS_DURATION_INFINITE \
        }

extern const DDS_WriterDataLifecycleQosPolicy DDS_WriterDataLifecycleQosPolicy_default;

#define DDS_SchedulingQosPolicy_default_value { \
            { DDS_SCHEDULE_DEFAULT }, \
            { DDS_PRIORITY_RELATIVE }, \
            0 \
        }

extern const DDS_SchedulingQosPolicy DDS_SchedulingQosPolicy_default;

#define DDS_UserDataQosPolicy_default_value { \
            { 0, 0, NULL, FALSE } \
        }

extern const DDS_UserDataQosPolicy DDS_UserDataQosPolicy_default;

#define DDS_TopicDataQosPolicy_default_value { \
             { 0, 0, NULL, FALSE } \
        }

extern const DDS_TopicDataQosPolicy DDS_TopicDataQosPolicy_default;

#define DDS_GroupDataQosPolicy_default_value { \
             { 0, 0, NULL, FALSE } \
        }

extern const DDS_GroupDataQosPolicy DDS_GroupDataQosPolicy_default;

#define DDS_PartitionQosPolicy_default_value { \
             { 0, 0, NULL, FALSE } \
        }

extern const DDS_PartitionQosPolicy DDS_PartitionQosPolicy_default;

#define DDS_ReaderDataLifecycleQosPolicy_default_value { \
             DDS_DURATION_INFINITE, \
             DDS_DURATION_INFINITE, \
             FALSE, \
             TRUE, \
             { DDS_MINIMUM_INVALID_SAMPLES } \
        }

extern const DDS_ReaderDataLifecycleQosPolicy DDS_ReaderDataLifecycleQosPolicy_default;

#define DDS_TimeBasedFilterQosPolicy_default_value { \
             DDS_DURATION_ZERO \
        }

extern const DDS_TimeBasedFilterQosPolicy DDS_TimeBasedFilterQosPolicy_default;

#define DDS_SubscriptionKeyQosPolicy_default_value { \
             FALSE, \
             { 0, 0, NULL, FALSE } \
        }

extern const DDS_SubscriptionKeyQosPolicy DDS_SubscriptionKeyQosPolicy_default;

#define DDS_ReaderLifespanQosPolicy_default_value { \
             FALSE, \
             DDS_DURATION_INFINITE \
        }

extern const DDS_ReaderLifespanQosPolicy DDS_ReaderLifespanQosPolicy_default;

#define DDS_ShareQosPolicy_default_value { \
             "", \
             FALSE \
        }

extern const DDS_ShareQosPolicy DDS_ShareQosPolicy_default;

#define DDS_ViewKeyQosPolicy_default_value { \
             FALSE, \
             { 0, 0, NULL, FALSE } \
        }

extern const DDS_ViewKeyQosPolicy DDS_ViewKeyQosPolicy_default;

#define DDS_DurabilityServiceQosPolicy_default_value { \
            DDS_DURATION_ZERO, \
            DDS_KEEP_LAST_HISTORY_QOS, \
            1, \
            DDS_RESOURCE_LIMIT_INFINITE, \
            DDS_RESOURCE_LIMIT_INFINITE, \
            DDS_RESOURCE_LIMIT_INFINITE \
        }

/* The following symbols are made public visible for the qosprovider test.
 * TODO: replace this either by changing the test or extend the API to have some other means to initialize qos.
 */
#ifdef OSPL_BUILD_DCPSSAC
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

OS_API extern const DDS_DurabilityServiceQosPolicy DDS_DurabilityServiceQosPolicy_default;

OS_API extern const DDS_DomainParticipantFactoryQos DDS_DomainParticipantFactoryQos_default;

OS_API extern const DDS_DomainParticipantQos DDS_DomainParticipantQos_default;

OS_API extern const DDS_TopicQos DDS_TopicQos_default;

OS_API extern const DDS_PublisherQos DDS_PublisherQos_default;

OS_API extern const DDS_SubscriberQos DDS_SubscriberQos_default;

OS_API extern const DDS_DataReaderQos DDS_DataReaderQos_default;

OS_API extern const DDS_DataReaderViewQos DDS_DataReaderViewQos_default;

OS_API extern const DDS_DataWriterQos DDS_DataWriterQos_default;

#undef OS_API

C_CLASS(_Object);
C_CLASS(_Entity);
C_CLASS(_DomainParticipantFactory);
C_CLASS(_DomainParticipant);
C_CLASS(_Domain);
C_CLASS(_TypeSupport);
C_CLASS(_WaitSet);
C_CLASS(_TopicDescription);
C_CLASS(_Topic);
C_CLASS(_ContentFilteredTopic);
C_CLASS(_MultiTopic);
C_CLASS(_Publisher);
C_CLASS(_Subscriber);
C_CLASS(_DataWriter);
C_CLASS(_DataReader);
C_CLASS(_DataReaderView);
C_CLASS(_Condition);
C_CLASS(_StatusCondition);
C_CLASS(_ReadCondition);
C_CLASS(_QueryCondition);
C_CLASS(_GuardCondition);
C_CLASS(_ErrorInfo);
C_CLASS(_QosProvider);

C_CLASS(DDS_LoanRegistry);
C_CLASS(DDS_expression);

/* **** WARNING!!!!! ****
 * Do NOT alter the sequence of the following enum values
 * without addapting DDS_Object_claim().
 */
typedef enum {
    DDS_UNDEFINED,

    /* Objects: */
    DDS_ERRORINFO,
    DDS_DOMAINFACTORY,
    DDS_TYPESUPPORT,
    DDS_WAITSET,
    DDS_DOMAIN,

    /* Conditions: */
    DDS_CONDITION,
    DDS_STATUSCONDITION,
    DDS_GUARDCONDITION,
    DDS_READCONDITION,
    DDS_QUERYCONDITION,

    /* Entities: */
    DDS_ENTITY,
    DDS_DOMAINPARTICIPANT,
    DDS_PUBLISHER,
    DDS_SUBSCRIBER,
    DDS_DATAWRITER,
    DDS_DATAREADER,
    DDS_DATAREADERVIEW,

    /* Topic Descriptions: */
    DDS_TOPICDESCRIPTION,
    DDS_TOPIC,
    DDS_CONTENTFILTEREDTOPIC,
    DDS_MULTITOPIC,

    DDS_QOSPROVIDER,

    DDS_OBJECT_COUNT
} DDS_ObjectKind;

#define _CAST(_this, type) ((type)(_this))

#define DDS_Object(_this)                   _CAST(_this, DDS_Object)
#define DDS_Entity(_this)                   _CAST(_this, DDS_Entity)
#define DDS_DomainParticipantFactory(_this) _CAST(_this, DDS_DomainParticipantFactory)
#define DDS_DomainParticipant(_this)        _CAST(_this, DDS_DomainParticipant)
#define DDS_TypeSupport(_this)              _CAST(_this, DDS_TypeSupport)
#define DDS_WaitSet(_this)                  _CAST(_this, DDS_WaitSet)
#define DDS_Domain(_this)                   _CAST(_this, DDS_Domain)
#define DDS_TopicDescription(_this)         _CAST(_this, DDS_TopicDescription)
#define DDS_Topic(_this)                    _CAST(_this, DDS_Topic)
#define DDS_ContentFilteredTopic(_this)     _CAST(_this, DDS_ContentFilteredTopic)
#define DDS_MultiTopic(_this)               _CAST(_this, DDS_MultiTopic)
#define DDS_Publisher(_this)                _CAST(_this, DDS_Publisher)
#define DDS_DataWriter(_this)               _CAST(_this, DDS_DataWriter)
#define DDS_Subscriber(_this)               _CAST(_this, DDS_Subscriber)
#define DDS_DataReader(_this)               _CAST(_this, DDS_DataReader)
#define DDS_DataReaderView(_this)           _CAST(_this, DDS_DataReaderView)
#define DDS_Condition(_this)                _CAST(_this, DDS_Condition)
#define DDS_StatusCondition(_this)          _CAST(_this, DDS_StatusCondition)
#define DDS_ReadCondition(_this)            _CAST(_this, DDS_ReadCondition)
#define DDS_QueryCondition(_this)           _CAST(_this, DDS_QueryCondition)
#define DDS_GuardCondition(_this)           _CAST(_this, DDS_GuardCondition)
#define DDS_ErrorInfo(_this)                _CAST(_this, DDS_ErrorInfo)

#define _Object(_this)                   _CAST(_this, _Object)
#define _Entity(_this)                   _CAST(_this, _Entity)
#define _DomainParticipantFactory(_this) _CAST(_this, _DomainParticipantFactory)
#define _DomainParticipant(_this)        _CAST(_this, _DomainParticipant)
#define _TypeSupport(_this)              _CAST(_this, _TypeSupport)
#define _WaitSet(_this)                  _CAST(_this, _WaitSet)
#define _Domain(_this)                   _CAST(_this, _Domain)
#define _TopicDescription(_this)         _CAST(_this, _TopicDescription)
#define _Topic(_this)                    _CAST(_this, _Topic)
#define _ContentFilteredTopic(_this)     _CAST(_this, _ContentFilteredTopic)
#define _MultiTopic(_this)               _CAST(_this, _MultiTopic)
#define _Publisher(_this)                _CAST(_this, _Publisher)
#define _DataWriter(_this)               _CAST(_this, _DataWriter)
#define _Subscriber(_this)               _CAST(_this, _Subscriber)
#define _DataReader(_this)               _CAST(_this, _DataReader)
#define _DataReaderView(_this)           _CAST(_this, _DataReaderView)
#define _Condition(_this)                _CAST(_this, _Condition)
#define _StatusCondition(_this)          _CAST(_this, _StatusCondition)
#define _ReadCondition(_this)            _CAST(_this, _ReadCondition)
#define _QueryCondition(_this)           _CAST(_this, _QueryCondition)
#define _GuardCondition(_this)           _CAST(_this, _GuardCondition)
#define _ErrorInfo(_this)                _CAST(_this, _ErrorInfo)

typedef DDS_ReturnCode_t (*_Object_destructor_t)(_Object o);

C_STRUCT(_Object) {
    DDS_ObjectKind       kind;
    _Object_destructor_t destructor;
    os_mutex             mutex;
    os_cond              cond;
    os_int32             domainId;
};

C_STRUCT(_DomainParticipantFactory) {
    C_EXTENDS(_Object);
    DDS_DomainParticipantFactoryQos qos;
    c_iter                          participantList;
    c_iter                          domainList;
    DDS_DomainParticipantQos        defaultQos;
};

C_STRUCT(_Entity) {
    C_EXTENDS(_Object);
    u_entity                uEntity;
    DDS_StatusMask          interest;
    DDS_StatusCondition     statusCondition;
    DDS_InstanceHandle_t    handle;
    DDS_boolean             listenerEnabled;
    cmn_listenerDispatcher  listenerDispatcher;
    os_int64                maxSupportedSeconds;
    DDS_EntityUserData      userData;
};

C_STRUCT(_Domain) {
    C_EXTENDS(_Entity);
    DDS_DomainId_t domainId;
};

C_STRUCT(_TypeSupport) {
    C_EXTENDS(_Object);
    DDS_string           type_name;
    DDS_string           type_keys;
    const char **        type_desc;
    int                  type_descArrSize;
    int                  type_descLength;
    DDS_unsigned_long    alloc_size;
    DDS_allocBuffer      alloc_buffer;
    DDS_copyIn           copy_in;
    DDS_copyOut          copy_out;
    DDS_copyCache        copy_cache;
    const char *         internal_type_name;
};

C_STRUCT(_WaitSet) {
    C_EXTENDS(_Object);
    u_waitset uWaitset;
    c_iter conditions;
    c_iter guards;
};

C_STRUCT(_DomainParticipant) {
    C_EXTENDS(_Entity);
    DDS_DomainParticipantFactory factory;
    DDS_DomainId_t               domainId;
    DDS_char                    *domainName;
    DDS_PublisherQos            *defaultPublisherQos;
    DDS_SubscriberQos           *defaultSubscriberQos;
    DDS_TopicQos                *defaultTopicQos;
    struct DDS_ExtDomainParticipantListener listener;
    DDS_Subscriber               builtinSubscriber;
    c_iter                       builtinTopicList;
    c_iter                       publisherList;
    c_iter                       subscriberList;
    c_iter                       topicList;
    c_iter                       cfTopicList;
    c_iter                       multiTopicList;
    c_iter                       typeSupportBindings;
    DDS_boolean                  factoryAutoEnable;
};

C_STRUCT(_TopicDescription) {
    C_EXTENDS(_Entity);
    DDS_DomainParticipant  participant;
    DDS_string             type_name;
    DDS_TypeSupport        typeSupport;
    DDS_string             topic_name;
    DDS_long               refCount;
    DDS_string             expression;
};

C_STRUCT(_Topic) {
    C_EXTENDS(_TopicDescription);
    struct DDS_ExtTopicListener listener;
    DDS_StatusMask topicListenerInterest;
    DDS_StatusMask participantListenerInterest;
};

C_STRUCT(_ContentFilteredTopic) {
    C_EXTENDS(_TopicDescription);
    DDS_Topic       relatedTopic;
    DDS_char       *expression;
    DDS_StringSeq  *parameters;
};

C_STRUCT(_MultiTopic) {
    C_EXTENDS(_TopicDescription);
    void        *empty;
};

C_STRUCT(_Publisher) {
    C_EXTENDS(_Entity);
    DDS_DomainParticipant        participant;
    DDS_DataWriterQos           *defaultDataWriterQos;
    struct DDS_PublisherListener listener;
    c_iter                       writerList;
    DDS_boolean                  factoryAutoEnable;
};

C_STRUCT(_Subscriber) {
    C_EXTENDS(_Entity);
    DDS_DomainParticipant         participant;
    DDS_DataReaderQos            *defaultDataReaderQos;
    struct DDS_SubscriberListener listener;
    c_iter                        readerList;
    DDS_boolean                   factoryAutoEnable;
};

C_STRUCT(_DataWriter)
{
    C_EXTENDS(_Entity);

    DDS_Publisher                  publisher;
    DDS_Topic                      topic;
    struct DDS_DataWriterListener  listener;
    DDS_copyIn                     copy_in;
    DDS_copyOut                    copy_out;
    DDS_copyCache                  copy_cache;

    DDS_unsigned_long              allocSize;
};

C_STRUCT(_DataReader)
{
    C_EXTENDS(_Entity);

    DDS_Subscriber        subscriber;
    DDS_TopicDescription  topicDescription;
    DDS_DataReaderViewQos *defaultDataReaderViewQos;
    struct DDS_DataReaderListener listener;
    DDS_copyIn            copy_in;
    DDS_copyOut           copy_out;
    DDS_copyCache         copy_cache;

    DDS_unsigned_long     messageOffset;
    DDS_unsigned_long     userdataOffset;

    c_iter                readConditionList;
    c_iter                queryConditionList;
    c_iter                dataReaderViewList;
    DDS_LoanRegistry      loanRegistry;
    cmn_samplesList       samplesList;
};

C_STRUCT(_DataReaderView) {
    C_EXTENDS(_Entity);
    DDS_DataReader   datareader;
    c_iter           readConditionList;
    c_iter           queryConditionList;
    DDS_LoanRegistry loanRegistry;
    cmn_samplesList  samplesList;
};

typedef DDS_boolean (*GetTriggerValue)(_Condition condition);

C_STRUCT(_Condition) {
    C_EXTENDS(_Object);
    u_object            uObject;
    c_iter              waitsets;
    GetTriggerValue     getTriggerValue;
};

C_STRUCT(_StatusCondition) {
    C_EXTENDS(_Condition);
    DDS_Entity      entity;
    DDS_StatusMask  enabledStatusMask;
};

C_STRUCT(_ReadCondition) {
    C_EXTENDS(_Condition);
    u_query             uQuery;
    u_kind              sourceKind;
    DDS_Entity          source;
    cmn_samplesList     samplesList;
    DDS_SampleStateMask sample_states;
    DDS_ViewStateMask view_states;
    DDS_InstanceStateMask instance_states;
};

C_STRUCT(_QueryCondition) {
    C_EXTENDS(_ReadCondition);
    DDS_char       *query_expression;
    DDS_StringSeq  *query_parameters;
};

C_STRUCT(_GuardCondition) {
    C_EXTENDS(_Condition);
    DDS_boolean     triggerValue;
};

C_STRUCT(_ErrorInfo) {
    C_EXTENDS(_Object);
    DDS_boolean      valid;
    DDS_ReturnCode_t code;
    DDS_string       location;
    DDS_string       source_line;
    DDS_string       stack_trace;
    DDS_string       message;
};

C_STRUCT(_QosProvider) {
    C_EXTENDS(_Object);
    cmn_qosProvider qpQosProvider;
};

DDS_ReturnCode_t
DDS_ReturnCode_get(
    u_result result);

const DDS_char *
DDS_ObjectKind_image(
    DDS_ObjectKind kind);

v_eventMask
DDS_StatusMask_get_eventMask(
    DDS_StatusMask statusMask);

DDS_ReturnCode_t
DDS_Duration_init_mapping (
    const DDS_Duration_t *from,
    os_duration *to);

DDS_ReturnCode_t
DDS_Duration_copyIn (
    const DDS_Duration_t *from,
    os_duration *to);

DDS_ReturnCode_t
DDS_Duration_copyOut (
    const os_duration *from,
    DDS_Duration_t *to);

DDS_ReturnCode_t
DDS_Time_copyIn (
    const DDS_Time_t *from,
    os_timeW *to,
    os_int64 maxSupportedSeconds);

DDS_ReturnCode_t
DDS_Time_copyOut (
    const os_timeW *from,
    DDS_Time_t *to);

DDS_boolean
DDS_Time_is_valid (
    const DDS_Time_t *time,
    os_int64 maxSupportedSeconds);

DDS_boolean
DDS_sequence_is_valid (
    const _DDS_sequence seq);

DDS_boolean
DDS_Duration_is_valid(
    const DDS_Duration_t *duration);

void
DDS_Duration_from_mapping (
    const os_duration *from,
    DDS_Duration_t *to);

/*
 * StringSeq operations
 */
DDS_boolean
DDS_StringSeq_is_valid (
    const DDS_StringSeq *seq);

DDS_string
DDS_StringSeq_to_string (
    const DDS_StringSeq *sequence,
    const DDS_string delimiter);

DDS_boolean
DDS_string_to_StringSeq (
    const DDS_string  string,
    const DDS_string  delimiter,
    DDS_StringSeq    *sequence);

DDS_StringSeq *
DDS_StringSeq_dup(
    const DDS_StringSeq *in);

DDS_ReturnCode_t
DDS_StringSeq_init(
    DDS_StringSeq *seq,
    const DDS_StringSeq *template);

DDS_ReturnCode_t
DDS_StringSeq_deinit(
    DDS_StringSeq *seq);

/*
 * DomainParticipantFactoryQos operations
 */
DDS_ReturnCode_t
DDS_DomainParticipantFactoryQos_is_consistent (
    const DDS_DomainParticipantFactoryQos *qos);

DDS_ReturnCode_t
DDS_DomainParticipantFactoryQos_init (
    DDS_DomainParticipantFactoryQos *qos,
    const DDS_DomainParticipantFactoryQos *template);

DDS_ReturnCode_t
DDS_DomainParticipantFactoryQos_deinit (
    DDS_DomainParticipantFactoryQos *qos);

/*
 * DomainParticipantQos operations
 */
DDS_ReturnCode_t
DDS_DomainParticipantQos_is_consistent (
    const DDS_DomainParticipantQos *qos);

DDS_ReturnCode_t
DDS_DomainParticipantQos_init (
    DDS_DomainParticipantQos *qos,
    const DDS_DomainParticipantQos *template);

DDS_ReturnCode_t
DDS_DomainParticipantQos_deinit (
    DDS_DomainParticipantQos *qos);

u_participantQos
DDS_DomainParticipantQos_copyIn (
    const DDS_DomainParticipantQos *qos);

DDS_ReturnCode_t
DDS_DomainParticipantQos_copyOut (
    const u_participantQos uQos,
    DDS_DomainParticipantQos *qos);

/*
 * TopicQos operations
 */
DDS_ReturnCode_t
DDS_TopicQos_is_consistent (
    const DDS_TopicQos *qos);

DDS_ReturnCode_t
DDS_TopicQos_init (
    DDS_TopicQos *qos,
    const DDS_TopicQos *template);

DDS_ReturnCode_t
DDS_TopicQos_deinit (
    DDS_TopicQos *qos);

u_topicQos
DDS_TopicQos_copyIn (
    const DDS_TopicQos *qos);

DDS_ReturnCode_t
DDS_TopicQos_copyOut (
    const u_topicQos uQos,
    DDS_TopicQos *qos);

/*
 * PublisherQos operations
 */
DDS_ReturnCode_t
DDS_PublisherQos_is_consistent (
    const DDS_PublisherQos *qos);

DDS_ReturnCode_t
DDS_PublisherQos_init (
    DDS_PublisherQos *qos,
    const DDS_PublisherQos *template);

DDS_ReturnCode_t
DDS_PublisherQos_deinit (
    DDS_PublisherQos *qos);

u_publisherQos
DDS_PublisherQos_copyIn (
    const DDS_PublisherQos *qos);

DDS_ReturnCode_t
DDS_PublisherQos_copyOut (
    const u_publisherQos uQos,
    DDS_PublisherQos *qos);

/*
 * SubscriberQos operations
 */
DDS_ReturnCode_t
DDS_SubscriberQos_is_consistent (
    const DDS_SubscriberQos *qos);

DDS_ReturnCode_t
DDS_SubscriberQos_init (
    DDS_SubscriberQos *qos,
    const DDS_SubscriberQos *template);

DDS_ReturnCode_t
DDS_SubscriberQos_deinit (
    DDS_SubscriberQos *qos);

u_subscriberQos
DDS_SubscriberQos_copyIn (
    const DDS_SubscriberQos *qos);

DDS_ReturnCode_t
DDS_SubscriberQos_copyOut (
    const u_subscriberQos uQos,
    DDS_SubscriberQos *qos);

/*
 * DataWriterQos operations
 */
DDS_ReturnCode_t
DDS_DataWriterQos_is_consistent (
    const DDS_DataWriterQos *qos);

DDS_ReturnCode_t
DDS_DataWriterQos_init (
    DDS_DataWriterQos *qos,
    const DDS_DataWriterQos *template);

DDS_ReturnCode_t
DDS_DataWriterQos_deinit (
    DDS_DataWriterQos *qos);

u_writerQos
DDS_DataWriterQos_copyIn (
    const DDS_DataWriterQos *qos);

DDS_ReturnCode_t
DDS_DataWriterQos_copyOut (
    const v_writerQos uQos,
    DDS_DataWriterQos *qos);

/*
 * DataReaderQos operations
 */
DDS_ReturnCode_t
DDS_DataReaderQos_is_consistent (
    const DDS_DataReaderQos *qos);

DDS_ReturnCode_t
DDS_DataReaderQos_init (
    DDS_DataReaderQos *qos,
    const DDS_DataReaderQos *template);

DDS_ReturnCode_t
DDS_DataReaderQos_deinit (
    DDS_DataReaderQos *qos);

u_readerQos
DDS_DataReaderQos_copyIn (
    const DDS_DataReaderQos *qos);

DDS_ReturnCode_t
DDS_DataReaderQos_copyOut (
    const u_readerQos uQos,
    DDS_DataReaderQos *qos);

/*
 * DataReaderViewQos operations
 */
DDS_ReturnCode_t
DDS_DataReaderViewQos_is_consistent (
    const DDS_DataReaderViewQos *qos);

DDS_ReturnCode_t
DDS_DataReaderViewQos_init (
    DDS_DataReaderViewQos *qos,
    const DDS_DataReaderViewQos *template);

DDS_ReturnCode_t
DDS_DataReaderViewQos_deinit (
    DDS_DataReaderViewQos *qos);

u_dataViewQos
DDS_DataReaderViewQos_copyIn (
    const DDS_DataReaderViewQos *qos);

DDS_ReturnCode_t
DDS_DataReaderViewQos_copyOut (
    const u_dataViewQos uQos,
    DDS_DataReaderViewQos *qos);

void
DDS_InconsistentTopicStatus_init (
    DDS_InconsistentTopicStatus *status,
    struct v_inconsistentTopicInfo *info);

void
DDS_LivelinessLostStatus_init (
    DDS_LivelinessLostStatus *status,
    struct v_livelinessLostInfo *info);

void
DDS_RequestedDeadlineMissedStatus_init (
    DDS_RequestedDeadlineMissedStatus *status,
    struct v_deadlineMissedInfo *info);

void
DDS_OfferedDeadlineMissedStatus_init (
    DDS_OfferedDeadlineMissedStatus *status,
    struct v_deadlineMissedInfo *info);

void
DDS_SampleRejectedStatus_init (
    DDS_SampleRejectedStatus *status,
    struct v_sampleRejectedInfo *info);

void
DDS_LivelinessChangedStatus_init (
    DDS_LivelinessChangedStatus *status,
    struct v_livelinessChangedInfo *info);

void
DDS_RequestedIncompatibleQosStatus_init (
    DDS_RequestedIncompatibleQosStatus *status,
    struct v_incompatibleQosInfo *info);

void
DDS_OfferedIncompatibleQosStatus_init (
    DDS_OfferedIncompatibleQosStatus *status,
    struct v_incompatibleQosInfo *info);

void
DDS_SampleLostStatus_init (
    DDS_SampleLostStatus *status,
    struct v_sampleLostInfo *info);

void
DDS_SubscriptionMatchedStatus_init (
    DDS_SubscriptionMatchedStatus *status,
    struct v_topicMatchInfo *info);

void
DDS_PublicationMatchedStatus_init (
    DDS_PublicationMatchedStatus *status,
    struct v_topicMatchInfo *info);

#endif
