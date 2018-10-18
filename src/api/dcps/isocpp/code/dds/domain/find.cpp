/*
*                         Vortex OpenSplice
*
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
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
