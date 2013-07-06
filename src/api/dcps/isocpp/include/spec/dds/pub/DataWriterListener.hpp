#ifndef OMG_DDS_PUB_DATA_WRITER_LISTENER_HPP_
#define OMG_DDS_PUB_DATA_WRITER_LISTENER_HPP_

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


#include <dds/pub/DataWriter.hpp>

namespace dds { namespace pub {

  template <typename T>
  class DataWriterListener {
  public:
    virtual ~DataWriterListener() { }

  public:
    virtual void on_offered_deadline_missed(
        dds::pub::DataWriter<T>& writer,
        const dds::core::status::OfferedDeadlineMissedStatus& status) = 0;

    /* @todo OSPL-1944 corrected typo */
    virtual void on_offered_incompatible_qos(
        dds::pub::DataWriter<T>& writer,
        const dds::core::status::OfferedIncompatibleQosStatus&  status) = 0;

    virtual void on_liveliness_lost(
        dds::pub::DataWriter<T>& writer,
        const dds::core::status::LivelinessLostStatus& status) = 0;

    virtual void on_publication_matched(
        dds::pub::DataWriter<T>& writer,
        const dds::core::status::PublicationMatchedStatus& status) = 0;
  };


  template <typename T>
  class NoOpDataWriterListener : public virtual DataWriterListener<T> {
  public:
    virtual ~NoOpDataWriterListener();

  public:
    virtual void
    on_offered_deadline_missed(dds::pub::DataWriter<T>& writer,
        const dds::core::status::OfferedDeadlineMissedStatus& status);

    virtual void
    on_offered_incompatible_qos(dds::pub::DataWriter<T> writer,
        const dds::core::status::OfferedIncompatibleQosStatus&  status);

    virtual void
    on_liveliness_lost(dds::pub::DataWriter<T>& writer,
        const dds::core::status::LivelinessLostStatus& status);

    virtual void
    on_publication_matched(dds::pub::DataWriter<T>& writer,
        const dds::core::status::PublicationMatchedStatus& status);
  };

} }

#endif /* OMG_DDS_PUB_DATA_WRITER_LISTENER_HPP_ */
