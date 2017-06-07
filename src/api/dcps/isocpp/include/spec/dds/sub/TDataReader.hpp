#ifndef OMG_DDS_SUB_TDATA_READER_HPP_
#define OMG_DDS_SUB_TDATA_READER_HPP_

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
#include <dds/core/detail/conformance.hpp>
#include <dds/core/TEntity.hpp>
#include <dds/topic/ContentFilteredTopic.hpp>
#include <dds/topic/TopicInstance.hpp>
#include <dds/sub/LoanedSamples.hpp>
#include <dds/sub/Subscriber.hpp>


namespace dds
{
namespace sub
{
template <typename FOO> class TQuery;
}
}

namespace dds
{
namespace sub
{
namespace detail
{
class FooQuery;
typedef dds::sub::TQuery<dds::sub::detail::FooQuery> Query;
}
}
}

namespace dds
{
namespace sub
{
typedef dds::sub::detail::Query Query;

template <typename T, template <typename Q> class DELEGATE>
class DataReader;

template <typename T>
class DataReaderListener;
}
}

/**
 * A DataReader allows the application:
 *
 * 1. to declare the data it wishes to receive (i.e., make a subscription)
 * 2. to access the data received by the attached Subscriber
 */
template <typename T, template <typename Q> class DELEGATE>
class dds::sub::DataReader : public dds::core::TEntity< DELEGATE<T> >
{

public:
    typedef T                                        DataType;
    typedef ::dds::sub::DataReaderListener<T>        Listener;

public:

    /**
     * The Selector class is used by the DataReader
     * to compose read operations.
     *
     * eg.
     * ~~~~~~~~~~~~~~~{.cpp}
     * dds::sub::LoanedSamples<SAMPLE> FooSamples = reader.select().
     *                            instance(FooInstance).
     *                            state(dds::sub::status::DataState::any_data())
     *                            .read();
     *
     * //Or
     * unsigned int readSamples = reader.select().
     *                            instance(FooInstance).
     *                            state(dds::sub::status::DataState::any_data())
     *                            .take(back_inserter(FooSamples));
     *
     * ~~~~~~~~~~~~~~~
     *
     * By default the instance is nil.
     */
    class Selector
    {
    public:
        /**
         * Contruct a Selector for a DataReader.
         *
         * @param DataReader
         */
        Selector(DataReader& dr);

        /**
         * Set InstanceHandle.
         *
         * @param handle the InstanceHandle to read/take for
         */
        Selector& instance(const dds::core::InstanceHandle& handle);

        /**
         * Set DataState.
         *
         * @param state the requested DataState of the samples
         */
        Selector& state(const dds::sub::status::DataState& state);

        /**
         * Set Query.
         *
         * @param query the Query to apply to the selector
         */
        Selector& content(const dds::sub::Query& query);

        /**
         * Set max_samples.
         *
         * @param maxsamples maximum number of samples to read/take
         */
        Selector& max_samples(uint32_t maxsamples);

        /**
         * DataReader.read().
         *
         * @return the LoanedSamples from the DataReader
         */
        dds::sub::LoanedSamples<T> read();

        /**
         * DataReader.take().
         *
         * @return the LoanedSamples from the DataReader
         */
        dds::sub::LoanedSamples<T> take();

        // --- Forward Iterators: --- //
        /**
         * Read a number of samples into a iterable container.
         *
         * eg.
         * ~~~~~~~~~~~~~~~{.cpp}
         * unsigned int readSamples6  = reader.select().
         *                               instance(FooInstance).
         *                               state(dds::sub::status::DataState::any_data())
         *                               .read(FooSamples.begin(), 20);
         * ~~~~~~~~~~~~~~~
         *
         * @param sfit forward inserting iterator to user container
         * @param max_samples number of samples to read
         * @return number of samples
         */
        template <typename SamplesFWIterator>
        uint32_t
        read(SamplesFWIterator sfit, uint32_t max_samples);

        /**
         * Take a number samples into a iterable container.
         *
         * eg.
         * ~~~~~~~~~~~~~~~{.cpp}
         * unsigned int readSamples6  = reader.select().
         *                               instance(FooInstance).
         *                               state(dds::sub::status::DataState::any_data())
         *                               .take(FooSamples.begin(), 20);
         * ~~~~~~~~~~~~~~~
         *
         * @param sfit forward inserting iterator to user container
         * @param max_samples number of samples to read
         * @return number of samples
         */
        template <typename SamplesFWIterator>
        uint32_t
        take(SamplesFWIterator sfit,  uint32_t max_samples);

