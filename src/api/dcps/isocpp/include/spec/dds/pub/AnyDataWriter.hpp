#ifndef OMG_DDS_PUB_ANY_DATA_WRITER_HPP_
#define OMG_DDS_PUB_ANY_DATA_WRITER_HPP_

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

#include <dds/core/macros.hpp>
#include <dds/core/ref_traits.hpp>
#include <dds/core/Exception.hpp>
#include <dds/pub/DataWriter.hpp>
#include <dds/pub/detail/AnyDataWriter.hpp>

namespace dds {
  namespace pub {
    class OS_API_EXPORT AnyDataWriter;

    /**
     * Extracts a typed <code>DataWriter</code> from an
     * <code>AnyDataWriter</code>.
     *
     */
    template <typename T>
    DataWriter<T> get(const AnyDataWriter& adw);
  }
}

/**
 * This class provides an holder for representing a generic DDS
 * <code>DataWriter</code>.
 */
class OS_API_EXPORT dds::pub::AnyDataWriter {
public:
  template <typename T>
  AnyDataWriter(const dds::pub::DataWriter<T>& dw);

public:
  const dds::pub::qos::DataWriterQos& qos() const;

  void qos(const ::dds::pub::qos::DataWriterQos& q);

  const std::string& topic_name() const;

  const std::string& type_name() const;

  const dds::pub::Publisher& publisher() const;

  void wait_for_acknowledgments(const dds::core::Duration& timeout);

  void close();

  void retain(bool b);

public:
  inline AnyDataWriter&
  swap(AnyDataWriter& rhs);

  template <typename T> AnyDataWriter&
  operator =(const dds::pub::DataWriter<T>& rhs);

  inline AnyDataWriter& operator =(AnyDataWriter rhs);

public:
  /**
   * Extracts a typed <code>DataWriter</code> from this.
   *
   */
  template <typename T>
  dds::pub::DataWriter<T> get();

public:
  /**
   * Direct access to methods available on the delegate. Notice that
   * this should only be used to access proprietary extensions.
   */
  const detail::DWHolderBase* operator->() const;

  /**
   * Direct access to methods available on the delegate. Notice that
   * this should only be used to access proprietary extensions.
   */
  detail::DWHolderBase* operator->();


private:
  dds::core::smart_ptr_traits<detail::DWHolderBase>::ref_type holder_;
};

#endif /* OMG_DDS_PUB_ANY_DATA_WRITER_HPP_ */
