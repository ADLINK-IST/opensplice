/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2012 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef OSPL_DDS_PUB_DISCOVERY_HPP_
#define OSPL_DDS_PUB_DISCOVERY_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/pub/discovery.hpp>
#include <org/opensplice/core/policy/PolicyConverter.hpp>
#include <org/opensplice/core/exception_helper.hpp>

using namespace org::opensplice::core::policy;

// Implementation

namespace dds
{
namespace pub
{

template <typename FwdIterator>
void ignore(const dds::domain::DomainParticipant& dp, FwdIterator begin, FwdIterator end)
{
    for(FwdIterator i = begin; i < end; i++)
    {
        DDS::ReturnCode_t result = ((dds::domain::DomainParticipant)dp)->dp_->ignore_subscription(i->handle());
        org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::ignore_subscription"));
    }
}

template <typename T>
::dds::core::InstanceHandleSeq
matched_subscriptions(const dds::pub::DataWriter<T>& dw)
{
    dds::core::InstanceHandleSeq isocppSeq;
    DDS::InstanceHandleSeq ddsSeq;
    DDS::ReturnCode_t result = ((dds::pub::DataWriter<T>)dw)->get_raw_writer()->get_matched_subscriptions(ddsSeq);
    org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::get_matched_subscriptions"));
    for(uint32_t i = 0; i < ddsSeq.length(); i++)
    {
        isocppSeq.push_back(ddsSeq[i]);
    }
    return isocppSeq;
}

template <typename T, typename FwdIterator>
uint32_t
matched_subscriptions(const dds::pub::DataWriter<T>& dw,
                      FwdIterator begin, uint32_t max_size)
{
    DDS::InstanceHandleSeq ddsSeq;
    DDS::ReturnCode_t result = ((dds::pub::DataWriter<T>)dw)->get_raw_writer()->get_matched_subscriptions(ddsSeq);
    org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::get_matched_subscriptions"));

    ddsSeq.length() < max_size ? max_size = ddsSeq.length() : max_size = max_size;

    for(uint32_t i = 0; i < max_size; i++)
    {
        *begin = ddsSeq[i];
        begin++;
    }

    return max_size;
}

template <typename T>
const dds::topic::SubscriptionBuiltinTopicData
matched_subscription_data(const dds::pub::DataWriter<T>& dw,
                          const ::dds::core::InstanceHandle& h)
{
    dds::topic::SubscriptionBuiltinTopicData isocppData;
    DDS::SubscriptionBuiltinTopicData ddsData;

    DDS::ReturnCode_t result = ((dds::pub::DataWriter<T>)dw)->get_raw_writer()->get_matched_subscription_data(ddsData, h->handle());
    org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::get_matched_subscription_data"));

    int32_t key[] = {ddsData.key[0], ddsData.key[1], ddsData.key[2]};
    isocppData->key_.value(key);
    int32_t participant_key[] = {ddsData.participant_key[0], ddsData.participant_key[1], ddsData.participant_key[2]};
    isocppData->participant_key_.value(participant_key);
    isocppData->topic_name_ = ddsData.topic_name;
    isocppData->type_name_ = ddsData.type_name;
    isocppData->durability_ = convertPolicy(ddsData.durability);
    isocppData->deadline_ = convertPolicy(ddsData.deadline);
    isocppData->latency_budget_ = convertPolicy(ddsData.latency_budget);
    isocppData->liveliness_ = convertPolicy(ddsData.liveliness);
    isocppData->reliability_ = convertPolicy(ddsData.reliability);
    isocppData->ownership_ = convertPolicy(ddsData.ownership);
    isocppData->destination_order_ = convertPolicy(ddsData.destination_order);
    isocppData->user_data_ = convertPolicy(ddsData.user_data);
    isocppData->time_based_filter_ = convertPolicy(ddsData.time_based_filter);
    isocppData->presentation_ = convertPolicy(ddsData.presentation);
    isocppData->partition_ = convertPolicy(ddsData.partition);
    isocppData->topic_data_ = convertPolicy(ddsData.topic_data);
    isocppData->group_data_ = convertPolicy(ddsData.group_data);

    return isocppData;
}

}
}

// End of implementation

#endif /* OSPL_DDS_PUB_DISCOVERY_HPP_ */