        // --- Back-Inserting Iterators: --- //
        /**
         * Read samples into a iterable container.
         *
         * eg.
         * ~~~~~~~~~~~~~~~{.cpp}
         * unsigned int readSamples6  = reader.select().
         *                               instance(FooInstance).
         *                               state(dds::sub::status::DataState::any_data())
         *                               .read(back_inserter(FooSamples));
         * ~~~~~~~~~~~~~~~
         *
         * @param sbit back inserting iterator to user container
         * @return number of samples
         */
        template <typename SamplesBIIterator>
        uint32_t
        read(SamplesBIIterator sbit);

        /**
         * Take samples into a iterable container.
         *
         * eg.
         * ~~~~~~~~~~~~~~~{.cpp}
         * unsigned int readSamples6  = reader.select().
         *                               instance(FooInstance).
         *                               state(dds::sub::status::DataState::any_data())
         *                               .take(back_inserter(FooSamples));
         * ~~~~~~~~~~~~~~~
         *
         * @param sbit back inserting iterator to user container
         * @return number of samples
         */
        template <typename SamplesBIIterator>
        uint32_t
        take(SamplesBIIterator sbit);

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
         * @param DataReader
         */
        ManipulatorSelector(DataReader& dr);

        /**
         * Get the read_mode.
         *
         * The read_mode specifies if a sample should be read or taken:
         *   true = read
         *   false = take
         *
         * @return true if read_mode is true
         */
        bool read_mode();

        /**
         * Set the read_mode.
         *
         * The read_mode specifies if a sample should be read or taken:
         *   true = read
         *   false = take
         *
         * @param readmode the read mode of the DataReader
         */
        void read_mode(bool readmode);

        /**
         * Set max_samples.
         *
         * @param n maximum number of samples
         */
        ManipulatorSelector& max_samples(uint32_t n);

        /**
         * Set InstanceHandle.
         *
         * @param handle the InstanceHandle for the read/take
         */
        ManipulatorSelector& instance(const dds::core::InstanceHandle& handle);

        /**
         * Set next InstanceHandle.
         *
         * @param handle the next InstanceHandle associated with the read/take
         */
        ManipulatorSelector& next_instance(const dds::core::InstanceHandle& handle);

        /**
         * Set DataState.
         *
         * @param state the required DataState of the samples
         */
        ManipulatorSelector& state(const dds::sub::status::DataState& state);

        /**
         * Set Query.
         *
         * @param query The Query to apply to a read/take
         */
        ManipulatorSelector& content(const dds::sub::Query& query);

        /**
         * Put the result of the ManipulatorSelector into a
         * user application-supplied LoanedSamples container.
         *
         * @param samples the LoanedSamples to put the read/taken samples into
         */
        ManipulatorSelector&
        operator >>(dds::sub::LoanedSamples<T>& samples);

        /**
         * Overload to allow chaining of a ManipulatorSelector.
         */
        ManipulatorSelector&
        operator >> (ManipulatorSelector & (manipulator)(ManipulatorSelector&));

        template <typename Functor>
        ManipulatorSelector
        operator >> (Functor f);

    private:
        typename DELEGATE<T>::ManipulatorSelector impl_;

    };

public:
    OMG_DDS_REF_TYPE(DataReader, dds::core::TEntity, DELEGATE<T>)

public:
    /**
     * Create a DataReader. The QoS will be the same as
     * "sub.default_datareader_qos()".
     *
     * @param sub the subscriber owning this DataReader
     * @param topic the topic associated with this DataReader.
     *
     * See \ref DCPS_Modules_Subscription_DataReader "DataReader" for more information
     */
    DataReader(const dds::sub::Subscriber& sub,
               const ::dds::topic::Topic<T>& topic);
    /**
     * Create a DataReader.
     *
     * @param sub the subscriber owning this DataReader
     * @param topic the topic associated with this DataReader
     * @param qos the QoS settings for this DataReader
     * @param listener the listener
     * @param mask the event mask associated to the DataReader listener
     */
    DataReader(const dds::sub::Subscriber& sub,
               const ::dds::topic::Topic<T>& topic,
               const dds::sub::qos::DataReaderQos& qos,
               dds::sub::DataReaderListener<T>* listener = NULL,
               const dds::core::status::StatusMask& mask = ::dds::core::status::StatusMask::none());

    #ifdef OMG_DDS_CONTENT_SUBSCRIPTION_SUPPORT

    /**
     * Create a DataReader for a ContentFilteredTopic.
     * This DataReader will only receive that data that matches the
     * Filter associated with the ContentFilteredTopic.
     * The QoS will be set to sub.default_datareader_qos().
     *
     * @param sub the subscriber owning this DataReader
     * @param topic the content filtered topic
     */
    DataReader(const dds::sub::Subscriber& sub,
               const ::dds::topic::ContentFilteredTopic<T>& topic);

