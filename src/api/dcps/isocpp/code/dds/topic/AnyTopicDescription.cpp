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

/**
 * @file
 */

#ifndef OSPL_DDS_TOPIC_ANYTOPICDESCRIPTION_HPP_
#define OSPL_DDS_TOPIC_ANYTOPICDESCRIPTION_HPP_

/*
 * OMG PSM class declaration
 */
#include <spec/dds/topic/AnyTopicDescription.hpp>

// Implementation
namespace dds
{
namespace topic
{

const dds::domain::DomainParticipant& AnyTopicDescription::domain_participant() const
{
    return holder_->domain_participant();
}

const std::string& AnyTopicDescription::name() const
{
    return holder_->name();
}

const std::string& AnyTopicDescription::type_name() const
{
    return holder_->type_name();
}

const detail::TDHolderBase* AnyTopicDescription::operator->() const
{
    return holder_.get();
}

detail::TDHolderBase* AnyTopicDescription::operator->()
{
    return holder_.get();
}

}
}
// End of implementation

#endif /* OSPL_DDS_TOPIC_ANYTOPICDESCRIPTION_CPP_ */
