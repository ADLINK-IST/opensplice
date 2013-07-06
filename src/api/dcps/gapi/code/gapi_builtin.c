/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#include "gapi_dataReader.h"
#include "gapi_builtin.h"
#include "gapi_qos.h"
#include "gapi_typeSupport.h"
#include "gapi_subscriber.h"
#include "gapi_topicDescription.h"
#include "gapi_topic.h"
#include "gapi_condition.h"
#include "gapi_domainParticipant.h"
#include "gapi_domainParticipantFactory.h"
#include "gapi_structured.h"
#include "gapi_kernel.h"
#include "gapi_expression.h"

#include "os_heap.h"
#include "os_report.h"
#include "u_reader.h"
#include "v_topic.h"
#include "v_dataReader.h"
#include "u_dataReader.h"
#include "kernelModule.h"

#define BUILTIN_TOPIC_COPY_IN

#define _BUILTIN_PARTICIPANT_INFO  0
#define _BUILTIN_TOPIC_INFO        1
#define _BUILTIN_PUBLICATION_INFO  2
#define _BUILTIN_SUBSCRIPTION_INFO 3

const BuiltinTopicTypeInfo builtinTopicTypeInfo[] = {
    {
        "DCPSParticipant",
        "DDS::ParticipantBuiltinTopicData",
        sizeof(gapi_participantBuiltinTopicData),
        (gapi_topicAllocBuffer)NULL,
        NULL,
        NULL
    },
    {
        "DCPSTopic",
        "DDS::TopicBuiltinTopicData",
        sizeof(gapi_topicBuiltinTopicData),
        (gapi_topicAllocBuffer)NULL,
        NULL,
        NULL
    },
    {
        "DCPSPublication",
        "DDS::PublicationBuiltinTopicData",
        sizeof(gapi_publicationBuiltinTopicData),
        (gapi_topicAllocBuffer)NULL,
        NULL,
        NULL
    },
    {
        "DCPSSubscription",
        "DDS::SubscriptionBuiltinTopicData",
        sizeof(gapi_subscriptionBuiltinTopicData),
        (gapi_topicAllocBuffer)NULL,
        NULL,
        NULL
    }
};

static void
initBuiltinDataReaderQos(
    gapi_dataReaderQos *rQos)
{
    rQos->durability.kind                              = GAPI_TRANSIENT_DURABILITY_QOS;
    rQos->deadline.period.sec                          = GAPI_DURATION_INFINITE_SEC;
    rQos->deadline.period.nanosec                      = GAPI_DURATION_INFINITE_NSEC;
    rQos->latency_budget.duration.sec                  = GAPI_DURATION_ZERO_SEC;
    rQos->latency_budget.duration.nanosec              = GAPI_DURATION_ZERO_NSEC;
    rQos->liveliness.kind                              = GAPI_AUTOMATIC_LIVELINESS_QOS;
    rQos->liveliness.lease_duration.sec                = GAPI_DURATION_ZERO_SEC;
    rQos->liveliness.lease_duration.nanosec            = GAPI_DURATION_ZERO_NSEC;
    rQos->reliability.kind                             = GAPI_RELIABLE_RELIABILITY_QOS;
    rQos->reliability.max_blocking_time.sec            = GAPI_DURATION_ZERO_SEC;
    rQos->reliability.max_blocking_time.nanosec        = GAPI_DURATION_ZERO_NSEC;
    rQos->reliability.synchronous                      = FALSE;
    rQos->destination_order.kind                       = GAPI_BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS;
    rQos->history.kind                                 = GAPI_KEEP_LAST_HISTORY_QOS;
    rQos->history.depth                                = 1;
    rQos->resource_limits.max_samples                  = GAPI_LENGTH_UNLIMITED;
    rQos->resource_limits.max_instances                = GAPI_LENGTH_UNLIMITED;
    rQos->resource_limits.max_samples_per_instance     = GAPI_LENGTH_UNLIMITED;
    rQos->user_data.value._maximum                     = 0;
    rQos->user_data.value._length                      = 0;
    rQos->user_data.value._buffer                      = NULL;
    rQos->user_data.value._release                     = FALSE;
    rQos->ownership.kind                               = GAPI_SHARED_OWNERSHIP_QOS;
    rQos->time_based_filter.minimum_separation.sec     = GAPI_DURATION_ZERO_SEC;
    rQos->time_based_filter.minimum_separation.nanosec = GAPI_DURATION_ZERO_NSEC;
    rQos->reader_data_lifecycle.autopurge_nowriter_samples_delay.sec
                                                      = GAPI_DURATION_INFINITE_SEC;
    rQos->reader_data_lifecycle.autopurge_nowriter_samples_delay.nanosec
                                                      = GAPI_DURATION_INFINITE_NSEC;
    rQos->reader_data_lifecycle.autopurge_disposed_samples_delay.sec
                                                      = GAPI_DURATION_INFINITE_SEC;
    rQos->reader_data_lifecycle.autopurge_disposed_samples_delay.nanosec
                                                      = GAPI_DURATION_INFINITE_NSEC;
    rQos->reader_data_lifecycle.enable_invalid_samples = TRUE;
    rQos->reader_data_lifecycle.invalid_sample_visibility.kind
                                                      = GAPI_MINIMUM_INVALID_SAMPLES;
    rQos->reader_lifespan.use_lifespan                 = FALSE;
    rQos->reader_lifespan.duration.sec                 = GAPI_DURATION_INFINITE_SEC;
    rQos->reader_lifespan.duration.nanosec             = GAPI_DURATION_INFINITE_NSEC;
    rQos->share.enable                                 = FALSE;
    rQos->share.name                                   = NULL;
    rQos->subscription_keys.use_key_list               = FALSE;
    rQos->subscription_keys.key_list._maximum          = 0;
    rQos->subscription_keys.key_list._length           = 0;
    rQos->subscription_keys.key_list._buffer           = NULL;
    rQos->subscription_keys.key_list._release          = FALSE;
}

