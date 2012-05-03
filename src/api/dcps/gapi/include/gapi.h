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
#ifndef GAPI_H
#define GAPI_H

#ifdef INTEGRITY
#pragma ghs nowarning 1795
#pragma ghs nowarning 997
#endif

#include "c_base.h"
#include "c_metabase.h"
#include "v_kernel.h"
#include "os_if.h"

#ifdef OSPL_BUILD_DCPSGAPI
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#if defined (__cplusplus)
extern "C" {
#endif

#if 0
typedef short int                               gapi_short;
typedef int                                     gapi_long;
typedef long long int                           gapi_long_long;
typedef unsigned short int                      gapi_unsigned_short;
typedef unsigned int                            gapi_unsigned_long;
typedef unsigned long long int                  gapi_unsigned_long_long;
typedef float                                   gapi_float;
typedef double                                  gapi_double;
typedef long double                             gapi_long_double;
typedef char                                    gapi_char;
typedef unsigned char                           gapi_octet;
typedef unsigned char                           gapi_boolean;
typedef gapi_char *                             gapi_string;
#else
typedef c_short     gapi_short;
typedef c_long      gapi_long;
typedef c_longlong  gapi_long_long;
typedef c_ushort    gapi_unsigned_short;
typedef c_ulong     gapi_unsigned_long;
typedef c_ulonglong gapi_unsigned_long_long;
typedef c_float     gapi_float;
typedef c_double    gapi_double;
typedef c_char      gapi_char;
typedef c_octet     gapi_octet;
typedef c_bool      gapi_boolean;
typedef c_string    gapi_string;
#endif

typedef gapi_boolean (*_dealloactorType)(void *object);
typedef void *(*_bufferAllocatorType)(gapi_unsigned_long len);
typedef c_bool (*gapi_ReaderInstanceAction)(v_dataReaderInstance instance, c_voidp arg);
typedef void (*gapi_readerAction) (c_voidp from, c_voidp to);


/* Generic DDS object pointer */

C_CLASS(gapi_handle);

typedef gapi_handle gapi_object;

#define gapi_handle(h) ((gapi_handle)h)
#define gapi_object(o) ((gapi_object)o)

#define GAPI_OBJECT_NIL (gapi_object)NULL

/* Private obj management functions */
OS_API void *
gapi__malloc (
    gapi_boolean (*ff)(void *),
    gapi_unsigned_long hl,
    gapi_unsigned_long len);

OS_API void
gapi__free (
    void *object);

OS_API void *
gapi__header (
    void *object);

/* Generic allocation/release functions */
OS_API void
gapi_free (
    void *);

OS_API void *
gapi_alloc (
    gapi_unsigned_long l);


OS_API gapi_char *
gapi_string_alloc (
    gapi_unsigned_long len);

OS_API gapi_char *
gapi_string_dup (
    const gapi_char *src);

OS_API void
gapi_string_clean (
    gapi_char **string);

OS_API void
gapi_string_replace (
    gapi_char *src,
    gapi_char **dst);

OS_API void *
gapi_sequence_malloc (
    void);

OS_API gapi_boolean
gapi_sequence_free (
    void *sequence);

OS_API void
gapi_sequence_clean (
    void *sequence);

OS_API void *
gapi_sequence_allocbuf (
    _dealloactorType deallocator,
    gapi_unsigned_long len,
    gapi_unsigned_long count);

OS_API void
gapi_sequence_replacebuf (
    void *sequence,
    _bufferAllocatorType allocbuf,
    gapi_unsigned_long count);

OS_API void *
gapi_sequence_create (
    _dealloactorType deallocator,
    gapi_unsigned_long len,
    gapi_unsigned_long count);

/* Sequence support routines */
OS_API void
gapi_sequence_set_release (
    void *sequence,
    gapi_boolean release);

OS_API gapi_boolean
gapi_sequence_get_release (
    void *sequence);

/* Default QoS definitions */
#define GAPI_PARTICIPANTFACTORY_QOS_DEFAULT     NULL
#define GAPI_PARTICIPANT_QOS_DEFAULT            NULL
#define GAPI_TOPIC_QOS_DEFAULT                  NULL
#define GAPI_PUBLISHER_QOS_DEFAULT              NULL
#define GAPI_SUBSCRIBER_QOS_DEFAULT             NULL
#define GAPI_DATAREADER_QOS_DEFAULT             NULL
#define GAPI_DATAVIEW_QOS_DEFAULT               NULL
#define GAPI_DATAWRITER_QOS_DEFAULT             NULL
#define GAPI_DATAWRITER_QOS_USE_TOPIC_QOS   ((gapi_dataWriterQos *)-1)
#define GAPI_DATAREADER_QOS_USE_TOPIC_QOS   ((gapi_dataReaderQos *)-1)

/* Environment replacement type */

#define GAPI_DOMAINID_TYPE_NATIVE           gapi_string
#define GAPI_HANDLE_TYPE_NATIVE             gapi_long_long
#define GAPI_HANDLE_NIL_NATIVE              0L
#define GAPI_BUILTIN_TOPIC_KEY_TYPE_NATIVE  gapi_long

#define _TheParticipantFactory              gapi_domainParticipantFactory_get_instance()

/*
 * DDS DCPS IDL definitions
 */

/*
 * typedef DOMAINID_TYPE_NATIVE DomainId_t;
 */
typedef GAPI_DOMAINID_TYPE_NATIVE gapi_domainId_t;

/*
 * typedef HANDLE_TYPE_NATIVE InstanceHandle_t;
 */
typedef GAPI_HANDLE_TYPE_NATIVE gapi_instanceHandle_t;

/*
 * typedef BUILTIN_TOPIC_KEY_TYPE_NATIVE BuiltinTopicKey_t[3];
 */
typedef GAPI_BUILTIN_TOPIC_KEY_TYPE_NATIVE gapi_builtinTopicKey_t[3];

/*
 * typedef long ReturnCode_t;
 */
typedef gapi_long gapi_returnCode_t;

/*
 * typedef sequence<InstanceHandle_t> InstanceHandleSeq;
 */
#ifndef _GAPI_INSTANCEHANDLESEQ_DEFINED
#define _GAPI_INSTANCEHANDLESEQ_DEFINED
typedef struct {
    gapi_unsigned_long _maximum;
    gapi_unsigned_long _length;
    gapi_instanceHandle_t *_buffer;
    gapi_boolean _release;
} gapi_instanceHandleSeq;
#endif /* GAPI_INSTANCEHANDLESEQ_DEFINED */

OS_API gapi_instanceHandleSeq *gapi_instanceHandleSeq__alloc (void);
OS_API gapi_instanceHandle_t *gapi_instanceHandleSeq_allocbuf (gapi_unsigned_long len);

/* Helper functions for efficient implementation of DLRL */

typedef struct {
    gapi_unsigned_long gid1;
    gapi_unsigned_long gid2;
    gapi_unsigned_long gid3;
} gapi_globalId_t;

OS_API gapi_returnCode_t
gapi_instanceHandle_to_global_id(
    gapi_instanceHandle_t h,
    gapi_globalId_t *globalId);

OS_API gapi_returnCode_t
gapi_instanceHandle_from_global_id(
    gapi_instanceHandle_t *h,
    gapi_globalId_t globalId);

/*
 * typedef long QosPolicyId_t;
 */
typedef gapi_long gapi_qosPolicyId_t;

/*
 * typedef sequence<string> StringSeq;
 */
#ifndef _GAPI_STRINGSEQ_DEFINED
#define _GAPI_STRINGSEQ_DEFINED
typedef struct {
    gapi_unsigned_long _maximum;
    gapi_unsigned_long _length;
    gapi_string *_buffer;
    gapi_boolean _release;
} gapi_stringSeq;

OS_API gapi_stringSeq *
gapi_stringSeq__alloc (void);

OS_API gapi_string *
gapi_stringSeq_allocbuf (gapi_unsigned_long len);

#endif /* GAPI_STRINGSEQ_DEFINED */

OS_API gapi_stringSeq *
gapi_stringSeq__alloc (void);

OS_API gapi_string *
gapi_stringSeq_allocbuf (gapi_unsigned_long len);

OS_API gapi_boolean
gapi_stringSeq_set_length (gapi_stringSeq *seq, gapi_unsigned_long len);

/*
 * struct Duration_t {
 *     long sec;
 *     unsigned long nanosec;
 * };
 */
typedef C_STRUCT(gapi_duration_t) {
    gapi_long sec;
    gapi_unsigned_long nanosec;
} gapi_duration_t;

OS_API gapi_duration_t *
gapi_duration_t__alloc (void);
/*
 * struct Time_t {
 *     long sec;
 *     unsigned long nanosec;
 * };
 */
typedef C_STRUCT(gapi_time_t) {
    gapi_long sec;
    gapi_unsigned_long nanosec;
} gapi_time_t;

OS_API gapi_time_t *
gapi_time_t__alloc (void);

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
#define GAPI_HANDLE_NIL                                 GAPI_HANDLE_NIL_NATIVE
#define GAPI_LENGTH_UNLIMITED                           -1
#define GAPI_DURATION_INFINITE_SEC                      0x7fffffff
#define GAPI_DURATION_INFINITE_NSEC                     0x7fffffffU
#define GAPI_DURATION_INFINITE                          { GAPI_DURATION_INFINITE_SEC, GAPI_DURATION_INFINITE_NSEC }
#define GAPI_DURATION_ZERO_SEC                          0
#define GAPI_DURATION_ZERO_NSEC                         0U
#define GAPI_DURATION_ZERO                              { GAPI_DURATION_ZERO_SEC, GAPI_DURATION_ZERO_NSEC }
#define GAPI_TIMESTAMP_INVALID_SEC                      -1
#define GAPI_TIMESTAMP_INVALID_NSEC                     4294967295U
#define GAPI_TIMESTAMP_INVALID                          { GAPI_TIMESTAMP_INVALID_SEC, GAPI_TIMESTAMP_INVALID_NSEC }

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
 * const ReturnCode_t RETCODE_ILLEGAL_OPERATION        = 12;
 */
#define GAPI_RETCODE_OK                             0
#define GAPI_RETCODE_ERROR                          1
#define GAPI_RETCODE_UNSUPPORTED                    2
#define GAPI_RETCODE_BAD_PARAMETER                  3
#define GAPI_RETCODE_PRECONDITION_NOT_MET           4
#define GAPI_RETCODE_OUT_OF_RESOURCES               5
#define GAPI_RETCODE_NOT_ENABLED                    6
#define GAPI_RETCODE_IMMUTABLE_POLICY               7
#define GAPI_RETCODE_INCONSISTENT_POLICY            8
#define GAPI_RETCODE_ALREADY_DELETED                9
#define GAPI_RETCODE_TIMEOUT                        10
#define GAPI_RETCODE_NO_DATA                        11
#define GAPI_RETCODE_ILLEGAL_OPERATION               12

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
 * const StatusKind PUBLICATION_MATCH_STATUS           = 0x0001 << 13;
 * const StatusKind SUBSCRIPTION_MATCH_STATUS          = 0x0001 << 14;
 */
typedef gapi_unsigned_long gapi_statusKind;
typedef gapi_unsigned_long gapi_statusMask;

#define GAPI_INCONSISTENT_TOPIC_STATUS              1U
#define GAPI_OFFERED_DEADLINE_MISSED_STATUS         2U
#define GAPI_REQUESTED_DEADLINE_MISSED_STATUS       4U
#define GAPI_OFFERED_INCOMPATIBLE_QOS_STATUS        32U
#define GAPI_REQUESTED_INCOMPATIBLE_QOS_STATUS      64U
#define GAPI_SAMPLE_LOST_STATUS                     128U
#define GAPI_SAMPLE_REJECTED_STATUS                 256U
#define GAPI_DATA_ON_READERS_STATUS                 512U
#define GAPI_DATA_AVAILABLE_STATUS                  1024U
#define GAPI_LIVELINESS_LOST_STATUS                 2048U
#define GAPI_LIVELINESS_CHANGED_STATUS              4096U
#define GAPI_PUBLICATION_MATCH_STATUS               8192U
#define GAPI_SUBSCRIPTION_MATCH_STATUS              16384U

#define GAPI_ALL_DATA_DISPOSED_STATUS               1U<<31

#define GAPI_ANY_STATUS              0x7FE7
#define GAPI_STATUS_ANY_V1_2         0x7FE7

/*
 * struct InconsistentTopicStatus {
 *     long total_count;
 *     long total_count_change;
 * };
 */
typedef C_STRUCT(gapi_inconsistentTopicStatus) {
    gapi_long total_count;
    gapi_long total_count_change;
} gapi_inconsistentTopicStatus;

/*
 * struct AllDataDisposedStatus {
 *     long total_count;
 *     long total_count_change;
 * };
 */
typedef C_STRUCT(gapi_allDataDisposedTopicStatus) {
    gapi_long total_count;
    gapi_long total_count_change;
} gapi_allDataDisposedTopicStatus;

/*
 * struct SampleLostStatus {
 *     long total_count;
 *     long total_count_change;
 * };
 */
typedef C_STRUCT(gapi_sampleLostStatus) {
    gapi_long total_count;
    gapi_long total_count_change;
} gapi_sampleLostStatus;

/*
 * enum SampleRejectedStatusKind {
 *     REJECTED_BY_INSTANCE_LIMIT,
 *     REJECTED_BY_TOPIC_LIMIT
 * };
 */
typedef enum
{
    GAPI_NOT_REJECTED,
    GAPI_REJECTED_BY_INSTANCES_LIMIT,
    GAPI_REJECTED_BY_SAMPLES_LIMIT,
    GAPI_REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT
} gapi_sampleRejectedStatusKind;

/*
 * struct SampleRejectedStatus {
 *     long total_count;
 *     long total_count_change;
 *     SampleRejectedStatusKind last_reason;
 *     InstanceHandle_t last_instance_handle;
 * };
 */
typedef C_STRUCT(gapi_sampleRejectedStatus) {
    gapi_long total_count;
    gapi_long total_count_change;
    gapi_sampleRejectedStatusKind last_reason;
    gapi_instanceHandle_t last_instance_handle;
} gapi_sampleRejectedStatus;

/*
 * struct LivelinessLostStatus {
 *     long total_count;
 *     long total_count_change;
 * };
 */
typedef C_STRUCT(gapi_livelinessLostStatus) {
    gapi_long total_count;
    gapi_long total_count_change;
} gapi_livelinessLostStatus;

/*
 * struct LivelinessChangedStatus {
 *     long alive_count;
 *     long not_alive_count;
 *     long alive_count_change;
 *     long not_alive_count_change;
 *     InstanceHandle_t last_publication_handle;
 * };
 */
typedef C_STRUCT(gapi_livelinessChangedStatus) {
    gapi_long alive_count;
    gapi_long not_alive_count;
    gapi_long alive_count_change;
    gapi_long not_alive_count_change;
    gapi_instanceHandle_t last_publication_handle;
} gapi_livelinessChangedStatus;

/*
 * struct OfferedDeadlineMissedStatus {
 *     long total_count;
 *     long total_count_change;
 *     InstanceHandle_t last_instance_handle;
 * };
 */
typedef C_STRUCT(gapi_offeredDeadlineMissedStatus) {
    gapi_long total_count;
    gapi_long total_count_change;
    gapi_instanceHandle_t last_instance_handle;
} gapi_offeredDeadlineMissedStatus;

/*
 * struct RequestedDeadlineMissedStatus {
 *     long total_count;
 *     long total_count_change;
 *     InstanceHandle_t last_instance_handle;
 * };
 */
typedef C_STRUCT(gapi_requestedDeadlineMissedStatus) {
    gapi_long total_count;
    gapi_long total_count_change;
    gapi_instanceHandle_t last_instance_handle;
} gapi_requestedDeadlineMissedStatus;

/*
 * struct QosPolicyCount {
 *     QosPolicyId_t policy_id;
 *     long count;
 * };
 */
typedef C_STRUCT(gapi_qosPolicyCount) {
    gapi_qosPolicyId_t policy_id;
    gapi_long count;
} gapi_qosPolicyCount;

/*
 * typedef sequence<QosPolicyCount> QosPolicyCountSeq;
 */
#ifndef _GAPI_QOSPOLICYCOUNTSEQ_DEFINED
#define _GAPI_QOSPOLICYCOUNTSEQ_DEFINED
typedef struct {
    gapi_unsigned_long _maximum;
    gapi_unsigned_long _length;
    gapi_qosPolicyCount *_buffer;
    gapi_boolean _release;
} gapi_qosPolicyCountSeq;

OS_API gapi_qosPolicyCountSeq *
gapi_qosPolicyCountSeq__alloc (void);

OS_API gapi_qosPolicyCount *
gapi_qosPolicyCountSeq_allocbuf (gapi_unsigned_long len);

#endif /* GAPI_QOSPOLICYCOUNTSEQ_DEFINED */

OS_API gapi_qosPolicyCountSeq *
gapi_qosPolicyCountSeq__alloc (void);

OS_API gapi_qosPolicyCount *
gapi_qosPolicyCountSeq_allocbuf (gapi_unsigned_long len);

/*
 * struct OfferedIncompatibleQosStatus {
 *     long total_count;
 *     long total_count_change;
 *     QosPolicyId_t last_policy_id;
 *     QosPolicyCountSeq policies;
 * };
 */
typedef C_STRUCT(gapi_offeredIncompatibleQosStatus) {
    gapi_long total_count;
    gapi_long total_count_change;
    gapi_qosPolicyId_t last_policy_id;
    gapi_qosPolicyCountSeq policies;
} gapi_offeredIncompatibleQosStatus;

/*
 * struct RequestedIncompatibleQosStatus {
 *     long total_count;
 *     long total_count_change;
 *     QosPolicyId_t last_policy_id;
 *     QosPolicyCountSeq policies;
 * };
 */
typedef C_STRUCT(gapi_requestedIncompatibleQosStatus) {
    gapi_long total_count;
    gapi_long total_count_change;
    gapi_qosPolicyId_t last_policy_id;
    gapi_qosPolicyCountSeq policies;
} gapi_requestedIncompatibleQosStatus;

/*
 * struct PublicationMatchededStatus {
 *     long total_count;
 *     long total_count_change;
 *     long current_count;
 *     long current_count_change;
 *     InstanceHandle_t last_subscription_handle;
 * };
 */
typedef C_STRUCT(gapi_publicationMatchedStatus) {
    gapi_long total_count;
    gapi_long total_count_change;
    gapi_long current_count;
    gapi_long current_count_change;
    gapi_instanceHandle_t last_subscription_handle;
} gapi_publicationMatchedStatus;

/*
 * struct SubscriptionMatchedStatus {
 *     long total_count;
 *     long total_count_change;
 *     long current_count;
 *     long current_count_change;
 *     InstanceHandle_t last_publication_handle;
 * };
 */
typedef C_STRUCT(gapi_subscriptionMatchedStatus) {
    gapi_long total_count;
    gapi_long total_count_change;
    gapi_long current_count;
    gapi_long current_count_change;
    gapi_instanceHandle_t last_publication_handle;
} gapi_subscriptionMatchedStatus;


OS_API gapi_requestedIncompatibleQosStatus *
gapi_requestedIncompatibleQosStatus_alloc (
    void);

OS_API gapi_offeredIncompatibleQosStatus *
gapi_offeredIncompatibleQosStatus_alloc (
    void);

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
 * interface TypeSupport;
 */
typedef struct gapi_listener gapi_listener;
typedef gapi_object gapi_entity;
typedef gapi_object gapi_topicDescription;
typedef gapi_object gapi_topic;
typedef gapi_object gapi_contentFilteredTopic;
typedef gapi_object gapi_multiTopic;
typedef gapi_object gapi_dataWriter;
typedef gapi_object gapi_dataReader;
typedef gapi_object gapi_dataReaderView;
typedef gapi_object gapi_subscriber;
typedef gapi_object gapi_publisher;
typedef gapi_object gapi_typeSupport;

typedef void (*gapi_listenerThreadAction)(void *listener_data);

/*
 * typedef sequence<Topic> TopicSeq;
 */
#ifndef _GAPI_TOPICSEQ_DEFINED
#define _GAPI_TOPICSEQ_DEFINED
typedef struct {
    gapi_unsigned_long _maximum;
    gapi_unsigned_long _length;
    gapi_topic *_buffer;
    gapi_boolean _release;
} gapi_topicSeq;
#endif /* _GAPI_TOPICSEQ_DEFINED */

OS_API gapi_topicSeq *gapi_topicSeq__alloc (void);
OS_API gapi_topic *gapi_topicSeq_allocbuf (gapi_unsigned_long len);

/*
 * typedef sequence<DataReader> DataReaderSeq;
 */
#ifndef _GAPI_DATAREADERSEQ_DEFINED
#define _GAPI_DATAREADERSEQ_DEFINED
typedef struct {
    gapi_unsigned_long _maximum;
    gapi_unsigned_long _length;
    gapi_dataReader *_buffer;
    gapi_boolean _release;
} gapi_dataReaderSeq;
#endif /* _GAPI_DATAREADERSEQ_DEFINED */

OS_API gapi_dataReaderSeq *
gapi_dataReaderSeq__alloc (void);

OS_API gapi_dataReader *
gapi_dataReaderSeq_allocbuf (gapi_unsigned_long len);

/*
 * typedef sequence<DataReaderView> DataReaderViewSeq;
 */
#ifndef _GAPI_DATAVIEWSEQ_DEFINED
#define _GAPI_DATAVIEWSEQ_DEFINED
typedef struct {
    gapi_unsigned_long _maximum;
    gapi_unsigned_long _length;
    gapi_dataReaderView *_buffer;
    gapi_boolean _release;
} gapi_dataReaderViewSeq;
#endif /* _GAPI_DATAVIEWSEQ_DEFINED */

OS_API gapi_dataReaderViewSeq *
gapi_dataReaderViewSeq__alloc (void);

OS_API gapi_dataReaderView *
gapi_dataReaderViewSeq_allocbuf (gapi_unsigned_long len);

/*
 * interface Listener { };
 */
struct gapi_listener {
    void *listener_data;
};

/*
 * interface TopicListener : Listener
 */
typedef void (*gapi_listener_InconsistentTopicListener)
    (
    void *listener_data,
    gapi_topic topic,
    const gapi_inconsistentTopicStatus *status);

typedef void (*gapi_listener_AllDataDisposedListener)
    (
    void *listener_data,
    gapi_topic topic);

struct gapi_topicListener {
    void *listener_data;
    gapi_listener_InconsistentTopicListener on_inconsistent_topic;
    gapi_listener_AllDataDisposedListener on_all_data_disposed;
};
OS_API struct gapi_topicListener *
gapi_topicListener__alloc (void);

/*
 * interface DataWriterListener : Listener {
 */
typedef void (*gapi_listener_OfferedDeadlineMissedListener)
    (
    void *listener_data,
    gapi_dataWriter writer,
    const gapi_offeredDeadlineMissedStatus *status);

typedef void (*gapi_listener_LivelinessLostListener)
    (
    void *listener_data,
    gapi_dataWriter writer,
    const gapi_livelinessLostStatus *status);

typedef void (*gapi_listener_OfferedIncompatibleQosListener)
    (
    void *listener_data,
    gapi_dataWriter writer,
    const gapi_offeredIncompatibleQosStatus *status);

typedef void (*gapi_listener_PublicationMatchedListener)
    (
    void *listener_data,
    gapi_dataWriter writer,
    const gapi_publicationMatchedStatus *status);

struct gapi_dataWriterListener {
    void *listener_data;
    gapi_listener_OfferedDeadlineMissedListener on_offered_deadline_missed;
    gapi_listener_OfferedIncompatibleQosListener on_offered_incompatible_qos;
    gapi_listener_LivelinessLostListener on_liveliness_lost;
    gapi_listener_PublicationMatchedListener on_publication_match;
};
OS_API struct gapi_dataWriterListener *gapi_dataWriterListener__alloc (void);

/*
 * interface PublisherListener : DataWriterListener { };
 */
struct gapi_publisherListener {
    void *listener_data;
    gapi_listener_OfferedDeadlineMissedListener on_offered_deadline_missed;
    gapi_listener_OfferedIncompatibleQosListener on_offered_incompatible_qos;
    gapi_listener_LivelinessLostListener on_liveliness_lost;
    gapi_listener_PublicationMatchedListener on_publication_match;
};
OS_API struct gapi_publisherListener *gapi_publisherListener__alloc (void);

/*
 * interface DataReaderListener : Listener
 */
typedef void (*gapi_listener_RequestedDeadlineMissedListener)
    (
    void *listener_data,
    gapi_dataReader reader,
    const gapi_requestedDeadlineMissedStatus *status);

typedef void (*gapi_listener_LivelinessChangedListener)
    (
    void *listener_data,
    gapi_dataReader reader,
    const gapi_livelinessChangedStatus *status);

typedef void (*gapi_listener_RequestedIncompatibleQosListener)
    (
    void *listener_data,
    gapi_dataReader reader,
    const gapi_requestedIncompatibleQosStatus *status);

typedef void (*gapi_listener_SampleRejectedListener)
    (
    void *listener_data,
    gapi_dataReader reader,
    const gapi_sampleRejectedStatus *status);

typedef void (*gapi_listener_DataAvailableListener)
    (
    void *listener_data,
    gapi_dataReader reader);

typedef void (*gapi_listener_SubscriptionMatchedListener)
    (
    void *listener_data,
    gapi_dataReader reader,
    const gapi_subscriptionMatchedStatus *status);

typedef void (*gapi_listener_SampleLostListener)
    (
    void *listener_data,
    gapi_dataReader reader,
    const gapi_sampleLostStatus *status);

struct gapi_dataReaderListener {
    void *listener_data;
    gapi_listener_RequestedDeadlineMissedListener on_requested_deadline_missed;
    gapi_listener_RequestedIncompatibleQosListener on_requested_incompatible_qos;
    gapi_listener_SampleRejectedListener on_sample_rejected;
    gapi_listener_LivelinessChangedListener on_liveliness_changed;
    gapi_listener_DataAvailableListener on_data_available;
    gapi_listener_SubscriptionMatchedListener on_subscription_match;
    gapi_listener_SampleLostListener on_sample_lost;
};

OS_API struct gapi_dataReaderListener *gapi_dataReaderListener__alloc (void);

/*
 * interface SubscriberListener : DataReaderListener {
 */
typedef void (*gapi_listener_DataOnReadersListener)
    (
    void *listener_data,
    gapi_subscriber sub);

struct gapi_subscriberListener {
    void *listener_data;
    gapi_listener_RequestedDeadlineMissedListener on_requested_deadline_missed;
    gapi_listener_RequestedIncompatibleQosListener on_requested_incompatible_qos;
    gapi_listener_SampleRejectedListener on_sample_rejected;
    gapi_listener_LivelinessChangedListener on_liveliness_changed;
    gapi_listener_DataAvailableListener on_data_available;
    gapi_listener_SubscriptionMatchedListener on_subscription_match;
    gapi_listener_SampleLostListener on_sample_lost;
    gapi_listener_DataOnReadersListener on_data_on_readers;
};

OS_API struct gapi_subscriberListener *gapi_subscriberListener__alloc (void);

/*
 * interface DomainParticipantListener : TopicListener, PublisherListener, SubscriberListener
 */
struct gapi_domainParticipantListener {
    void *listener_data;
    gapi_listener_InconsistentTopicListener on_inconsistent_topic;
    gapi_listener_AllDataDisposedListener on_all_data_disposed;
    gapi_listener_OfferedDeadlineMissedListener on_offered_deadline_missed;
    gapi_listener_OfferedIncompatibleQosListener on_offered_incompatible_qos;
    gapi_listener_LivelinessLostListener on_liveliness_lost;
    gapi_listener_PublicationMatchedListener on_publication_match;
    gapi_listener_RequestedDeadlineMissedListener on_requested_deadline_missed;
    gapi_listener_RequestedIncompatibleQosListener on_requested_incompatible_qos;
    gapi_listener_SampleRejectedListener on_sample_rejected;
    gapi_listener_LivelinessChangedListener on_liveliness_changed;
    gapi_listener_DataAvailableListener on_data_available;
    gapi_listener_SubscriptionMatchedListener on_subscription_match;
    gapi_listener_SampleLostListener on_sample_lost;
    gapi_listener_DataOnReadersListener on_data_on_readers;
};
OS_API struct gapi_domainParticipantListener *gapi_domainParticipantListener__alloc (void);

/*
 * interface Condition
 */
typedef gapi_object gapi_condition;

/*
 *     boolean
 *     get_trigger_value();
 */
OS_API gapi_boolean
gapi_condition_get_trigger_value (
    gapi_condition _this);

/*
 * typedef sequence<Condition> ConditionSeq;
 */
#ifndef _GAPI_CONDITIONSEQ_DEFINED
#define _GAPI_CONDITIONSEQ_DEFINED
typedef struct {
    gapi_unsigned_long _maximum;
    gapi_unsigned_long _length;
    gapi_condition *_buffer;
    gapi_boolean _release;
} gapi_conditionSeq;

#endif /* _GAPI_CONDITIONSEQ_DEFINED */

OS_API gapi_conditionSeq *gapi_conditionSeq__alloc (void);
OS_API gapi_condition *gapi_conditionSeq_allocbuf (gapi_unsigned_long len);

/*
 * interface WaitSet
 */
typedef gapi_object gapi_waitSet;

/*     ReturnCode_t
 *     wait(
 *         inout ConditionSeq active_conditions,
 *         in Duration_t timeout);
 */
OS_API gapi_returnCode_t
gapi_waitSet_wait (
    gapi_waitSet _this,
    gapi_conditionSeq *active_conditions,
    const gapi_duration_t *timeout);

/*     ReturnCode_t
 *     attach_condition(
 *         in Condition cond);
 */
OS_API gapi_returnCode_t
gapi_waitSet_attach_condition (
    gapi_waitSet _this,
    const gapi_condition cond);

/*     ReturnCode_t
 *     detach_condition(
 *         in Condition cond);
 */
OS_API gapi_returnCode_t
gapi_waitSet_detach_condition(
    gapi_waitSet _this,
    const gapi_condition cond);

/*     ReturnCode_t
 *     get_conditions(
 *         inout ConditionSeq attached_conditions);
 */
OS_API gapi_returnCode_t
gapi_waitSet_get_conditions(
    gapi_waitSet _this,
    gapi_conditionSeq *attached_conditions);

/*     WaitSet
 *     WaitSet__alloc (
 *         void);
 */
OS_API gapi_waitSet
gapi_waitSet__alloc (
    void);

/*
 * interface GuardCondition : Condition
 */
typedef gapi_object gapi_guardCondition;

/* From Condition
 *     get_trigger_value
 */
#define gapi_guardCondition_get_trigger_value \
        gapi_condition_get_trigger_value

/*     ReturnCode_t
 *     set_trigger_value(
 *         in boolean value);
 * };
 */
OS_API gapi_returnCode_t
gapi_guardCondition_set_trigger_value (
    gapi_guardCondition _this,
    const gapi_boolean value);

/*     GuardCondition
 *     GuardCondition__alloc (
 *         void);
 */
OS_API gapi_guardCondition
gapi_guardCondition__alloc (
    void);

/*
 * interface StatusCondition : Condition {
 */
typedef gapi_object gapi_statusCondition;

/* From Condition
 *     get_trigger_value
 */
#define gapi_statusCondition_get_trigger_value \
        gapi_condition_get_trigger_value

/*     StatusKindMask
 *     get_enabled_statuses();
 */
OS_API gapi_statusMask
gapi_statusCondition_get_enabled_statuses (
    gapi_statusCondition _this);

/*     ReturnCode_t
 *     set_enabled_statuses(
 *         in StatusKindMask mask);
 */
OS_API gapi_returnCode_t
gapi_statusCondition_set_enabled_statuses (
    gapi_statusCondition _this,
    const gapi_statusMask mask);

/*     Entity
 *     get_entity();
 */
OS_API gapi_entity
gapi_statusCondition_get_entity (
    gapi_statusCondition _this);

/*
 * // Sample states to support reads
 * typedef unsigned long SampleStateKind;
 * typedef sequence <SampleStateKind> SampleStateSeq;
 */
typedef gapi_unsigned_long gapi_sampleStateKind;

#ifndef _GAPI_SAMPLESTATESEQ_DEFINED
#define _GAPI_SAMPLESTATESEQ_DEFINED
typedef struct {
    gapi_unsigned_long _maximum;
    gapi_unsigned_long _length;
    gapi_sampleStateKind *_buffer;
    gapi_boolean _release;
} gapi_sampleStateSeq;

#endif /* _GAPI_SAMPLESTATEKINDSEQ_DEFINED */

OS_API gapi_sampleStateSeq *
gapi_sampleStateSeq__alloc (void);

OS_API gapi_sampleStateKind *
gapi_sampleStateSeq_allocbuf (gapi_unsigned_long len);

/*
 * const SampleStateKind READ_SAMPLE_STATE                     = 0x0001 << 0;
 * const SampleStateKind NOT_READ_SAMPLE_STATE                 = 0x0001 << 1;
 */
#define GAPI_READ_SAMPLE_STATE                      1U
#define GAPI_NOT_READ_SAMPLE_STATE                  2U

/* // This is a bit-mask SampleStateKind
 * typedef unsigned long SampleStateMask;
 */
typedef gapi_unsigned_long gapi_sampleStateMask;

/* const SampleStateMask ANY_SAMPLE_STATE                      = 0xffff;
 */
#define GAPI_ANY_SAMPLE_STATE                       65535U

/* // View states to support reads
 * typedef unsigned long ViewStateKind;
 * typedef sequence<ViewStateKind> ViewStateSeq;
 */
typedef gapi_unsigned_long gapi_viewStateKind;

#ifndef _GAPI_VIEWSTATESEQ_DEFINED
#define _GAPI_VIEWSTATESEQ_DEFINED
typedef struct {
    gapi_unsigned_long _maximum;
    gapi_unsigned_long _length;
    gapi_viewStateKind *_buffer;
    gapi_boolean _release;
} gapi_viewStateSeq;

#endif /* _GAPI_VIEWSTATESEQ_DEFINED */

OS_API gapi_viewStateSeq *
gapi_viewStateSeq__alloc (void);

OS_API gapi_viewStateKind *
gapi_viewStateSeq_allocbuf (gapi_unsigned_long len);


/* const ViewStateKind NEW_VIEW_STATE                          = 0x0001 << 0;
 * const ViewStateKind NOT_NEW_VIEW_STATE                      = 0x0001 << 1;
 */
#define GAPI_NEW_VIEW_STATE                         1U
#define GAPI_NOT_NEW_VIEW_STATE                     2U

/* // This is a bit-mask ViewStateKind
 * typedef unsigned long ViewStateMask;
 */
typedef gapi_unsigned_long gapi_viewStateMask;

/* const ViewStateMask ANY_VIEW_STATE                          = 0xffff;
 */
#define GAPI_ANY_VIEW_STATE                         65535U

/* // Instance states to support reads
 * typedef unsigned long InstanceStateKind;
 * typedef sequence<InstanceStateKind> InstanceStateSeq;
 */
typedef gapi_unsigned_long gapi_instanceStateKind;
#ifndef _GAPI_INSTANCESTATESEQ_DEFINED
#define _GAPI_INSTANCESTATESEQ_DEFINED
typedef struct {
    gapi_unsigned_long _maximum;
    gapi_unsigned_long _length;
    gapi_instanceStateKind *_buffer;
    gapi_boolean _release;
} gapi_instanceStateSeq;

#endif /* _GAPI_INSTANCESTATESEQ_DEFINED */

OS_API gapi_instanceStateSeq *
gapi_instanceStateSeq__alloc (void);

OS_API gapi_instanceStateKind *
gapi_instanceStateSeq_allocbuf (gapi_unsigned_long len);

/* const InstanceStateKind ALIVE_INSTANCE_STATE                = 0x0001 << 0;
 * const InstanceStateKind NOT_ALIVE_DISPOSED_INSTANCE_STATE   = 0x0001 << 1;
 * const InstanceStateKind NOT_ALIVE_NO_WRITERS_INSTANCE_STATE = 0x0001 << 2;
 */
#define GAPI_ALIVE_INSTANCE_STATE                       1U
#define GAPI_NOT_ALIVE_DISPOSED_INSTANCE_STATE          2U
#define GAPI_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE        4U

/* // This is a bit-mask InstanceStateKind
 * typedef unsigned long InstanceStateMask;
 */
typedef gapi_unsigned_long gapi_instanceStateMask;

/* const InstanceStateMask ANY_INSTANCE_STATE                  = 0xffff;
 * const InstanceStateMask NOT_ALIVE_INSTANCE_STATE            = 0x006;
 */
#define GAPI_ANY_INSTANCE_STATE                         65535U
#define GAPI_NOT_ALIVE_INSTANCE_STATE                   6U

/*
 * interface ReadCondition : Condition
 */
typedef gapi_object gapi_readCondition;

/* From Condition
 *     get_trigger_value
 */
#define gapi_readCondition_get_trigger_value \
        gapi_condition_get_trigger_value

/*     SampleStateMask
 *     get_sample_state_mask();
 */
OS_API gapi_sampleStateMask
gapi_readCondition_get_sample_state_mask (
    gapi_readCondition _this);
/*     ViewStateMask
 *     get_view_state_mask();
 */
OS_API gapi_viewStateMask
gapi_readCondition_get_view_state_mask (
    gapi_readCondition _this);

/*     InstanceStateMask
 *     get_instance_state_mask();
 */
OS_API gapi_instanceStateMask
gapi_readCondition_get_instance_state_mask (
    gapi_readCondition _this);

/*     DataReader
 *     get_datareader();
 */
OS_API gapi_dataReader
gapi_readCondition_get_datareader(
    gapi_readCondition _this);

/*     DataReader
 *     get_datareaderview();
 */
OS_API gapi_dataReaderView
gapi_readCondition_get_datareaderview(
    gapi_readCondition _this);

/*
 * interface QueryCondition : ReadCondition
 */
typedef gapi_object gapi_queryCondition;

/* From Condition
 *     get_trigger_value
 */
#define gapi_queryCondition_get_trigger_value \
        gapi_condition_get_trigger_value

/* From ReadCondition
 *     get_sample_state_mask
 */
#define gapi_queryCondition_get_sample_state_mask \
        gapi_readCondition_get_sample_state_mask

/* From ReadCondition
 *     get_view_state_mask
 */
#define gapi_queryCondition_get_view_state_mask \
        gapi_readCondition_get_view_state_mask

/* From ReadCondition
 *     get_instance_state_mask
 */
#define gapi_queryCondition_get_instance_state_mask \
        gapi_readCondition_get_instance_state_mask

/* From ReadCondition
 *     get_datareader
 */
#define gapi_queryCondition_get_datareader \
        gapi_readCondition_get_datareader

/* From ReadCondition
 *     get_datareader
 */
#define gapi_queryCondition_get_datareaderview \
        gapi_readCondition_get_datareaderview

/*     string
 *     get_query_expression();
 */
OS_API gapi_string
gapi_queryCondition_get_query_expression (
    gapi_queryCondition _this);

/*     ReturnCode_t
 *       get_query_parameters(
 *             inout StringSeq query_parameters);
 */
OS_API gapi_returnCode_t
gapi_queryCondition_get_query_parameters (
    gapi_queryCondition _this,
    gapi_stringSeq * query_parameters);

/*     ReturnCode_t
 *     set_query_parameters(
 *         in StringSeq query_parameters);
 */
OS_API gapi_returnCode_t
gapi_queryCondition_set_query_parameters (
    gapi_queryCondition _this,
    const gapi_stringSeq *query_parameters);

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
 * const string LIFESPAN_QOS_POLICY_NAME               = "Lifespan";
 * const string DURABILITYSERVICE_QOS_POLICY_NAME      = "DurabilityService";
 * const string SUBSCRIPTIONKEY_QOS_POLICY_NAME        = "SubscriptionKey"
 * const string VIEWKEY_QOS_POLICY_NAME                = "ViewKey"
 * const string READERLIFESPAN_QOS_POLICY_NAME         = "ReaderLifespan"
 * const string SHARE_QOS_POLICY_NAME                  = "Share"
 * const string SCHEDULING_QOS_POLICY_NAME             = "Scheduling"
 */
#define GAPI_USERDATA_QOS_POLICY_NAME                   "UserData"
#define GAPI_DURABILITY_QOS_POLICY_NAME                 "Durability"
#define GAPI_PRESENTATION_QOS_POLICY_NAME               "Presentation"
#define GAPI_DEADLINE_QOS_POLICY_NAME                   "Deadline"
#define GAPI_LATENCYBUDGET_QOS_POLICY_NAME              "LatencyBudget"
#define GAPI_OWNERSHIP_QOS_POLICY_NAME                  "Ownership"
#define GAPI_OWNERSHIPSTRENGTH_QOS_POLICY_NAME          "OwnershipStrength"
#define GAPI_LIVELINESS_QOS_POLICY_NAME                 "Liveliness"
#define GAPI_TIMEBASEDFILTER_QOS_POLICY_NAME            "TimeBasedFilter"
#define GAPI_PARTITION_QOS_POLICY_NAME                  "Partition"
#define GAPI_RELIABILITY_QOS_POLICY_NAME                "Reliability"
#define GAPI_DESTINATIONORDER_QOS_POLICY_NAME           "DestinationOrder"
#define GAPI_HISTORY_QOS_POLICY_NAME                    "History"
#define GAPI_RESOURCELIMITS_QOS_POLICY_NAME             "ResourceLimits"
#define GAPI_ENTITYFACTORY_QOS_POLICY_NAME              "EntityFactory"
#define GAPI_WRITERDATALIFECYCLE_QOS_POLICY_NAME        "WriterDataLifecycle"
#define GAPI_READERDATALIFECYCLE_QOS_POLICY_NAME        "ReaderDataLifecycle"
#define GAPI_TOPICDATA_QOS_POLICY_NAME                  "TopicData"
#define GAPI_GROUPDATA_QOS_POLICY_NAME                  "GroupData"
#define GAPI_TRANSPORTPRIORITY_QOS_POLICY_NAME          "TransportPriority"
#define GAPI_LIFESPAN_QOS_POLICY_NAME                   "Lifespan"
#define GAPI_DURABILITYSERVICE_QOS_POLICY_NAME          "DurabilityService"
#define GAPI_SUBSCRIPTIONKEY_QOS_POLICY_NAME            "SubscriptionKey"
#define GAPI_VIEWKEY_QOS_POLICY_NAME                    "ViewKey"
#define GAPI_READERLIFESPAN_QOS_POLICY_NAME             "ReaderLifespan"
#define GAPI_SHARE_QOS_POLICY_NAME                      "Share"
#define GAPI_SCHEDULING_QOS_POLICY_NAME                 "Scheduling"

/* const QosPolicyId_t INVALID_QOS_POLICY_ID                   = 0;
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
#define GAPI_INVALID_QOS_POLICY_ID                      0
#define GAPI_USERDATA_QOS_POLICY_ID                     1
#define GAPI_DURABILITY_QOS_POLICY_ID                   2
#define GAPI_PRESENTATION_QOS_POLICY_ID                 3
#define GAPI_DEADLINE_QOS_POLICY_ID                     4
#define GAPI_LATENCYBUDGET_QOS_POLICY_ID                5
#define GAPI_OWNERSHIP_QOS_POLICY_ID                    6
#define GAPI_OWNERSHIPSTRENGTH_QOS_POLICY_ID            7
#define GAPI_LIVELINESS_QOS_POLICY_ID                   8
#define GAPI_TIMEBASEDFILTER_QOS_POLICY_ID              9
#define GAPI_PARTITION_QOS_POLICY_ID                    10
#define GAPI_RELIABILITY_QOS_POLICY_ID                  11
#define GAPI_DESTINATIONORDER_QOS_POLICY_ID             12
#define GAPI_HISTORY_QOS_POLICY_ID                      13
#define GAPI_RESOURCELIMITS_QOS_POLICY_ID               14
#define GAPI_ENTITYFACTORY_QOS_POLICY_ID                15
#define GAPI_WRITERDATALIFECYCLE_QOS_POLICY_ID          16
#define GAPI_READERDATALIFECYCLE_QOS_POLICY_ID          17
#define GAPI_TOPICDATA_QOS_POLICY_ID                    18
#define GAPI_GROUPDATA_QOS_POLICY_ID                    19
#define GAPI_TRANSPORTPRIORITY_QOS_POLICY_ID            20
#define GAPI_LIFESPAN_QOS_POLICY_ID                     21
#define GAPI_DURABILITYSERVICE_QOS_POLICY_ID            22
#define GAPI_SUBSCRIPTIONKEY_QOS_POLICY_ID              23
#define GAPI_VIEWKEY_QOS_POLICY_ID                      24
#define GAPI_READERLIFESPAN_QOS_POLICY_ID               25
#define GAPI_SHARE_QOS_POLICY_ID                        26
#define GAPI_SCHEDULING_QOS_POLICY_ID                   27


/*
 * struct UserDataQosPolicy {
 *     sequence<octet> value;
 * };
 */
#ifndef _GAPI_OCTETSEQ_DEFINED
#define _GAPI_OCTETSEQ_DEFINED
typedef struct {
    gapi_unsigned_long _maximum;
    gapi_unsigned_long _length;
    gapi_octet *_buffer;
    gapi_boolean _release;
} gapi_octetSeq;

OS_API gapi_octetSeq *gapi_octetSeq__alloc (void);
OS_API gapi_octet *gapi_octetSeq_allocbuf (gapi_unsigned_long len);
#endif /* _GAPI_OCTETSEQ_DEFINED */

typedef C_STRUCT(gapi_userDataQosPolicy) {
    gapi_octetSeq value;
} gapi_userDataQosPolicy;

/*
 * struct TopicDataQosPolicy {
 *     sequence<octet> value;
 * };
 */
typedef C_STRUCT(gapi_topicDataQosPolicy) {
    gapi_octetSeq value;
} gapi_topicDataQosPolicy;

/* struct GroupDataQosPolicy {
 *     sequence<octet> value;
 * };
 */
typedef C_STRUCT(gapi_groupDataQosPolicy) {
    gapi_octetSeq value;
} gapi_groupDataQosPolicy;

/* struct TransportPriorityQosPolicy {
 *     long value;
 * };
 */
typedef C_STRUCT(gapi_transportPriorityQosPolicy) {
    gapi_long value;
} gapi_transportPriorityQosPolicy;

/* struct LifespanQosPolicy {
 *     Duration_t duration;
 * };
 */
typedef C_STRUCT(gapi_lifespanQosPolicy) {
    gapi_duration_t duration;
} gapi_lifespanQosPolicy;

/* enum DurabilityQosPolicyKind {
 *     VOLATILE_DURABILITY_QOS,
 *     TRANSIENT_LOCAL_DURABILITY_QOS,
 *     TRANSIENT_DURABILITY_QOS,
 *     PERSISTENT_DURABILITY_QOS
 * };
 */
typedef enum
{
    GAPI_VOLATILE_DURABILITY_QOS,
    GAPI_TRANSIENT_LOCAL_DURABILITY_QOS,
    GAPI_TRANSIENT_DURABILITY_QOS,
    GAPI_PERSISTENT_DURABILITY_QOS
} gapi_durabilityQosPolicyKind;

/* struct DurabilityQosPolicy {
 *     DurabilityQosPolicyKind kind;
 *     Duration_t service_cleanup_delay;
 * };
 */
typedef C_STRUCT(gapi_durabilityQosPolicy) {
    gapi_durabilityQosPolicyKind kind;
} gapi_durabilityQosPolicy;

/* enum PresentationQosPolicyAccessScopeKind {
 *     INSTANCE_PRESENTATION_QOS,
 *     TOPIC_PRESENTATION_QOS,
 *     GROUP_PRESENTATION_QOS
 * };
 */
typedef enum
{
    GAPI_INSTANCE_PRESENTATION_QOS,
    GAPI_TOPIC_PRESENTATION_QOS,
    GAPI_GROUP_PRESENTATION_QOS
} gapi_presentationQosPolicyAccessScopeKind;

/* struct PresentationQosPolicy {
 *     PresentationQosPolicyAccessScopeKind access_scope;
 *     boolean coherent_access;
 *     boolean ordered_access;
 * };
 */
typedef C_STRUCT(gapi_presentationQosPolicy) {
    gapi_presentationQosPolicyAccessScopeKind access_scope;
    gapi_boolean coherent_access;
    gapi_boolean ordered_access;
} gapi_presentationQosPolicy;

/* struct DeadlineQosPolicy {
 *     Duration_t period;
 * };
 */
typedef C_STRUCT(gapi_deadlineQosPolicy) {
    gapi_duration_t period;
} gapi_deadlineQosPolicy;

/* struct LatencyBudgetQosPolicy {
 *     Duration_t duration;
 * };
 */
typedef C_STRUCT(gapi_latencyBudgetQosPolicy) {
    gapi_duration_t duration;
} gapi_latencyBudgetQosPolicy;

/* enum OwnershipQosPolicyKind {
 *     SHARED_OWNERSHIP_QOS,
 *     EXCLUSIVE_OWNERSHIP_QOS
 * };
 */
typedef enum
{
    GAPI_SHARED_OWNERSHIP_QOS,
    GAPI_EXCLUSIVE_OWNERSHIP_QOS
} gapi_ownershipQosPolicyKind;

/* struct OwnershipQosPolicy {
 *     OwnershipQosPolicyKind kind;
 * };
 */
typedef C_STRUCT(gapi_ownershipQosPolicy) {
    gapi_ownershipQosPolicyKind kind;
} gapi_ownershipQosPolicy;

/* struct OwnershipStrengthQosPolicy {
 *     long value;
 * };
 */
typedef C_STRUCT(gapi_ownershipStrengthQosPolicy) {
    gapi_long value;
} gapi_ownershipStrengthQosPolicy;

/* enum LivelinessQosPolicyKind {
 *     AUTOMATIC_LIVELINESS_QOS,
 *     MANUAL_BY_PARTICIPANT_LIVELINESS_QOS,
 *     MANUAL_BY_TOPIC_LIVELINESS_QOS
 * };
 */
typedef enum
{
    GAPI_AUTOMATIC_LIVELINESS_QOS,
    GAPI_MANUAL_BY_PARTICIPANT_LIVELINESS_QOS,
    GAPI_MANUAL_BY_TOPIC_LIVELINESS_QOS
} gapi_livelinessQosPolicyKind;

/* struct LivelinessQosPolicy {
 *     LivelinessQosPolicyKind kind;
 *     Duration_t lease_duration;
 * };
 */
typedef C_STRUCT(gapi_livelinessQosPolicy) {
    gapi_livelinessQosPolicyKind kind;
    gapi_duration_t lease_duration;
} gapi_livelinessQosPolicy;

/* struct TimeBasedFilterQosPolicy {
 *     Duration_t minimum_separation;
 * };
 */
typedef C_STRUCT(gapi_timeBasedFilterQosPolicy) {
    gapi_duration_t minimum_separation;
} gapi_timeBasedFilterQosPolicy;

/* struct PartitionQosPolicy {
 *     StringSeq name;
 * };
 */
typedef C_STRUCT(gapi_partitionQosPolicy) {
    gapi_stringSeq name;
} gapi_partitionQosPolicy;

/* struct ShareQosPolicy {
 *     String name;
 *     Boolean enable;
 * };
 */
typedef C_STRUCT(gapi_shareQosPolicy) {
    gapi_string name;
    gapi_boolean enable;
} gapi_shareQosPolicy;

/* enum ReliabilityQosPolicyKind {
 *     BEST_EFFORT_RELIABILITY_QOS,
 *     RELIABLE_RELIABILITY_QOS
 * };
 */
typedef enum
{
    GAPI_BEST_EFFORT_RELIABILITY_QOS,
    GAPI_RELIABLE_RELIABILITY_QOS
} gapi_reliabilityQosPolicyKind;

/* struct ReliabilityQosPolicy {
 *     ReliabilityQosPolicyKind kind;
 *     Duration_t max_blocking_time;
 *     boolean synchronous;
 * };
 */
typedef C_STRUCT(gapi_reliabilityQosPolicy) {
    gapi_reliabilityQosPolicyKind kind;
    gapi_duration_t max_blocking_time;
    gapi_boolean synchronous;
} gapi_reliabilityQosPolicy;

/* enum DestinationOrderQosPolicyKind {
 *     BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS,
 *     BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS
 * };
 */
typedef enum
{
    GAPI_BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS,
    GAPI_BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS
} gapi_destinationOrderQosPolicyKind;

/* struct DestinationOrderQosPolicy {
 *     DestinationOrderQosPolicyKind kind;
 * };
 */
typedef C_STRUCT(gapi_destinationOrderQosPolicy) {
    gapi_destinationOrderQosPolicyKind kind;
} gapi_destinationOrderQosPolicy;

/* enum HistoryQosPolicyKind {
 *     KEEP_LAST_HISTORY_QOS,
 *     KEEP_ALL_HISTORY_QOS
 * };
 */
typedef enum
{
    GAPI_KEEP_LAST_HISTORY_QOS,
    GAPI_KEEP_ALL_HISTORY_QOS
} gapi_historyQosPolicyKind;

/* struct HistoryQosPolicy {
 *     HistoryQosPolicyKind kind;
 *     long depth;
 * };
 */
typedef C_STRUCT(gapi_historyQosPolicy) {
    gapi_historyQosPolicyKind kind;
    gapi_long depth;
} gapi_historyQosPolicy;

/* struct ResourceLimitsQosPolicy {
 *     long max_samples;
 *     long max_instances;
 *     long max_samples_per_instance;
 * };
 */
typedef C_STRUCT(gapi_resourceLimitsQosPolicy) {
    gapi_long max_samples;
    gapi_long max_instances;
    gapi_long max_samples_per_instance;
} gapi_resourceLimitsQosPolicy;

/* struct DurabilityServiceQosPolicy {
 *     HistoryQosPolicyKind history_kind;
 *     long history_depth;
 *     long max_samples;
 *     long max_instances;
 *     long max_samples_per_instance;
 *     Duration_t service_cleanup_delay;
 * };
 */
typedef C_STRUCT(gapi_durabilityServiceQosPolicy) {
    gapi_duration_t service_cleanup_delay;
    gapi_historyQosPolicyKind history_kind;
    gapi_long history_depth;
    gapi_long max_samples;
    gapi_long max_instances;
    gapi_long max_samples_per_instance;
} gapi_durabilityServiceQosPolicy;

/* struct EntityFactoryQosPolicy {
 *     boolean autoenable_created_entities;
 * };
 */
typedef C_STRUCT(gapi_entityFactoryQosPolicy) {
    gapi_boolean autoenable_created_entities;
} gapi_entityFactoryQosPolicy;

/* struct WriterDataLifecycleQosPolicy {
 *     boolean autodispose_unregistered_instances;
 * };
 */
typedef C_STRUCT(gapi_writerDataLifecycleQosPolicy) {
    gapi_boolean autodispose_unregistered_instances;
    gapi_duration_t autopurge_suspended_samples_delay;
    gapi_duration_t autounregister_instance_delay;
} gapi_writerDataLifecycleQosPolicy;

/* struct ReaderDataLifecycleQosPolicy {
 *     Duration_t autopurge_nowriter_samples_delay;
 * };
 */
typedef C_STRUCT(gapi_readerDataLifecycleQosPolicy) {
    gapi_duration_t autopurge_nowriter_samples_delay;
    gapi_duration_t autopurge_disposed_samples_delay;
    gapi_boolean enable_invalid_samples;
} gapi_readerDataLifecycleQosPolicy;


/* struct SubscriptionKeyQosPolicy {
 *     Boolean   use_key_list;
 *     StringSeq key_list;
 * };
 */
typedef C_STRUCT(gapi_subscriptionKeyQosPolicy) {
    gapi_boolean   use_key_list;
    gapi_stringSeq key_list;
} gapi_subscriptionKeyQosPolicy;

/* struct ViewKeyQosPolicy {
 *     Boolean   use_key_list;
 *     StringSeq key_list;
 * };
 */
typedef C_STRUCT(gapi_viewKeyQosPolicy) {
    gapi_boolean   use_key_list;
    gapi_stringSeq key_list;
} gapi_viewKeyQosPolicy;

/* struct ReaderLifespanQosPolicy {
 *     Boolean    use_lifespan;
 *     Duration_t duration;
 * };
 */
typedef C_STRUCT(gapi_readerLifespanQosPolicy) {
    gapi_boolean    use_lifespan;
    gapi_duration_t duration;
} gapi_readerLifespanQosPolicy;

/* enum SchedulingClassQosPolicyKind {
 *     SCHEDULE_DEFAULT,
 *     SCHEDULE_TIMESHARING,
 *     SCHEDULE_REALTIME
 * };
 */
typedef enum {
    GAPI_SCHEDULE_DEFAULT,
    GAPI_SCHEDULE_TIMESHARING,
    GAPI_SCHEDULE_REALTIME
} gapi_schedulingClassQosPolicyKind;

/* struct SchedulingClassQosPolicy {
 *     SchedulingClassQosPolicyKind kind;
 * };
 */
typedef C_STRUCT(gapi_schedulingClassQosPolicy) {
    gapi_schedulingClassQosPolicyKind kind;
} gapi_schedulingClassQosPolicy;

/* enum SchedulingPriorityQosPolicyKind {
 *     PRIORITY_RELATIVE,
 *     PRIORITY_ABSOLUTE
 * };
 */
typedef enum {
    GAPI_PRIORITY_RELATIVE,
    GAPI_PRIORITY_ABSOLUTE
} gapi_schedulingPriorityQosPolicyKind;

/* struct SchedulingPriorityQosPolicy {
 *     SchedulingPriorityQosPolicyKind kind;
 * };
 */
typedef C_STRUCT(gapi_schedulingPriorityQosPolicy) {
    gapi_schedulingPriorityQosPolicyKind kind;
} gapi_schedulingPriorityQosPolicy;

/* struct SchedulingQosPolicy {
 *     SchedulingClassQosPolicy    scheduling_class;
 *     SchedulingPriorityQosPolicy scheduling_priority_kind;
 *     long                        scheduling_priority;
 * };
 */
typedef C_STRUCT(gapi_schedulingQosPolicy) {
    gapi_schedulingClassQosPolicy    scheduling_class;
    gapi_schedulingPriorityQosPolicy scheduling_priority_kind;
    gapi_long                        scheduling_priority;
} gapi_schedulingQosPolicy;

/*
 * struct DomainParticipantFactoryQos {
 *     UserDataQosPolicy user_data;
 *     EntityFactoryQosPolicy entity_factory;
 * };
 */
typedef struct gapi_domainParticipantFactoryQos_s gapi_domainParticipantFactoryQos;
struct gapi_domainParticipantFactoryQos_s {
    gapi_entityFactoryQosPolicy entity_factory;
};
OS_API gapi_domainParticipantFactoryQos *gapi_domainParticipantFactoryQos__alloc (void);

/*
 * struct DomainParticipantQos {
 *     UserDataQosPolicy user_data;
 *     EntityFactoryQosPolicy entity_factory;
 * };
 */
typedef C_STRUCT(gapi_domainParticipantQos) {
    gapi_userDataQosPolicy user_data;
    gapi_entityFactoryQosPolicy entity_factory;
    gapi_schedulingQosPolicy watchdog_scheduling;
    gapi_schedulingQosPolicy listener_scheduling;
} gapi_domainParticipantQos;

OS_API gapi_domainParticipantQos *
gapi_domainParticipantQos__alloc (void);

/*
 * struct TopicQos {
 *     TopicDataQosPolicy topic_data;
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
 *     OwnershipQosPolicy ownership;
 * };
 */
typedef C_STRUCT(gapi_topicQos) {
    gapi_topicDataQosPolicy topic_data;
    gapi_durabilityQosPolicy durability;
    gapi_durabilityServiceQosPolicy durability_service;
    gapi_deadlineQosPolicy deadline;
    gapi_latencyBudgetQosPolicy latency_budget;
    gapi_livelinessQosPolicy liveliness;
    gapi_reliabilityQosPolicy reliability;
    gapi_destinationOrderQosPolicy destination_order;
    gapi_historyQosPolicy history;
    gapi_resourceLimitsQosPolicy resource_limits;
    gapi_transportPriorityQosPolicy transport_priority;
    gapi_lifespanQosPolicy lifespan;
    gapi_ownershipQosPolicy ownership;
} gapi_topicQos;

OS_API gapi_topicQos *
gapi_topicQos__alloc (void);

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
typedef C_STRUCT(gapi_dataWriterQos) {
    gapi_durabilityQosPolicy durability;
    gapi_deadlineQosPolicy deadline;
    gapi_latencyBudgetQosPolicy latency_budget;
    gapi_livelinessQosPolicy liveliness;
    gapi_reliabilityQosPolicy reliability;
    gapi_destinationOrderQosPolicy destination_order;
    gapi_historyQosPolicy history;
    gapi_resourceLimitsQosPolicy resource_limits;
    gapi_transportPriorityQosPolicy transport_priority;
    gapi_lifespanQosPolicy lifespan;
    gapi_userDataQosPolicy user_data;
    gapi_ownershipQosPolicy ownership;
    gapi_ownershipStrengthQosPolicy ownership_strength;
    gapi_writerDataLifecycleQosPolicy writer_data_lifecycle;
} gapi_dataWriterQos;

OS_API gapi_dataWriterQos *
gapi_dataWriterQos__alloc (void);

/*
 * struct PublisherQos {
 *     PresentationQosPolicy presentation;
 *     PartitionQosPolicy partition;
 *     GroupDataQosPolicy group_data;
 *     EntityFactoryQosPolicy entity_factory;
 * };
 */
typedef C_STRUCT(gapi_publisherQos) {
    gapi_presentationQosPolicy presentation;
    gapi_partitionQosPolicy partition;
    gapi_groupDataQosPolicy group_data;
    gapi_entityFactoryQosPolicy entity_factory;
} gapi_publisherQos;

OS_API gapi_publisherQos *
gapi_publisherQos__alloc (void);

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
 *     ReaderLifespanQosPolicy reader_lifespan;
 *     ShareQosPolicy share;
 * };
 */
