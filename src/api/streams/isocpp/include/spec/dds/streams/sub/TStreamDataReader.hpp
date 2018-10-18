#ifndef SPEC_DDS_STREAMS_SUB_TSTREAMDATAREADER_HPP_
#define SPEC_DDS_STREAMS_SUB_TSTREAMDATAREADER_HPP_
/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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

#include <dds/streams/sub/StreamLoanedSamples.hpp>

/**
 * The StreamDataReader allows the application to read batched data from a specified stream.
 * The selector functions can be used to specify both how and what data should be read.
 */
template <typename T, template <typename Q> class DELEGATE>
class dds::streams::sub::StreamDataReader : public dds::core::TEntity< DELEGATE<T> >
{
public:
    /**
     * The Selector class is used by the StreamDataReader
     * to compose read operations.
     */
    class Selector
    {
    public:
        /**
         * Contruct a Selector for a StreamDataReader.
         *
         * @param sdr the StreamDataReader
         */
        Selector(StreamDataReader& dr);

        /**
         * Sets the maximum amount of data samples retrieved.
         *
         * Default is dds::core::LENGTH_UNLIMITED.
         *
         * @param maxsamples maximum number of samples to get
         */
        Selector& max_samples(uint32_t maxsamples);

        /**
         * If no data is available initially, the get operation blocks for a maximum period
         * specified in the timeout parameter. If data becomes available during the timeout
         * period the FooStreamDataReader proceeds to retrieve the data and return it to the
         * application. To return immediately, the application can use the special value
         * dds::core::Duration::zero() as a timeout parameter. To block indefinitely until data is
         * available, the value dds::core::Duration::infinite() should be passed.
         *
         * Default is dds::core::Duration::zero().
         *
         * @param timeout blocking time in case no data is immediately available.
         */
        Selector& timeout(const dds::core::Duration& timeout);

        /**
         * Filters the samples returned based on the supplied filter function. The function
         * should return true if the sample passed as a parameter is to be returned.
         *
         * @param filter_func the filter function
         */
        Selector& filter(bool (*filter_func)(T));

        /**
         * Reads samples from the currently set stream
         *
         * @return the samples sequence
         */
        dds::streams::sub::StreamLoanedSamples<T> get();

        /**
         * Read up to max_samples samples from the currently set stream.
         * The samples are copied into the application provided container using the
         * forward iterator parameter.
         *
         * @param sfit samples forward iterator
         * @param max_samples the maximum number of samples to read
         * @return the number of read samples.
         */
        template <typename SamplesFWIterator>
        uint32_t get(SamplesFWIterator sfit, uint32_t max_samples);

        /**
         * Read all samples available in the reader cache from the currently set stream.
         * The samples are copied into the application provided container using a
         * back-inserting iterator. Notice that as a consequence of using a back-inserting
         * iterator, this operation may allocate memory to resize the underlying container.
         *
         * @param sbit samples back-inserting iterator
         * @return the number of read samples
         */
        template <typename SamplesBIIterator>
        uint32_t get(SamplesBIIterator sbit);

    private:
        typename DELEGATE<T>::Selector impl_;
    };

    /**
     * Selector class enabling the streaming API.
     */
    class ManipulatorSelector
    {
    public:
        /**
         * Contruct a ManipulatorSelector for a DataReader.
         *
         * @param dr the StreamDataReader
         */
        ManipulatorSelector(StreamDataReader& dr);

        /**
         * Sets the maximum amount of data samples retrieved.
         *
         * @param maxsamples maximum number of samples to get
         */
        ManipulatorSelector& max_samples(uint32_t maxsamples);

        /**
         * Sets the blocking time.
         *
         * @param timeout blocking time in case no data is immediately available.
         */
        ManipulatorSelector& timeout(const dds::core::Duration& timeout);

        /**
         * Filters the samples returned based on the supplied filter function. The function
         * should return true if the sample passed as a parameter is to be returned.
         *
         * @param filter_func the filter function
         */
        ManipulatorSelector& filter(bool (*filter_func)(T));

        /**
         * Put the result of the ManipulatorSelector into a
         * user application-supplied LoanedSamples container.
         *
         * @param samples the StreamLoanedSamples to put the samples into
         */
        ManipulatorSelector& operator>>(dds::streams::sub::StreamLoanedSamples<T>& samples);