_Subscriber
_BuiltinSubscriberNew (
    u_participant uParticipant,
    _DomainParticipantFactory factory,
    _DomainParticipant participant)
{
    u_subscriber s;
    _Status status;
    _Subscriber newSubscriber = _SubscriberAlloc();
    gapi_handle handle;
    _TypeSupport typeSupport;
    gapi_dataReaderQos rQos;
    long i;

    s = u_participantGetBuiltinSubscriber(uParticipant);

    if (s) {
        newSubscriber = _SubscriberAlloc();

        if (newSubscriber != NULL) {

            _EntityInit(_Entity(newSubscriber), _Entity(participant));

            U_SUBSCRIBER_SET(newSubscriber, s);

            status = _StatusNew(_Entity(newSubscriber),
                                STATUS_KIND_SUBSCRIBER,
                                NULL, 0);
            if (status) {
                for ( i = 0; i < MAX_BUILTIN_TOPIC; i++ ) {
                    _DataReader reader = NULL;
                    _Topic topic = NULL;

                    typeSupport = _DomainParticipantFindType(participant, _BuiltinTopicTypeName(i));
                    if (typeSupport) {
                        c_iter uTopicList;
                        u_topic uTopic;

                        uTopicList = u_participantFindTopic(uParticipant,
                                                            _BuiltinTopicName(i),
                                                            C_TIME_ZERO);
                        uTopic = c_iterTakeFirst(uTopicList);
                        if (uTopic) {
                            topic = _TopicFromKernelTopic(uTopic,
                                                          _BuiltinTopicName(i),
                                                          _BuiltinTopicTypeName(i),
                                                          participant,
                                                          NULL);
                            while (uTopic) {
                                uTopic = c_iterTakeFirst(uTopicList);
                                /* multiple instances should not occure but
                                 * just in case this loop frees all references.
                                 */
                                assert(uTopic == NULL);
                                u_entityFree(u_entity(uTopic));
                            }
                        } else {
                            OS_REPORT_2(OS_WARNING,"_BuiltinSubscriberNew",0,
                                        "failed to resolve User layer Topic "
                                        "'%s' for Participant 0x%x",
                                        _BuiltinTopicName(i), participant);
                        }
                    } else {
                        OS_REPORT_2(OS_WARNING,"_BuiltinSubscriberNew",0,
                                    "Builtin TypeSupport for type '%s' is not "
                                    "yet registered for Participant 0x%x",
                                    _BuiltinTopicTypeName(i), participant);
                    }

                    if (topic) {
                        initBuiltinDataReaderQos(&rQos);
                        reader = _DataReaderNew(_TopicDescription(topic),
                                                typeSupport,
                                                &rQos,
                                                NULL, 0,
                                                newSubscriber);

                        _EntityRelease(topic);
                    } else {
                        OS_REPORT_2(OS_WARNING,"_BuiltinSubscriberNew",0,
                                    "failed to create Builtin Topic '%s' "
                                    "for Participant 0x%x",
                                    _BuiltinTopicName(i), participant);
                    }

                    if ( reader ) {
                        _ENTITY_REGISTER_OBJECT(_Entity(newSubscriber),
                                                (_Object)reader);
                        handle = _EntityRelease(reader);
                        gapi_entity_enable(handle);
                    }
                }
                newSubscriber->builtin = TRUE;
                _EntityStatus(newSubscriber) = status;
            } else {
                _EntityDispose(_Entity(newSubscriber));
                newSubscriber = NULL;
            }
        }
    }
    return newSubscriber;
}


void
_BuiltinSubscriberFree (
    _Subscriber subscriber)
{
    _Status status;

    assert(subscriber != NULL);

    status = _EntityStatus(subscriber);
    _StatusSetListener(status, NULL, 0);

    _EntityClaim(status);
    _StatusDeinit(status);

    u_subscriberFree(U_SUBSCRIBER_GET(subscriber));

    _EntityDispose(_Entity(subscriber));
}

const char *
_BuiltinTopicName  (
    long index)
{
    const char *name = NULL;

    if ( index >= 0 && index < MAX_BUILTIN_TOPIC ) {
        name = builtinTopicTypeInfo[index].topicName;
    }
    return name;
}

const char *
_BuiltinTopicTypeName  (
    long index)
{
    const char *name = NULL;

    if ( index >= 0 && index < MAX_BUILTIN_TOPIC ) {
        name = builtinTopicTypeInfo[index].typeName;
    }
    return name;
}

const char *
_BuiltinFindTopicName (
    _Entity entity)
{
    const char *name = NULL;

    switch (_ObjectGetKind(_Object(entity))) {
    case OBJECT_KIND_DOMAINPARTICIPANT:
        name = builtinTopicTypeInfo[_BUILTIN_PARTICIPANT_INFO].topicName;
    break;
    case OBJECT_KIND_TOPIC:
        name = builtinTopicTypeInfo[_BUILTIN_TOPIC_INFO].topicName;
    break;
    case OBJECT_KIND_DATAWRITER:
        name = builtinTopicTypeInfo[_BUILTIN_PUBLICATION_INFO].topicName;
    break;
    case OBJECT_KIND_DATAREADER:
        name = builtinTopicTypeInfo[_BUILTIN_SUBSCRIPTION_INFO].topicName;
    break;
    default:
    break;
    }

    return name;
}

const BuiltinTopicTypeInfo *
_BuiltinTopicFindTypeInfo (
    const char *topicName)
{
    const BuiltinTopicTypeInfo *info = NULL;
    long i;

    for ( i = 0; i < MAX_BUILTIN_TOPIC; i++ ) {
        if ( strcmp(builtinTopicTypeInfo[i].topicName, topicName) == 0 ) {
            info = &builtinTopicTypeInfo[i];
        }
    }

    return info;
}