typedef C_STRUCT(gapi_dataReaderQos) {
    gapi_durabilityQosPolicy durability;
    gapi_deadlineQosPolicy deadline;
    gapi_latencyBudgetQosPolicy latency_budget;
    gapi_livelinessQosPolicy liveliness;
    gapi_reliabilityQosPolicy reliability;
    gapi_destinationOrderQosPolicy destination_order;
    gapi_historyQosPolicy history;
    gapi_resourceLimitsQosPolicy resource_limits;
    gapi_userDataQosPolicy user_data;
    gapi_ownershipQosPolicy ownership;
    gapi_timeBasedFilterQosPolicy time_based_filter;
    gapi_readerDataLifecycleQosPolicy reader_data_lifecycle;
    gapi_subscriptionKeyQosPolicy subscription_keys;
    gapi_readerLifespanQosPolicy reader_lifespan;
    gapi_shareQosPolicy share;
} gapi_dataReaderQos;

OS_API gapi_dataReaderQos *
gapi_dataReaderQos__alloc (void);

/*
 * struct DataReaderViewQos {
 *     SubscriptionKeyQosPolicy subscription_keys;
 * };
 */
typedef C_STRUCT(gapi_dataReaderViewQos) {
    gapi_viewKeyQosPolicy view_keys;
} gapi_dataReaderViewQos;

