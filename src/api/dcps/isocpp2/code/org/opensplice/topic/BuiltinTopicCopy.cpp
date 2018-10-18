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
 */

#include <dds/topic/BuiltinTopic.hpp>
#include <dds/topic/BuiltinTopicKey.hpp>
#include <dds/core/policy/CorePolicy.hpp>
#include "org/opensplice/topic/BuiltinTopicCopy.hpp"

#include <kernelModule.h>

namespace org
{
namespace opensplice
{
namespace topic
{


static c_array
copyInOctetSequenceToArray(
    c_type dbType,
    const dds::core::ByteSeq& from)
{
    c_type subType = c_collectionTypeSubType(c_typeActualType(dbType));
    c_array result = c_arrayNew(subType, (c_ulong) from.size());
    memcpy(result, &from.front(), from.size());

    return result;
}

static c_sequence
copyInOctetSequenceToSequence(
    c_type dbType,
    const dds::core::ByteSeq& from)
{
    c_sequence result;
    size_t metaDataSize = from.size();
    c_type subType = c_collectionTypeSubType(c_typeActualType(dbType));
    result = c_sequenceNew_s(subType, metaDataSize, metaDataSize);

    if (result) {
        memcpy(result, &from.front(), metaDataSize);
    }

    return result;
}

static c_sequence
copyInStringSeq(
    c_type dbType,
    const dds::core::StringSeq& from)
{
    unsigned long size = from.size();
    c_type subType = c_collectionTypeSubType(c_typeActualType(dbType));

    c_sequence result = c_sequenceNew(subType, size, size);

    for (unsigned int i = 0; i < size; i++) {
         result[i] = c_stringNew(c_getBase(dbType), from[i].c_str());
    }

    return result;
}

static void
copyInTopicKey(
    const dds::topic::BuiltinTopicKey& from,
    v_builtinTopicKey *to)
{
    const int32_t *v = from.delegate().value();

    to->systemId = v[0];
    to->localId = v[1];
    to->serial = v[2];
}


u_bool
__ParticipantBuiltinTopicData__copyIn(
    c_type dbType,
    const dds::topic::ParticipantBuiltinTopicData *from,
    struct v_participantInfo *to)
{
    u_bool result = OS_C_TRUE;

    copyInTopicKey(from->key(), &to->key);
    to->user_data.value = copyInOctetSequenceToArray(c_memberType(c_structureMember(dbType, 1)), from->user_data().value());