const BuiltinTopicTypeInfo *
_BuiltinTopicFindTypeInfoByType (
    const char *typeName)
{
    const BuiltinTopicTypeInfo *info = NULL;
    long i;

    for ( i = 0; (i < MAX_BUILTIN_TOPIC) && (info == NULL); i++ ) {
        if ( strcmp(builtinTopicTypeInfo[i].typeName, typeName) == 0 ) {
            info = &builtinTopicTypeInfo[i];
        }
    }

    return info;
}


static void
builtinTopicKeyCopyout (
    const v_builtinTopicKey *src,
    gapi_builtinTopicKey_t  *dst)
{
    assert(sizeof(v_builtinTopicKey) == sizeof(gapi_builtinTopicKey_t));

    memcpy(dst, src, sizeof(gapi_builtinTopicKey_t));
}

#ifdef BUILTIN_TOPIC_COPY_IN
static void
builtinTopicKeyCopyin (
    const gapi_builtinTopicKey_t *src,
    v_builtinTopicKey            *dst)
{
    assert(sizeof(v_builtinTopicKey) == sizeof(gapi_builtinTopicKey_t));

    memcpy(dst, src, sizeof(v_builtinTopicKey));
}
#endif

static void
builtinUserDataQosPolicyCopyout (
    const struct v_builtinUserDataPolicy *src,
    gapi_userDataQosPolicy               *dst)
{
    unsigned long len = c_arraySize(src->value);

    if ( dst->value._maximum > 0 ) {
        if ( len != dst->value._maximum ) {
            if ( dst->value._release ) {
                gapi_free(dst->value._buffer);
            }
            dst->value._maximum = 0;
            dst->value._length  = 0;
            dst->value._buffer  = NULL;
        }
    }

    if ( len > 0 ) {
        if ( dst->value._length == 0 ) {
            if ( dst->value._maximum == 0 ) {
                dst->value._buffer  = gapi_octetSeq_allocbuf(len) ;
                dst->value._maximum = len;
                dst->value._length  = 0;
                dst->value._release = TRUE;
            }
        }
        if ( dst->value._maximum >= len ) {
            memcpy(dst->value._buffer, src->value, len);
        }
    }

    dst->value._length = len;
}

static void
builtinUserDataQosPolicyCopyin (
    c_base base,
    const gapi_userDataQosPolicy   *src,
    struct v_builtinUserDataPolicy *dst)
{
    static c_type gapi_builtinUserData_type = NULL;

    if ( src->value._length > 0 ) {
        if (gapi_builtinUserData_type == NULL) {
            c_type type = c_octet_t(base);
            if (type) {
                gapi_builtinUserData_type =
                    c_metaArrayTypeNew(c_metaObject(base),
                                       "C_ARRAY<c_octet>",
                                       type,0);
            }
        }

        if ( gapi_builtinUserData_type ) {
            dst->value = c_newArray(c_collectionType(gapi_builtinUserData_type),
                                    src->value._length);
            if ( dst->value ) {
                memcpy(dst->value, src->value._buffer, src->value._length);
            }
        }
    } else {
        dst->value = NULL;
    }
}

static void
builtinTopicDataQosPolicyCopyout (
    const struct v_builtinTopicDataPolicy *src,
    gapi_topicDataQosPolicy               *dst)
{
    unsigned long len = c_arraySize(src->value);

    if ( dst->value._maximum > 0 ) {
        if ( len != dst->value._maximum ) {
            if ( dst->value._release ) {
                gapi_free(dst->value._buffer);
            }
            dst->value._maximum = 0;
            dst->value._length  = 0;
            dst->value._buffer  = NULL;
        }
    }

    if ( len > 0 ) {
        if ( dst->value._length == 0 ) {
            if ( dst->value._maximum == 0 ) {
                dst->value._buffer  = gapi_octetSeq_allocbuf(len) ;
                dst->value._maximum = len;
                dst->value._length  = 0;
                dst->value._release = TRUE;
            }
        }
        if ( dst->value._maximum >= len ) {
            memcpy(dst->value._buffer, src->value, len);
        }
    }

    dst->value._length = len;
}

static void
builtinTopicDataQosPolicyCopyin (
    c_base base,
    const gapi_topicDataQosPolicy   *src,
    struct v_builtinTopicDataPolicy *dst)
{
    static c_type gapi_topicData_type = NULL;

    if ( src->value._length > 0 ) {
        if (gapi_topicData_type == NULL) {
            c_type type = c_octet_t(base);
            if (type) {
                gapi_topicData_type =
                        c_metaArrayTypeNew(c_metaObject(base),
                                           "C_ARRAY<c_char>",
                                           type,0);
                c_free(type);
            }
        }

        if ( gapi_topicData_type ) {
            dst->value = c_newArray(c_collectionType(gapi_topicData_type),
                                    src->value._length);
            if ( dst->value ) {
                memcpy(dst->value, src->value._buffer, src->value._length);
            }
        }
    } else {
        dst->value = NULL;
    }
}

static void
builtinPartitionQosPolicyCopyout (
    const struct v_builtinPartitionPolicy *src,
    gapi_partitionQosPolicy               *dst)
{
    unsigned long len = c_arraySize(src->name);

    if ( dst->name._maximum > 0 ) {
        if ( len != dst->name._maximum ) {
            if ( dst->name._release ) {
                gapi_free(dst->name._buffer);
            }
            dst->name._maximum = 0;
            dst->name._length  = 0;
            dst->name._buffer  = NULL;
        }
    }

    if ( len > 0 ) {
        if ( dst->name._length == 0 ) {
            if ( dst->name._maximum == 0 ) {
                dst->name._buffer  = gapi_stringSeq_allocbuf(len) ;
                dst->name._maximum = len;
                dst->name._length  = 0;
                dst->name._release = TRUE;
            }
        }
        if ( dst->name._maximum >= len ) {
            unsigned long i;
            for ( i = 0; i < len; i++ ) {
                dst->name._buffer[i] = gapi_string_dup(src->name[i]);
            }
        }
    }

    dst->name._length = len;
}

