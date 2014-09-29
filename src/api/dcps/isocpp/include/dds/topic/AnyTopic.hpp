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
#ifndef OSPL_DDS_TOPIC_ANYTOPIC_HPP_
#define OSPL_DDS_TOPIC_ANYTOPIC_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/topic/AnyTopic.hpp>
#include <org/opensplice/core/Retain.hpp>

// Implementation
namespace dds
{
namespace topic
{

template <typename T>
AnyTopic::AnyTopic(const dds::topic::Topic<T>& t): holder_(new detail::THolder<T>(t)) { }

template <typename T>
Topic<T> AnyTopic::get()
{
    OMG_DDS_STATIC_ASSERT(::dds::topic::is_topic_type<T>::value == 1);
    detail::THolder<T>* h = dynamic_cast<detail::THolder<T>* >(holder_.get());
    if(h == 0)
    {
        throw dds::core::InvalidDowncastError("invalid type");
    }
    return h->get();
}


inline const detail::THolderBase* AnyTopic::operator->() const
{
    return holder_.get();
}

}
}
template <typename T, template <typename Q> class DELEGATE>
void
dds::topic::Topic<T, DELEGATE>::close()
{
    this->delegate()->close();
    dds::topic::AnyTopic at(*this);
    org::opensplice::core::retain_remove<dds::topic::AnyTopic>(at);
}

template <typename T, template <typename Q> class DELEGATE>
void
dds::topic::Topic<T, DELEGATE>::retain()
{
    this->delegate()->retain();
    dds::topic::AnyTopic at(*this);
    org::opensplice::core::retain_add<dds::topic::AnyTopic>(at);
}
// End of implementation

#endif /* OSPL_DDS_TOPIC_ANYTOPIC_HPP_ */
