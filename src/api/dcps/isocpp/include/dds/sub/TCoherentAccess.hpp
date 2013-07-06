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
#ifndef OSPL_DDS_SUB_TCOHERENTACCESS_HPP_
#define OSPL_DDS_SUB_TCOHERENTACCESS_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/sub/TCoherentAccess.hpp>

// Implementation
namespace dds
{
namespace sub
{
/** @bug OSPL-2476 No implementation
* @todo Implementation required - see OSPL-2476
* @see http://jira.prismtech.com:8080/browse/OSPL-2476 */

template <typename D>
TCoherentAccess<D>::~TCoherentAccess(void) {}

template <typename D>
TCoherentAccess<D>::TCoherentAccess(const dds::sub::Subscriber& sub) {}

template <typename D>
void TCoherentAccess<D>::end() {}
}
}
// End of implementation

#endif /* OSPL_DDS_SUB_TCOHERENTACCESS_HPP_ */
