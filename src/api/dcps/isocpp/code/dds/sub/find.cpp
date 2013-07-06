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


#include <dds/sub/find.hpp>

namespace dds
{
namespace sub
{

/** @todo See OSPL-1743 implementation correct? RTF Issue re how 'built in' subscriber to be constructed. */
const dds::sub::Subscriber builtin_subscriber(const dds::domain::DomainParticipant& dp)
{
    return Subscriber(dp, true);
}

}
}
