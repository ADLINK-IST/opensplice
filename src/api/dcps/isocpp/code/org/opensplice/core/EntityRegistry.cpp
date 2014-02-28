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

#include <org/opensplice/core/EntityRegistry.hpp>
#include <dds/domain/DomainParticipant.hpp>
#include <dds/pub/Publisher.hpp>
#include <dds/sub/Subscriber.hpp>

namespace org
{
namespace opensplice
{
namespace core
{

/* DomainParticipant */

template <>
OSPL_ISOCPP_IMPL_API std::map<DDS::DomainParticipant_ptr, dds::core::WeakReference<dds::domain::DomainParticipant> >&
org::opensplice::core::EntityRegistry<DDS::DomainParticipant_ptr, dds::domain::DomainParticipant>::registry()
{
    static std::map<DDS::DomainParticipant_ptr, dds::core::WeakReference<dds::domain::DomainParticipant> > registry_;
    return registry_;
}

/* Publisher */

template <>
OSPL_ISOCPP_IMPL_API std::map<DDS::Publisher_ptr, dds::core::WeakReference<dds::pub::Publisher> >&
org::opensplice::core::EntityRegistry<DDS::Publisher_ptr, dds::pub::Publisher>::registry()
{
    static std::map<DDS::Publisher_ptr, dds::core::WeakReference<dds::pub::Publisher> > registry_;
    return registry_;
}

/* Subscriber */

template <>
OSPL_ISOCPP_IMPL_API std::map<DDS::Subscriber_ptr, dds::core::WeakReference<dds::sub::Subscriber> >&
org::opensplice::core::EntityRegistry<DDS::Subscriber_ptr, dds::sub::Subscriber>::registry()
{
    static std::map<DDS::Subscriber_ptr, dds::core::WeakReference<dds::sub::Subscriber> > registry_;
    return registry_;
}

}
}
}