    /**
     * Create a DataReader for a ContentFilteredTopic.
     * This DataReader will only receive that data that matches the
     * Filter associated with the ContentFilteredTopic.
     *
     * @param sub the subscriber owning this DataReader
     * @param topic the content filtered topic
     * @param qos the QoS settings for this DataReader
     * @param listener the listener
     * @param mask the event mask associated to the DataReader listener
     */
    DataReader(const dds::sub::Subscriber& sub,
               const ::dds::topic::ContentFilteredTopic<T>& topic,
               const dds::sub::qos::DataReaderQos& qos,
               dds::sub::DataReaderListener<T>* listener = NULL,
               const dds::core::status::StatusMask& mask = ::dds::core::status::StatusMask::none());
    #endif /* OMG_DDS_CONTENT_SUBSCRIPTION_SUPPORT */

    #ifdef OMG_DDS_MULTI_TOPIC_SUPPORT

    /**
     * Create a DataReader for a MultiTopic.
     * This DataReader will only receive that data that matches the
     * Filter associated with the ContentFilteredTopic.
     * The QoS will be set to sub.default_datareader_qos().
     *
     * @param sub the subscriber owning this DataReader
     * @param topic the multi-topic
     */
    DataReader(const dds::sub::Subscriber& sub,
               const ::dds::topic::MultiTopic<T>& topic);


    /**
     * Create a DataReader for a MultiTopic.
     * This DataReader will only receive that data that matches the
     * Filter associated with the ContentFilteredTopic.
     *
     * @param sub the subscriber owning this DataReader
     * @param topic the multi-topic
     * @param qos the QoS settings for this DataReader
     * @param listener the listener
     * @param mask the event mask associated to the DataReader listener
     */
    DataReader(const dds::sub::Subscriber& sub,
               const ::dds::topic::MultiTopic<T>& topic,
               const dds::sub::qos::DataReaderQos& qos,
               dds::sub::DataReaderListener<T>* listener = NULL,
               const dds::core::status::StatusMask& mask = ::dds::core::status::StatusMask::none());

    #endif /* OMG_DDS_MULTI_TOPIC_SUPPORT */
public:
    virtual ~DataReader();


public:
    // == ReadState Management

    /**
     * Returns the default read-state (if not changed, it is set to
     * DataState::any()).
     */
    const dds::sub::status::DataState& default_filter_state();

    /**
     * Set the default state filter for read/take operations.
     *
     * @param state the state mask that will be used to read/take samples
     */
    DataReader& default_filter_state(const dds::sub::status::DataState& state);

    //== Streaming read/take

    /**
     * Manipulators are defined externally to make it possible to control whether the
     * streaming operators reads or takes.
     *   dr >> read >> loanedSamples;
     *   dr >> take >> loanedSamples;
     *
     */
    DataReader& operator >>(dds::sub::LoanedSamples<T>& ls);

    ManipulatorSelector
    operator >> (ManipulatorSelector & (manipulator)(ManipulatorSelector&));

    template <typename Functor>
    ManipulatorSelector
    operator >> (Functor f);


    ///////////////////////////////////////////////////////////////////////
public:
    //== Loan Read/Take API ==================================================

    /**
     * Read all samples using the default filter state. The memory used for
     * storing the sample may be loaned by the middleware thus allowing zero
     * copy operations.
     * Implementors should strive to make this read implementation
     * exception safe.
     */
    LoanedSamples<T> read();

    /**
     * Take all samples using the default filter state. The memory used for
     * storing the sample may be loaned by the middleware thus allowing zero
     * copy operations.
     * Implementors should strive to make this take implementation
     * exception safe.
     */
    LoanedSamples<T> take();

    //== Copy Read/Take API ==================================================

    // --- Forward Iterators: --- //

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
    uint32_t
    read(SamplesFWIterator sfit,
         uint32_t max_samples);

    /**
     * Take up to max_samples samples using the default filter state.
     * The samples are copied into the application provided container using the
     * forward iterator parameter.
     *
     * @param sfit samples forward iterator
     * @param max_samples the maximum number of samples to take
     * @return the number of taken samples
     */
    template <typename SamplesFWIterator>
    uint32_t
    take(SamplesFWIterator sfit,
         uint32_t max_samples);


    // --- Back-Inserting Iterators: --- //

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
    uint32_t
    read(SamplesBIIterator sbit);

    /**
     * Take all samples available in the reader cache samples using the default filter state.
     * The samples are copied into the application provided container using a
     * back-inserting iterator. Notice that as a consequence of using a back-inserting
     * iterator, this operation may allocate memory to resize the underlying container.
     *
     * @param sbit samples back-inserting iterator
     * @return the number of taken samples
     */
    template <typename SamplesBIIterator>
    uint32_t
    take(SamplesBIIterator sbit);
public:
    //========================================================================
    //== DSL Method for dealing with instances, content and status filters.

