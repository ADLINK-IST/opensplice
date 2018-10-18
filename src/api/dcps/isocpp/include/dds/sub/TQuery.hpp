/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#ifndef OSPL_DDS_SUB_TQUERY_HPP_
#define OSPL_DDS_SUB_TQUERY_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/sub/TQuery.hpp>

namespace dds
{
namespace sub
{

template<typename DELEGATE>
template<typename T>
TQuery<DELEGATE>::TQuery(const dds::sub::DataReader<T>& dr, const std::string& query_expression) :
    dds::core::Reference<DELEGATE>(new DELEGATE(dr, query_expression))
{
}

template<typename DELEGATE>
template<typename T, typename FWIterator>
TQuery<DELEGATE>::TQuery(const dds::sub::DataReader<T>& dr, const std::string& query_expression,
                         const FWIterator& params_begin, const FWIterator& params_end) :
    dds::core::Reference<DELEGATE>(new DELEGATE(dr, query_expression, params_begin,
                                   params_end))
{
}

template<typename DELEGATE>
template<typename T>
TQuery<DELEGATE>::TQuery(const dds::sub::DataReader<T>& dr, const std::string& query_expression,
                         const std::vector<std::string>& params) :
    dds::core::Reference<DELEGATE>(new DELEGATE(dr, query_expression, params.begin(),
                                   params.end()))
{
}

template<typename DELEGATE>
template<typename FWIterator>
void TQuery<DELEGATE>::parameters(const FWIterator& begin, const FWIterator end)
{
    this->delegate()->parameters(begin, end);
}

template<typename DELEGATE>
const std::string& TQuery<DELEGATE>::expression() const
{
    return this->delegate()->expression();
}

template<typename DELEGATE>
void TQuery<DELEGATE>::expression(const std::string& expr)
{
    return this->delegate()->expression(expr);
}

template<typename DELEGATE>
typename TQuery<DELEGATE>::const_iterator TQuery<DELEGATE>::begin() const
{
    return this->delegate()->begin();
}

template<typename DELEGATE>
typename TQuery<DELEGATE>::const_iterator TQuery<DELEGATE>::end() const
{
    return this->delegate()->end();
}

template<typename DELEGATE>
typename TQuery<DELEGATE>::iterator TQuery<DELEGATE>::begin()
{
    return this->delegate()->begin();
}

template<typename DELEGATE>
typename TQuery<DELEGATE>::iterator TQuery<DELEGATE>::end()
{
    return this->delegate()->end();
}

template<typename DELEGATE>
void TQuery<DELEGATE>::add_parameter(const std::string& param)
{
    this->delegate()->add_parameter(param);
}

template<typename DELEGATE>
uint32_t TQuery<DELEGATE>::parameters_length() const
{
    return this->delegate()->parameters_length();
}

template<typename DELEGATE>
const AnyDataReader& TQuery<DELEGATE>::data_reader() const
{
    return this->delegate()->data_reader();
}

}
}

#endif
