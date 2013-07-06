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
#ifndef OSPL_DDS_TOPIC_TTOPICDESCRIPTION_HPP_
#define OSPL_DDS_TOPIC_TTOPICDESCRIPTION_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/topic/TTopicDescription.hpp>

// Implementation

namespace dds
{
namespace topic
{

template <typename T, template <typename Q> class DELEGATE>
TopicDescription<T, DELEGATE>::~TopicDescription() { }

template <typename T, template <typename Q> class DELEGATE>
const std::string& TopicDescription<T, DELEGATE>::name() const
{
    return this->delegate()->name();
}

template <typename T, template <typename Q> class DELEGATE>
const std::string& TopicDescription<T, DELEGATE>::type_name() const
{
    return this->delegate()->type_name();
}

template <typename T, template <typename Q> class DELEGATE>
const dds::domain::DomainParticipant& TopicDescription<T, DELEGATE>::domain_participant() const
{
    return this->delegate()->domain_participant();
}

template <typename T, template <typename Q> class DELEGATE>
TopicDescription<T, DELEGATE>::TopicDescription(const dds::domain::DomainParticipant& dp,
        const std::string& topic_name,
        const std::string& type_name)
    : ::dds::core::TEntity< DELEGATE<T> >(new DELEGATE<T>(dp, topic_name, type_name))
{ }

}
}

// End of implementation

#endif /* OSPL_DDS_TOPIC_TTOPICDESCRIPTION_HPP_ */
