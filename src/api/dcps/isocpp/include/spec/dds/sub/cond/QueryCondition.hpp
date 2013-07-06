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

namespace dds { namespace sub { namespace cond {
  class QueryCondition;
} } }

/**
 * ReadCondition objects are conditions specifically dedicated
 * to read operations and attached to one DataReader.
 * <br>
 * ReadCondition objects allow an application to specify the data samples
 * it is interested in (by specifying the desired sample states, view states,
 * and instance states). See the parameter definitions for DataReader's
 * read/take operations.) This allows the middleware to enable the condition
 * only when suitable information is available. They are to be used in
 * conjunction with a WaitSet as normal conditions. More than one
 * ReadCondition may be attached to the same DataReader.
 */

class OMG_DDS_API dds::sub::cond::QueryCondition : public dds::sub::cond::TReadCondition<detail::QueryCondition> {
    typedef detail::QueryCondition DELEGATE;
public:
  // Random access iterators
  typedef DELEGATE::iterator iterator;
  typedef DELEGATE::const_iterator const_iterator;
public:

  QueryCondition(const dds::sub::Query& query,
      const dds::sub::status::DataState& status);

  template <typename FUN>
  QueryCondition(const dds::sub::Query& query,
      const dds::sub::status::DataState& status, const FUN& functor);

  ~QueryCondition();

public:

  void expression(const std::string& expr);
  const std::string& expression();

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

  template<typename FWIterator>
  void parameters(const FWIterator& begin, const FWIterator end);

  void add_parameter(const std::string& param);

  uint32_t parameters_length() const;
};

#endif  // OMG_DDS_CONTENT_SUBSCRIPTION_SUPPORT

#endif /* OMG_DDS_SUB_TQUERY_CONDITION_HPP_ */