    return result;
}


u_bool
__TopicBuiltinTopicData__copyIn(
    c_type dbType,
    const dds::topic::TopicBuiltinTopicData *from,
    v_topicInfo *to)
{
    u_bool result = OS_C_TRUE;

    copyInTopicKey(from->delegate().key(), &to->key);
    to->name = c_stringNew(c_getBase(dbType), from->name().c_str());
    to->type_name = c_stringNew(c_getBase(dbType), from->type_name().c_str());
    to->durability = from->durability().delegate().v_policy();
    to->durabilityService = from->durability_service().delegate().v_policy();
    to->deadline = from->deadline().delegate().v_policy();
    to->latency_budget = from->latency_budget().delegate().v_policy();
    to->liveliness = from->liveliness().delegate().v_policy();
    to->reliability = from->reliability().delegate().v_policy();
    to->transport_priority = from->transport_priority().delegate().v_policy();
    to->lifespan = from->lifespan().delegate().v_policy();
    to->destination_order = from->destination_order().delegate().v_policy();
    to->history = from->history().delegate().v_policy();
    to->resource_limits = from->resource_limits().delegate().v_policy();
    to->ownership = from->ownership().delegate().v_policy();
    to->topic_data.value = copyInOctetSequenceToArray(c_memberType(c_structureMember(dbType, 15)), from->topic_data().value());

    return result;
}

u_bool
__PublicationBuiltinTopicData__copyIn(
    c_type dbType,
    const dds::topic::PublicationBuiltinTopicData *from,
    v_publicationInfo *to)
{
    u_bool result = OS_C_TRUE;

    copyInTopicKey(from->key(), &to->key);
    copyInTopicKey(from->participant_key(), &to->participant_key);
    to->topic_name = c_stringNew(c_getBase(dbType), from->topic_name().c_str());
    to->type_name = c_stringNew(c_getBase(dbType), from->type_name().c_str());
    to->durability = from->durability().delegate().v_policy();
    to->deadline = from->deadline().delegate().v_policy();
    to->latency_budget = from->latency_budget().delegate().v_policy();
    to->liveliness = from->liveliness().delegate().v_policy();
    to->reliability = from->reliability().delegate().v_policy();
    to->destination_order = from->destination_order().delegate().v_policy();
    to->user_data.value = copyInOctetSequenceToArray(c_memberType(c_structureMember(dbType, 10)), from->user_data().value());
    to->ownership = from->ownership().delegate().v_policy();
    to->ownership_strength = from->ownership_strength().delegate().v_policy();
    to->presentation = from->presentation().delegate().v_policy();
    to->partition.name = copyInStringSeq(c_memberType(c_structureMember(dbType, 14)), from->partition().name());
    to->topic_data.value = copyInOctetSequenceToArray(c_memberType(c_structureMember(dbType, 15)), from->topic_data().value());
    to->group_data.value = copyInOctetSequenceToArray(c_memberType(c_structureMember(dbType, 16)), from->group_data().value());

    return result;
}


u_bool
__SubscriptionBuiltinTopicData__copyIn(
    c_type dbType,
    const dds::topic::SubscriptionBuiltinTopicData *from,
    struct v_subscriptionInfo *to)
{
    u_bool result = OS_C_TRUE;

    copyInTopicKey(from->key(), &to->key);
    copyInTopicKey(from->participant_key(), &to->participant_key);
    to->topic_name = c_stringNew(c_getBase(dbType), from->topic_name().c_str());
    to->type_name = c_stringNew(c_getBase(dbType), from->type_name().c_str());
    to->durability = from->durability().delegate().v_policy();
    to->deadline = from->deadline().delegate().v_policy();
    to->latency_budget = from->latency_budget().delegate().v_policy();
    to->liveliness = from->liveliness().delegate().v_policy();
    to->reliability = from->reliability().delegate().v_policy();
    to->destination_order = from->destination_order().delegate().v_policy();
    to->user_data.value = copyInOctetSequenceToArray(c_memberType(c_structureMember(dbType, 10)), from->user_data().value());
    to->time_based_filter = from->time_based_filter().delegate().v_policy();
    to->ownership = from->ownership().delegate().v_policy();
    to->presentation = from->presentation().delegate().v_policy();
    to->partition.name = copyInStringSeq(c_memberType(c_structureMember(dbType, 14)), from->partition().name());
    to->topic_data.value = copyInOctetSequenceToArray(c_memberType(c_structureMember(dbType, 15)), from->topic_data().value());
    to->group_data.value = copyInOctetSequenceToArray(c_memberType(c_structureMember(dbType, 16)), from->group_data().value());

    return result;
}

u_bool
__CMParticipantBuiltinTopicData__copyIn(
    c_type dbType,
    const org::opensplice::topic::CMParticipantBuiltinTopicData *from,
    struct v_participantCMInfo *to)
{
    u_bool result = OS_C_TRUE;

    copyInTopicKey(from->key(), &to->key);
    to->product.value = c_stringNew(c_getBase(dbType), from->product().delegate().name().c_str());;

    return result;
}


u_bool
__CMPublisherBuiltinTopicData__copyIn(
    c_type dbType,
    const  org::opensplice::topic::CMPublisherBuiltinTopicData *from,
    struct v_publisherCMInfo  *to)
{
    u_bool result = OS_C_TRUE;