        /**
         * Overload to allow chaining of a ManipulatorSelector.
         */
        ManipulatorSelector& operator>>(ManipulatorSelector & (manipulator)(ManipulatorSelector&));

        template <typename Functor>
        ManipulatorSelector operator>>(Functor f);

    private:
        typename DELEGATE<T>::ManipulatorSelector impl_;
    };

    OMG_DDS_BASIC_REF_TYPE(StreamDataReader, dds::core::TEntity, DELEGATE<T>)

    /**
     * Creates a StreamDataReader
     *
     * @param stream_name the stream name
     * @param qos the StreamDataReaderQos
     */
    StreamDataReader(const std::string& stream_name,
                     const dds::streams::sub::qos::StreamDataReaderQos& qos = dds::streams::sub::qos::StreamDataReaderQos());

    /**
     * Creates a StreamDataReader
     *
     * @param domain_id the domain id
     * @param stream_name the stream name
     * @param qos the StreamDataReaderQos
     */
    StreamDataReader(uint32_t domain_id, const std::string& stream_name,
                     const dds::streams::sub::qos::StreamDataReaderQos& qos = dds::streams::sub::qos::StreamDataReaderQos());

    /**
     * Creates a StreamDataReader
     *
     * @param publisher the publisher
     * @param stream_name the stream name
     * @param qos the StreamDataReaderQos
     */
    StreamDataReader(const dds::sub::Subscriber& subscriber, const std::string& stream_name,
                     const dds::streams::sub::qos::StreamDataReaderQos& qos = dds::streams::sub::qos::StreamDataReaderQos());

    /**
     * Sets the stream id that StreamDataReader operations will act on.
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
    StreamDataReader& stream(uint32_t id);

    /**
     * Gets the currently set stream id.
     *
     * @return the stream id
     */
    uint32_t stream();

    /**
     * Reads samples from the currently set stream using the default filter state
     *
     * @return the samples sequence
     */
    dds::streams::sub::StreamLoanedSamples<T> get();

    /**
     * Read up to max_samples samples using the default filter state.
     * The samples are copied into the application provided container using the
     * forward iterator parameter.
     *
     * @param sfit samples forward iterator
     * @param max_samples the maximum number of samples to read
     * @return the number of read samples.
     */
    template <typename SamplesFWIterator>
    uint32_t get(SamplesFWIterator sfit, uint32_t max_samples);

    /**
     * Read all samples available in the reader cache using the default filter state.
     * The samples are copied into the application provided container using a
     * back-inserting iterator. Notice that as a consequence of using a back-inserting
     * iterator, this operation may allocate memory to resize the underlying container.
     *
     * @param sbit samples back-inserting iterator
     * @return the number of read samples
     */
    template <typename SamplesBIIterator>
    uint32_t get(SamplesBIIterator sbit);

    /**
     * Get a Selector to perform complex data selections
     *
     * @return the selector
     */
    Selector select();

    /**
     * Manipulators are to allow streaming get operations
     */
    StreamDataReader& operator>>(dds::streams::sub::StreamLoanedSamples<T>& sls);

    ManipulatorSelector operator>>(ManipulatorSelector & (manipulator)(ManipulatorSelector&));

    template <typename Functor>
    ManipulatorSelector operator>>(Functor f);

    /**
     * Get the QoS associated with this reader.
     *
     * @return the StreamDataReaderQos
     */
    const dds::streams::sub::qos::StreamDataReaderQos& qos() const;

    /**
     * Set the QoS associated with this reader.
     *
     * @param qos the new QoS
     */
    void qos(const dds::streams::sub::qos::StreamDataReaderQos& qos);

    /**
     * Set the QoS associated with this reader.
     *
     * @param qos the new reader QoS
     */
    dds::streams::sub::qos::StreamDataReaderQos& operator<<(const dds::streams::sub::qos::StreamDataReaderQos& qos);

    /**
     * Get the QoS associated with this reader.
     *
     * @param qos the current reader QoS
     * @return the StreamDataReaderQos
     */
    const StreamDataReader& operator>>(dds::streams::sub::qos::StreamDataReaderQos& qos) const;
};

#endif /* SPEC_DDS_STREAMS_SUB_TSTREAMDATAREADER_HPP_ */
