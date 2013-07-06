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
#ifndef OSPL_DDS_SUB_DETAIL_RANK_HPP_
#define OSPL_DDS_SUB_DETAIL_RANK_HPP_

/**
 * @file
 */

// Implementation

#include <dds/sub/TRank.hpp>
#include <org/opensplice/sub/RankImpl.hpp>

namespace dds
{
namespace sub
{
namespace detail
{
typedef dds::sub::TRank< org::opensplice::sub::RankImpl > Rank;
}
}
}

// End of implementation

#endif /* OSPL_DDS_SUB_DETAIL_RANK_HPP_ */
