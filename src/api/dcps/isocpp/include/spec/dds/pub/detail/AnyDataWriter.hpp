#ifndef OMG_DDS_PUB_DETAIL_ANY_DATA_WRITER_HPP_
#define OMG_DDS_PUB_DETAIL_ANY_DATA_WRITER_HPP_

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

#include <string>
#include <dds/pub/qos/DataWriterQos.hpp>
#include <dds/pub/Publisher.hpp>
#include <dds/pub/DataWriter.hpp>

// NOTE: This code is non-normative and is provided only as a guide
// to implementor of this specification.

namespace dds { namespace pub { namespace detail {
    class DWHolderBase;
    template <typename T> class DWHolder;
} } }

class dds::pub::detail::DWHolderBase {
public:
  virtual ~DWHolderBase() { }

    virtual const dds::pub::qos::DataWriterQos& qos() const = 0;

    virtual void qos(const ::dds::pub::qos::DataWriterQos& qos) = 0;

    virtual const std::string& topic_name() const = 0;

    virtual const std::string& type_name() const = 0;

    virtual  const dds::pub::Publisher& publisher() const = 0;

    virtual void wait_for_acknowledgments(const dds::core::Duration& timeout) = 0;

    virtual void close() = 0;

    virtual void retain(bool b) = 0;
};

template <typename T>
class dds::pub::detail::DWHolder : public DWHolderBase {
public:
    DWHolder(const dds::pub::DataWriter<T>& dw) : dw_(dw) { }
    virtual ~DWHolder() { }
public:
    virtual const ::dds::pub::qos::DataWriterQos& qos() const {
        return dw_.qos();
    }

    virtual void qos(const ::dds::pub::qos::DataWriterQos& the_qos) {
      dw_.qos(the_qos);
    }

    virtual const std::string& topic_name() const {
        return dw_.topic().name();
    }

    virtual const std::string& type_name() const {
        return dw_.topic().type_name();
    }

    virtual const ::dds::pub::Publisher& publisher() const {
        return dw_.publisher();
    }

  virtual void wait_for_acknowledgments(const dds::core::Duration& timeout) {
      dw_.wait_for_acknowledgments(timeout);
    }

    virtual void close() {
        dw_.close();
    }

    virtual void retain(bool b)  { }

    const dds::pub::DataWriter<T>& get() const { return dw_; }

private:
    dds::pub::DataWriter<T> dw_;
};


#endif /* OMG_DDS_PUB_DETAIL_ANY_DATA_WRITER_HPP_ */