static void
builtinPartitionQosPolicyCopyin (
    c_base base,
    const gapi_partitionQosPolicy   *src,
    struct v_builtinPartitionPolicy *dst)
{
    static c_type gapi_partitionQos_type = NULL;

    if ( src->name._length > 0 ) {
        if (gapi_partitionQos_type == NULL) {
            c_type type = c_string_t(base);
            if (type) {
                gapi_partitionQos_type =
                        c_metaSequenceTypeNew(c_metaObject(base),
                                           "C_SEQUENCE<c_string>",
                                           type,0);
                c_free(type);
            }
        }

        if ( gapi_partitionQos_type ) {
            dst->name = c_newSequence(c_collectionType(gapi_partitionQos_type),
                                   src->name._length);
            if ( dst->name ) {
                gapi_unsigned_long i;
                for ( i = 0; i < src->name._length; i++ ) {
                    if ( src->name._buffer[i] ) {
                        dst->name[i] = c_stringNew(base, src->name._buffer[i]);
                    } else {
                        dst->name[i] = NULL;
                    }
                }
            }
        }
    } else {
        dst->name = NULL;
    }
}

static void
builtinGroupDataQosPolicyCopyout (
    const struct v_builtinGroupDataPolicy *src,
    gapi_groupDataQosPolicy               *dst)
{
    unsigned long len = c_arraySize(src->value);

    if ( dst->value._maximum > 0 ) {
        if ( len != dst->value._maximum ) {
            if ( dst->value._release ) {
                gapi_free(dst->value._buffer);
            }
            dst->value._maximum = 0;
            dst->value._length  = 0;
            dst->value._buffer  = NULL;
        }
    }

    if ( len > 0 ) {
        if ( dst->value._length == 0 ) {
            if ( dst->value._maximum == 0 ) {
                dst->value._buffer  = gapi_octetSeq_allocbuf(len) ;
                dst->value._maximum = len;
                dst->value._length  = 0;
                dst->value._release = TRUE;
            }
        }
        if ( dst->value._maximum >= len ) {
            memcpy(dst->value._buffer, src->value, len);
        }
    }

    dst->value._length = len;
}

static void
builtinGroupDataQosPolicyCopyin (
    c_base base,
    const gapi_groupDataQosPolicy   *src,
    struct v_builtinGroupDataPolicy *dst)
{
    static c_type gapi_groupData_type = NULL;

    if ( src->value._length > 0 ) {
        if (gapi_groupData_type == NULL) {
            c_type type = c_octet_t(base);
            if (type) {
                gapi_groupData_type =
                        c_metaArrayTypeNew(c_metaObject(base),
                                           "C_ARRAY<c_octet>",
                                           type,0);
                c_free(type);
            }
        }

        if ( gapi_groupData_type ) {
            dst->value = c_newArray(c_collectionType(gapi_groupData_type),
                                    src->value._length);
            if ( dst->value ) {
                memcpy(dst->value, src->value._buffer, src->value._length);
            }
        }
    } else {
        dst->value = NULL;
    }
}

static void
userDataQosPolicyCopyout (
    const struct v_userDataPolicy *src,
    gapi_userDataQosPolicy        *dst)
{
    unsigned long len = c_arraySize(src->value);

    if ( dst->value._maximum > 0 ) {
        if ( len != dst->value._maximum ) {
            if ( dst->value._release ) {
                gapi_free(dst->value._buffer);
            }
            dst->value._maximum = 0;
            dst->value._length  = 0;
            dst->value._buffer  = NULL;
        }
    }

    if ( len > 0 ) {
        if ( dst->value._length == 0 ) {
            if ( dst->value._maximum == 0 ) {
                dst->value._buffer  = gapi_octetSeq_allocbuf(len) ;
                dst->value._maximum = len;
                dst->value._length  = 0;
                dst->value._release = TRUE;
            }
        }
        if ( dst->value._maximum >= len ) {
            memcpy(dst->value._buffer, src->value, len);
        }
    }

    dst->value._length = len;
}

static void
userDataQosPolicyCopyin (
    c_base base,
    const gapi_userDataQosPolicy *src,
    struct v_userDataPolicy      *dst)
{
    static c_type gapi_userData_type = NULL;

    if ( src->value._length > 0 ) {
        if (gapi_userData_type == NULL) {
            c_type type = c_octet_t(base);
            if (type) {
                gapi_userData_type =
                        c_metaArrayTypeNew(c_metaObject(base),
                                           "C_ARRAY<c_octet>",
                                           type,0);
                c_free(type);
            }
        }

        if ( gapi_userData_type ) {
            dst->value = c_newArray(c_collectionType(gapi_userData_type),
                                    src->value._length);
            if ( dst->value ) {
                memcpy(dst->value, src->value._buffer, src->value._length);
            }
        }
    } else {
        dst->value = NULL;
    }
}

#define durabilityServiceQosPolicyCopyin(src,dst) \
        { \
            ((struct v_durabilityServicePolicy *)dst)->history_kind = \
            ((gapi_durabilityServiceQosPolicy *)src)->history_kind; \
            ((struct v_durabilityServicePolicy *)dst)->history_depth = \
            ((gapi_durabilityServiceQosPolicy *)src)->history_depth; \
            ((struct v_durabilityServicePolicy *)dst)->max_samples = \
            ((gapi_durabilityServiceQosPolicy *)src)->max_samples; \
            ((struct v_durabilityServicePolicy *)dst)->max_instances = \
            ((gapi_durabilityServiceQosPolicy *)src)->max_instances; \
            ((struct v_durabilityServicePolicy *)dst)->max_samples_per_instance = \
            ((gapi_durabilityServiceQosPolicy *)src)->max_samples_per_instance; \
            kernelCopyInDuration(&((gapi_durabilityServiceQosPolicy *)src)->service_cleanup_delay, \
                                 &((struct v_durabilityServicePolicy *)dst)->service_cleanup_delay); \
        }

