#ifndef OMG_DDS_SUB_TSAMPLE_HPP_
#define OMG_DDS_SUB_TSAMPLE_HPP_

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

#include <dds/core/Value.hpp>
#include <dds/sub/SampleInfo.hpp>

namespace dds {
  namespace sub {
    template <typename T, template <typename Q> class DELEGATE>
    class Sample;
  }
}

/**
 * This class encapsulates the data and meta-data associated with
 * DDS samples.
 */
template <typename T, template <typename Q> class DELEGATE>
class dds::sub::Sample : public dds::core::Value< DELEGATE<T> >
{
public:
  typedef T DataType;

public:
  /**
   * Create a sample with invalid data.
   */
  Sample();

  Sample(const T& data, const SampleInfo& info);

  Sample(const Sample& other);

  const DataType& data() const;
  void data(const DataType& d);

  const SampleInfo& info() const;
  void info(const SampleInfo& i);
};

#endif /* OMG_DDS_SUB_TSAMPLE_HPP_ */
