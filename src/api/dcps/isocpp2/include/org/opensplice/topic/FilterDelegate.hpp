/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
#ifndef OSPL_DDS_TOPIC_DETAIL_FILTER_HPP_
#define OSPL_DDS_TOPIC_DETAIL_FILTER_HPP_

/**
 * @file
 */

// Implementation

#include <string>
#include <vector>
#include <iterator>

#include <dds/core/detail/inttypes.hpp>
#include <dds/core/macros.hpp>

namespace org
{
namespace opensplice
{
namespace topic
{
class OMG_DDS_API FilterDelegate
{
public:
    typedef std::vector<std::string>::iterator iterator;
    typedef std::vector<std::string>::const_iterator const_iterator;

public:
    FilterDelegate();
    FilterDelegate(const std::string& query_expression);


    template <typename FWIterator>
    FilterDelegate(const std::string& query_expression,
           const FWIterator& params_begin, const FWIterator& params_end)
               : myExpression(query_expression)
    {
        std::copy(params_begin, params_end, std::back_insert_iterator<std::vector<std::string> >(myParams));
    }

    const std::string& expression() const;

    /**
    *  @internal Provides the begin iterator to the parameter list.
    */
    const_iterator begin() const;

    /**
     *  @internal The const end iterator to the parameter list.
     */
    const_iterator end() const;

    /**
     *  @internal Provides the begin const iterator to the parameter list.
     */
    iterator begin();

    /**
     *  @internal The end iterator to the parameter list.
     */
    iterator end();

    template <typename FWIterator>
    void parameters(const FWIterator& begin, const FWIterator end)
    {
        myParams.erase(myParams.begin(), myParams.end());
        std::copy(begin, end, std::back_insert_iterator<std::vector<std::string> >(myParams));
    }

    void add_parameter(const std::string& param);

    uint32_t parameters_length() const;

    bool operator ==(const FilterDelegate& other) const;

private:
    std::string myExpression;
    std::vector<std::string> myParams;
};


// End of implementation
}
}
}


#endif /* OSPL_DDS_TOPIC_DETAIL_FILTER_HPP_ */