#define durabilityServiceQosPolicyCopyout(src,dst) \
        { \
            ((gapi_durabilityServiceQosPolicy *)dst)->history_kind = \
            ((struct v_durabilityServicePolicy *)src)->history_kind; \
            ((gapi_durabilityServiceQosPolicy *)dst)->history_depth = \
            ((struct v_durabilityServicePolicy *)src)->history_depth; \
            ((gapi_durabilityServiceQosPolicy *)dst)->max_samples = \
            ((struct v_durabilityServicePolicy *)src)->max_samples; \
            ((gapi_durabilityServiceQosPolicy *)dst)->max_samples_per_instance = \
            ((struct v_durabilityServicePolicy *)src)->max_instances; \
            ((gapi_durabilityServiceQosPolicy *)dst)->max_samples_per_instance = \
            ((struct v_durabilityServicePolicy *)src)->max_samples_per_instance; \
            kernelCopyOutDuration(&((struct v_durabilityServicePolicy *)src)->service_cleanup_delay, \
                                 &((gapi_durabilityServiceQosPolicy *)dst)->service_cleanup_delay); \
        }

#define presentationQosPolicyCopyin(src,dst) \
        { \
            ((struct v_presentationPolicy *)dst)->access_scope = \
            ((gapi_presentationQosPolicy *)src)->access_scope; \
            ((struct v_presentationPolicy *)dst)->coherent_access = \
            ((gapi_presentationQosPolicy *)src)->coherent_access; \
            ((struct v_presentationPolicy *)dst)->ordered_access = \
            ((gapi_presentationQosPolicy *)src)->ordered_access; \
        }

#define presentationQosPolicyCopyout(src,dst) \
        { \
            ((gapi_presentationQosPolicy *)dst)->access_scope = \
            ((struct v_presentationPolicy *)src)->access_scope; \
            ((gapi_presentationQosPolicy *)dst)->coherent_access = \
            ((struct v_presentationPolicy *)src)->coherent_access; \
            ((gapi_presentationQosPolicy *)dst)->ordered_access = \
            ((struct v_presentationPolicy *)src)->ordered_access; \
        }

#define livelinessQosPolicyCopyin(src,dst) \
        { \
            ((struct v_livelinessPolicy *)dst)->kind = \
            ((gapi_livelinessQosPolicy *)src)->kind; \
            kernelCopyInDuration(&((gapi_livelinessQosPolicy *)src)->lease_duration, \
                                 &((struct v_livelinessPolicy *)dst)->lease_duration); \
        }

#define livelinessQosPolicyCopyout(src,dst) \
        { \
            ((gapi_livelinessQosPolicy *)dst)->kind = \
            ((struct v_livelinessPolicy *)src)->kind; \
            kernelCopyOutDuration(&((struct v_livelinessPolicy *)src)->lease_duration, \
                                 &((gapi_livelinessQosPolicy *)dst)->lease_duration); \
        }

#define reliabilityQosPolicyCopyin(src,dst) \
        { \
            ((struct v_reliabilityPolicy *)dst)->kind = \
            ((gapi_reliabilityQosPolicy *)src)->kind; \
            kernelCopyInDuration(&((gapi_reliabilityQosPolicy *)src)->max_blocking_time, \
                                 &((struct v_reliabilityPolicy *)dst)->max_blocking_time); \
            ((struct v_reliabilityPolicy *)dst)->synchronous = \
            ((gapi_reliabilityQosPolicy *)src)->synchronous; \
        }

#define reliabilityQosPolicyCopyout(src,dst) \
        { \
            ((gapi_reliabilityQosPolicy *)dst)->kind = \
            ((struct v_reliabilityPolicy *)src)->kind; \
            kernelCopyOutDuration(&((struct v_reliabilityPolicy *)src)->max_blocking_time, \
                                 &((gapi_reliabilityQosPolicy *)dst)->max_blocking_time); \
            ((gapi_reliabilityQosPolicy *)dst)->synchronous = \
            ((struct v_reliabilityPolicy *)src)->synchronous; \
        }

#define historyQosPolicyCopyin(src,dst) \
        { \
            ((struct v_historyPolicy *)dst)->kind = \
            ((gapi_historyQosPolicy *)src)->kind; \
            ((struct v_historyPolicy *)dst)->depth = \
            ((gapi_historyQosPolicy *)src)->depth; \
        }

#define historyQosPolicyCopyout(src,dst) \
        { \
            ((gapi_historyQosPolicy *)dst)->kind = \
            ((struct v_historyPolicy *)src)->kind; \
            ((gapi_historyQosPolicy *)dst)->depth = \
            ((struct v_historyPolicy *)src)->depth; \
        }

#define resourceLimitsQosPolicyCopyin(src,dst) \
        { \
            ((struct v_resourcePolicy *)dst)->max_samples = \
            ((gapi_resourceLimitsQosPolicy *)src)->max_samples; \
            ((struct v_resourcePolicy *)dst)->max_instances = \
            ((gapi_resourceLimitsQosPolicy *)src)->max_instances; \
            ((struct v_resourcePolicy *)dst)->max_samples_per_instance = \
            ((gapi_resourceLimitsQosPolicy *)src)->max_samples_per_instance; \
        }

#define resourceLimitsQosPolicyCopyout(src,dst) \
        { \
            ((gapi_resourceLimitsQosPolicy *)dst)->max_samples = \
            ((struct v_resourcePolicy *)src)->max_samples; \
            ((gapi_resourceLimitsQosPolicy *)dst)->max_instances = \
            ((struct v_resourcePolicy *)src)->max_instances; \
            ((gapi_resourceLimitsQosPolicy *)dst)->max_samples_per_instance = \
            ((struct v_resourcePolicy *)src)->max_samples_per_instance; \
        }

