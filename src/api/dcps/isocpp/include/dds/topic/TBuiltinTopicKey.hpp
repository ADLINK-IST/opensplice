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
#ifndef OSPL_DDS_TOPIC_TBUILTINTOPICKEY_HPP_
#define OSPL_DDS_TOPIC_TBUILTINTOPICKEY_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/topic/TBuiltinTopicKey.hpp>

// Implementation

namespace dds
{
namespace topic
{

template <typename D>
const int32_t* TBuiltinTopicKey<D>::value() const
{
    return this->delegate().value();
}

template <typename D>
void TBuiltinTopicKey<D>::value(int32_t v[])
{
    return this->delegate().value(v);
}

}
}

// End of implementation

#endif /* OSPL_DDS_TOPIC_TBUILTINTOPICKEY_HPP_ */
