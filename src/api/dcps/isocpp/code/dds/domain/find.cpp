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

dds::domain::DomainParticipant find(uint32_t)
{
    /** @bug OSPL-1743 No implementation
    * @todo Implementation required - see OSPL-1743
    * @see http://jira.prismtech.com:8080/browse/OSPL-1743 */
    throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
            OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : Method not yet implemented")));
}

}
}