#define durabilityQosPolicyCopyin(src,dst) \
        ((struct v_durabilityPolicy *)dst)->kind = ((gapi_durabilityQosPolicy *)src)->kind

#define durabilityQosPolicyCopyout(src,dst) \
        ((gapi_durabilityQosPolicy *)dst)->kind = ((struct v_durabilityPolicy *)src)->kind

#define deadlineQosPolicyCopyin(src,dst) \
        kernelCopyInDuration(&((gapi_deadlineQosPolicy *)src)->period, \
                             &((struct v_deadlinePolicy*)dst)->period)

#define deadlineQosPolicyCopyout(src,dst) \
        kernelCopyOutDuration(&((struct v_deadlinePolicy *)src)->period, \
                             &((gapi_deadlineQosPolicy *)dst)->period)

#define latencyBudgetQosPolicyCopyin(src,dst) \
        kernelCopyInDuration(&((gapi_latencyBudgetQosPolicy *)src)->duration, \
                             &((struct v_latencyPolicy *)dst)->duration)

#define latencyBudgetQosPolicyCopyout(src,dst) \
        kernelCopyOutDuration(&((struct v_latencyPolicy *)src)->duration, \
                             &((gapi_latencyBudgetQosPolicy *)dst)->duration)

#define ownershipQosPolicyCopyin(src,dst) \
        ((struct v_ownershipPolicy *)dst)->kind = ((gapi_ownershipQosPolicy *)src)->kind

#define ownershipQosPolicyCopyout(src,dst) \
        ((gapi_ownershipQosPolicy *)dst)->kind = ((struct v_ownershipPolicy *)src)->kind

#define ownershipStrengthQosPolicyCopyin(src,dst) \
        ((struct v_strengthPolicy *)dst)->value = ((gapi_ownershipStrengthQosPolicy *)src)->value

#define ownershipStrengthQosPolicyCopyout(src,dst) \
        ((gapi_ownershipStrengthQosPolicy *)dst)->value = ((struct v_strengthPolicy *)src)->value

#define timeBasedFilterQosPolicyCopyin(src,dst) \
        kernelCopyInDuration(&((gapi_timeBasedFilterQosPolicy *)src)->minimum_separation, \
                             &((struct v_pacingPolicy *)dst)->minSeperation)

#define timeBasedFilterQosPolicyCopyout(src,dst) \
        kernelCopyOutDuration(&((struct v_pacingPolicy *)src)->minSeperation, \
                              &((gapi_timeBasedFilterQosPolicy *)dst)->minimum_separation)

#define destinationOrderQosPolicyCopyin(src,dst) \
        ((struct v_orderbyPolicy *)dst)->kind = ((gapi_destinationOrderQosPolicy *)src)->kind

#define destinationOrderQosPolicyCopyout(src,dst) \
        ((gapi_destinationOrderQosPolicy *)dst)->kind = ((struct v_orderbyPolicy *)src)->kind

#define entityFactoryQosPolicyCopyin(src,dst) \
        ((struct v_entityFactoryPolicy *)dst)->autoenable_created_entities = \
        ((gapi_entityFactoryQosPolicy *)src)->autoenable_created_entities

#define entityFactoryQosPolicyCopyout(src,dst) \
        ((gapi_entityFactoryQosPolicy *)dst)->autoenable_created_entities = \
        ((struct v_entityFactoryPolicy *)src)->autoenable_created_entities

#define writerDataLifecycleQosPolicyCopyout(src,dst) \
        ((gapi_writerDataLifecycleQosPolicy *)dst)->autodispose_unregistered_instances = \
        ((struct v_writerLifecyclePolicy *)src)->autodispose_unregistered_instances

#define writerDataLifecycleQosPolicyCopyin(src,dst) \
        ((struct v_writerLifecyclePolicy *)dst)->autodispose_unregistered_instances = \
        ((gapi_writerDataLifecycleQosPolicy *)src)->autodispose_unregistered_instances

#define readerDataLifecycleQosPolicyCopyin(src,dst) \
        kernelCopyInDuration(&((gapi_readerDataLifecycleQosPolicy *)src)->autopurge_nowriter_samples_delay, \
                             &((struct v_readerLifecyclePolicy *)dst)->autopurge_nowriter_samples_delay)

#define readerDataLifecycleQosPolicyCopyout(src,dst) \
        kernelCopyOutDuration(&((struct v_readerLifecyclePolicy *)src)->autopurge_nowriter_samples_delay, \
                              &((gapi_readerDataLifecycleQosPolicy *)dst)->autopurge_nowriter_samples_delay)

#define transportPriorityQosPolicyCopyin(src,dst) \
        ((struct v_transportPolicy *)dst)->value = \
        ((gapi_transportPriorityQosPolicy *)src)->value

#define transportPriorityQosPolicyCopyout(src,dst) \
        ((gapi_transportPriorityQosPolicy *)dst)->value = \
        ((struct v_transportPolicy  *)src)->value

#define lifespanQosPolicyCopyin(src,dst) \
        kernelCopyInDuration(&((gapi_lifespanQosPolicy *)src)->duration, \
                             &((struct v_lifespanPolicy *)dst)->duration)

#define lifespanQosPolicyCopyout(src,dst) \
        kernelCopyOutDuration(&((struct v_lifespanPolicy *)src)->duration, \
                              &((gapi_lifespanQosPolicy *)dst)->duration)


void
gapi_participantBuiltinTopicData__copyOut (
    void *_from,
    void *_to)
{
    const struct v_participantInfo   *from = (struct v_participantInfo *)_from;
    gapi_participantBuiltinTopicData *to   = (gapi_participantBuiltinTopicData *)_to;

    builtinTopicKeyCopyout(&from->key, &to->key);
    userDataQosPolicyCopyout(&from->user_data, &to->user_data);
}

