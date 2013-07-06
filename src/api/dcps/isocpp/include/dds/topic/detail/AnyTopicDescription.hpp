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
#ifndef OSPL_DDS_TOPIC_DETAIL_ANYTOPICDESCRIPTION_HPP_
#define OSPL_DDS_TOPIC_DETAIL_ANYTOPICDESCRIPTION_HPP_

/**
 * @file
 */

// Implementation

#include <string>

#include <dds/domain/DomainParticipant.hpp>
#include <dds/topic/TopicDescription.hpp>

namespace dds
{
namespace topic
{
namespace detail
{
class TDHolderBase
{
public:
    virtual ~TDHolderBase() { }

    virtual const dds::domain::DomainParticipant& domain_participant() const = 0;

    virtual const std::string& name() const = 0;

    virtual const std::string& type_name() const = 0;
};

template <typename T>
class TDHolder : public virtual TDHolderBase
{
public:
    TDHolder(const dds::topic::TopicDescription<T>& t) : td_(t) { }
    virtual ~TDHolder() { }
public:
    virtual const dds::domain::DomainParticipant& domain_participant() const
    {
        return td_.domain_participant();
    }

    virtual const std::string& name() const
    {
        return td_.name();
    }

    virtual const std::string& type_name() const
    {
        return td_.type_name();
    }

    const dds::topic::TopicDescription<T>& get() const
    {
        return td_;
    }

protected:
    dds::topic::TopicDescription<T> td_;
};
}
}
}

// End of implementation

#endif /* OSPL_DDS_TOPIC_DETAIL_ANYTOPICDESCRIPTION_HPP_ */
