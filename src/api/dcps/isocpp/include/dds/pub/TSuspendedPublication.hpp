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
#ifndef OSPL_DDS_PUB_TSUSPENDEDPUBLICATION_HPP_
#define OSPL_DDS_PUB_TSUSPENDEDPUBLICATION_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/pub/TSuspendedPublication.hpp>

// Implementation

namespace dds
{
namespace pub
{

template <typename DELEGATE>
TSuspendedPublication<DELEGATE>::TSuspendedPublication(const dds::pub::Publisher& pub) : dds::core::Value<DELEGATE>(pub) { }

template <typename DELEGATE>
void TSuspendedPublication<DELEGATE>::resume()
{
    this->delegate().end();
}

template <typename DELEGATE>
TSuspendedPublication<DELEGATE>::~TSuspendedPublication() { }

}
}

// End of implementation

#endif /* OSPL_DDS_PUB_TSUSPENDEDPUBLICATION_HPP_ */