OS_API gapi_dataReaderViewQos *
gapi_dataReaderViewQos__alloc (void);

/*
 * struct SubscriberQos {
 *     PresentationQosPolicy presentation;
 *     PartitionQosPolicy partition;
 *     GroupDataQosPolicy group_data;
 *     EntityFactoryQosPolicy entity_factory;
 *     ShareQosPolicy share;
 * };
 */
typedef C_STRUCT(gapi_subscriberQos) {
    gapi_presentationQosPolicy presentation;
    gapi_partitionQosPolicy partition;
    gapi_groupDataQosPolicy group_data;
    gapi_entityFactoryQosPolicy entity_factory;
    gapi_shareQosPolicy share;
} gapi_subscriberQos;

OS_API gapi_subscriberQos *
gapi_subscriberQos__alloc (void);

/*
 * struct ParticipantBuiltinTopicData {
 *     BuiltinTopicKey_t key;
 *     UserDataQosPolicy user_data;
 * };
 */
typedef C_STRUCT(gapi_participantBuiltinTopicData) {
    gapi_builtinTopicKey_t key;
    gapi_userDataQosPolicy user_data;
} gapi_participantBuiltinTopicData;

OS_API gapi_participantBuiltinTopicData *
gapi_participantBuiltinTopicData__alloc (void);

#ifndef _GAPI_PARTICIPANTBUILTINTOPICDATASEQ_DEFINED
#define _GAPI_PARTICIPANTBUILTINTOPICDATASEQ_DEFINED
typedef struct {
    gapi_unsigned_long _maximum;
    gapi_unsigned_long _length;
    gapi_participantBuiltinTopicData *_buffer;
    gapi_boolean _release;
} gapi_participantBuiltinTopicDataSeq;

OS_API gapi_participantBuiltinTopicDataSeq *
gapi_participantBuiltinTopicDataSeq__alloc (void);

OS_API gapi_participantBuiltinTopicData *
gapi_participantBuiltinTopicDataSeq_allocbuf (gapi_unsigned_long len);
#endif /* _GAPI_PARTICIPANTBUILTINTOPICDATASEQ_DEFINED */

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
typedef C_STRUCT(gapi_topicBuiltinTopicData) {
    gapi_builtinTopicKey_t key;
    gapi_string name;
    gapi_string type_name;
    gapi_durabilityQosPolicy durability;
    gapi_durabilityServiceQosPolicy durability_service;
    gapi_deadlineQosPolicy deadline;
    gapi_latencyBudgetQosPolicy latency_budget;
    gapi_livelinessQosPolicy liveliness;
    gapi_reliabilityQosPolicy reliability;
    gapi_transportPriorityQosPolicy transport_priority;
    gapi_lifespanQosPolicy lifespan;
    gapi_destinationOrderQosPolicy destination_order;
    gapi_historyQosPolicy history;
    gapi_resourceLimitsQosPolicy resource_limits;
    gapi_ownershipQosPolicy ownership;
    gapi_topicDataQosPolicy topic_data;
} gapi_topicBuiltinTopicData;

OS_API gapi_topicBuiltinTopicData *
gapi_topicBuiltinTopicData__alloc (void);

#ifndef _GAPI_TOPICBUILTINTOPICDATASEQ_DEFINED
#define _GAPI_TOPICBUILTINTOPICDATASEQ_DEFINED
typedef struct {
    gapi_unsigned_long _maximum;
    gapi_unsigned_long _length;
    gapi_topicBuiltinTopicData *_buffer;
    gapi_boolean _release;
} gapi_topicBuiltinTopicDataSeq;

OS_API gapi_topicBuiltinTopicDataSeq *
gapi_topicBuiltinTopicDataSeq__alloc (void);

OS_API gapi_topicBuiltinTopicData *
gapi_topicBuiltinTopicDataSeq_allocbuf (gapi_unsigned_long len);
#endif /* _GAPI_TOPICBUILTINTOPICDATASEQ_DEFINED */

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
typedef C_STRUCT(gapi_publicationBuiltinTopicData) {
    gapi_builtinTopicKey_t key;
    gapi_builtinTopicKey_t participant_key;
    gapi_string topic_name;
    gapi_string type_name;
    gapi_durabilityQosPolicy durability;
    gapi_deadlineQosPolicy deadline;
    gapi_latencyBudgetQosPolicy latency_budget;
    gapi_livelinessQosPolicy liveliness;
    gapi_reliabilityQosPolicy reliability;
    gapi_lifespanQosPolicy lifespan;
    gapi_destinationOrderQosPolicy destination_order;
    gapi_userDataQosPolicy user_data;
    gapi_ownershipQosPolicy ownership;
    gapi_ownershipStrengthQosPolicy ownership_strength;
    gapi_presentationQosPolicy presentation;
    gapi_partitionQosPolicy partition;
    gapi_topicDataQosPolicy topic_data;
    gapi_groupDataQosPolicy group_data;
} gapi_publicationBuiltinTopicData;

OS_API gapi_publicationBuiltinTopicData *
gapi_publicationBuiltinTopicData__alloc (void);

#ifndef _GAPI_PUBLICATIONBUILTINTOPICDATASEQ_DEFINED
#define _GAPI_PUBLICATIONBUILTINTOPICDATASEQ_DEFINED
typedef struct {
    gapi_unsigned_long _maximum;
    gapi_unsigned_long _length;
    gapi_publicationBuiltinTopicData *_buffer;
    gapi_boolean _release;
} gapi_publicationBuiltinTopicDataSeq;

OS_API gapi_publicationBuiltinTopicDataSeq *
gapi_publicationBuiltinTopicDataSeq__alloc (void);

OS_API gapi_publicationBuiltinTopicData *
gapi_publicationBuiltinTopicDataSeq_allocbuf (gapi_unsigned_long len);
#endif /* _GAPI_PUBLICATIONBUILTINTOPICDATASEQ_DEFINED */

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
 *     DestinationOrderQosPolicy destination_order;
 *     UserDataQosPolicy user_data;
 *     OwnershipQosPolicy ownership;
 *     TimeBasedFilterQosPolicy time_based_filter;
 *     PresentationQosPolicy presentation;
 *     PartitionQosPolicy partition;
 *     TopicDataQosPolicy topic_data;
 *     GroupDataQosPolicy group_data;
 * };
 */
typedef C_STRUCT(gapi_subscriptionBuiltinTopicData) {
    gapi_builtinTopicKey_t key;
    gapi_builtinTopicKey_t participant_key;
    gapi_string topic_name;
    gapi_string type_name;
    gapi_durabilityQosPolicy durability;
    gapi_deadlineQosPolicy deadline;
    gapi_latencyBudgetQosPolicy latency_budget;
    gapi_livelinessQosPolicy liveliness;
    gapi_reliabilityQosPolicy reliability;
    gapi_ownershipQosPolicy ownership;
    gapi_destinationOrderQosPolicy destination_order;
    gapi_userDataQosPolicy user_data;
    gapi_timeBasedFilterQosPolicy time_based_filter;
    gapi_presentationQosPolicy presentation;
    gapi_partitionQosPolicy partition;
    gapi_topicDataQosPolicy topic_data;
    gapi_groupDataQosPolicy group_data;
} gapi_subscriptionBuiltinTopicData;

OS_API gapi_subscriptionBuiltinTopicData *
gapi_subscriptionBuiltinTopicData__alloc (void);

#ifndef _GAPI_SUBSCRIPTIONBUILTINTOPICDATASEQ_DEFINED
#define _GAPI_SUBSCRIPTIONBUILTINTOPICDATASEQ_DEFINED
typedef struct {
    gapi_unsigned_long _maximum;
    gapi_unsigned_long _length;
    gapi_subscriptionBuiltinTopicData *_buffer;
    gapi_boolean _release;
} gapi_subscriptionBuiltinTopicDataSeq;

OS_API gapi_subscriptionBuiltinTopicDataSeq *
gapi_subscriptionBuiltinTopicDataSeq__alloc (void);

OS_API gapi_subscriptionBuiltinTopicData *
gapi_subscriptionBuiltinTopicDataSeq_allocbuf (gapi_unsigned_long len);
#endif /* _GAPI_SUBSCRIPTIONBUILTINTOPICDATASEQ_DEFINED */

OS_API void
gapi_participantBuiltinTopicData__copyOut (
    void *_from,
    void *_to);

OS_API gapi_boolean
gapi_participantBuiltinTopicData__copyIn (
    c_base base,
    void *_from,
    void *_to);

OS_API void
gapi_topicBuiltinTopicData__copyOut (
    void *_from,
    void *_to);

OS_API gapi_boolean
gapi_topicBuiltinTopicData__copyIn (
    c_base base,
    void *_from,
    void *_to);

OS_API void
gapi_publicationBuiltinTopicData__copyOut (
    void *_from,
    void *_to);

OS_API gapi_boolean
gapi_publicationBuiltinTopicData__copyIn (
    c_base base,
    void *_from,
    void *_to);

OS_API void
gapi_subscriptionBuiltinTopicData__copyOut (
    void *_from,
    void *_to);

OS_API gapi_boolean
gapi_subscriptionBuiltinTopicData__copyIn (
    c_base base,
    void *_from,
    void *_to);

/*
 * interface Entity {
 */

/*
 * // Abstract methods
 *
 * //  ReturnCode_t
 * //  set_qos(
 * //      in EntityQos qos);
 * //
 * //  void
 * //  get_qos(
 * //      inout EntityQos qos);
 * //
 * //  ReturnCode_t
 * //  set_listener(
 * //      in Listener l,
 * //      in StatusKindMask mask);
 * //
 * //  Listener
 * //  get_listener();
 */

/*
 *     ReturnCode_t
 *     enable();
 */
OS_API gapi_returnCode_t
gapi_entity_enable (
    gapi_entity _this);

/*
 *     StatusCondition
 *     get_statuscondition();
 */
OS_API gapi_statusCondition
gapi_entity_get_statuscondition (
    gapi_entity _this);

/*
 *     StatusKindMask
 *     get_status_changes();
 */
OS_API gapi_statusMask
gapi_entity_get_status_changes (
    gapi_entity _this);