    copyInTopicKey(from->key(), &to->key);
    to->product.value = c_stringNew(c_getBase(dbType), from->product().name().c_str());;
    copyInTopicKey(from->participant_key(), &to->participant_key);
    to->name = c_stringNew(c_getBase(dbType), from->name().c_str());
    to->entity_factory = from->entity_factory().delegate().v_policy();
    to->partition.name = copyInStringSeq(c_memberType(c_structureMember(dbType, 5)), from->partition().name());

    return result;
}

u_bool
__CMSubscriberBuiltinTopicData__copyIn(
    c_type dbType,
    const org::opensplice::topic::CMSubscriberBuiltinTopicData *from,
    struct v_subscriberCMInfo *to)
{
    u_bool result = OS_C_TRUE;

    copyInTopicKey(from->key(), &to->key);
    to->product.value = c_stringNew(c_getBase(dbType), from->product().name().c_str());
    copyInTopicKey(from->participant_key(), &to->participant_key);
    to->name = c_stringNew(c_getBase(dbType), from->name().c_str());
    to->entity_factory = from->entity_factory().delegate().v_policy();
    to->share.enable = from->share().enable();
    to->share.name = c_stringNew(c_getBase(dbType), from->share().name().c_str());
    to->partition.name = copyInStringSeq(c_memberType(c_structureMember(dbType, 7)), from->partition().name());

    return result;
}

u_bool
__CMDataWriterBuiltinTopicData__copyIn(
    c_type dbType,
    const org::opensplice::topic::CMDataWriterBuiltinTopicData *from,
    struct v_dataWriterCMInfo *to)
{
    u_bool result = OS_C_TRUE;

    copyInTopicKey(from->key(), &to->key);
    to->product.value = c_stringNew(c_getBase(dbType), from->product().name().c_str());
    copyInTopicKey(from->publisher_key(), &to->publisher_key);
    to->name = c_stringNew(c_getBase(dbType), from->name().c_str());
    to->history = from->history().delegate().v_policy();
    to->resource_limits = from->resource_limits().delegate().v_policy();
    to->writer_data_lifecycle = from->writer_data_lifecycle().delegate().v_policy();

    return result;
}

u_bool
__CMDataReaderBuiltinTopicData__copyIn(
    c_type dbType,
    const org::opensplice::topic::CMDataReaderBuiltinTopicData *from,
    struct v_dataReaderCMInfo *to)
{
    u_bool result = OS_C_TRUE;

    copyInTopicKey(from->key(), &to->key);
    to->product.value = c_stringNew(c_getBase(dbType), from->product().name().c_str());
    copyInTopicKey(from->subscriber_key(), &to->subscriber_key);
    to->name = c_stringNew(c_getBase(dbType), from->name().c_str());
    to->history = from->history().delegate().v_policy();
    to->resource_limits = from->resource_limits().delegate().v_policy();
    to->reader_data_lifecycle = from->reader_data_lifecycle().delegate().v_policy();
    struct v_userKeyPolicy v = from->subscription_keys().delegate().v_policy();
    to->subscription_keys.enable = v.enable;
    to->subscription_keys.expression = c_stringNew(c_getBase(dbType), v.expression);
    os_free(v.expression);
    to->reader_lifespan = from->reader_lifespan().delegate().v_policy();
    to->share.enable = from->share().enable();
    to->share.name = c_stringNew(c_getBase(dbType), from->share().name().c_str());

    return result;
}

u_bool
__TypeBuiltinTopicData__copyIn(
    c_type dbType,
    const org::opensplice::topic::TypeBuiltinTopicData *from,
    struct v_typeInfo *to)
{
    u_bool result = OS_C_TRUE;

    to->name = c_stringNew(c_getBase(dbType), from->name().c_str());
    to->data_representation_id = from->data_representation_id();
    TypeHash typeHash = from->type_hash();
    to->type_hash.msb = typeHash.msb();
    to->type_hash.lsb = typeHash.lsb();
    to->meta_data  = copyInOctetSequenceToSequence(c_memberType(c_structureMember(dbType, 3)), from->meta_data() );
    to->extentions = copyInOctetSequenceToSequence(c_memberType(c_structureMember(dbType, 4)), from->extentions());

