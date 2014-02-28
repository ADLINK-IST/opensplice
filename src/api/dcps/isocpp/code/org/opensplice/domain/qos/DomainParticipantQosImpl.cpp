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

#include <org/opensplice/domain/qos/DomainParticipantQosImpl.hpp>

namespace org
{
namespace opensplice
{
namespace domain
{
namespace qos
{

void
DomainParticipantQosImpl::policy(const dds::core::policy::UserData& ud)
{
    user_data_ = ud;
}

void
DomainParticipantQosImpl::policy(const dds::core::policy::EntityFactory& efp)
{
    entity_factory_ = efp;
}

}
}
}
}
