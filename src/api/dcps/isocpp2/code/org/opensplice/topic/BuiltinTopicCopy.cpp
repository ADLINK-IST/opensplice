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
copyInOctetSequence(
    c_base base,
    const dds::core::ByteSeq& from)
{
    c_type type0 = c_type(c_metaResolve(c_metaObject(base), "c_octet"));
    c_array result = c_arrayNew(type0, (c_ulong) from.size());
    c_free(type0);

    memcpy(result, &from.front(), from.size());

    return result;
}

static c_sequence
copyInStringSeq(
    c_base base,
    const dds::core::StringSeq& from)
{
    unsigned long size = from.size();

    c_type type0 = c_type(c_metaResolve(c_metaObject(base), "c_string"));
    c_sequence result = c_sequenceNew(type0, size, size);
    c_free(type0);

    for (unsigned int i = 0; i < size; i++) {
         result[i] = c_stringNew(base, from[i].c_str());
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
    c_base base,
    const dds::topic::ParticipantBuiltinTopicData *from,
    struct v_participantInfo *to)
{
    u_bool result = OS_C_TRUE;

    copyInTopicKey(from->key(), &to->key);
    to->user_data.value = copyInOctetSequence(base, from->user_data().value());

    return result;
}


u_bool
__TopicBuiltinTopicData__copyIn(
    c_base base,
    const dds::topic::TopicBuiltinTopicData *from,
    v_topicInfo *to)
{
    u_bool result = OS_C_TRUE;

    copyInTopicKey(from->delegate().key(), &to->key);
    to->name = c_stringNew(base, from->name().c_str());
    to->type_name = c_stringNew(base, from->type_name().c_str());
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
    to->topic_data.value = copyInOctetSequence(base, from->topic_data().value());

    return result;
}

u_bool
__PublicationBuiltinTopicData__copyIn(
    c_base base,
    const dds::topic::PublicationBuiltinTopicData *from,
    v_publicationInfo *to)
{
    u_bool result = OS_C_TRUE;

    copyInTopicKey(from->key(), &to->key);
    copyInTopicKey(from->participant_key(), &to->participant_key);
    to->topic_name = c_stringNew(base, from->topic_name().c_str());
    to->type_name = c_stringNew(base, from->type_name().c_str());
    to->durability = from->durability().delegate().v_policy();
    to->deadline = from->deadline().delegate().v_policy();
    to->latency_budget = from->latency_budget().delegate().v_policy();
    to->liveliness = from->liveliness().delegate().v_policy();
    to->reliability = from->reliability().delegate().v_policy();
    to->destination_order = from->destination_order().delegate().v_policy();
    to->user_data.value = copyInOctetSequence(base, from->user_data().value());
    to->ownership = from->ownership().delegate().v_policy();
    to->ownership_strength = from->ownership_strength().delegate().v_policy();
    to->presentation = from->presentation().delegate().v_policy();
    to->partition.name = copyInStringSeq(base, from->partition().name());
    to->topic_data.value = copyInOctetSequence(base, from->topic_data().value());
    to->group_data.value = copyInOctetSequence(base, from->group_data().value());

    return result;
}


u_bool
__SubscriptionBuiltinTopicData__copyIn(
    c_base base,
    const dds::topic::SubscriptionBuiltinTopicData *from,
    struct v_subscriptionInfo *to)
{
    u_bool result = OS_C_TRUE;

    copyInTopicKey(from->key(), &to->key);
    copyInTopicKey(from->participant_key(), &to->participant_key);
    to->topic_name = c_stringNew(base, from->topic_name().c_str());
    to->type_name = c_stringNew(base, from->type_name().c_str());
    to->durability = from->durability().delegate().v_policy();
    to->deadline = from->deadline().delegate().v_policy();
    to->latency_budget = from->latency_budget().delegate().v_policy();
    to->liveliness = from->liveliness().delegate().v_policy();
    to->reliability = from->reliability().delegate().v_policy();
    to->destination_order = from->destination_order().delegate().v_policy();
    to->user_data.value = copyInOctetSequence(base, from->user_data().value());
    to->time_based_filter = from->time_based_filter().delegate().v_policy();
    to->ownership = from->ownership().delegate().v_policy();
    to->presentation = from->presentation().delegate().v_policy();
    to->partition.name = copyInStringSeq(base, from->partition().name());
    to->topic_data.value = copyInOctetSequence(base, from->topic_data().value());
    to->group_data.value = copyInOctetSequence(base, from->group_data().value());

    return result;
}

u_bool
__CMParticipantBuiltinTopicData__copyIn(
    c_base base,
    const org::opensplice::topic::CMParticipantBuiltinTopicData *from,
    struct v_participantCMInfo *to)
{
    u_bool result = OS_C_TRUE;

    copyInTopicKey(from->key(), &to->key);
    to->product.value = c_stringNew(base, from->product().delegate().name().c_str());;

    return result;
}


u_bool
__CMPublisherBuiltinTopicData__copyIn(
    c_base base,
    const  org::opensplice::topic::CMPublisherBuiltinTopicData *from,
    struct v_publisherCMInfo  *to)
{
    u_bool result = OS_C_TRUE;

    copyInTopicKey(from->key(), &to->key);
    to->product.value = c_stringNew(base, from->product().name().c_str());;
    copyInTopicKey(from->participant_key(), &to->participant_key);
    to->name = c_stringNew(base, from->name().c_str());
    to->entity_factory = from->entity_factory().delegate().v_policy();
    to->partition.name = copyInStringSeq(base, from->partition().name());

    return result;
}

u_bool
__CMSubscriberBuiltinTopicData__copyIn(
    c_base base,
    const org::opensplice::topic::CMSubscriberBuiltinTopicData *from,
    struct v_subscriberCMInfo *to)
{
    u_bool result = OS_C_TRUE;

    copyInTopicKey(from->key(), &to->key);
    to->product.value = c_stringNew(base, from->product().name().c_str());
    copyInTopicKey(from->participant_key(), &to->participant_key);
    to->name = c_stringNew(base, from->name().c_str());
    to->entity_factory = from->entity_factory().delegate().v_policy();
    to->share.enable = from->share().enable();
    to->share.name = c_stringNew(base, from->share().name().c_str());
    to->partition.name = copyInStringSeq(base, from->partition().name());

    return result;
}

u_bool
__CMDataWriterBuiltinTopicData__copyIn(
    c_base base,
    const org::opensplice::topic::CMDataWriterBuiltinTopicData *from,
    struct v_dataWriterCMInfo *to)
{
    u_bool result = OS_C_TRUE;

    copyInTopicKey(from->key(), &to->key);
    to->product.value = c_stringNew(base, from->product().name().c_str());
    copyInTopicKey(from->publisher_key(), &to->publisher_key);
    to->name = c_stringNew(base, from->name().c_str());
    to->history = from->history().delegate().v_policy();
    to->resource_limits = from->resource_limits().delegate().v_policy();
    to->writer_data_lifecycle = from->writer_data_lifecycle().delegate().v_policy();

    return result;
}

u_bool
__CMDataReaderBuiltinTopicData__copyIn(
    c_base base,
    const org::opensplice::topic::CMDataReaderBuiltinTopicData *from,
    struct v_dataReaderCMInfo *to)
{
    u_bool result = OS_C_TRUE;

    copyInTopicKey(from->key(), &to->key);
    to->product.value = c_stringNew(base, from->product().name().c_str());
    copyInTopicKey(from->subscriber_key(), &to->subscriber_key);
    to->name = c_stringNew(base, from->name().c_str());
    to->history = from->history().delegate().v_policy();
    to->resource_limits = from->resource_limits().delegate().v_policy();
    to->reader_data_lifecycle = from->reader_data_lifecycle().delegate().v_policy();
    struct v_userKeyPolicy v = from->subscription_keys().delegate().v_policy();
    to->subscription_keys.enable = v.enable;
    to->subscription_keys.expression = c_stringNew(base, v.expression);
    os_free(v.expression);
    to->reader_lifespan = from->reader_lifespan().delegate().v_policy();
    to->share.enable = from->share().enable();
    to->share.name = c_stringNew(base, from->share().name().c_str());

    return result;
}

u_bool
__TypeBuiltinTopicData__copyIn(
    c_base base,
    const org::opensplice::topic::TypeBuiltinTopicData *from,
    struct v_typeInfo *to)
{
    u_bool result = OS_C_TRUE;

    to->name = c_stringNew(base, from->name().c_str());
    to->data_representation_id = from->data_representation_id();
    TypeHash typeHash = from->type_hash();
    to->type_hash.msb = typeHash.msb();
    to->type_hash.lsb = typeHash.lsb();
    size_t metaDataSize = from->meta_data().size();
    to->meta_data = c_sequenceNew_s(c_octet_t(base), metaDataSize, metaDataSize);
    memcpy(to->meta_data, &from->meta_data()[0], metaDataSize);
    size_t extentionsSize = from->extentions().size();
    to->extentions = c_sequenceNew_s(c_octet_t(base), extentionsSize, extentionsSize);
    memcpy(to->extentions, &from->extentions()[0], extentionsSize);

    return result;
}










static const dds::core::ByteSeq
copyOutOctetSequence(
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
    to->delegate().meta_data(copyOutOctetSequence(from->meta_data));
    to->delegate().extentions(copyOutOctetSequence(from->extentions));
}

}
}
}