    if ((to->meta_data == NULL) || (to->extentions == NULL)) {
        result = OS_C_FALSE;
    }

    return result;
}

u_bool
__Bytes__copyIn(
    c_type dbType,
    const dds::core::BytesTopicType *from,
    struct _DDS_Bytes *to)
{
    u_bool result = OS_C_TRUE;

    to->value = copyInOctetSequenceToSequence(c_memberType(c_structureMember(dbType, 0)), from->data());
    if(to->value == NULL) {
        result = OS_C_FALSE;
    }

    return result;
}

u_bool
__String__copyIn(
    c_type dbType,
    const dds::core::StringTopicType *from,
    struct _DDS_String *to)
{
    u_bool result = OS_C_TRUE;

    to->value = c_stringNew_s(c_getBase(dbType), from->data().c_str());
    if(to->value == NULL) {
        result = OS_C_FALSE;
    }

    return result;
}

u_bool
__KeyedBytes__copyIn(
    c_type dbType,
    const dds::core::KeyedBytesTopicType *from,
    struct _DDS_KeyedBytes *to)
{
    u_bool result = OS_C_TRUE;

    to->key = c_stringNew_s(c_getBase(dbType), from->key().c_str());
    if(to->key == NULL) {
        result = OS_C_FALSE;
    }

    to->value = copyInOctetSequenceToSequence(c_memberType(c_structureMember(dbType, 0)), from->value());
    if(to->value == NULL) {
        result = OS_C_FALSE;
    }

    return result;
}

u_bool
__KeyedString__copyIn(
    c_type dbType,
    const dds::core::KeyedStringTopicType *from,
    struct _DDS_KeyedString *to)
{
    u_bool result = OS_C_TRUE;

    to->key = c_stringNew_s(c_getBase(dbType), from->key().c_str());
    if(to->key == NULL) {
        result = OS_C_FALSE;
    }

    to->value = c_stringNew_s(c_getBase(dbType), from->value().c_str());
    if(to->value == NULL) {
        result = OS_C_FALSE;
    }