gapi_boolean
gapi_participantBuiltinTopicData__copyIn (
    c_base base,
    void *_from,
    void *_to)
{
#ifdef BUILTIN_TOPIC_COPY_IN
    const gapi_participantBuiltinTopicData *from = (gapi_participantBuiltinTopicData *)_from;
    struct v_participantInfo               *to   = (struct v_participantInfo *)_to;

    builtinTopicKeyCopyin(&from->key, &to->key);
    userDataQosPolicyCopyin(base, &from->user_data, &to->user_data);

#endif
    return TRUE;
}


void
gapi_topicBuiltinTopicData__copyOut (
    void *_from,
    void *_to)
{
    struct v_topicInfo         *from = (struct v_topicInfo *)_from;
    gapi_topicBuiltinTopicData *to   = (gapi_topicBuiltinTopicData *)_to;

    builtinTopicKeyCopyout(&from->key, &to->key);

    to->name       = gapi_string_dup(from->name);
    to->type_name  = gapi_string_dup(from->type_name);

    durabilityQosPolicyCopyout(&from->durability, &to->durability);
    durabilityServiceQosPolicyCopyout(&from->durabilityService, &to->durability_service);
    deadlineQosPolicyCopyout(&from->deadline, &to->deadline);
    latencyBudgetQosPolicyCopyout(&from->latency_budget, &to->latency_budget);
    livelinessQosPolicyCopyout(&from->liveliness, &to->liveliness);
    reliabilityQosPolicyCopyout(&from->reliability, &to->reliability);
    transportPriorityQosPolicyCopyout(&from->transport_priority, &to->transport_priority);
    lifespanQosPolicyCopyout(&from->lifespan, &to->lifespan);
    destinationOrderQosPolicyCopyout(&from->destination_order, &to->destination_order);
    historyQosPolicyCopyout(&from->history, &to->history);
    resourceLimitsQosPolicyCopyout(&from->resource_limits, &to->resource_limits);
    ownershipQosPolicyCopyout(&from->ownership, &to->ownership);
    builtinTopicDataQosPolicyCopyout(&from->topic_data, &to->topic_data);
}

gapi_boolean
gapi_topicBuiltinTopicData__copyIn (
    c_base base,
    void *_from,
    void *_to)
{
#ifdef BUILTIN_TOPIC_COPY_IN
    const gapi_topicBuiltinTopicData *from = (gapi_topicBuiltinTopicData *)_from;
    struct v_topicInfo               *to   = (struct v_topicInfo *)_to;

    builtinTopicKeyCopyin(&from->key, &to->key);

    if ( from->name ) {
        to->name = c_stringNew(base, from->name);
    } else {
        to->name = NULL;
    }

    if ( from->type_name ) {
        to->type_name  = c_stringNew(base, from->type_name);
    } else {
        to->type_name = NULL;
    }

    durabilityQosPolicyCopyin(&from->durability, &to->durability);
    durabilityServiceQosPolicyCopyin(&from->durability_service, &to->durabilityService);
    deadlineQosPolicyCopyin(&from->deadline, &to->deadline);
    latencyBudgetQosPolicyCopyin(&from->latency_budget, &to->latency_budget);
    livelinessQosPolicyCopyin(&from->liveliness, &to->liveliness);
    reliabilityQosPolicyCopyin(&from->reliability, &to->reliability);
    transportPriorityQosPolicyCopyin(&from->transport_priority, &to->transport_priority);
    lifespanQosPolicyCopyin(&from->lifespan, &to->lifespan);
    destinationOrderQosPolicyCopyin(&from->destination_order, &to->destination_order);
    historyQosPolicyCopyin(&from->history, &to->history);
    resourceLimitsQosPolicyCopyin(&from->resource_limits, &to->resource_limits);
    ownershipQosPolicyCopyin(&from->ownership, &to->ownership);
    builtinTopicDataQosPolicyCopyin(base, &from->topic_data, &to->topic_data);
#endif
    return TRUE;
}

v_result
gapi_publicationBuiltinTopicData__copyOut (
    void *_from,
    void *_to)
{
    struct v_publicationInfo         *from = (struct v_publicationInfo *)_from;
    gapi_publicationBuiltinTopicData *to   = (gapi_publicationBuiltinTopicData *)_to;

    builtinTopicKeyCopyout(&from->key, &to->key);
    builtinTopicKeyCopyout(&from->participant_key, &to->participant_key);

    to->topic_name = gapi_string_dup(from->topic_name);
    to->type_name  = gapi_string_dup(from->type_name);

    durabilityQosPolicyCopyout(&from->durability, &to->durability);
    deadlineQosPolicyCopyout(&from->deadline, &to->deadline);
    latencyBudgetQosPolicyCopyout(&from->latency_budget, &to->latency_budget);
    livelinessQosPolicyCopyout(&from->liveliness, &to->liveliness);
    reliabilityQosPolicyCopyout(&from->reliability, &to->reliability);
    lifespanQosPolicyCopyout(&from->lifespan, &to->lifespan);
    destinationOrderQosPolicyCopyout(&from->destination_order, &to->destination_order);
    builtinUserDataQosPolicyCopyout(&from->user_data, &to->user_data);
    ownershipQosPolicyCopyout(&from->ownership, &to->ownership);
    ownershipStrengthQosPolicyCopyout(&from->ownership_strength, &to->ownership_strength);
    presentationQosPolicyCopyout(&from->presentation, &to->presentation);
    builtinPartitionQosPolicyCopyout(&from->partition, &to->partition);
    builtinTopicDataQosPolicyCopyout(&from->topic_data, &to->topic_data);
    builtinGroupDataQosPolicyCopyout(&from->group_data, &to->group_data);
    return V_RESULT_OK;
}