/*
 *     InstanceHandle_t
 *     get_instance_handle();
 */
OS_API gapi_instanceHandle_t
gapi_entity_get_instance_handle (
    gapi_entity _this);

OS_API typedef void (*gapi_delete_action)(void *userData, void *arg);

OS_API void
gapi_object_set_user_data (
    gapi_object _this,
    void *userData,
    gapi_delete_action deleteAction,
    void *deleteActionArg);

OS_API void *
gapi_object_get_user_data (
    gapi_object _this);

/*
 * interface DomainParticipant : Entity {
 */
typedef gapi_object gapi_domainParticipant;

/*  From Entity
 *     enable
 */
#define gapi_domainParticipant_enable(obj) \
        gapi_entity_enable((gapi_entity)obj)

/*  From Entity
 *     get_statuscondition
 */
#define gapi_domainParticipant_get_statuscondition(obj) \
        gapi_entity_get_statuscondition((gapi_entity)obj)

/*  From Entity
 *     get_status_changes
 */
#define gapi_domainParticipant_get_status_changes(obj) \
        gapi_entity_get_status_changes((gapi_entity)obj)

/*  From Entity
 *     set_user_data
 */
#define gapi_domainParticipant_set_user_data(obj,data) \
        gapi_entity_set_user_data((gapi_entity)obj,data)

/*  From Entity
 *     get_user_data
 */
#define gapi_domainParticipant_get_user_data(obj) \
        gapi_entity_get_user_data((gapi_entity)obj)

/*  From Entity
 *     get_instance_handle
 */
#define gapi_domainParticipant_get_instance_handle(obj) \
        gapi_entity_get_instance_handle((gapi_entity)obj)

/*     Publisher
 *     create_publisher(
 *         in PublisherQos qos,
 *         in PublisherListener a_listener);
 */
OS_API gapi_publisher
gapi_domainParticipant_create_publisher (
    gapi_domainParticipant _this,
    const gapi_publisherQos *qos,
    const struct gapi_publisherListener *a_listener,
    const gapi_statusMask mask);

/*     ReturnCode_t
 *     delete_publisher(
 *         in Publisher p);
 */
OS_API gapi_returnCode_t
gapi_domainParticipant_delete_publisher (
    gapi_domainParticipant _this,
    const gapi_publisher p);

/*     Subscriber
 *     create_subscriber(
 *         in SubscriberQos qos,
 *         in SubscriberListener a_listener);
 */
OS_API gapi_subscriber
gapi_domainParticipant_create_subscriber (
    gapi_domainParticipant _this,
    const gapi_subscriberQos *qos,
    const struct gapi_subscriberListener *a_listener,
    const gapi_statusMask mask);

/*     ReturnCode_t
 *     delete_subscriber(
 *         in Subscriber s);
 */
OS_API gapi_returnCode_t
gapi_domainParticipant_delete_subscriber (
    gapi_domainParticipant _this,
    const gapi_subscriber s);

/*     Subscriber
 *     get_builtin_subscriber();
 */
OS_API gapi_subscriber
gapi_domainParticipant_get_builtin_subscriber (
    gapi_domainParticipant _this);

/*     Topic
 *     create_topic(
 *         in string topic_name,
 *         in string type_name,
 *         in TopicQos qos,
 *         in TopicListener a_listener);
 */
OS_API gapi_topic
gapi_domainParticipant_create_topic (
    gapi_domainParticipant _this,
    const gapi_char *topic_name,
    const gapi_char *type_name,
    const gapi_topicQos *qos,
    const struct gapi_topicListener *a_listener,
    const gapi_statusMask mask);

/*     ReturnCode_t
 *     delete_topic(
 *         in Topic a_topic);
 */
OS_API gapi_returnCode_t
gapi_domainParticipant_delete_topic (
    gapi_domainParticipant _this,
    const gapi_topic a_topic);

/*     Topic
 *     find_topic(
 *         in string topic_name,
 *         in Duration_t timeout);
 */
OS_API gapi_topic
gapi_domainParticipant_find_topic (
    gapi_domainParticipant _this,
    const gapi_char *topic_name,
    const gapi_duration_t *timeout);

/*     TopicDescription
 *     lookup_topicdescription(
 *         in string name);
 */
OS_API gapi_topicDescription
gapi_domainParticipant_lookup_topicdescription (
    gapi_domainParticipant _this,
    const gapi_char *name);

/*     ContentFilteredTopic
 *     create_contentfilteredtopic(
 *         in string name,
 *         in Topic related_topic,
 *         in string filter_expression,
 *         in StringSeq filter_parameters);
 */
OS_API gapi_contentFilteredTopic
gapi_domainParticipant_create_contentfilteredtopic (
    gapi_domainParticipant _this,
    const gapi_char *name,
    const gapi_topic related_topic,
    const gapi_char *filter_expression,
    const gapi_stringSeq *filter_parameters);

/*     ReturnCode_t
 *     delete_contentfilteredtopic(
 *         in ContentFilteredTopic a_contentfilteredtopic);
 */
OS_API gapi_returnCode_t
gapi_domainParticipant_delete_contentfilteredtopic (
    gapi_domainParticipant _this,
    const gapi_contentFilteredTopic a_contentfilteredtopic);

/*     MultiTopic
 *     create_multitopic(
 *         in string name,
 *         in string type_name,
 *         in string subscription_expression,
 *         in StringSeq expression_parameters);
 */
OS_API gapi_multiTopic
gapi_domainParticipant_create_multitopic (
    gapi_domainParticipant _this,
    const gapi_char *name,
    const gapi_char *type_name,
    const gapi_char *subscription_expression,
    const gapi_stringSeq *expression_parameters);

/*     ReturnCode_t
 *     delete_multitopic(
 *         in MultiTopic a_multitopic);
 */
OS_API gapi_returnCode_t
gapi_domainParticipant_delete_multitopic (
    gapi_domainParticipant _this,
    const gapi_multiTopic a_multitopic);

typedef void (*gapi_deleteEntityAction)(void *entity_data, void *arg);

/*     ReturnCode_t
 *     delete_contained_entities();
 */
OS_API gapi_returnCode_t
gapi_domainParticipant_delete_contained_entities (
    gapi_domainParticipant _this);

/*     ReturnCode_t
 *     set_qos(
 *         in DomainParticipantQos qos);
 */
OS_API gapi_returnCode_t
gapi_domainParticipant_set_qos (
    gapi_domainParticipant _this,
    const gapi_domainParticipantQos *qos);

/*     ReturnCode_t
 *     get_qos(
 *         inout DomainParticipantQos qos);
 */
OS_API gapi_returnCode_t
gapi_domainParticipant_get_qos (
    gapi_domainParticipant _this,
    gapi_domainParticipantQos *qos);

/*     ReturnCode_t
 *     set_listener(
 *         in DomainParticipantListener a_listener,
 *         in StatusKindMask mask);
 */
OS_API gapi_returnCode_t
gapi_domainParticipant_set_listener (
    gapi_domainParticipant _this,
    const struct gapi_domainParticipantListener *a_listener,
    const gapi_statusMask mask);

/*     DomainParticipantListener
 *     get_listener();
 */
OS_API struct gapi_domainParticipantListener
gapi_domainParticipant_get_listener (
    gapi_domainParticipant _this);

/*     ReturnCode_t
 *     ignore_participant(
 *         in InstanceHandle_t handle);
 */
OS_API gapi_returnCode_t
gapi_domainParticipant_ignore_participant (
    gapi_domainParticipant _this,
    const gapi_instanceHandle_t handle);

/*     ReturnCode_t
 *     ignore_topic(
 *         in InstanceHandle_t handle);
 */
OS_API gapi_returnCode_t
gapi_domainParticipant_ignore_topic (
    gapi_domainParticipant _this,
    const gapi_instanceHandle_t handle);

/*     ReturnCode_t
 *     ignore_publication(
 *         in InstanceHandle_t handle);
 */
OS_API gapi_returnCode_t
gapi_domainParticipant_ignore_publication (
    gapi_domainParticipant _this,
    const gapi_instanceHandle_t handle);

/*     ReturnCode_t
 *     ignore_subscription(
 *         in InstanceHandle_t handle);
 */
OS_API gapi_returnCode_t
gapi_domainParticipant_ignore_subscription (
    gapi_domainParticipant _this,
    const gapi_instanceHandle_t handle);

/*     DomainId_t
 *     get_domain_id();
 */
OS_API gapi_domainId_t
gapi_domainParticipant_get_domain_id (
    gapi_domainParticipant _this);

/*     ReturnCode_t
 *     assert_liveliness();
 */
OS_API gapi_returnCode_t
gapi_domainParticipant_assert_liveliness (
    gapi_domainParticipant _this);

/*     ReturnCode_t
 *     set_default_publisher_qos(
 *         in PublisherQos qos);
 */
OS_API gapi_returnCode_t
gapi_domainParticipant_set_default_publisher_qos (
    gapi_domainParticipant _this,
    const gapi_publisherQos *qos);

/*     ReturnCode_t
 *     get_default_publisher_qos(
 *         inout PublisherQos qos);
 */
OS_API gapi_returnCode_t
gapi_domainParticipant_get_default_publisher_qos (
    gapi_domainParticipant _this,
    gapi_publisherQos *qos);

/*     ReturnCode_t
 *     set_default_subscriber_qos(
 *         in SubscriberQos qos);
 */
OS_API gapi_returnCode_t
gapi_domainParticipant_set_default_subscriber_qos (
    gapi_domainParticipant _this,
    const gapi_subscriberQos *qos);

/*     ReturnCode_t
 *     get_default_subscriber_qos(
 *         inout SubscriberQos qos);
 */
OS_API gapi_returnCode_t
gapi_domainParticipant_get_default_subscriber_qos (
    gapi_domainParticipant _this,
    gapi_subscriberQos *qos);

/*     ReturnCode_t
 *     set_default_topic_qos(
 *         in TopicQos qos);
 */
OS_API gapi_returnCode_t
gapi_domainParticipant_set_default_topic_qos (
    gapi_domainParticipant _this,
    const gapi_topicQos *qos);

/*     ReturnCode_t
 *     get_default_topic_qos(
 *         inout TopicQos qos);
 */
OS_API gapi_returnCode_t
gapi_domainParticipant_get_default_topic_qos (
    gapi_domainParticipant _this,
    gapi_topicQos *qos);

/*     ReturnCode_t
 *     get_discovered_participants (
 *         inout InstanceHandleSeq participant_handles);
 */
OS_API gapi_returnCode_t
gapi_domainParticipant_get_discovered_participants (
    gapi_domainParticipant _this,
    gapi_ReaderInstanceAction action,
    c_voidp arg);


/*     ReturnCode_t
 *     get_discovered_participant_data (
 *         in InstanceHandle_t handle,
 *         inout ParticipantBuiltinTopicData *participant_data);
 */
OS_API gapi_returnCode_t
gapi_domainParticipant_get_discovered_participant_data (
    gapi_domainParticipant _this,
    c_voidp participant_data,
    gapi_instanceHandle_t handle,
    gapi_readerAction action);

/*     ReturnCode_t
 *     get_discovered_topics (
 *         inout InstanceHandleSeq topic_handles);
 */
OS_API gapi_returnCode_t
gapi_domainParticipant_get_discovered_topics (
    gapi_domainParticipant _this,
    gapi_ReaderInstanceAction action,
    c_voidp arg);

/*     ReturnCode_t
 *     get_discovered_topic_data (
 *         in InstanceHandle_t handle,
 *         inout TopicBuiltinTopicData *topic_data);
 */
OS_API gapi_returnCode_t
gapi_domainParticipant_get_discovered_topic_data (
    gapi_domainParticipant _this,
    c_voidp topic_data,
    gapi_instanceHandle_t handle,
    gapi_readerAction action);

/*     Boolean
 *     contains_entity (
 *         in InstanceHandle_t a_hande);
 */
OS_API gapi_boolean
gapi_domainParticipant_contains_entity (
    gapi_domainParticipant _this,
    gapi_instanceHandle_t  a_handle);

/*     ReturnCode_t
 *     get_current_time (
 *         inout Time_t current_time);
 */
OS_API gapi_returnCode_t
gapi_domainParticipant_get_current_time (
    gapi_domainParticipant _this,
    gapi_time_t  *current_time);


OS_API gapi_returnCode_t
gapi_domainParticipant_delete_historical_data (
    gapi_domainParticipant _this,
    const gapi_string partition_expression,
    const gapi_string topic_expression);

/*     gapi_metaDescription
 *     get_type_metadescription (
 *         in string type_name);
 */
typedef void *gapi_metaDescription;

OS_API gapi_metaDescription
gapi_domainParticipant_get_type_metadescription (
    gapi_domainParticipant _this,
    const gapi_char *type_name);

/*     gapi_typeSupport
 *     get_typesupport (
 *         in string registered_name);
 */
OS_API gapi_typeSupport
gapi_domainParticipant_get_typesupport (
    gapi_domainParticipant _this,
    const gapi_char *type_name);

/*     gapi_typeSupport
 *     find_typesupport (
 *         in string registered_type_name);
 */
OS_API gapi_typeSupport
gapi_domainParticipant_lookup_typesupport (
    gapi_domainParticipant _this,
    const gapi_char *type_name);

/*
 * interface Domain {
 */
typedef gapi_object gapi_domain;

OS_API gapi_returnCode_t
gapi_domain_create_persistent_snapshot (
    gapi_domain _this,
    const gapi_char * partition_expression,
    const gapi_char * topic_expression,
    const gapi_char * URI);

/*
 * interface DomainParticipantFactory {
 */
typedef gapi_object gapi_domainParticipantFactory;

/*
 * From Specification
 *
 *     DomainParticipantFactory get_instance (void)
 */
OS_API gapi_domainParticipantFactory
gapi_domainParticipantFactory_get_instance (
    void);

/*     DomainParticipant
 *     create_participant(
 *         in DomainId_t domainId,
 *         in DomainParticipantQos qos,
 *         in DomainParticipantListener a_listener,
 *         in gapi_listenerThreadAction thread_start_action,
 *         in gapi_listenerThreadAction thread_stop_action,
 *         in void *thread_action_arg);
 */
OS_API gapi_domainParticipant
gapi_domainParticipantFactory_create_participant (
    gapi_domainParticipantFactory _this,
    const gapi_domainId_t domainId,
    const gapi_domainParticipantQos *qos,
    const struct gapi_domainParticipantListener *a_listener,
    const gapi_statusMask mask,
    gapi_listenerThreadAction thread_start_action,
    gapi_listenerThreadAction thread_stop_action,
    void *thread_action_arg);

/*     ReturnCode_t
 *     delete_participant(
 *         in DomainParticipant a_participant);
 */
OS_API gapi_returnCode_t
gapi_domainParticipantFactory_delete_participant (
    gapi_domainParticipantFactory _this,
    const gapi_domainParticipant a_participant);

/*     ReturnCode_t
 *     delete_contained_entities(
 *         );
 */
OS_API gapi_returnCode_t
gapi_domainParticipantFactory_delete_contained_entities(
    gapi_domainParticipantFactory _this);

/*     DomainParticipant
 *     lookup_participant(
 *         in DomainId_t domainId);
 */
OS_API gapi_domainParticipant
gapi_domainParticipantFactory_lookup_participant (
    gapi_domainParticipantFactory _this,
    const gapi_domainId_t domainId);

/*     ReturnCode_t
 *     set_qos(
 *         in DomainParticipantFactoryQos qos);
 */
OS_API gapi_returnCode_t
gapi_domainParticipantFactory_set_qos (
    gapi_domainParticipantFactory _this,
    const gapi_domainParticipantFactoryQos *qos);

/*     ReturnCode_t
 *     get_qos(
 *         inout DomainParticipantFactoryQos qos);
 */
OS_API gapi_returnCode_t
gapi_domainParticipantFactory_get_qos (
    gapi_domainParticipantFactory _this,
    gapi_domainParticipantFactoryQos *qos);


/*     ReturnCode_t
 *     set_default_participant_qos(
 *         in DomainParticipantQos qos);
 */
OS_API gapi_returnCode_t
gapi_domainParticipantFactory_set_default_participant_qos (
    gapi_domainParticipantFactory _this,
    const gapi_domainParticipantQos *qos);

/*     ReturnCode_t
 *     get_default_participant_qos(
 *         inout DomainParticipantQos qos);
 */
OS_API gapi_returnCode_t
gapi_domainParticipantFactory_get_default_participant_qos (
    gapi_domainParticipantFactory _this,
    gapi_domainParticipantQos *qos);

/*     Domain
 *     lookup_domain(
 *         in DomainId domain_id);
 */
OS_API gapi_domain
gapi_domainParticipantFactory_lookup_domain (
    gapi_domainParticipantFactory _this,
    const gapi_domainId_t domain_id);


/*     ReturnCode_t
 *     delete_domain(
 *         in Domain a_domain);
 */
OS_API gapi_returnCode_t
gapi_domainParticipantFactory_delete_domain (
    gapi_domainParticipantFactory _this,
    gapi_domain a_domain);

/*
 * interface TypeSupport
 */

/*
 * // Abstract methods
 *
 * // ReturnCode_t
 * //  register_type(
 * //      in DomainParticipant domain,
 * //      in string type_name);
 */

OS_API gapi_typeSupport
gapi_typeSupport__alloc (
    const gapi_char *type_name,
    const gapi_char *type_keys,
    const gapi_char *type_desc);

/* ReturnCode_t
 * register_type(
 *     in DomainParticipant domain,
 *     in string type_name);
 */
OS_API gapi_returnCode_t
gapi_typeSupport_register_type (
    gapi_typeSupport _this,
    gapi_domainParticipant domain,
    gapi_string name);

/*     string
 *     get_type_name();
 */
OS_API gapi_string
gapi_typeSupport_get_type_name (
    gapi_typeSupport _this);

/* gapi_char *
 * get_description ();
 */
OS_API gapi_char *
gapi_typeSupport_get_description (
    gapi_typeSupport _this);

/* gapi_string
 * get_key_list ();
 */
OS_API gapi_string
gapi_typeSupport_get_key_list (
    gapi_typeSupport _this);

/* void *
 * allocbuf(
 *     in unsigned long len);
 */
OS_API void *
gapi_typeSupport_allocbuf (
    gapi_typeSupport _this,
    gapi_unsigned_long len);


/*
 * interface TopicDescription
 */

/*     string
 *     get_type_name();
 */
OS_API gapi_string
gapi_topicDescription_get_type_name (
    gapi_topicDescription _this);

/*     string
 *     get_name();
 */
OS_API gapi_string
gapi_topicDescription_get_name (
    gapi_topicDescription _this);

/*     DomainParticipant
 *     get_participant();
 */
OS_API gapi_domainParticipant
gapi_topicDescription_get_participant (
    gapi_topicDescription _this);

/*
 * interface Topic : Entity, TopicDescription
 */

/*  From Entity
 *     enable
 */
#define gapi_topic_enable(obj) \
        gapi_entity_enable((gapi_entity)obj)

/*  From Entity
 *     get_statuscondition
 */
#define gapi_topic_get_statuscondition(obj) \
        gapi_entity_get_statuscondition((gapi_entity)obj)

/*  From Entity
 *     get_status_changes
 */
#define gapi_topic_get_status_changes(obj) \
        gapi_entity_get_status_changes((gapi_entity)obj)

/*  From Entity
 *     set_user_data
 */
#define gapi_topic_set_user_data(obj,data) \
        gapi_entity_set_user_data((gapi_entity)obj,data)

/*  From Entity
 *     get_user_data
 */
#define gapi_topic_get_user_data(obj) \
        gapi_entity_get_user_data((gapi_entity)obj)

/*  From Entity
 *     get_instance_handle
 */
#define gapi_topic_get_instance_handle(obj) \
        gapi_entity_get_instance_handle((gapi_entity)obj)

/*  From TopicDescription
 *     get_type_name
 */
#define gapi_topic_get_type_name(obj) \
        gapi_topicDescription_get_type_name((gapi_topicDescription)obj)

/*  From TopicDescription
 *     get_name
 */
#define gapi_topic_get_name(obj) \
        gapi_topicDescription_get_name((gapi_topicDescription)obj)

/*  From TopicDescription
 *     get_participant
 */
#define gapi_topic_get_participant(obj) \
        gapi_topicDescription_get_participant((gapi_topicDescription)obj)

/*     // Access the status
 *     ReturnCode_t
 *     get_inconsistent_topic_status( inout InconsistentTopicStatus);
 */
OS_API gapi_returnCode_t
gapi_topic_get_inconsistent_topic_status (
    gapi_topic _this,
    gapi_inconsistentTopicStatus *status);

/*     // Access the status
 *     ReturnCode_t
 *     get_all_data_disposed_topic_status( inout AllDataDisposedTopicStatus);
 */
OS_API gapi_returnCode_t
gapi_topic_get_all_data_disposed_topic_status (
    gapi_topic _this,
    gapi_allDataDisposedTopicStatus *status);


/*     // Access the status
 *     ReturnCode_t
 *     get_all_data_disposed_status( inout AllDataDispsoedStatus);
 */
OS_API gapi_returnCode_t
gapi_topic_get_all_data_disposed_status (
    gapi_topic _this,
    gapi_allDataDisposedTopicStatus *status);

/*     ReturnCode_t
 *     set_listener(
 *         in TopicListener a_listener,
 *         in StatusKindMask mask);
 */
OS_API gapi_returnCode_t
gapi_topic_set_listener (
    gapi_topic _this,
    const struct gapi_topicListener *a_listener,
    const gapi_statusMask mask);

/*     TopicListener
 *     get_listener();
 */
OS_API struct gapi_topicListener
gapi_topic_get_listener (
    gapi_topic _this);

/*     ReturnCode_t
 *     set_qos(
 *         in TopicQos qos);
 */
OS_API gapi_returnCode_t
gapi_topic_set_qos (
    gapi_topic _this,
    const gapi_topicQos *qos);

/*     ReturnCode_t
 *     get_qos(
 *         inout TopicQos qos);
 */
OS_API gapi_returnCode_t
gapi_topic_get_qos (
    gapi_topic _this,
    gapi_topicQos *qos);

/*     DDS_ReturnCode_t
 *     dispose_all_data();
 */
OS_API gapi_returnCode_t
gapi_topic_dispose_all_data (
    gapi_topic _this);

/*
 * interface ContentFilteredTopic : TopicDescription
 */

/*  From TopicDescription
 *     get_type_name
 */
#define gapi_contentFilteredTopic_get_type_name(obj) \
        gapi_topicDescription_get_type_name((gapi_topicDescription)obj)

/*  From TopicDescription
 *     get_name
 */
#define gapi_contentFilteredTopic_get_name(obj) \
        gapi_topicDescription_get_name((gapi_topicDescription)obj)

/*  From TopicDescription
 *     get_participant
 */
#define gapi_contentFilteredTopic_get_participant(obj) \
        gapi_topicDescription_get_participant((gapi_topicDescription)obj)

/*  From Entity
 *     set_user_data
 */
#define gapi_contentFilteredTopic_set_user_data(obj,data) \
        gapi_entity_set_user_data((gapi_entity)obj,data)

/*  From Entity
 *     get_user_data
 */
#define gapi_contentFilteredTopic_get_user_data(obj) \
        gapi_entity_get_user_data((gapi_entity)obj)

/*     string
 *     get_filter_expression();
 */
OS_API gapi_string
gapi_contentFilteredTopic_get_filter_expression (
    gapi_contentFilteredTopic _this);

/*     ReturnCode_t
 *     get_expression_parameters(
 *         inout StringSeq expression_parameters);
 */
OS_API gapi_returnCode_t
gapi_contentFilteredTopic_get_expression_parameters (
    gapi_contentFilteredTopic _this,
    gapi_stringSeq *expression_parameters);

/*     ReturnCode_t
 *     set_expression_parameters(
 *         in StringSeq expression_parameters);
 */
OS_API gapi_returnCode_t
gapi_contentFilteredTopic_set_expression_parameters (
    gapi_contentFilteredTopic _this,
    const gapi_stringSeq *expression_parameters);

/*     Topic
 *     get_related_topic();
 */
OS_API gapi_topic
gapi_contentFilteredTopic_get_related_topic (
    gapi_contentFilteredTopic _this);

/*
 * interface MultiTopic : TopicDescription
 */

/*  From TopicDescription
 *     get_type_name
 */
#define gapi_multiTopic_get_type_name(obj) \
        gapi_topicDescription_get_type_name((gapi_topicDescription)obj)

/*  From TopicDescription
 *     get_name
 */
#define gapi_multiTopic_get_name(obj) \
        gapi_topicDescription_get_name((gapi_topicDescription)obj)

