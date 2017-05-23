#ifndef OMG_DDS_SUB_TQUERY_CONDITION_HPP_
#define OMG_DDS_SUB_TQUERY_CONDITION_HPP_

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

#include <dds/core/detail/conformance.hpp>
#include <dds/sub/cond/detail/QueryCondition.hpp>
#include <dds/sub/cond/TReadCondition.hpp>


#ifdef OMG_DDS_CONTENT_SUBSCRIPTION_SUPPORT

namespace dds
{
namespace sub
{
namespace cond
{
class QueryCondition;
}
}
}

/**
 * QueryCondition objects are specialized ReadCondition objects that allow
 * the application to also specify a filter on the locally available data.
 *
 * The query (query_expression) is similar to an SQL WHERE clause can be
 * parameterized by arguments.
 *
 * Precise syntax for the query expression can be found in Annex A.
 */

class OMG_DDS_API dds::sub::cond::QueryCondition : public dds::sub::cond::TReadCondition<dds::sub::cond::detail::QueryCondition>
{
    typedef detail::QueryCondition DELEGATE;
public:
    // Random access iterators
    typedef DELEGATE::iterator iterator;
    typedef DELEGATE::const_iterator const_iterator;
public:
    /**
     * Creates a null QueryCondition.
     */
    QueryCondition();
    QueryCondition(const dds::core::null_type&);

    /**
     * Creates a QueryCondition instance.
     *
     * @param query a query
     * @param status the data state
     */
    QueryCondition(const dds::sub::Query& query,
                   const dds::sub::status::DataState& status);

    /**
     * Creates a QueryCondition instance.
     *
     * @param query a query
     * @param status the data state
     * @param functor the functor to be called when the condition is triggered
     */
    template <typename FUN>
    QueryCondition(const dds::sub::Query& query,
                   const dds::sub::status::DataState& status, const FUN& functor);

    virtual ~QueryCondition();

public:
    /**
     * Set the Query expression.
     *
     * @param expr the expression
     */
    void expression(const std::string& expr);

    /**
     * Get the Query expression.
     *
     * @return the expression
     */
    const std::string& expression();

    /**
     * Provides the beginning iterator of the parameter list.
     *
     * @return the beginning iterator
     */
    const_iterator begin() const;

    /**
     * Provides the end iterator of the parameter list.
     *
     * @param the end iterator
     */
    const_iterator end() const;

    /**
     * Provides the beginning iterator of the parameter list.
     *
     * @return the beginning iterator
     */
    iterator begin();

    /**
     * Provides the end iterator of the parameter list.
     *
     * @param the end iterator
     */
    iterator end();

    /**
     * Sets the Query parameters.
     *
     * @param begin an iterator pointing to the beginning of the parameters to set
     * @param end an iterator pointing to the end of the parameters to set
     */
    template<typename FWIterator>
    void parameters(const FWIterator& begin, const FWIterator end);

    /**
     * Adds a parameter to the Query.
     *
     * @param param the parameter to add
     */
    void add_parameter(const std::string& param);

    /**
     * Gets the number of parameters in the Query.
     *
     * @return the number of parameters in the Query
     */
    uint32_t parameters_length() const;
};

#endif  // OMG_DDS_CONTENT_SUBSCRIPTION_SUPPORT

#endif /* OMG_DDS_SUB_TQUERY_CONDITION_HPP_ */
