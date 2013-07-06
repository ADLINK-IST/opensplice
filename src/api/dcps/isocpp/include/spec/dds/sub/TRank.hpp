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
#ifndef OMG_DDS_SUB_TRANK_HPP_
#define OMG_DDS_SUB_TRANK_HPP_

#include <dds/core/Value.hpp>

namespace dds  { namespace sub {
  template <typename DELEGATE>
  class TRank;
} }


/**
 * This class encapsulate the concept of Rank for a sample.
 */
template <typename DELEGATE>
class dds::sub::TRank : public dds::core::Value<DELEGATE> {
public:
  TRank();

  TRank(int32_t s, int32_t a, int32_t ag);

  int32_t       absolute_generation() const;
  inline int32_t   generation() const;
  inline int32_t   sample() const;
};

#endif /* OMG_DDS_SUB_TRANK_HPP_ */