gapi_boolean
gapi_publicationBuiltinTopicData__copyIn (
    c_base base,
    void *_from,
    void *_to)
{
#ifdef BUILTIN_TOPIC_COPY_IN
    const gapi_publicationBuiltinTopicData *from = (gapi_publicationBuiltinTopicData *)_from;
    struct v_publicationInfo               *to   = (struct v_publicationInfo *)_to;

    builtinTopicKeyCopyin(&from->key, &to->key);
    builtinTopicKeyCopyin(&from->participant_key, &to->participant_key);

    if ( from->topic_name ) {
        to->topic_name = c_stringNew(base, from->topic_name);
    } else {
        to->topic_name = NULL;
    }

    if ( from->type_name ) {
        to->type_name  = c_stringNew(base, from->type_name);
    } else {
        to->type_name = NULL;
    }

    durabilityQosPolicyCopyin(&from->durability, &to->durability);
    deadlineQosPolicyCopyin(&from->deadline, &to->deadline);
    latencyBudgetQosPolicyCopyin(&from->latency_budget, &to->latency_budget);
    livelinessQosPolicyCopyin(&from->liveliness, &to->liveliness);
    reliabilityQosPolicyCopyin(&from->reliability, &to->reliability);
    lifespanQosPolicyCopyin(&from->lifespan, &to->lifespan);
    destinationOrderQosPolicyCopyin(&from->destination_order, &to->destination_order);
    builtinUserDataQosPolicyCopyin(base, &from->user_data, &to->user_data);
    ownershipQosPolicyCopyin(&from->ownership, &to->ownership);
    ownershipStrengthQosPolicyCopyin(&from->ownership_strength, &to->ownership_strength);
    presentationQosPolicyCopyin(&from->presentation, &to->presentation);
    builtinPartitionQosPolicyCopyin(base, &from->partition, &to->partition);
    builtinTopicDataQosPolicyCopyin(base, &from->topic_data, &to->topic_data);
    builtinGroupDataQosPolicyCopyin(base, &from->group_data, &to->group_data);
#endif
    return TRUE;
}

v_result
gapi_subscriptionBuiltinTopicData__copyOut (
    void *_from,
    void *_to)
{
    struct v_subscriptionInfo         *from = (struct v_subscriptionInfo *)_from;
    gapi_subscriptionBuiltinTopicData *to   = (gapi_subscriptionBuiltinTopicData *)_to;

    builtinTopicKeyCopyout(&from->key, &to->key);
    builtinTopicKeyCopyout(&from->participant_key, &to->participant_key);

    to->topic_name = gapi_string_dup(from->topic_name);
    to->type_name  = gapi_string_dup(from->type_name);

    durabilityQosPolicyCopyout(&from->durability, &to->durability);
    deadlineQosPolicyCopyout(&from->deadline, &to->deadline);
    latencyBudgetQosPolicyCopyout(&from->latency_budget, &to->latency_budget);
    livelinessQosPolicyCopyout(&from->liveliness, &to->liveliness);
    reliabilityQosPolicyCopyout(&from->reliability, &to->reliability);
    destinationOrderQosPolicyCopyout(&from->destination_order, &to->destination_order);
    builtinUserDataQosPolicyCopyout(&from->user_data, &to->user_data);
    ownershipQosPolicyCopyout(&from->ownership, &to->ownership);
    timeBasedFilterQosPolicyCopyout(&from->time_based_filter, &to->time_based_filter);
    presentationQosPolicyCopyout(&from->presentation, &to->presentation);
    builtinPartitionQosPolicyCopyout(&from->partition, &to->partition);
    builtinTopicDataQosPolicyCopyout(&from->topic_data, &to->topic_data);
    builtinGroupDataQosPolicyCopyout(&from->group_data, &to->group_data);
    return V_RESULT_OK;
}

gapi_boolean
gapi_subscriptionBuiltinTopicData__copyIn (
    c_base base,
    void *_from,
    void *_to)
{
#ifdef BUILTIN_TOPIC_COPY_IN
    const gapi_subscriptionBuiltinTopicData *from = (gapi_subscriptionBuiltinTopicData *)_from;
    struct v_subscriptionInfo               *to   = (struct v_subscriptionInfo *)_to;

    builtinTopicKeyCopyin(&from->key, &to->key);
    builtinTopicKeyCopyin(&from->participant_key, &to->participant_key);

    if ( from->topic_name ) {
        to->topic_name = c_stringNew(base, from->topic_name);
    } else {
        to->topic_name = NULL;
    }

    if ( from->type_name ) {
        to->type_name  = c_stringNew(base, from->type_name);
    } else {
        to->type_name = NULL;
    }

    durabilityQosPolicyCopyin(&from->durability, &to->durability);
    deadlineQosPolicyCopyin(&from->deadline, &to->deadline);
    latencyBudgetQosPolicyCopyin(&from->latency_budget, &to->latency_budget);
    livelinessQosPolicyCopyin(&from->liveliness, &to->liveliness);
    reliabilityQosPolicyCopyin(&from->reliability, &to->reliability);
    destinationOrderQosPolicyCopyin(&from->destination_order, &to->destination_order);
    builtinUserDataQosPolicyCopyin(base, &from->user_data, &to->user_data);
    ownershipQosPolicyCopyin(&from->ownership, &to->ownership);
    timeBasedFilterQosPolicyCopyin(&from->time_based_filter, &to->time_based_filter);
    presentationQosPolicyCopyin(&from->presentation, &to->presentation);
    builtinPartitionQosPolicyCopyin(base, &from->partition, &to->partition);
    builtinTopicDataQosPolicyCopyin(base, &from->topic_data, &to->topic_data);
    builtinGroupDataQosPolicyCopyin(base, &from->group_data, &to->group_data);
#endif
    return TRUE;
}

