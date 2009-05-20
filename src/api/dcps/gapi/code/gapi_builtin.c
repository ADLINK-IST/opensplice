/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech
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
#include "gapi_dataReaderStatus.h"
#include "gapi_structured.h"
#include "gapi_subscriberStatus.h"
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


const gapi_dataReaderQos  _builtinDataReaderQos = {
    /* gapi_durabilityQosPolicy durability */
    {
        GAPI_TRANSIENT_DURABILITY_QOS   /* gapi_durabilityQosPolicyKind kind */
    },
    /* gapi_deadlineQosPolicy deadline */
    {
        GAPI_DURATION_INFINITE          /* gapi_duration_t period */
    },
    /* gapi_latencyBudgetQosPolicy latency_budget */
    {
        GAPI_DURATION_ZERO              /* gapi_duration_t duration */
    },
    /* gapi_livelinessQosPolicy liveliness */
    {
        GAPI_AUTOMATIC_LIVELINESS_QOS,  /* gapi_livelinessQosPolicyKind kind */
        GAPI_DURATION_INFINITE          /* gapi_duration_t lease_duration */
    },
    /* gapi_reliabilityQosPolicy reliability */
    {
        GAPI_RELIABLE_RELIABILITY_QOS,  /* gapi_reliabilityQosPolicyKind kind */
        GAPI_DURATION_INFINITE          /* gapi_duration_t max_blocking_time */
    },
    /* gapi_destinationOrderQosPolicy destination_order */
    {
        GAPI_BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS   /* gapi_destinationOrderQosPolicyKind kind */
    },
    /* gapi_historyQosPolicy history */
    {
        GAPI_KEEP_LAST_HISTORY_QOS,     /* gapi_historyQosPolicyKind kind */
        1                               /* gapi_long depth */
    },
    /* gapi_resourceLimitsQosPolicy resource_limits */
    {
        GAPI_LENGTH_UNLIMITED,          /* gapi_long max_samples */
        GAPI_LENGTH_UNLIMITED,          /* gapi_long max_instances */
        GAPI_LENGTH_UNLIMITED           /* gapi_long max_samples_per_instance */
    },
    /* gapi_userDataQosPolicy user_data */
    {
        /* gapi_octetSeq value */
        {
            0,          /* _maximum */
            0,          /* _length  */
            NULL,       /* _buffer  */
            FALSE       /* _release */
        }
    },
    {
        GAPI_SHARED_OWNERSHIP_QOS
    },
    /* gapi_timeBasedFilterQosPolicy time_based_filter */
    {
        GAPI_DURATION_ZERO              /* gapi_duration_t minimum_separation */
    },
    /* gapi_readerDataLifecycleQosPolicy reader_data_lifecycle */
    {
        GAPI_DURATION_INFINITE,          /* gapi_duration_t autopurge_nowriter_samples_delay */
        GAPI_DURATION_INFINITE,          /* gapi_duration_t autopurge_disposed_samples_delay */
        TRUE                             /* enable_invalid_samples */
    },
    /* gapi_subscriptionKeyQosPolicy subscription_keys; */
    {
        /* gapi_boolean use_key_list */
        FALSE,
        /* gapi_stringSeq key_list */
        {
            0,          /* _maximum */
            0,          /* _length  */
            NULL,       /* _buffer  */
            FALSE       /* _release */
        }
    },
    /* gapi_readerDataLifespanQosPolicy reader_lifespan */
    {
        FALSE,                  /* gapi_boolean    use_lifespan */
        GAPI_DURATION_INFINITE, /* gapi_duration_t duration     */
    },
    /* gapi_shareQosPolicy share */
    {
        NULL,
        FALSE
    }
};

