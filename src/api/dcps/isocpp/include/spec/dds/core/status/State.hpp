#ifndef OMG_DDS_CORE_STATUS_STATE_HPP_
#define OMG_DDS_CORE_STATUS_STATE_HPP_

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
#include <dds/core/macros.hpp>
#include <dds/core/types.hpp>


namespace dds { namespace core { namespace status {


  class OMG_DDS_API SampleRejectedState : public std::bitset<OMG_DDS_STATE_BIT_COUNT> {
  public:
    typedef std::bitset<OMG_DDS_STATE_BIT_COUNT> MaskType;

  public:
    SampleRejectedState();
    SampleRejectedState(const SampleRejectedState& src);
    SampleRejectedState(const MaskType& src);

  public:
    inline static const SampleRejectedState not_rejected() {
      return SampleRejectedState(0u);
    }
    inline static const SampleRejectedState rejected_by_samples_limit() {
      return SampleRejectedState(0x0001 << 1u);
    }
    inline static const SampleRejectedState rejected_by_instances_limit() {
      return SampleRejectedState(0x0001 << 0u);
    }
    inline static const SampleRejectedState rejected_by_samples_per_instance_limit() {
      return SampleRejectedState(0x0001 << 2u);
    }

  private:
    SampleRejectedState(uint32_t s);
  };


  // StatusMask create_status_mask(uint64_t);

  class OMG_DDS_API StatusMask : public std::bitset<OMG_DDS_STATUS_COUNT> {
  public:
    typedef std::bitset<OMG_DDS_STATUS_COUNT> MaskType;

  public:
    StatusMask();
    explicit StatusMask(uint32_t mask);
    StatusMask(const StatusMask& other);
    ~StatusMask();

  public:
    inline StatusMask& operator << (const dds::core::status::StatusMask& mask)
    {
        *this |= mask;
        return *this;
    }

    inline static const StatusMask all() {
      return StatusMask(0x7fe7u);
    }

    inline static const StatusMask none() {
      return StatusMask(0u);
    }

  public:
    inline static const StatusMask inconsistent_topic() {
      return StatusMask(0x00000001 << 0u);
    }

    inline static const StatusMask offered_deadline_missed() {
      return StatusMask(0x00000001 << 1u);
    }

    inline static const StatusMask requested_deadline_missed() {
      return StatusMask(0x00000001 << 2u);
    }

    inline static const StatusMask offered_incompatible_qos() {
      return StatusMask(0x00000001 << 5u);
    }

    inline static const StatusMask requested_incompatible_qos() {
      return StatusMask(0x00000001 << 6u);
    }

    inline static const StatusMask sample_lost() {
      return StatusMask(0x00000001 << 7u);
    }

    inline static const StatusMask sample_rejected() {
      return StatusMask(0x00000001 << 8u);
    }

    inline static const StatusMask data_on_readers() {
      return StatusMask(0x00000001 << 9u);
    }

    inline static const StatusMask data_available() {
      return StatusMask(0x00000001 << 10u);
    }

    inline static const StatusMask liveliness_lost() {
      return StatusMask(0x00000001 << 11u);
    }

    inline static const StatusMask liveliness_changed() {
      return StatusMask(0x00000001 << 12u);
    }

    inline static const StatusMask publication_matched() {
      return StatusMask(0x00000001 << 13u);
    }

    inline static const StatusMask subscription_matched() {
      return StatusMask(0x00000001 << 14u);
    }

  };

} } } /* namespace dds / core / status*/


#endif /* OMG_DDS_CORE_STATUS_STATE_HPP_ */
