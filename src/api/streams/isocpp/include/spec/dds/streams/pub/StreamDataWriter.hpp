#ifndef SPEC_DDS_STREAMS_PUB_STREAM_DATA_WRITER_HPP_
#define SPEC_DDS_STREAMS_PUB_STREAM_DATA_WRITER_HPP_
/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

/**
 * @file
 */

#include <dds/core/refmacros.hpp>
#include <dds/pub/Publisher.hpp>
#include <dds/streams/pub/detail/StreamDataWriter.hpp>

namespace dds
{
namespace streams
{
namespace pub
{

template <typename T, template <typename Q> class DELEGATE = dds::streams::pub::detail::StreamDataWriter >
class StreamDataWriter;

}
}
}


/**
 * The StreamDataWriter allows data to be appended to a specified stream which is then written as
 * a batch, either by calling the flush function explicitly or by customising the StreamFlush QoS
 * to perform flushes automatically.
 */
template <typename T, template <typename Q> class DELEGATE>
class dds::streams::pub::StreamDataWriter : public ::dds::core::TEntity< DELEGATE<T> >
{

public:
    OMG_DDS_BASIC_REF_TYPE(StreamDataWriter, ::dds::core::TEntity, DELEGATE<T>)

    /**
     * Creates a StreamDataWriter
     *
     * @param stream_name the stream name
     * @param qos the StreamDataWriterQos
     */
    StreamDataWriter(const std::string& stream_name,
                     const dds::streams::pub::qos::StreamDataWriterQos& qos = dds::streams::pub::qos::StreamDataWriterQos());

    /**
     * Creates a StreamDataWriter
     *
     * @param domain_id the domain id
     * @param stream_name the stream name
     * @param qos the StreamDataWriterQos
     */
    StreamDataWriter(uint32_t domain_id,
                     const std::string& stream_name,
                     const dds::streams::pub::qos::StreamDataWriterQos& qos = dds::streams::pub::qos::StreamDataWriterQos());

    /**
     * Creates a StreamDataWriter
     *
     * @param publisher the publisher
     * @param stream_name the stream name
     * @param qos the StreamDataWriterQos
     */
    StreamDataWriter(const dds::pub::Publisher& publisher,
                     const std::string& stream_name,
                     const dds::streams::pub::qos::StreamDataWriterQos& qos = dds::streams::pub::qos::StreamDataWriterQos());

    /**
     * Sets the stream id that StreamDataWriter operations will act on.
     *
     * For each stream of a certain type, multiple ‘instances’ of this stream-type can be created
     * by assigning unique ids to each of streams. Each id then represents an ‘instance’ of
     * the stream of the associated type. So the actual stream instance is selected based on
     * the supplied StreamId.
     *
     * When the stream doesn’t exist it is automatically created based on the current QoS
     * settings.
     *
     * @param id the stream id
     */
    StreamDataWriter& stream(uint32_t id);

    /**
     * Gets the currently set stream id.
     *
     * @return the stream id
     */
    uint32_t stream();

    /**
     * Appends a sample to the currently set stream to be written upon the next
     * flush.
     *
     * @param data the data sample
     */
    StreamDataWriter& append(const T& data);

    /**
     * Appends a sequence of samples (determined by the template specialization) to the
     * currently set stream to be written upon the next flush.
     *
     * @param begin an iterator pointing to the beginning of a sequence of
     * samples
     * @param end an iterator pointing to the end of a sequence of
     * samples
     */
    template <typename FWIterator>
    StreamDataWriter& append(const FWIterator& begin, const FWIterator& end);

    /**
     * Writes a sample to the currently set stream.
     *
     * @param data the data sample
     */
    StreamDataWriter& operator<<(const T& data);

    /*
     * Flushes the currently set stream.
     *
     * When a stream is flushed, all data in the stream is delivered to DDS and the stream
     * is emptied. The memory allocated will be reused the next time data is appended to
     * the stream.
     */
    StreamDataWriter& flush();

    /**
     * Gets the StreamDataWriter QoS.
     *
     * @return the StreamDataWriter QoS
     */
    const dds::streams::pub::qos::StreamDataWriterQos& qos() const;

    /**
     * Sets the StreamDataWriter QoS.
     *
     * @param qos the new qos for this StreamDataWriter
     */
    void qos(const dds::streams::pub::qos::StreamDataWriterQos& qos);

    /**
     * Sets the StreamDataWriter QoS.
     *
     * @param qos the new qos for this StreamDataWriter
     */
    dds::streams::pub::qos::StreamDataWriterQos& operator<<(const dds::streams::pub::qos::StreamDataWriterQos& qos);

    /**
     * Gets the StreamDataWriter QoS.
     *
     * @param qos will be set to the current qos of this DataWriter
     */
    const StreamDataWriter& operator>>(dds::streams::pub::qos::StreamDataWriterQos& qos) const;

};

#endif /* SPEC_DDS_STREAMS_PUB_STREAM_DATA_WRITER_HPP_ */
