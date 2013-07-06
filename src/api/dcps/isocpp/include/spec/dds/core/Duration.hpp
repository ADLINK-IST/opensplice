#ifndef OMG_DDS_CORE_DURATION_HPP_
#define OMG_DDS_CORE_DURATION_HPP_

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

namespace dds { namespace core {

  /**
   * This class represents a time interval.
   */
  class OMG_DDS_API Duration {
  public:
    static const Duration zero();       // {0, 0}
    static const Duration infinite();   // {0x7fffffff, 0x7fffffff}
  public:
    /**
     * Create a duration elapsing zero seconds.
     */
    Duration();

    /**
     * Create a duration elapsing a specific amount of time.
     */
    explicit Duration(int64_t sec, uint32_t nanosec = 0);

    ~Duration();

  public:
    static const Duration from_microsecs(int64_t microseconds);
    static const Duration from_millisecs(int64_t milliseconds);
    static const Duration from_secs(double seconds);

  public:
    int64_t sec() const;
    void    sec(int64_t s);

    uint32_t nanosec() const;
    void     nanosec(uint32_t ns);

  public:
    int compare(const Duration& that) const;

    bool operator >(const Duration& that) const;
    bool operator >=(const Duration& that) const;

    bool operator ==(const Duration& that) const;

    bool operator <=(const Duration& that) const;
    bool operator <(const Duration& that) const;

  public:
    Duration& operator+=(const Duration &a_ti);
    Duration& operator-=(const Duration &a_ti);
    Duration& operator*=(uint64_t factor);

    const Duration operator +(const Duration& other) const;
    const Duration operator -(const Duration& other) const;
  public:
    /**
     * Returns this <code>Duration</code> in milli-seconds.
     *
     * @return the duration in milliseconds
     */
    int64_t to_millisecs() const;

    /**
     * Returns this <code>Duration</code> in micro-seconds.
     *
     * @return the duration in micro-seconds
     */
    int64_t to_microsecs() const;

    /**
     * Returns this <code>Duration</code> in seconds.
     *
     * @return the duration in seconds
     */
    double to_secs() const;

  private:
    int32_t sec_;
    uint32_t nsec_;
  };

  const Duration OMG_DDS_API operator *(uint64_t lhs,
      const Duration& rhs);

  const Duration OMG_DDS_API operator *(const Duration& lhs,
      uint64_t rhs);

  const Duration OMG_DDS_API operator /(const Duration& lhs,
      uint64_t rhs);

} } /* namespace dds / namespace core  */
#endif /* OMG_DDS_CORE_DURATION_HPP_ */
