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


/**
 * @file
 */

#ifndef OSPL_DDS_SUB_COND_QUERYCONDITION_CPP_
#define OSPL_DDS_SUB_COND_QUERYCONDITION_CPP_

/*
 * OMG PSM class declaration
 */
#include <dds/sub/cond/QueryCondition.hpp>

// Implementation

namespace dds
{
namespace sub
{
namespace cond
{

QueryCondition::QueryCondition() : dds::sub::cond::TReadCondition<DELEGATE>(dds::core::null) { }

QueryCondition::QueryCondition(const dds::core::null_type&) : dds::sub::cond::TReadCondition<DELEGATE>(dds::core::null) { }

QueryCondition::QueryCondition(const dds::sub::Query& query, const dds::sub::status::DataState& status) : TReadCondition<detail::QueryCondition>(new detail::QueryCondition(query, status)) { }

QueryCondition::~QueryCondition() { }


void QueryCondition::expression(const std::string& expr)
{
    this->delegate()->expression(expr);
}

const std::string& QueryCondition::expression()
{
    return this->delegate()->expression();
}

QueryCondition::const_iterator QueryCondition::begin() const
{
    return this->delegate()->begin();
}

QueryCondition::const_iterator QueryCondition::end() const
{
    return this->delegate()->end();
}

QueryCondition::iterator QueryCondition::begin()
{
    return this->delegate()->begin();
}

QueryCondition::iterator QueryCondition::end()
{
    return this->delegate()->end();
}

void QueryCondition::add_parameter(const std::string& param)
{
    this->delegate()->add_parameter(param);
}

uint32_t QueryCondition::parameters_length() const
{
    return this->delegate()->parameters_length();
}

}
}
}

// End of implementation

#endif /* OSPL_DDS_SUB_COND_QUERYCONDITION_CPP_ */
