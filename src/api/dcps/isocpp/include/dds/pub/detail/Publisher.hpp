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
#ifndef OSPL_DDS_PUB_DETAIL_PUBLISHER_HPP_
#define OSPL_DDS_PUB_DETAIL_PUBLISHER_HPP_

/**
 * @file
 */

// Implementation

#include <dds/pub/TPublisher.hpp>
#include <org/opensplice/pub/PublisherDelegate.hpp>

namespace dds
{
namespace pub
{
namespace detail
{
typedef dds::pub::TPublisher<org::opensplice::pub::PublisherDelegate> Publisher;
}
}
}

// End of implementation

#endif /* OSPL_DDS_PUB_DETAIL_PUBLISHER_HPP_ */
