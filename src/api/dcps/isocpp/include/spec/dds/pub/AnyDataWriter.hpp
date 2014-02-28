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
#include <dds/pub/detail/AnyDataWriter.hpp>

namespace dds
{
namespace pub
{
class AnyDataWriter;

/**
 * Extracts a typed DataWriter from an AnyDataWriter.
 *
 * @param adw the AnyDataWriter
 * @return the typed DataWriter
 */
template <typename T>
DataWriter<T> get(const AnyDataWriter& adw);
}
}

/**
 * This class provides a holder for representing a generic DDS
 * DataWriter.
 */
class OMG_DDS_API dds::pub::AnyDataWriter
{
public:
    /**
     * Creates a null AnyDataWriter.
     *
     * @param src dds::core::null
     */
    inline AnyDataWriter(const dds::core::null_type& src);

    /**
     * Constructs an AnyDataWriter from a typed DataWriter.
     *
     * @param dw the DataWriter
     */
    template <typename T>
    AnyDataWriter(const dds::pub::DataWriter<T>& dw);

public:
    /**
     * Gets the qos.
     *
     * @return the qos
     */
    const dds::pub::qos::DataWriterQos& qos() const;

    /**
     * Sets the qos.
     *
     * @param qos the qos
     */
    void qos(const ::dds::pub::qos::DataWriterQos& qos);

    /**
     * Gets the topic_name.
     *
     * @return the topic_name
     */
    const std::string& topic_name() const;

    /**
     * Gets the type_name.
     *
     * @return the type_name
     */
    const std::string& type_name() const;

    /**
     * Gets the Publisher associated with the DataWriter.
     *
     * @return the Publisher associated with the DataWriter
     */
    const dds::pub::Publisher& publisher() const;

    /**
     * This operation blocks the calling thread until either all data
     * written by the reliable DataWriter entities is acknowledged by
     * all matched reliable DataReader entities, or else the duration
     * specified by the timeout parameter elapses, whichever happens
     * first.
     *
     * @param timeout the timeout duration
     */
    void wait_for_acknowledgments(const dds::core::Duration& timeout);

    /**
     * This function closes the entity and releases all resources associated
     * with DDS, such as threads, sockets, buffers, etc. Any attempt to
     * invoke functions on a closed entity will raise an exception.
     */
    void close();

    /**
     * Indicates that references to this object may go out of scope but that
     * the application expects to look it up again later. Therefore, the
     * Service must consider this object to be still in use and may not
     * close it automatically.
     */
    void retain(bool b);

public:
    inline AnyDataWriter&
    swap(AnyDataWriter& rhs);

    template <typename T> AnyDataWriter&
    operator =(const dds::pub::DataWriter<T>& rhs);

    inline AnyDataWriter& operator =(AnyDataWriter rhs);

    bool operator==(const dds::pub::AnyDataWriter& other) const
    {
        return holder_.get()->get_dds_datawriter() == other.holder_.get()->get_dds_datawriter();
    }

public:
    /**
     * Extracts a typed DataWriter from this AnyDataWriter.
     *
     * @return a typed DataWriter
     */
    template <typename T>
    dds::pub::DataWriter<T> get();

public:
    /**
     * Direct access to functions available on the delegate. Notice that
     * this should only be used to access proprietary extensions.
     */
    const detail::DWHolderBase* operator->() const;

    /**
     * Direct access to functions available on the delegate. Notice that
     * this should only be used to access proprietary extensions.
     */
    detail::DWHolderBase* operator->();


private:
    dds::core::smart_ptr_traits<detail::DWHolderBase>::ref_type holder_;
};

#endif /* OMG_DDS_PUB_ANY_DATA_WRITER_HPP_ */
