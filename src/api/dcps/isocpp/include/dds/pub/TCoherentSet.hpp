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
#ifndef OSPL_DDS_PUB_TCOHERENTSET_HPP_
#define OSPL_DDS_PUB_TCOHERENTSET_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/pub/TCoherentSet.hpp>

// Implementation

namespace dds
{
namespace pub
{

template <typename DELEGATE>
TCoherentSet<DELEGATE>::TCoherentSet(const dds::pub::Publisher& pub) : dds::core::Value<DELEGATE>(pub)  {  }

template <typename DELEGATE>
void TCoherentSet<DELEGATE>::end()
{
    this->delegate().end();
}

template <typename DELEGATE>
TCoherentSet<DELEGATE>::~TCoherentSet()
{
    //this->delegate().end();
}

}
}

// End of implementation

#endif /* OSPL_DDS_PUB_TCOHERENTSET_HPP_ */