/*  From TopicDescription
 *     get_participant
 */
#define gapi_multiTopic_get_participant(obj) \
        gapi_topicDescription_get_participant((gapi_topicDescription)obj)

/*  From Entity
 *     set_user_data
 */
#define gapi_multiTopic_set_user_data(obj,data) \
        gapi_entity_set_user_data((gapi_entity)obj,data)

/*  From Entity
 *     get_user_data
 */
#define gapi_multiTopic_get_user_data(obj) \
        gapi_entity_get_user_data((gapi_entity)obj)

/*     string
 *     get_subscription_expression();
 */
OS_API gapi_string
gapi_multiTopic_get_subscription_expression (
    gapi_multiTopic _this);

/*     ReturnCode_t
 *     get_expression_parameters(
 *         inout StringSeq expression_parameters);
 */
OS_API gapi_returnCode_t
gapi_multiTopic_get_expression_parameters (
    gapi_multiTopic _this,
    gapi_stringSeq *expression_parameters);

/*     ReturnCode_t
 *     set_expression_parameters(
 *         in StringSeq expression_parameters);
 */
OS_API gapi_returnCode_t
gapi_multiTopic_set_expression_parameters (
    gapi_multiTopic _this,
    const gapi_stringSeq *expression_parameters);

/*
 * interface Publisher : Entity
 */

/*  From Entity
 *     enable
 */
#define gapi_publisher_enable(obj) \
        gapi_entity_enable((gapi_entity)obj)

/*  From Entity
 *     get_statuscondition
 */
#define gapi_publisher_get_statuscondition(obj) \
        gapi_entity_get_statuscondition((gapi_entity)obj)

/*  From Entity
 *     get_status_changes
 */
#define gapi_publisher_get_status_changes(obj) \
        gapi_entity_get_status_changes((gapi_entity)obj)

/*  From Entity
 *     set_user_data
 */
#define gapi_publisher_set_user_data(obj,data) \
        gapi_entity_set_user_data((gapi_entity)obj,data)

/*  From Entity
 *     get_user_data
 */
#define gapi_publisher_get_user_data(obj) \
        gapi_entity_get_user_data((gapi_entity)obj)

/*  From Entity
 *     get_instance_handle
 */
#define gapi_publisher_get_instance_handle(obj) \
        gapi_entity_get_instance_handle((gapi_entity)obj)

/*     DataWriter
 *     create_datawriter(
 *         in Topic a_topic,
 *         in DataWriterQos qos,
 *         in DataWriterListener a_listener);
 */
OS_API gapi_dataWriter
gapi_publisher_create_datawriter (
    gapi_publisher _this,
    const gapi_topic a_topic,
    const gapi_dataWriterQos *qos,
    const struct gapi_dataWriterListener *a_listener,
    const gapi_statusMask mask);

/*     ReturnCode_t
 *     delete_datawriter(
 *         in DataWriter a_datawriter);
 */
OS_API gapi_returnCode_t
gapi_publisher_delete_datawriter (
    gapi_publisher _this,
    const gapi_dataWriter a_datawriter);

/*     DataWriter
 *     lookup_datawriter(
 *         in string topic_name);
 */
OS_API gapi_dataWriter
gapi_publisher_lookup_datawriter (
    gapi_publisher _this,
    const gapi_char *topic_name);

/*     ReturnCode_t
 *     delete_contained_entities();
 */
OS_API gapi_returnCode_t
gapi_publisher_delete_contained_entities (
    gapi_publisher _this);

/*     ReturnCode_t
 *     set_qos(
 *         in PublisherQos qos);
 */
OS_API gapi_returnCode_t
gapi_publisher_set_qos (
    gapi_publisher _this,
    const gapi_publisherQos *qos);

/*     ReturnCode_t
 *     get_qos(
 *         inout PublisherQos qos);
 */
OS_API gapi_returnCode_t
gapi_publisher_get_qos (
    gapi_publisher _this,
    gapi_publisherQos *qos);

/*     ReturnCode_t
 *     set_listener(
 *         in PublisherListener a_listener,
 *         in StatusKindMask mask);
 */
OS_API gapi_returnCode_t
gapi_publisher_set_listener (
    gapi_publisher _this,
    const struct gapi_publisherListener *a_listener,
    const gapi_statusMask mask);

/*     PublisherListener
 *     get_listener();
 */
OS_API struct gapi_publisherListener
gapi_publisher_get_listener (
    gapi_publisher _this);

/*     ReturnCode_t
 *     suspend_publications();
 */
OS_API gapi_returnCode_t
gapi_publisher_suspend_publications (
    gapi_publisher _this);

/*     ReturnCode_t
 *     resume_publications();
 */
OS_API gapi_returnCode_t
gapi_publisher_resume_publications (
    gapi_publisher _this);

/*     ReturnCode_t
 *     begin_coherent_changes();
 */
OS_API gapi_returnCode_t
gapi_publisher_begin_coherent_changes (
    gapi_publisher _this);

/*     ReturnCode_t
 *     end_coherent_changes();
 */
OS_API gapi_returnCode_t
gapi_publisher_end_coherent_changes (
    gapi_publisher _this);

/* ReturnCode_t
 *   wait_for_acknowledgments(
 *      in Duration_t max_wait);
 */
OS_API gapi_returnCode_t
gapi_publisher_wait_for_acknowledgments (
    gapi_publisher _this,
    const gapi_duration_t *max_wait);

/*     DomainParticipant
 *     get_participant();
 */
OS_API gapi_domainParticipant
gapi_publisher_get_participant (
    gapi_publisher _this);

/*     ReturnCode_t
 *     set_default_datawriter_qos(
 *         in DataWriterQos qos);
 */
OS_API gapi_returnCode_t
gapi_publisher_set_default_datawriter_qos (
    gapi_publisher _this,
    const gapi_dataWriterQos *qos);

/*     ReturnCode_t
 *     get_default_datawriter_qos(
 *         inout DataWriterQos qos);
 */
OS_API gapi_returnCode_t
gapi_publisher_get_default_datawriter_qos (
    gapi_publisher _this,
    gapi_dataWriterQos *qos);

/*     ReturnCode_t
 *     copy_from_topic_qos(
 *         inout DataWriterQos a_datawriter_qos,
 *         in TopicQos a_topic_qos);
 */
OS_API gapi_returnCode_t
gapi_publisher_copy_from_topic_qos (
    gapi_publisher _this,
    gapi_dataWriterQos *a_datawriter_qos,
    const gapi_topicQos *a_topic_qos);

/*
 * interface DataWriter : Entity
 */

/*
 * // Abstract methods
 *
 * //  InstanceHandle_t
 * //  register_instance(
 * //      in Data instance_data);
 * //
 * //  InstanceHandle_t
 * //  register_instance_w_timestamp(
 * //      in Data instance_data,
 * //      in Time_t source_timestamp);
 * //
 * //  ReturnCode_t
 * //  unregister_instance(
 * //      in Data instance_data,
 * //      in InstanceHandle_t handle);
 * //
 * //  ReturnCode_t
 * //  unregister_instance_w_timestamp(
 * //      in Data instance_data,
 * //      in InstanceHandle_t handle,
 * //      in Time_t source_timestamp);
 * //
 * //  ReturnCode_t
 * //  write(
 * //      in Data instance_data,
 * //      in InstanceHandle_t handle);
 * //
 * //  ReturnCode_t
 * //  write_w_timestamp(
 * //      in Data instance_data,
 * //      in InstanceHandle_t handle,
 * //      in Time_t source_timestamp);
 * //
 * //  ReturnCode_t
 * //  dispose(
 * //      in Data instance_data,
 * //      in InstanceHandle_t instance_handle);
 * //
 * //  ReturnCode_t
 * //  dispose_w_timestamp(
 * //      in Data instance_data,
 * //      in InstanceHandle_t instance_handle,
 * //      in Time_t source_timestamp);
 * //
 * //  ReturnCode_t
 * //  get_key_value(
 * //      inout Data key_holder,
 * //      in InstanceHandle_t handle);
 */

/*  From Entity
 *     enable
 */
#define gapi_dataWriter_enable(obj) \
        gapi_entity_enable((gapi_entity)obj)

/*  From Entity
 *     get_statuscondition
 */
#define gapi_dataWriter_get_statuscondition(obj) \
        gapi_entity_get_statuscondition((gapi_entity)obj)

/*  From Entity
 *     get_status_changes
 */
#define gapi_dataWriter_get_status_changes(obj) \
        gapi_entity_get_status_changes((gapi_entity)obj)

/*  From Entity
 *     set_user_data
 */
#define gapi_dataWriter_set_user_data(obj,data) \
        gapi_entity_set_user_data((gapi_entity)obj,data)

/*  From Entity
 *     get_user_data
 */
#define gapi_dataWriter_get_user_data(obj) \
        gapi_entity_get_user_data((gapi_entity)obj)

/*  From Entity
 *     get_instance_handle
 */
#define gapi_dataWriter_get_instance_handle(obj) \
        gapi_entity_get_instance_handle((gapi_entity)obj)

/*     ReturnCode_t
 *     set_qos(
 *         in DataWriterQos qos);
 */
OS_API gapi_returnCode_t
gapi_dataWriter_set_qos (
    gapi_dataWriter _this,
    const gapi_dataWriterQos *qos);

/*     ReturnCode_t
 *     get_qos(
 *         inout DataWriterQos qos);
 */
OS_API gapi_returnCode_t
gapi_dataWriter_get_qos (
    gapi_dataWriter _this,
    gapi_dataWriterQos *qos);

/*     ReturnCode_t
 *     set_listener(
 *         in DataWriterListener a_listener,
 *         in StatusKindMask mask);
 */
OS_API gapi_returnCode_t
gapi_dataWriter_set_listener (
    gapi_dataWriter _this,
    const struct gapi_dataWriterListener *a_listener,
    const gapi_statusMask mask);

/*     DataWriterListener
 *     get_listener();
 */
OS_API struct gapi_dataWriterListener
gapi_dataWriter_get_listener (
    gapi_dataWriter _this);

/*     Topic
 *     get_topic();
 */
OS_API gapi_topic
gapi_dataWriter_get_topic (
    gapi_dataWriter _this);

/*     Publisher
 *     get_publisher();
 */
OS_API gapi_publisher
gapi_dataWriter_get_publisher (
    gapi_dataWriter _this);

/* ReturnCode_t
 *   wait_for_acknowledgments(
 *      in Duration_t max_wait);
 */
OS_API gapi_returnCode_t
gapi_dataWriter_wait_for_acknowledgments (
    gapi_dataWriter _this,
    const gapi_duration_t *max_wait);


/*     // Access the status
 *     LivelinessLostStatus
 *     get_liveliness_lost_status();
 */
OS_API gapi_returnCode_t
gapi_dataWriter_get_liveliness_lost_status (
    gapi_dataWriter _this,
    gapi_livelinessLostStatus * status);

/*     OfferedDeadlineMissedStatus
 *     get_offered_deadline_missed_status();
 */
OS_API gapi_returnCode_t
gapi_dataWriter_get_offered_deadline_missed_status (
    gapi_dataWriter _this,
    gapi_offeredDeadlineMissedStatus * status);

/*     OfferedIncompatibleQosStatus
 *     get_offered_incompatible_qos_status();
 */
OS_API gapi_returnCode_t
gapi_dataWriter_get_offered_incompatible_qos_status (
    gapi_dataWriter _this,
    gapi_offeredIncompatibleQosStatus * status);

/*     PublicationMatchedStatus
 *     get_publication_matched_status();
 */
OS_API gapi_returnCode_t
gapi_dataWriter_get_publication_matched_status (
    gapi_dataWriter _this,
    gapi_publicationMatchedStatus * status);

/*     ReturnCode_t
 *     assert_liveliness();
 */
OS_API gapi_returnCode_t
gapi_dataWriter_assert_liveliness (
    gapi_dataWriter _this);

/*     ReturnCode_t
 *     get_matched_subscriptions(
 *         inout InstanceHandleSeq subscription_handles);
 */
OS_API gapi_returnCode_t
gapi_dataWriter_get_matched_subscriptions (
    gapi_dataWriter _this,
    gapi_instanceHandleSeq *subscription_handles);

/*     ReturnCode_t
 *     get_matched_subscription_data(
 *         inout SubscriptionBuiltinTopicData subscription_data,
 *         in InstanceHandle_t subscription_handle);
 */
OS_API gapi_returnCode_t
gapi_dataWriter_get_matched_subscription_data (
    gapi_dataWriter _this,
    gapi_subscriptionBuiltinTopicData *subscription_data,
    const gapi_instanceHandle_t subscription_handle);

/*
 * interface Subscriber : Entity
 */

/*  From Entity
 *     enable
 */
#define gapi_subscriber_enable(obj) \
        gapi_entity_enable((gapi_entity)obj)

/*  From Entity
 *     get_statuscondition
 */
#define gapi_subscriber_get_statuscondition(obj) \
        gapi_entity_get_statuscondition((gapi_entity)obj)

/*  From Entity
 *     get_status_changes
 */
#define gapi_subscriber_get_status_changes(obj) \
        gapi_entity_get_status_changes((gapi_entity)obj)

/*  From Entity
 *     set_user_data
 */
#define gapi_subscriber_set_user_data(obj,data) \
        gapi_entity_set_user_data((gapi_entity)obj,data)

/*  From Entity
 *     get_user_data
 */
#define gapi_subscriber_get_user_data(obj) \
        gapi_entity_get_user_data((gapi_entity)obj)

/*  From Entity
 *     get_instance_handle
 */
#define gapi_subscriber_get_instance_handle(obj) \
        gapi_entity_get_instance_handle((gapi_entity)obj)

/*     DataReader
 *     create_datareader(
 *         in TopicDescription a_topic,
 *         in DataReaderQos qos,
 *         in DataReaderListener a_listener);
 */
OS_API gapi_dataReader
gapi_subscriber_create_datareader (
    gapi_subscriber _this,
    const gapi_topicDescription a_topic,
    const gapi_dataReaderQos *qos,
    const struct gapi_dataReaderListener *a_listener,
    const gapi_statusMask mask);

/*     ReturnCode_t
 *     delete_datareader(
 *         in DataReader a_datareader);
 */
OS_API gapi_returnCode_t
gapi_subscriber_delete_datareader (
    gapi_subscriber _this,
    const gapi_dataReader a_datareader);

/*     ReturnCode_t
 *     delete_contained_entities();
 */
OS_API gapi_returnCode_t
gapi_subscriber_delete_contained_entities (
    gapi_subscriber _this);

/*     DataReader
 *     lookup_datareader(
 *         in string topic_name);
 */
OS_API gapi_dataReader
gapi_subscriber_lookup_datareader (
    gapi_subscriber _this,
    const gapi_char *topic_name);

/*     ReturnCode_t
 *     get_datareaders(
 *         inout DataReaderSeq readers,
 *         in SampleStateMask sample_states,
 *         in ViewStateMask view_states,
 *         in InstanceStateMask instance_states);
 */
OS_API gapi_returnCode_t
gapi_subscriber_get_datareaders (
    gapi_subscriber _this,
    gapi_dataReaderSeq *readers,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states);

/*     ReturnCode_t
 *     notify_datareaders();
 */
OS_API gapi_returnCode_t
gapi_subscriber_notify_datareaders (
    gapi_subscriber _this);

/*     ReturnCode_t
 *     set_qos(
 *         in SubscriberQos qos);
 */
OS_API gapi_returnCode_t
gapi_subscriber_set_qos (
    gapi_subscriber _this,
    const gapi_subscriberQos *qos);

/*     ReturnCode_t
 *     get_qos(
 *         inout SubscriberQos qos);
 */
OS_API gapi_returnCode_t
gapi_subscriber_get_qos (
    gapi_subscriber _this,
    gapi_subscriberQos *qos);

/*     ReturnCode_t
 *     set_listener(
 *         in SubscriberListener a_listener,
 *         in StatusKindMask mask);
 */
OS_API gapi_returnCode_t
gapi_subscriber_set_listener (
    gapi_subscriber _this,
    const struct gapi_subscriberListener *a_listener,
    const gapi_statusMask mask);

/*     SubscriberListener
 *     get_listener();
 */
OS_API struct gapi_subscriberListener
gapi_subscriber_get_listener (
    gapi_subscriber _this);

/*     ReturnCode_t
 *     begin_access();
 */
OS_API gapi_returnCode_t
gapi_subscriber_begin_access (
    gapi_subscriber _this);

/*     ReturnCode_t
 *     end_access();
 */
OS_API gapi_returnCode_t
gapi_subscriber_end_access (
    gapi_subscriber _this);

/*     DomainParticipant
 *     get_participant();
 */
OS_API gapi_domainParticipant
gapi_subscriber_get_participant (
    gapi_subscriber _this);

/*     ReturnCode_t
 *     set_default_datareader_qos(
 *         in DataReaderQos qos);
 */
OS_API gapi_returnCode_t
gapi_subscriber_set_default_datareader_qos (
    gapi_subscriber _this,
    const gapi_dataReaderQos *qos);

/*     ReturnCode_t
 *     get_default_datareader_qos(
 *         inout DataReaderQos qos);
 */
OS_API gapi_returnCode_t
gapi_subscriber_get_default_datareader_qos (
    gapi_subscriber _this,
    gapi_dataReaderQos *qos);

/*     ReturnCode_t
 *     copy_from_topic_qos(
 *         inout DataReaderQos a_datareader_qos,
 *         in TopicQos a_topic_qos);
 */
OS_API gapi_returnCode_t
gapi_subscriber_copy_from_topic_qos (
    gapi_subscriber _this,
    gapi_dataReaderQos *a_datareader_qos,
    const gapi_topicQos *a_topic_qos);

/*
 * interface DataReader : Entity
 */

/*
 * // Abstract methods
 *
 * // ReturnCode_t
 * //  read(
 * //      inout DataSeq data_values,
 * //      inout SampleInfoSeq info_seq,
 * //      in long max_samples,
 * //      in SampleStateMask sample_states,
 * //      in ViewStateMask view_states,
 * //      in InstanceStateMask instance_states);
 * //
 * //  ReturnCode_t
 * //  take(
 * //      inout DataSeq data_values,
 * //      inout SampleInfoSeq info_seq,
 * //      in long max_samples,
 * //      in SampleStateMask sample_states,
 * //      in ViewStateMask view_states,
 * //      in InstanceStateMask instance_states);
 * //
 * //  ReturnCode_t
 * //  read_w_condition(
 * //      inout DataSeq data_values,
 * //      inout SampleInfoSeq info_seq,
 * //      in long max_samples,
 * //      in ReadCondition a_condition);
 * //
 * //  ReturnCode_t
 * //  take_w_condition(
 * //      inout DataSeq data_values,
 * //      inout SampleInfoSeq info_seq,
 * //      in long max_samples,
 * //      in ReadCondition a_condition);
 * //
 * //  ReturnCode_t
 * //  read_next_sample(
 * //      inout Data data_values,
 * //      inout SampleInfo sample_info);
 * //
 * //  ReturnCode_t
 * //  take_next_sample(
 * //      inout Data data_values,
 * //      inout SampleInfo sample_info);
 * //
 * //  ReturnCode_t
 * //  read_instance(
 * //      inout DataSeq data_values,
 * //      inout SampleInfoSeq info_seq,
 * //      in long max_samples,
 * //      in InstanceHandle_t a_handle,
 * //      in SampleStateMask sample_states,
 * //      in ViewStateMask view_states,
 * //      in InstanceStateMask instance_states);
 * //
 * //  ReturnCode_t
 * //  take_instance(
 * //      inout DataSeq data_values,
 * //      inout SampleInfoSeq info_seq,
 * //      in long max_samples,
 * //      in InstanceHandle_t a_handle,
 * //      in SampleStateMask sample_states,
 * //      in ViewStateMask view_states,
 * //      in InstanceStateMask instance_states);
 * //
 * //  ReturnCode_t
 * //  read_next_instance(
 * //      inout DataSeq data_values,
 * //      inout SampleInfoSeq info_seq,
 * //      in long max_samples,
 * //      in InstanceHandle_t a_handle,
 * //      in SampleStateMask sample_states,
 * //      in ViewStateMask view_states,
 * //      in InstanceStateMask instance_states);
 * //
 * //  ReturnCode_t
 * //  take_next_instance(
 * //      inout DataSeq data_values,
 * //      inout SampleInfoSeq info_seq,
 * //      in long max_samples,
 * //      in InstanceHandle_t a_handle,
 * //      in SampleStateMask sample_states,
 * //      in ViewStateMask view_states,
 * //      in InstanceStateMask instance_states);
 * //
 * //  ReturnCode_t
 * //  read_next_instance_w_condition(
 * //      inout DataSeq data_values,
 * //      inout SampleInfoSeq info_seq,
 * //      in long max_samples,
 * //      in InstanceHandle_t a_handle,
 * //      in ReadCondition a_condition);
 * //
 * //  ReturnCode_t
 * //  take_next_instance_w_condition(
 * //      inout DataSeq data_values,
 * //      inout SampleInfoSeq info_seq,
 * //      in long max_samples,
 * //      in InstanceHandle_t a_handle,
 * //      in ReadCondition a_condition);
 * //
 * //  ReturnCode_t
 * //  return_loan(
 * //      inout DataSeq data_values,
 * //      inout SampleInfoSeq info_seq);
 * //
 * //  ReturnCode_t
 * //  get_key_value(
 * //      inout Data key_holder,
 * //      in InstanceHandle_t handle);
 */

/*  From Entity
 *     enable
 */
#define gapi_dataReader_enable(obj) \
        gapi_entity_enable((gapi_entity)obj)

/*  From Entity
 *     get_statuscondition
 */
#define gapi_dataReader_get_statuscondition(obj) \
        gapi_entity_get_statuscondition((gapi_entity)obj)

/*  From Entity
 *     get_status_changes
 */
#define gapi_dataReader_get_status_changes(obj) \
        gapi_entity_get_status_changes((gapi_entity)obj)

/*  From Entity
 *     set_user_data
 */
#define gapi_dataReader_set_user_data(obj,data) \
        gapi_entity_set_user_data((gapi_entity)obj,data)

/*  From Entity
 *     get_user_data
 */
#define gapi_dataReader_get_user_data(obj) \
        gapi_entity_get_user_data((gapi_entity)obj)

/*  From Entity
 *     get_instance_handle
 */
#define gapi_dataReader_get_instance_handle(obj) \
        gapi_entity_get_instance_handle((gapi_entity)obj)

/*     ReadCondition
 *     create_readcondition(
 *         in SampleStateMask sample_states,
 *         in ViewStateMask view_states,
 *         in InstanceStateMask instance_states);
 */
OS_API gapi_readCondition
gapi_dataReader_create_readcondition (
    gapi_dataReader _this,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states);

/*     QueryCondition
 *     create_querycondition(
 *         in SampleStateMask sample_states,
 *         in ViewStateMask view_states,
 *         in InstanceStateMask instance_states,
 *         in string query_expression,
 *         in StringSeq query_parameters);
 */
OS_API gapi_queryCondition
gapi_dataReader_create_querycondition (
    gapi_dataReader _this,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states,
    const gapi_char *query_expression,
    const gapi_stringSeq *query_parameters);

/*     ReturnCode_t
 *     delete_readcondition(
 *         in ReadCondition a_condition);
 */
OS_API gapi_returnCode_t
gapi_dataReader_delete_readcondition (
    gapi_dataReader _this,
    const gapi_readCondition a_condition);

/*     ReturnCode_t
 *     delete_contained_entities();
 */
OS_API gapi_returnCode_t
gapi_dataReader_delete_contained_entities (
    gapi_dataReader _this);

/*     DataReaderView
  *     create_view (
  *     in DataReaderViewQos * qos);
 */
OS_API gapi_dataReaderView
gapi_dataReader_create_view (
    gapi_dataReader _this,
    const gapi_dataReaderViewQos * qos);

/*     ReturnCode_t
 *     delete_view(
 *        in DataReaderView a_view);
 */
OS_API gapi_returnCode_t
gapi_dataReader_delete_view (
    gapi_dataReader _this,
    gapi_dataReaderView a_view);

/*     ReturnCode_t
 *     set_default_datareaderview_qos(
 *         in DataReaderViewQos qos);
 */
OS_API gapi_returnCode_t
gapi_dataReader_set_default_datareaderview_qos (
    gapi_dataReader _this,
    const gapi_dataReaderViewQos *qos);

/*     ReturnCode_t
 *     get_default_datareaderview_qos(
 *         inout DataReaderViewQos qos);
 */