    /**
     * Get a Selector to perform complex data selections, such as
     * per-instance selection, content and status filtering.
     *
     * The selector can be used as follows:
     *
     * ~~~~~~~~~~~~~~~{.cpp}
     * reader.select()
     *        .instance(FooInstance)
     *        .state(dds::sub::status::DataState::any_data())
     *        .content(query)
     *        .take(sbit);
     * ~~~~~~~~~~~~~~~
     *
     * This shows how samples can be taken by selecting a specific instance,
     * then filtering by state and content.
     */
    Selector select();

    //========================================================================
    //== Instance Management
public:
    /**
     * This operation can be used to retrieve the instance key that corresponds
     * to an instance_handle. The operation will only fill the fields that form
     * the key inside the key_holder instance.
     * This operation may raise a BadParameter exception if the InstanceHandle
     * does not correspond to an existing data-object known to the DataWriter.
     * If the implementation is not able to check invalid handles, then the
     * result in this situation is unspecified.
     */
    dds::topic::TopicInstance<T> key_value(const dds::core::InstanceHandle& h);

    /**
     * This operation can be used to retrieve the instance key that corresponds
     * to an instance_handle. The operation will only fill the fields that form
     * the key inside the key_holder instance.
     * This operation may raise a BadParameter exception if the InstanceHandle
     * does not correspond to an existing data-object known to the DataWriter.
     * If the implementation is not able to check invalid handles, then the
     * result in this situation is unspecified.
     */
    T& key_value(T& sample, const dds::core::InstanceHandle& h);

    /**
     * This operation takes as a parameter an instance and returns a handle
     * that can be used in subsequent operations that accept an instance handle
     * as an argument. The instance parameter is only used for the purpose
     * of examining the fields that define the key. This operation does not
     * register the instance in question. If the instance has not been
     * previously registered, or if for any other reason the Service is unable
     * to provide an instance handle, the Service will return a TopicInstance
     * whose handle will be set to the HANDLE_NIL value.
     */
    const dds::core::InstanceHandle
    lookup_instance(const T& key) const;

    //==========================================================================
    // -- Entity Navigation API
public:
    /**
     * Get the TopicDescription associated with this reader.
     *
     * @return the TopicDescription
     */
    dds::topic::TopicDescription<DataType> topic_description() const;

    /**
     * Get the subscriber owning this reader.
     *
     * @return the Subscriber
     */
    const dds::sub::Subscriber& subscriber() const;

    /**
     * Set/Re-set the listener associated with this reader.
     *
     * @param listener the listener
     * @param event_mask the event mask associated with the listener
     */
    void listener(Listener* listener,
                  const dds::core::status::StatusMask& event_mask);

    /**
     * Get the listener associated with this reader.
     *
     * @return the Listener
     */
    Listener* listener() const;

    // -- Qos Getter/Setter

    /**
     * Get the QoS associated with this reader.
     *
     * @return the DataReaderQos
     */
    const dds::sub::qos::DataReaderQos&
    qos() const;

    /**
     * Set the QoS associated with this reader.
     *
     * @param qos the new QoS
     */
    void qos(const dds::sub::qos::DataReaderQos& qos);

    /**
     * Set the QoS associated with this reader.
     *
     * @param qos the new reader QoS
     */
    dds::sub::qos::DataReaderQos& operator << (const dds::sub::qos::DataReaderQos& qos);

    /**
     * Get the QoS associated with this reader.
     *
     * @param qos the current reader QoS
     * @return the DataReaderQos
     */
    const DataReader& operator >> (dds::sub::qos::DataReaderQos& qos) const;

    /**
     * Wait for historical data for a given amount of time. This time
     * may be set to infinite.
     *
     * @param timeout the time to wait for historical data
     */
    void
    wait_for_historical_data(const dds::core::Duration& timeout);

    //========================================================================
    //== Status API
    const dds::core::status::LivelinessChangedStatus&
    liveliness_changed_status();

    const dds::core::status::SampleRejectedStatus&
    sample_rejected_status();

    const dds::core::status::SampleLostStatus&
    sample_lost_status();

    const dds::core::status::RequestedDeadlineMissedStatus&
    requested_deadline_missed_status();

    const dds::core::status::RequestedIncompatibleQosStatus&
    requested_incompatible_qos_status();

    const dds::core::status::SubscriptionMatchedStatus&
    subscription_matched_status();

    /**
     * This function closes the entity and releases all resources associated with
     * DDS, such as threads, sockets, buffers, etc. Any attempt to invoke
     * functions on a closed entity will raise an exception.
     */
    virtual void close();

    /**
     * Indicates that references to this object may go out of scope but that
     * the application expects to look it up again later. Therefore, the
     * Service must consider this object to be still in use and may not
     * close it automatically.
     */
    virtual void retain();
};


#endif /* OMG_DDS_SUB_TDATA_READER_HPP_ */
