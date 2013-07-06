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
#ifndef OSPL_DDS_PUB_DETAIL_SUSPENDEDPUBLICATION_HPP_
#define OSPL_DDS_PUB_DETAIL_SUSPENDEDPUBLICATION_HPP_

/**
 * @file
 */

// Implementation

#include <dds/pub/TSuspendedPublication.hpp>
#include <org/opensplice/pub/SuspendedPublicationImpl.hpp>

namespace dds
{
namespace pub
{
namespace detail
{
typedef dds::pub::TSuspendedPublication<org::opensplice::pub::SuspendedPublicationImpl> SuspendedPublication;
}
}
}

// End of implementation

#endif /* OSPL_DDS_PUB_DETAIL_SUSPENDEDPUBLICATION_HPP_ */
