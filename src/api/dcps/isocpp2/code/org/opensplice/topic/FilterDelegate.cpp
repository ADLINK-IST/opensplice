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

#include <org/opensplice/topic/FilterDelegate.hpp>

org::opensplice::topic::FilterDelegate::FilterDelegate() { }
org::opensplice::topic::FilterDelegate::FilterDelegate(const std::string& query_expression)
    : myExpression(query_expression) { }

const std::string&
org::opensplice::topic::FilterDelegate::expression() const
{
    return myExpression;
}

org::opensplice::topic::FilterDelegate::const_iterator
org::opensplice::topic::FilterDelegate::begin() const
{
    return myParams.begin();
}

org::opensplice::topic::FilterDelegate::const_iterator
org::opensplice::topic::FilterDelegate::end() const
{
    return myParams.end();
}

org::opensplice::topic::FilterDelegate::iterator
org::opensplice::topic::FilterDelegate::begin()
{
    return myParams.begin();
}

org::opensplice::topic::FilterDelegate::iterator
org::opensplice::topic::FilterDelegate::end()
{
    return myParams.end();
}

void
org::opensplice::topic::FilterDelegate::add_parameter(const std::string& param)
{
    myParams.push_back(param);
}

uint32_t
org::opensplice::topic::FilterDelegate::parameters_length() const
{
    return static_cast<uint32_t>(myParams.size());
}

bool
org::opensplice::topic::FilterDelegate::operator ==(const FilterDelegate& other) const
{
    return other.myExpression == myExpression && other.myParams == myParams;
}

// End of implementation
