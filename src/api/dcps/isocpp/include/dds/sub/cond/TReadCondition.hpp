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
#ifndef OSPL_DDS_SUB_COND_TREADCONDITION_HPP_
#define OSPL_DDS_SUB_COND_TREADCONDITION_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/sub/cond/TReadCondition.hpp>

// Implementation

namespace dds
{
namespace sub
{
namespace cond
{

template <typename DELEGATE>
template <typename T>
TReadCondition<DELEGATE>::TReadCondition(const dds::sub::DataReader<T>& dr, const dds::sub::status::DataState& status)
    : dds::core::cond::TCondition<DELEGATE>(new DELEGATE(dr, status)) { }

template <typename DELEGATE>
template <typename T, typename FUN>
TReadCondition<DELEGATE>::TReadCondition(const dds::sub::DataReader<T>& dr,
        const dds::sub::status::DataState& status,
        const FUN& functor)
    : dds::core::cond::TCondition<DELEGATE>(new DELEGATE(dr, status, functor)) { }

template <typename DELEGATE>
template <typename FUN>
TReadCondition<DELEGATE>::TReadCondition(const dds::sub::AnyDataReader& adr,
        const dds::sub::status::DataState& status,
        const FUN& functor)
    : dds::core::cond::TCondition<DELEGATE>(new DELEGATE(adr, status, functor)) { }

template <typename DELEGATE>
TReadCondition<DELEGATE>::~TReadCondition() { }

template <typename DELEGATE>
const dds::sub::status::DataState TReadCondition<DELEGATE>::state_filter() const
{
    return this->delegate()->state_filter();
}

template <typename DELEGATE>
const AnyDataReader& TReadCondition<DELEGATE>::data_reader() const
{
    return this->delegate()->data_reader();
}

}
}
}
// End of implementation

#endif /* OSPL_DDS_SUB_COND_TREADCONDITION_HPP_ */
