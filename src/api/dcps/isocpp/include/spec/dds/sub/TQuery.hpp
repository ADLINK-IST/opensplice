#ifndef OMG_TDDS_DDS_SUB_QUERY_HPP_
#define OMG_TDDS_DDS_SUB_QUERY_HPP_

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

#include <dds/core/Reference.hpp>
#include <dds/sub/DataReader.hpp>
#include <dds/sub/AnyDataReader.hpp>

/*
 * NOTE: The Query class is specified as follows to limit the issues associated
 * with includes dependencies and to make it easier to forward declare in places
 * where the information about the delegate layer should not pop out.
 *
 */
namespace dds {
  namespace sub {
    template <typename DELEGATE>
    class TQuery;
  }
}

template <typename DELEGATE>
class dds::sub::TQuery : public dds::core::Reference<DELEGATE> {
public:
  OMG_DDS_REF_TYPE(TQuery, dds::core::Reference, DELEGATE)

public:
  // Random access iterators
  typedef typename DELEGATE::iterator iterator;
  typedef typename DELEGATE::const_iterator const_iterator;

public:
  template<typename T>
  TQuery(const dds::sub::DataReader<T>& dr, const std::string& query_expression);

  template<typename T, typename FWIterator>
  TQuery(const dds::sub::DataReader<T>& dr, const std::string& query_expression,
      const FWIterator& params_begin, const FWIterator& params_end);

  template<typename T>
  TQuery(const dds::sub::DataReader<T>& dr, const std::string& query_expression,
      const std::vector<std::string>& params);

  const std::string& expression() const;

  void expression(const std::string& expr);

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

  const AnyDataReader& data_reader() const;
};

#endif /* OMG_TDDS_DDS_SUB_QUERY_HPP_ */
