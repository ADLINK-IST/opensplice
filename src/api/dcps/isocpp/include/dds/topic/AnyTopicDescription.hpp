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
#ifndef OSPL_DDS_TOPIC_ANYTOPICDESCRIPTION_HPP_
#define OSPL_DDS_TOPIC_ANYTOPICDESCRIPTION_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/topic/AnyTopicDescription.hpp>

// Implementation
namespace dds
{
namespace topic
{

template <typename T>
inline AnyTopicDescription::AnyTopicDescription(const dds::topic::TopicDescription<T>& t)
    : holder_(new detail::TDHolder<T>(t)) { }


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

inline AnyTopicDescription::AnyTopicDescription(detail::TDHolderBase* holder)
    : holder_(holder) { }

inline typename AnyTopicDescription::AnyTopicDescription& AnyTopicDescription::swap(AnyTopicDescription& rhs)
{
    holder_.swap(rhs.holder_);
    return *this;
}

template <typename T>
AnyTopicDescription& AnyTopicDescription::operator =(const dds::topic::Topic<T>& rhs)
{
    holder_.reset(new detail::TDHolder<T>(rhs));
    return *this;
}

inline AnyTopicDescription& AnyTopicDescription::operator =(AnyTopicDescription rhs)
{
    return this->swap(rhs);
}

template <typename T>
const dds::topic::TopicDescription<T>& AnyTopicDescription::get()
{
    OMG_DDS_STATIC_ASSERT(::dds::topic::is_topic_type<T>::value == 1);
    detail::TDHolder<T>* h = dynamic_cast<detail::TDHolder<T>* >(holder_.get());
    if(h == 0)
    {
        throw dds::core::InvalidDowncastError("invalid type");
    }
    return h->get();
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

#endif /* OSPL_DDS_TOPIC_ANYTOPICDESCRIPTION_HPP_ */
