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
