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
#ifndef OSPL_DDS_SUB_TRANK_HPP_
#define OSPL_DDS_SUB_TRANK_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/sub/TRank.hpp>

// Implementation
namespace dds
{
namespace sub
{

template <typename DELEGATE>
TRank<DELEGATE>::TRank() { }

template <typename DELEGATE>
TRank<DELEGATE>::TRank(int32_t s, int32_t a, int32_t ag)
    : dds::core::Value<DELEGATE>(s, a, ag) { }

template <typename DELEGATE>
int32_t TRank<DELEGATE>::absolute_generation() const
{
    return this->delegate().absolute_generation();
}

template <typename DELEGATE>
inline int32_t TRank<DELEGATE>::generation() const
{
    return this->delegate().generation();
}

template <typename DELEGATE>
inline int32_t TRank<DELEGATE>::sample() const
{
    return this->delegate().sample();
}

}
}
// End of implementation

#endif /* OSPL_DDS_SUB_TRANK_HPP_ */
