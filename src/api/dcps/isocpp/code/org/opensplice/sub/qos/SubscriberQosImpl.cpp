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

#include <org/opensplice/sub/qos/SubscriberQosImpl.hpp>

namespace org
{
namespace opensplice
{
namespace sub
{
namespace qos
{

SubscriberQosImpl::SubscriberQosImpl() {  }

SubscriberQosImpl::SubscriberQosImpl(
    dds::core::policy::Presentation presentation,
    dds::core::policy::Partition partition,
    dds::core::policy::GroupData group_data,
    dds::core::policy::EntityFactory entity_factory) :
    presentation_(presentation),
    partition_(partition),
    group_data_(group_data),
    entity_factory_(entity_factory) {  }

org::opensplice::sub::qos::SubscriberQosImpl::~SubscriberQosImpl() { }

void SubscriberQosImpl::policy(const dds::core::policy::Presentation& presentation)
{
    presentation_ = presentation;
}

void SubscriberQosImpl::policy(const dds::core::policy::Partition& partition)
{
    partition_ = partition;
}

void SubscriberQosImpl::policy(const dds::core::policy::GroupData& group_data)
{
    group_data_ = group_data;
}

void SubscriberQosImpl::policy(const dds::core::policy::EntityFactory& entity_factory)
{
    entity_factory_ = entity_factory;
}

}
}
}
}