OS_API gapi_returnCode_t
gapi_dataReader_get_default_datareaderview_qos (
    gapi_dataReader _this,
    gapi_dataReaderViewQos *qos);

/*     ReturnCode_t
 *     set_qos(
 *         in DataReaderQos qos);
 */
OS_API gapi_returnCode_t
gapi_dataReader_set_qos (
    gapi_dataReader _this,
    const gapi_dataReaderQos *qos);

/*     ReturnCode_t
 *     get_qos(
 *         inout DataReaderQos qos);
 */
OS_API gapi_returnCode_t
gapi_dataReader_get_qos (
    gapi_dataReader _this,
    gapi_dataReaderQos *qos);

/*     ReturnCode_t
 *     set_listener(
 *         in DataReaderListener a_listener,
 *         in StatusKindMask mask);
 */
OS_API gapi_returnCode_t
gapi_dataReader_set_listener (
    gapi_dataReader _this,
    const struct gapi_dataReaderListener *a_listener,
    const gapi_statusMask mask);

/*     DataReaderListener
 *     get_listener();
 */
OS_API struct gapi_dataReaderListener
gapi_dataReader_get_listener (
    gapi_dataReader _this);

/*     TopicDescription
 *     get_topicdescription();
 */
OS_API gapi_topicDescription
gapi_dataReader_get_topicdescription (
    gapi_dataReader _this);

/*     Subscriber
 *     get_subscriber();
 */
OS_API gapi_subscriber
gapi_dataReader_get_subscriber (
    gapi_dataReader _this);

/*     SampleRejectedStatus
 *     get_sample_rejected_status();
 */
OS_API gapi_returnCode_t
gapi_dataReader_get_sample_rejected_status (
    gapi_dataReader _this,
    gapi_sampleRejectedStatus * status);

/*     LivelinessChangedStatus
 *     get_liveliness_changed_status();
 */
OS_API gapi_returnCode_t
gapi_dataReader_get_liveliness_changed_status (
    gapi_dataReader _this,
    gapi_livelinessChangedStatus * status);

/*     RequestedDeadlineMissedStatus
 *     get_requested_deadline_missed_status();
 */
OS_API gapi_returnCode_t
gapi_dataReader_get_requested_deadline_missed_status (
    gapi_dataReader _this,
    gapi_requestedDeadlineMissedStatus * status);

/*     RequestedIncompatibleQosStatus
 *     get_requested_incompatible_qos_status();
 */
OS_API gapi_returnCode_t
gapi_dataReader_get_requested_incompatible_qos_status (
    gapi_dataReader _this,
    gapi_requestedIncompatibleQosStatus * status);

/*     SubscriptionMatchedStatus
 *     get_subscription_match_status();
 */
OS_API gapi_returnCode_t
gapi_dataReader_get_subscription_matched_status (
    gapi_dataReader _this,
    gapi_subscriptionMatchedStatus * status);

/*     SampleLostStatus
 *     get_sample_lost_status();
 */
OS_API gapi_returnCode_t
gapi_dataReader_get_sample_lost_status (
    gapi_dataReader _this,
    gapi_sampleLostStatus * status);

/*     ReturnCode_t
 *     wait_for_historical_data(
 *         in Duration_t max_wait);
 */
OS_API gapi_returnCode_t
gapi_dataReader_wait_for_historical_data (
    gapi_dataReader _this,
    const gapi_duration_t *max_wait);


/*     ReturnCode_t
 *     wait_for_historical_data_w_condition(
 *         in String filter_expression,
 *         in StringSeq filter_parameters,
 *         in Time_t min_source_timestamp,
 *         in Time_t max_source_timestamp,
 *         in ResourceLimitsQosPolicy resource_limits,
 *         in Duration_t max_wait);
 */
OS_API gapi_returnCode_t
gapi_dataReader_wait_for_historical_data_w_condition (
    gapi_dataReader _this,
    const gapi_char *filter_expression,
    const gapi_stringSeq *filter_parameters,
    const gapi_time_t *min_source_timestamp,
    const gapi_time_t *max_source_timestamp,
    const gapi_resourceLimitsQosPolicy *resource_limits,
    const gapi_duration_t *max_wait);

/*     ReturnCode_t get_matched_publications(
 *     inout InstanceHandleSeq publication_handles);
 */
OS_API gapi_returnCode_t
gapi_dataReader_get_matched_publications (
    gapi_dataReader _this,
    gapi_instanceHandleSeq *publication_handles);

/*     ReturnCode_t
 *     get_matched_publication_data(
 *         inout PublicationBuiltinTopicData publication_data,
 *         in InstanceHandle_t publication_handle);
 */
OS_API gapi_returnCode_t
gapi_dataReader_get_matched_publication_data (
    gapi_dataReader _this,
    gapi_publicationBuiltinTopicData *publication_data,
    const gapi_instanceHandle_t publication_handle);



/*
 * interface DataReaderView : Entity
 */

/*
 * // Abstract methods
 *
 * // ReturnCode_t
 * //  read(
 * //      inout DataSeq data_values,
 * //      inout SampleInfoSeq info_seq,
 * //      in long max_samples,
 * //      in SampleStateMask sample_states,
 * //      in ViewStateMask view_states,
 * //      in InstanceStateMask instance_states);
 * //
 * //  ReturnCode_t
 * //  take(
 * //      inout DataSeq data_values,
 * //      inout SampleInfoSeq info_seq,
 * //      in long max_samples,
 * //      in SampleStateMask sample_states,
 * //      in ViewStateMask view_states,
 * //      in InstanceStateMask instance_states);
 * //
 * //  ReturnCode_t
 * //  read_next_sample(
 * //      inout Data data_values,
 * //      inout SampleInfo sample_info);
 * //
 * //  ReturnCode_t
 * //  take_next_sample(
 * //      inout Data data_values,
 * //      inout SampleInfo sample_info);
 * //
 * //  ReturnCode_t
 * //  read_instance(
 * //      inout DataSeq data_values,
 * //      inout SampleInfoSeq info_seq,
 * //      in long max_samples,
 * //      in InstanceHandle_t a_handle,
 * //      in SampleStateMask sample_states,
 * //      in ViewStateMask view_states,
 * //      in InstanceStateMask instance_states);
 * //
 * //  ReturnCode_t
 * //  take_instance(
 * //      inout DataSeq data_values,
 * //      inout SampleInfoSeq info_seq,
 * //      in long max_samples,
 * //      in InstanceHandle_t a_handle,
 * //      in SampleStateMask sample_states,
 * //      in ViewStateMask view_states,
 * //      in InstanceStateMask instance_states);
 * //
 * //  ReturnCode_t
 * //  read_next_instance(
 * //      inout DataSeq data_values,
 * //      inout SampleInfoSeq info_seq,
 * //      in long max_samples,
 * //      in InstanceHandle_t a_handle,
 * //      in SampleStateMask sample_states,
 * //      in ViewStateMask view_states,
 * //      in InstanceStateMask instance_states);
 * //
 * //  ReturnCode_t
 * //  take_next_instance(
 * //      inout DataSeq data_values,
 * //      inout SampleInfoSeq info_seq,
 * //      in long max_samples,
 * //      in InstanceHandle_t a_handle,
 * //      in SampleStateMask sample_states,
 * //      in ViewStateMask view_states,
 * //      in InstanceStateMask instance_states);
 * //
 */

/*  From Entity
 *     enable
 */
#define gapi_dataReaderView_enable(obj) \
        gapi_entity_enable((gapi_entity)obj)

/*  From Entity
 *     get_statuscondition
 */
OS_API gapi_statusCondition
gapi_dataReaderView_get_statuscondition(
    gapi_dataReaderView _this);

/*  From Entity
 *     get_status_changes
 */
OS_API gapi_statusMask
gapi_dataReaderView_get_status_changes(
    gapi_dataReaderView _this);

/*  From Entity
 *     set_user_data
 */
#define gapi_dataReaderView_set_user_data(obj,data) \
        gapi_entity_set_user_data((gapi_entity)obj,data)

/*  From Entity
 *     get_user_data
 */
#define gapi_dataReaderView_get_user_data(obj) \
        gapi_entity_get_user_data((gapi_entity)obj)

/*  From Entity
 *     get_instance_handle
 */
#define gapi_dataReaderView_get_instance_handle(obj) \
        gapi_entity_get_instance_handle((gapi_entity)obj)


/*     ReturnCode_t
 *     set_qos(
 *         in DataReaderViewQos qos);
 */
OS_API gapi_returnCode_t
gapi_dataReaderView_set_qos (
    gapi_dataReaderView _this,
    const gapi_dataReaderViewQos *qos);

/*     ReturnCode_t
 *     get_qos(
 *         inout DataReaderViewQos qos);
 */
OS_API gapi_returnCode_t
gapi_dataReaderView_get_qos (
    gapi_dataReaderView _this,
    gapi_dataReaderViewQos *qos);

/*     Datareader
 *     get_datareader();
 */
OS_API gapi_dataReader
gapi_dataReaderView_get_datareader (
    gapi_dataReaderView _this);

/*     ReadCondition
 *     create_readcondition(
 *         in SampleStateMask sample_states,
 *         in ViewStateMask view_states,
 *         in InstanceStateMask instance_states);
 */
OS_API gapi_readCondition
gapi_dataReaderView_create_readcondition (
    gapi_dataReaderView _this,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states);

/*     QueryCondition
 *     create_querycondition(
 *         in SampleStateMask sample_states,
 *         in ViewStateMask view_states,
 *         in InstanceStateMask instance_states,
 *         in string query_expression,
 *         in StringSeq query_parameters);
 */
OS_API gapi_queryCondition
gapi_dataReaderView_create_querycondition (
    gapi_dataReaderView _this,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states,
    const gapi_char *query_expression,
    const gapi_stringSeq *query_parameters);

/*     ReturnCode_t
 *     delete_readcondition(
 *         in ReadCondition a_condition);
 */
OS_API gapi_returnCode_t
gapi_dataReaderView_delete_readcondition (
    gapi_dataReaderView _this,
    const gapi_readCondition a_condition);

/*     ReturnCode_t
 *     delete_contained_entities();
 */
OS_API gapi_returnCode_t
gapi_dataReaderView_delete_contained_entities (
    gapi_dataReaderView _this);


/*
 * struct SampleInfo {
 *     SampleStateKind sample_state;
 *     ViewStateKind view_state;
 *     InstanceStateKind instance_state;
 *     gapi_boolean valid_data;
 *     Time_t source_timestamp;
 *     InstanceHandle_t instance_handle;
 *     long disposed_generation_count;
 *     long no_writers_generation_count;
 *     long sample_rank;
 *     long generation_rank;
 *     long absolute_generation_rank;
 * };
 */
typedef C_STRUCT(gapi_sampleInfo) {
    gapi_sampleStateKind sample_state;
    gapi_viewStateKind view_state;
    gapi_instanceStateKind instance_state;
    gapi_boolean valid_data;
    gapi_time_t source_timestamp;
    gapi_instanceHandle_t instance_handle;
    gapi_instanceHandle_t publication_handle;
    gapi_long disposed_generation_count;
    gapi_long no_writers_generation_count;
    gapi_long sample_rank;
    gapi_long generation_rank;
    gapi_long absolute_generation_rank;
    gapi_time_t arrival_timestamp;
} gapi_sampleInfo;

/*
 * typedef sequence<SampleInfo> SampleInfoSeq;
 */
#ifndef _GAPI_SAMPLEINFOSEQ_DEFINED
#define _GAPI_SAMPLEINFOSEQ_DEFINED
typedef struct {
    gapi_unsigned_long _maximum;
    gapi_unsigned_long _length;
    gapi_sampleInfo *_buffer;
    gapi_boolean _release;
} gapi_sampleInfoSeq;
#endif /* _GAPI_SAMPLEINFOSEQ_DEFINED */

OS_API gapi_sampleInfoSeq *
gapi_sampleInfoSeq__alloc (void);

OS_API gapi_sampleInfo *
gapi_sampleInfoSeq_allocbuf (gapi_unsigned_long len);

/*
 * The following FooTypeSupport operations are not for
 * use by the application programmer, these interfaces
 * are used by the generated Application Typed code.
 */

/* Generic user type */
typedef gapi_octet gapi_foo;
typedef gapi_object gapi_fooDataWriter;
typedef gapi_object gapi_fooDataReader;
typedef gapi_object gapi_fooDataReaderView;
typedef gapi_object gapi_fooTypeSupport;


#ifndef _GAPI_FOOSEQ_DEFINED
#define _GAPI_FOOSEQ_DEFINED
/* Foo sequence type */
typedef struct {
    gapi_unsigned_long _maximum;
    gapi_unsigned_long _length;
    gapi_foo *_buffer;
    gapi_boolean _release;
} gapi_fooSeq;

#endif /* _GAPI_FOOSEQ_DEFINED */

OS_API gapi_fooSeq *
gapi_fooSeq__alloc (void);

OS_API gapi_foo *
gapi_fooSeq_allocbuf (gapi_unsigned_long len);

typedef C_STRUCT(gapi_dataSample) {
    void           *data;
    gapi_sampleInfo info;
#define _NOL_
#ifdef _NOL_
    void           *message;
#endif
} gapi_dataSample;

typedef C_STRUCT(gapi_dataSampleSeq) {
    gapi_unsigned_long _maximum;
    gapi_unsigned_long _length;
    gapi_dataSample   *_buffer;
    gapi_boolean       _release;
} gapi_dataSampleSeq;


C_CLASS(gapi_copyCache);
typedef C_STRUCT(c_metaObject) *
        (*gapi_typeSupportLoad)(
            c_base base);

typedef gapi_dataWriter
        (*gapi_createDataWriter)(
            gapi_topic topic,
            const gapi_dataWriterQos *qos,
            const struct gapi_dataWriterListener *listener,
            const gapi_statusMask mask,
            const gapi_publisher publisher);

typedef gapi_dataReader
        (*gapi_createDataReader)(
            gapi_topic topic,
            const gapi_dataReaderQos *qos,
            const struct gapi_dataReaderListener *listener,
            const gapi_statusMask mask,
            const gapi_subscriber subscriber);

typedef gapi_boolean
        (*gapi_copyIn)(
            c_base base,
            void *src,
            void *dst);

typedef void
        (*gapi_copyOut)(
            void *src,
             void *dst);

typedef gapi_boolean
        (*gapi_writerCopy)(
            c_type type,
             void *data,
             void *to);

typedef void *
        (*gapi_topicAllocBuffer)(
            gapi_unsigned_long len);

typedef struct gapi_readerInfo_s {
    gapi_unsigned_long    max_samples;
    gapi_unsigned_long    num_samples;
    gapi_copyOut          copy_out;
    gapi_copyCache        copy_cache;
    gapi_unsigned_long    alloc_size;
    gapi_topicAllocBuffer alloc_buffer;
    void                  *data_buffer;
    void                  *info_buffer;
    void                  **loan_registry;
} gapi_readerInfo;

typedef void
        (*gapi_readerCopy)(
            gapi_dataSampleSeq *samples,
            gapi_readerInfo *readerInfo);

/* From typeSupport
 *     get_type_name();
 */
#define gapi_fooTypeSupport_get_type_name \
        gapi_typeSupport_get_type_name

/* From typeSupport
 * get_description ();
 */
#define gapi_fooTypeSupport_get_description \
        gapi_typeSupport_get_description

/* From typeSupport
 * allocbuf ();
 */
#define gapi_fooTypeSupport_allocbuf \
        gapi_typeSupport_allocbuf


/* ReturnCode_t
 * register_type(
 *     in DomainParticipant domain,
 *     in string type_name);
 */
OS_API gapi_returnCode_t
gapi_fooTypeSupport_register_type (
    gapi_typeSupport _this,
    gapi_domainParticipant domain,
    gapi_string name);

OS_API gapi_typeSupport
gapi_fooTypeSupport__alloc (
    const gapi_char *type_name,
    const gapi_char *type_keys,
    const gapi_char *type_def,
    gapi_typeSupportLoad type_load,
    gapi_copyIn copy_in,
    gapi_copyOut copy_out,
    gapi_unsigned_long alloc_size,
    gapi_topicAllocBuffer alloc_buffer,
    gapi_writerCopy writer_copy,
    gapi_readerCopy reader_copy);

/*
 * The following FooDataWriter operations are not for
 * use by the application programmer, these interfaces
 * are used by the generated Application Typed code.
 */

/*  From Entity
 *     enable
 */
#define gapi_fooDataWriter_enable \
        gapi_entity_enable

/*  From Entity
 *     get_status_changes
 */
#define gapi_fooDataWriter_get_status_changes \
        gapi_entity_get_status_changes

/*  From Entity
 *     get_statuscondition
 */
#define gapi_fooDataWriter_get_statuscondition \
        gapi_entity_get_statuscondition

/*  From Entity
 *     get_instance_handle
 */
#define gapi_fooDataWriter_get_instance_handle(obj) \
        gapi_entity_get_instance_handle((gapi_entity)obj)

/*  From DataWriter
 *     assert_liveliness
 */
#define gapi_fooDataWriter_assert_liveliness \
        gapi_dataWriter_assert_liveliness

/*  From DataWriter
 *     get_listener
 */
#define gapi_fooDataWriter_get_listener \
        gapi_dataWriter_get_listener

/*  From DataWriter
 *     get_liveliness_lost_status
 */
#define gapi_fooDataWriter_get_liveliness_lost_status \
        gapi_dataWriter_get_liveliness_lost_status

/*  From DataWriter
 *     get_matched_subscription_data
 */
#define gapi_fooDataWriter_get_matched_subscription_data \
        gapi_dataWriter_get_matched_subscription_data

/*  From DataWriter
 *     get_matched_subscriptions
 */
#define gapi_fooDataWriter_get_matched_subscriptions \
        gapi_dataWriter_get_matched_subscriptions

/*  From DataWriter
 *     get_offered_deadline_missed_status
 */
#define gapi_fooDataWriter_get_offered_deadline_missed_status \
        gapi_dataWriter_get_offered_deadline_missed_status

/*  From DataWriter
 *     get_offered_incompatible_qos_status
 */
#define gapi_fooDataWriter_get_offered_incompatible_qos_status \
        gapi_dataWriter_get_offered_incompatible_qos_status

/*  From DataWriter
 *     get_publication_matched_status
 */
#define gapi_fooDataWriter_get_publication_matched_status \
        gapi_dataWriter_get_publication_matched_status

/*  From DataWriter
 *     get_publisher
 */
#define gapi_fooDataWriter_get_publisher \
        gapi_dataWriter_get_publisher

/*  From DataWriter
 *     get_qos
 */
#define gapi_fooDataWriter_get_qos \
        gapi_dataWriter_get_qos

/*  From DataWriter
 *     get_topic
 */
#define gapi_fooDataWriter_get_topic \
        gapi_dataWriter_get_topic

/*  From DataWriter
 *     set_listener
 */
#define gapi_fooDataWriter_set_listener \
        gapi_dataWriter_set_listener

/*  From DataWriter
 *     set_qos
 */
#define gapi_fooDataWriter_set_qos \
        gapi_dataWriter_set_qos

/* InstanceHandle_t
 * register_instance(
 *     in Data instance_data);
 */
OS_API gapi_instanceHandle_t
gapi_fooDataWriter_register_instance (
    gapi_dataWriter _this,
    const gapi_foo * instance_data);

/* InstanceHandle_t
 * register_instance_w_timestamp(
 *    in Data instance_data,
 *     in Time_t source_timestamp);
 */
OS_API gapi_instanceHandle_t
gapi_fooDataWriter_register_instance_w_timestamp (
    gapi_dataWriter _this,
    const gapi_foo * instance_data,
    const gapi_time_t *source_timestamp);

/* ReturnCode_t
 * unregister_instance(
 *     in Data instance_data,
 *     in InstanceHandle_t handle);
 */
OS_API gapi_returnCode_t
gapi_fooDataWriter_unregister_instance (
    gapi_dataWriter _this,
    const gapi_foo * instance_data,
    const gapi_instanceHandle_t handle);

/* ReturnCode_t
 * unregister_instance_w_timestamp(
 *     in Data instance_data,
 *     in InstanceHandle_t handle,
 *     in Time_t source_timestamp);
 */
OS_API gapi_returnCode_t
gapi_fooDataWriter_unregister_instance_w_timestamp (
    gapi_dataWriter _this,
    const gapi_foo * instance_data,
    const gapi_instanceHandle_t handle,
    const gapi_time_t *source_timestamp);

/* ReturnCode_t
 * write(
 *     in Data instance_data,
 *     in InstanceHandle_t handle);
 */
OS_API gapi_returnCode_t
gapi_fooDataWriter_write (
    gapi_dataWriter _this,
    const gapi_foo * instance_data,
    const gapi_instanceHandle_t handle);

/* ReturnCode_t
 * write_w_timestamp(
 *     in Data instance_data,
 *     in InstanceHandle_t handle,
 *     in Time_t source_timestamp);
 */
OS_API gapi_returnCode_t
gapi_fooDataWriter_write_w_timestamp (
    gapi_dataWriter _this,
    const gapi_foo * instance_data,
    const gapi_instanceHandle_t handle,
    const gapi_time_t *source_timestamp);

/* ReturnCode_t
 * dispose(
 *     in Data instance_data,
 *     in InstanceHandle_t instance_handle);
 */
OS_API gapi_returnCode_t
gapi_fooDataWriter_dispose (
    gapi_dataWriter _this,
    const gapi_foo * instance_data,
    const gapi_instanceHandle_t instance_handle);

/* ReturnCode_t
 * dispose_w_timestamp(
 *     in Data instance_data,
 *     in InstanceHandle_t instance_handle,
 *     in Time_t source_timestamp);
 */
OS_API gapi_returnCode_t
gapi_fooDataWriter_dispose_w_timestamp (
    gapi_dataWriter _this,
    const gapi_foo * instance_data,
    const gapi_instanceHandle_t instance_handle,
    const gapi_time_t *source_timestamp);

/* ReturnCode_t
 * writedispose(
 *     in Data instance_data,
 *     in InstanceHandle_t instance_handle);
 */
OS_API gapi_returnCode_t
gapi_fooDataWriter_writedispose (
    gapi_dataWriter _this,
    const gapi_foo * instance_data,
    const gapi_instanceHandle_t instance_handle);

/* ReturnCode_t
 * writedispose_w_timestamp(
 *     in Data instance_data,
 *     in InstanceHandle_t instance_handle,
 *     in Time_t source_timestamp);
 */
OS_API gapi_returnCode_t
gapi_fooDataWriter_writedispose_w_timestamp (
    gapi_dataWriter _this,
    const gapi_foo * instance_data,
    const gapi_instanceHandle_t instance_handle,
    const gapi_time_t *source_timestamp);

/* ReturnCode_t
 * get_key_value(
 *     inout Data key_holder,
 *     in InstanceHandle_t handle);
 */
OS_API gapi_returnCode_t
gapi_fooDataWriter_get_key_value (
    gapi_dataWriter _this,
    gapi_foo * key_holder,
    const gapi_instanceHandle_t handle);

/* InstanceHandle_t
 *   lookup_instance(
 *       in Data instance_data);
 */
OS_API gapi_instanceHandle_t
gapi_fooDataWriter_lookup_instance(
    gapi_dataWriter _this,
    const gapi_foo* instance_data);

/*
 * The following FooDataReader operations are not for
 * use by the application programmer, these interfaces
 * are used by the generated Application Typed code.
 */

/*  From Entity
 *     enable
 */
#define gapi_fooDataReader_enable \
        gapi_entity_enable

/*  From Entity
 *     get_status_changes
 */
#define gapi_fooDataReader_get_status_changes \
        gapi_entity_get_status_changes

/*  From Entity
 *     get_statuscondition
 */
#define gapi_fooDataReader_get_statuscondition \
        gapi_entity_get_statuscondition

/*  From Entity
 *     get_instance_handle
 */
#define gapi_fooDataReader_get_instance_handle(obj) \
        gapi_entity_get_instance_handle((gapi_entity)obj)

/*  From DataReader
 *     create_querycondition
 */
#define gapi_fooDataReader_create_querycondition \
        gapi_dataReader_create_querycondition

/*  From DataReader
 *     create_readcondition
 */
#define gapi_fooDataReader_create_readcondition \
        gapi_dataReader_create_readcondition

/*  From DataReader
 *     delete_contained_entities
 */
#define gapi_fooDataReader_delete_contained_entities \
        gapi_dataReader_delete_contained_entities

/*     DataReaderView
  *     create_view
 */
#define gapi_fooDataReader_create_view \
        gapi_dataReader_create_view

/*     ReturnCode_t
 *     delete_view
 */
