#ifndef DDS_TOPIC_TFILTER__HPP_
#define DDS_TOPIC_TFILTER__HPP_

/* Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Corp.
 * Copyright 2010, Real-Time Innovations, Inc.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <dds/core/types.hpp>
#include <dds/core/Value.hpp>

namespace dds
{
namespace topic
{
template<typename D>
class TFilter;
}
}

/**
 * This class defines a filter that can be used to create a
 * ContentFilteredTopic.
 */
template<typename D>
class dds::topic::TFilter: public dds::core::Value<D>
{
public:
    // Random access iterators
    typedef typename D::iterator iterator;
    typedef typename D::const_iterator const_iterator;

public:
    /**
     * Create a Filter based on a query expression.
     *
     * @param query_expression the query string on which to base the Filter
     */
    TFilter(const std::string& query_expression);

    /**
     * Create a Filter based on a query expression and an
     * iterable parameter container.
     *
     * @param query_expression the query string on which to base the Filter
     * @param params_begin the begining forward iterator to the user's parameters
     * @param params_end the end iterator for the user's parameters
     */
    template<typename FWIterator>
    TFilter(const std::string& query_expression, const FWIterator& params_begin,
            const FWIterator& params_end);

    /**
     * Create a Filter based on a query expression and parameter vector.
     *
     * @param query_expression the query string on which to base the Filter
     * @param params the user-supplied vector of strings parameters
     */
    TFilter(const std::string& query_expression,
            const std::vector<std::string>& params);

    /**
     * Get the query expression.
     *
     * @return the query expression
     */
    const std::string& expression() const;

    /**
     * Provides the begin iterator to the parameter list.
     */
    const_iterator begin() const;

    /**
     * The end iterator to the parameter list.
     */
    const_iterator end() const;

    /**
     * Provides the begin iterator to the parameter list.
     */
    iterator begin();

    /**
     * The end iterator to the parameter list.
     */
    iterator end();

    /**
     * Set the parameters using a iterable parameter container.
     *
     * @param params_begin the begining forward iterator to the user's parameters
     * @param end the end iterator for the user's parameters
     */
    template<typename FWIterator>
    void parameters(const FWIterator& begin, const FWIterator end);

    /**
     * Add a sigular parameter.
     *
     * @param param a sigular parameter
     */
    void add_parameter(const std::string& param);

    /**
     * Get the number of parameters associated with a Filter.
     *
     * @return the number of parameters
     */
    uint32_t parameters_length() const;
};

#endif /* DDS_TOPIC_TFILTER__HPP_ */
