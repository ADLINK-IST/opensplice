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
#ifndef OSPL_DDS_TOPIC_TFILTER_HPP_
#define OSPL_DDS_TOPIC_TFILTER_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/topic/TFilter.hpp>

// Implementation

namespace dds
{
namespace topic
{

template<typename D>
TFilter<D>::TFilter(const std::string& query_expression) :
    dds::core::Value<D>(query_expression)
{
}

template<typename D>
template<typename FWIterator>
TFilter<D>::TFilter(const std::string& query_expression, const FWIterator& params_begin,
                    const FWIterator& params_end)
    : dds::core::Value<D>(query_expression, params_begin, params_end)
{ }

template<typename D>
TFilter<D>::TFilter(const std::string& query_expression,
                    const std::vector<std::string>& params) :
    dds::core::Value<D>(query_expression, params.begin(), params.end())
{
}

template<typename D>
const std::string& TFilter<D>::expression() const
{
    return this->delegate().expression();
}

template<typename D>
typename TFilter<D>::const_iterator TFilter<D>::begin() const
{
    return this->delegate().begin();
}

template<typename D>
typename TFilter<D>::const_iterator TFilter<D>::end() const
{
    return this->delegate().end();
}

template<typename D>
typename TFilter<D>::iterator TFilter<D>::begin()
{
    return this->delegate().begin();
}

template<typename D>
typename TFilter<D>::iterator TFilter<D>::end()
{
    return this->delegate().end();
}

template<typename D>
template<typename FWIterator>
void TFilter<D>::parameters(const FWIterator& begin, const FWIterator end)
{
    this->delegate().parameters(begin, end);
}

template<typename D>
void TFilter<D>::add_parameter(const std::string& param)
{
    this->delegate().add_parameter(param);
}

template<typename D>
uint32_t TFilter<D>::parameters_length() const
{
    return this->delegate().parameters_length();
}

}
}

// End of implementation

#endif /* OSPL_DDS_TOPIC_TFILTER_HPP_ */