#define gapi_fooDataReader_delete_view  \
        gapi_dataReader_delete_view

/*  From DataReader
 *     delete_readcondition
 */
#define gapi_fooDataReader_delete_readcondition \
        gapi_dataReader_delete_readcondition

/*  From DataReader
 *     get_listener
 */
#define gapi_fooDataReader_get_listener \
        gapi_dataReader_get_listener

/*  From DataReader
 *     get_liveliness_changed_status
 */
#define gapi_fooDataReader_get_liveliness_changed_status \
        gapi_dataReader_get_liveliness_changed_status

/*  From DataReader
 *     get_matched_publication_data
 */
#define gapi_fooDataReader_get_matched_publication_data \
        gapi_dataReader_get_matched_publication_data

/*  From DataReader
 *     get_matched_publications
 */
#define gapi_fooDataReader_get_matched_publications \
        gapi_dataReader_get_matched_publications

/*  From DataReader
 *     get_qos
 */
#define gapi_fooDataReader_get_qos \
        gapi_dataReader_get_qos

/*  From DataReader
 *     get_requested_deadline_missed_status
 */
#define gapi_fooDataReader_get_requested_deadline_missed_status \
        gapi_dataReader_get_requested_deadline_missed_status

/*  From DataReader
 *     get_requested_incompatible_qos_status
 */
#define gapi_fooDataReader_get_requested_incompatible_qos_status \
        gapi_dataReader_get_requested_incompatible_qos_status

/*  From DataReader
 *     get_sample_lost_status
 */
#define gapi_fooDataReader_get_sample_lost_status \
        gapi_dataReader_get_sample_lost_status

/*  From DataReader
 *     get_sample_rejected_status
 */
#define gapi_fooDataReader_get_sample_rejected_status \
        gapi_dataReader_get_sample_rejected_status

/*  From DataReader
 *     get_subscriber
 */
#define gapi_fooDataReader_get_subscriber \
        gapi_dataReader_get_subscriber

/*  From DataReader
 *     get_subscription_matched_status
 */
#define gapi_fooDataReader_get_subscription_matched_status \
        gapi_dataReader_get_subscription_matched_status

/*  From DataReader
 *     get_topicdescription
 */
#define gapi_fooDataReader_get_topicdescription \
        gapi_dataReader_get_topicdescription

/*  From DataReader
 *     set_listener
 */
#define gapi_fooDataReader_set_listener \
        gapi_dataReader_set_listener

/*  From DataReader
 *     set_qos
 */
#define gapi_fooDataReader_set_qos \
        gapi_dataReader_set_qos

/*  From DataReader
 *     wait_for_historical_data
 */
#define gapi_fooDataReader_wait_for_historical_data \
        gapi_dataReader_wait_for_historical_data

/* ReturnCode_t
 * read(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in SampleStateMask sample_states,
 *     in ViewStateMask view_states,
 *     in InstanceStateMask instance_states);
 */
OS_API gapi_returnCode_t
gapi_fooDataReader_read (
    gapi_fooDataReader _this,
    void *data_values,
    void *info_data,
    const gapi_long max_samples,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states);

/* ReturnCode_t
 * take(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in SampleStateMask sample_states,
 *     in ViewStateMask view_states,
 *     in InstanceStateMask instance_states);
 */
OS_API gapi_returnCode_t
gapi_fooDataReader_take (
    gapi_fooDataReader _this,
    void *data_values,
    void *info_data,
    const gapi_long max_samples,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states);

/* ReturnCode_t
 * read_w_condition(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in ReadCondition a_condition);
 */
OS_API gapi_returnCode_t
gapi_fooDataReader_read_w_condition (
    gapi_fooDataReader _this,
    void *data_values,
    void *info_data,
    const gapi_long max_samples,
    const gapi_readCondition a_condition);

/* ReturnCode_t
 * take_w_condition(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in ReadCondition a_condition);
 */
OS_API gapi_returnCode_t
gapi_fooDataReader_take_w_condition (
    gapi_fooDataReader _this,
    void *data_values,
    void *info_data,
    const gapi_long max_samples,
    const gapi_readCondition a_condition);

/* ReturnCode_t
 * read_next_sample(
 *     inout Data data_values,
 *     inout SampleInfo sample_info);
 */
OS_API gapi_returnCode_t
gapi_fooDataReader_read_next_sample (
    gapi_fooDataReader _this,
    gapi_foo * data_values,
    gapi_sampleInfo *sample_info);

/* ReturnCode_t
 * take_next_sample(
 *     inout Data data_values,
 *     inout SampleInfo sample_info);
 */
OS_API gapi_returnCode_t
gapi_fooDataReader_take_next_sample (
    gapi_fooDataReader _this,
    gapi_foo * data_values,
    gapi_sampleInfo *sample_info);

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
OS_API gapi_returnCode_t
gapi_fooDataReader_read_instance (
    gapi_fooDataReader _this,
    void *data_values,
    void *info_data,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states);

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
OS_API gapi_returnCode_t
gapi_fooDataReader_take_instance (
    gapi_fooDataReader _this,
    void *data_values,
    void *info_data,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states);

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
OS_API gapi_returnCode_t
gapi_fooDataReader_read_next_instance (
    gapi_fooDataReader _this,
    void *data_values,
    void *info_data,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states);

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
OS_API gapi_returnCode_t
gapi_fooDataReader_take_next_instance (
    gapi_fooDataReader _this,
    void *data_values,
    void *info_data,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states);

/* ReturnCode_t
 * read_next_instance_w_condition(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in InstanceHandle_t a_handle,
 *     in ReadCondition a_condition);
 */
OS_API gapi_returnCode_t
gapi_fooDataReader_read_next_instance_w_condition (
    gapi_fooDataReader _this,
    void *data_values,
    void *info_data,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_readCondition a_condition);

/* ReturnCode_t
 * take_next_instance_w_condition(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in InstanceHandle_t a_handle,
 *     in ReadCondition a_condition);
 */
OS_API gapi_returnCode_t
gapi_fooDataReader_take_next_instance_w_condition (
    gapi_fooDataReader _this,
    void *data_values,
    void *info_seq,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_readCondition a_condition);

/* ReturnCode_t
 * return_loan(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq);
 */
OS_API gapi_returnCode_t
gapi_fooDataReader_return_loan (
    gapi_fooDataReader _this,
    void *data_buffer,
    void *info_seq);

/* Boolean
 * is_loan(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq);
 */
OS_API gapi_boolean
gapi_fooDataReader_is_loan (
    gapi_fooDataReader _this,
    void *data_buffer,
    void *info_seq);


/* ReturnCode_t
 * get_key_value(
 *     inout Data key_holder,
 *     in InstanceHandle_t handle);
 */
OS_API gapi_returnCode_t
gapi_fooDataReader_get_key_value (
    gapi_fooDataReader _this,
    gapi_foo * key_holder,
    const gapi_instanceHandle_t handle);

/* InstanceHandle_t
 * lookup_instance(
 *     in Data instance);
 */
OS_API gapi_instanceHandle_t
gapi_fooDataReader_lookup_instance (
    gapi_fooDataReader _this,
    const gapi_foo * instance_data);

/*
 * Get userdata for datareader instance
 */
OS_API gapi_returnCode_t
gapi_fooDataReader_getInstanceUserData (
    gapi_fooDataReader _this,
    gapi_instanceHandle_t instance,
    c_voidp* data_out);

/*
 * Set userdata for datareader instance
 */
OS_API gapi_returnCode_t
gapi_fooDataReader_setInstanceUserData (
    gapi_fooDataReader _this,
    gapi_instanceHandle_t instance,
    c_voidp data);

/*
 * The following FooDataReaderView operations are not for
 * use by the application programmer, these interfaces
 * are used by the generated Application Typed code.
 */

/*  From Entity
 *     enable
 */
#define gapi_fooDataReaderView_enable \
        gapi_entity_enable

/*  From Entity
 *     get_instance_handle
 */
#define gapi_fooDataReaderView_get_instance_handle(obj) \
        gapi_entity_get_instance_handle((gapi_entity)obj)

/*  From DataReaderView
 *     get_qos
 */
#define gapi_fooDataReaderView_get_qos \
        gapi_dataReaderView_get_qos

/*  From DataReaderView
 *     set_qos
 */
#define gapi_fooDataReaderView_set_qos \
        gapi_dataReaderView_set_qos

/*  From DataReaderView
 *     get_datareader
 */
#define gapi_fooDataReaderView_get_datareader \
        gapi_dataReaderView_get_datareader

/*  From DataReaderView
 *     create_querycondition
 */
#define gapi_fooDataReaderView_create_querycondition \
        gapi_dataReaderView_create_querycondition

/*  From DataReaderView
 *     create_readcondition
 */
#define gapi_fooDataReaderView_create_readcondition \
        gapi_dataReaderView_create_readcondition

/*  From DataReaderView
 *     delete_readcondition
 */
#define gapi_fooDataReaderView_delete_readcondition \
        gapi_dataReaderView_delete_readcondition

/*  From DataReaderView
 *     delete_contained_entities
 */
#define gapi_fooDataReaderView_delete_contained_entities \
        gapi_dataReaderView_delete_contained_entities


/* ReturnCode_t
 * read(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in sampleStateMask sample_states,
 *     in viewStateMask view_states,
 *     in instanceStateMask instance_states);
 */
OS_API gapi_returnCode_t
gapi_fooDataReaderView_read (
    gapi_fooDataReaderView _this,
    void *data_values,
    void *info_data,
    const gapi_long max_samples,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states);

/* ReturnCode_t
 * take(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in sampleStateMask sample_states,
 *     in viewStateMask view_states,
 *     in instanceStateMask instance_states);
 */
OS_API gapi_returnCode_t
gapi_fooDataReaderView_take (
    gapi_fooDataReaderView _this,
    void *data_values,
    void *info_data,
    const gapi_long max_samples,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states);

/* ReturnCode_t
 * read_next_sample(
 *     inout Data data_values,
 *     inout SampleInfo sample_info);
 */
OS_API gapi_returnCode_t
gapi_fooDataReaderView_read_next_sample (
    gapi_fooDataReaderView _this,
    gapi_foo * data_values,
    gapi_sampleInfo *sample_info);

/* ReturnCode_t
 * take_next_sample(
 *     inout Data data_values,
 *     inout SampleInfo sample_info);
 */
OS_API gapi_returnCode_t
gapi_fooDataReaderView_take_next_sample (
    gapi_fooDataReaderView _this,
    gapi_foo * data_values,
    gapi_sampleInfo *sample_info);

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
OS_API gapi_returnCode_t
gapi_fooDataReaderView_read_instance (
    gapi_fooDataReaderView _this,
    void *data_values,
    void *info_data,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states);

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
OS_API gapi_returnCode_t
gapi_fooDataReaderView_take_instance (
    gapi_fooDataReaderView _this,
    void *data_values,
    void *info_data,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states);

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
OS_API gapi_returnCode_t
gapi_fooDataReaderView_read_next_instance (
    gapi_fooDataReaderView _this,
    void *data_values,
    void *info_data,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states);

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
OS_API gapi_returnCode_t
gapi_fooDataReaderView_take_next_instance (
    gapi_fooDataReaderView _this,
    void *data_values,
    void *info_data,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states);

/* ReturnCode_t
 * read_w_condition(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in ReadCondition a_condition);
 */
OS_API gapi_returnCode_t
gapi_fooDataReaderView_read_w_condition (
    gapi_fooDataReaderView _this,
    void *data_values,
    void *info_data,
    const gapi_long max_samples,
    const gapi_readCondition a_condition);

/* ReturnCode_t
 * take_w_condition(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in ReadCondition a_condition);
 */
OS_API gapi_returnCode_t
gapi_fooDataReaderView_take_w_condition (
    gapi_fooDataReaderView _this,
    void *data_values,
    void *info_data,
    const gapi_long max_samples,
    const gapi_readCondition a_condition);

/* ReturnCode_t
 * read_next_instance_w_condition(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in InstanceHandle_t a_handle,
 *     in ReadCondition a_condition);
 */
OS_API gapi_returnCode_t
gapi_fooDataReaderView_read_next_instance_w_condition (
    gapi_fooDataReaderView _this,
    void *data_values,
    void *info_data,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_readCondition a_condition);

/* ReturnCode_t
 * take_next_instance_w_condition(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in InstanceHandle_t a_handle,
 *     in ReadCondition a_condition);
 */
OS_API gapi_returnCode_t
gapi_fooDataReaderView_take_next_instance_w_condition (
    gapi_fooDataReaderView _this,
    void *data_values,
    void *info_seq,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_readCondition a_condition);

/* Boolean
 * is_loan(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq);
 */
OS_API gapi_boolean
gapi_fooDataReaderView_is_loan (
    gapi_fooDataReaderView _this,
    void *data_buffer,
    void *info_seq);


/* ReturnCode_t
 * get_key_value(
 *     inout Data key_holder,
 *     in InstanceHandle_t handle);
 */
OS_API gapi_returnCode_t
gapi_fooDataReaderView_get_key_value (
    gapi_fooDataReaderView _this,
    gapi_foo * key_holder,
    const gapi_instanceHandle_t handle);

/* InstanceHandle_t
 * lookup_instance(
 *     in Data instance);
 */
OS_API gapi_instanceHandle_t
gapi_fooDataReaderView_lookup_instance (
    gapi_fooDataReaderView _this,
    const gapi_foo * instance_data);

/* ReturnCode_t
 * return_loan(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq);
 */
OS_API gapi_returnCode_t
gapi_fooDataReaderView_return_loan (
    gapi_fooDataReaderView _this,
    void *data_bufffer,
    void *info_seq);

/*
 * Builtin Topic pre-defined DataReader interfaces
 */

/*
 * ParticipantBuiltinTopicData
 */
OS_API gapi_returnCode_t
gapi_participantBuiltinTopicDataDataReader_read (
    gapi_fooDataReader _this,
    gapi_participantBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq,
    const gapi_long max_samples,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states);

OS_API gapi_returnCode_t
gapi_participantBuiltinTopicDataDataReader_take (
    gapi_fooDataReader _this,
    gapi_participantBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq,
    const gapi_long max_samples,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states);

#define gapi_participantBuiltinTopicDataDataReader \
        gapi_dataReader

#define gapi_participantBuiltinTopicDataDataReader_enable \
        gapi_entity_enable

#define gapi_participantBuiltinTopicDataDataReader_get_status_changes \
        gapi_entity_get_status_changes

#define gapi_participantBuiltinTopicDataDataReader_get_statuscondition \
        gapi_entity_get_statuscondition

#define gapi_participantBuiltinTopicDataDataReader_get_instance_handle \
        gapi_entity_get_instance_handle

#define gapi_participantBuiltinTopicDataDataReader_create_querycondition \
        gapi_dataReader_create_querycondition

#define gapi_participantBuiltinTopicDataDataReader_create_readcondition \
        gapi_dataReader_create_readcondition

#define gapi_participantBuiltinTopicDataDataReader_delete_contained_entities \
        gapi_dataReader_delete_contained_entities

#define gapi_participantBuiltinTopicDataDataReader_delete_readcondition \
        gapi_dataReader_delete_readcondition

#define gapi_participantBuiltinTopicDataDataReader_get_listener \
        gapi_dataReader_get_listener

#define gapi_participantBuiltinTopicDataDataReader_get_liveliness_changed_status \
        gapi_dataReader_get_liveliness_changed_status

#define gapi_participantBuiltinTopicDataDataReader_get_matched_publication_data \
        gapi_dataReader_get_matched_publication_data

#define gapi_participantBuiltinTopicDataDataReader_get_matched_publications \
        gapi_dataReader_get_matched_publications

#define gapi_participantBuiltinTopicDataDataReader_get_qos \
        gapi_dataReader_get_qos

#define gapi_participantBuiltinTopicDataDataReader_get_requested_deadline_missed_status \
        gapi_dataReader_get_requested_deadline_missed_status

#define gapi_participantBuiltinTopicDataDataReader_get_requested_incompatible_qos_status \
        gapi_dataReader_get_requested_incompatible_qos_status

#define gapi_participantBuiltinTopicDataDataReader_get_sample_lost_status \
        gapi_dataReader_get_sample_lost_status

#define gapi_participantBuiltinTopicDataDataReader_get_sample_rejected_status \
        gapi_dataReader_get_sample_rejected_status

#define gapi_participantBuiltinTopicDataDataReader_get_subscriber \
        gapi_dataReader_get_subscriber

#define gapi_participantBuiltinTopicDataDataReader_get_subscription_matched_status gapi_dataReader_get_subscription_matched_status

#define gapi_participantBuiltinTopicDataDataReader_get_topicdescription \
        gapi_dataReader_get_topicdescription

#define gapi_participantBuiltinTopicDataDataReader_set_listener \
        gapi_dataReader_set_listener

#define gapi_participantBuiltinTopicDataDataReader_set_qos \
        gapi_dataReader_set_qos

#define gapi_participantBuiltinTopicDataDataReader_wait_for_historical_data \
        gapi_dataReader_wait_for_historical_data

OS_API gapi_returnCode_t
gapi_participantBuiltinTopicDataDataReader_read_w_condition (
    gapi_fooDataReader _this,
    gapi_participantBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq,
    const gapi_long max_samples,
    const gapi_readCondition a_condition);

OS_API gapi_returnCode_t
gapi_participantBuiltinTopicDataDataReader_take_w_condition (
    gapi_fooDataReader _this,
    gapi_participantBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq,
    const gapi_long max_samples,
    const gapi_readCondition a_condition);

OS_API gapi_returnCode_t
gapi_participantBuiltinTopicDataDataReader_read_next_sample (
    gapi_fooDataReader _this,
    gapi_participantBuiltinTopicData *data_values,
    gapi_sampleInfo *sample_info);

OS_API gapi_returnCode_t
gapi_participantBuiltinTopicDataDataReader_take_next_sample (
    gapi_fooDataReader _this,
    gapi_participantBuiltinTopicData *data_values,
    gapi_sampleInfo *sample_info);

OS_API gapi_returnCode_t
gapi_participantBuiltinTopicDataDataReader_read_instance (
    gapi_fooDataReader _this,
    gapi_participantBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states);

OS_API gapi_returnCode_t
gapi_participantBuiltinTopicDataDataReader_take_instance (
    gapi_fooDataReader _this,
    gapi_participantBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states);

OS_API gapi_returnCode_t
gapi_participantBuiltinTopicDataDataReader_read_next_instance (
    gapi_fooDataReader _this,
    gapi_participantBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states);

OS_API gapi_returnCode_t
gapi_participantBuiltinTopicDataDataReader_take_next_instance (
    gapi_fooDataReader _this,
    gapi_participantBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states);

OS_API gapi_returnCode_t
gapi_participantBuiltinTopicDataDataReader_read_next_instance_w_condition (
    gapi_fooDataReader _this,
    gapi_participantBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_readCondition a_condition);

OS_API gapi_returnCode_t
gapi_participantBuiltinTopicDataDataReader_take_next_instance_w_condition (
    gapi_fooDataReader _this,
    gapi_participantBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_readCondition a_condition);

OS_API gapi_returnCode_t
gapi_participantBuiltinTopicDataDataReader_return_loan (
    gapi_fooDataReader _this,
    gapi_participantBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq);

OS_API gapi_returnCode_t
gapi_participantBuiltinTopicDataDataReader_get_key_value (
    gapi_fooDataReader _this,
    gapi_participantBuiltinTopicData *key_holder,
    const gapi_instanceHandle_t handle);

/*
 * TopicBuiltinTopicData
 */
OS_API gapi_returnCode_t
gapi_topicBuiltinTopicDataDataReader_read (
    gapi_fooDataReader _this,
    gapi_topicBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq,
    const gapi_long max_samples,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states);

OS_API gapi_returnCode_t
gapi_topicBuiltinTopicDataDataReader_take (
    gapi_fooDataReader _this,
    gapi_topicBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq,
    const gapi_long max_samples,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states);

#define gapi_topicBuiltinTopicDataDataReader \
        gapi_dataReader

#define gapi_topicBuiltinTopicDataDataReader_enable \
        gapi_entity_enable

#define gapi_topicBuiltinTopicDataDataReader_get_status_changes \
        gapi_entity_get_status_changes

#define gapi_topicBuiltinTopicDataDataReader_get_statuscondition \
        gapi_entity_get_statuscondition

#define gapi_topicBuiltinTopicDataDataReader_get_instance_handle \
        gapi_entity_get_instance_handle

#define gapi_topicBuiltinTopicDataDataReader_create_querycondition \
        gapi_dataReader_create_querycondition

#define gapi_topicBuiltinTopicDataDataReader_create_readcondition \
        gapi_dataReader_create_readcondition

#define gapi_topicBuiltinTopicDataDataReader_delete_contained_entities \
        gapi_dataReader_delete_contained_entities

#define gapi_topicBuiltinTopicDataDataReader_delete_readcondition \
        gapi_dataReader_delete_readcondition

#define gapi_topicBuiltinTopicDataDataReader_get_listener \
        gapi_dataReader_get_listener

#define gapi_topicBuiltinTopicDataDataReader_get_liveliness_changed_status \
        gapi_dataReader_get_liveliness_changed_status

#define gapi_topicBuiltinTopicDataDataReader_get_matched_publication_data \
        gapi_dataReader_get_matched_publication_data

#define gapi_topicBuiltinTopicDataDataReader_get_matched_publications \
        gapi_dataReader_get_matched_publications

#define gapi_topicBuiltinTopicDataDataReader_get_qos \
        gapi_dataReader_get_qos

#define gapi_topicBuiltinTopicDataDataReader_get_requested_deadline_missed_status \
        gapi_dataReader_get_requested_deadline_missed_status

#define gapi_topicBuiltinTopicDataDataReader_get_requested_incompatible_qos_status \
        gapi_dataReader_get_requested_incompatible_qos_status

#define gapi_topicBuiltinTopicDataDataReader_get_sample_lost_status \
        gapi_dataReader_get_sample_lost_status

#define gapi_topicBuiltinTopicDataDataReader_get_sample_rejected_status \
        gapi_dataReader_get_sample_rejected_status

#define gapi_topicBuiltinTopicDataDataReader_get_subscriber \
        gapi_dataReader_get_subscriber

#define gapi_topicBuiltinTopicDataDataReader_get_subscription_matched_status gapi_dataReader_get_subscription_matched_status

#define gapi_topicBuiltinTopicDataDataReader_get_topicdescription \
        gapi_dataReader_get_topicdescription

#define gapi_topicBuiltinTopicDataDataReader_set_listener \
        gapi_dataReader_set_listener

#define gapi_topicBuiltinTopicDataDataReader_set_qos \
        gapi_dataReader_set_qos

#define gapi_topicBuiltinTopicDataDataReader_wait_for_historical_data \
        gapi_dataReader_wait_for_historical_data

OS_API gapi_returnCode_t
gapi_topicBuiltinTopicDataDataReader_read_w_condition (
    gapi_fooDataReader _this,
    gapi_topicBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq,
    const gapi_long max_samples,
    const gapi_readCondition a_condition);

OS_API gapi_returnCode_t
gapi_topicBuiltinTopicDataDataReader_take_w_condition (
    gapi_fooDataReader _this,
    gapi_topicBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq,
    const gapi_long max_samples,
    const gapi_readCondition a_condition);

OS_API gapi_returnCode_t
gapi_topicBuiltinTopicDataDataReader_read_next_sample (
    gapi_fooDataReader _this,
    gapi_topicBuiltinTopicData *data_values,
    gapi_sampleInfo *sample_info);

OS_API gapi_returnCode_t
gapi_topicBuiltinTopicDataDataReader_take_next_sample (
    gapi_fooDataReader _this,
    gapi_topicBuiltinTopicData *data_values,
    gapi_sampleInfo *sample_info);

OS_API gapi_returnCode_t
gapi_topicBuiltinTopicDataDataReader_read_instance (
    gapi_fooDataReader _this,
    gapi_topicBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states);

OS_API gapi_returnCode_t
gapi_topicBuiltinTopicDataDataReader_take_instance (
    gapi_fooDataReader _this,
    gapi_topicBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states);

OS_API gapi_returnCode_t
gapi_topicBuiltinTopicDataDataReader_read_next_instance (
    gapi_fooDataReader _this,
    gapi_topicBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states);

OS_API gapi_returnCode_t
gapi_topicBuiltinTopicDataDataReader_take_next_instance (
    gapi_fooDataReader _this,
    gapi_topicBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states);

OS_API gapi_returnCode_t
gapi_topicBuiltinTopicDataDataReader_read_next_instance_w_condition (
    gapi_fooDataReader _this,
    gapi_topicBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_readCondition a_condition);

