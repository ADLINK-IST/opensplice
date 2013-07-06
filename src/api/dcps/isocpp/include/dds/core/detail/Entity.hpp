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
#ifndef OSPL_DDS_CORE_DETAIL_ENTITY_HPP_
#define OSPL_DDS_CORE_DETAIL_ENTITY_HPP_

/**
 * @file
 */

// Implementation

#include <dds/core/TEntity.hpp>
#include <org/opensplice/core/EntityDelegate.hpp>

namespace dds
{
namespace core
{
namespace detail
{
typedef dds::core::TEntity<org::opensplice::core::EntityDelegate> Entity;
}
}
}

// End of implementation

#endif /* OSPL_DDS_CORE_DETAIL_ENTITY_HPP_ */
