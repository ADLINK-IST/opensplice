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

#include <dds/domain/DomainParticipantListener.hpp>
#include <org/opensplice/domain/DomainEventForwarder.hpp>

namespace org
{
namespace opensplice
{
namespace domain
{

template<>
DomainParticipantEventForwarder<dds::domain::DomainParticipant>::DomainParticipantEventForwarder(
    const dds::domain::DomainParticipant& domain,
    dds::domain::DomainParticipantListener* listener) :
    domain_(domain),
    listener_(listener)

{
}

template<>
DomainParticipantEventForwarder<dds::domain::DomainParticipant>::~DomainParticipantEventForwarder()
{
}

template<>
dds::domain::DomainParticipantListener*
DomainParticipantEventForwarder<dds::domain::DomainParticipant>::listener()
{
    return listener_;
}

}
}
}
