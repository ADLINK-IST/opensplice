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

#include <dds/domain/DomainParticipant.hpp>
#include <dds/domain/find.hpp>

namespace dds
{
namespace domain
{

dds::domain::DomainParticipant find(uint32_t id)
{
    DDS::DomainParticipantFactory_var dpf = DDS::DomainParticipantFactory::get_instance();
    if(dpf.in() == 0)
    {
        throw dds::core::PreconditionNotMetError(org::opensplice::core::exception_helper(
                    OSPL_CONTEXT_LITERAL(
                        "dds::core::PreconditionNotMetError: Unable to resolve the DomainParticipant Factory.")));
    }
    DDS::DomainParticipant_ptr ddsdp = dpf->lookup_participant(id);
    if(ddsdp)
    {
        dds::domain::DomainParticipant dp =
            org::opensplice::core::EntityRegistry<DDS::DomainParticipant_ptr, dds::domain::DomainParticipant>::get(ddsdp);
        if(dp != dds::core::null)
        {
            return dp;
        }
    }
    return dds::domain::DomainParticipant(dds::core::null);
}

}
}
