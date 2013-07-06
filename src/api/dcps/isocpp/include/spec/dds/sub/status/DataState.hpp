#ifndef OMG_DDS_SUB_DATA_STATE_HPP_
#define OMG_DDS_SUB_DATA_STATE_HPP_

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

#include <bitset>

#include <dds/core/types.hpp>


namespace dds {
  namespace sub {
    namespace status {
      class SampleState;
      class ViewState;
      class InstanceState;
      class DataState;
    }
  }
}


class OMG_DDS_API dds::sub::status::SampleState : public std::bitset<OMG_DDS_STATE_BIT_COUNT> {
public:
  typedef std::bitset<OMG_DDS_STATE_BIT_COUNT> MaskType;

public:
  SampleState();
  explicit SampleState(uint32_t i);
  SampleState(const SampleState& src);
  SampleState(const MaskType& src);

public:
  inline static const SampleState read();

  inline static const SampleState not_read();

  inline static const SampleState any();

};


class OMG_DDS_API dds::sub::status::ViewState : public std::bitset<OMG_DDS_STATE_BIT_COUNT> {
public:
  typedef std::bitset<OMG_DDS_STATE_BIT_COUNT> MaskType;

public:
  ViewState();
  explicit ViewState(uint32_t m);
  ViewState(const ViewState& src);
  ViewState(const MaskType& src);

public:
  inline static const ViewState new_view();

  inline static const ViewState not_new_view();

  inline static const ViewState any();

};


class OMG_DDS_API dds::sub::status::InstanceState : public std::bitset<OMG_DDS_STATE_BIT_COUNT> {
public:
  typedef std::bitset<OMG_DDS_STATE_BIT_COUNT> MaskType;

public:
  explicit InstanceState(uint32_t m);
  InstanceState();
  InstanceState(const InstanceState& src);
  InstanceState(const MaskType& src);

public:
  inline static const InstanceState alive();

  inline static const InstanceState not_alive_disposed();

  inline static const InstanceState not_alive_no_writers();

  inline static const InstanceState not_alive_mask();

  inline static const InstanceState any();

};

class OMG_DDS_API dds::sub::status::DataState {
public:
  DataState()
  : ss_(dds::sub::status::SampleState::any()),
    vs_(dds::sub::status::ViewState::any()),
    is_(dds::sub::status::InstanceState::any())
  { }

  /* implicit */ DataState(const dds::sub::status::SampleState& ss)
  : ss_(ss),
    vs_(dds::sub::status::ViewState::any()),
    is_(dds::sub::status::InstanceState::any())
  { }

  /* implicit */ DataState(const dds::sub::status::ViewState& vs)
  : ss_(dds::sub::status::SampleState::any()),
    vs_(vs),
    is_(dds::sub::status::InstanceState::any())
  { }

  /* implicit */ DataState(const dds::sub::status::InstanceState& is)
  : ss_(dds::sub::status::SampleState::any()),
    vs_(dds::sub::status::ViewState::any()),
    is_(is)
  { }

  DataState(const dds::sub::status::SampleState& ss,
      const dds::sub::status::ViewState& vs,
      const dds::sub::status::InstanceState& is)
  : ss_(ss), vs_(vs), is_(is)
  { }

  DataState& operator << (const dds::sub::status::SampleState& ss) {
    ss_ = ss;
    return *this;
  }
  DataState& operator << (const dds::sub::status::InstanceState& is) {
    is_ = is;
    return *this;
  }
  DataState& operator << (const dds::sub::status::ViewState& vs) {
    vs_ = vs;
    return *this;
  }

  const DataState& operator >> (dds::sub::status::SampleState& ss) const {
    ss = ss_;
    return *this;
  }

  const DataState& operator >> (dds::sub::status::InstanceState& is) const {
    is = is_;
    return *this;
  }

  const DataState& operator >> (dds::sub::status::ViewState& vs) const {
    vs = vs_;
    return *this;
  }

  const dds::sub::status::SampleState& sample_state() const {
    return ss_;
  }

  void sample_state(const dds::sub::status::SampleState& ss) {
    *this << ss;
  }

  const dds::sub::status::InstanceState& instance_state() const{
    return is_;
  }
  void instance_state(const dds::sub::status::InstanceState& is) {
    *this << is;
  }

  const dds::sub::status::ViewState& view_state() const {
    return vs_;
  }
  void view_state(const dds::sub::status::ViewState& vs) {
    *this << vs;
  }

  static DataState any() {
    return DataState(dds::sub::status::SampleState::any(),
        dds::sub::status::ViewState::any(),
        dds::sub::status::InstanceState::any());
  }

  static DataState new_data() {
    return DataState(dds::sub::status::SampleState::not_read(),
        dds::sub::status::ViewState::any(),
        dds::sub::status::InstanceState::alive());
  }

  static DataState any_data() {
    return DataState(dds::sub::status::SampleState::any(),
        dds::sub::status::ViewState::any(),
        dds::sub::status::InstanceState::alive());
  }

  static DataState new_instance() {
    return DataState(dds::sub::status::SampleState::any(),
        dds::sub::status::ViewState::new_view(),
        dds::sub::status::InstanceState::alive());
  }
private:
  dds::sub::status::SampleState ss_;
  dds::sub::status::ViewState vs_;
  dds::sub::status::InstanceState is_;

};

#endif /* OMG_DDS_SUB_DATA_STATE_HPP_ */