const gapi_subscriberQos  _builtinSubscriberQos = {
    /* gapi_presentationQosPolicy presentation */
    {
        GAPI_TOPIC_PRESENTATION_QOS,    /* gapi_presentationQosPolicyAccessScopeKind access_scope */
        FALSE,                          /* gapi_boolean coherent_access */
        FALSE                           /* gapi_boolean ordered_access */
    },
    /* gapi_partitionQosPolicy partition */
    {
        /* gapi_stringSeq name */
        {
            0,          /* _maximum */
            0,          /* _length  */
            NULL,       /* _buffer  */
            FALSE       /* _release */
        }
    },
    /* gapi_groupDataQosPolicy group_data */
    {
        /* gapi_octetSeq value */
        {
            0,          /* _maximum */
            0,          /* _length  */
            NULL,       /* _buffer  */
            FALSE       /* _release */
        }
    },
    /* gapi_entityFactoryQosPolicy entity_factory */
    {
        TRUE            /* gapi_boolean autoenable_created_entities */
    },
    /* gapi_shareQosPolicy share */
    {
        NULL,
        FALSE
    }
};

static void
_BuiltinDataReaderFree (
    _DataReader dataReader);

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


_Subscriber
_SubscriberBuiltinNew (
    u_participant uParticipant,
    _DomainParticipantFactory factory,
    _DomainParticipant participant)
{
    _Subscriber newSubscriber = _SubscriberAlloc();

    if (newSubscriber != NULL) {
        _DomainEntityInit (_DomainEntity(newSubscriber),
                           participant,
                           _Entity(participant),
                           TRUE);

        newSubscriber->dataReaderSet = gapi_setNew(gapi_objectRefCompare);
        if (newSubscriber->dataReaderSet == NULL) {
            _DomainEntityDispose(_DomainEntity(newSubscriber));
            newSubscriber = NULL;
        }

        if ( newSubscriber ) {
            U_SUBSCRIBER_SET(newSubscriber, u_participantGetBuiltinSubscriber(uParticipant));
            if (U_SUBSCRIBER_GET(newSubscriber) == NULL) {
                _DomainEntityDispose(_DomainEntity(newSubscriber));
                newSubscriber = NULL;
            }
        }

        if ( newSubscriber ) {
            _EntityStatus(newSubscriber) = _Status(_SubscriberStatusNew(newSubscriber, NULL, 0));
            if ( _EntityStatus(newSubscriber) == NULL ) {
                _DomainEntityDispose(_DomainEntity(newSubscriber));
                newSubscriber = NULL;
            }
        }

        if ( newSubscriber ) {
            long i;

            for ( i = 0; i < MAX_BUILTIN_TOPIC; i++ ) {
                _DataReader reader;

                reader = _BuiltinDataReaderNew(newSubscriber, _BuiltinTopicName(i));
                if ( reader ) {
                    gapi_setAdd(newSubscriber->dataReaderSet, (gapi_object)reader);
                    _ENTITY_REGISTER_OBJECT(_Entity(newSubscriber), (_Object)reader);
                    _EntityRelease(reader);
                }
            }

            newSubscriber->builtin = TRUE;
            _EntityEnabled(newSubscriber) = TRUE;
        }
    }

    return newSubscriber;
}


void
_BuiltinSubscriberFree (
    _Subscriber subscriber)
{
    gapi_setIter iterSet;

    assert(subscriber != NULL);

    iterSet = gapi_setFirst(subscriber->dataReaderSet);
    while ( gapi_setIterObject(iterSet) ) {
        _DataReader datareader = _DataReader(gapi_setIterObject(iterSet));
        _EntityClaim(datareader);
        _BuiltinDataReaderFree(datareader);
        gapi_setIterRemove (iterSet);
    }
    gapi_setIterFree (iterSet);

    _SubscriberStatusSetListener(_SubscriberStatus(_Entity(subscriber)->status), NULL, 0);
    _SubscriberStatusFree(_SubscriberStatus(_Entity(subscriber)->status));

    u_subscriberFree(U_SUBSCRIBER_GET(subscriber));

    gapi_setFree (subscriber->dataReaderSet);
    subscriber->dataReaderSet = NULL;

    _DomainEntityDispose(_DomainEntity(subscriber));
}

