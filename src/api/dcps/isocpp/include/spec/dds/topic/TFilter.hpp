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

namespace dds {
  namespace topic {
    template<typename D>
    class TFilter;
  }
}

/**
 * This clas defines a filter that can be used to create a
 * <code>ContentFilteredTopic</code>.
 */
template<typename D>
class dds::topic::TFilter: public dds::core::Value<D> {
public:
  // Random access iterators
  typedef typename D::iterator iterator;
  typedef typename D::const_iterator const_iterator;

public:
  TFilter(const std::string& query_expression);

  template<typename FWIterator>
  TFilter(const std::string& query_expression, const FWIterator& params_begin,
      const FWIterator& params_end);

  TFilter(const std::string& query_expression,
      const std::vector<std::string>& params);

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

  template<typename FWIterator>
  void parameters(const FWIterator& begin, const FWIterator end);

  void add_parameter(const std::string& param);

  uint32_t parameters_length() const;
};

#endif /* DDS_TOPIC_TFILTER__HPP_ */
