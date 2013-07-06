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
#ifndef OSPL_DDS_SUB_TGENERATIONCOUNT_HPP_
#define OSPL_DDS_SUB_TGENERATIONCOUNT_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/sub/TGenerationCount.hpp>

// Implementation

namespace dds
{
namespace sub
{

template <typename DELEGATE>
TGenerationCount<DELEGATE>::TGenerationCount() { }

template <typename DELEGATE>
TGenerationCount<DELEGATE>::TGenerationCount(int32_t dgc, int32_t nwgc)
    : dds::core::Value<DELEGATE>(dgc, nwgc) { }

template <typename DELEGATE>
int32_t TGenerationCount<DELEGATE>::disposed() const
{
    return this->delegate().disposed();
}

template <typename DELEGATE>
inline int32_t TGenerationCount<DELEGATE>::no_writers() const
{
    return this->delegate().no_writers();
}

}
}

// End of implementation

#endif /* OSPL_DDS_SUB_TGENERATIONCOUNT_HPP_ */