    return result;
}

static const dds::core::ByteSeq
copyOutOctetSequenceFromSequence(
    const c_sequence from)
{
    c_ulong size;
    c_octet *data = (c_octet *)from;

    size = c_sequenceSize(from);

    dds::core::ByteSeq result;
    result.insert(result.end(), data, data + size);

    return result;
}

void
__ParticipantBuiltinTopicData__copyOut(
    const void *_from,
    void *_to)
{
    const struct v_participantInfo *from = (const struct v_participantInfo*)_from;
    dds::topic::ParticipantBuiltinTopicData *to = (dds::topic::ParticipantBuiltinTopicData *)_to;

    to->delegate().key(from->key);
    to->delegate().user_data(from->user_data);
}


void
__TopicBuiltinTopicData__copyOut(
    const void *_from,
    void *_to)
{
    const struct v_topicInfo *from = (const struct v_topicInfo *)_from;
    dds::topic::TopicBuiltinTopicData *to = (dds::topic::TopicBuiltinTopicData *)_to;

    to->delegate().key(from->key);
    to->delegate().name(from->name);
    to->delegate().type_name(from->type_name);
    to->delegate().durability(from->durability);
    to->delegate().durability_service(from->durabilityService);
    to->delegate().deadline(from->deadline);
    to->delegate().latency_budget(from->latency_budget);
    to->delegate().liveliness(from->liveliness);
    to->delegate().reliability(from->reliability);
    to->delegate().transport_priority(from->transport_priority);
    to->delegate().lifespan(from->lifespan);
    to->delegate().destination_order(from->destination_order);
    to->delegate().history(from->history);
    to->delegate().resource_limits(from->resource_limits);
    to->delegate().ownership(from->ownership);
    to->delegate().topic_data(from->topic_data);
}


void
__PublicationBuiltinTopicData__copyOut(
    const void *_from,
    void *_to)
{
    const struct v_publicationInfo *from = (const struct v_publicationInfo *)_from;
    dds::topic::PublicationBuiltinTopicData *to = (dds::topic::PublicationBuiltinTopicData *)_to;

    to->delegate().key(from->key);
    to->delegate().participant_key(from->participant_key);
    to->delegate().topic_name(from->topic_name);
    to->delegate().type_name(from->type_name);
    to->delegate().durability(from->durability);
    to->delegate().deadline(from->deadline);
    to->delegate().latency_budget(from->latency_budget);
    to->delegate().liveliness(from->liveliness);
    to->delegate().reliability(from->reliability);
    to->delegate().lifespan(from->lifespan);
    to->delegate().user_data(from->user_data);
    to->delegate().ownership(from->ownership);
#ifdef  OMG_DDS_OWNERSHIP_SUPPORT
    to->delegate().ownership_strength(from->ownership_strength);
#endif  // OMG_DDS_OWNERSHIP_SUPPORT
    to->delegate().destination_order(from->destination_order);
    to->delegate().presentation(from->presentation);
    to->delegate().partition(from->partition);
    to->delegate().topic_data(from->topic_data);
    to->delegate().group_data(from->group_data);
}

void
__SubscriptionBuiltinTopicData__copyOut(
    const void *_from,
    void *_to)
{
    const struct v_subscriptionInfo *from = (const struct v_subscriptionInfo *)_from;
    dds::topic::SubscriptionBuiltinTopicData *to = (dds::topic::SubscriptionBuiltinTopicData *)_to;

    to->delegate().key(from->key);
    to->delegate().participant_key(from->participant_key);
    to->delegate().topic_name(from->topic_name);
    to->delegate().type_name(from->type_name);
    to->delegate().durability(from->durability);
    to->delegate().deadline(from->deadline);
    to->delegate().latency_budget(from->latency_budget);
    to->delegate().liveliness(from->liveliness);
    to->delegate().reliability(from->reliability);
    to->delegate().ownership(from->ownership);
    to->delegate().user_data(from->user_data);
    to->delegate().destination_order(from->destination_order);
    to->delegate().time_based_filter(from->time_based_filter);
    to->delegate().presentation(from->presentation);
    to->delegate().partition(from->partition);
    to->delegate().topic_data(from->topic_data);
    to->delegate().group_data(from->group_data);
}

void
__CMParticipantBuiltinTopicData__copyOut(
    const void *_from,
    void *_to)
{
    const struct v_participantCMInfo *from = (const struct v_participantCMInfo *)_from;
    org::opensplice::topic::CMParticipantBuiltinTopicData *to = (org::opensplice::topic::CMParticipantBuiltinTopicData *)_to;

    to->delegate().key(from->key);
    to->delegate().product(from->product);

}

void
__CMPublisherBuiltinTopicData__copyOut(
    const void *_from,
    void *_to)
{
    const struct v_publisherCMInfo *from = (const struct v_publisherCMInfo *)_from;
    org::opensplice::topic::CMPublisherBuiltinTopicData *to = (org::opensplice::topic::CMPublisherBuiltinTopicData *)_to;

    to->delegate().key(from->key);
    to->delegate().product(from->product);
    to->delegate().participant_key(from->participant_key);
    to->delegate().entity_factory(from->entity_factory);
    to->delegate().partition(from->partition);
}

void
__CMSubscriberBuiltinTopicData__copyOut(
    const void *_from,
    void *_to)
{
    const struct v_subscriberCMInfo *from = (const struct v_subscriberCMInfo *)_from;
    org::opensplice::topic::CMSubscriberBuiltinTopicData *to = (org::opensplice::topic::CMSubscriberBuiltinTopicData *)_to;

    to->delegate().key(from->key);
    to->delegate().product(from->product);
    to->delegate().participant_key(from->participant_key);
    to->delegate().entity_factory(from->entity_factory);
    to->delegate().share(from->share);
    to->delegate().partition(from->partition);
}

void
__CMDataWriterBuiltinTopicData__copyOut(
    const void *_from,
    void *_to)
{
    const struct v_dataWriterCMInfo *from = (const struct v_dataWriterCMInfo *)_from;
    org::opensplice::topic::CMDataWriterBuiltinTopicData *to = (org::opensplice::topic::CMDataWriterBuiltinTopicData *)_to;

    to->delegate().key(from->key);
    to->delegate().product(from->product);
    to->delegate().publisher_key(from->publisher_key);
    to->delegate().name(from->name);
    to->delegate().history(from->history);
    to->delegate().resource_limits(from->resource_limits);
    to->delegate().writer_data_lifecycle(from->writer_data_lifecycle);
}

void
__CMDataReaderBuiltinTopicData__copyOut(
    const void *_from,
    void *_to)
{
    const struct v_dataReaderCMInfo *from = (const struct v_dataReaderCMInfo *)_from;
    org::opensplice::topic::CMDataReaderBuiltinTopicData *to = (org::opensplice::topic::CMDataReaderBuiltinTopicData *)_to;

    to->delegate().key(from->key);
    to->delegate().product(from->product);
    to->delegate().subscriber_key(from->subscriber_key);
    to->delegate().name(from->name);
    to->delegate().history(from->history);
    to->delegate().resource_limits(from->resource_limits);
    to->delegate().reader_data_lifecycle(from->reader_data_lifecycle);
    to->delegate().subscription_keys(from->subscription_keys);
    to->delegate().reader_data_lifecycle(from->reader_data_lifecycle);
    to->delegate().share(from->share);
}

void
__TypeBuiltinTopicData__copyOut(
    const void *_from,
    void *_to)
{
    const struct v_typeInfo *from = (const struct v_typeInfo *)_from;
    org::opensplice::topic::TypeBuiltinTopicData *to = (org::opensplice::topic::TypeBuiltinTopicData *)_to;

    to->delegate().name(from->name);
    to->delegate().data_representation_id(from->data_representation_id);
    to->delegate().type_hash(org::opensplice::topic::TypeHash(from->type_hash.msb, from->type_hash.lsb));
    to->delegate().meta_data(copyOutOctetSequenceFromSequence(from->meta_data));
    to->delegate().extentions(copyOutOctetSequenceFromSequence(from->extentions));
}

void
__Bytes__copyOut(
    const void *_from,
    void *_to)
{
    const struct _DDS_Bytes *from = (const struct _DDS_Bytes *)_from;
    dds::core::BytesTopicType *to = (dds::core::BytesTopicType *)_to;

    to->delegate().value(copyOutOctetSequenceFromSequence(from->value));
}

void
__String__copyOut(
    const void *_from,
    void *_to)
{
    const struct _DDS_String *from = (const struct _DDS_String *)_from;
    dds::core::StringTopicType *to = (dds::core::StringTopicType *)_to;

    to->delegate().value(from->value);
}

void
__KeyedBytes__copyOut(
    const void *_from,
    void *_to)
{
    const struct _DDS_KeyedBytes *from = (const struct _DDS_KeyedBytes *)_from;
    dds::core::KeyedBytesTopicType *to = (dds::core::KeyedBytesTopicType *)_to;

    to->delegate().key(from->key);
    to->delegate().value(copyOutOctetSequenceFromSequence(from->value));
}

void
__KeyedString__copyOut(
    const void *_from,
    void *_to)
{
    const struct _DDS_KeyedString *from = (const struct _DDS_KeyedString *)_from;
    dds::core::KeyedStringTopicType *to = (dds::core::KeyedStringTopicType *)_to;

    to->delegate().key(from->key);
    to->delegate().value(from->value);
}

}
}
}
