/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef DDS_DCPS_H
#define DDS_DCPS_H

#if defined (__cplusplus)
extern "C" {
#endif
#include "dds_dcpsSplDcps.h"
#include "os_if.h"

#ifdef OSPL_BUILD_DCPSSAC
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */


typedef short int                               DDS_short;
typedef int                                     DDS_long;
typedef long long int                           DDS_long_long;
typedef unsigned short int                      DDS_unsigned_short;
typedef unsigned int                            DDS_unsigned_long;
typedef unsigned long long int                  DDS_unsigned_long_long;
typedef float                                   DDS_float;
typedef double                                  DDS_double;
typedef long double                             DDS_long_double;
typedef char                                    DDS_char;
typedef unsigned char                           DDS_octet;
typedef unsigned char                           DDS_boolean;
typedef DDS_char *                              DDS_string;

#ifndef FALSE
#define FALSE                                   0
#endif
#ifndef TRUE
#define TRUE                                    1
#endif

/* Generic DDS object pointer */
typedef void *                                  DDS_Object;
#if defined (__cplusplus)
#define DDS_OBJECT_NIL                          NULL
#else
#define DDS_OBJECT_NIL                          (DDS_Object)NULL
#endif

/* Generic allocation/release functions */
OS_API void DDS_free (void *);
OS_API extern DDS_char *DDS_string_alloc (DDS_unsigned_long len);
typedef DDS_boolean (*dealloactorType)(void *object);
OS_API void * DDS_sequence_malloc (void);
OS_API void * DDS_sequence_allocbuf (dealloactorType deallocator,
                                     DDS_unsigned_long len,
                                     DDS_unsigned_long count);

/* Sequence support routines */
OS_API extern void DDS_sequence_set_release (void *sequence, DDS_boolean release);
OS_API extern DDS_boolean DDS_sequence_get_release (void *sequence);

/* Default QoS definitions */
#define DDS_PARTICIPANT_QOS_DEFAULT         NULL
#define DDS_TOPIC_QOS_DEFAULT               NULL
#define DDS_PUBLISHER_QOS_DEFAULT           NULL
#define DDS_SUBSCRIBER_QOS_DEFAULT          NULL
#define DDS_DATAREADER_QOS_DEFAULT          NULL
#define DDS_DATAVIEW_QOS_DEFAULT            NULL
#define DDS_DATAWRITER_QOS_DEFAULT          NULL
#define DDS_DATAWRITER_QOS_USE_TOPIC_QOS    ((DDS_DataWriterQos *)-1)
#define DDS_DATAREADER_QOS_USE_TOPIC_QOS    ((DDS_DataReaderQos *)-1)

/* Environment replacement type */

#define DDS_DOMAINID_TYPE_NATIVE            DDS_string
#define DDS_HANDLE_TYPE_NATIVE              long long
#define DDS_HANDLE_NIL_NATIVE               0LL
#define DDS_BUILTIN_TOPIC_KEY_TYPE_NATIVE   DDS_long

#define DDS_TheParticipantFactory           DDS_DomainParticipantFactory_get_instance()

/*
 * DDS DCPS IDL definitions
 */

/*
 * typedef DOMAINID_TYPE_NATIVE DomainId_t;
 */
typedef DDS_DOMAINID_TYPE_NATIVE DDS_DomainId_t;

/*
 * typedef HANDLE_TYPE_NATIVE InstanceHandle_t;
 */
typedef DDS_HANDLE_TYPE_NATIVE DDS_InstanceHandle_t;

/*
 * typedef BUILTIN_TOPIC_KEY_TYPE_NATIVE BuiltinTopicKey_t[3];
 */
typedef DDS_BUILTIN_TOPIC_KEY_TYPE_NATIVE DDS_BuiltinTopicKey_t[3];

/*
 * typedef sequence<InstanceHandle_t> InstanceHandleSeq;
 */
#ifndef _DDS_sequence_DDS_InstanceHandle_t_defined
#define _DDS_sequence_DDS_InstanceHandle_t_defined
typedef struct {
    DDS_unsigned_long _maximum;
    DDS_unsigned_long _length;
    DDS_InstanceHandle_t *_buffer;
    DDS_boolean _release;
} DDS_sequence_DDS_InstanceHandle_t;

OS_API DDS_sequence_DDS_InstanceHandle_t *DDS_sequence_DDS_InstanceHandle_t__alloc(void);
OS_API DDS_InstanceHandle_t *DDS_sequence_DDS_InstanceHandle_t_allocbuf (DDS_unsigned_long len);
#endif /* _DDS_sequence_DDS_InstanceHandle_t_defined */

typedef DDS_sequence_DDS_InstanceHandle_t DDS_InstanceHandleSeq;
OS_API DDS_InstanceHandleSeq *DDS_InstanceHandleSeq__alloc (void);
OS_API DDS_InstanceHandle_t *DDS_InstanceHandleSeq_allocbuf (DDS_unsigned_long len);

/*
 * typedef long ReturnCode_t;
 */
typedef DDS_long DDS_ReturnCode_t;

/*
 * typedef long QosPolicyId_t;
 */
typedef DDS_long DDS_QosPolicyId_t;

/*
 * typedef sequence<string> StringSeq;
 */
#ifndef _DDS_sequence_string_defined
#define _DDS_sequence_string_defined
typedef struct {
    DDS_unsigned_long _maximum;
    DDS_unsigned_long _length;
    DDS_string *_buffer;
    DDS_boolean _release;
} DDS_sequence_string;

OS_API DDS_sequence_string *DDS_sequence_string__alloc (void);
OS_API DDS_string *DDS_sequence_string_allocbuf (DDS_unsigned_long len);
#endif /* _DDS_sequence_string_defined */

typedef DDS_sequence_string DDS_StringSeq;
OS_API DDS_StringSeq *DDS_StringSeq__alloc (void);
OS_API DDS_string *DDS_StringSeq_allocbuf (DDS_unsigned_long len);

/*
 * struct Duration_t {
 *     long sec;
 *     unsigned long nanosec;
 * };
 */
typedef struct DDS_Duration_t_s DDS_Duration_t;
struct DDS_Duration_t_s
{
    DDS_long sec;
    DDS_unsigned_long nanosec;
};
OS_API DDS_Duration_t *DDS_Duration_t__alloc (void);

/*
 * struct Time_t {
 *     long sec;
 *     unsigned long nanosec;
 * };
 */
typedef struct DDS_Time_t_s DDS_Time_t;
struct DDS_Time_t_s
{
    DDS_long sec;
    DDS_unsigned_long nanosec;
};
OS_API DDS_Time_t *DDS_Time_t__alloc (void);

/*
 * // ----------------------------------------------------------------------
 * // Pre-defined values
 * // ----------------------------------------------------------------------
 * const InstanceHandle_t HANDLE_NIL                   = HANDLE_NIL_NATIVE;
 * const long LENGTH_UNLIMITED                         = -1;
 * const long DURATION_INFINITE_SEC                    = 0x7ffffff;
 * const unsigned long DURATION_INFINITE_NSEC          = 0x7ffffff;
 * const long DURATION_ZERO_SEC                        = 0;
 * const unsigned long DURATION_ZERO_NSEC              = 0;
 * const long TIMESTAMP_INVALID_SEC                    = -1;
 * const unsigned long TIMESTAMP_INVALID_NSEC          = 0xffffffff;
 */
#define DDS_HANDLE_NIL                              DDS_HANDLE_NIL_NATIVE
#define DDS_LENGTH_UNLIMITED                        -1
#define DDS_DURATION_INFINITE_SEC                   0x7fffffff
#define DDS_DURATION_INFINITE_NSEC                  0x7fffffffU
#define DDS_DURATION_INFINITE                       { DDS_DURATION_INFINITE_SEC, DDS_DURATION_INFINITE_NSEC }

#define DDS_DURATION_ZERO_SEC                       0
#define DDS_DURATION_ZERO_NSEC                      0U
#define DDS_DURATION_ZERO                           { DDS_DURATION_ZERO_SEC, DDS_DURATION_ZERO_NSEC }
#define DDS_TIMESTAMP_INVALID_SEC                   -1
#define DDS_TIMESTAMP_INVALID_NSEC                  4294967295U
#define DDS_TIMESTAMP_INVALID                       { DDS_TIMESTAMP_INVALID_SEC, DDS_TIMESTAMP_INVALID_NSEC }

/*
 * // ----------------------------------------------------------------------
 * // Return codes
 * // ----------------------------------------------------------------------
 * const ReturnCode_t RETCODE_OK                       = 0;
 * const ReturnCode_t RETCODE_ERROR                    = 1;
 * const ReturnCode_t RETCODE_UNSUPPORTED              = 2;
 * const ReturnCode_t RETCODE_BAD_PARAMETER            = 3;
 * const ReturnCode_t RETCODE_PRECONDITION_NOT_MET     = 4;
 * const ReturnCode_t RETCODE_OUT_OF_RESOURCES         = 5;
 * const ReturnCode_t RETCODE_NOT_ENABLED              = 6;
 * const ReturnCode_t RETCODE_IMMUTABLE_POLICY         = 7;
 * const ReturnCode_t RETCODE_INCONSISTENT_POLICY      = 8;
 * const ReturnCode_t RETCODE_ALREADY_DELETED          = 9;
 * const ReturnCode_t RETCODE_TIMEOUT                  = 10;
 * const ReturnCode_t RETCODE_NO_DATA                  = 11;
 * const ReturnCode_t RETCODE_ILLEGAL_OPERATION = 12;
 */
#define DDS_RETCODE_OK                                  0
#define DDS_RETCODE_ERROR                               1
#define DDS_RETCODE_UNSUPPORTED                         2
#define DDS_RETCODE_BAD_PARAMETER                       3
#define DDS_RETCODE_PRECONDITION_NOT_MET                4
#define DDS_RETCODE_OUT_OF_RESOURCES                    5
#define DDS_RETCODE_NOT_ENABLED                         6
#define DDS_RETCODE_IMMUTABLE_POLICY                    7
#define DDS_RETCODE_INCONSISTENT_POLICY                 8
#define DDS_RETCODE_ALREADY_DELETED                     9
#define DDS_RETCODE_TIMEOUT                             10
#define DDS_RETCODE_NO_DATA                             11
#define DDS_RETCODE_ILLEGAL_OPERATION                   12

/*
 * // ----------------------------------------------------------------------
 * // Status to support listeners and conditions
 * // ----------------------------------------------------------------------
 * typedef unsigned long StatusKind;
 * typedef unsigned long StatusMask; // bit-mask StatusKind
 *
 * const StatusKind INCONSISTENT_TOPIC_STATUS          = 0x0001 << 0;
 * const StatusKind OFFERED_DEADLINE_MISSED_STATUS     = 0x0001 << 1;
 * const StatusKind REQUESTED_DEADLINE_MISSED_STATUS   = 0x0001 << 2;
 * const StatusKind OFFERED_INCOMPATIBLE_QOS_STATUS    = 0x0001 << 5;
 * const StatusKind REQUESTED_INCOMPATIBLE_QOS_STATUS  = 0x0001 << 6;
 * const StatusKind SAMPLE_LOST_STATUS                 = 0x0001 << 7;
 * const StatusKind SAMPLE_REJECTED_STATUS             = 0x0001 << 8;
 * const StatusKind DATA_ON_READERS_STATUS             = 0x0001 << 9;
 * const StatusKind DATA_AVAILABLE_STATUS              = 0x0001 << 10;
 * const StatusKind LIVELINESS_LOST_STATUS             = 0x0001 << 11;
 * const StatusKind LIVELINESS_CHANGED_STATUS          = 0x0001 << 12;
 * const StatusKind PUBLICATION_MATCHED_STATUS           = 0x0001 << 13;
 * const StatusKind SUBSCRIPTION_MATCHED_STATUS          = 0x0001 << 14;
 * const StatusKind ALL_DATA_DISPOSED_TOPIC_STATUS       = 0x0001 << 31;
 */
typedef DDS_unsigned_long DDS_StatusKind;
typedef DDS_unsigned_long DDS_StatusMask;

#define DDS_INCONSISTENT_TOPIC_STATUS                       1U
#define DDS_OFFERED_DEADLINE_MISSED_STATUS                  2U
#define DDS_REQUESTED_DEADLINE_MISSED_STATUS                4U
#define DDS_OFFERED_INCOMPATIBLE_QOS_STATUS                 32U
#define DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS               64U
#define DDS_SAMPLE_LOST_STATUS                              128U
#define DDS_SAMPLE_REJECTED_STATUS                          256U
#define DDS_DATA_ON_READERS_STATUS                          512U
#define DDS_DATA_AVAILABLE_STATUS                           1024U
#define DDS_LIVELINESS_LOST_STATUS                          2048U
#define DDS_LIVELINESS_CHANGED_STATUS                       4096U
#define DDS_PUBLICATION_MATCHED_STATUS                      8192U
#define DDS_SUBSCRIPTION_MATCHED_STATUS                     16384U

/* Opensplice Extensions */
#define DDS_ALL_DATA_DISPOSED_TOPIC_STATUS                  0x80000000U

#define DDS_ANY_STATUS                                        0x7FE7
#define DDS_STATUS_MASK_ANY_V1_2                              0x7FE7
#define DDS_STATUS_MASK_NONE                                  0x0

/*
 * struct InconsistentTopicStatus {
 *     long total_count;
 *     long total_count_change;
 * };
 */
typedef struct DDS_InconsistentTopicStatus_s DDS_InconsistentTopicStatus;
struct DDS_InconsistentTopicStatus_s {
    DDS_long total_count;
    DDS_long total_count_change;
};

/*
 * struct SampleLostStatus {
 *     long total_count;
 *     long total_count_change;
 * };
 */
typedef struct DDS_SampleLostStatus_s DDS_SampleLostStatus;
struct DDS_SampleLostStatus_s {
    DDS_long total_count;
    DDS_long total_count_change;
};

/*
 * enum SampleRejectedStatusKind {
 *     NOT_REJECTED,
 *     REJECTED_BY_INSTANCES_LIMIT,
 *     REJECTED_BY_SAMPLES_LIMIT,
 *     REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT
 * };
 */
typedef enum
{
    DDS_NOT_REJECTED,
    DDS_REJECTED_BY_INSTANCES_LIMIT,
    DDS_REJECTED_BY_SAMPLES_LIMIT,
    DDS_REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT
} DDS_SampleRejectedStatusKind;

/*
 * struct SampleRejectedStatus {
 *     long total_count;
 *     long total_count_change;
 *     SampleRejectedStatusKind last_reason;
 *     InstanceHandle_t last_instance_handle;
 * };
 */
typedef struct DDS_SampleRejectedStatus_s DDS_SampleRejectedStatus;
struct DDS_SampleRejectedStatus_s {
    DDS_long total_count;
    DDS_long total_count_change;
    DDS_SampleRejectedStatusKind last_reason;
    DDS_InstanceHandle_t last_instance_handle;
};

/*
 * struct LivelinessLostStatus {
 *     long total_count;
 *     long total_count_change;
 * };
 */
typedef struct DDS_LivelinessLostStatus_s DDS_LivelinessLostStatus;
struct DDS_LivelinessLostStatus_s {
    DDS_long total_count;
    DDS_long total_count_change;
};


/*
 * struct LivelinessChangedStatus {
 *     long alive_count;
 *     long not_alive_count;
 *     long alive_count_change;
 *     long not_alive_count_change;
 *     InstanceHandle_t last_publication_handle;
 * };
 */
typedef struct DDS_LivelinessChangedStatus_s DDS_LivelinessChangedStatus;
struct DDS_LivelinessChangedStatus_s {
    DDS_long alive_count;
    DDS_long not_alive_count;
    DDS_long alive_count_change;
    DDS_long not_alive_count_change;
    DDS_InstanceHandle_t last_publication_handle;
};

/*
 * struct OfferedDeadlineMissedStatus {
 *     long total_count;
 *     long total_count_change;
 *     InstanceHandle_t last_instance_handle;
 * };
 */
typedef struct DDS_OfferedDeadlineMissedStatus_s DDS_OfferedDeadlineMissedStatus;
struct DDS_OfferedDeadlineMissedStatus_s {
    DDS_long total_count;
    DDS_long total_count_change;
    DDS_InstanceHandle_t last_instance_handle;
};

/*
 * struct RequestedDeadlineMissedStatus {
 *     long total_count;
 *     long total_count_change;
 *     InstanceHandle_t last_instance_handle;
 * };
 */
typedef struct DDS_RequestedDeadlineMissedStatus_s DDS_RequestedDeadlineMissedStatus;
struct DDS_RequestedDeadlineMissedStatus_s {
    DDS_long total_count;
    DDS_long total_count_change;
    DDS_InstanceHandle_t last_instance_handle;
};

/*
 * struct QosPolicyCount {
 *     QosPolicyId_t policy_id;
 *     long count;
 * };
 */
typedef struct DDS_QosPolicyCount_s DDS_QosPolicyCount;
struct DDS_QosPolicyCount_s {
    DDS_QosPolicyId_t policy_id;
    DDS_long count;
};

/*
 * typedef sequence<QosPolicyCount> QosPolicyCountSeq;
 */
#ifndef _DDS_sequence_DDS_QosPolicyCount_defined
#define _DDS_sequence_DDS_QosPolicyCount_defined
typedef struct {
    DDS_unsigned_long _maximum;
    DDS_unsigned_long _length;
    DDS_QosPolicyCount *_buffer;
    DDS_boolean _release;
} DDS_sequence_DDS_QosPolicyCount;

OS_API DDS_sequence_DDS_QosPolicyCount *DDS_sequence_DDS_QosPolicyCount__alloc (void);
OS_API DDS_QosPolicyCount *DDS_sequence_DDS_QosPolicyCount_allocbuf (DDS_unsigned_long len);
#endif /* _DDS_sequence_DDS_QosPolicyCount_defined */

typedef DDS_sequence_DDS_QosPolicyCount DDS_QosPolicyCountSeq;
OS_API DDS_QosPolicyCountSeq *DDS_QosPolicyCountSeq__alloc (void);
OS_API DDS_QosPolicyCount *DDS_QosPolicyCountSeq_allocbuf (DDS_unsigned_long len);

/*
 * struct OfferedIncompatibleQosStatus {
 *     long total_count;
 *     long total_count_change;
 *     QosPolicyId_t last_policy_id;
 *     QosPolicyCountSeq policies;
 * };
 */
typedef struct DDS_OfferedIncompatibleQosStatus_s DDS_OfferedIncompatibleQosStatus;
struct DDS_OfferedIncompatibleQosStatus_s {
    DDS_long total_count;
    DDS_long total_count_change;
    DDS_QosPolicyId_t last_policy_id;
    DDS_QosPolicyCountSeq policies;
};

/*
 * struct RequestedIncompatibleQosStatus {
 *     long total_count;
 *     long total_count_change;
 *     QosPolicyId_t last_policy_id;
 *     QosPolicyCountSeq policies;
 * };
 */
typedef struct DDS_RequestedIncompatibleQosStatus_s DDS_RequestedIncompatibleQosStatus;
struct DDS_RequestedIncompatibleQosStatus_s {
    DDS_long total_count;
    DDS_long total_count_change;
    DDS_QosPolicyId_t last_policy_id;
    DDS_QosPolicyCountSeq policies;
};

/*
 * struct PublicationMatchedStatus {
 *     long total_count;
 *     long total_count_change;
 *     long current_count;
 *     long current_count_change;
 *     InstanceHandle_t last_subscription_handle;
 * };
 */
typedef struct DDS_PublicationMatchedStatus_s DDS_PublicationMatchedStatus;
struct DDS_PublicationMatchedStatus_s {
    DDS_long total_count;
    DDS_long total_count_change;
    DDS_long current_count;
    DDS_long current_count_change;
    DDS_InstanceHandle_t last_subscription_handle;
};

/*
 * struct SubscriptionMatchedStatus {
 *     long total_count;
 *     long total_count_change;
 *     long current_count;
 *     long current_count_change;
 *     InstanceHandle_t last_publication_handle;
 * };
 */
typedef struct DDS_SubscriptionMatchedStatus_s DDS_SubscriptionMatchedStatus;
struct DDS_SubscriptionMatchedStatus_s {
    DDS_long total_count;
    DDS_long total_count_change;
    DDS_long current_count;
    DDS_long current_count_change;
    DDS_InstanceHandle_t last_publication_handle;
};

/*
 * // ----------------------------------------------------------------------
 * // Listeners
 * // ----------------------------------------------------------------------
 * interface Listener;
 * interface Entity;
 * interface TopicDescription;
 * interface Topic;
 * interface ContentFilteredTopic;
 * interface MultiTopic;
 * interface DataWriter;
 * interface DataReader;
 * interface Subscriber;
 * interface Publisher;
 */
typedef struct DDS_Listener DDS_Listener;
typedef DDS_Object DDS_Entity;
typedef DDS_Object DDS_TopicDescription;
typedef DDS_Object DDS_Topic;
typedef DDS_Object DDS_ContentFilteredTopic;
typedef DDS_Object DDS_MultiTopic;
typedef DDS_Object DDS_DataWriter;
typedef DDS_Object DDS_DataReader;
typedef DDS_Object DDS_DataReaderView;
typedef DDS_Object DDS_Subscriber;
typedef DDS_Object DDS_Publisher;

/*
 * typedef sequence<Topic> TopicSeq;
 */
#ifndef _DDS_sequence_DDS_Topic_defined
#define _DDS_sequence_DDS_Topic_defined
typedef struct {
    DDS_unsigned_long _maximum;
    DDS_unsigned_long _length;
    DDS_Topic *_buffer;
    DDS_boolean _release;
} DDS_sequence_DDS_Topic;

OS_API DDS_sequence_DDS_Topic *DDS_sequence_DDS_Topic__alloc (void);
OS_API DDS_Topic *DDS_sequence_DDS_Topic_allocbuf (DDS_unsigned_long len);
#endif /* _DDS_sequence_DDS_Topic_defined */

typedef DDS_sequence_DDS_Topic DDS_TopicSeq;
OS_API DDS_TopicSeq *DDS_TopicSeq__alloc (void);
OS_API DDS_Topic *DDS_TopicSeq_allocbuf (DDS_unsigned_long len);

/*
 * typedef sequence<DataReader> DataReaderSeq;
 */
#ifndef _DDS_sequence_DDS_DataReader_defined
#define _DDS_sequence_DDS_DataReader_defined
typedef struct {
    DDS_unsigned_long _maximum;
    DDS_unsigned_long _length;
    DDS_DataReader *_buffer;
    DDS_boolean _release;
} DDS_sequence_DDS_DataReader;
OS_API DDS_sequence_DDS_DataReader *DDS_sequence_DDS_DataReader__alloc (void);
OS_API DDS_DataReader *DDS_sequence_DDS_DataReader_allocbuf (DDS_unsigned_long len);
#endif /* _DDS_sequence_DDS_DataReader_defined */

typedef DDS_sequence_DDS_DataReader DDS_DataReaderSeq;
OS_API DDS_DataReaderSeq *DDS_DataReaderSeq__alloc (void);
OS_API DDS_DataReader *DDS_DataReaderSeq_allocbuf (DDS_unsigned_long len);

/*
 * interface Listener { };
 */
struct DDS_Listener {
    void *listener_data;
};

/*
 * interface TopicListener : Listener
 */
typedef void (*DDS_TopicListener_InconsistentTopicListener)
    (void *listener_data,
     DDS_Topic topic,
     const DDS_InconsistentTopicStatus *status);

struct DDS_TopicListener {
    void *listener_data;
    DDS_TopicListener_InconsistentTopicListener on_inconsistent_topic;
};
OS_API struct DDS_TopicListener *DDS_TopicListener__alloc (void);

/*
 * interface ExtTopicListener : TopicListener
 */

typedef void (*DDS_ExtTopicListener_AllDataDisposedListener)
    (void *listener_data,
     DDS_Topic topic);

struct DDS_ExtTopicListener {
    void *listener_data;
    DDS_TopicListener_InconsistentTopicListener on_inconsistent_topic;
    DDS_ExtTopicListener_AllDataDisposedListener on_all_data_disposed;
};
OS_API struct DDS_ExtTopicListener *DDS_ExtTopicListener__alloc (void);

/*
 * interface DataWriterListener : Listener {
 */
typedef void (*DDS_DataWriterListener_OfferedDeadlineMissedListener)
    (void *listener_data,
     DDS_DataWriter writer,
     const DDS_OfferedDeadlineMissedStatus *status);

typedef void (*DDS_DataWriterListener_LivelinessLostListener)
    (void *listener_data,
     DDS_DataWriter writer,
     const DDS_LivelinessLostStatus *status);

typedef void (*DDS_DataWriterListener_OfferedIncompatibleQosListener)
    (void *listener_data,
     DDS_DataWriter writer,
     const DDS_OfferedIncompatibleQosStatus *status);

typedef void (*DDS_DataWriterListener_PublicationMatchedListener)
    (void *listener_data,
     DDS_DataWriter writer,
     const DDS_PublicationMatchedStatus *status);

struct DDS_DataWriterListener {
    void *listener_data;
    DDS_DataWriterListener_OfferedDeadlineMissedListener on_offered_deadline_missed;
    DDS_DataWriterListener_OfferedIncompatibleQosListener on_offered_incompatible_qos;
    DDS_DataWriterListener_LivelinessLostListener on_liveliness_lost;
    DDS_DataWriterListener_PublicationMatchedListener on_publication_matched;
};

OS_API struct DDS_DataWriterListener *DDS_DataWriterListener__alloc (void);

typedef void (*DDS_PublisherListener_OfferedDeadlineMissedListener)
    (void *listener_data,
     DDS_DataWriter writer,
     const DDS_OfferedDeadlineMissedStatus *status);

typedef void (*DDS_PublisherListener_LivelinessLostListener)
    (void *listener_data,
     DDS_DataWriter writer,
     const DDS_LivelinessLostStatus *status);

typedef void (*DDS_PublisherListener_OfferedIncompatibleQosListener)
    (void *listener_data,
     DDS_DataWriter writer,
     const DDS_OfferedIncompatibleQosStatus *status);

typedef void (*DDS_PublisherListener_PublicationMatchedListener)
    (void *listener_data,
     DDS_DataWriter writer,
     const DDS_PublicationMatchedStatus *status);

struct DDS_PublisherListener {
    void *listener_data;
    DDS_PublisherListener_OfferedDeadlineMissedListener on_offered_deadline_missed;
    DDS_PublisherListener_OfferedIncompatibleQosListener on_offered_incompatible_qos;
    DDS_PublisherListener_LivelinessLostListener on_liveliness_lost;
    DDS_PublisherListener_PublicationMatchedListener on_publication_matched;
};
OS_API struct DDS_PublisherListener *DDS_PublisherListener__alloc (void);

/*
 * interface DataReaderListener : Listener
 */
typedef void (*DDS_DataReaderListener_RequestedDeadlineMissedListener)
    (void *listener_data,
     DDS_DataReader reader,
     const DDS_RequestedDeadlineMissedStatus *status);

typedef void (*DDS_DataReaderListener_LivelinessChangedListener)
    (void *listener_data,
     DDS_DataReader reader,
     const DDS_LivelinessChangedStatus *status);

typedef void (*DDS_DataReaderListener_RequestedIncompatibleQosListener)
    (void *listener_data,
     DDS_DataReader reader,
     const DDS_RequestedIncompatibleQosStatus *status);

typedef void (*DDS_DataReaderListener_SampleRejectedListener)
    (void *listener_data,
     DDS_DataReader reader,
     const DDS_SampleRejectedStatus *status);

typedef void (*DDS_DataReaderListener_DataAvailableListener)
    (void *listener_data,
     DDS_DataReader reader);

typedef void (*DDS_DataReaderListener_SubscriptionMatchedListener)
    (void *listener_data,
     DDS_DataReader reader,
     const DDS_SubscriptionMatchedStatus *status);


typedef void (*DDS_DataReaderListener_SampleLostListener)
    (void *listener_data,
     DDS_DataReader reader,
     const DDS_SampleLostStatus *status);

struct DDS_DataReaderListener {
    void *listener_data;
    DDS_DataReaderListener_RequestedDeadlineMissedListener on_requested_deadline_missed;
    DDS_DataReaderListener_RequestedIncompatibleQosListener on_requested_incompatible_qos;
    DDS_DataReaderListener_SampleRejectedListener on_sample_rejected;
    DDS_DataReaderListener_LivelinessChangedListener on_liveliness_changed;
    DDS_DataReaderListener_DataAvailableListener on_data_available;
    DDS_DataReaderListener_SubscriptionMatchedListener on_subscription_matched;
    DDS_DataReaderListener_SampleLostListener on_sample_lost;
};
OS_API struct DDS_DataReaderListener *DDS_DataReaderListener__alloc (void);

/*
 * interface SubscriberListener : DataReaderListener {
 */
typedef void (*DDS_SubscriberListener_RequestedDeadlineMissedListener)
    (void *listener_data,
     DDS_DataReader reader,
     const DDS_RequestedDeadlineMissedStatus *status);

typedef void (*DDS_SubscriberListener_LivelinessChangedListener)
    (void *listener_data,
     DDS_DataReader reader,
     const DDS_LivelinessChangedStatus *status);

typedef void (*DDS_SubscriberListener_RequestedIncompatibleQosListener)
    (void *listener_data,
     DDS_DataReader reader,
     const DDS_RequestedIncompatibleQosStatus *status);

typedef void (*DDS_SubscriberListener_SampleRejectedListener)
    (void *listener_data,
     DDS_DataReader reader,
     const DDS_SampleRejectedStatus *status);

typedef void (*DDS_SubscriberListener_DataAvailableListener)
    (void *listener_data,
     DDS_DataReader reader);

typedef void (*DDS_SubscriberListener_SubscriptionMatchedListener)
    (void *listener_data,
     DDS_DataReader reader,
     const DDS_SubscriptionMatchedStatus *status);

typedef void (*DDS_SubscriberListener_SampleLostListener)
    (void *listener_data,
     DDS_DataReader reader,
     const DDS_SampleLostStatus *status);

typedef void (*DDS_SubscriberListener_DataOnReadersListener)
    (void *listener_data,
     DDS_Subscriber subs);

struct DDS_SubscriberListener {
    void *listener_data;
    DDS_SubscriberListener_RequestedDeadlineMissedListener on_requested_deadline_missed;
    DDS_SubscriberListener_RequestedIncompatibleQosListener on_requested_incompatible_qos;
    DDS_SubscriberListener_SampleRejectedListener on_sample_rejected;
    DDS_SubscriberListener_LivelinessChangedListener on_liveliness_changed;
    DDS_SubscriberListener_DataAvailableListener on_data_available;
    DDS_SubscriberListener_SubscriptionMatchedListener on_subscription_matched;
    DDS_SubscriberListener_SampleLostListener on_sample_lost;
    DDS_SubscriberListener_DataOnReadersListener on_data_on_readers;
};
OS_API struct DDS_SubscriberListener *DDS_SubscriberListener__alloc (void);

/*
 * interface DomainParticipantListener : TopicListener, PublisherListener, SubscriberListener
 */

typedef void (*DDS_DomainParticipantListener_InconsistentTopicListener)
    (void *listener_data,
     DDS_Topic topic,
     const DDS_InconsistentTopicStatus *status);

typedef void (*DDS_DomainParticipantListener_OfferedDeadlineMissedListener)
    (void *listener_data,
     DDS_DataWriter writer,
     const DDS_OfferedDeadlineMissedStatus *status);

typedef void (*DDS_DomainParticipantListener_LivelinessLostListener)
    (void *listener_data,
     DDS_DataWriter writer,
     const DDS_LivelinessLostStatus *status);

typedef void (*DDS_DomainParticipantListener_OfferedIncompatibleQosListener)
    (void *listener_data,
     DDS_DataWriter writer,
     const DDS_OfferedIncompatibleQosStatus *status);

typedef void (*DDS_DomainParticipantListener_PublicationMatchedListener)
    (void *listener_data,
     DDS_DataWriter writer,
     const DDS_PublicationMatchedStatus *status);


/*
 * interface DomainParticipantListener : TopicListener, PublisherListener, SubscriberListener
 */

typedef void (*DDS_DomainParticipantListener_RequestedDeadlineMissedListener)
    (void *listener_data,
     DDS_DataReader reader,
     const DDS_RequestedDeadlineMissedStatus *status);

typedef void (*DDS_DomainParticipantListener_LivelinessChangedListener)
    (void *listener_data,
     DDS_DataReader reader,
     const DDS_LivelinessChangedStatus *status);

typedef void (*DDS_DomainParticipantListener_RequestedIncompatibleQosListener)
    (void *listener_data,
     DDS_DataReader reader,
     const DDS_RequestedIncompatibleQosStatus *status);

typedef void (*DDS_DomainParticipantListener_SampleRejectedListener)
    (void *listener_data,
     DDS_DataReader reader,
     const DDS_SampleRejectedStatus *status);

typedef void (*DDS_DomainParticipantListener_DataAvailableListener)
    (void *listener_data,
     DDS_DataReader reader);

typedef void (*DDS_DomainParticipantListener_SubscriptionMatchedListener)
    (void *listener_data,
     DDS_DataReader reader,
     const DDS_SubscriptionMatchedStatus *status);

typedef void (*DDS_DomainParticipantListener_SampleLostListener)
    (void *listener_data,
     DDS_DataReader reader,
     const DDS_SampleLostStatus *status);

typedef void (*DDS_DomainParticipantListener_DataOnReadersListener)
    (void *listener_data,
     DDS_Subscriber subs);

struct DDS_DomainParticipantListener {
    void *listener_data;
    DDS_DomainParticipantListener_InconsistentTopicListener on_inconsistent_topic;
    DDS_DomainParticipantListener_OfferedDeadlineMissedListener on_offered_deadline_missed;
    DDS_DomainParticipantListener_OfferedIncompatibleQosListener on_offered_incompatible_qos;
    DDS_DomainParticipantListener_LivelinessLostListener on_liveliness_lost;
    DDS_DomainParticipantListener_PublicationMatchedListener on_publication_matched;
    DDS_DomainParticipantListener_RequestedDeadlineMissedListener on_requested_deadline_missed;
    DDS_DomainParticipantListener_RequestedIncompatibleQosListener on_requested_incompatible_qos;
    DDS_DomainParticipantListener_SampleRejectedListener on_sample_rejected;
    DDS_DomainParticipantListener_LivelinessChangedListener on_liveliness_changed;
    DDS_DomainParticipantListener_DataAvailableListener on_data_available;
    DDS_DomainParticipantListener_SubscriptionMatchedListener on_subscription_matched;
    DDS_DomainParticipantListener_SampleLostListener on_sample_lost;
    DDS_DomainParticipantListener_DataOnReadersListener on_data_on_readers;
};
OS_API struct DDS_DomainParticipantListener *DDS_DomainParticipantListener__alloc (void);

/*
 * interface Condition
 */
typedef DDS_Object DDS_Condition;

/*
 *     boolean
 *     get_trigger_value();
 */
OS_API DDS_boolean
DDS_Condition_get_trigger_value (
    DDS_Condition _this);

/*
 * typedef sequence<Condition> ConditionSeq;
 */
#ifndef _DDS_sequence_DDS_Condition_defined
#define _DDS_sequence_DDS_Condition_defined
typedef struct {
    DDS_unsigned_long _maximum;
    DDS_unsigned_long _length;
    DDS_Condition *_buffer;
    DDS_boolean _release;
} DDS_sequence_DDS_Condition;

OS_API DDS_sequence_DDS_Condition *DDS_sequence_DDS_Condition__alloc (void);
OS_API DDS_Condition *DDS_sequence_DDS_Condition_allocbuf (DDS_unsigned_long len);
#endif /* _DDS_sequence_DDS_Condition_defined */

typedef DDS_sequence_DDS_Condition DDS_ConditionSeq;
OS_API DDS_ConditionSeq *DDS_ConditionSeq__alloc (void);
OS_API DDS_Condition *DDS_ConditionSeq_allocbuf (DDS_unsigned_long len);

/*
 * interface WaitSet
 */
typedef DDS_Object DDS_WaitSet;

/*     ReturnCode_t
 *     wait(
 *         inout ConditionSeq active_conditions,
 *         in Duration_t timeout);
 */
OS_API DDS_ReturnCode_t
DDS_WaitSet_wait (
    DDS_WaitSet _this,
    DDS_ConditionSeq *active_conditions,
    const DDS_Duration_t *timeout);



/*     ReturnCode_t
 *     attach_condition(
 *         in Condition cond);
 */
OS_API DDS_ReturnCode_t
DDS_WaitSet_attach_condition (
    DDS_WaitSet _this,
    const DDS_Condition cond);

/*     ReturnCode_t
 *     detach_condition(
 *         in Condition cond);
 */
OS_API DDS_ReturnCode_t
DDS_WaitSet_detach_condition(
    DDS_WaitSet _this,
    const DDS_Condition cond);

/*     ReturnCode_t
 *     get_conditions(
 *         out ConditionSeq attached_conditions);
 */
OS_API DDS_ReturnCode_t
DDS_WaitSet_get_conditions(
    DDS_WaitSet _this,
    DDS_ConditionSeq *attached_conditions);


/*     WaitSet
 *     WaitSet__alloc (
 *         void);
 */
OS_API DDS_WaitSet DDS_WaitSet__alloc (void);

/*
 * interface GuardCondition : Condition
 */
typedef DDS_Object DDS_GuardCondition;

/* From Condition
 *     get_trigger_value
 */
#define DDS_GuardCondition_get_trigger_value DDS_Condition_get_trigger_value

/*     void
 *     set_trigger_value(
 *         in boolean value);
 * };
 */
OS_API DDS_ReturnCode_t
DDS_GuardCondition_set_trigger_value (
    DDS_GuardCondition _this,
    const DDS_boolean value);

/*     GuardCondition
 *     GuardCondition__alloc (
 *         void);
 */
OS_API DDS_GuardCondition DDS_GuardCondition__alloc (void);

/*
 * interface StatusCondition : Condition
 */
typedef DDS_Object DDS_StatusCondition;

/* From Condition
 *     get_trigger_value
 */
#define DDS_StatusCondition_get_trigger_value DDS_Condition_get_trigger_value

/*     StatusMask
 *     get_enabled_statuses();
 */
OS_API DDS_StatusMask
DDS_StatusCondition_get_enabled_statuses (
    DDS_StatusCondition _this);

/*     ReturnCode_t
 *     set_enabled_statuses(
 *         in StatusMask mask);
 */
OS_API DDS_ReturnCode_t
DDS_StatusCondition_set_enabled_statuses (
    DDS_StatusCondition _this,
    const DDS_StatusMask mask);

/*     Entity
 *     get_entity();
 */
OS_API DDS_Entity
DDS_StatusCondition_get_entity (
    DDS_StatusCondition _this);

/*
 * // Sample states to support reads
 * typedef unsigned long SampleStateKind;
 * typedef sequence <SampleStateKind> SampleStateSeq;
 */
typedef DDS_unsigned_long DDS_SampleStateKind;
#ifndef _DDS_sequence_DDS_SampleStateKind_defined
#define _DDS_sequence_DDS_SampleStateKind_defined
typedef struct {
    DDS_unsigned_long _maximum;
    DDS_unsigned_long _length;
    DDS_SampleStateKind *_buffer;
    DDS_boolean _release;
} DDS_sequence_DDS_SampleStateKind;

OS_API DDS_sequence_DDS_SampleStateKind *DDS_sequence_DDS_SampleStateKind__alloc (void);
OS_API DDS_SampleStateKind *DDS_sequence_DDS_SampleStateKind_allocbuf (DDS_unsigned_long len);
#endif /* _DDS_sequence_DDS_SampleStateKind_defined */

typedef DDS_sequence_DDS_SampleStateKind DDS_SampleStateSeq;
OS_API DDS_SampleStateSeq *DDS_SampleStateSeq__alloc (void);
OS_API DDS_SampleStateKind *DDS_SampleStateSeq_allocbuf (DDS_unsigned_long len);

/*
 * const SampleStateKind READ_SAMPLE_STATE                     = 0x0001 << 0;
 * const SampleStateKind NOT_READ_SAMPLE_STATE                 = 0x0001 << 1;
 */
#define DDS_READ_SAMPLE_STATE                                   1U
#define DDS_NOT_READ_SAMPLE_STATE                               2U

/* // This is a bit-mask SampleStateKind
 * typedef unsigned long SampleStateMask;
 */
typedef DDS_unsigned_long DDS_SampleStateMask;

/* const SampleStateMask ANY_SAMPLE_STATE                      = 0xffff;
 */
#define DDS_ANY_SAMPLE_STATE                                    65535U

/* // View states to support reads
 * typedef unsigned long ViewStateKind;
 * typedef sequence<ViewStateKind> ViewStateSeq;
 */
typedef DDS_unsigned_long DDS_ViewStateKind;
#ifndef _DDS_sequence_DDS_ViewStateKind_defined
#define _DDS_sequence_DDS_ViewStateKind_defined
typedef struct {
    DDS_unsigned_long _maximum;
    DDS_unsigned_long _length;
    DDS_ViewStateKind *_buffer;
    DDS_boolean _release;
} DDS_sequence_DDS_ViewStateKind;

OS_API DDS_sequence_DDS_ViewStateKind *DDS_sequence_DDS_ViewStateKind__alloc (void);
OS_API DDS_ViewStateKind *DDS_sequence_DDS_ViewStateKind_allocbuf (DDS_unsigned_long len);
#endif /* _DDS_sequence_DDS_ViewStateKind_defined */

typedef DDS_sequence_DDS_ViewStateKind DDS_ViewStateSeq;
OS_API DDS_ViewStateSeq *DDS_ViewStateSeq__alloc (void);
OS_API DDS_ViewStateKind *DDS_ViewStateSeq_allocbuf (DDS_unsigned_long len);

/* const ViewStateKind NEW_VIEW_STATE                          = 0x0001 << 0;
 * const ViewStateKind NOT_NEW_VIEW_STATE                      = 0x0001 << 1;
 */
#define DDS_NEW_VIEW_STATE                                      1U
#define DDS_NOT_NEW_VIEW_STATE                                  2U

/* // This is a bit-mask ViewStateKind
 * typedef unsigned long ViewStateMask;
 */
typedef DDS_unsigned_long DDS_ViewStateMask;

/* const ViewStateMask ANY_VIEW_STATE                          = 0xffff;
 */
#define DDS_ANY_VIEW_STATE                                      65535U

/* // Instance states to support reads
 * typedef unsigned long InstanceStateKind;
 * typedef sequence<InstanceStateKind> InstanceStateSeq;
 */
typedef DDS_unsigned_long DDS_InstanceStateKind;
#ifndef _DDS_sequence_DDS_InstanceStateKind_defined
#define _DDS_sequence_DDS_InstanceStateKind_defined
typedef struct {
    DDS_unsigned_long _maximum;
    DDS_unsigned_long _length;
    DDS_InstanceStateKind *_buffer;
    DDS_boolean _release;
} DDS_sequence_DDS_InstanceStateKind;

OS_API DDS_sequence_DDS_InstanceStateKind *DDS_sequence_DDS_InstanceStateKind__alloc (void);
OS_API DDS_InstanceStateKind *DDS_sequence_DDS_InstanceStateKind_allocbuf (DDS_unsigned_long len);
#endif /* _DDS_sequence_DDS_InstanceStateKind_defined */

typedef DDS_sequence_DDS_InstanceStateKind DDS_InstanceStateSeq;
OS_API DDS_InstanceStateSeq *DDS_InstanceStateSeq__alloc (void);
OS_API DDS_InstanceStateKind *DDS_InstanceStateSeq_allocbuf (DDS_unsigned_long len);

/* const InstanceStateKind ALIVE_INSTANCE_STATE                = 0x0001 << 0;
 * const InstanceStateKind NOT_ALIVE_DISPOSED_INSTANCE_STATE   = 0x0001 << 1;
 * const InstanceStateKind NOT_ALIVE_NO_WRITERS_INSTANCE_STATE = 0x0001 << 2;
 */
#define DDS_ALIVE_INSTANCE_STATE                                1U
#define DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE                   2U
#define DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE                 4U

/* // This is a bit-mask InstanceStateKind
 * typedef unsigned long InstanceStateMask;
 */
typedef DDS_unsigned_long DDS_InstanceStateMask;

/* const InstanceStateMask ANY_INSTANCE_STATE                  = 0xffff;
 * const InstanceStateMask NOT_ALIVE_INSTANCE_STATE            = 0x006;
 */
#define DDS_ANY_INSTANCE_STATE                                  65535U
#define DDS_NOT_ALIVE_INSTANCE_STATE                            6U

/*
 * interface ReadCondition : Condition
 */
typedef DDS_Object DDS_ReadCondition;

/* From Condition
 *     get_trigger_value
 */
#define DDS_ReadCondition_get_trigger_value DDS_Condition_get_trigger_value

/*     SampleStateMask
 *     get_sample_state_mask();
 */
OS_API DDS_SampleStateMask
DDS_ReadCondition_get_sample_state_mask (
    DDS_ReadCondition _this);
/*     ViewStateMask
 *     get_view_state_mask();
 */
OS_API DDS_ViewStateMask
DDS_ReadCondition_get_view_state_mask (
    DDS_ReadCondition _this);

/*     InstanceStateMask
 *     get_instance_state_mask();
 */
OS_API DDS_InstanceStateMask
DDS_ReadCondition_get_instance_state_mask (
    DDS_ReadCondition _this);

/*     DataReader
 *     get_datareader();
 */
OS_API DDS_DataReader
DDS_ReadCondition_get_datareader(
    DDS_ReadCondition _this);

/*     DataReaderView
 *     get_datareaderview();
 */
OS_API DDS_DataReaderView
DDS_ReadCondition_get_datareaderview(
    DDS_ReadCondition _this);

/*
 * interface QueryCondition : ReadCondition
 */
typedef DDS_Object DDS_QueryCondition;

/* From Condition
 *     get_trigger_value
 */
#define DDS_QueryCondition_get_trigger_value DDS_Condition_get_trigger_value

/* From ReadCondition
 *     get_sample_state_mask
 */
#define DDS_QueryCondition_get_sample_state_mask DDS_ReadCondition_get_sample_state_mask

/* From ReadCondition
 *     get_view_state_mask
 */
#define DDS_QueryCondition_get_view_state_mask DDS_ReadCondition_get_view_state_mask

/* From ReadCondition
 *     get_instance_state_mask
 */
#define DDS_QueryCondition_get_instance_state_mask DDS_ReadCondition_get_instance_state_mask

/* From ReadCondition
 *     get_datareader
 */
#define DDS_QueryCondition_get_datareader DDS_ReadCondition_get_datareader

/* From ReadCondition
 *     get_datareaderview
 */
#define DDS_QueryCondition_get_datareaderview DDS_ReadCondition_get_datareaderview

/*     string
 *     get_query_expression();
 */
OS_API DDS_string
DDS_QueryCondition_get_query_expression (
    DDS_QueryCondition _this);

/*     ReturnCode_t
 *       get_query_parameters(
 *             inout StringSeq query_parameters);
 */
OS_API DDS_ReturnCode_t
DDS_QueryCondition_get_query_parameters (
    DDS_QueryCondition _this,
    DDS_StringSeq *query_parameters);

/*     ReturnCode_t
 *     set_query_parameters(
 *         in StringSeq query_parameters);
 */
OS_API DDS_ReturnCode_t
DDS_QueryCondition_set_query_parameters (
    DDS_QueryCondition _this,
    const DDS_StringSeq *query_parameters);

/*
 * // ----------------------------------------------------------------------
 * // Qos
 * // ----------------------------------------------------------------------
 */

/* const string USERDATA_QOS_POLICY_NAME               = "UserData";
 * const string DURABILITY_QOS_POLICY_NAME             = "Durability";
 * const string PRESENTATION_QOS_POLICY_NAME           = "Presentation";
 * const string DEADLINE_QOS_POLICY_NAME               = "Deadline";
 * const string LATENCYBUDGET_QOS_POLICY_NAME          = "LatencyBudget";
 * const string OWNERSHIP_QOS_POLICY_NAME              = "Ownership";
 * const string OWNERSHIPSTRENGTH_QOS_POLICY_NAME      = "OwnershipStrength";
 * const string LIVELINESS_QOS_POLICY_NAME             = "Liveliness";
 * const string TIMEBASEDFILTER_QOS_POLICY_NAME        = "TimeBasedFilter";
 * const string PARTITION_QOS_POLICY_NAME              = "Partition";
 * const string RELIABILITY_QOS_POLICY_NAME            = "Reliability";
 * const string DESTINATIONORDER_QOS_POLICY_NAME       = "DestinationOrder";
 * const string HISTORY_QOS_POLICY_NAME                = "History";
 * const string RESOURCELIMITS_QOS_POLICY_NAME         = "ResourceLimits";
 * const string ENTITYFACTORY_QOS_POLICY_NAME          = "EntityFactory";
 * const string WRITERDATALIFECYCLE_QOS_POLICY_NAME    = "WriterDataLifecycle";
 * const string READERDATALIFECYCLE_QOS_POLICY_NAME    = "ReaderDataLifecycle";
 * const string TOPICDATA_QOS_POLICY_NAME              = "TopicData";
 * const string GROUPDATA_QOS_POLICY_NAME              = "GroupData";
 * const string TRANSPORTPRIORITY_QOS_POLICY_NAME      = "TransportPriority";
 * const string DURABILITYSERVICE_QOS_POLICY_NAME      = "DurabilityService";
 * const string LIFESPAN_QOS_POLICY_NAME               = "Lifespan";
 * const string SUBSCRIPTIONKEY_QOS_POLICY_NAME        = "SubscriptionKey"
 * const string VIEWKEY_QOS_POLICY_NAME                = "ViewKey"
 * const string READERLIFESPAN_QOS_POLICY_NAME         = "ReaderLifespan"
 * const string SHARE_QOS_POLICY_NAME                  = "Share"
 * const string SCHEDULING_QOS_POLICY_NAME             = "Scheduling"
 */
#define DDS_USERDATA_QOS_POLICY_NAME                            "UserData"
#define DDS_DURABILITY_QOS_POLICY_NAME                          "Durability"
#define DDS_PRESENTATION_QOS_POLICY_NAME                        "Presentation"
#define DDS_DEADLINE_QOS_POLICY_NAME                            "Deadline"
#define DDS_LATENCYBUDGET_QOS_POLICY_NAME                       "LatencyBudget"
#define DDS_OWNERSHIP_QOS_POLICY_NAME                           "Ownership"
#define DDS_OWNERSHIPSTRENGTH_QOS_POLICY_NAME                   "OwnershipStrength"
#define DDS_LIVELINESS_QOS_POLICY_NAME                          "Liveliness"
#define DDS_TIMEBASEDFILTER_QOS_POLICY_NAME                     "TimeBasedFilter"
#define DDS_PARTITION_QOS_POLICY_NAME                           "Partition"
#define DDS_RELIABILITY_QOS_POLICY_NAME                         "Reliability"
#define DDS_DESTINATIONORDER_QOS_POLICY_NAME                    "DestinationOrder"
#define DDS_HISTORY_QOS_POLICY_NAME                             "History"
#define DDS_RESOURCELIMITS_QOS_POLICY_NAME                      "ResourceLimits"
#define DDS_ENTITYFACTORY_QOS_POLICY_NAME                       "EntityFactory"
#define DDS_WRITERDATALIFECYCLE_QOS_POLICY_NAME                 "WriterDataLifecycle"
#define DDS_READERDATALIFECYCLE_QOS_POLICY_NAME                 "ReaderDataLifecycle"
#define DDS_TOPICDATA_QOS_POLICY_NAME                           "TopicData"
#define DDS_GROUPDATA_QOS_POLICY_NAME                           "GroupData"
#define DDS_TRANSPORTPRIORITY_QOS_POLICY_NAME                   "TransportPriority"
#define DDS_LIFESPAN_QOS_POLICY_NAME                            "Lifespan"
#define DDS_DURABILITYSERVICE_QOS_POLICY_NAME                   "DurabilityService"
#define DDS_SUBSCRIPTIONKEY_QOS_POLICY_NAME                     "SubscriptionKey"
#define DDS_VIEWKEY_QOS_POLICY_NAME                             "ViewKey"
#define DDS_READERLIFESPAN_QOS_POLICY_NAME                      "ReaderLifespan"
#define DDS_SHARE_QOS_POLICY_NAME                               "Share"
#define DDS_SCHEDULING_QOS_POLICY_NAME                          "Scheduling"

/* const QosPolicyId_t INVALID_QOS_POLICY_ID                     = 0;
 * const QosPolicyId_t USERDATA_QOS_POLICY_ID                  = 1;
 * const QosPolicyId_t DURABILITY_QOS_POLICY_ID                = 2;
 * const QosPolicyId_t PRESENTATION_QOS_POLICY_ID              = 3;
 * const QosPolicyId_t DEADLINE_QOS_POLICY_ID                  = 4;
 * const QosPolicyId_t LATENCYBUDGET_QOS_POLICY_ID             = 5;
 * const QosPolicyId_t OWNERSHIP_QOS_POLICY_ID                 = 6;
 * const QosPolicyId_t OWNERSHIPSTRENGTH_QOS_POLICY_ID         = 7;
 * const QosPolicyId_t LIVELINESS_QOS_POLICY_ID                = 8;
 * const QosPolicyId_t TIMEBASEDFILTER_QOS_POLICY_ID           = 9;
 * const QosPolicyId_t PARTITION_QOS_POLICY_ID                 = 10;
 * const QosPolicyId_t RELIABILITY_QOS_POLICY_ID               = 11;
 * const QosPolicyId_t DESTINATIONORDER_QOS_POLICY_ID          = 12;
 * const QosPolicyId_t HISTORY_QOS_POLICY_ID                   = 13;
 * const QosPolicyId_t RESOURCELIMITS_QOS_POLICY_ID            = 14;
 * const QosPolicyId_t ENTITYFACTORY_QOS_POLICY_ID             = 15;
 * const QosPolicyId_t WRITERDATALIFECYCLE_QOS_POLICY_ID       = 16;
 * const QosPolicyId_t READERDATALIFECYCLE_QOS_POLICY_ID       = 17;
 * const QosPolicyId_t TOPICDATA_QOS_POLICY_ID                 = 18;
 * const QosPolicyId_t GROUPDATA_QOS_POLICY_ID                 = 19;
 * const QosPolicyId_t TRANSPORTPRIORITY_QOS_POLICY_ID         = 20;
 * const QosPolicyId_t LIFESPAN_QOS_POLICY_ID                  = 21;
 * const QosPolicyId_t DURABILITYSERVICE_QOS_POLICY_ID         = 22;
 * const QosPolicyId_t SUBSCRIPTIONKEY_QOS_POLICY_ID           = 23;
 * const QosPolicyId_t VIEWKEY_QOS_POLICY_ID                   = 24;
 * const QosPolicyId_t READERLIFESPAN_QOS_POLICY_ID            = 25;
 * const QosPolicyId_t SHARE_QOS_POLICY_ID                     = 26;
 * const QosPolicyId_t SCHEDULING_QOS_POLICY_ID                = 27;
 */
#define DDS_INVALID_QOS_POLICY_ID                              0
#define DDS_USERDATA_QOS_POLICY_ID                              1
#define DDS_DURABILITY_QOS_POLICY_ID                            2
#define DDS_PRESENTATION_QOS_POLICY_ID                          3
#define DDS_DEADLINE_QOS_POLICY_ID                              4
#define DDS_LATENCYBUDGET_QOS_POLICY_ID                         5
#define DDS_OWNERSHIP_QOS_POLICY_ID                             6
#define DDS_OWNERSHIPSTRENGTH_QOS_POLICY_ID                     7
#define DDS_LIVELINESS_QOS_POLICY_ID                            8
#define DDS_TIMEBASEDFILTER_QOS_POLICY_ID                       9
#define DDS_PARTITION_QOS_POLICY_ID                             10
#define DDS_RELIABILITY_QOS_POLICY_ID                           11
#define DDS_DESTINATIONORDER_QOS_POLICY_ID                      12
#define DDS_HISTORY_QOS_POLICY_ID                               13
#define DDS_RESOURCELIMITS_QOS_POLICY_ID                        14
#define DDS_ENTITYFACTORY_QOS_POLICY_ID                         15
#define DDS_WRITERDATALIFECYCLE_QOS_POLICY_ID                   16
#define DDS_READERDATALIFECYCLE_QOS_POLICY_ID                   17
#define DDS_TOPICDATA_QOS_POLICY_ID                             18
#define DDS_GROUPDATA_QOS_POLICY_ID                             19
#define DDS_TRANSPORTPRIORITY_QOS_POLICY_ID                     20
#define DDS_LIFESPAN_QOS_POLICY_ID                              21
#define DDS_DURABILITYSERVICE_QOS_POLICY_ID                     22
#define DDS_SUBSCRIPTIONKEY_QOS_POLICY_ID                       23
#define DDS_VIEWKEY_QOS_POLICY_ID                               24
#define DDS_READERLIFESPAN_QOS_POLICY_ID                        25
#define DDS_SHARE_QOS_POLICY_ID                                 26
#define DDS_SCHEDULING_QOS_POLICY_ID                            27

/*
 * struct UserDataQosPolicy {
 *     sequence<octet> value;
 * };
 */
#ifndef _DDS_sequence_octet_defined
#define _DDS_sequence_octet_defined
typedef struct {
    DDS_unsigned_long _maximum;
    DDS_unsigned_long _length;
    DDS_octet *_buffer;
    DDS_boolean _release;
} DDS_sequence_octet;
OS_API DDS_sequence_octet *DDS_sequence_octet__alloc (void);
OS_API DDS_octet *DDS_sequence_octet_allocbuf (DDS_unsigned_long len);
#endif /* _DDS_sequence_octet_defined */

typedef struct DDS_UserDataQosPolicy_s DDS_UserDataQosPolicy;
struct DDS_UserDataQosPolicy_s {
    DDS_sequence_octet value;
};

/*
 * struct TopicDataQosPolicy {
 *     sequence<octet> value;
 * };
 */
typedef struct DDS_TopicDataQosPolicy_s DDS_TopicDataQosPolicy;
struct DDS_TopicDataQosPolicy_s {
    DDS_sequence_octet value;
};

/* struct GroupDataQosPolicy {
 *     sequence<octet> value;
 * };
 */
typedef struct DDS_GroupDataQosPolicy_s DDS_GroupDataQosPolicy;
struct DDS_GroupDataQosPolicy_s {
    DDS_sequence_octet value;
};

/* struct TransportPriorityQosPolicy {
 *     long value;
 * };
 */
typedef struct DDS_TransportPriorityQosPolicy_s DDS_TransportPriorityQosPolicy;
struct DDS_TransportPriorityQosPolicy_s {
    DDS_long value;
};

/* struct LifespanQosPolicy {
 *     Duration_t duration;
 * };
 */
typedef struct DDS_LifespanQosPolicy_s DDS_LifespanQosPolicy;
struct DDS_LifespanQosPolicy_s {
    DDS_Duration_t duration;
};

/* enum DurabilityQosPolicyKind {
 *     VOLATILE_DURABILITY_QOS,
 *     TRANSIENT_LOCAL_DURABILITY_QOS,
 *     TRANSIENT_DURABILITY_QOS,
 *     PERSISTENT_DURABILITY_QOS
 * };
 */
typedef enum
{
    DDS_VOLATILE_DURABILITY_QOS,
    DDS_TRANSIENT_LOCAL_DURABILITY_QOS,
    DDS_TRANSIENT_DURABILITY_QOS,
    DDS_PERSISTENT_DURABILITY_QOS
} DDS_DurabilityQosPolicyKind;

/* struct DurabilityQosPolicy {
 *     DurabilityQosPolicyKind kind;
 *     Duration_t service_cleanup_delay;
 * };
 */
typedef struct DDS_DurabilityQosPolicy_s DDS_DurabilityQosPolicy;
struct DDS_DurabilityQosPolicy_s {
    DDS_DurabilityQosPolicyKind kind;
};

/* enum PresentationQosPolicyAccessScopeKind {
 *     INSTANCE_PRESENTATION_QOS,
 *     TOPIC_PRESENTATION_QOS,
 *     GROUP_PRESENTATION_QOS
 * };
 */
typedef enum
{
    DDS_INSTANCE_PRESENTATION_QOS,
    DDS_TOPIC_PRESENTATION_QOS,
    DDS_GROUP_PRESENTATION_QOS
} DDS_PresentationQosPolicyAccessScopeKind;

/* struct PresentationQosPolicy {
 *     PresentationQosPolicyAccessScopeKind access_scope;
 *     boolean coherent_access;
 *     boolean ordered_access;
 * };
 */
typedef struct DDS_PresentationQosPolicy_s DDS_PresentationQosPolicy;
struct DDS_PresentationQosPolicy_s {
    DDS_PresentationQosPolicyAccessScopeKind access_scope;
    DDS_boolean coherent_access;
    DDS_boolean ordered_access;
};

/* struct DeadlineQosPolicy {
 *     Duration_t period;
 * };
 */
typedef struct DDS_DeadlineQosPolicy_s DDS_DeadlineQosPolicy;
struct DDS_DeadlineQosPolicy_s {
    DDS_Duration_t period;
};

/* struct LatencyBudgetQosPolicy {
 *     Duration_t duration;
 * };
 */
typedef struct DDS_LatencyBudgetQosPolicy_s DDS_LatencyBudgetQosPolicy;
struct DDS_LatencyBudgetQosPolicy_s {
    DDS_Duration_t duration;
};

/* enum OwnershipQosPolicyKind {
 *     SHARED_OWNERSHIP_QOS,
 *     EXCLUSIVE_OWNERSHIP_QOS
 * };
 */
typedef enum
{
    DDS_SHARED_OWNERSHIP_QOS,
    DDS_EXCLUSIVE_OWNERSHIP_QOS
} DDS_OwnershipQosPolicyKind;

/* struct OwnershipQosPolicy {
 *     OwnershipQosPolicyKind kind;
 * };
 */
typedef struct DDS_OwnershipQosPolicy_s DDS_OwnershipQosPolicy;
struct DDS_OwnershipQosPolicy_s {
    DDS_OwnershipQosPolicyKind kind;
};

/* struct OwnershipStrengthQosPolicy {
 *     long value;
 * };
 */
typedef struct DDS_OwnershipStrengthQosPolicy_s DDS_OwnershipStrengthQosPolicy;
struct DDS_OwnershipStrengthQosPolicy_s {
    DDS_long value;
};

/* enum LivelinessQosPolicyKind {
 *     AUTOMATIC_LIVELINESS_QOS,
 *     MANUAL_BY_PARTICIPANT_LIVELINESS_QOS,
 *     MANUAL_BY_TOPIC_LIVELINESS_QOS
 * };
 */
typedef enum
{
    DDS_AUTOMATIC_LIVELINESS_QOS,
    DDS_MANUAL_BY_PARTICIPANT_LIVELINESS_QOS,
    DDS_MANUAL_BY_TOPIC_LIVELINESS_QOS
} DDS_LivelinessQosPolicyKind;

/* struct LivelinessQosPolicy {
 *     LivelinessQosPolicyKind kind;
 *     Duration_t lease_duration;
 * };
 */
typedef struct DDS_LivelinessQosPolicy_s DDS_LivelinessQosPolicy;
struct DDS_LivelinessQosPolicy_s {
    DDS_LivelinessQosPolicyKind kind;
    DDS_Duration_t lease_duration;
};

/* struct TimeBasedFilterQosPolicy {
 *     Duration_t minimum_separation;
 * };
 */
typedef struct DDS_TimeBasedFilterQosPolicy_s DDS_TimeBasedFilterQosPolicy;
struct DDS_TimeBasedFilterQosPolicy_s {
    DDS_Duration_t minimum_separation;
};

/* struct PartitionQosPolicy {
 *     StringSeq name;
 * };
 */
typedef struct DDS_PartitionQosPolicy_s DDS_PartitionQosPolicy;
struct DDS_PartitionQosPolicy_s {
    DDS_StringSeq name;
};


/* enum ReliabilityQosPolicyKind {
 *     BEST_EFFORT_RELIABILITY_QOS,
 *     RELIABLE_RELIABILITY_QOS
 * };
 */
typedef enum
{
    DDS_BEST_EFFORT_RELIABILITY_QOS,
    DDS_RELIABLE_RELIABILITY_QOS
} DDS_ReliabilityQosPolicyKind;

/* struct ReliabilityQosPolicy {
 *     ReliabilityQosPolicyKind kind;
 *     Duration_t max_blocking_time;
 * };
 */
typedef struct DDS_ReliabilityQosPolicy_s DDS_ReliabilityQosPolicy;
struct DDS_ReliabilityQosPolicy_s {
    DDS_ReliabilityQosPolicyKind kind;
    DDS_Duration_t max_blocking_time;
    DDS_boolean synchronous;
};

/* enum DestinationOrderQosPolicyKind {
 *     BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS,
 *     BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS
 * };
 */
typedef enum
{
    DDS_BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS,
    DDS_BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS
} DDS_DestinationOrderQosPolicyKind;

/* struct DestinationOrderQosPolicy {
 *     DestinationOrderQosPolicyKind kind;
 * };
 */
typedef struct DDS_DestinationOrderQosPolicy_s DDS_DestinationOrderQosPolicy;
struct DDS_DestinationOrderQosPolicy_s {
    DDS_DestinationOrderQosPolicyKind kind;
};

/* enum HistoryQosPolicyKind {
 *     KEEP_LAST_HISTORY_QOS,
 *     KEEP_ALL_HISTORY_QOS
 * };
 */
typedef enum
{
    DDS_KEEP_LAST_HISTORY_QOS,
    DDS_KEEP_ALL_HISTORY_QOS
} DDS_HistoryQosPolicyKind;

/* struct HistoryQosPolicy {
 *     HistoryQosPolicyKind kind;
 *     long depth;
 * };
 */
typedef struct DDS_HistoryQosPolicy_s DDS_HistoryQosPolicy;
struct DDS_HistoryQosPolicy_s {
    DDS_HistoryQosPolicyKind kind;
    DDS_long depth;
};

/* struct ResourceLimitsQosPolicy {
 *     long max_samples;
 *     long max_instances;
 *     long max_samples_per_instance;
 * };
 */
typedef struct DDS_ResourceLimitsQosPolicy_s DDS_ResourceLimitsQosPolicy;
struct DDS_ResourceLimitsQosPolicy_s {
    DDS_long max_samples;
    DDS_long max_instances;
    DDS_long max_samples_per_instance;
};

/* struct EntityFactoryQosPolicy {
 *     boolean autoenable_created_entities;
 * };
 */
typedef struct DDS_EntityFactoryQosPolicy_s DDS_EntityFactoryQosPolicy;
struct DDS_EntityFactoryQosPolicy_s {
    DDS_boolean autoenable_created_entities;
};

/* struct WriterDataLifecycleQosPolicy {
 *     boolean autodispose_unregistered_instances;
 * };
 */
typedef struct DDS_WriterDataLifecycleQosPolicy_s DDS_WriterDataLifecycleQosPolicy;
struct DDS_WriterDataLifecycleQosPolicy_s {
    DDS_boolean autodispose_unregistered_instances;
    DDS_Duration_t autopurge_suspended_samples_delay;
    DDS_Duration_t autounregister_instance_delay;
};

/* struct ReaderDataLifecycleQosPolicy {
 *     Duration_t autopurge_nowriter_samples_delay;
 *     Duration_t autopurge_disposed_samples_delay;
 * };
 */
typedef struct DDS_ReaderDataLifecycleQosPolicy_s DDS_ReaderDataLifecycleQosPolicy;
struct DDS_ReaderDataLifecycleQosPolicy_s {
    DDS_Duration_t autopurge_nowriter_samples_delay;
    DDS_Duration_t autopurge_disposed_samples_delay;
    DDS_boolean enable_invalid_samples;
};

/* struct DurabilityServiceQosPolicy {
 *     HistoryQosPolicyKind history_kind;
 *     long history_depth;
 *     long max_samples;
 *     long max_instances;
 *     long max_samples_per_instance;
 *     Duration_t service_cleanup_delay;
 * };
 */
typedef struct DDS_DurabilityServiceQosPolicy_s DDS_DurabilityServiceQosPolicy;
struct DDS_DurabilityServiceQosPolicy_s {
    DDS_Duration_t service_cleanup_delay;
    DDS_HistoryQosPolicyKind history_kind;
    DDS_long history_depth;
    DDS_long max_samples;
    DDS_long max_instances;
    DDS_long max_samples_per_instance;
};

/* struct SubscriptionKeyQosPolicy {
 *     Boolean   use_key_list;
 *     StringSeq key_list;
 * };
 */
typedef struct DDS_SubscriptionKeyQosPolicy_s DDS_SubscriptionKeyQosPolicy;
struct DDS_SubscriptionKeyQosPolicy_s {
    DDS_boolean   use_key_list;
    DDS_StringSeq key_list;
};

/* struct ViewKeyQosPolicy {
 *     Boolean   use_key_list;
 *     StringSeq key_list;
 * };
 */
typedef struct DDS_ViewKeyQosPolicy_s DDS_ViewKeyQosPolicy;
struct DDS_ViewKeyQosPolicy_s {
    DDS_boolean   use_key_list;
    DDS_StringSeq key_list;
};

/* struct ReaderLifespanQosPolicy {
 *     Boolean    use_lifespan;
 *     Duration_t duration;
 * };
 */
typedef struct DDS_ReaderLifespanQosPolicy_s DDS_ReaderLifespanQosPolicy;
struct DDS_ReaderLifespanQosPolicy_s {
    DDS_boolean    use_lifespan;
    DDS_Duration_t duration;
};

/* struct ShareQosPolicy {
 *     String name;
 *     Boolean enable;
 * };
 */
typedef struct DDS_ShareQosPolicy_s DDS_ShareQosPolicy;
struct DDS_ShareQosPolicy_s {
    DDS_string name;
    DDS_boolean enable;
};

/* enum SchedulingClassQosPolicyKind {
 *     SCHEDULE_DEFAULT,
 *     SCHEDULE_TIMESHARING,
 *     SCHEDULE_REALTIME
 * };
 */
typedef enum {
    DDS_SCHEDULE_DEFAULT,
    DDS_SCHEDULE_TIMESHARING,
    DDS_SCHEDULE_REALTIME
} DDS_SchedulingClassQosPolicyKind;

/* struct SchedulingClassQosPolicy {
 *     SchedulingClassQosPolicyKind kind;
 * };
 */
typedef struct DDS_SchedulingClassQosPolicy_s DDS_SchedulingClassQosPolicy;
struct DDS_SchedulingClassQosPolicy_s {
    DDS_SchedulingClassQosPolicyKind kind;
};

/* enum SchedulingPriorityQosPolicyKind {
 *     PRIORITY_RELATIVE,
 *     PRIORITY_ABSOLUTE
 * };
 */
typedef enum {
    DDS_PRIORITY_RELATIVE,
    DDS_PRIORITY_ABSOLUTE
} DDS_SchedulingPriorityQosPolicyKind;

/* struct SchedulingPriorityQosPolicy {
 *     SchedulingPriorityQosPolicyKind kind;
 * };
 */
typedef struct DDS_SchedulingPriorityQosPolicy_s DDS_SchedulingPriorityQosPolicy;
struct DDS_SchedulingPriorityQosPolicy_s {
    DDS_SchedulingPriorityQosPolicyKind kind;
};

/* struct SchedulingQosPolicy {
 *     SchedulingClassQosPolicy    scheduling_class;
 *     SchedulingPriorityQosPolicy scheduling_priority_kind;
 *     long                        scheduling_priority;
 * };
 */
typedef struct DDS_SchedulingQosPolicy_s DDS_SchedulingQosPolicy;
struct DDS_SchedulingQosPolicy_s {
    DDS_SchedulingClassQosPolicy    scheduling_class;
    DDS_SchedulingPriorityQosPolicy scheduling_priority_kind;
    DDS_long                        scheduling_priority;
};

/*
 * struct DomainParticipantFactoryQos {
 *     UserDataQosPolicy user_data;
 *     EntityFactoryQosPolicy entity_factory;
 * };
 */
typedef struct DDS_DomainParticipantFactoryQos_s DDS_DomainParticipantFactoryQos;
struct DDS_DomainParticipantFactoryQos_s {
    DDS_EntityFactoryQosPolicy entity_factory;
};
OS_API DDS_DomainParticipantFactoryQos *DDS_DomainParticipantFactoryQos__alloc (void);

/*
 * struct DomainParticipantQos {
 *     UserDataQosPolicy user_data;
 *     EntityFactoryQosPolicy entity_factory;
 * };
 */
typedef struct DDS_DomainParticipantQos_s DDS_DomainParticipantQos;
struct DDS_DomainParticipantQos_s {
    DDS_UserDataQosPolicy user_data;
    DDS_EntityFactoryQosPolicy entity_factory;
    DDS_SchedulingQosPolicy watchdog_scheduling;
    DDS_SchedulingQosPolicy listener_scheduling;
};
OS_API DDS_DomainParticipantQos *DDS_DomainParticipantQos__alloc (void);

/*
 * struct TopicQos {
 *     TopicDataQosPolicy topic_data;
 *     DurabilityQosPolicy durability;
 *     DurabilityServiceQosPolicy durability_service;
 *     DeadlineQosPolicy deadline;
 *     LatencyBudgetQosPolicy latency_budget;
 *     LivelinessQosPolicy liveliness;
 *     ReliabilityQosPolicy reliability;
 *     DestinationOrderQosPolicy destination_order;
 *     HistoryQosPolicy history;
 *     ResourceLimitsQosPolicy resource_limits;
 *     TransportPriorityQosPolicy transport_priority;
 *     LifespanQosPolicy lifespan;
 *     OwnershipQosPolicy ownership;
 * };
 */
typedef struct DDS_TopicQos_s DDS_TopicQos;
struct DDS_TopicQos_s {
    DDS_TopicDataQosPolicy topic_data;
    DDS_DurabilityQosPolicy durability;
    DDS_DurabilityServiceQosPolicy durability_service;
    DDS_DeadlineQosPolicy deadline;
    DDS_LatencyBudgetQosPolicy latency_budget;
    DDS_LivelinessQosPolicy liveliness;
    DDS_ReliabilityQosPolicy reliability;
    DDS_DestinationOrderQosPolicy destination_order;
    DDS_HistoryQosPolicy history;
    DDS_ResourceLimitsQosPolicy resource_limits;
    DDS_TransportPriorityQosPolicy transport_priority;
    DDS_LifespanQosPolicy lifespan;
    DDS_OwnershipQosPolicy ownership;
};
OS_API DDS_TopicQos *DDS_TopicQos__alloc (void);

/*
 * struct DataWriterQos {
 *     DurabilityQosPolicy durability;
 *     DeadlineQosPolicy deadline;
 *     LatencyBudgetQosPolicy latency_budget;
 *     LivelinessQosPolicy liveliness;
 *     ReliabilityQosPolicy reliability;
 *     DestinationOrderQosPolicy destination_order;
 *     HistoryQosPolicy history;
 *     ResourceLimitsQosPolicy resource_limits;
 *     TransportPriorityQosPolicy transport_priority;
 *     LifespanQosPolicy lifespan;
 *     UserDataQosPolicy user_data;
 *     OwnershipQosPolicy ownership;
 *     OwnershipStrengthQosPolicy ownership_strength;
 *     WriterDataLifecycleQosPolicy writer_data_lifecycle;
 * };
 */
typedef struct DDS_DataWriterQos_s DDS_DataWriterQos;
struct DDS_DataWriterQos_s {
    DDS_DurabilityQosPolicy durability;
    DDS_DeadlineQosPolicy deadline;
    DDS_LatencyBudgetQosPolicy latency_budget;
    DDS_LivelinessQosPolicy liveliness;
    DDS_ReliabilityQosPolicy reliability;
    DDS_DestinationOrderQosPolicy destination_order;
    DDS_HistoryQosPolicy history;
    DDS_ResourceLimitsQosPolicy resource_limits;
    DDS_TransportPriorityQosPolicy transport_priority;
    DDS_LifespanQosPolicy lifespan;
    DDS_UserDataQosPolicy user_data;
    DDS_OwnershipQosPolicy ownership;
    DDS_OwnershipStrengthQosPolicy ownership_strength;
    DDS_WriterDataLifecycleQosPolicy writer_data_lifecycle;
};
OS_API DDS_DataWriterQos *DDS_DataWriterQos__alloc (void);

/*
 * struct PublisherQos {
 *     PresentationQosPolicy presentation;
 *     PartitionQosPolicy partition;
 *     GroupDataQosPolicy group_data;
 *     EntityFactoryQosPolicy entity_factory;
 * };
 */
typedef struct DDS_PublisherQos_s DDS_PublisherQos;
struct DDS_PublisherQos_s {
    DDS_PresentationQosPolicy presentation;
    DDS_PartitionQosPolicy partition;
    DDS_GroupDataQosPolicy group_data;
    DDS_EntityFactoryQosPolicy entity_factory;
};
OS_API DDS_PublisherQos *DDS_PublisherQos__alloc (void);

/*
 * struct DataReaderQos {
 *     DurabilityQosPolicy durability;
 *     DeadlineQosPolicy deadline;
 *     LatencyBudgetQosPolicy latency_budget;
 *     LivelinessQosPolicy liveliness;
 *     ReliabilityQosPolicy reliability;
 *     DestinationOrderQosPolicy destination_order;
 *     HistoryQosPolicy history;
 *     ResourceLimitsQosPolicy resource_limits;
 *     UserDataQosPolicy user_data;
 *     OwnershipQosPolicy ownership;
 *     TimeBasedFilterQosPolicy time_based_filter;
 *     ReaderDataLifecycleQosPolicy reader_data_lifecycle;
 *     SubscriptionKeyQosPolicy subscription_keys;
 * };
 */
typedef struct DDS_DataReaderQos_s DDS_DataReaderQos;
struct DDS_DataReaderQos_s {
    DDS_DurabilityQosPolicy durability;
    DDS_DeadlineQosPolicy deadline;
    DDS_LatencyBudgetQosPolicy latency_budget;
    DDS_LivelinessQosPolicy liveliness;
    DDS_ReliabilityQosPolicy reliability;
    DDS_DestinationOrderQosPolicy destination_order;
    DDS_HistoryQosPolicy history;
    DDS_ResourceLimitsQosPolicy resource_limits;
    DDS_UserDataQosPolicy user_data;
    DDS_OwnershipQosPolicy ownership;
    DDS_TimeBasedFilterQosPolicy time_based_filter;
    DDS_ReaderDataLifecycleQosPolicy reader_data_lifecycle;
    DDS_SubscriptionKeyQosPolicy subscription_keys;
    DDS_ReaderLifespanQosPolicy reader_lifespan;
    DDS_ShareQosPolicy share;
};
OS_API DDS_DataReaderQos *DDS_DataReaderQos__alloc (void);

/*
 * struct DataReaderViewQos {
 *      SubscriptionKeyQosPolicy subscription_keys;
 * };
 */
typedef struct DDS_DataReaderViewQos_s DDS_DataReaderViewQos;
struct DDS_DataReaderViewQos_s {
    DDS_ViewKeyQosPolicy view_keys;
};
OS_API DDS_DataReaderViewQos *DDS_DataReaderViewQos__alloc (void);

/*
 * struct SubscriberQos {
 *     PresentationQosPolicy presentation;
 *     PartitionQosPolicy partition;
 *     GroupDataQosPolicy group_data;
 *     EntityFactoryQosPolicy entity_factory;
 * };
 */
typedef struct DDS_SubscriberQos_s DDS_SubscriberQos;
struct DDS_SubscriberQos_s {
    DDS_PresentationQosPolicy presentation;
    DDS_PartitionQosPolicy partition;
    DDS_GroupDataQosPolicy group_data;
    DDS_EntityFactoryQosPolicy entity_factory;
    DDS_ShareQosPolicy share;
};
OS_API DDS_SubscriberQos *DDS_SubscriberQos__alloc (void);

/*
 * struct ParticipantBuiltinTopicData {
 *     BuiltinTopicKey_t key;
 *     UserDataQosPolicy user_data;
 * };
 */
typedef struct DDS_ParticipantBuiltinTopicData_s DDS_ParticipantBuiltinTopicData;
struct DDS_ParticipantBuiltinTopicData_s {
    DDS_BuiltinTopicKey_t key;
    DDS_UserDataQosPolicy user_data;
};
OS_API DDS_ParticipantBuiltinTopicData *DDS_ParticipantBuiltinTopicData__alloc (void);

#ifndef _DDS_sequence_DDS_ParticipantBuiltinTopicData_defined
#define _DDS_sequence_DDS_ParticipantBuiltinTopicData_defined
typedef struct {
    DDS_unsigned_long _maximum;
    DDS_unsigned_long _length;
    DDS_ParticipantBuiltinTopicData *_buffer;
    DDS_boolean _release;
} DDS_sequence_DDS_ParticipantBuiltinTopicData;

OS_API DDS_sequence_DDS_ParticipantBuiltinTopicData *DDS_sequence_DDS_ParticipantBuiltinTopicData__alloc (void);
OS_API DDS_ParticipantBuiltinTopicData *DDS_sequence_DDS_ParticipantBuiltinTopicData_allocbuf (DDS_unsigned_long len);
#endif /* _DDS_sequence_DDS_ParticipantBuiltinTopicData_defined */

/*
 * struct TopicBuiltinTopicData {
 *     BuiltinTopicKey_t key;
 *     string name;
 *     string type_name;
 *     DurabilityQosPolicy durability;
 *     DeadlineQosPolicy deadline;
 *     LatencyBudgetQosPolicy latency_budget;
 *     LivelinessQosPolicy liveliness;
 *     ReliabilityQosPolicy reliability;
 *     TransportPriorityQosPolicy transport_priority;
 *     LifespanQosPolicy lifespan;
 *     DestinationOrderQosPolicy destination_order;
 *     HistoryQosPolicy history;
 *     ResourceLimitsQosPolicy resource_limits;
 *     OwnershipQosPolicy ownership;
 *     TopicDataQosPolicy topic_data;
 * };
 */
typedef struct DDS_TopicBuiltinTopicData_s DDS_TopicBuiltinTopicData;
struct DDS_TopicBuiltinTopicData_s {
    DDS_BuiltinTopicKey_t key;
    DDS_string name;
    DDS_string type_name;
    DDS_DurabilityQosPolicy durability;
    DDS_DurabilityServiceQosPolicy durability_service;
    DDS_DeadlineQosPolicy deadline;
    DDS_LatencyBudgetQosPolicy latency_budget;
    DDS_LivelinessQosPolicy liveliness;
    DDS_ReliabilityQosPolicy reliability;
    DDS_TransportPriorityQosPolicy transport_priority;
    DDS_LifespanQosPolicy lifespan;
    DDS_DestinationOrderQosPolicy destination_order;
    DDS_HistoryQosPolicy history;
    DDS_ResourceLimitsQosPolicy resource_limits;
    DDS_OwnershipQosPolicy ownership;
    DDS_TopicDataQosPolicy topic_data;
};
OS_API DDS_TopicBuiltinTopicData *DDS_TopicBuiltinTopicData__alloc (void);

#ifndef _DDS_sequence_DDS_TopicBuiltinTopicData_defined
#define _DDS_sequence_DDS_TopicBuiltinTopicData_defined
typedef struct {
    DDS_unsigned_long _maximum;
    DDS_unsigned_long _length;
    DDS_TopicBuiltinTopicData *_buffer;
    DDS_boolean _release;
} DDS_sequence_DDS_TopicBuiltinTopicData;

OS_API DDS_sequence_DDS_TopicBuiltinTopicData *DDS_sequence_DDS_TopicBuiltinTopicData__alloc (void);
OS_API DDS_TopicBuiltinTopicData *DDS_sequence_DDS_TopicBuiltinTopicData_allocbuf (DDS_unsigned_long len);
#endif /* _DDS_sequence_DDS_TopicBuiltinTopicData_defined */

/*
 * struct PublicationBuiltinTopicData {
 *     BuiltinTopicKey_t key;
 *     BuiltinTopicKey_t participant_key;
 *     string topic_name;
 *     string type_name;
 *     DurabilityQosPolicy durability;
 *     DeadlineQosPolicy deadline;
 *     LatencyBudgetQosPolicy latency_budget;
 *     LivelinessQosPolicy liveliness;
 *     ReliabilityQosPolicy reliability;
 *     LifespanQosPolicy lifespan;
 *     DestinationOrderQosPolicy destination_order;
 *     UserDataQosPolicy user_data;
 *     OwnershipQosPolicy ownership;
 *     OwnershipStrengthQosPolicy ownership_strength;
 *     PresentationQosPolicy presentation;
 *     PartitionQosPolicy partition;
 *     TopicDataQosPolicy topic_data;
 *     GroupDataQosPolicy group_data;
 * };
 */
typedef struct DDS_PublicationBuiltinTopicData_s DDS_PublicationBuiltinTopicData;
struct DDS_PublicationBuiltinTopicData_s {
    DDS_BuiltinTopicKey_t key;
    DDS_BuiltinTopicKey_t participant_key;
    DDS_string topic_name;
    DDS_string type_name;
    DDS_DurabilityQosPolicy durability;
    DDS_DeadlineQosPolicy deadline;
    DDS_LatencyBudgetQosPolicy latency_budget;
    DDS_LivelinessQosPolicy liveliness;
    DDS_ReliabilityQosPolicy reliability;
    DDS_LifespanQosPolicy lifespan;
    DDS_DestinationOrderQosPolicy destination_order;
    DDS_UserDataQosPolicy user_data;
    DDS_OwnershipQosPolicy ownership;
    DDS_OwnershipStrengthQosPolicy ownership_strength;
    DDS_PresentationQosPolicy presentation;
    DDS_PartitionQosPolicy partition;
    DDS_TopicDataQosPolicy topic_data;
    DDS_GroupDataQosPolicy group_data;
};
OS_API DDS_PublicationBuiltinTopicData *DDS_PublicationBuiltinTopicData__alloc (void);

#ifndef _DDS_sequence_DDS_PublicationBuiltinTopicData_defined
#define _DDS_sequence_DDS_PublicationBuiltinTopicData_defined
typedef struct {
    DDS_unsigned_long _maximum;
    DDS_unsigned_long _length;
    DDS_PublicationBuiltinTopicData *_buffer;
    DDS_boolean _release;
} DDS_sequence_DDS_PublicationBuiltinTopicData;

OS_API DDS_sequence_DDS_PublicationBuiltinTopicData *DDS_sequence_DDS_PublicationBuiltinTopicData__alloc (void);
OS_API DDS_PublicationBuiltinTopicData *DDS_sequence_DDS_PublicationBuiltinTopicData_allocbuf (DDS_unsigned_long len);
#endif /* _DDS_sequence_DDS_PublicationBuiltinTopicData_defined */

/*
 * struct SubscriptionBuiltinTopicData {
 *     BuiltinTopicKey_t key;
 *     BuiltinTopicKey_t participant_key;
 *     string topic_name;
 *     string type_name;
 *     DurabilityQosPolicy durability;
 *     DeadlineQosPolicy deadline;
 *     LatencyBudgetQosPolicy latency_budget;
 *     LivelinessQosPolicy liveliness;
 *     ReliabilityQosPolicy reliability;
 *     OwnershipQosPolicy ownership;
 *     DestinationOrderQosPolicy destination_order;
 *     UserDataQosPolicy user_data;
 *     TimeBasedFilterQosPolicy time_based_filter;
 *     PresentationQosPolicy presentation;
 *     PartitionQosPolicy partition;
 *     TopicDataQosPolicy topic_data;
 *     GroupDataQosPolicy group_data;
 * };
 */
typedef struct DDS_SubscriptionBuiltinTopicData_s DDS_SubscriptionBuiltinTopicData;
struct DDS_SubscriptionBuiltinTopicData_s {
    DDS_BuiltinTopicKey_t key;
    DDS_BuiltinTopicKey_t participant_key;
    DDS_string topic_name;
    DDS_string type_name;
    DDS_DurabilityQosPolicy durability;
    DDS_DeadlineQosPolicy deadline;
    DDS_LatencyBudgetQosPolicy latency_budget;
    DDS_LivelinessQosPolicy liveliness;
    DDS_ReliabilityQosPolicy reliability;
    DDS_OwnershipQosPolicy ownership;
    DDS_DestinationOrderQosPolicy destination_order;
    DDS_UserDataQosPolicy user_data;
    DDS_TimeBasedFilterQosPolicy time_based_filter;
    DDS_PresentationQosPolicy presentation;
    DDS_PartitionQosPolicy partition;
    DDS_TopicDataQosPolicy topic_data;
    DDS_GroupDataQosPolicy group_data;
};
OS_API DDS_SubscriptionBuiltinTopicData *DDS_SubscriptionBuiltinTopicData__alloc (void);

#ifndef _DDS_sequence_DDS_SubscriptionBuiltinTopicData_defined
#define _DDS_sequence_DDS_SubscriptionBuiltinTopicData_defined
typedef struct {
    DDS_unsigned_long _maximum;
    DDS_unsigned_long _length;
    DDS_SubscriptionBuiltinTopicData *_buffer;
    DDS_boolean _release;
} DDS_sequence_DDS_SubscriptionBuiltinTopicData;

OS_API DDS_sequence_DDS_SubscriptionBuiltinTopicData *DDS_sequence_DDS_SubscriptionBuiltinTopicData__alloc (void);
OS_API DDS_SubscriptionBuiltinTopicData *DDS_sequence_DDS_SubscriptionBuiltinTopicData_allocbuf (DDS_unsigned_long len);
#endif /* _DDS_sequence_DDS_SubscriptionBuiltinTopicData_defined */

/*
 * interface Entity
 */

/*
 * // Abstract methods
 *
 * ReturnCode_t
 * set_qos(
 *     in EntityQos qos);
 *
 * ReturnCode_t
 * get_qos(
 *     inout EntityQos qos);
 *
 * ReturnCode_t
 * set_listener(
 *     in Listener l,
 *     in StatusMask mask);
 *
 * Listener
 * get_listener();
 */

/*
 *     ReturnCode_t
 *     enable();
 */
OS_API DDS_ReturnCode_t
DDS_Entity_enable (
    DDS_Entity _this);

/*
 *     StatusCondition
 *     get_statuscondition();
 */
OS_API DDS_StatusCondition
DDS_Entity_get_statuscondition (
    DDS_Entity _this);

/*
 *     StatusMask
 *     get_status_changes();
 */
OS_API DDS_StatusMask
DDS_Entity_get_status_changes (
    DDS_Entity _this);

/*
 *     InstanceHandle_t
 *     get_instance_handle();
 */
OS_API DDS_InstanceHandle_t
DDS_Entity_get_instance_handle (
    DDS_Entity _this);

/*
 * interface TypeSupport
 */
typedef DDS_Object DDS_TypeSupport;

/*
 * interface DomainParticipant : Entity
 */
typedef DDS_Object DDS_DomainParticipant;

/*  From Entity
 *     enable
 */
#define DDS_DomainParticipant_enable DDS_Entity_enable

/*  From Entity
 *     get_statuscondition
 */
#define DDS_DomainParticipant_get_statuscondition DDS_Entity_get_statuscondition

/*  From Entity
 *     get_status_changes
 */
#define DDS_DomainParticipant_get_status_changes DDS_Entity_get_status_changes

/*  From Entity
 *     get_instance_handle
 */
#define DDS_DomainParticipant_get_instance_handle DDS_Entity_get_instance_handle

/*     Publisher
 *     create_publisher(
 *         in PublisherQos qos,
 *         in PublisherListener a_listener,
 *         in StatusMask mask);
 */
OS_API DDS_Publisher
DDS_DomainParticipant_create_publisher (
    DDS_DomainParticipant _this,
    const DDS_PublisherQos *qos,
    const struct DDS_PublisherListener *a_listener,
    const DDS_StatusMask mask);

/*     ReturnCode_t
 *     delete_publisher(
 *         in Publisher p);
 */
OS_API DDS_ReturnCode_t
DDS_DomainParticipant_delete_publisher (
    DDS_DomainParticipant _this,
    const DDS_Publisher p);

/*     Subscriber
 *     create_subscriber(
 *         in SubscriberQos qos,
 *         in SubscriberListener a_listener,
 *         in StatusMask mask);
 */
OS_API DDS_Subscriber
DDS_DomainParticipant_create_subscriber (
    DDS_DomainParticipant _this,
    const DDS_SubscriberQos *qos,
    const struct DDS_SubscriberListener *a_listener,
    const DDS_StatusMask mask);

/*     ReturnCode_t
 *     delete_subscriber(
 *         in Subscriber s);
 */
OS_API DDS_ReturnCode_t
DDS_DomainParticipant_delete_subscriber (
    DDS_DomainParticipant _this,
    const DDS_Subscriber s);

/*     Subscriber
 *     get_builtin_subscriber();
 */
OS_API DDS_Subscriber
DDS_DomainParticipant_get_builtin_subscriber (
    DDS_DomainParticipant _this);

/*     Topic
 *     create_topic(
 *         in string topic_name,
 *         in string type_name,
 *         in TopicQos qos,
 *         in TopicListener a_listener,
 *         in StatusMask mask);
 */
OS_API DDS_Topic
DDS_DomainParticipant_create_topic (
    DDS_DomainParticipant _this,
    const DDS_char *topic_name,
    const DDS_char *type_name,
    const DDS_TopicQos *qos,
    const struct DDS_TopicListener *a_listener,
    const DDS_StatusMask mask);

/*     ReturnCode_t
 *     delete_topic(
 *         in Topic a_topic);
 */
OS_API DDS_ReturnCode_t
DDS_DomainParticipant_delete_topic (
    DDS_DomainParticipant _this,
    const DDS_Topic a_topic);

/*     Topic
 *     find_topic(
 *         in string topic_name,
 *         in Duration_t timeout);
 */
OS_API DDS_Topic
DDS_DomainParticipant_find_topic (
    DDS_DomainParticipant _this,
    const DDS_char *topic_name,
    const DDS_Duration_t *timeout);

/*     TopicDescription
 *     lookup_topicdescription(
 *         in string name);
 */
OS_API DDS_TopicDescription
DDS_DomainParticipant_lookup_topicdescription (
    DDS_DomainParticipant _this,
    const DDS_char *name);

/*     ContentFilteredTopic
 *     create_contentfilteredtopic(
 *         in string name,
 *         in Topic related_topic,
 *         in string filter_expression,
 *         in StringSeq filter_parameters);
 */
OS_API DDS_ContentFilteredTopic
DDS_DomainParticipant_create_contentfilteredtopic (
    DDS_DomainParticipant _this,
    const DDS_char *name,
    const DDS_Topic related_topic,
    const DDS_char *filter_expression,
    const DDS_StringSeq *filter_parameters);

/*     ReturnCode_t
 *     delete_contentfilteredtopic(
 *         in ContentFilteredTopic a_contentfilteredtopic);
 */
OS_API DDS_ReturnCode_t
DDS_DomainParticipant_delete_contentfilteredtopic (
    DDS_DomainParticipant _this,
    const DDS_ContentFilteredTopic a_contentfilteredtopic);

/*     MultiTopic
 *     create_multitopic(
 *         in string name,
 *         in string type_name,
 *         in string subscription_expression,
 *         in StringSeq expression_parameters);
 */
OS_API DDS_MultiTopic
DDS_DomainParticipant_create_multitopic (
    DDS_DomainParticipant _this,
    const DDS_char *name,
    const DDS_char *type_name,
    const DDS_char *subscription_expression,
    const DDS_StringSeq *expression_parameters);

/*     ReturnCode_t
 *     delete_multitopic(
 *         in MultiTopic a_multitopic);
 */
OS_API DDS_ReturnCode_t
DDS_DomainParticipant_delete_multitopic (
    DDS_DomainParticipant _this,
    const DDS_MultiTopic a_multitopic);

/*     ReturnCode_t
 *     delete_contained_entities();
 */
OS_API DDS_ReturnCode_t
DDS_DomainParticipant_delete_contained_entities (
    DDS_DomainParticipant _this);

/*     ReturnCode_t
 *     set_qos(
 *         in DomainParticipantQos qos);
 */
OS_API DDS_ReturnCode_t
DDS_DomainParticipant_set_qos (
    DDS_DomainParticipant _this,
    const DDS_DomainParticipantQos *qos);

/*     ReturnCode_t
 *     get_qos(
 *         inout DomainParticipantQos qos);
 */
OS_API DDS_ReturnCode_t
DDS_DomainParticipant_get_qos (
    DDS_DomainParticipant _this,
    DDS_DomainParticipantQos *qos);

/*     ReturnCode_t
 *     set_listener(
 *         in DomainParticipantListener a_listener,
 *         in StatusMask mask);
 */
OS_API DDS_ReturnCode_t
DDS_DomainParticipant_set_listener (
    DDS_DomainParticipant _this,
    const struct DDS_DomainParticipantListener *a_listener,
    const DDS_StatusMask mask);

/*     DomainParticipantListener
 *     get_listener();
 */
OS_API struct DDS_DomainParticipantListener
DDS_DomainParticipant_get_listener (
    DDS_DomainParticipant _this);

/*     ReturnCode_t
 *     ignore_participant(
 *         in InstanceHandle_t handle);
 */
OS_API DDS_ReturnCode_t
DDS_DomainParticipant_ignore_participant (
    DDS_DomainParticipant _this,
    const DDS_InstanceHandle_t handle);

/*     ReturnCode_t
 *     ignore_topic(
 *         in InstanceHandle_t handle);
 */
OS_API DDS_ReturnCode_t
DDS_DomainParticipant_ignore_topic (
    DDS_DomainParticipant _this,
    const DDS_InstanceHandle_t handle);

/*     ReturnCode_t
 *     ignore_publication(
 *         in InstanceHandle_t handle);
 */
OS_API DDS_ReturnCode_t
DDS_DomainParticipant_ignore_publication (
    DDS_DomainParticipant _this,
    const DDS_InstanceHandle_t handle);

/*     ReturnCode_t
 *     ignore_subscription(
 *         in InstanceHandle_t handle);
 */
OS_API DDS_ReturnCode_t
DDS_DomainParticipant_ignore_subscription (
    DDS_DomainParticipant _this,
    const DDS_InstanceHandle_t handle);

/*     DomainId_t
 *     get_domain_id();
 */
OS_API DDS_DomainId_t
DDS_DomainParticipant_get_domain_id (
    DDS_DomainParticipant _this);

/*     ReturnCode_t
 *     assert_liveliness();
 */
OS_API DDS_ReturnCode_t
DDS_DomainParticipant_assert_liveliness (
    DDS_DomainParticipant _this);

/*     ReturnCode_t
 *     set_default_publisher_qos(
 *         in PublisherQos qos);
 */
OS_API DDS_ReturnCode_t
DDS_DomainParticipant_set_default_publisher_qos (
    DDS_DomainParticipant _this,
    const DDS_PublisherQos *qos);

/*     ReturnCode_t
 *     get_default_publisher_qos(
 *         inout PublisherQos qos);
 */
OS_API DDS_ReturnCode_t
DDS_DomainParticipant_get_default_publisher_qos (
    DDS_DomainParticipant _this,
    DDS_PublisherQos *qos);

/*     ReturnCode_t
 *     set_default_subscriber_qos(
 *         in SubscriberQos qos);
 */
OS_API DDS_ReturnCode_t
DDS_DomainParticipant_set_default_subscriber_qos (
    DDS_DomainParticipant _this,
    const DDS_SubscriberQos *qos);

/*     ReturnCode_t
 *     get_default_subscriber_qos(
 *         inout SubscriberQos qos);
 */
OS_API DDS_ReturnCode_t
DDS_DomainParticipant_get_default_subscriber_qos (
    DDS_DomainParticipant _this,
    DDS_SubscriberQos *qos);

/*     ReturnCode_t
 *     set_default_topic_qos(
 *         in TopicQos qos);
 */
OS_API DDS_ReturnCode_t
DDS_DomainParticipant_set_default_topic_qos (
    DDS_DomainParticipant _this,
    const DDS_TopicQos *qos);

/*     ReturnCode_t
 *     get_default_topic_qos(
 *         inout TopicQos qos);
 */
OS_API DDS_ReturnCode_t
DDS_DomainParticipant_get_default_topic_qos (
    DDS_DomainParticipant _this,
    DDS_TopicQos *qos);

/*     ReturnCode_t
 *     get_discovered_participants (
 *         inout InstanceHandleSeq participant_handles);
 */
OS_API DDS_ReturnCode_t
DDS_DomainParticipant_get_discovered_participants (
    DDS_DomainParticipant _this,
    DDS_InstanceHandleSeq  *participant_handles);

/*     ReturnCode_t
 *     get_discovered_participant_data (
 *         in InstanceHandle_t handle,
 *         inout ParticipantBuiltinTopicData *participant_data);
 */
OS_API DDS_ReturnCode_t
DDS_DomainParticipant_get_discovered_participant_data (
    DDS_DomainParticipant _this,
    DDS_ParticipantBuiltinTopicData *participant_data,
    DDS_InstanceHandle_t  handle);

/*     ReturnCode_t
 *     get_discovered_topics (
 *         inout InstanceHandleSeq topic_handles);
 */
OS_API DDS_ReturnCode_t
DDS_DomainParticipant_get_discovered_topics (
    DDS_DomainParticipant _this,
    DDS_InstanceHandleSeq  *topic_handles);

/*     ReturnCode_t
 *     get_discovered_topic_data (
 *         in InstanceHandle_t handle,
 *         inout TopicBuiltinTopicData *topic_data);
 */
OS_API DDS_ReturnCode_t
DDS_DomainParticipant_get_discovered_topic_data (
    DDS_DomainParticipant _this,
    DDS_TopicBuiltinTopicData *topic_data,
    DDS_InstanceHandle_t  handle);

/*     Boolean
 *     contains_entity (
 *         in InstanceHandle_t a_hande);
 */
OS_API DDS_boolean
DDS_DomainParticipant_contains_entity (
    DDS_DomainParticipant _this,
    DDS_InstanceHandle_t  a_handle);

/*     ReturnCode_t
 *     get_current_time (
 *         inout Time_t current_time);
 */
OS_API DDS_ReturnCode_t
DDS_DomainParticipant_get_current_time (
    DDS_DomainParticipant _this,
    DDS_Time_t  *current_time);

/*     TypeSupport
 *     lookup_typesupport (
 *         in string registered_type_name);
 */
OS_API DDS_TypeSupport
DDS_DomainParticipant_lookup_typesupport (
    DDS_DomainParticipant _this,
    const DDS_char *type_name);

/*     ReturnCode_t
 *     delete_historical_data(
 *         in string partition_expression,
 *         in string topic_expression);
 */
OS_API DDS_ReturnCode_t
DDS_DomainParticipant_delete_historical_data (
    DDS_DomainParticipant _this,
    const DDS_string partition_expression,
    const DDS_string topic_expression);

/*
 * interface Domain
 */
typedef DDS_Object DDS_Domain;

/*     DDS_ReturnCode_t
 *     create_persistent_snapshot(
 *         in String partition_expression,
 *         in String topic_expression,
 *         in String URI);
 */
OS_API DDS_ReturnCode_t
DDS_Domain_create_persistent_snapshot (
    DDS_DomainParticipant _this,
    const DDS_char *partition_expression,
    const DDS_char *topic_expression,
    const DDS_char *URI);

/*
 * interface DomainParticipantFactory
 */
typedef DDS_Object DDS_DomainParticipantFactory;

/*
 * From Specification
 *
 *     DomainParticipantFactory get_instance (void)
 */
OS_API DDS_DomainParticipantFactory
DDS_DomainParticipantFactory_get_instance (void);

/*     DomainParticipant
 *     create_participant(
 *         in DomainId_t domainId,
 *         in DomainParticipantQos qos,
 *         in DomainParticipantListener a_listener,
 *         in StatusMask mask);
 */
OS_API DDS_DomainParticipant
DDS_DomainParticipantFactory_create_participant (
     DDS_DomainParticipantFactory _this,
     const DDS_DomainId_t domain_id,
     const DDS_DomainParticipantQos *qos,
     const struct DDS_DomainParticipantListener *a_listener,
     const DDS_StatusMask mask);

/*     ReturnCode_t
 *     delete_participant(
 *         in DomainParticipant a_participant);
 */
OS_API DDS_ReturnCode_t
DDS_DomainParticipantFactory_delete_participant (
    DDS_DomainParticipantFactory _this,
    const DDS_DomainParticipant a_participant);

/*     DomainParticipant
 *     lookup_participant(
 *         in DomainId_t domainId);
 */
OS_API DDS_DomainParticipant
DDS_DomainParticipantFactory_lookup_participant (
    DDS_DomainParticipantFactory _this,
    const DDS_DomainId_t domain_id);

/*     ReturnCode_t
 *     set_qos(
 *         in DomainParticipantFactoryQos qos);
 */
OS_API DDS_ReturnCode_t
DDS_DomainParticipantFactory_set_qos (
    DDS_DomainParticipantFactory _this,
    const DDS_DomainParticipantFactoryQos *qos);

/*     ReturnCode_t
 *     get_qos(
 *         inout DomainParticipantFactoryQos qos);
 */
OS_API DDS_ReturnCode_t
DDS_DomainParticipantFactory_get_qos (
    DDS_DomainParticipantFactory _this,
    DDS_DomainParticipantFactoryQos *qos);


/*     ReturnCode_t
 *     set_default_participant_qos(
 *         in DomainParticipantQos qos);
 */
OS_API DDS_ReturnCode_t
DDS_DomainParticipantFactory_set_default_participant_qos (
    DDS_DomainParticipantFactory _this,
    const DDS_DomainParticipantQos *qos);

/*     ReturnCode_t
 *     get_default_participant_qos(
 *         inout DomainParticipantQos qos);
 */
OS_API DDS_ReturnCode_t
DDS_DomainParticipantFactory_get_default_participant_qos (
    DDS_DomainParticipantFactory _this,
    DDS_DomainParticipantQos *qos);

/*     Domain
 *     lookup_domain(
 *         in DomainId domain_id);
 */
OS_API DDS_Domain
DDS_DomainParticipantFactory_lookup_domain (
    DDS_DomainParticipantFactory _this,
    const DDS_DomainId_t domain_id);

/*     ReturnCode_t
 *     delete_domain(
 *         in Domain a_domain);
 */
OS_API DDS_ReturnCode_t
DDS_DomainParticipantFactory_delete_domain (
    DDS_DomainParticipantFactory _this,
    DDS_Domain a_domain);

/*     ReturnCode_t
 *     delete_contained_entities(
 *         );
 */
OS_API DDS_ReturnCode_t
DDS_DomainParticipantFactory_delete_contained_entities (
    DDS_DomainParticipantFactory _this);

/*
 * interface TypeSupport
 */
OS_API DDS_TypeSupport
DDS_TypeSupport__alloc (
    const DDS_char *type_name,
    const DDS_char *type_keys,
    const DDS_char *type_descr);

/*
 * // Abstract methods
 *
 * ReturnCode_t
 * register_type(
 *     in DomainParticipant domain,
 *     in string type_name);
 */

OS_API DDS_ReturnCode_t
DDS_TypeSupport_register_type (
    DDS_TypeSupport _this,
    const DDS_DomainParticipant domain,
    const DDS_string name);

/* char *
 * get_description ();
 */
OS_API DDS_char *
DDS_TypeSupport_get_description (
    DDS_TypeSupport _this);

/* char *
 * get_key_list ();
 */
OS_API DDS_char *
DDS_TypeSupport_get_key_list (
    DDS_TypeSupport _this);

/* string
 *   get_type_name();
 */
OS_API DDS_char *
DDS_TypeSupport_get_type_name(
    DDS_TypeSupport _this);

/* void *
 * allocbuf(
 *     in unsigned long len);
 */
OS_API void *
DDS_TypeSupport_allocbuf (
    DDS_TypeSupport _this,
    DDS_unsigned_long len);

/*
 * interface TopicDescription
 */

/*     string
 *     get_type_name();
 */
OS_API DDS_string
DDS_TopicDescription_get_type_name (
    DDS_TopicDescription _this);

/*     string
 *     get_name();
 */
OS_API DDS_string
DDS_TopicDescription_get_name (
    DDS_TopicDescription _this);

/*     DomainParticipant
 *     get_participant();
 */
OS_API DDS_DomainParticipant
DDS_TopicDescription_get_participant (
    DDS_TopicDescription _this);

/*
 * interface Topic : Entity, TopicDescription
 */

/*  From Entity
 *     enable
 */
#define DDS_Topic_enable DDS_Entity_enable

/*  From Entity
 *     get_statuscondition
 */
#define DDS_Topic_get_statuscondition DDS_Entity_get_statuscondition

/*  From Entity
 *     get_status_changes
 */
#define DDS_Topic_get_status_changes DDS_Entity_get_status_changes

/*  From Entity
 *     get_instance_handle
 */
#define DDS_Topic_get_instance_handle DDS_Entity_get_instance_handle

/*  From TopicDescription
 *     get_type_name
 */
#define DDS_Topic_get_type_name DDS_TopicDescription_get_type_name

/*  From TopicDescription
 *     get_name
 */
#define DDS_Topic_get_name DDS_TopicDescription_get_name

/*  From TopicDescription
 *     get_participant
 */
#define DDS_Topic_get_participant DDS_TopicDescription_get_participant

/*     // Access the status
 * ReturnCode_t
 * get_inconsistent_topic_status(
 *       inout InconsistentTopicStatus a_status);
 */
OS_API DDS_ReturnCode_t
DDS_Topic_get_inconsistent_topic_status (
    DDS_Topic _this,
    DDS_InconsistentTopicStatus *a_status);

/*     ReturnCode_t
 *     set_listener(
 *         in TopicListener a_listener,
 *         in StatusMask mask);
 */
OS_API DDS_ReturnCode_t
DDS_Topic_set_listener (
    DDS_Topic _this,
    const struct DDS_TopicListener *a_listener,
    const DDS_StatusMask mask);

/*     TopicListener
 *     get_listener();
 */
OS_API struct DDS_TopicListener
DDS_Topic_get_listener (
    DDS_Topic _this);

/*     ReturnCode_t
 *     set_qos(
 *         in TopicQos qos);
 */
OS_API DDS_ReturnCode_t
DDS_Topic_set_qos (
    DDS_Topic _this,
    const DDS_TopicQos *qos);

/*     ReturnCode_t
 *     get_qos(
 *         inout TopicQos qos);
 */
OS_API DDS_ReturnCode_t
DDS_Topic_get_qos (
    DDS_Topic _this,
    DDS_TopicQos *qos);

/*     DDS_ReturnCode_t
 *     dispose_all_data();
 */
OS_API DDS_ReturnCode_t
DDS_Topic_dispose_all_data (
    DDS_Topic _this);

/*
 * interface ContentFilteredTopic : TopicDescription
 */

/*  From TopicDescription
 *     get_type_name
 */
#define DDS_ContentFilteredTopic_get_type_name DDS_TopicDescription_get_type_name

/*  From TopicDescription
 *     get_name
 */
#define DDS_ContentFilteredTopic_get_name DDS_TopicDescription_get_name

/*  From TopicDescription
 *     get_participant
 */
#define DDS_ContentFilteredTopic_get_participant DDS_TopicDescription_get_participant

/*     string
 *     get_filter_expression();
 */
OS_API DDS_string
DDS_ContentFilteredTopic_get_filter_expression (
    DDS_ContentFilteredTopic _this);

/*     ReturnCode_t
 *     get_expression_parameters(
 *         inout StringSeq);
 */
OS_API DDS_ReturnCode_t
DDS_ContentFilteredTopic_get_expression_parameters (
    DDS_ContentFilteredTopic _this,
    DDS_StringSeq *expression_parameters);

/*     ReturnCode_t
 *     set_expression_parameters(
 *         in StringSeq expression_parameters);
 */
OS_API DDS_ReturnCode_t
DDS_ContentFilteredTopic_set_expression_parameters (
    DDS_ContentFilteredTopic _this,
    const DDS_StringSeq *expression_parameters);

/*     Topic
 *     get_related_topic();
 */
OS_API DDS_Topic
DDS_ContentFilteredTopic_get_related_topic (
    DDS_ContentFilteredTopic _this);

/*
 * interface MultiTopic : TopicDescription
 */

/*  From TopicDescription
 *     get_type_name
 */
#define DDS_MultiTopic_get_type_name DDS_TopicDescription_get_type_name

/*  From TopicDescription
 *     get_name
 */
#define DDS_MultiTopic_get_name DDS_TopicDescription_get_name

/*  From TopicDescription
 *     get_participant
 */
#define DDS_MultiTopic_get_participant DDS_TopicDescription_get_participant

/*     string
 *     get_subscription_expression();
 */
OS_API DDS_string
DDS_MultiTopic_get_subscription_expression (
    DDS_MultiTopic _this);

/*     ReturnCode_t
 *     get_expression_parameters(
 *          inout StringSeq);
 */
OS_API DDS_ReturnCode_t
DDS_MultiTopic_get_expression_parameters (
    DDS_MultiTopic _this,
    DDS_StringSeq *expression_parameters);

/*     ReturnCode_t
 *     set_expression_parameters(
 *         in StringSeq expression_parameters);
 */
OS_API DDS_ReturnCode_t
DDS_MultiTopic_set_expression_parameters (
    DDS_MultiTopic _this,
    const DDS_StringSeq *expression_parameters);

/*
 * interface Publisher : Entity
 */

/*  From Entity
 *     enable
 */
#define DDS_Publisher_enable DDS_Entity_enable

/*  From Entity
 *     get_statuscondition
 */
#define DDS_Publisher_get_statuscondition DDS_Entity_get_statuscondition

/*  From Entity
 *     get_status_changes
 */
#define DDS_Publisher_get_status_changes DDS_Entity_get_status_changes

/*  From Entity
 *     get_instance_handle
 */
#define DDS_Publisher_get_instance_handle DDS_Entity_get_instance_handle

/*     DataWriter
 *     create_datawriter(
 *         in Topic a_topic,
 *         in DataWriterQos qos,
 *         in DataWriterListener a_listener,
 *         in StatusMask mask);
 */
OS_API DDS_DataWriter
DDS_Publisher_create_datawriter (
    DDS_Publisher _this,
    const DDS_Topic a_topic,
    const DDS_DataWriterQos *qos,
    const struct DDS_DataWriterListener *a_listener,
    const DDS_StatusMask mask);

/*     ReturnCode_t
 *     delete_datawriter(
 *         in DataWriter a_datawriter);
 */
OS_API DDS_ReturnCode_t
DDS_Publisher_delete_datawriter (
    DDS_Publisher _this,
    const DDS_DataWriter a_datawriter);

/*     DataWriter
 *     lookup_datawriter(
 *         in string topic_name);
 */
OS_API DDS_DataWriter
DDS_Publisher_lookup_datawriter (
    DDS_Publisher _this,
    const DDS_char *topic_name);

/*     ReturnCode_t
 *     delete_contained_entities();
 */
OS_API DDS_ReturnCode_t
DDS_Publisher_delete_contained_entities (
    DDS_Publisher _this);

/*     ReturnCode_t
 *     set_qos(
 *         in PublisherQos qos);
 */
OS_API DDS_ReturnCode_t
DDS_Publisher_set_qos (
    DDS_Publisher _this,
    const DDS_PublisherQos *qos);

/*     ReturnCode_t
 *     get_qos(
 *         inout PublisherQos qos);
 */
OS_API DDS_ReturnCode_t
DDS_Publisher_get_qos (
    DDS_Publisher _this,
    DDS_PublisherQos *qos);

/*     ReturnCode_t
 *     set_listener(
 *         in PublisherListener a_listener,
 *         in StatusMask mask);
 */
OS_API DDS_ReturnCode_t
DDS_Publisher_set_listener (
    DDS_Publisher _this,
    const struct DDS_PublisherListener *a_listener,
    const DDS_StatusMask mask);

/*     PublisherListener
 *     get_listener();
 */
OS_API struct DDS_PublisherListener
DDS_Publisher_get_listener (
    DDS_Publisher _this);

/*     ReturnCode_t
 *     suspend_publications();
 */
OS_API DDS_ReturnCode_t
DDS_Publisher_suspend_publications (
    DDS_Publisher _this);

/*     ReturnCode_t
 *     resume_publications();
 */
OS_API DDS_ReturnCode_t
DDS_Publisher_resume_publications (
    DDS_Publisher _this);

/*     ReturnCode_t
 *     begin_coherent_changes();
 */
OS_API DDS_ReturnCode_t
DDS_Publisher_begin_coherent_changes (
    DDS_Publisher _this);

/*     ReturnCode_t
 *     end_coherent_changes();
 */
OS_API DDS_ReturnCode_t
DDS_Publisher_end_coherent_changes (
    DDS_Publisher _this);

/* ReturnCode_t
 *   wait_for_acknowledgments(
 *      in Duration_t max_wait);
 */
OS_API DDS_ReturnCode_t
DDS_Publisher_wait_for_acknowledgments (
    DDS_Publisher _this,
    const DDS_Duration_t *max_wait);

/*     DomainParticipant
 *     get_participant();
 */
OS_API DDS_DomainParticipant
DDS_Publisher_get_participant (
    DDS_Publisher _this);

/*     ReturnCode_t
 *     set_default_datawriter_qos(
 *         in DataWriterQos qos);
 */
OS_API DDS_ReturnCode_t
DDS_Publisher_set_default_datawriter_qos (
    DDS_Publisher _this,
    const DDS_DataWriterQos *qos);

/*     ReturnCode_t
 *     get_default_datawriter_qos(
 *         inout DataWriterQos qos);
 */
OS_API DDS_ReturnCode_t
DDS_Publisher_get_default_datawriter_qos (
    DDS_Publisher _this,
    DDS_DataWriterQos *qos);

/*     ReturnCode_t
 *     copy_from_topic_qos(
 *         inout DataWriterQos a_datawriter_qos,
 *         in TopicQos a_topic_qos);
 */
OS_API DDS_ReturnCode_t
DDS_Publisher_copy_from_topic_qos (
    DDS_Publisher _this,
    DDS_DataWriterQos *a_datawriter_qos,
    const DDS_TopicQos *a_topic_qos);

/*
 * interface DataWriter : Entity
 */

/*
 * // Abstract methods
 *
 * InstanceHandle_t
 * register_instance(
 *     in Data instance_data);
 *
 * InstanceHandle_t
 * register_instance_w_timestamp(
 *     in Data instance_data,
 *     in Time_t source_timestamp);
 *
 * ReturnCode_t
 * unregister_instance(
 *     in Data instance_data,
 *     in InstanceHandle_t handle);
 *
 * ReturnCode_t
 * unregister_instance_w_timestamp(
 *     in Data instance_data,
 *     in InstanceHandle_t handle,
 *     in Time_t source_timestamp);
 *
 * ReturnCode_t
 * write(
 *     in Data instance_data,
 *     in InstanceHandle_t handle);
 *
 * ReturnCode_t
 * write_w_timestamp(
 *     in Data instance_data,
 *     in InstanceHandle_t handle,
 *     in Time_t source_timestamp);
 *
 * ReturnCode_t
 * dispose(
 *     in Data instance_data,
 *     in InstanceHandle_t instance_handle);
 *
 * ReturnCode_t
 * dispose_w_timestamp(
 *     in Data instance_data,
 *     in InstanceHandle_t instance_handle,
 *     in Time_t source_timestamp);
 *
 * ReturnCode_t
 * get_key_value(
 *     inout Data key_holder,
 *     in InstanceHandle_t handle);
 */

/*  From Entity
 *     enable
 */
#define DDS_DataWriter_enable DDS_Entity_enable

/*  From Entity
 *     get_statuscondition
 */
#define DDS_DataWriter_get_statuscondition DDS_Entity_get_statuscondition

/*  From Entity
 *     get_status_changes
 */
#define DDS_DataWriter_get_status_changes DDS_Entity_get_status_changes

/*  From Entity
 *     get_instance_handle
 */
#define DDS_DataWriter_get_instance_handle DDS_Entity_get_instance_handle

/*     ReturnCode_t
 *     set_qos(
 *         in DataWriterQos qos);
 */
OS_API DDS_ReturnCode_t
DDS_DataWriter_set_qos (
    DDS_DataWriter _this,
    const DDS_DataWriterQos *qos);

/*     ReturnCode_t
 *     get_qos(
 *         inout DataWriterQos qos);
 */
OS_API DDS_ReturnCode_t
DDS_DataWriter_get_qos (
    DDS_DataWriter _this,
    DDS_DataWriterQos *qos);

/*     ReturnCode_t
 *     set_listener(
 *         in DataWriterListener a_listener,
 *         in StatusMask mask);
 */
OS_API DDS_ReturnCode_t
DDS_DataWriter_set_listener (
    DDS_DataWriter _this,
    const struct DDS_DataWriterListener *a_listener,
    const DDS_StatusMask mask);

/*     DataWriterListener
 *     get_listener();
 */
OS_API struct DDS_DataWriterListener
DDS_DataWriter_get_listener (
    DDS_DataWriter _this);

/*     Topic
 *     get_topic();
 */
OS_API DDS_Topic
DDS_DataWriter_get_topic (
    DDS_DataWriter _this);

/*     Publisher
 *     get_publisher();
 */
OS_API DDS_Publisher
DDS_DataWriter_get_publisher (
    DDS_DataWriter _this);

/* ReturnCode_t
 *   wait_for_acknowledgments(
 *      in Duration_t max_wait);
 */
OS_API DDS_ReturnCode_t
DDS_DataWriter_wait_for_acknowledgments (
    DDS_DataWriter _this,
    const DDS_Duration_t *max_wait);

/*     // Access the status
 * ReturnCode_t
 * get_liveliness_lost_status(
 *       inout LivelinessLostStatus a_status);
 */
OS_API DDS_ReturnCode_t
DDS_DataWriter_get_liveliness_lost_status (
    DDS_DataWriter _this,
    DDS_LivelinessLostStatus *a_status);

/*     // Access the status
 * ReturnCode_t
 * get_offered_deadline_missed_status(
 *       inout OfferedDeadlineMissedStatus a_status);
 */
OS_API DDS_ReturnCode_t
DDS_DataWriter_get_offered_deadline_missed_status (
    DDS_DataWriter _this,
    DDS_OfferedDeadlineMissedStatus *a_status);

/*     // Access the status
 * ReturnCode_t
 * get_offered_incompatible_qos_status(
 *       inout OfferedIncompatibleQosStatus a_status);
 */
OS_API DDS_ReturnCode_t
DDS_DataWriter_get_offered_incompatible_qos_status (
    DDS_DataWriter _this,
    DDS_OfferedIncompatibleQosStatus *a_status);

/*     // Access the status
 * ReturnCode_t
 * get_publication_matched_status(
 *       inout PublicationMatchedStatus a_status);
 */
OS_API DDS_ReturnCode_t
DDS_DataWriter_get_publication_matched_status (
    DDS_DataWriter _this,
    DDS_PublicationMatchedStatus *a_status);

/*     ReturnCode_t
 *     assert_liveliness();
 */
OS_API DDS_ReturnCode_t
DDS_DataWriter_assert_liveliness (
    DDS_DataWriter _this);

/*     ReturnCode_t
 *     get_matched_subscriptions(
 *         inout InstanceHandleSeq subscription_handles);
 */
OS_API DDS_ReturnCode_t
DDS_DataWriter_get_matched_subscriptions (
    DDS_DataWriter _this,
    DDS_InstanceHandleSeq *subscription_handles);

/*     ReturnCode_t
 *     get_matched_subscription_data(
 *         inout SubscriptionBuiltinTopicData subscription_data,
 *         in InstanceHandle_t subscription_handle);
 */
OS_API DDS_ReturnCode_t
DDS_DataWriter_get_matched_subscription_data (
    DDS_DataWriter _this,
    DDS_SubscriptionBuiltinTopicData *subscription_data,
    const DDS_InstanceHandle_t subscription_handle);

/*
 * interface Subscriber : Entity
 */

/*  From Entity
 *     enable
 */
#define DDS_Subscriber_enable DDS_Entity_enable

/*  From Entity
 *     get_statuscondition
 */
#define DDS_Subscriber_get_statuscondition DDS_Entity_get_statuscondition

/*  From Entity
 *     get_status_changes
 */
#define DDS_Subscriber_get_status_changes DDS_Entity_get_status_changes

/*  From Entity
 *     get_instance_handle
 */
#define DDS_Subscriber_get_instance_handle DDS_Entity_get_instance_handle

/*     DataReader
 *     create_datareader(
 *         in TopicDescription a_topic,
 *         in DataReaderQos qos,
 *         in DataReaderListener a_listener,
 *         in StatusMask mask);
 */
OS_API DDS_DataReader
DDS_Subscriber_create_datareader (
    DDS_Subscriber _this,
    const DDS_TopicDescription a_topic,
    const DDS_DataReaderQos *qos,
    const struct DDS_DataReaderListener *a_listener,
    const DDS_StatusMask mask);

/*     ReturnCode_t
 *     delete_datareader(
 *         in DataReader a_datareader);
 */
OS_API DDS_ReturnCode_t
DDS_Subscriber_delete_datareader (
    DDS_Subscriber _this,
    const DDS_DataReader a_datareader);

/*     ReturnCode_t
 *     delete_contained_entities();
 */
OS_API DDS_ReturnCode_t
DDS_Subscriber_delete_contained_entities (
    DDS_Subscriber _this);

/*     DataReader
 *     lookup_datareader(
 *         in string topic_name);
 */
OS_API DDS_DataReader
DDS_Subscriber_lookup_datareader (
    DDS_Subscriber _this,
    const DDS_char *topic_name);

/*     ReturnCode_t
 *     get_datareaders(
 *         inout DataReaderSeq readers,
 *         in SampleStateMask sample_states,
 *         in ViewStateMask view_states,
 *         in InstanceStateMask instance_states);
 */
OS_API DDS_ReturnCode_t
DDS_Subscriber_get_datareaders (
    DDS_Subscriber _this,
    DDS_DataReaderSeq *readers,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states);

/*     ReturnCode_t
 *     notify_datareaders();
 */
OS_API DDS_ReturnCode_t
DDS_Subscriber_notify_datareaders (
    DDS_Subscriber _this);

/*     ReturnCode_t
 *     set_qos(
 *         in SubscriberQos qos);
 */
OS_API DDS_ReturnCode_t
DDS_Subscriber_set_qos (
    DDS_Subscriber _this,
    const DDS_SubscriberQos *qos);

/*     ReturnCode_t
 *     get_qos(
 *         inout SubscriberQos qos);
 */
OS_API DDS_ReturnCode_t
DDS_Subscriber_get_qos (
    DDS_Subscriber _this,
    DDS_SubscriberQos *qos);

/*     ReturnCode_t
 *     set_listener(
 *         in SubscriberListener a_listener,
 *         in StatusMask mask);
 */
OS_API DDS_ReturnCode_t
DDS_Subscriber_set_listener (
    DDS_Subscriber _this,
    const struct DDS_SubscriberListener *a_listener,
    const DDS_StatusMask mask);

/*     SubscriberListener
 *     get_listener();
 */
OS_API struct DDS_SubscriberListener
DDS_Subscriber_get_listener (
    DDS_Subscriber _this);

/*     ReturnCode_t
 *     begin_access();
 */
OS_API DDS_ReturnCode_t
DDS_Subscriber_begin_access (
    DDS_Subscriber _this);

/*     ReturnCode_t
 *     end_access();
 */
OS_API DDS_ReturnCode_t
DDS_Subscriber_end_access (
    DDS_Subscriber _this);

/*     DomainParticipant
 *     get_participant();
 */
OS_API DDS_DomainParticipant
DDS_Subscriber_get_participant (
    DDS_Subscriber _this);

/*     ReturnCode_t
 *     set_default_datareader_qos(
 *         in DataReaderQos qos);
 */
OS_API DDS_ReturnCode_t
DDS_Subscriber_set_default_datareader_qos (
    DDS_Subscriber _this,
    const DDS_DataReaderQos *qos);

/*     ReturnCode_t
 *     get_default_datareader_qos(
 *         inout DataReaderQos qos);
 */
OS_API DDS_ReturnCode_t
DDS_Subscriber_get_default_datareader_qos (
    DDS_Subscriber _this,
    DDS_DataReaderQos *qos);

/*     ReturnCode_t
 *     copy_from_topic_qos(
 *         inout DataReaderQos a_datareader_qos,
 *         in TopicQos a_topic_qos);
 */
OS_API DDS_ReturnCode_t
DDS_Subscriber_copy_from_topic_qos (
    DDS_Subscriber _this,
    DDS_DataReaderQos *a_datareader_qos,
    const DDS_TopicQos *a_topic_qos);

/*
 * interface DataReader : Entity
 */

/*
 * // Abstract methods
 *
 * ReturnCode_t
 * read(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in SampleStateMask sample_states,
 *     in ViewStateMask view_states,
 *     in InstanceStateMask instance_states);
 *
 * ReturnCode_t
 * take(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in SampleStateMask sample_states,
 *     in ViewStateMask view_states,
 *     in InstanceStateMask instance_states);
 *
 * ReturnCode_t
 * read_w_condition(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in ReadCondition a_condition);
 *
 * ReturnCode_t
 * take_w_condition(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in ReadCondition a_condition);
 *
 * ReturnCode_t
 * read_next_sample(
 *     inout Data data_values,
 *     inout SampleInfo sample_info);
 *
 * ReturnCode_t
 * take_next_sample(
 *     inout Data data_values,
 *     inout SampleInfo sample_info);
 *
 * ReturnCode_t
 * read_instance(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in InstanceHandle_t a_handle,
 *     in SampleStateMask sample_states,
 *     in ViewStateMask view_states,
 *     in InstanceStateMask instance_states);
 *
 * ReturnCode_t
 * take_instance(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in InstanceHandle_t a_handle,
 *     in SampleStateMask sample_states,
 *     in ViewStateMask view_states,
 *     in InstanceStateMask instance_states);
 *
 * ReturnCode_t
 * read_next_instance(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in InstanceHandle_t a_handle,
 *     in SampleStateMask sample_states,
 *     in ViewStateMask view_states,
 *     in InstanceStateMask instance_states);
 *
 * ReturnCode_t
 * take_next_instance(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in InstanceHandle_t a_handle,
 *     in SampleStateMask sample_states,
 *     in ViewStateMask view_states,
 *     in InstanceStateMask instance_states);
 *
 * ReturnCode_t
 * read_next_instance_w_condition(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in InstanceHandle_t a_handle,
 *     in ReadCondition a_condition);
 *
 * ReturnCode_t
 * take_next_instance_w_condition(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in InstanceHandle_t a_handle,
 *     in ReadCondition a_condition);
 *
 * ReturnCode_t
 * return_loan(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq);
 *
 * ReturnCode_t
 * get_key_value(
 *     inout Data key_holder,
 *     in InstanceHandle_t handle);
 */

/*  From Entity
 *     enable
 */
#define DDS_DataReader_enable DDS_Entity_enable

/*  From Entity
 *     get_statuscondition
 */
#define DDS_DataReader_get_statuscondition DDS_Entity_get_statuscondition

/*  From Entity
 *     get_status_changes
 */
#define DDS_DataReader_get_status_changes DDS_Entity_get_status_changes

/*  From Entity
 *     get_instance_handle
 */
#define DDS_DataReader_get_instance_handle DDS_Entity_get_instance_handle

/*     ReadCondition
 *     create_readcondition(
 *         in SampleStateMask sample_states,
 *         in ViewStateMask view_states,
 *         in InstanceStateMask instance_states);
 */
OS_API DDS_ReadCondition
DDS_DataReader_create_readcondition (
    DDS_DataReader _this,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states);

/*     QueryCondition
 *     create_querycondition(
 *         in SampleStateMask sample_states,
 *         in ViewStateMask view_states,
 *         in InstanceStateMask instance_states,
 *         in string query_expression,
 *         in StringSeq query_parameters);
 */
OS_API DDS_QueryCondition
DDS_DataReader_create_querycondition (
    DDS_DataReader _this,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states,
    const DDS_char *query_expression,
    const DDS_StringSeq *query_parameters);

/*     ReturnCode_t
 *     delete_readcondition(
 *         in ReadCondition a_condition);
 */
OS_API DDS_ReturnCode_t
DDS_DataReader_delete_readcondition (
    DDS_DataReader _this,
    const DDS_ReadCondition a_condition);

/*     ReturnCode_t
 *     delete_contained_entities();
 */
OS_API DDS_ReturnCode_t
DDS_DataReader_delete_contained_entities (
    DDS_DataReader _this);

/*     DataReaderView
  *     create_view (
  *     in DataReaderViewQos * qos);
 */
OS_API DDS_DataReaderView
DDS_DataReader_create_view (
    DDS_DataReader _this,
    const DDS_DataReaderViewQos * qos);

/*     ReturnCode_t
 *     delete_view(
 *        in DataReaderView a_view);
 */
OS_API DDS_ReturnCode_t
DDS_DataReader_delete_view (
    DDS_DataReader _this,
    const DDS_DataReaderView a_view);

/*     ReturnCode_t
 *     set_qos(
 *         in DataReaderQos qos);
 */
OS_API DDS_ReturnCode_t
DDS_DataReader_set_qos (
    DDS_DataReader _this,
    const DDS_DataReaderQos *qos);

/*     ReturnCode_t
 *     get_qos(
 *         inout DataReaderQos qos);
 */
OS_API DDS_ReturnCode_t
DDS_DataReader_get_qos (
    DDS_DataReader _this,
    DDS_DataReaderQos *qos);

/*     ReturnCode_t
 *     set_listener(
 *         in DataReaderListener a_listener,
 *         in StatusMask mask);
 */
OS_API DDS_ReturnCode_t
DDS_DataReader_set_listener (
    DDS_DataReader _this,
    const struct DDS_DataReaderListener *a_listener,
    const DDS_StatusMask mask);

/*     DataReaderListener
 *     get_listener();
 */
OS_API struct DDS_DataReaderListener
DDS_DataReader_get_listener (
    DDS_DataReader _this);

/*     TopicDescription
 *     get_topicdescription();
 */
OS_API DDS_TopicDescription
DDS_DataReader_get_topicdescription (
    DDS_DataReader _this);

/*     Subscriber
 *     get_subscriber();
 */
OS_API DDS_Subscriber
DDS_DataReader_get_subscriber (
    DDS_DataReader _this);

/* ReturnCode_t
 *get_sample_rejected_status(
 *inout SampleRejectedStatus status);
 */
OS_API DDS_ReturnCode_t
DDS_DataReader_get_sample_rejected_status (
    DDS_DataReader _this,
    DDS_SampleRejectedStatus *status);
/* ReturnCode_t
 *get_liveliness_changed_status(
 *inout LivelinessChangedStatus status);
*/
OS_API DDS_ReturnCode_t
DDS_DataReader_get_liveliness_changed_status (
    DDS_DataReader _this,
    DDS_LivelinessChangedStatus *status);
/* ReturnCode_t
 *get_requested_deadline_missed_status(
 *inout RequestedDeadlineMissedStatus status);
*/
OS_API DDS_ReturnCode_t
DDS_DataReader_get_requested_deadline_missed_status (
    DDS_DataReader _this,
    DDS_RequestedDeadlineMissedStatus *status);
/* ReturnCode_t
 *get_requested_incompatible_qos_status(
 *inout RequestedIncompatibleQosStatus status);
*/
OS_API DDS_ReturnCode_t
DDS_DataReader_get_requested_incompatible_qos_status (
    DDS_DataReader _this,
    DDS_RequestedIncompatibleQosStatus *status);
/* ReturnCode_t
 *get_sample_lost_status(
 *inout SampleLostStatus status);
*/
OS_API DDS_ReturnCode_t
DDS_DataReader_get_sample_lost_status (
    DDS_DataReader _this,
    DDS_SampleLostStatus *status);

/* ReturnCode_t
 * get_subscription_matched_status(
 *      inout SubscriptionMatchedStatus status);
 */
OS_API DDS_ReturnCode_t
DDS_DataReader_get_subscription_matched_status (
    DDS_DataReader _this,
    DDS_SubscriptionMatchedStatus *status);

/*     ReturnCode_t
 *     wait_for_historical_data(
 *         in Duration_t max_wait);
 */
OS_API DDS_ReturnCode_t
DDS_DataReader_wait_for_historical_data (
    DDS_DataReader _this,
    const DDS_Duration_t *max_wait);

OS_API DDS_ReturnCode_t
DDS_DataReader_wait_for_historical_data_w_condition (
    DDS_DataReader _this,
    const DDS_char *filter_expression,
    const DDS_StringSeq *filter_parameters,
    const DDS_Time_t *min_source_timestamp,
    const DDS_Time_t *max_source_timestamp,
    const DDS_ResourceLimitsQosPolicy *resource_limits,
    const DDS_Duration_t *max_wait);

/*     ReturnCode_t get_matched_publications(
 *     inout InstanceHandleSeq publication_handles);
 */
OS_API DDS_ReturnCode_t
DDS_DataReader_get_matched_publications (
    DDS_DataReader _this,
    DDS_InstanceHandleSeq *publication_handles);

/*     ReturnCode_t
 *     get_matched_publication_data(
 *         inout PublicationBuiltinTopicData publication_data,
 *         in InstanceHandle_t publication_handle);
 */
OS_API DDS_ReturnCode_t
DDS_DataReader_get_matched_publication_data (
    DDS_DataReader _this,
    DDS_PublicationBuiltinTopicData *publication_data,
    const DDS_InstanceHandle_t publication_handle);

/*     ReturnCode_t
 *     set_default_datareaderview_qos(
 *         in DataReaderViewQos qos);
 */
OS_API DDS_ReturnCode_t
DDS_DataReader_set_default_datareaderview_qos (
    DDS_DataReader _this,
    const DDS_DataReaderViewQos *qos);

/*     ReturnCode_t
 *     get_default_datareaderview_qos(
 *         inout DataReaderViewQos qos);
 */
OS_API DDS_ReturnCode_t
DDS_DataReader_get_default_datareaderview_qos (
    DDS_DataReader _this,
    DDS_DataReaderViewQos *qos);

/*
 * interface DataReaderView : Entity
 */

/*
 * // Abstract methods
 *
 * ReturnCode_t
 * read(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in SampleStateMask sample_states,
 *     in ViewStateMask view_states,
 *     in InstanceStateMask instance_states);
 *
 * ReturnCode_t
 * take(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in SampleStateMask sample_states,
 *     in ViewStateMask view_states,
 *     in InstanceStateMask instance_states);
 *
 * ReturnCode_t
 * read_next_sample(
 *     inout Data data_values,
 *     inout SampleInfo sample_info);
 *
 * ReturnCode_t
 * take_next_sample(
 *     inout Data data_values,
 *     inout SampleInfo sample_info);
 *
 * ReturnCode_t
 * read_instance(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in InstanceHandle_t a_handle,
 *     in SampleStateMask sample_states,
 *     in ViewStateMask view_states,
 *     in InstanceStateMask instance_states);
 *
 * ReturnCode_t
 * take_instance(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in InstanceHandle_t a_handle,
 *     in SampleStateMask sample_states,
 *     in ViewStateMask view_states,
 *     in InstanceStateMask instance_states);
 *
 * ReturnCode_t
 * read_next_instance(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in InstanceHandle_t a_handle,
 *     in SampleStateMask sample_states,
 *     in ViewStateMask view_states,
 *     in InstanceStateMask instance_states);
 *
 * ReturnCode_t
 * take_next_instance(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in InstanceHandle_t a_handle,
 *     in SampleStateMask sample_states,
 *     in ViewStateMask view_states,
 *     in InstanceStateMask instance_states);
 *
 * ReturnCode_t
 * return_loan(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq);
 *
 */

/*  From Entity
 *     enable
 */
#define DDS_DataReaderView_enable DDS_Entity_enable

/*  From Entity
 *     get_instance_handle
 */
#define DDS_DataReaderView_get_instance_handle DDS_Entity_get_instance_handle

/*  From Entity
 *     get_statuscondition
 */
OS_API DDS_StatusCondition
DDS_DataReaderView_get_statuscondition(
    DDS_DataReaderView _this);

/*  From Entity
 *     enable
 */
OS_API DDS_StatusMask
DDS_DataReaderView_get_status_changes (
    DDS_DataReaderView _this);

/*     ReturnCode_t
 *     set_qos(
 *         in DataReaderViewQos qos);
 */
OS_API DDS_ReturnCode_t
DDS_DataReaderView_set_qos (
    DDS_DataReaderView _this,
    const DDS_DataReaderViewQos *qos);

/*     ReturnCode_t
 *     get_qos(
 *         inout DataReaderViewQos qos);
 */
OS_API DDS_ReturnCode_t
DDS_DataReaderView_get_qos (
    DDS_DataReaderView _this,
    DDS_DataReaderViewQos *qos);

/*     DataReader
 *     get_datareader();
 */
OS_API DDS_DataReader
DDS_DataReaderView_get_datareader (
    DDS_DataReaderView _this);

/*     ReadCondition
 *     create_readcondition(
 *         in SampleStateMask sample_states,
 *         in ViewStateMask view_states,
 *         in InstanceStateMask instance_states);
 */
OS_API DDS_ReadCondition
DDS_DataReaderView_create_readcondition (
    DDS_DataReaderView _this,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states);

/*     QueryCondition
 *     create_querycondition(
 *         in SampleStateMask sample_states,
 *         in ViewStateMask view_states,
 *         in InstanceStateMask instance_states,
 *         in string query_expression,
 *         in StringSeq query_parameters);
 */
OS_API DDS_QueryCondition
DDS_DataReaderView_create_querycondition (
    DDS_DataReaderView _this,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states,
    const DDS_char *query_expression,
    const DDS_StringSeq *query_parameters);

/*     ReturnCode_t
 *     delete_readcondition(
 *         in ReadCondition a_condition);
 */
OS_API DDS_ReturnCode_t
DDS_DataReaderView_delete_readcondition (
    DDS_DataReaderView _this,
    const DDS_ReadCondition a_condition);

/*     ReturnCode_t
 *     delete_contained_entities();
 */
OS_API DDS_ReturnCode_t
DDS_DataReaderView_delete_contained_entities (
    DDS_DataReaderView _this);

/*
 * struct SampleInfo {
 *     SampleStateKind sample_state;
 *     ViewStateKind view_state;
 *     InstanceStateKind instance_state;
 *     boolean valid_data;
 *     Time_t source_timestamp;
 *     InstanceHandle_t instance_handle;
 *     long disposed_generation_count;
 *     long no_writers_generation_count;
 *     long sample_rank;
 *     long generation_rank;
 *     long absolute_generation_rank;
 * };
 */
typedef struct DDS_SampleInfo_s DDS_SampleInfo;
struct DDS_SampleInfo_s {
    DDS_SampleStateKind sample_state;
    DDS_ViewStateKind view_state;
    DDS_InstanceStateKind instance_state;
    DDS_boolean valid_data;
    DDS_Time_t source_timestamp;
    DDS_InstanceHandle_t instance_handle;
    DDS_InstanceHandle_t publication_handle;
    DDS_long disposed_generation_count;
    DDS_long no_writers_generation_count;
    DDS_long sample_rank;
    DDS_long generation_rank;
    DDS_long absolute_generation_rank;
    DDS_Time_t arrival_timestamp;
};

/*
 * typedef sequence<SampleInfo> SampleInfoSeq;
 */
#ifndef _DDS_sequence_DDS_SampleInfo_defined
#define _DDS_sequence_DDS_SampleInfo_defined
typedef struct {
    DDS_unsigned_long _maximum;
    DDS_unsigned_long _length;
    DDS_SampleInfo *_buffer;
    DDS_boolean _release;
} DDS_sequence_DDS_SampleInfo;

OS_API DDS_sequence_DDS_SampleInfo *DDS_sequence_DDS_SampleInfo__alloc (void);
OS_API DDS_SampleInfo *DDS_sequence_DDS_SampleInfo_allocbuf (DDS_unsigned_long len);
#endif /* _DDS_sequence_DDS_SampleInfo_defined */

typedef DDS_sequence_DDS_SampleInfo DDS_SampleInfoSeq;
OS_API DDS_SampleInfoSeq *DDS_SampleInfoSeq__alloc (void);
OS_API DDS_SampleInfo *DDS_SampleInfoSeq_allocbuf (DDS_unsigned_long len);

#if 1
/*
 * The following FooTypeSupport operations are not for
 * use by the application programmer, these interfaces
 * are used by the generated Application Typed code.
 */

/* ReturnCode_t
 * register_type(
 *     in DomainParticipant domain,
 *     in string type_name);
 */
OS_API DDS_ReturnCode_t
DDS__FooTypeSupport_register_type (
    DDS_TypeSupport _this,
    const DDS_DomainParticipant domain,
    const DDS_string name);

typedef void *(*DDS_typeSupportLoad)(void *);
typedef void (*DDS_copyIn)(void *, void *, void *);
typedef void (*DDS_copyOut)(void *, void *);
typedef void *(*DDS_typeSupportAllocBuffer)(DDS_unsigned_long len);

OS_API DDS_TypeSupport
DDS__FooTypeSupport__alloc (
    const DDS_char *type_name,
    const DDS_char *type_keys,
    const DDS_char *type_def,
    DDS_typeSupportLoad type_load,
    const DDS_copyIn copy_in,
    const DDS_copyOut copy_out,
    const DDS_unsigned_long alloc_size,
    const DDS_typeSupportAllocBuffer alloc_buffer);
#endif

OS_API DDS_string
DDS__FooTypeSupport_get_type_name (
    DDS_TypeSupport _this);

OS_API void *
DDS__FooTypeSupport_allocbuf (
    DDS_TypeSupport _this,
    DDS_unsigned_long len);

/*
 * The following FooDataWriter operations are not for
 * use by the application programmer, these interfaces
 * are used by the generated Application Typed code.
 */

typedef void *        DDS_sample;
typedef void *        DDS_sequence;

/*  From Entity
 *     enable
 */
#define DDS__FooDataWriter_enable DDS_Entity_enable

/*  From Entity
 *     get_status_changes
 */
#define DDS__FooDataWriter_get_status_changes DDS_Entity_get_status_changes

/*  From Entity
 *     get_statuscondition
 */
#define DDS__FooDataWriter_get_statuscondition DDS_Entity_get_statuscondition

/*  From Entity
 *     get_instance_handle
 */
#define DDS__FooDataWriter_get_instance_handle DDS_Entity_get_instance_handle

/*  From DataWriter
 *     assert_liveliness
 */
#define DDS__FooDataWriter_assert_liveliness DDS_DataWriter_assert_liveliness

/*  From DataWriter
 *     get_listener
 */
#define DDS__FooDataWriter_get_listener DDS_DataWriter_get_listener

/*  From DataWriter
 *     get_liveliness_lost_status
 */
#define DDS__FooDataWriter_get_liveliness_lost_status DDS_DataWriter_get_liveliness_lost_status

/*  From DataWriter
 *     get_matched_subscription_data
 */
#define DDS__FooDataWriter_get_matched_subscription_data DDS_DataWriter_get_matched_subscription_data

/*  From DataWriter
 *     get_matched_subscriptions
 */
#define DDS__FooDataWriter_get_matched_subscriptions DDS_DataWriter_get_matched_subscriptions

/*  From DataWriter
 *     get_offered_deadline_missed_status
 */
#define DDS__FooDataWriter_get_offered_deadline_missed_status DDS_DataWriter_get_offered_deadline_missed_status

/*  From DataWriter
 *     get_offered_incompatible_qos_status
 */
#define DDS__FooDataWriter_get_offered_incompatible_qos_status DDS_DataWriter_get_offered_incompatible_qos_status

/*  From DataWriter
 *     get_publication_match_status
 */
#define DDS__FooDataWriter_get_publication_matched_status DDS_DataWriter_get_publication_matched_status

/*  From DataWriter
 *     get_publisher
 */
#define DDS__FooDataWriter_get_publisher DDS_DataWriter_get_publisher

/*  From DataWriter
 *     get_qos
 */
#define DDS__FooDataWriter_get_qos DDS_DataWriter_get_qos

/*  From DataWriter
 *     get_topic
 */
#define DDS__FooDataWriter_get_topic DDS_DataWriter_get_topic

/*  From DataWriter
 *     set_listener
 */
#define DDS__FooDataWriter_set_listener DDS_DataWriter_set_listener

/*  From DataWriter
 *     set_qos
 */
#define DDS__FooDataWriter_set_qos DDS_DataWriter_set_qos

/* InstanceHandle_t
 * register_instance(
 *     in Data instance_data);
 */
OS_API DDS_InstanceHandle_t
DDS__FooDataWriter_register_instance (
    DDS_DataWriter _this,
    const DDS_sample instance_data);

/* InstanceHandle_t
 * register_instance_w_timestamp(
 *    in Data instance_data,
 *     in Time_t source_timestamp);
 */
OS_API DDS_InstanceHandle_t
DDS__FooDataWriter_register_instance_w_timestamp (
    DDS_DataWriter _this,
    const DDS_sample instance_data,
    const DDS_Time_t *source_timestamp);

/* ReturnCode_t
 * unregister_instance(
 *     in Data instance_data,
 *     in InstanceHandle_t handle);
 */
OS_API DDS_ReturnCode_t
DDS__FooDataWriter_unregister_instance (
    DDS_DataWriter _this,
    const DDS_sample instance_data,
    const DDS_InstanceHandle_t handle);

/* ReturnCode_t
 * unregister_instance_w_timestamp(
 *     in Data instance_data,
 *     in InstanceHandle_t handle,
 *     in Time_t source_timestamp);
 */
OS_API DDS_ReturnCode_t
DDS__FooDataWriter_unregister_instance_w_timestamp (
    DDS_DataWriter _this,
    const DDS_sample instance_data,
    const DDS_InstanceHandle_t handle,
    const DDS_Time_t *source_timestamp);

/* ReturnCode_t
 * write(
 *     in Data instance_data,
 *     in InstanceHandle_t handle);
 */
OS_API DDS_ReturnCode_t
DDS__FooDataWriter_write (
    DDS_DataWriter _this,
    const DDS_sample instance_data,
    const DDS_InstanceHandle_t handle);

/* ReturnCode_t
 * write_w_timestamp(
 *     in Data instance_data,
 *     in InstanceHandle_t handle,
 *     in Time_t source_timestamp);
 */
OS_API DDS_ReturnCode_t
DDS__FooDataWriter_write_w_timestamp (
    DDS_DataWriter _this,
    const DDS_sample instance_data,
    const DDS_InstanceHandle_t handle,
    const DDS_Time_t *source_timestamp);

/* ReturnCode_t
 * dispose(
 *     in Data instance_data,
 *     in InstanceHandle_t instance_handle);
 */
OS_API DDS_ReturnCode_t
DDS__FooDataWriter_dispose (
    DDS_DataWriter _this,
    const DDS_sample instance_data,
    const DDS_InstanceHandle_t instance_handle);

/* ReturnCode_t
 * dispose_w_timestamp(
 *     in Data instance_data,
 *     in InstanceHandle_t instance_handle,
 *     in Time_t source_timestamp);
 */
OS_API DDS_ReturnCode_t
DDS__FooDataWriter_dispose_w_timestamp (
    DDS_DataWriter _this,
    const DDS_sample instance_data,
    const DDS_InstanceHandle_t instance_handle,
    const DDS_Time_t *source_timestamp);

/* ReturnCode_t
 * writedispose(
 *     in Data instance_data,
 *     in InstanceHandle_t instance_handle);
 */
OS_API DDS_ReturnCode_t
DDS__FooDataWriter_writedispose (
    DDS_DataWriter _this,
    const DDS_sample instance_data,
    const DDS_InstanceHandle_t instance_handle);

/* ReturnCode_t
 * writedispose_w_timestamp(
 *     in Data instance_data,
 *     in InstanceHandle_t instance_handle,
 *     in Time_t source_timestamp);
 */
OS_API DDS_ReturnCode_t
DDS__FooDataWriter_writedispose_w_timestamp (
    DDS_DataWriter _this,
    const DDS_sample instance_data,
    const DDS_InstanceHandle_t instance_handle,
    const DDS_Time_t *source_timestamp);

/* ReturnCode_t
 * get_key_value(
 *     inout Data key_holder,
 *     in InstanceHandle_t handle);
 */
OS_API DDS_ReturnCode_t
DDS__FooDataWriter_get_key_value (
    DDS_DataWriter _this,
    DDS_sample key_holder,
    const DDS_InstanceHandle_t handle);

/* InstanceHandle_t
 *   lookup_instance(
 *       in Data instance_data);
 */
OS_API DDS_InstanceHandle_t
DDS__FooDataWriter_lookup_instance(
    DDS_DataWriter _this,
    const DDS_sample instance_data);

/*
 * The following FooDataReader operations are not for
 * use by the application programmer, these interfaces
 * are used by the generated Application Typed code.
 */

/*  From Entity
 *     enable
 */
#define DDS__FooDataReader_enable DDS_Entity_enable

/*  From Entity
 *     get_status_changes
 */
#define DDS__FooDataReader_get_status_changes DDS_Entity_get_status_changes

/*  From Entity
 *     get_statuscondition
 */
#define DDS__FooDataReader_get_statuscondition DDS_Entity_get_statuscondition

/*  From Entity
 *     get_instance_handle
 */
#define DDS__FooDataReader_get_instance_handle DDS_Entity_get_instance_handle

/*  From DataReader
 *     create_querycondition
 */
#define DDS__FooDataReader_create_querycondition DDS_DataReader_create_querycondition

/*  From DataReader
 *     create_readcondition
 */
#define DDS__FooDataReader_create_readcondition DDS_DataReader_create_readcondition

/*  From DataReader
 *     delete_contained_entities
 */
#define DDS__FooDataReader_delete_contained_entities DDS_DataReader_delete_contained_entities

/*     DataReaderView
  *     create_view
 */
#define DDS_FooDataReader_create_view DDS_DataReader_create_view

/*     ReturnCode_t
 *     delete_view
 */
#define DDS_FooDataReader_delete_view  DDS_DataReader_delete_view

/*  From DataReader
 *     delete_readcondition
 */
#define DDS__FooDataReader_delete_readcondition DDS_DataReader_delete_readcondition

/*  From DataReader
 *     get_listener
 */
#define DDS__FooDataReader_get_listener DDS_DataReader_get_listener

/*  From DataReader
 *     get_liveliness_changed_status
 */
#define DDS__FooDataReader_get_liveliness_changed_status DDS_DataReader_get_liveliness_changed_status

/*  From DataReader
 *     get_matched_publication_data
 */
#define DDS__FooDataReader_get_matched_publication_data DDS_DataReader_get_matched_publication_data

/*  From DataReader
 *     get_matched_publications
 */
#define DDS__FooDataReader_get_matched_publications DDS_DataReader_get_matched_publications

/*  From DataReader
 *     get_qos
 */
#define DDS__FooDataReader_get_qos DDS_DataReader_get_qos

/*  From DataReader
 *     get_requested_deadline_missed_status
 */
#define DDS__FooDataReader_get_requested_deadline_missed_status DDS_DataReader_get_requested_deadline_missed_status

/*  From DataReader
 *     get_requested_incompatible_qos_status
 */
#define DDS__FooDataReader_get_requested_incompatible_qos_status DDS_DataReader_get_requested_incompatible_qos_status

/*  From DataReader
 *     get_sample_lost_status
 */
#define DDS__FooDataReader_get_sample_lost_status DDS_DataReader_get_sample_lost_status

/*  From DataReader
 *     get_sample_rejected_status
 */
#define DDS__FooDataReader_get_sample_rejected_status DDS_DataReader_get_sample_rejected_status

/*  From DataReader
 *     get_subscriber
 */
#define DDS__FooDataReader_get_subscriber DDS_DataReader_get_subscriber

/*  From DataReader
 *     get_subscription_match_status
 */
#define DDS__FooDataReader_get_subscription_matched_status DDS_DataReader_get_subscription_matched_status

/*  From DataReader
 *     get_topicdescription
 */
#define DDS__FooDataReader_get_topicdescription DDS_DataReader_get_topicdescription

/*  From DataReader
 *     set_listener
 */
#define DDS__FooDataReader_set_listener DDS_DataReader_set_listener

/*  From DataReader
 *     set_qos
 */
#define DDS__FooDataReader_set_qos DDS_DataReader_set_qos

/*  From DataReader
 *     wait_for_historical_data
 */
#define DDS__FooDataReader_wait_for_historical_data DDS_DataReader_wait_for_historical_data

/* ReturnCode_t
 * read(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in SampleStateMask sample_states,
 *     in ViewStateMask view_states,
 *     in InstanceStateMask instance_states);
 */
OS_API DDS_ReturnCode_t
DDS__FooDataReader_read (
    DDS_DataReader _this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states);

/* ReturnCode_t
 * take(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in SampleStateMask sample_states,
 *     in ViewStateMask view_states,
 *     in InstanceStateMask instance_states);
 */
OS_API DDS_ReturnCode_t
DDS__FooDataReader_take (
    DDS_DataReader _this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states);

/* ReturnCode_t
 * read_w_condition(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in ReadCondition a_condition);
 */
OS_API DDS_ReturnCode_t
DDS__FooDataReader_read_w_condition (
    DDS_DataReader _this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_ReadCondition a_condition);

/* ReturnCode_t
 * take_w_condition(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in ReadCondition a_condition);
 */
OS_API DDS_ReturnCode_t
DDS__FooDataReader_take_w_condition (
    DDS_DataReader _this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_ReadCondition a_condition);

/* ReturnCode_t
 * read_next_sample(
 *     inout Data data_values,
 *     inout SampleInfo sample_info);
 */
OS_API DDS_ReturnCode_t
DDS__FooDataReader_read_next_sample (
    DDS_DataReader _this,
    DDS_sample data_values,
    DDS_SampleInfo *sample_info);

/* ReturnCode_t
 * take_next_sample(
 *     inout Data data_values,
 *     inout SampleInfo sample_info);
 */
OS_API DDS_ReturnCode_t
DDS__FooDataReader_take_next_sample (
    DDS_DataReader _this,
    DDS_sample data_values,
    DDS_SampleInfo *sample_info);

/* ReturnCode_t
 * read_instance(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in InstanceHandle_t a_handle,
 *     in SampleStateMask sample_states,
 *     in ViewStateMask view_states,
 *     in InstanceStateMask instance_states);
 */
OS_API DDS_ReturnCode_t
DDS__FooDataReader_read_instance (
    DDS_DataReader _this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states);

/* ReturnCode_t
 * take_instance(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in InstanceHandle_t a_handle,
 *     in SampleStateMask sample_states,
 *     in ViewStateMask view_states,
 *     in InstanceStateMask instance_states);
 */
OS_API DDS_ReturnCode_t
DDS__FooDataReader_take_instance (
    DDS_DataReader _this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states);

/* ReturnCode_t
 * read_next_instance(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in InstanceHandle_t a_handle,
 *     in SampleStateMask sample_states,
 *     in ViewStateMask view_states,
 *     in InstanceStateMask instance_states);
 */
OS_API DDS_ReturnCode_t
DDS__FooDataReader_read_next_instance (
    DDS_DataReader _this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states);

/* ReturnCode_t
 * take_next_instance(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in InstanceHandle_t a_handle,
 *     in SampleStateMask sample_states,
 *     in ViewStateMask view_states,
 *     in InstanceStateMask instance_states);
 */
OS_API DDS_ReturnCode_t
DDS__FooDataReader_take_next_instance (
    DDS_DataReader _this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states);

/* ReturnCode_t
 * read_next_instance_w_condition(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in InstanceHandle_t a_handle,
 *     in ReadCondition a_condition);
 */
OS_API DDS_ReturnCode_t
DDS__FooDataReader_read_next_instance_w_condition (
    DDS_DataReader _this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_ReadCondition a_condition);

/* ReturnCode_t
 * take_next_instance_w_condition(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in InstanceHandle_t a_handle,
 *     in ReadCondition a_condition);
 */
OS_API DDS_ReturnCode_t
DDS__FooDataReader_take_next_instance_w_condition (
    DDS_DataReader _this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_ReadCondition a_condition);

/* ReturnCode_t
 * return_loan(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq);
 */
OS_API DDS_ReturnCode_t
DDS__FooDataReader_return_loan (
    DDS_DataReader _this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq);

/* ReturnCode_t
 * get_key_value(
 *     inout Data key_holder,
 *     in InstanceHandle_t handle);
 */
OS_API DDS_ReturnCode_t
DDS__FooDataReader_get_key_value (
    DDS_DataReader _this,
    DDS_sample key_holder,
    const DDS_InstanceHandle_t handle);

/* InstanceHandle_t
 * lookup_instance (
 *     in Data instance_data);
 */
OS_API DDS_InstanceHandle_t
DDS__FooDataReader_lookup_instance (
    DDS_DataReader _this,
    const DDS_sample instance_data);

/*
 * The following FooDataReaderView operations are not for
 * use by the application programmer, these interfaces
 * are used by the generated Application Typed code.
 */

/*  From Entity
 *     enable
 */
#define DDS__FooDataReaderView_enable DDS_Entity_enable

/*  From Entity
 *     get_instance_handle
 */
#define DDS__FooDataReaderView_get_instance_handle DDS_Entity_get_instance_handle


/*  From DataReaderView
 *     get_qos
 */
#define DDS__FooDataReaderView_get_qos DDS_DataReaderView_get_qos

/*  From DataReaderView
 *     set_qos
 */
#define DDS__FooDataReaderView_set_qos DDS_DataReaderView_set_qos

/*  From DataReaderView
 *     get_subscriber
 */
#define DDS__FooDataReaderView_get_datareader DDS_DataReaderView_get_datareader

/*  From DataReaderView
 *     create_querycondition
 */
#define DDS__FooDataReaderView_create_querycondition DDS_DataReaderView_create_querycondition

/*  From DataReaderView
 *     create_readcondition
 */
#define DDS__FooDataReaderView_create_readcondition DDS_DataReaderView_create_readcondition

/*  From DataReaderView
 *     delete_contained_entities
 */
#define DDS__FooDataReaderView_delete_contained_entities DDS_DataReaderView_delete_contained_entities

/*  From DataReaderView
 *     delete_readcondition
 */
#define DDS__FooDataReaderView_delete_readcondition DDS_DataReaderView_delete_readcondition


/* ReturnCode_t
 * read(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in sampleStateMask sample_states,
 *     in viewStateMask view_states,
 *     in instanceStateMask instance_states);
 */
OS_API DDS_ReturnCode_t
DDS__FooDataReaderView_read (
    DDS_DataReaderView _this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states);

/* ReturnCode_t
 * take(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in sampleStateMask sample_states,
 *     in viewStateMask view_states,
 *     in instanceStateMask instance_states);
 */
OS_API DDS_ReturnCode_t
DDS__FooDataReaderView_take (
    DDS_DataReaderView _this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states);


/* ReturnCode_t
 * read_next_sample(
 *     inout Data data_values,
 *     inout SampleInfo sample_info);
 */
OS_API DDS_ReturnCode_t
DDS__FooDataReaderView_read_next_sample (
    DDS_DataReaderView _this,
    DDS_sample data_values,
    DDS_SampleInfo *sample_info);

/* ReturnCode_t
 * take_next_sample(
 *     inout Data data_values,
 *     inout SampleInfo sample_info);
 */
OS_API DDS_ReturnCode_t
DDS__FooDataReaderView_take_next_sample (
    DDS_DataReaderView _this,
    DDS_sample data_values,
    DDS_SampleInfo *sample_info);

/* ReturnCode_t
 * read_instance(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in InstanceHandle_t a_handle,
 *     in sampleStateMask sample_states,
 *     in viewStateMask view_states,
 *     in instanceStateMask instance_states);
 */
OS_API DDS_ReturnCode_t
DDS__FooDataReaderView_read_instance (
    DDS_DataReaderView _this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states);

/* ReturnCode_t
 * take_instance(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in InstanceHandle_t a_handle,
 *     in sampleStateMask sample_states,
 *     in viewStateMask view_states,
 *     in instanceStateMask instance_states);
 */
OS_API DDS_ReturnCode_t
DDS__FooDataReaderView_take_instance (
    DDS_DataReaderView _this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states);

/* ReturnCode_t
 * read_next_instance(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in InstanceHandle_t a_handle,
 *     in sampleStateMask sample_states,
 *     in viewStateMask view_states,
 *     in instanceStateMask instance_states);
 */
OS_API DDS_ReturnCode_t
DDS__FooDataReaderView_read_next_instance (
    DDS_DataReaderView _this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states);

/* ReturnCode_t
 * take_next_instance(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in InstanceHandle_t a_handle,
 *     in sampleStateMask sample_states,
 *     in viewStateMask view_states,
 *     in instanceStateMask instance_states);
 */
OS_API DDS_ReturnCode_t
DDS__FooDataReaderView_take_next_instance (
    DDS_DataReaderView _this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states);


/* ReturnCode_t
 * read_w_condition(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in ReadCondition a_condition);
 */
OS_API DDS_ReturnCode_t
DDS__FooDataReaderView_read_w_condition (
    DDS_DataReaderView _this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_ReadCondition a_condition);

/* ReturnCode_t
 * take_w_condition(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in ReadCondition a_condition);
 */
OS_API DDS_ReturnCode_t
DDS__FooDataReaderView_take_w_condition (
    DDS_DataReaderView _this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_ReadCondition a_condition);

/* ReturnCode_t
 * read_next_instance_w_condition(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in InstanceHandle_t a_handle,
 *     in ReadCondition a_condition);
 */
OS_API DDS_ReturnCode_t
DDS__FooDataReaderView_read_next_instance_w_condition (
    DDS_DataReaderView _this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_ReadCondition a_condition);

/* ReturnCode_t
 * take_next_instance_w_condition(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in InstanceHandle_t a_handle,
 *     in ReadCondition a_condition);
 */
OS_API DDS_ReturnCode_t
DDS__FooDataReaderView_take_next_instance_w_condition (
    DDS_DataReaderView _this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_ReadCondition a_condition);

/* ReturnCode_t
 * return_loan(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq);
 */
OS_API DDS_ReturnCode_t
DDS__FooDataReaderView_return_loan (
    DDS_DataReaderView _this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq);

/* ReturnCode_t
 * get_key_value(
 *     inout Data key_holder,
 *     in InstanceHandle_t handle);
 */
OS_API DDS_ReturnCode_t
DDS__FooDataReaderView_get_key_value (
    DDS_DataReaderView _this,
    DDS_sample key_holder,
    const DDS_InstanceHandle_t handle);

/* InstanceHandle_t
 * lookup_instance (
 *     in Data instance_data);
 */
OS_API DDS_InstanceHandle_t
DDS__FooDataReaderView_lookup_instance (
    DDS_DataReaderView _this,
    DDS_sample instance_data);

/*
 * Builtin Topic pre-defined DataReader interfaces
 */

/*
 * ParticipantBuiltinTopicData
 */
OS_API DDS_ReturnCode_t
DDS_ParticipantBuiltinTopicDataDataReader_read (
    DDS_DataReader _this,
    DDS_sequence_DDS_ParticipantBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states);

OS_API DDS_ReturnCode_t
DDS_ParticipantBuiltinTopicDataDataReader_take (
    DDS_DataReader _this,
    DDS_sequence_DDS_ParticipantBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states);

#define DDS_ParticipantBuiltinTopicDataDataReader DDS_DataReader

#define DDS_ParticipantBuiltinTopicDataDataReader_enable DDS_Entity_enable

#define DDS_ParticipantBuiltinTopicDataDataReader_get_status_changes DDS_Entity_get_status_changes

#define DDS_ParticipantBuiltinTopicDataDataReader_get_statuscondition DDS_Entity_get_statuscondition

#define DDS_ParticipantBuiltinTopicDataDataReader_get_instance_handle DDS_Entity_get_instance_handle

#define DDS_ParticipantBuiltinTopicDataDataReader_create_querycondition DDS_DataReader_create_querycondition

#define DDS_ParticipantBuiltinTopicDataDataReader_create_readcondition DDS_DataReader_create_readcondition

#define DDS_ParticipantBuiltinTopicDataDataReader_delete_contained_entities DDS_DataReader_delete_contained_entities

#define DDS_ParticipantBuiltinTopicDataDataReader_delete_readcondition DDS_DataReader_delete_readcondition

#define DDS_ParticipantBuiltinTopicDataDataReader_get_listener DDS_DataReader_get_listener

#define DDS_ParticipantBuiltinTopicDataDataReader_get_liveliness_changed_status DDS_DataReader_get_liveliness_changed_status

#define DDS_ParticipantBuiltinTopicDataDataReader_get_matched_publication_data DDS_DataReader_get_matched_publication_data

#define DDS_ParticipantBuiltinTopicDataDataReader_get_matched_publications DDS_DataReader_get_matched_publications

#define DDS_ParticipantBuiltinTopicDataDataReader_get_qos DDS_DataReader_get_qos

#define DDS_ParticipantBuiltinTopicDataDataReader_get_requested_deadline_missed_status DDS_DataReader_get_requested_deadline_missed_status

#define DDS_ParticipantBuiltinTopicDataDataReader_get_requested_incompatible_qos_status DDS_DataReader_get_requested_incompatible_qos_status

#define DDS_ParticipantBuiltinTopicDataDataReader_get_sample_lost_status DDS_DataReader_get_sample_lost_status

#define DDS_ParticipantBuiltinTopicDataDataReader_get_sample_rejected_status DDS_DataReader_get_sample_rejected_status

#define DDS_ParticipantBuiltinTopicDataDataReader_get_subscriber DDS_DataReader_get_subscriber

#define DDS_ParticipantBuiltinTopicDataDataReader_get_subscription_matched_status DDS_DataReader_get_subscription_matched_status

#define DDS_ParticipantBuiltinTopicDataDataReader_get_topicdescription DDS_DataReader_get_topicdescription

#define DDS_ParticipantBuiltinTopicDataDataReader_set_listener DDS_DataReader_set_listener

#define DDS_ParticipantBuiltinTopicDataDataReader_set_qos DDS_DataReader_set_qos

#define DDS_ParticipantBuiltinTopicDataDataReader_wait_for_historical_data DDS_DataReader_wait_for_historical_data

OS_API DDS_ReturnCode_t
DDS_ParticipantBuiltinTopicDataDataReader_read_w_condition (
    DDS_DataReader _this,
    DDS_sequence_DDS_ParticipantBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_ReadCondition a_condition);

OS_API DDS_ReturnCode_t
DDS_ParticipantBuiltinTopicDataDataReader_take_w_condition (
    DDS_DataReader _this,
    DDS_sequence_DDS_ParticipantBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_ReadCondition a_condition);

OS_API DDS_ReturnCode_t
DDS_ParticipantBuiltinTopicDataDataReader_read_next_sample (
    DDS_DataReader _this,
    DDS_ParticipantBuiltinTopicData *data_values,
    DDS_SampleInfo *sample_info);

OS_API DDS_ReturnCode_t
DDS_ParticipantBuiltinTopicDataDataReader_take_next_sample (
    DDS_DataReader _this,
    DDS_ParticipantBuiltinTopicData *data_values,
    DDS_SampleInfo *sample_info);

OS_API DDS_ReturnCode_t
DDS_ParticipantBuiltinTopicDataDataReader_read_instance (
    DDS_DataReader _this,
    DDS_sequence_DDS_ParticipantBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states);

OS_API DDS_ReturnCode_t
DDS_ParticipantBuiltinTopicDataDataReader_take_instance (
    DDS_DataReader _this,
    DDS_sequence_DDS_ParticipantBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states);

OS_API DDS_ReturnCode_t
DDS_ParticipantBuiltinTopicDataDataReader_read_next_instance (
    DDS_DataReader _this,
    DDS_sequence_DDS_ParticipantBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states);

OS_API DDS_ReturnCode_t
DDS_ParticipantBuiltinTopicDataDataReader_take_next_instance (
    DDS_DataReader _this,
    DDS_sequence_DDS_ParticipantBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states);

OS_API DDS_ReturnCode_t
DDS_ParticipantBuiltinTopicDataDataReader_read_next_instance_w_condition (
    DDS_DataReader _this,
    DDS_sequence_DDS_ParticipantBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_ReadCondition a_condition);

OS_API DDS_ReturnCode_t
DDS_ParticipantBuiltinTopicDataDataReader_take_next_instance_w_condition (
    DDS_DataReader _this,
    DDS_sequence_DDS_ParticipantBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_ReadCondition a_condition);

OS_API DDS_ReturnCode_t
DDS_ParticipantBuiltinTopicDataDataReader_return_loan (
    DDS_DataReader _this,
    DDS_sequence_DDS_ParticipantBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq);

OS_API DDS_ReturnCode_t
DDS_ParticipantBuiltinTopicDataDataReader_get_key_value (
    DDS_DataReader _this,
    DDS_ParticipantBuiltinTopicData *key_holder,
    const DDS_InstanceHandle_t handle);

OS_API DDS_InstanceHandle_t
DDS_ParticipantBuiltinTopicDataDataReader_lookup_instance (
    DDS_DataReader _this,
    const DDS_ParticipantBuiltinTopicData *key_holder);

/*
 * TopicBuiltinTopicData
 */
OS_API DDS_ReturnCode_t
DDS_TopicBuiltinTopicDataDataReader_read (
    DDS_DataReader _this,
    DDS_sequence_DDS_TopicBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states);

OS_API DDS_ReturnCode_t
DDS_TopicBuiltinTopicDataDataReader_take (
    DDS_DataReader _this,
    DDS_sequence_DDS_TopicBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states);

#define DDS_TopicBuiltinTopicDataDataReader DDS_DataReader

#define DDS_TopicBuiltinTopicDataDataReader_enable DDS_Entity_enable

#define DDS_TopicBuiltinTopicDataDataReader_get_status_changes DDS_Entity_get_status_changes

#define DDS_TopicBuiltinTopicDataDataReader_get_statuscondition DDS_Entity_get_statuscondition

#define DDS_TopicBuiltinTopicDataDataReader_get_instance_handle DDS_Entity_get_instance_handle

#define DDS_TopicBuiltinTopicDataDataReader_create_querycondition DDS_DataReader_create_querycondition

#define DDS_TopicBuiltinTopicDataDataReader_create_readcondition DDS_DataReader_create_readcondition

#define DDS_TopicBuiltinTopicDataDataReader_delete_contained_entities DDS_DataReader_delete_contained_entities

#define DDS_TopicBuiltinTopicDataDataReader_delete_readcondition DDS_DataReader_delete_readcondition

#define DDS_TopicBuiltinTopicDataDataReader_get_listener DDS_DataReader_get_listener

#define DDS_TopicBuiltinTopicDataDataReader_get_liveliness_changed_status DDS_DataReader_get_liveliness_changed_status

#define DDS_TopicBuiltinTopicDataDataReader_get_matched_publication_data DDS_DataReader_get_matched_publication_data

#define DDS_TopicBuiltinTopicDataDataReader_get_matched_publications DDS_DataReader_get_matched_publications

#define DDS_TopicBuiltinTopicDataDataReader_get_qos DDS_DataReader_get_qos

#define DDS_TopicBuiltinTopicDataDataReader_get_requested_deadline_missed_status DDS_DataReader_get_requested_deadline_missed_status

#define DDS_TopicBuiltinTopicDataDataReader_get_requested_incompatible_qos_status DDS_DataReader_get_requested_incompatible_qos_status

#define DDS_TopicBuiltinTopicDataDataReader_get_sample_lost_status DDS_DataReader_get_sample_lost_status

#define DDS_TopicBuiltinTopicDataDataReader_get_sample_rejected_status DDS_DataReader_get_sample_rejected_status

#define DDS_TopicBuiltinTopicDataDataReader_get_subscriber DDS_DataReader_get_subscriber

#define DDS_TopicBuiltinTopicDataDataReader_get_subscription_matched_status DDS_DataReader_get_subscription_matched_status

#define DDS_TopicBuiltinTopicDataDataReader_get_topicdescription DDS_DataReader_get_topicdescription

#define DDS_TopicBuiltinTopicDataDataReader_set_listener DDS_DataReader_set_listener

#define DDS_TopicBuiltinTopicDataDataReader_set_qos DDS_DataReader_set_qos

#define DDS_TopicBuiltinTopicDataDataReader_wait_for_historical_data DDS_DataReader_wait_for_historical_data

OS_API DDS_ReturnCode_t
DDS_TopicBuiltinTopicDataDataReader_read_w_condition (
    DDS_DataReader _this,
    DDS_sequence_DDS_TopicBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_ReadCondition a_condition);

OS_API DDS_ReturnCode_t
DDS_TopicBuiltinTopicDataDataReader_take_w_condition (
    DDS_DataReader _this,
    DDS_sequence_DDS_TopicBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_ReadCondition a_condition);

OS_API DDS_ReturnCode_t
DDS_TopicBuiltinTopicDataDataReader_read_next_sample (
    DDS_DataReader _this,
    DDS_TopicBuiltinTopicData *data_values,
    DDS_SampleInfo *sample_info);

OS_API DDS_ReturnCode_t
DDS_TopicBuiltinTopicDataDataReader_take_next_sample (
    DDS_DataReader _this,
    DDS_TopicBuiltinTopicData *data_values,
    DDS_SampleInfo *sample_info);

OS_API DDS_ReturnCode_t
DDS_TopicBuiltinTopicDataDataReader_read_instance (
    DDS_DataReader _this,
    DDS_sequence_DDS_TopicBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states);

OS_API DDS_ReturnCode_t
DDS_TopicBuiltinTopicDataDataReader_take_instance (
    DDS_DataReader _this,
    DDS_sequence_DDS_TopicBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states);

OS_API DDS_ReturnCode_t
DDS_TopicBuiltinTopicDataDataReader_read_next_instance (
    DDS_DataReader _this,
    DDS_sequence_DDS_TopicBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states);

OS_API DDS_ReturnCode_t
DDS_TopicBuiltinTopicDataDataReader_take_next_instance (
    DDS_DataReader _this,
    DDS_sequence_DDS_TopicBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states);

OS_API DDS_ReturnCode_t
DDS_TopicBuiltinTopicDataDataReader_read_next_instance_w_condition (
    DDS_DataReader _this,
    DDS_sequence_DDS_TopicBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_ReadCondition a_condition);

OS_API DDS_ReturnCode_t
DDS_TopicBuiltinTopicDataDataReader_take_next_instance_w_condition (
    DDS_DataReader _this,
    DDS_sequence_DDS_TopicBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_ReadCondition a_condition);

OS_API DDS_ReturnCode_t
DDS_TopicBuiltinTopicDataDataReader_return_loan (
    DDS_DataReader _this,
    DDS_sequence_DDS_TopicBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq);

OS_API DDS_ReturnCode_t
DDS_TopicBuiltinTopicDataDataReader_get_key_value (
    DDS_DataReader _this,
    DDS_TopicBuiltinTopicData *key_holder,
    const DDS_InstanceHandle_t handle);

OS_API DDS_InstanceHandle_t
DDS_TopicBuiltinTopicDataDataReader_lookup_instance (
    DDS_DataReader _this,
    const DDS_TopicBuiltinTopicData *key_holder);

/*
 * PublicationBuiltinTopicData
 */
OS_API DDS_ReturnCode_t
DDS_PublicationBuiltinTopicDataDataReader_read (
    DDS_DataReader _this,
    DDS_sequence_DDS_PublicationBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states);

OS_API DDS_ReturnCode_t
DDS_PublicationBuiltinTopicDataDataReader_take (
    DDS_DataReader _this,
    DDS_sequence_DDS_PublicationBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states);

#define DDS_PublicationBuiltinTopicDataDataReader DDS_DataReader

#define DDS_PublicationBuiltinTopicDataDataReader_enable DDS_Entity_enable

#define DDS_PublicationBuiltinTopicDataDataReader_get_status_changes DDS_Entity_get_status_changes

#define DDS_PublicationBuiltinTopicDataDataReader_get_statuscondition DDS_Entity_get_statuscondition

#define DDS_PublicationBuiltinTopicDataDataReader_get_instance_handle DDS_Entity_get_instance_handle

#define DDS_PublicationBuiltinTopicDataDataReader_create_querycondition DDS_DataReader_create_querycondition

#define DDS_PublicationBuiltinTopicDataDataReader_create_readcondition DDS_DataReader_create_readcondition

#define DDS_PublicationBuiltinTopicDataDataReader_delete_contained_entities DDS_DataReader_delete_contained_entities

#define DDS_PublicationBuiltinTopicDataDataReader_delete_readcondition DDS_DataReader_delete_readcondition

#define DDS_PublicationBuiltinTopicDataDataReader_get_listener DDS_DataReader_get_listener

#define DDS_PublicationBuiltinTopicDataDataReader_get_liveliness_changed_status DDS_DataReader_get_liveliness_changed_status

#define DDS_PublicationBuiltinTopicDataDataReader_get_matched_publication_data DDS_DataReader_get_matched_publication_data

#define DDS_PublicationBuiltinTopicDataDataReader_get_matched_publications DDS_DataReader_get_matched_publications

#define DDS_PublicationBuiltinTopicDataDataReader_get_qos DDS_DataReader_get_qos

#define DDS_PublicationBuiltinTopicDataDataReader_get_requested_deadline_missed_status DDS_DataReader_get_requested_deadline_missed_status

#define DDS_PublicationBuiltinTopicDataDataReader_get_requested_incompatible_qos_status DDS_DataReader_get_requested_incompatible_qos_status

#define DDS_PublicationBuiltinTopicDataDataReader_get_sample_lost_status DDS_DataReader_get_sample_lost_status

#define DDS_PublicationBuiltinTopicDataDataReader_get_sample_rejected_status DDS_DataReader_get_sample_rejected_status

#define DDS_PublicationBuiltinTopicDataDataReader_get_subscriber DDS_DataReader_get_subscriber

#define DDS_PublicationBuiltinTopicDataDataReader_get_subscription_matched_status DDS_DataReader_get_subscription_matched_status

#define DDS_PublicationBuiltinTopicDataDataReader_get_topicdescription DDS_DataReader_get_topicdescription

#define DDS_PublicationBuiltinTopicDataDataReader_set_listener DDS_DataReader_set_listener

#define DDS_PublicationBuiltinTopicDataDataReader_set_qos DDS_DataReader_set_qos

#define DDS_PublicationBuiltinTopicDataDataReader_wait_for_historical_data DDS_DataReader_wait_for_historical_data

OS_API DDS_ReturnCode_t
DDS_PublicationBuiltinTopicDataDataReader_read_w_condition (
    DDS_DataReader _this,
    DDS_sequence_DDS_PublicationBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_ReadCondition a_condition);

OS_API DDS_ReturnCode_t
DDS_PublicationBuiltinTopicDataDataReader_take_w_condition (
    DDS_DataReader _this,
    DDS_sequence_DDS_PublicationBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_ReadCondition a_condition);

OS_API DDS_ReturnCode_t
DDS_PublicationBuiltinTopicDataDataReader_read_next_sample (
    DDS_DataReader _this,
    DDS_PublicationBuiltinTopicData *data_values,
    DDS_SampleInfo *sample_info);

OS_API DDS_ReturnCode_t
DDS_PublicationBuiltinTopicDataDataReader_take_next_sample (
    DDS_DataReader _this,
    DDS_PublicationBuiltinTopicData *data_values,
    DDS_SampleInfo *sample_info);

OS_API DDS_ReturnCode_t
DDS_PublicationBuiltinTopicDataDataReader_read_instance (
    DDS_DataReader _this,
    DDS_sequence_DDS_PublicationBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states);

OS_API DDS_ReturnCode_t
DDS_PublicationBuiltinTopicDataDataReader_take_instance (
    DDS_DataReader _this,
    DDS_sequence_DDS_PublicationBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states);

OS_API DDS_ReturnCode_t
DDS_PublicationBuiltinTopicDataDataReader_read_next_instance (
    DDS_DataReader _this,
    DDS_sequence_DDS_PublicationBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states);

OS_API DDS_ReturnCode_t
DDS_PublicationBuiltinTopicDataDataReader_take_next_instance (
    DDS_DataReader _this,
    DDS_sequence_DDS_PublicationBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states);

OS_API DDS_ReturnCode_t
DDS_PublicationBuiltinTopicDataDataReader_read_next_instance_w_condition (
    DDS_DataReader _this,
    DDS_sequence_DDS_PublicationBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_ReadCondition a_condition);

OS_API DDS_ReturnCode_t
DDS_PublicationBuiltinTopicDataDataReader_take_next_instance_w_condition (
    DDS_DataReader _this,
    DDS_sequence_DDS_PublicationBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_ReadCondition a_condition);

OS_API DDS_ReturnCode_t
DDS_PublicationBuiltinTopicDataDataReader_return_loan (
    DDS_DataReader _this,
    DDS_sequence_DDS_PublicationBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq);

OS_API DDS_ReturnCode_t
DDS_PublicationBuiltinTopicDataDataReader_get_key_value (
    DDS_DataReader _this,
    DDS_PublicationBuiltinTopicData *key_holder,
    const DDS_InstanceHandle_t handle);

OS_API DDS_InstanceHandle_t
DDS_PublicationBuiltinTopicDataDataReader_lookup_instance (
    DDS_DataReader _this,
    const DDS_PublicationBuiltinTopicData *key_holder);

/*
 * SubscriptionBuiltinTopicData
 */
OS_API DDS_ReturnCode_t
DDS_SubscriptionBuiltinTopicDataDataReader_read (
    DDS_DataReader _this,
    DDS_sequence_DDS_SubscriptionBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states);

OS_API DDS_ReturnCode_t
DDS_SubscriptionBuiltinTopicDataDataReader_take (
    DDS_DataReader _this,
    DDS_sequence_DDS_SubscriptionBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states);

#define DDS_SubscriptionBuiltinTopicDataDataReader DDS_DataReader

#define DDS_SubscriptionBuiltinTopicDataDataReader_enable DDS_Entity_enable

#define DDS_SubscriptionBuiltinTopicDataDataReader_get_status_changes DDS_Entity_get_status_changes

#define DDS_SubscriptionBuiltinTopicDataDataReader_get_statuscondition DDS_Entity_get_statuscondition

#define DDS_SubscriptionBuiltinTopicDataDataReader_get_instance_handle DDS_Entity_get_instance_handle

#define DDS_SubscriptionBuiltinTopicDataDataReader_create_querycondition DDS_DataReader_create_querycondition

#define DDS_SubscriptionBuiltinTopicDataDataReader_create_readcondition DDS_DataReader_create_readcondition

#define DDS_SubscriptionBuiltinTopicDataDataReader_delete_contained_entities DDS_DataReader_delete_contained_entities

#define DDS_SubscriptionBuiltinTopicDataDataReader_delete_readcondition DDS_DataReader_delete_readcondition

#define DDS_SubscriptionBuiltinTopicDataDataReader_get_listener DDS_DataReader_get_listener

#define DDS_SubscriptionBuiltinTopicDataDataReader_get_liveliness_changed_status DDS_DataReader_get_liveliness_changed_status

#define DDS_SubscriptionBuiltinTopicDataDataReader_get_matched_publication_data DDS_DataReader_get_matched_publication_data

#define DDS_SubscriptionBuiltinTopicDataDataReader_get_matched_publications DDS_DataReader_get_matched_publications

#define DDS_SubscriptionBuiltinTopicDataDataReader_get_qos DDS_DataReader_get_qos

#define DDS_SubscriptionBuiltinTopicDataDataReader_get_requested_deadline_missed_status DDS_DataReader_get_requested_deadline_missed_status

#define DDS_SubscriptionBuiltinTopicDataDataReader_get_requested_incompatible_qos_status DDS_DataReader_get_requested_incompatible_qos_status

#define DDS_SubscriptionBuiltinTopicDataDataReader_get_sample_lost_status DDS_DataReader_get_sample_lost_status

#define DDS_SubscriptionBuiltinTopicDataDataReader_get_sample_rejected_status DDS_DataReader_get_sample_rejected_status

#define DDS_SubscriptionBuiltinTopicDataDataReader_get_subscriber DDS_DataReader_get_subscriber

#define DDS_SubscriptionBuiltinTopicDataDataReader_get_subscription_matched_status DDS_DataReader_get_subscription_matched_status

#define DDS_SubscriptionBuiltinTopicDataDataReader_get_topicdescription DDS_DataReader_get_topicdescription

#define DDS_SubscriptionBuiltinTopicDataDataReader_set_listener DDS_DataReader_set_listener

#define DDS_SubscriptionBuiltinTopicDataDataReader_set_qos DDS_DataReader_set_qos

#define DDS_SubscriptionBuiltinTopicDataDataReader_wait_for_historical_data DDS_DataReader_wait_for_historical_data

OS_API DDS_ReturnCode_t
DDS_SubscriptionBuiltinTopicDataDataReader_read_w_condition (
    DDS_DataReader _this,
    DDS_sequence_DDS_SubscriptionBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_ReadCondition a_condition);

OS_API DDS_ReturnCode_t
DDS_SubscriptionBuiltinTopicDataDataReader_take_w_condition (
    DDS_DataReader _this,
    DDS_sequence_DDS_SubscriptionBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_ReadCondition a_condition);

OS_API DDS_ReturnCode_t
DDS_SubscriptionBuiltinTopicDataDataReader_read_next_sample (
    DDS_DataReader _this,
    DDS_SubscriptionBuiltinTopicData *data_values,
    DDS_SampleInfo *sample_info);

OS_API DDS_ReturnCode_t
DDS_SubscriptionBuiltinTopicDataDataReader_take_next_sample (
    DDS_DataReader _this,
    DDS_SubscriptionBuiltinTopicData *data_values,
    DDS_SampleInfo *sample_info);

OS_API DDS_ReturnCode_t
DDS_SubscriptionBuiltinTopicDataDataReader_read_instance (
    DDS_DataReader _this,
    DDS_sequence_DDS_SubscriptionBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states);

OS_API DDS_ReturnCode_t
DDS_SubscriptionBuiltinTopicDataDataReader_take_instance (
    DDS_DataReader _this,
    DDS_sequence_DDS_SubscriptionBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states);

OS_API DDS_ReturnCode_t
DDS_SubscriptionBuiltinTopicDataDataReader_read_next_instance (
    DDS_DataReader _this,
    DDS_sequence_DDS_SubscriptionBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states);

OS_API DDS_ReturnCode_t
DDS_SubscriptionBuiltinTopicDataDataReader_take_next_instance (
    DDS_DataReader _this,
    DDS_sequence_DDS_SubscriptionBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states);

OS_API DDS_ReturnCode_t
DDS_SubscriptionBuiltinTopicDataDataReader_read_next_instance_w_condition (
    DDS_DataReader _this,
    DDS_sequence_DDS_SubscriptionBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_ReadCondition a_condition);

OS_API DDS_ReturnCode_t
DDS_SubscriptionBuiltinTopicDataDataReader_take_next_instance_w_condition (
    DDS_DataReader _this,
    DDS_sequence_DDS_SubscriptionBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_ReadCondition a_condition);

OS_API DDS_ReturnCode_t
DDS_SubscriptionBuiltinTopicDataDataReader_return_loan (
    DDS_DataReader _this,
    DDS_sequence_DDS_SubscriptionBuiltinTopicData *data_values,
    DDS_SampleInfoSeq *info_seq);

OS_API DDS_ReturnCode_t
DDS_SubscriptionBuiltinTopicDataDataReader_get_key_value (
    DDS_DataReader _this,
    DDS_SubscriptionBuiltinTopicData *key_holder,
    const DDS_InstanceHandle_t handle);

OS_API DDS_InstanceHandle_t
DDS_SubscriptionBuiltinTopicDataDataReader_lookup_instance (
    DDS_DataReader _this,
    const DDS_SubscriptionBuiltinTopicData *key_holder);

typedef enum {
    DDS_TYPE_ELEMENT_KIND_MODULE,
    DDS_TYPE_ELEMENT_KIND_STRUCT,
    DDS_TYPE_ELEMENT_KIND_MEMBER,
    DDS_TYPE_ELEMENT_KIND_UNION,
    DDS_TYPE_ELEMENT_KIND_UNIONCASE,
    DDS_TYPE_ELEMENT_KIND_UNIONSWITCH,
    DDS_TYPE_ELEMENT_KIND_UNIONLABEL,
    DDS_TYPE_ELEMENT_KIND_TYPEDEF,
    DDS_TYPE_ELEMENT_KIND_ENUM,
    DDS_TYPE_ELEMENT_KIND_ENUMLABEL,
    DDS_TYPE_ELEMENT_KIND_TYPE,
    DDS_TYPE_ELEMENT_KIND_ARRAY,
    DDS_TYPE_ELEMENT_KIND_SEQUENCE,
    DDS_TYPE_ELEMENT_KIND_STRING,
    DDS_TYPE_ELEMENT_KIND_CHAR,
    DDS_TYPE_ELEMENT_KIND_BOOLEAN,
    DDS_TYPE_ELEMENT_KIND_OCTET,
    DDS_TYPE_ELEMENT_KIND_SHORT,
    DDS_TYPE_ELEMENT_KIND_USHORT,
    DDS_TYPE_ELEMENT_KIND_LONG,
    DDS_TYPE_ELEMENT_KIND_ULONG,
    DDS_TYPE_ELEMENT_KIND_LONGLONG,
    DDS_TYPE_ELEMENT_KIND_ULONGLONG,
    DDS_TYPE_ELEMENT_KIND_FLOAT,
    DDS_TYPE_ELEMENT_KIND_DOUBLE,
    DDS_TYPE_ELEMENT_KIND_TIME,
    DDS_TYPE_ELEMENT_KIND_UNIONLABELDEFAULT
} DDS_TypeElementKind;

typedef enum {
    DDS_TYPE_ATTRIBUTE_KIND_NUMBER,
    DDS_TYPE_ATTRIBUTE_KIND_STRING
} DDS_TypeAttributeKind;

typedef struct DDS_TypeAttributeValue {
    DDS_TypeAttributeKind _d;
    union {
        DDS_long   nvalue;
        DDS_string svalue;
    } _u;
} DDS_TypeAttributeValue;

typedef struct DDS_TypeAttribute {
    DDS_string             name;
    DDS_TypeAttributeValue value;
} DDS_TypeAttribute;

typedef struct {
    DDS_unsigned_long  _maximum;
    DDS_unsigned_long  _length;
    DDS_TypeAttribute *_buffer;
    DDS_boolean        _release;
} DDS_sequence_DDS_TypeAttribute;
typedef DDS_sequence_DDS_TypeAttribute DDS_TypeAttributeSeq;

typedef void * DDS_TypeParserHandle;

typedef DDS_boolean (*DDS_TypeParserCallback)(
    DDS_TypeElementKind         kind,
    const DDS_string            elementName,
    const DDS_TypeAttributeSeq *attributes,
    DDS_TypeParserHandle        handle,
    void                       *argument);

OS_API DDS_ReturnCode_t
DDS_TypeSupport_parse_type_description (
    const DDS_string        description,
    DDS_TypeParserCallback  callback,
    void                   *argument);

OS_API DDS_ReturnCode_t
DDS_TypeSupport_walk_type_description (
    DDS_TypeParserHandle    handle,
    DDS_TypeParserCallback  callback,
    void                   *argument);

/*
 * interface ErrorInfo
 */

/*
 * typedef long ErrorCodeCode_t;
 */
typedef DDS_long DDS_ErrorCode_t;

/*
 * // ----------------------------------------------------------------------
 * // Error codes
 * // ----------------------------------------------------------------------
 *    const ErrorCode_t ERRORCODE_UNDEFINED                     = 0;
 *    const ErrorCode_t ERRORCODE_ERROR                 = 1;
 *    const ErrorCode_t ERRORCODE_OUT_OF_RESOURCES          = 2;
 *    const ErrorCode_t ERRORCODE_CREATION_KERNEL_ENTITY_FAILED = 3;
 *    const ErrorCode_t ERRORCODE_INVALID_VALUE             = 4;
 *    const ErrorCode_t ERRORCODE_INVALID_DURATION      = 5;
 *    const ErrorCode_t ERRORCODE_INVALID_TIME          = 6;
 *    const ErrorCode_t ERRORCODE_ENTITY_INUSE              = 7;
 *    const ErrorCode_t ERRORCODE_CONTAINS_ENTITIES               = 8;
 *    const ErrorCode_t ERRORCODE_ENTITY_UNKNOWN        = 9;
 *    const ErrorCode_t ERRORCODE_HANDLE_NOT_REGISTERED     = 10;
 *    const ErrorCode_t ERRORCODE_HANDLE_NOT_MATCH      = 11;
 *    const ErrorCode_t ERRORCODE_HANDLE_INVALID                  = 12;
 *    const ErrorCode_t ERRORCODE_INVALID_SEQUENCE                = 13;
 *    const ErrorCode_t ERRORCODE_UNSUPPORTED_VALUE               = 14;
 *    const ErrorCode_t ERRORCODE_INCONSISTENT_VALUE              = 15;
 *    const ErrorCode_t ERRORCODE_IMMUTABLE_QOS_POLICY            = 16;
 *    const ErrorCode_t ERRORCODE_INCONSISTENT_QOS                = 17;
 *    const ErrorCode_t ERRORCODE_UNSUPPORTED_QOS_POLICY          = 18;
 *    const ErrorCode_t ERRORCODE_CONTAINS_CONDITIONS             = 19;
 *    const ErrorCode_t ERRORCODE_CONTAINS_LOANS                  = 20;
 *    const ErrorCode_t ERRORCODE_INCONSISTENT_TOPIC              = 21;
 */

#define DDS_ERRORCODE_UNDEFINED                 0
#define DDS_ERRORCODE_ERROR             1
#define DDS_ERRORCODE_OUT_OF_RESOURCES              2
#define DDS_ERRORCODE_CREATION_KERNEL_ENTITY_FAILED 3
#define DDS_ERRORCODE_INVALID_VALUE                 4
#define DDS_ERRORCODE_INVALID_DURATION          5
#define DDS_ERRORCODE_INVALID_TIME              6
#define DDS_ERRORCODE_ENTITY_INUSE                  7
#define DDS_ERRORCODE_CONTAINS_ENTITIES             8
#define DDS_ERRORCODE_ENTITY_UNKNOWN            9
#define DDS_ERRORCODE_HANDLE_NOT_REGISTERED         10
#define DDS_ERRORCODE_HANDLE_NOT_MATCH          11
#define DDS_ERRORCODE_HANDLE_INVALID                12
#define DDS_ERRORCODE_INVALID_SEQUENCE              13
#define DDS_ERRORCODE_UNSUPPORTED_VALUE             14
#define DDS_ERRORCODE_INCONSISTENT_VALUE            15
#define DDS_ERRORCODE_IMMUTABLE_QOS_POLICY          16
#define DDS_ERRORCODE_INCONSISTENT_QOS              17
#define DDS_ERRORCODE_UNSUPPORTED_QOS_POLICY        18
#define DDS_ERRORCODE_CONTAINS_CONDITIONS           19
#define DDS_ERRORCODE_CONTAINS_LOANS                20
#define DDS_ERRORCODE_INCONSISTENT_TOPIC            21

typedef DDS_Object DDS_ErrorInfo;

/*     ReturnCode_t
 *     update( );
 */
OS_API DDS_ReturnCode_t
DDS_ErrorInfo_update (
    DDS_ErrorInfo _this);

/*     ReturnCode_t
 *     get_code(
 *         out ErrorCode code);
 */
OS_API DDS_ReturnCode_t
DDS_ErrorInfo_get_code (
    DDS_ErrorInfo _this,
    DDS_ErrorCode_t *code);

/*     ReturnCode_t
 *     get_location(
 *         out String location);
 */
OS_API DDS_ReturnCode_t
DDS_ErrorInfo_get_location(
    DDS_ErrorInfo _this,
    DDS_string *location);

/*     ReturnCode_t
 *     get_source_line(
 *         out String source_line);
 */
OS_API DDS_ReturnCode_t
DDS_ErrorInfo_get_source_line(
    DDS_ErrorInfo _this,
    DDS_string *source_line);

/*     ReturnCode_t
 *     get_stack_trace(
 *         out String stack_trace);
 */
OS_API DDS_ReturnCode_t
DDS_ErrorInfo_get_stack_trace(
    DDS_ErrorInfo _this,
    DDS_string *stack_trace);

/*     ReturnCode_t
 *     get_message(
 *         out String message);
 */
OS_API DDS_ReturnCode_t
DDS_ErrorInfo_get_message(
    DDS_ErrorInfo _this,
    DDS_string *message);


/*     ErrorInfo
 *     ErrorInfo__alloc (
 *         void);
 */
OS_API DDS_ErrorInfo
DDS_ErrorInfo__alloc (void);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* DDS_DCPS_H */