OS_API gapi_returnCode_t
gapi_topicBuiltinTopicDataDataReader_take_next_instance_w_condition (
    gapi_fooDataReader _this,
    gapi_topicBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_readCondition a_condition);

OS_API gapi_returnCode_t
gapi_topicBuiltinTopicDataDataReader_return_loan (
    gapi_fooDataReader _this,
    gapi_topicBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq);

OS_API gapi_returnCode_t
gapi_topicBuiltinTopicDataDataReader_get_key_value (
    gapi_fooDataReader _this,
    gapi_topicBuiltinTopicData *key_holder,
    const gapi_instanceHandle_t handle);

/*
 * PublicationBuiltinTopicData
 */
OS_API gapi_returnCode_t
gapi_publicationBuiltinTopicDataDataReader_read (
    gapi_fooDataReader _this,
    gapi_publicationBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq,
    const gapi_long max_samples,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states);

OS_API gapi_returnCode_t
gapi_publicationBuiltinTopicDataDataReader_take (
    gapi_fooDataReader _this,
    gapi_publicationBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq,
    const gapi_long max_samples,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states);

#define gapi_publicationBuiltinTopicDataDataReader \
        gapi_dataReader

#define gapi_publicationBuiltinTopicDataDataReader_enable \
        gapi_entity_enable

#define gapi_publicationBuiltinTopicDataDataReader_get_status_changes \
        gapi_entity_get_status_changes

#define gapi_publicationBuiltinTopicDataDataReader_get_statuscondition \
        gapi_entity_get_statuscondition

#define gapi_publicationBuiltinTopicDataDataReader_get_instance_handle \
        gapi_entity_get_instance_handle

#define gapi_publicationBuiltinTopicDataDataReader_create_querycondition \
        gapi_dataReader_create_querycondition

#define gapi_publicationBuiltinTopicDataDataReader_create_readcondition \
        gapi_dataReader_create_readcondition

#define gapi_publicationBuiltinTopicDataDataReader_delete_contained_entities \
        gapi_dataReader_delete_contained_entities

#define gapi_publicationBuiltinTopicDataDataReader_delete_readcondition \
        gapi_dataReader_delete_readcondition

#define gapi_publicationBuiltinTopicDataDataReader_get_listener \
        gapi_dataReader_get_listener

#define gapi_publicationBuiltinTopicDataDataReader_get_liveliness_changed_status \
        gapi_dataReader_get_liveliness_changed_status

#define gapi_publicationBuiltinTopicDataDataReader_get_matched_publication_data \
        gapi_dataReader_get_matched_publication_data

#define gapi_publicationBuiltinTopicDataDataReader_get_matched_publications \
        gapi_dataReader_get_matched_publications

#define gapi_publicationBuiltinTopicDataDataReader_get_qos \
        gapi_dataReader_get_qos

#define gapi_publicationBuiltinTopicDataDataReader_get_requested_deadline_missed_status \
        gapi_dataReader_get_requested_deadline_missed_status

#define gapi_publicationBuiltinTopicDataDataReader_get_requested_incompatible_qos_status \
        gapi_dataReader_get_requested_incompatible_qos_status

#define gapi_publicationBuiltinTopicDataDataReader_get_sample_lost_status \
        gapi_dataReader_get_sample_lost_status

#define gapi_publicationBuiltinTopicDataDataReader_get_sample_rejected_status \
        gapi_dataReader_get_sample_rejected_status

#define gapi_publicationBuiltinTopicDataDataReader_get_subscriber \
        gapi_dataReader_get_subscriber

#define gapi_publicationBuiltinTopicDataDataReader_get_subscription_matched_status gapi_dataReader_get_subscription_matched_status

#define gapi_publicationBuiltinTopicDataDataReader_get_topicdescription \
        gapi_dataReader_get_topicdescription

#define gapi_publicationBuiltinTopicDataDataReader_set_listener \
        gapi_dataReader_set_listener

#define gapi_publicationBuiltinTopicDataDataReader_set_qos \
        gapi_dataReader_set_qos

#define gapi_publicationBuiltinTopicDataDataReader_wait_for_historical_data \
        gapi_dataReader_wait_for_historical_data

OS_API gapi_returnCode_t
gapi_publicationBuiltinTopicDataDataReader_read_w_condition (
    gapi_fooDataReader _this,
    gapi_publicationBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq,
    const gapi_long max_samples,
    const gapi_readCondition a_condition);

OS_API gapi_returnCode_t
gapi_publicationBuiltinTopicDataDataReader_take_w_condition (
    gapi_fooDataReader _this,
    gapi_publicationBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq,
    const gapi_long max_samples,
    const gapi_readCondition a_condition);

OS_API gapi_returnCode_t
gapi_publicationBuiltinTopicDataDataReader_read_next_sample (
    gapi_fooDataReader _this,
    gapi_publicationBuiltinTopicData *data_values,
    gapi_sampleInfo *sample_info);

OS_API gapi_returnCode_t
gapi_publicationBuiltinTopicDataDataReader_take_next_sample (
    gapi_fooDataReader _this,
    gapi_publicationBuiltinTopicData *data_values,
    gapi_sampleInfo *sample_info);

OS_API gapi_returnCode_t
gapi_publicationBuiltinTopicDataDataReader_read_instance (
    gapi_fooDataReader _this,
    gapi_publicationBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states);

OS_API gapi_returnCode_t
gapi_publicationBuiltinTopicDataDataReader_take_instance (
    gapi_fooDataReader _this,
    gapi_publicationBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states);

OS_API gapi_returnCode_t
gapi_publicationBuiltinTopicDataDataReader_read_next_instance (
    gapi_fooDataReader _this,
    gapi_publicationBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states);

OS_API gapi_returnCode_t
gapi_publicationBuiltinTopicDataDataReader_take_next_instance (
    gapi_fooDataReader _this,
    gapi_publicationBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states);

OS_API gapi_returnCode_t
gapi_publicationBuiltinTopicDataDataReader_read_next_instance_w_condition (
    gapi_fooDataReader _this,
    gapi_publicationBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_readCondition a_condition);

OS_API gapi_returnCode_t
gapi_publicationBuiltinTopicDataDataReader_take_next_instance_w_condition (
    gapi_fooDataReader _this,
    gapi_publicationBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_readCondition a_condition);

OS_API gapi_returnCode_t
gapi_publicationBuiltinTopicDataDataReader_return_loan (
    gapi_fooDataReader _this,
    gapi_publicationBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq);

OS_API gapi_returnCode_t
gapi_publicationBuiltinTopicDataDataReader_get_key_value (
    gapi_fooDataReader _this,
    gapi_publicationBuiltinTopicData *key_holder,
    const gapi_instanceHandle_t handle);

/*
 * SubscriptionBuiltinTopicData
 */
OS_API gapi_returnCode_t
gapi_subscriptionBuiltinTopicDataDataReader_read (
    gapi_fooDataReader _this,
    gapi_subscriptionBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq,
    const gapi_long max_samples,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states);

OS_API gapi_returnCode_t
gapi_subscriptionBuiltinTopicDataDataReader_take (
    gapi_fooDataReader _this,
    gapi_subscriptionBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq,
    const gapi_long max_samples,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states);

#define gapi_subscriptionBuiltinTopicDataDataReader \
        gapi_dataReader

#define gapi_subscriptionBuiltinTopicDataDataReader_enable \
        gapi_entity_enable

#define gapi_subscriptionBuiltinTopicDataDataReader_get_status_changes \
        gapi_entity_get_status_changes

#define gapi_subscriptionBuiltinTopicDataDataReader_get_statuscondition \
        gapi_entity_get_statuscondition

#define gapi_subscriptionBuiltinTopicDataDataReader_get_instance_handle \
        gapi_entity_get_instance_handle

#define gapi_subscriptionBuiltinTopicDataDataReader_create_querycondition \
        gapi_dataReader_create_querycondition

#define gapi_subscriptionBuiltinTopicDataDataReader_create_readcondition \
        gapi_dataReader_create_readcondition

#define gapi_subscriptionBuiltinTopicDataDataReader_delete_contained_entities \
        gapi_dataReader_delete_contained_entities

#define gapi_subscriptionBuiltinTopicDataDataReader_delete_readcondition \
        gapi_dataReader_delete_readcondition

#define gapi_subscriptionBuiltinTopicDataDataReader_get_listener \
        gapi_dataReader_get_listener

#define gapi_subscriptionBuiltinTopicDataDataReader_get_liveliness_changed_status \
        gapi_dataReader_get_liveliness_changed_status

#define gapi_subscriptionBuiltinTopicDataDataReader_get_matched_publication_data \
        gapi_dataReader_get_matched_publication_data

#define gapi_subscriptionBuiltinTopicDataDataReader_get_matched_publications \
        gapi_dataReader_get_matched_publications

#define gapi_subscriptionBuiltinTopicDataDataReader_get_qos \
        gapi_dataReader_get_qos

#define gapi_subscriptionBuiltinTopicDataDataReader_get_requested_deadline_missed_status \
        gapi_dataReader_get_requested_deadline_missed_status

#define gapi_subscriptionBuiltinTopicDataDataReader_get_requested_incompatible_qos_status \
        gapi_dataReader_get_requested_incompatible_qos_status

#define gapi_subscriptionBuiltinTopicDataDataReader_get_sample_lost_status \
        gapi_dataReader_get_sample_lost_status

#define gapi_subscriptionBuiltinTopicDataDataReader_get_sample_rejected_status \
        gapi_dataReader_get_sample_rejected_status

#define gapi_subscriptionBuiltinTopicDataDataReader_get_subscriber \
        gapi_dataReader_get_subscriber

#define gapi_subscriptionBuiltinTopicDataDataReader_get_subscription_matched_status gapi_dataReader_get_subscription_matched_status

#define gapi_subscriptionBuiltinTopicDataDataReader_get_topicdescription \
        gapi_dataReader_get_topicdescription

#define gapi_subscriptionBuiltinTopicDataDataReader_set_listener \
        gapi_dataReader_set_listener

#define gapi_subscriptionBuiltinTopicDataDataReader_set_qos \
        gapi_dataReader_set_qos

#define gapi_subscriptionBuiltinTopicDataDataReader_wait_for_historical_data \
        gapi_dataReader_wait_for_historical_data

OS_API gapi_returnCode_t
gapi_subscriptionBuiltinTopicDataDataReader_read_w_condition (
    gapi_fooDataReader _this,
    gapi_subscriptionBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq,
    const gapi_long max_samples,
    const gapi_readCondition a_condition);

OS_API gapi_returnCode_t
gapi_subscriptionBuiltinTopicDataDataReader_take_w_condition (
    gapi_fooDataReader _this,
    gapi_subscriptionBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq,
    const gapi_long max_samples,
    const gapi_readCondition a_condition);

OS_API gapi_returnCode_t
gapi_subscriptionBuiltinTopicDataDataReader_read_next_sample (
    gapi_fooDataReader _this,
    gapi_subscriptionBuiltinTopicData *data_values,
    gapi_sampleInfo *sample_info);

OS_API gapi_returnCode_t
gapi_subscriptionBuiltinTopicDataDataReader_take_next_sample (
    gapi_fooDataReader _this,
    gapi_subscriptionBuiltinTopicData *data_values,
    gapi_sampleInfo *sample_info);

OS_API gapi_returnCode_t
gapi_subscriptionBuiltinTopicDataDataReader_read_instance (
    gapi_fooDataReader _this,
    gapi_subscriptionBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states);

OS_API gapi_returnCode_t
gapi_subscriptionBuiltinTopicDataDataReader_take_instance (
    gapi_fooDataReader _this,
    gapi_subscriptionBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states);

OS_API gapi_returnCode_t
gapi_subscriptionBuiltinTopicDataDataReader_read_next_instance (
    gapi_fooDataReader _this,
    gapi_subscriptionBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states);

OS_API gapi_returnCode_t
gapi_subscriptionBuiltinTopicDataDataReader_take_next_instance (
    gapi_fooDataReader _this,
    gapi_subscriptionBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states);

OS_API gapi_returnCode_t
gapi_subscriptionBuiltinTopicDataDataReader_read_next_instance_w_condition (
    gapi_fooDataReader _this,
    gapi_subscriptionBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_readCondition a_condition);

OS_API gapi_returnCode_t
gapi_subscriptionBuiltinTopicDataDataReader_take_next_instance_w_condition (
    gapi_fooDataReader _this,
    gapi_subscriptionBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_readCondition a_condition);

OS_API gapi_returnCode_t
gapi_subscriptionBuiltinTopicDataDataReader_return_loan (
    gapi_fooDataReader _this,
    gapi_subscriptionBuiltinTopicDataSeq *data_values,
    gapi_sampleInfoSeq *info_seq);

OS_API gapi_returnCode_t
gapi_subscriptionBuiltinTopicDataDataReader_get_key_value (
    gapi_fooDataReader _this,
    gapi_subscriptionBuiltinTopicData *key_holder,
    const gapi_instanceHandle_t handle);


/*
 * Type description parser interface
 */
typedef enum {
	GAPI_TYPE_ELEMENT_KIND_MODULE,
	GAPI_TYPE_ELEMENT_KIND_STRUCT,
	GAPI_TYPE_ELEMENT_KIND_MEMBER,
	GAPI_TYPE_ELEMENT_KIND_UNION,
	GAPI_TYPE_ELEMENT_KIND_UNIONCASE,
	GAPI_TYPE_ELEMENT_KIND_UNIONSWITCH,
	GAPI_TYPE_ELEMENT_KIND_UNIONLABEL,
	GAPI_TYPE_ELEMENT_KIND_TYPEDEF,
	GAPI_TYPE_ELEMENT_KIND_ENUM,
	GAPI_TYPE_ELEMENT_KIND_ENUMLABEL,
	GAPI_TYPE_ELEMENT_KIND_TYPE,
	GAPI_TYPE_ELEMENT_KIND_ARRAY,
	GAPI_TYPE_ELEMENT_KIND_SEQUENCE,
	GAPI_TYPE_ELEMENT_KIND_STRING,
	GAPI_TYPE_ELEMENT_KIND_CHAR,
	GAPI_TYPE_ELEMENT_KIND_BOOLEAN,
	GAPI_TYPE_ELEMENT_KIND_OCTET,
	GAPI_TYPE_ELEMENT_KIND_SHORT,
	GAPI_TYPE_ELEMENT_KIND_USHORT,
	GAPI_TYPE_ELEMENT_KIND_LONG,
	GAPI_TYPE_ELEMEMT_KIND_ULONG,
	GAPI_TYPE_ELEMENT_KIND_LONGLONG,
	GAPI_TYPE_ELEMENT_KIND_ULONGLONG,
	GAPI_TYPE_ELEMENT_KIND_FLOAT,
	GAPI_TYPE_ELEMENT_KIND_DOUBLE,
    GAPI_TYPE_ELEMENT_KIND_TIME,
    GAPI_TYPE_ELEMENT_KIND_UNIONLABELDEFAULT
} gapi_typeElementKind;

typedef enum {
	GAPI_TYPE_ATTRIBUTE_KIND_NUMBER,
	GAPI_TYPE_ATTRIBUTE_KIND_STRING
} gapi_typeAttributeKind;

typedef struct gapi_typeAttributeValue {
	gapi_typeAttributeKind _d;
	union {
		gapi_long   nvalue;
		gapi_string svalue;
    } _u;
} gapi_typeAttributeValue;

typedef struct gapi_typeAttribute {
	gapi_string             name;
	gapi_typeAttributeValue value;
} gapi_typeAttribute;

typedef struct {
    gapi_unsigned_long  _maximum;
    gapi_unsigned_long  _length;
    gapi_typeAttribute *_buffer;
    gapi_boolean        _release;
} gapi_typeAttributeSeq;

typedef void * gapi_typeParserHandle;

typedef gapi_boolean (*gapi_typeParserCallback)(
	gapi_typeElementKind         kind,
	const gapi_string            elementName,
	const gapi_typeAttributeSeq *attributes,
    gapi_typeParserHandle        handle,
	void                        *argument);

OS_API gapi_returnCode_t
gapi_typeSupport_parse_type_description (
	const gapi_string        description,
	gapi_typeParserCallback  callback,
	void                    *argument);

OS_API gapi_returnCode_t
gapi_typeSupport_walk_type_description (
	gapi_typeParserHandle    handle,
	gapi_typeParserCallback  callback,
	void                    *argument);


/*
 * interface ErrorInfo
 */

/*
 * typedef long ErrorCode_t;
 */
typedef gapi_long gapi_errorCode_t;

/*    // ----------------------------------------------------------------------
 *    // Error codes
 *    // ----------------------------------------------------------------------
 *    const ErrorCode_t ERRORCODE_UNDEFINED  	                = 0;
 *    const ErrorCode_t ERRORCODE_ERROR			        = 1;
 *    const ErrorCode_t ERRORCODE_OUT_OF_RESOURCES	        = 2;
 *    const ErrorCode_t ERRORCODE_CREATION_KERNEL_ENTITY_FAILED	= 3;
 *    const ErrorCode_t ERRORCODE_INVALID_VALUE           	= 4;
 *    const ErrorCode_t ERRORCODE_INVALID_DURATION		= 5;
 *    const ErrorCode_t ERRORCODE_INVALID_TIME    		= 6;
 *    const ErrorCode_t ERRORCODE_ENTITY_INUSE            	= 7;
 *    const ErrorCode_t ERRORCODE_CONTAINS_ENTITIES               = 8;
 *    const ErrorCode_t ERRORCODE_ENTITY_UNKNOWN  		= 9;
 *    const ErrorCode_t ERRORCODE_HANDLE_NOT_REGISTERED   	= 10;
 *    const ErrorCode_t ERRORCODE_HANDLE_NOT_MATCH		= 11;
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

#define GAPI_ERRORCODE_UNDEFINED  	                0
#define GAPI_ERRORCODE_ERROR			        1
#define GAPI_ERRORCODE_OUT_OF_RESOURCES	                2
#define GAPI_ERRORCODE_CREATION_KERNEL_ENTITY_FAILED	3
#define GAPI_ERRORCODE_INVALID_VALUE           	        4
#define GAPI_ERRORCODE_INVALID_DURATION		        5
#define GAPI_ERRORCODE_INVALID_TIME    		        6
#define GAPI_ERRORCODE_ENTITY_INUSE            	        7
#define GAPI_ERRORCODE_CONTAINS_ENTITIES                8
#define GAPI_ERRORCODE_ENTITY_UNKNOWN  		        9
#define GAPI_ERRORCODE_HANDLE_NOT_REGISTERED   	        10
#define GAPI_ERRORCODE_HANDLE_NOT_MATCH		        11
#define GAPI_ERRORCODE_HANDLE_INVALID                   12
#define GAPI_ERRORCODE_INVALID_SEQUENCE                 13
#define GAPI_ERRORCODE_UNSUPPORTED_VALUE                14
#define GAPI_ERRORCODE_INCONSISTENT_VALUE               15
#define GAPI_ERRORCODE_IMMUTABLE_QOS_POLICY             16
#define GAPI_ERRORCODE_INCONSISTENT_QOS                 17
#define GAPI_ERRORCODE_UNSUPPORTED_QOS_POLICY           18
#define GAPI_ERRORCODE_CONTAINS_CONDITIONS              19
#define GAPI_ERRORCODE_CONTAINS_LOANS                   20
#define GAPI_ERRORCODE_INCONSISTENT_TOPIC               21

typedef gapi_object gapi_errorInfo;

/*     ReturnCode_t
 *     update( );
 */
OS_API gapi_returnCode_t
gapi_errorInfo_update (
    gapi_errorInfo _this);

/*     ReturnCode_t
 *     get_code(
 *         out ErrorCode code);
 */
OS_API gapi_returnCode_t
gapi_errorInfo_get_code (
    gapi_errorInfo _this,
    gapi_errorCode_t * code);

/*     ReturnCode_t
 *     get_location(
 *         out String location);
 */
OS_API gapi_returnCode_t
gapi_errorInfo_get_location(
    gapi_errorInfo _this,
    gapi_string * location);

/*     ReturnCode_t
 *     get_source_line(
 *         out String source_line);
 */
OS_API gapi_returnCode_t
gapi_errorInfo_get_source_line(
    gapi_errorInfo _this,
    gapi_string * source_line);

/*     ReturnCode_t
 *     get_stack_trace(
 *         out String stack_trace);
 */
OS_API gapi_returnCode_t
gapi_errorInfo_get_stack_trace(
    gapi_errorInfo _this,
    gapi_string * stack_trace);

/*     ReturnCode_t
 *     get_message(
 *         out String message);
 */
OS_API gapi_returnCode_t
gapi_errorInfo_get_message(
    gapi_errorInfo _this,
    gapi_string * message);


/*     ErrorInfo
 *     ErrorInfo__alloc (
 *         void);
 */
OS_API gapi_errorInfo
gapi_errorInfo__alloc (
    void);

/*
 *  GAPI MetaData Interface definitions.
 */

#define GAPI_SUBCLASS(parent,child) typedef parent child

/* Definition of gapi_baseObject. */
typedef void *gapi_baseObject;

/* Children for gapi_baseObject. */
GAPI_SUBCLASS(gapi_baseObject, gapi_specifier);
GAPI_SUBCLASS(gapi_baseObject, gapi_metaObject);

/* Children for gapi_specifier. */
GAPI_SUBCLASS(gapi_specifier, gapi_member);
GAPI_SUBCLASS(gapi_specifier, gapi_unionCase);

/* Children for gapi_metaObject. */
GAPI_SUBCLASS(gapi_metaObject, gapi_type);
GAPI_SUBCLASS(gapi_metaObject, gapi_property);

/* Children for gapi_type. */
GAPI_SUBCLASS(gapi_type, gapi_enumeration);
GAPI_SUBCLASS(gapi_type, gapi_primitive);
GAPI_SUBCLASS(gapi_type, gapi_collectionType);
GAPI_SUBCLASS(gapi_type, gapi_structure);
GAPI_SUBCLASS(gapi_type, gapi_union);

/**
 * Return the actual type of a type: this is never a typedef.
 */
OS_API gapi_type
gapi_typeActualType(gapi_type typeBase);

/**
 * Return the general kind of a type.
 */
OS_API c_metaKind
gapi_metaData_baseObjectKind(gapi_baseObject objBase);

/**
 * Return the type of a specifier (i.e. a member or a union branch).
 */
OS_API gapi_type
gapi_metaData_specifierType(gapi_specifier specBase);

/**
 * Return the name of a specifier (i.e. a member or a union branch).
 */
OS_API const gapi_char *
gapi_metaData_specifierName(gapi_specifier specBase);

/**
 * Return the size of a type.
 */
OS_API gapi_long
gapi_metaData_typeSize(gapi_type typeBase);

/**
 * Return the amount of labels from an enumeration.
 */
OS_API gapi_long
gapi_metaData_enumerationCount(gapi_enumeration enumBase);

/**
 * Return the exact primitive kind of a primitive type.
 */
OS_API c_primKind
gapi_metaData_primitiveKind(gapi_primitive primBase);

/**
 * Return the exact collection kind of a collection type.
 */
OS_API c_collKind
gapi_metaData_collectionTypeKind(gapi_collectionType collBase);

/**
 * Return the maximum size of a collection type.
 */
OS_API gapi_long
gapi_metaData_collectionTypeMaxSize(gapi_collectionType collBase);

/**
 * Return the element type of a collection type.
 */
OS_API gapi_type
gapi_metaData_collectionTypeSubType(gapi_collectionType collBase);

/**
 * Return the amount of members of a structure.
 */
OS_API gapi_long
gapi_metaData_structureMemberCount(gapi_structure structBase);

/**
 * Return the struct member that is specified by the index.
 */
OS_API gapi_member
gapi_metaData_structureMember(gapi_structure structBase, c_long index);

/**
 * Return the type of a member.
 */
OS_API gapi_type
gapi_metaData_memberType(gapi_member memberBase);

/**
 * Return the offset of a member.
 */
OS_API gapi_unsigned_long
gapi_metaData_memberOffset(gapi_member memberBase);

/**
 * Return the amount of branches of a union.
 */
OS_API gapi_long
gapi_metaData_unionUnionCaseCount(gapi_union unionBase);

/**
 * Return the union branch that is specified by the index.
 */
OS_API gapi_unionCase
gapi_metaData_unionUnionCase(gapi_union unionBase, c_long index);

/**
 * Return the type of a union branch.
 */
OS_API gapi_type
gapi_metaData_unionCaseType(gapi_unionCase caseBase);


#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* GAPI_H */
