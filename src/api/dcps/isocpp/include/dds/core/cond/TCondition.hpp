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
#ifndef OSPL_DDS_CORE_COND_TCONDITION_HPP_
#define OSPL_DDS_CORE_COND_TCONDITION_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/core/cond/TCondition.hpp>

// Implementation
namespace dds
{
namespace core
{
namespace cond
{

template <typename DELEGATE>
TCondition<DELEGATE>::~TCondition() { }

template <typename DELEGATE>
void TCondition<DELEGATE>::dispatch()
{
    this->delegate()->dispatch();
}

template <typename DELEGATE>
bool TCondition<DELEGATE>::trigger_value() const
{
    return this->delegate()->get_trigger_value();
}

}
}
}
// End of implementation

#endif /* OSPL_DDS_CORE_COND_TCONDITION_HPP_ */