_DataReader
_BuiltinDataReaderNew (
    _Subscriber subscriber,
    const char *topicName)
{
    _TypeSupport typeSupport  = NULL;
    _Topic       topic        = NULL;
    _DataReader newDataReader = NULL;
    u_subscriber uSubscriber;
    u_dataReader uReader      = NULL;
    c_iter uReaders;
    gapi_char   *typeName;
    _DomainParticipant participant;

    uSubscriber = _SubscriberUsubscriber(subscriber);
    uReaders    = u_subscriberLookupReaders(uSubscriber, topicName);
    uReader     = u_dataReader(c_iterTakeFirst(uReaders));
    c_iterFree(uReaders);

    if ( uReader ) {
        participant = _DomainEntityParticipant(_DomainEntity(subscriber));

        topic = _DomainParticipantFindBuiltinTopic(participant, topicName);

        if ( topic ) {
            typeName = _TopicGetTypeName(topic);
            if ( typeName ) {
                typeSupport = _DomainParticipantFindTypeSupport(participant, typeName);
                gapi_free(typeName);
            }
        }

        if ( typeSupport && topic ) {
            newDataReader = _DataReaderAlloc();
        }
    }

    if ( newDataReader ) {
        _DataReaderInit(newDataReader,
                        subscriber,
                        _TopicDescription(topic),
                        typeSupport,
                        NULL,
                        0,
                        uReader,
                        TRUE);
    }
    return newDataReader;
}


static void
_BuiltinDataReaderFree (
    _DataReader datareader)
{
    gapi_setIter iterSet;

    assert(datareader);

    iterSet = gapi_setFirst (datareader->conditionSet);
    while (gapi_setIterObject(iterSet)) {
        _ReadConditionFree(_ReadCondition(gapi_setIterObject(iterSet)));
        gapi_setIterRemove (iterSet);
    }
    gapi_setIterFree (iterSet);
    _DataReaderFree(datareader);
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

    for ( i = 0; i < MAX_BUILTIN_TOPIC; i++ ) {
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

            if ( dst->value._maximum >= len ) {
                memcpy(dst->value._buffer, src->value, len);
            }
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
                                       "C_SEQUENCE<c_octet>",
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

            if ( dst->value._maximum >= len ) {
                memcpy(dst->value._buffer, src->value, len);
            }
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
                                           "C_SEQUENCE<c_char>",
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

            if ( dst->name._maximum >= len ) {
                unsigned long i;
                for ( i = 0; i < len; i++ ) {
                    dst->name._buffer[i] = gapi_string_dup(src->name[i]);
                }
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
                        c_metaArrayTypeNew(c_metaObject(base),
                                           "C_SEQUENCE<c_string>",
                                           type,0);
                c_free(type);
            }
        }

        if ( gapi_partitionQos_type ) {
            dst->name = c_newArray(c_collectionType(gapi_partitionQos_type),
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

            if ( dst->value._maximum >= len ) {
                memcpy(dst->value._buffer, src->value, len);
            }
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
                                           "C_SEQUENCE<c_octet>",
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

            if ( dst->value._maximum >= len ) {
                memcpy(dst->value._buffer, src->value, len);
            }
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
                                           "C_SEQUENCE<c_octet>",
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
        }

#define reliabilityQosPolicyCopyout(src,dst) \
        { \
            ((gapi_reliabilityQosPolicy *)dst)->kind = \
            ((struct v_reliabilityPolicy *)src)->kind; \
            kernelCopyOutDuration(&((struct v_reliabilityPolicy *)src)->max_blocking_time, \
                                 &((gapi_reliabilityQosPolicy *)dst)->max_blocking_time); \
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

void
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
    builtinUserDataQosPolicyCopyout(&from->user_data, &to->user_data);
    ownershipQosPolicyCopyout(&from->ownership, &to->ownership);
    ownershipStrengthQosPolicyCopyout(&from->ownership_strength, &to->ownership_strength);
    presentationQosPolicyCopyout(&from->presentation, &to->presentation);
    builtinPartitionQosPolicyCopyout(&from->partition, &to->partition);
    builtinTopicDataQosPolicyCopyout(&from->topic_data, &to->topic_data);
    builtinGroupDataQosPolicyCopyout(&from->group_data, &to->group_data);

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

void
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

