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
#ifndef OSPL_DDS_SUB_TSAMPLEINFO_HPP_
#define OSPL_DDS_SUB_TSAMPLEINFO_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/sub/TSampleInfo.hpp>

// Implementation
namespace dds
{
namespace sub
{

template <typename DELEGATE>
TSampleInfo<DELEGATE>::TSampleInfo() { }

template <typename DELEGATE>
const dds::core::Time TSampleInfo<DELEGATE>::timestamp() const
{
    return this->delegate().timestamp();
}

template <typename DELEGATE>
const dds::sub::status::DataState TSampleInfo<DELEGATE>::state() const
{
    return this->delegate().state();
}

template <typename DELEGATE>
dds::sub::GenerationCount TSampleInfo<DELEGATE>::generation_count() const
{
    return this->delegate().generation_count();
}

template <typename DELEGATE>
dds::sub::Rank TSampleInfo<DELEGATE>::rank() const
{
    return this->delegate().rank();
}

template <typename DELEGATE>
bool TSampleInfo<DELEGATE>::valid() const
{
    return this->delegate().valid();
}

template <typename DELEGATE>
dds::core::InstanceHandle TSampleInfo<DELEGATE>::instance_handle() const
{
    return this->delegate().instance_handle();
}

template <typename DELEGATE>
dds::core::InstanceHandle TSampleInfo<DELEGATE>::publication_handle() const
{
    return this->delegate().publication_handle();
}
}
}
// End of implementation

#endif /* OSPL_DDS_SUB_TSAMPLEINFO_HPP_ */
