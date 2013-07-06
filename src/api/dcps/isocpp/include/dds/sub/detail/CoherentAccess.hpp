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
#ifndef OSPL_DDS_SUB_DETAIL_COHERENTACCESS_HPP_
#define OSPL_DDS_SUB_DETAIL_COHERENTACCESS_HPP_

/**
 * @file
 */

// Implementation

#include <org/opensplice/sub/CoherentAccessImpl.hpp>
#include <dds/sub/TCoherentAccess.hpp>

namespace dds
{
namespace sub
{
namespace detail
{
typedef ::dds::sub::TCoherentAccess<org::opensplice::sub::CoherentAccessImpl> CoherentAccess;
}
}
}

// End of implementation

#endif /* OSPL_DDS_SUB_DETAIL_COHERENTACCESS_HPP_ */
