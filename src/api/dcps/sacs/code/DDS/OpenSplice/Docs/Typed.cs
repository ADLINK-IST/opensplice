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
 */


#if DOXYGEN_FOR_CS
//
// The above compile switch is never (and must never) be defined in normal compilation.
//
// This file is just used to add an example of a generated data type, its typesupport and
// the relating entities DataReader and DataWriter to the documentation.
//

/// @file

namespace Space
{
    /// <summary>
    /// Example of a generated data type, used for documentation.
    /// </summary>
    /// <remarks>
    /// This type would be created from the following idl:
    /// <code>
    /// module Space {
    ///     struct Foo {
    ///        long Bar;
    ///     };
    ///     #pragma keylist Foo Bar
    /// };
    /// </code>
    /// </remarks>
    public sealed class Foo
    {
        public int Bar;
    };



    /// <summary>
    /// A DataReader allows the application to access published sample data.
    /// </summary>
    /// <remarks>
    /// @note This is an example of a generated typed DataReader, used for documentation.
    ///
    /// A typed DataReader allows the application:
    /// - to declare the data it wishes to receive (i.e., make a subscription)
    /// - to access the data received by the attached Subscriber
    ///
    /// A DataReader refers to exactly one DDS.ITopicDescription (either a DDS.ITopic,
    /// a DDS.IContentFilteredTopic or a DDS.IMultiTopic) that identifies the samples
    /// to be read. The DDS.ITopic must exist prior to the DataReader creation.
    ///
    /// A DataReader is attached to exactly one DDS.ISubscriber which acts as a factory
    /// for it.
    ///
    /// The DataReader may give access to several instances of the data type, which
    /// are distinguished from each other by their key.
    ///
    /// The pre-processor generates from IDL type descriptions the application
    /// &lt;module&gt;.&lt;struct&gt;DataReader classes. In this example it is Space.FooDataReader.
    /// For each application data type that is used as DDS.ITopic
    /// data type, a typed class &lt;module&gt;.&lt;struct&gt;DataReader is derived from the
    /// DDS.IDataReader class.
    ///
    /// @note Class hierarchy in the documentation has been simplified for clearity purposes.
    ///
    /// @anchor anchor_foo_bar_datareader_example
    /// <i><b>Example</b></i><br>
    /// The memory used for storing the sample may be loaned by the middleware thus
    /// allowing less copy operations. This also means that the buffer loans have to
    /// be returned to the middleware.
    /// <code>
    /// /* Simplest creation of a typed datareader.
    ///  * Defaults are used and possible errors are ignored. */
    ///
    /// /* Prepare Domain. */
    /// DDS.DomainParticipantFactory factory = DDS.DomainParticipantFactory.Instance;
    /// DDS.IDomainParticipant participant = factory.CreateParticipant(DDS.DomainId.Default);
    ///
    /// /* Add topic data type to the system. */
    /// DDS.ITypeSupport typesupport = new Space.FooTypeSupport();
    /// DDS.ReturnCode retcode = typesupport.RegisterType(participant, "Space.Foo");
    ///
    /// DDS.ITopic topic = participant.CreateTopic("SpaceFooTopic", "Space.Foo");
    ///
    /// /* Create typed datareader. */
    /// DDS.ISubscriber subscriber = participant.CreateSubscriber();
    /// Space.FooDataReader reader = (Space.FooDataReader)subscriber.CreateDataReader(topic);
    ///
    /// /* Read data (using defaults like unlimited samples). */
    /// Space.Foo[]         dataList = null;
    /// DDS.SampleInfo[]  infoList = null;
    /// retcode = reader.Read(ref dataList, ref infoList);
    /// if (retcode == DDS.ReturnCode.Ok) {
    ///     for (int i = 0; i < dataList.Length; i++) {
    ///         Space.Foo data = dataList[i];
    ///         DDS.SampleInfo info = infoList[i]
    ///         /* Use data and info. */
    ///     }
    ///     reader.ReturnLoan(ref dataList, ref infoList);
    /// }
    /// </code>
    /// </remarks>
    /// @see @ref DCPS_Modules_Subscription "Subscription concept"
    /// @see @ref DCPS_Modules_Subscription_DataReader "DataReader concept"
    public class FooDataReader : DDS.IDataReader
    {
        /// <summary>
        /// This operation reads an array of typed samples from the DataReader.
        /// </summary>
        /// <remarks>
        /// The data is put into the given dataValues array, while meta data is put into
        /// the given sampleInfos array.<br>
        /// The given arrays will be resized to fit the number of read samples.
        ///
        /// Reading a sample will just keep the sample in the DataReaders' buffer.
        /// The sample can be replaced by a write action depending on QoS history
        /// policy (DDS.HistoryQosPolicy) of that DataReader.<br>
        /// Taking the sample will remove the sample from the DataReaders' buffer.
        ///
        /// @anchor anchor_foo_bar_datareader_data_sequence
        /// <b><i>Data Sequence</i></b><br>
        /// On output, the dataValues and sampleInfos arrays are of the same length
        /// and are in an one-to-one correspondence. Each DDS.SampleInfo
        /// object provides information, such as the SourceTimestamp, the
        /// SampleState, ViewState, and InstanceState, etc., about the matching
        /// sample.
        ///
        /// Some elements in the returned sequence may not have valid data: the ValidData
        /// field in the DDS.SampleInfo indicates whether the corresponding data value contains
        /// any meaningful data. If not, the data value is just a ‘dummy’ sample for which only
        /// the keyfields have been assigned. It is used to accompany the DDS.SampleInfo that
        /// communicates a change in the InstanceState of an instance for which there is
        /// no ‘real’ sample available.
        ///
        /// For example, when an application always ‘takes’ all available samples of a
        /// particular instance, there is no sample available to report the disposal of that
        /// instance. In such a case the DataReader will insert a dummy sample into the
        /// dataValues array to accompany the DDS.SampleInfo element in the sampleInfos
        /// sequence that communicates the disposal of the instance.
        ///
        /// The act of reading a sample sets its SampleState to DDS.SampleStateKind Read state.
        /// If the sample belongs to the most recent generation of the instance, it also sets
        /// the ViewState of the instance to DDS.ViewStateKind NotNew state. It does not affect
        /// the SampleState of the instance.
        ///
        /// @anchor anchor_foo_bar_datareader_destination_order
        /// <b><i>Destination Order</i></b><br>
        /// In any case, the relative order between the samples of one instance is consistent
        /// with the DestinationOrderQosPolicy of the Subscriber.
        /// - When the DDS.DestinationOrderQosPolicykind is ByReceptionTimestampDestinationOrderQos,
        ///   the samples belonging to the same instances will appear in the relative order in which
        ///   they were received (FIFO).
        /// - When the DDS.DestinationOrderQosPolicykind is BySourceTimestampDestinationOrderQos,
        ///   the samples belonging to the same instances will appear in the relative order implied
        ///   by the BySourceTimestampDestinationOrderQos.
        ///
        /// @see Simple DataReader creation and read @ref anchor_foo_bar_datareader_example "example".
        /// </remarks>
        /// <param name="dataValues">
        ///     Array (will be resized) that'll contain the samples.</param>
        /// <param name="sampleInfos">
        ///     Array (will be resized) that'll contain the samples meta information.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - Data is available.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The DataReader has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS has ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode NotEnabled - The DataReader has not yet been enabled.</item>
        /// <item>DDS.ReturnCode NoData - No samples that meet the constraints are available.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode Read(
            ref Foo[] dataValues,
            ref DDS.SampleInfo[] sampleInfos)
        {
            return DDS.ReturnCode.Ok;
        }

        /// <summary>
        /// This operation reads an array of typed samples from the DataReader.
        /// </summary>
        /// <remarks>
        /// The data is put into the given dataValues array, while meta data is put into
        /// the given sampleInfos array.<br>
        /// The given arrays will be resized to fit the number of read samples.
        ///
        /// It will only read a maximum number of samples in one call.
        ///
        /// Reading a sample will just keep the sample in the DataReaders' buffer.
        /// The sample can be replaced by a write action depending on QoS history
        /// policy (DDS.HistoryQosPolicy) of that DataReader.<br>
        /// Taking the sample will remove the sample from the DataReaders' buffer.
        ///
        /// @see Simple DataReader creation and read @ref anchor_foo_bar_datareader_example "example".
        /// @see Data sequence meta-info @ref anchor_foo_bar_datareader_data_sequence "information".
        /// @see Destination order @ref anchor_foo_bar_datareader_destination_order "information".
        /// </remarks>
        /// <param name="dataValues">
        ///     Array (will be resized) that'll contain the samples.</param>
        /// <param name="sampleInfos">
        ///     Array (will be resized) that'll contain the samples meta information.</param>
        /// <param name="maxSamples">
        ///     The maximum count of samples that will be read.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - Data is available.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The DataReader has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS has ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode NotEnabled - The DataReader has not yet been enabled.</item>
        /// <item>DDS.ReturnCode NoData - No samples that meet the constraints are available.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode Read(
            ref Foo[] dataValues,
            ref DDS.SampleInfo[] sampleInfos,
            int maxSamples)
        {
            return DDS.ReturnCode.Ok;
        }

        /// <summary>
        /// This operation reads an array of typed samples from the DataReader.
        /// </summary>
        /// <remarks>
        /// The data is put into the given dataValues array, while meta data is put into
        /// the given sampleInfos array.<br>
        /// The given arrays will be resized to fit the number of read samples.
        ///
        /// It will only read samples that have the given states.
        ///
        /// Reading a sample will just keep the sample in the DataReaders' buffer.
        /// The sample can be replaced by a write action depending on QoS history
        /// policy (DDS.HistoryQosPolicy) of that DataReader.<br>
        /// Taking the sample will remove the sample from the DataReaders' buffer.
        ///
        /// @anchor anchor_foo_bar_datareader_state_masks
        /// <b><i>State Masks</i></b><br>
        /// The read operation depends on a selection of the samples by using three masks:
        /// - DDS.SampleStateKind sampleStates is the mask, which selects only those samples with
        ///   the desired sample states Read, NotRead or both.
        /// - DDS.ViewStateKind viewStates is the mask, which selects only those samples with the
        ///   desired view states New, NotNew or both.
        /// - DDS.InstanceStateKind instanceStates is the mask, which selects only those samples
        ///   with the desired instance states Alive, NotAliveDisposed, NotAliveNoWriters or a
        ///   combination of these.
        ///
        /// @see Simple DataReader creation and read @ref anchor_foo_bar_datareader_example "example".
        /// @see Data sequence meta-info @ref anchor_foo_bar_datareader_data_sequence "information".
        /// @see Destination order @ref anchor_foo_bar_datareader_destination_order "information".
        /// </remarks>
        /// <param name="dataValues">
        ///     Array (will be resized) that'll contain the samples.</param>
        /// <param name="sampleInfos">
        ///     Array (will be resized) that'll contain the samples meta information.</param>
        /// <param name="sampleStates">
        ///     A mask, which selects only those samples with the desired sample states.</param>
        /// <param name="viewStates">
        ///     A mask, which selects only those samples with the desired view states.</param>
        /// <param name="instanceStates">
        ///     A mask, which selects only those samples with the desired instance states.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - Data is available.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The DataReader has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS has ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode NotEnabled - The DataReader has not yet been enabled.</item>
        /// <item>DDS.ReturnCode NoData - No samples that meet the constraints are available.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode Read(
            ref Foo[] dataValues,
            ref DDS.SampleInfo[] sampleInfos,
            DDS.SampleStateKind sampleStates,
            DDS.ViewStateKind viewStates,
            DDS.InstanceStateKind instanceStates)
        {
            return DDS.ReturnCode.Ok;
        }

        /// <summary>
        /// This operation reads an array of typed samples from the DataReader.
        /// </summary>
        /// <remarks>
        /// The data is put into the given dataValues array, while meta data is put into
        /// the given sampleInfos array.<br>
        /// The given arrays will be resized to fit the number of read samples.
        ///
        /// It will only read a maximum number of samples in one call.
        ///
        /// It will also only read samples that have the given states.
        ///
        /// Reading a sample will just keep the sample in the DataReaders' buffer.
        /// The sample can be replaced by a write action depending on QoS history
        /// policy (DDS.HistoryQosPolicy) of that DataReader.<br>
        /// Taking the sample will remove the sample from the DataReaders' buffer.
        ///
        /// @see Simple DataReader creation and read @ref anchor_foo_bar_datareader_example "example".
        /// @see Data sequence meta-info @ref anchor_foo_bar_datareader_data_sequence "information".
        /// @see Destination order @ref anchor_foo_bar_datareader_destination_order "information".
        /// @see State masks @ref anchor_foo_bar_datareader_state_masks "information".
        /// </remarks>
        /// <param name="dataValues">
        ///     Array (will be resized) that'll contain the samples.</param>
        /// <param name="sampleInfos">
        ///     Array (will be resized) that'll contain the samples meta information.</param>
        /// <param name="maxSamples">
        ///     The maximum count of samples that will be read.</param>
        /// <param name="sampleStates">
        ///     A mask, which selects only those samples with the desired sample states.</param>
        /// <param name="viewStates">
        ///     A mask, which selects only those samples with the desired view states.</param>
        /// <param name="instanceStates">
        ///     A mask, which selects only those samples with the desired instance states.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - Data is available.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The DataReader has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS has ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode NotEnabled - The DataReader has not yet been enabled.</item>
        /// <item>DDS.ReturnCode NoData - No samples that meet the constraints are available.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode Read(
                ref Foo[] dataValues,
                ref DDS.SampleInfo[] sampleInfos,
                int maxSamples,
                DDS.SampleStateKind sampleStates,
                DDS.ViewStateKind viewStates,
                DDS.InstanceStateKind instanceStates)
        {
            return DDS.ReturnCode.Ok;
        }

        /// <summary>
        /// This operation takes an array of typed samples from the DataReader.
        /// </summary>
        /// <remarks>
        /// The data is put into the given dataValues array, while meta data is put into
        /// the given sampleInfos array.<br>
        /// The given arrays will be resized to fit the number of read samples.
        ///
        /// Reading a sample will just keep the sample in the DataReaders' buffer.<br>
        /// Taking the sample will remove the sample from the DataReaders' buffer.
        ///
        /// @see Simple DataReader creation and read @ref anchor_foo_bar_datareader_example "example".
        /// @see Data sequence meta-info @ref anchor_foo_bar_datareader_data_sequence "information".
        /// @see Destination order @ref anchor_foo_bar_datareader_destination_order "information".
        /// @see State masks @ref anchor_foo_bar_datareader_state_masks "information".
        /// </remarks>
        /// <param name="dataValues">
        ///     Array (will be resized) that'll contain the samples.</param>
        /// <param name="sampleInfos">
        ///     Array (will be resized) that'll contain the samples meta information.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - Data is available.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The DataReader has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS has ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode NotEnabled - The DataReader has not yet been enabled.</item>
        /// <item>DDS.ReturnCode NoData - No samples that meet the constraints are available.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode Take(
            ref Foo[] dataValues,
            ref DDS.SampleInfo[] sampleInfos)
        {
            return DDS.ReturnCode.Ok;
        }

        /// <summary>
        /// This operation takes an array of typed samples from the DataReader.
        /// </summary>
        /// <remarks>
        /// The data is put into the given dataValues array, while meta data is put into
        /// the given sampleInfos array.<br>
        /// The given arrays will be resized to fit the number of read samples.
        ///
        /// It will only take a maximum number of samples in one call.
        ///
        /// Reading a sample will just keep the sample in the DataReaders' buffer.<br>
        /// Taking the sample will remove the sample from the DataReaders' buffer.
        ///
        /// @see Simple DataReader creation and read @ref anchor_foo_bar_datareader_example "example".
        /// @see Data sequence meta-info @ref anchor_foo_bar_datareader_data_sequence "information".
        /// @see Destination order @ref anchor_foo_bar_datareader_destination_order "information".
        /// @see State masks @ref anchor_foo_bar_datareader_state_masks "information".
        /// </remarks>
        /// <param name="dataValues">
        ///     Array (will be resized) that'll contain the samples.</param>
        /// <param name="sampleInfos">
        ///     Array (will be resized) that'll contain the samples meta information.</param>
        /// <param name="maxSamples">
        ///     The maximum count of samples that will be taken.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - Data is available.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The DataReader has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS has ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode NotEnabled - The DataReader has not yet been enabled.</item>
        /// <item>DDS.ReturnCode NoData - No samples that meet the constraints are available.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode Take(
            ref Foo[] dataValues,
            ref DDS.SampleInfo[] sampleInfos,
            int maxSamples)
        {
            return DDS.ReturnCode.Ok;
        }

        /// <summary>
        /// This operation takes an array of typed samples from the DataReader.
        /// </summary>
        /// <remarks>
        /// The data is put into the given dataValues array, while meta data is put into
        /// the given sampleInfos array.<br>
        /// The given arrays will be resized to fit the number of read samples.
        ///
        /// It will only take samples that have the given states.
        ///
        /// Reading a sample will just keep the sample in the DataReaders' buffer.<br>
        /// Taking the sample will remove the sample from the DataReaders' buffer.
        ///
        /// @see Simple DataReader creation and read @ref anchor_foo_bar_datareader_example "example".
        /// @see Data sequence meta-info @ref anchor_foo_bar_datareader_data_sequence "information".
        /// @see Destination order @ref anchor_foo_bar_datareader_destination_order "information".
        /// @see State masks @ref anchor_foo_bar_datareader_state_masks "information".
        /// </remarks>
        /// <param name="dataValues">
        ///     Array (will be resized) that'll contain the samples.</param>
        /// <param name="sampleInfos">
        ///     Array (will be resized) that'll contain the samples meta information.</param>
        /// <param name="sampleStates">
        ///     A mask, which selects only those samples with the desired sample states.</param>
        /// <param name="viewStates">
        ///     A mask, which selects only those samples with the desired view states.</param>
        /// <param name="instanceStates">
        ///     A mask, which selects only those samples with the desired instance states.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - Data is available.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The DataReader has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS has ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode NotEnabled - The DataReader has not yet been enabled.</item>
        /// <item>DDS.ReturnCode NoData - No samples that meet the constraints are available.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode Take(
            ref Foo[] dataValues,
            ref DDS.SampleInfo[] sampleInfos,
            DDS.SampleStateKind sampleStates,
            DDS.ViewStateKind viewStates,
            DDS.InstanceStateKind instanceStates)
        {
            return DDS.ReturnCode.Ok;
        }

        /// <summary>
        /// This operation takes an array of typed samples from the DataReader.
        /// </summary>
        /// <remarks>
        /// The data is put into the given dataValues array, while meta data is put into
        /// the given sampleInfos array.<br>
        /// The given arrays will be resized to fit the number of read samples.
        ///
        /// It will only take a maximum number of samples in one call.
        ///
        /// It will also only take samples that have the given states.
        ///
        /// Reading a sample will just keep the sample in the DataReaders' buffer.<br>
        /// Taking the sample will remove the sample from the DataReaders' buffer.
        ///
        /// @see Simple DataReader creation and read @ref anchor_foo_bar_datareader_example "example".
        /// @see Data sequence meta-info @ref anchor_foo_bar_datareader_data_sequence "information".
        /// @see Destination order @ref anchor_foo_bar_datareader_destination_order "information".
        /// @see State masks @ref anchor_foo_bar_datareader_state_masks "information".
        /// </remarks>
        /// <param name="dataValues">
        ///     Array (will be resized) that'll contain the samples.</param>
        /// <param name="sampleInfos">
        ///     Array (will be resized) that'll contain the samples meta information.</param>
        /// <param name="maxSamples">
        ///     The maximum count of samples that will be taken.</param>
        /// <param name="sampleStates">
        ///     A mask, which selects only those samples with the desired sample states.</param>
        /// <param name="viewStates">
        ///     A mask, which selects only those samples with the desired view states.</param>
        /// <param name="instanceStates">
        ///     A mask, which selects only those samples with the desired instance states.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - Data is available.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The DataReader has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS has ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode NotEnabled - The DataReader has not yet been enabled.</item>
        /// <item>DDS.ReturnCode NoData - No samples that meet the constraints are available.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode Take(
                ref Foo[] dataValues,
                ref DDS.SampleInfo[] sampleInfos,
                int maxSamples,
                DDS.SampleStateKind sampleStates,
                DDS.ViewStateKind viewStates,
                DDS.InstanceStateKind instanceStates)
        {
            return DDS.ReturnCode.Ok;
        }

        /// <summary>
        /// This operation reads an array of typed samples from the DataReader.
        /// </summary>
        /// <remarks>
        /// The data that will be read is filtered by a DDS.IReadCondition or
        /// DDS.IQueryCondition.
        ///
        /// The data is put into the given dataValues array, while meta data is put into
        /// the given sampleInfos array.<br>
        /// The given arrays will be resized to fit the number of read samples.
        ///
        /// Reading a sample will just keep the sample in the DataReaders' buffer.
        /// The sample can be replaced by a write action depending on QoS history
        /// policy (DDS.HistoryQosPolicy) of that DataReader.<br>
        /// Taking the sample will remove the sample from the DataReaders' buffer.
        ///
        /// @see Simple DataReader creation and read @ref anchor_foo_bar_datareader_example "example".
        /// @see Data sequence meta-info @ref anchor_foo_bar_datareader_data_sequence "information".
        /// @see Destination order @ref anchor_foo_bar_datareader_destination_order "information".
        /// @see State masks @ref anchor_foo_bar_datareader_state_masks "information".
        /// </remarks>
        /// <param name="dataValues">
        ///     Array (will be resized) that'll contain the samples.</param>
        /// <param name="sampleInfos">
        ///     Array (will be resized) that'll contain the samples meta information.</param>
        /// <param name="readCondition">
        ///     A DDS.IReadCondition or DDS.IQueryCondition which filters the data before
        ///     it is returned by the read operation.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - Data is available.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The DataReader has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS has ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode NotEnabled - The DataReader has not yet been enabled.</item>
        /// <item>DDS.ReturnCode NoData - No samples that meet the constraints are available.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode ReadWithCondition(
            ref Foo[] dataValues,
            ref DDS.SampleInfo[] sampleInfos,
            DDS.IReadCondition readCondition)
        {
            return DDS.ReturnCode.Ok;
        }

        /// <summary>
        /// This operation reads an array of typed samples from the DataReader.
        /// </summary>
        /// <remarks>
        /// The data that will be read is filtered by a DDS.IReadCondition or
        /// DDS.IQueryCondition.
        ///
        /// The data is put into the given dataValues array, while meta data is put into
        /// the given sampleInfos array.<br>
        /// The given arrays will be resized to fit the number of read samples.
        ///
        /// It will only read a maximum number of samples in one call.
        ///
        /// Reading a sample will just keep the sample in the DataReaders' buffer.
        /// The sample can be replaced by a write action depending on QoS history
        /// policy (DDS.HistoryQosPolicy) of that DataReader.<br>
        /// Taking the sample will remove the sample from the DataReaders' buffer.
        ///
        /// @see Simple DataReader creation and read @ref anchor_foo_bar_datareader_example "example".
        /// @see Data sequence meta-info @ref anchor_foo_bar_datareader_data_sequence "information".
        /// @see Destination order @ref anchor_foo_bar_datareader_destination_order "information".
        /// @see State masks @ref anchor_foo_bar_datareader_state_masks "information".
        /// </remarks>
        /// <param name="dataValues">
        ///     Array (will be resized) that'll contain the samples.</param>
        /// <param name="sampleInfos">
        ///     Array (will be resized) that'll contain the samples meta information.</param>
        /// <param name="maxSamples">
        ///     The maximum count of samples that will be read.</param>
        /// <param name="readCondition">
        ///     A DDS.IReadCondition or DDS.IQueryCondition which filters the data before
        ///     it is returned by the read operation.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - Data is available.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The DataReader has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS has ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode NotEnabled - The DataReader has not yet been enabled.</item>
        /// <item>DDS.ReturnCode NoData - No samples that meet the constraints are available.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode ReadWithCondition(
                ref Foo[] dataValues,
                ref DDS.SampleInfo[] sampleInfos,
                int maxSamples,
                DDS.IReadCondition readCondition)
        {
            return DDS.ReturnCode.Ok;
        }

        /// <summary>
        /// This operation takes an array of typed samples from the DataReader.
        /// </summary>
        /// <remarks>
        /// The data that will be taken is filtered by a DDS.IReadCondition or
        /// DDS.IQueryCondition.
        ///
        /// The data is put into the given dataValues array, while meta data is put into
        /// the given sampleInfos array.<br>
        /// The given arrays will be resized to fit the number of read samples.
        ///
        /// Reading a sample will just keep the sample in the DataReaders' buffer.<br>
        /// Taking the sample will remove the sample from the DataReaders' buffer.
        ///
        /// @see Simple DataReader creation and read @ref anchor_foo_bar_datareader_example "example".
        /// @see Data sequence meta-info @ref anchor_foo_bar_datareader_data_sequence "information".
        /// @see Destination order @ref anchor_foo_bar_datareader_destination_order "information".
        /// @see State masks @ref anchor_foo_bar_datareader_state_masks "information".
        /// </remarks>
        /// <param name="dataValues">
        ///     Array (will be resized) that'll contain the samples.</param>
        /// <param name="sampleInfos">
        ///     Array (will be resized) that'll contain the samples meta information.</param>
        /// <param name="readCondition">
        ///     A DDS.IReadCondition or DDS.IQueryCondition which filters the data before
        ///     it is returned by the take operation.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - Data is available.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The DataReader has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS has ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode NotEnabled - The DataReader has not yet been enabled.</item>
        /// <item>DDS.ReturnCode NoData - No samples that meet the constraints are available.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode TakeWithCondition(
            ref Foo[] dataValues,
            ref DDS.SampleInfo[] sampleInfos,
            DDS.IReadCondition readCondition)
        {
            return DDS.ReturnCode.Ok;
        }

        /// <summary>
        /// This operation takes an array of typed samples from the DataReader.
        /// </summary>
        /// <remarks>
        /// The data that will be taken is filtered by a DDS.IReadCondition or
        /// DDS.IQueryCondition.
        ///
        /// The data is put into the given dataValues array, while meta data is put into
        /// the given sampleInfos array.<br>
        /// The given arrays will be resized to fit the number of read samples.
        ///
        /// It will only take a maximum number of samples in one call.
        ///
        /// Reading a sample will just keep the sample in the DataReaders' buffer.<br>
        /// Taking the sample will remove the sample from the DataReaders' buffer.
        ///
        /// @see Simple DataReader creation and read @ref anchor_foo_bar_datareader_example "example".
        /// @see Data sequence meta-info @ref anchor_foo_bar_datareader_data_sequence "information".
        /// @see Destination order @ref anchor_foo_bar_datareader_destination_order "information".
        /// @see State masks @ref anchor_foo_bar_datareader_state_masks "information".
        /// </remarks>
        /// <param name="dataValues">
        ///     Array (will be resized) that'll contain the samples.</param>
        /// <param name="sampleInfos">
        ///     Array (will be resized) that'll contain the samples meta information.</param>
        /// <param name="maxSamples">
        ///     The maximum count of samples that will be taken.</param>
        /// <param name="readCondition">
        ///     A DDS.IReadCondition or DDS.IQueryCondition which filters the data before
        ///     it is returned by the take operation.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - Data is available.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The DataReader has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS has ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode NotEnabled - The DataReader has not yet been enabled.</item>
        /// <item>DDS.ReturnCode NoData - No samples that meet the constraints are available.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode TakeWithCondition(
                ref Foo[] dataValues,
                ref DDS.SampleInfo[] sampleInfos,
                int maxSamples,
                DDS.IReadCondition readCondition)
        {
            return DDS.ReturnCode.Ok;
        }

        /// <summary>
        /// This operation reads an array of typed samples from the DataReader.
        /// </summary>
        /// <remarks>
        /// @note This operation is not yet implemented. It is scheduled for a future release.
        ///
        /// The data is put into the given dataValues array, while meta data is put into
        /// the given sampleInfos array.<br>
        /// The given arrays will be resized to fit the number of read samples.
        ///
        /// Reading a sample will just keep the sample in the DataReaders' buffer.
        /// The sample can be replaced by a write action depending on QoS history
        /// policy (DDS.HistoryQosPolicy) of that DataReader.<br>
        /// Taking the sample will remove the sample from the DataReaders' buffer.
        ///
        /// @see Simple DataReader creation and read @ref anchor_foo_bar_datareader_example "example".
        /// @see Data sequence meta-info @ref anchor_foo_bar_datareader_data_sequence "information".
        /// @see Destination order @ref anchor_foo_bar_datareader_destination_order "information".
        /// @see State masks @ref anchor_foo_bar_datareader_state_masks "information".
        /// </remarks>
        /// <param name="dataValue">Sample data.</param>
        /// <param name="sampleInfo">Sample info.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Unsupported - Not supported yet.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode ReadNextSample(
                ref Foo dataValue,
                ref DDS.SampleInfo sampleInfo)
        {
            return DDS.ReturnCode.Unsupported;
        }

        /// <summary>
        /// This operation takes an array of typed samples from the DataReader.
        /// </summary>
        /// <remarks>
        /// @note This operation is not yet implemented. It is scheduled for a future release.
        ///
        /// The data is put into the given dataValues array, while meta data is put into
        /// the given sampleInfos array.<br>
        /// The given arrays will be resized to fit the number of read samples.
        ///
        /// Reading a sample will just keep the sample in the DataReaders' buffer.<br>
        /// Taking the sample will remove the sample from the DataReaders' buffer.
        ///
        /// @see Simple DataReader creation and read @ref anchor_foo_bar_datareader_example "example".
        /// @see Data sequence meta-info @ref anchor_foo_bar_datareader_data_sequence "information".
        /// @see Destination order @ref anchor_foo_bar_datareader_destination_order "information".
        /// @see State masks @ref anchor_foo_bar_datareader_state_masks "information".
        /// </remarks>
        /// <param name="dataValue">Sample data.</param>
        /// <param name="sampleInfo">Sample info.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Unsupported - Not supported yet.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode TakeNextSample(
                ref Foo dataValue,
                ref DDS.SampleInfo sampleInfo)
        {
            return DDS.ReturnCode.Unsupported;
        }

        /// <summary>
        /// This operation reads an array of typed samples from the DataReader.
        /// </summary>
        /// <remarks>
        /// The data is put into the given dataValues array, while meta data is put into
        /// the given sampleInfos array.<br>
        /// The given arrays will be resized to fit the number of read samples.
        ///
        /// It will only read samples of the given instance.
        ///
        /// Reading a sample will just keep the sample in the DataReaders' buffer.
        /// The sample can be replaced by a write action depending on QoS history
        /// policy (DDS.HistoryQosPolicy) of that DataReader.<br>
        /// Taking the sample will remove the sample from the DataReaders' buffer.
        ///
        /// @see Simple DataReader creation and read @ref anchor_foo_bar_datareader_example "example".
        /// @see Data sequence meta-info @ref anchor_foo_bar_datareader_data_sequence "information".
        /// @see Destination order @ref anchor_foo_bar_datareader_destination_order "information".
        /// @see State masks @ref anchor_foo_bar_datareader_state_masks "information".
        /// </remarks>
        /// <param name="dataValues">
        ///     Array (will be resized) that'll contain the samples.</param>
        /// <param name="sampleInfos">
        ///     Array (will be resized) that'll contain the samples meta information.</param>
        /// <param name="instanceHandle">
        ///     The single instance, which the samples belong to.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - Data is available.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The DataReader has already been deleted.</item>
        /// <item>DDS.ReturnCode BadParameter - The parameter instanceHandle is not a valid handle.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS has ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode PreconditionNotMet - The instanceHandle is DDS.InstanceHandle.Nil.</item>
        /// <item>DDS.ReturnCode NotEnabled - The DataReader has not yet been enabled.</item>
        /// <item>DDS.ReturnCode NoData - No samples that meet the constraints are available.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode ReadInstance(
            ref Foo[] dataValues,
            ref DDS.SampleInfo[] sampleInfos,
            DDS.InstanceHandle instanceHandle)
        {
            return DDS.ReturnCode.Ok;
        }

        /// <summary>
        /// This operation reads an array of typed samples from the DataReader.
        /// </summary>
        /// <remarks>
        /// The data is put into the given dataValues array, while meta data is put into
        /// the given sampleInfos array.<br>
        /// The given arrays will be resized to fit the number of read samples.
        ///
        /// It will only read samples of the given instance.
        ///
        /// It will also only read a maximum number of samples in one call.
        ///
        /// Reading a sample will just keep the sample in the DataReaders' buffer.
        /// The sample can be replaced by a write action depending on QoS history
        /// policy (DDS.HistoryQosPolicy) of that DataReader.<br>
        /// Taking the sample will remove the sample from the DataReaders' buffer.
        ///
        /// @see Simple DataReader creation and read @ref anchor_foo_bar_datareader_example "example".
        /// @see Data sequence meta-info @ref anchor_foo_bar_datareader_data_sequence "information".
        /// @see Destination order @ref anchor_foo_bar_datareader_destination_order "information".
        /// @see State masks @ref anchor_foo_bar_datareader_state_masks "information".
        /// </remarks>
        /// <param name="dataValues">
        ///     Array (will be resized) that'll contain the samples.</param>
        /// <param name="sampleInfos">
        ///     Array (will be resized) that'll contain the samples meta information.</param>
        /// <param name="maxSamples">
        ///     The maximum count of samples that will be read.</param>
        /// <param name="instanceHandle">
        ///     The single instance, which the samples belong to.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - Data is available.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The DataReader has already been deleted.</item>
        /// <item>DDS.ReturnCode BadParameter - The parameter instanceHandle is not a valid handle.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS has ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode PreconditionNotMet - The instanceHandle is DDS.InstanceHandle.Nil.</item>
        /// <item>DDS.ReturnCode NotEnabled - The DataReader has not yet been enabled.</item>
        /// <item>DDS.ReturnCode NoData - No samples that meet the constraints are available.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode ReadInstance(
            ref Foo[] dataValues,
            ref DDS.SampleInfo[] sampleInfos,
            int maxSamples,
            DDS.InstanceHandle instanceHandle)
        {
            return DDS.ReturnCode.Ok;
        }

        /// <summary>
        /// This operation reads an array of typed samples from the DataReader.
        /// </summary>
        /// <remarks>
        /// The data is put into the given dataValues array, while meta data is put into
        /// the given sampleInfos array.<br>
        /// The given arrays will be resized to fit the number of read samples.
        ///
        /// It will only read samples of the given instance.
        ///
        /// It will also only read a maximum number of samples in one call.
        ///
        /// It will also only read samples that have the given states.
        ///
        /// Reading a sample will just keep the sample in the DataReaders' buffer.
        /// The sample can be replaced by a write action depending on QoS history
        /// policy (DDS.HistoryQosPolicy) of that DataReader.<br>
        /// Taking the sample will remove the sample from the DataReaders' buffer.
        ///
        /// @see Simple DataReader creation and read @ref anchor_foo_bar_datareader_example "example".
        /// @see Data sequence meta-info @ref anchor_foo_bar_datareader_data_sequence "information".
        /// @see Destination order @ref anchor_foo_bar_datareader_destination_order "information".
        /// @see State masks @ref anchor_foo_bar_datareader_state_masks "information".
        /// </remarks>
        /// <param name="dataValues">
        ///     Array (will be resized) that'll contain the samples.</param>
        /// <param name="sampleInfos">
        ///     Array (will be resized) that'll contain the samples meta information.</param>
        /// <param name="maxSamples">
        ///     The maximum count of samples that will be read.</param>
        /// <param name="instanceHandle">
        ///     The single instance, which the samples belong to.</param>
        /// <param name="sampleStates">
        ///     A mask, which selects only those samples with the desired sample states.</param>
        /// <param name="viewStates">
        ///     A mask, which selects only those samples with the desired view states.</param>
        /// <param name="instanceStates">
        ///     A mask, which selects only those samples with the desired instance states.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - Data is available.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The DataReader has already been deleted.</item>
        /// <item>DDS.ReturnCode BadParameter - The parameter instanceHandle is not a valid handle.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS has ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode PreconditionNotMet - The instanceHandle is DDS.InstanceHandle.Nil.</item>
        /// <item>DDS.ReturnCode NotEnabled - The DataReader has not yet been enabled.</item>
        /// <item>DDS.ReturnCode NoData - No samples that meet the constraints are available.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode ReadInstance(
                ref Foo[] dataValues,
                ref DDS.SampleInfo[] sampleInfos,
                int maxSamples,
                DDS.InstanceHandle instanceHandle,
                DDS.SampleStateKind sampleStates,
                DDS.ViewStateKind viewStates,
                DDS.InstanceStateKind instanceStates)
        {
            return DDS.ReturnCode.Ok;
        }

        /// <summary>
        /// This operation takes an array of typed samples from the DataReader.
        /// </summary>
        /// <remarks>
        /// The data is put into the given dataValues array, while meta data is put into
        /// the given sampleInfos array.<br>
        /// The given arrays will be resized to fit the number of read samples.
        ///
        /// It will only take samples of the given instance.
        ///
        /// Reading a sample will just keep the sample in the DataReaders' buffer.<br>
        /// Taking the sample will remove the sample from the DataReaders' buffer.
        ///
        /// @see Simple DataReader creation and read @ref anchor_foo_bar_datareader_example "example".
        /// @see Data sequence meta-info @ref anchor_foo_bar_datareader_data_sequence "information".
        /// @see Destination order @ref anchor_foo_bar_datareader_destination_order "information".
        /// @see State masks @ref anchor_foo_bar_datareader_state_masks "information".
        /// </remarks>
        /// <param name="dataValues">
        ///     Array (will be resized) that'll contain the samples.</param>
        /// <param name="sampleInfos">
        ///     Array (will be resized) that'll contain the samples meta information.</param>
        /// <param name="instanceHandle">
        ///     The single instance, which the samples belong to.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - Data is available.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The DataReader has already been deleted.</item>
        /// <item>DDS.ReturnCode BadParameter - The parameter instanceHandle is not a valid handle.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS has ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode PreconditionNotMet - The instanceHandle is DDS.InstanceHandle.Nil.</item>
        /// <item>DDS.ReturnCode NotEnabled - The DataReader has not yet been enabled.</item>
        /// <item>DDS.ReturnCode NoData - No samples that meet the constraints are available.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode TakeInstance(
            ref Foo[] dataValues,
            ref DDS.SampleInfo[] sampleInfos,
            DDS.InstanceHandle instanceHandle)
        {
            return DDS.ReturnCode.Ok;
        }

        /// <summary>
        /// This operation takes an array of typed samples from the DataReader.
        /// </summary>
        /// <remarks>
        /// The data is put into the given dataValues array, while meta data is put into
        /// the given sampleInfos array.<br>
        /// The given arrays will be resized to fit the number of read samples.
        ///
        /// It will only take samples of the given instance.
        ///
        /// It will also only take a maximum number of samples in one call.
        ///
        /// Reading a sample will just keep the sample in the DataReaders' buffer.<br>
        /// Taking the sample will remove the sample from the DataReaders' buffer.
        ///
        /// @see Simple DataReader creation and read @ref anchor_foo_bar_datareader_example "example".
        /// @see Data sequence meta-info @ref anchor_foo_bar_datareader_data_sequence "information".
        /// @see Destination order @ref anchor_foo_bar_datareader_destination_order "information".
        /// @see State masks @ref anchor_foo_bar_datareader_state_masks "information".
        /// </remarks>
        /// <param name="dataValues">
        ///     Array (will be resized) that'll contain the samples.</param>
        /// <param name="sampleInfos">
        ///     Array (will be resized) that'll contain the samples meta information.</param>
        /// <param name="maxSamples">
        ///     The maximum count of samples that will be taken.</param>
        /// <param name="instanceHandle">
        ///     The single instance, which the samples belong to.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - Data is available.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The DataReader has already been deleted.</item>
        /// <item>DDS.ReturnCode BadParameter - The parameter instanceHandle is not a valid handle.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS has ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode PreconditionNotMet - The instanceHandle is DDS.InstanceHandle.Nil.</item>
        /// <item>DDS.ReturnCode NotEnabled - The DataReader has not yet been enabled.</item>
        /// <item>DDS.ReturnCode NoData - No samples that meet the constraints are available.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode TakeInstance(
            ref Foo[] dataValues,
            ref DDS.SampleInfo[] sampleInfos,
            int maxSamples,
            DDS.InstanceHandle instanceHandle)
        {
            return DDS.ReturnCode.Ok;
        }

        /// <summary>
        /// This operation takes an array of typed samples from the DataReader.
        /// </summary>
        /// <remarks>
        /// The data is put into the given dataValues array, while meta data is put into
        /// the given sampleInfos array.<br>
        /// The given arrays will be resized to fit the number of read samples.
        ///
        /// It will only take samples of the given instance.
        ///
        /// It will also only take a maximum number of samples in one call.
        ///
        /// It will also only take samples that have the given states.
        ///
        /// Reading a sample will just keep the sample in the DataReaders' buffer.<br>
        /// Taking the sample will remove the sample from the DataReaders' buffer.
        ///
        /// @see Simple DataReader creation and read @ref anchor_foo_bar_datareader_example "example".
        /// @see Data sequence meta-info @ref anchor_foo_bar_datareader_data_sequence "information".
        /// @see Destination order @ref anchor_foo_bar_datareader_destination_order "information".
        /// @see State masks @ref anchor_foo_bar_datareader_state_masks "information".
        /// </remarks>
        /// <param name="dataValues">
        ///     Array (will be resized) that'll contain the samples.</param>
        /// <param name="sampleInfos">
        ///     Array (will be resized) that'll contain the samples meta information.</param>
        /// <param name="maxSamples">
        ///     The maximum count of samples that will be taken.</param>
        /// <param name="instanceHandle">
        ///     The single instance, which the samples belong to.</param>
        /// <param name="sampleStates">
        ///     A mask, which selects only those samples with the desired sample states.</param>
        /// <param name="viewStates">
        ///     A mask, which selects only those samples with the desired view states.</param>
        /// <param name="instanceStates">
        ///     A mask, which selects only those samples with the desired instance states.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - Data is available.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The DataReader has already been deleted.</item>
        /// <item>DDS.ReturnCode BadParameter - The parameter instanceHandle is not a valid handle.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS has ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode PreconditionNotMet - The instanceHandle is DDS.InstanceHandle.Nil.</item>
        /// <item>DDS.ReturnCode NotEnabled - The DataReader has not yet been enabled.</item>
        /// <item>DDS.ReturnCode NoData - No samples that meet the constraints are available.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode TakeInstance(
                ref Foo[] dataValues,
                ref DDS.SampleInfo[] sampleInfos,
                int maxSamples,
                DDS.InstanceHandle instanceHandle,
                DDS.SampleStateKind sampleStates,
                DDS.ViewStateKind viewStates,
                DDS.InstanceStateKind instanceStates)
        {
            return DDS.ReturnCode.Ok;
        }

        /// <summary>
        /// This operation reads an array of typed samples from the DataReader.
        /// </summary>
        /// <remarks>
        /// The data is put into the given dataValues array, while meta data is put into
        /// the given sampleInfos array.<br>
        /// The given arrays will be resized to fit the number of read samples.
        ///
        /// It will only read samples of a single instance. The behaviour is similar to
        /// ReadInstance (all samples returned belong to a single instance) except that
        /// the actual instance is not directly specified. Rather the samples will all
        /// belong to the ‘next’ instance with DDS.InstanceHandle ‘greater’ (according to
        /// some internal-defined order) than instanceHandle, that has available samples.<br>
        /// Providing DDS.InstanceHandle.Nil will return samples of the 'first' instance.
        ///
        /// Reading a sample will just keep the sample in the DataReaders' buffer.
        /// The sample can be replaced by a write action depending on QoS history
        /// policy (DDS.HistoryQosPolicy) of that DataReader.<br>
        /// Taking the sample will remove the sample from the DataReaders' buffer.
        ///
        /// @anchor anchor_foo_bar_datareader_instance_order
        /// <i><b>Instance Order</b></i><br>
        /// The internal-defined order is not important and is implementation specific. The
        /// important thing is that, according to the Data Distribution Service, all instances are
        /// ordered relative to each other. This ordering is between the instances, that is, it does
        /// not depend on the actual samples received. For the purposes of this explanation it is
        /// ‘as if’ each instance handle was represented as a unique integer.
        /// The behaviour of ReadNextInstance is ‘as if ’ the DataReader invoked
        /// ReadInstance passing the smallest instanceHandle among all the ones that:
        /// - are greater than instanceHandle
        /// - have available samples (i.e. samples that meet the constraints imposed by the
        ///   specified states).
        /// - The special value DDS.InstanceHandle.Nil is guaranteed to be ‘less than’ any valid
        ///   DDS.InstanceHandle.<br>
        ///   So the use of the parameter value instanceHandle==DDS.InstanceHandle.Nil will return
        ///   the samples for the instance which has the smallest DDS.InstanceHandle among all the
        ///   instances that contain available samples.
        ///
        /// @anchor anchor_foo_bar_datareader_next_instance_usage
        /// <i><b>Typical Use</b></i><br>
        /// The operation ReadNextInstance is intended to be used in an application-driven
        /// iteration where the application starts by passing instanceHandle==DDS.InstanceHandle.Nil,
        /// examines the samples returned, and then uses the DDS.InstanceHandle from the returned
        /// SampleInfo as the value of instanceHandle argument to the next call to ReadNextInstance.
        /// The iteration continues until ReadNextInstance returns the return value NoData.
        /// <code>
        /// DDS.ReturnCode result = DDS.ReturnCode.Ok;
        /// DDS.InstanceHandle handle = DDS.InstanceHandle.Nil;
        /// while (result == DDS.ReturnCode.Ok) {
        ///     result = reader.ReadNextInstance(ref dataList, ref infoList, handle);
        ///     if (result == DDS.ReturnCode.Ok) {
        ///         handle = infoList[0].InstanceHandle;
        ///         // Handle data and meta-info.
        ///     } else if (result == DDS.ReturnCode.NoData) {
        ///         // All instances have been read.
        ///     } else {
        ///         // An error has occured.
        ///     }
        /// }
        /// </code>
        ///
        /// @see Simple DataReader creation and read @ref anchor_foo_bar_datareader_example "example".
        /// @see Data sequence meta-info @ref anchor_foo_bar_datareader_data_sequence "information".
        /// @see Destination order @ref anchor_foo_bar_datareader_destination_order "information".
        /// @see State masks @ref anchor_foo_bar_datareader_state_masks "information".
        /// </remarks>
        /// <param name="dataValues">
        ///     Array (will be resized) that'll contain the samples.</param>
        /// <param name="sampleInfos">
        ///     Array (will be resized) that'll contain the samples meta information.</param>
        /// <param name="instanceHandle">
        ///     The single instance, which the samples belong to.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - Data is available.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The DataReader has already been deleted.</item>
        /// <item>DDS.ReturnCode BadParameter - The parameter instanceHandle is not a valid handle.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS has ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode PreconditionNotMet - The instanceHandle has not been registered with this DataReader.</item>
        /// <item>DDS.ReturnCode NotEnabled - The DataReader has not yet been enabled.</item>
        /// <item>DDS.ReturnCode NoData - No samples that meet the constraints are available.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode ReadNextInstance(
            ref Foo[] dataValues,
            ref DDS.SampleInfo[] sampleInfos,
            DDS.InstanceHandle instanceHandle)
        {
            return DDS.ReturnCode.Ok;
        }

        /// <summary>
        /// This operation reads an array of typed samples from the DataReader.
        /// </summary>
        /// <remarks>
        /// The data is put into the given dataValues array, while meta data is put into
        /// the given sampleInfos array.<br>
        /// The given arrays will be resized to fit the number of read samples.
        ///
        /// It will only read samples of a single instance. The behaviour is similar to
        /// ReadInstance (all samples returned belong to a single instance) except that
        /// the actual instance is not directly specified. Rather the samples will all
        /// belong to the ‘next’ instance with DDS.InstanceHandle ‘greater’ (according to
        /// some internal-defined order) than instanceHandle, that has available samples.<br>
        /// Providing DDS.InstanceHandle.Nil will return samples of the 'first' instance.
        ///
        /// It will also only read a maximum number of samples in one call.
        ///
        /// Reading a sample will just keep the sample in the DataReaders' buffer.
        /// The sample can be replaced by a write action depending on QoS history
        /// policy (DDS.HistoryQosPolicy) of that DataReader.<br>
        /// Taking the sample will remove the sample from the DataReaders' buffer.
        ///
        /// @see Simple DataReader creation and read @ref anchor_foo_bar_datareader_example "example".
        /// @see Simple ReadNextInstance usage @ref anchor_foo_bar_datareader_next_instance_usage "example".
        /// @see Data sequence meta-info @ref anchor_foo_bar_datareader_data_sequence "information".
        /// @see Destination order @ref anchor_foo_bar_datareader_destination_order "information".
        /// @see State masks @ref anchor_foo_bar_datareader_state_masks "information".
        /// @see Instance order @ref anchor_foo_bar_datareader_instance_order "information".
        /// </remarks>
        /// <param name="dataValues">
        ///     Array (will be resized) that'll contain the samples.</param>
        /// <param name="sampleInfos">
        ///     Array (will be resized) that'll contain the samples meta information.</param>
        /// <param name="maxSamples">
        ///     The maximum count of samples that will be read.</param>
        /// <param name="instanceHandle">
        ///     The single instance, which the samples belong to.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - Data is available.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The DataReader has already been deleted.</item>
        /// <item>DDS.ReturnCode BadParameter - The parameter instanceHandle is not a valid handle.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS has ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode PreconditionNotMet - The instanceHandle has not been registered with this DataReader.</item>
        /// <item>DDS.ReturnCode NotEnabled - The DataReader has not yet been enabled.</item>
        /// <item>DDS.ReturnCode NoData - No samples that meet the constraints are available.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode ReadNextInstance(
            ref Foo[] dataValues,
            ref DDS.SampleInfo[] sampleInfos,
            int maxSamples,
            DDS.InstanceHandle instanceHandle)
        {
            return DDS.ReturnCode.Ok;
        }

        /// <summary>
        /// This operation reads an array of typed samples from the DataReader.
        /// </summary>
        /// <remarks>
        /// The data is put into the given dataValues array, while meta data is put into
        /// the given sampleInfos array.<br>
        /// The given arrays will be resized to fit the number of read samples.
        ///
        /// It will only read samples of a single instance. The behaviour is similar to
        /// ReadInstance (all samples returned belong to a single instance) except that
        /// the actual instance is not directly specified. Rather the samples will all
        /// belong to the ‘next’ instance with DDS.InstanceHandle ‘greater’ (according to
        /// some internal-defined order) than instanceHandle, that has available samples.<br>
        /// Providing DDS.InstanceHandle.Nil will return samples of the 'first' instance.
        ///
        /// It will also only read a maximum number of samples in one call.
        ///
        /// It will also only read samples that have the given states.
        ///
        /// Reading a sample will just keep the sample in the DataReaders' buffer.
        /// The sample can be replaced by a write action depending on QoS history
        /// policy (DDS.HistoryQosPolicy) of that DataReader.<br>
        /// Taking the sample will remove the sample from the DataReaders' buffer.
        ///
        /// @see Simple DataReader creation and read @ref anchor_foo_bar_datareader_example "example".
        /// @see Simple ReadNextInstance usage @ref anchor_foo_bar_datareader_next_instance_usage "example".
        /// @see Data sequence meta-info @ref anchor_foo_bar_datareader_data_sequence "information".
        /// @see Destination order @ref anchor_foo_bar_datareader_destination_order "information".
        /// @see State masks @ref anchor_foo_bar_datareader_state_masks "information".
        /// @see Instance order @ref anchor_foo_bar_datareader_instance_order "information".
        /// </remarks>
        /// <param name="dataValues">
        ///     Array (will be resized) that'll contain the samples.</param>
        /// <param name="sampleInfos">
        ///     Array (will be resized) that'll contain the samples meta information.</param>
        /// <param name="maxSamples">
        ///     The maximum count of samples that will be read.</param>
        /// <param name="instanceHandle">
        ///     The single instance, which the samples belong to.</param>
        /// <param name="sampleStates">
        ///     A mask, which selects only those samples with the desired sample states.</param>
        /// <param name="viewStates">
        ///     A mask, which selects only those samples with the desired view states.</param>
        /// <param name="instanceStates">
        ///     A mask, which selects only those samples with the desired instance states.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - Data is available.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The DataReader has already been deleted.</item>
        /// <item>DDS.ReturnCode BadParameter - The parameter instanceHandle is not a valid handle.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS has ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode PreconditionNotMet - The instanceHandle has not been registered with this DataReader.</item>
        /// <item>DDS.ReturnCode NotEnabled - The DataReader has not yet been enabled.</item>
        /// <item>DDS.ReturnCode NoData - No samples that meet the constraints are available.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode ReadNextInstance(
                ref Foo[] dataValues,
                ref DDS.SampleInfo[] sampleInfos,
                int maxSamples,
                DDS.InstanceHandle instanceHandle,
                DDS.SampleStateKind sampleStates,
                DDS.ViewStateKind viewStates,
                DDS.InstanceStateKind instanceStates)
        {
            return DDS.ReturnCode.Ok;
        }

        /// <summary>
        /// This operation takes an array of typed samples from the DataReader.
        /// </summary>
        /// <remarks>
        /// The data is put into the given dataValues array, while meta data is put into
        /// the given sampleInfos array.<br>
        /// The given arrays will be resized to fit the number of read samples.
        ///
        /// It will only take samples of a single instance. The behaviour is similar to
        /// TakeInstance (all samples returned belong to a single instance) except that
        /// the actual instance is not directly specified. Rather the samples will all
        /// belong to the ‘next’ instance with DDS.InstanceHandle ‘greater’ (according to
        /// some internal-defined order) than instanceHandle, that has available samples.<br>
        /// Providing DDS.InstanceHandle.Nil will return samples of the 'first' instance.
        ///
        /// Reading a sample will just keep the sample in the DataReaders' buffer.<br>
        /// Taking the sample will remove the sample from the DataReaders' buffer.
        ///
        /// @see Simple DataReader creation and read @ref anchor_foo_bar_datareader_example "example".
        /// @see Simple ReadNextInstance usage @ref anchor_foo_bar_datareader_next_instance_usage "example".
        /// @see Data sequence meta-info @ref anchor_foo_bar_datareader_data_sequence "information".
        /// @see Destination order @ref anchor_foo_bar_datareader_destination_order "information".
        /// @see State masks @ref anchor_foo_bar_datareader_state_masks "information".
        /// @see Instance order @ref anchor_foo_bar_datareader_instance_order "information".
        /// </remarks>
        /// <param name="dataValues">
        ///     Array (will be resized) that'll contain the samples.</param>
        /// <param name="sampleInfos">
        ///     Array (will be resized) that'll contain the samples meta information.</param>
        /// <param name="instanceHandle">
        ///     The single instance, which the samples belong to.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - Data is available.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The DataReader has already been deleted.</item>
        /// <item>DDS.ReturnCode BadParameter - The parameter instanceHandle is not a valid handle.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS has ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode PreconditionNotMet - The instanceHandle has not been registered with this DataReader.</item>
        /// <item>DDS.ReturnCode NotEnabled - The DataReader has not yet been enabled.</item>
        /// <item>DDS.ReturnCode NoData - No samples that meet the constraints are available.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode TakeNextInstance(
            ref Foo[] dataValues,
            ref DDS.SampleInfo[] sampleInfos,
            DDS.InstanceHandle instanceHandle)
        {
            return DDS.ReturnCode.Ok;
        }

        /// <summary>
        /// This operation takes an array of typed samples from the DataReader.
        /// </summary>
        /// <remarks>
        /// The data is put into the given dataValues array, while meta data is put into
        /// the given sampleInfos array.<br>
        /// The given arrays will be resized to fit the number of read samples.
        ///
        /// It will only take samples of a single instance. The behaviour is similar to
        /// TakeInstance (all samples returned belong to a single instance) except that
        /// the actual instance is not directly specified. Rather the samples will all
        /// belong to the ‘next’ instance with DDS.InstanceHandle ‘greater’ (according to
        /// some internal-defined order) than instanceHandle, that has available samples.<br>
        /// Providing DDS.InstanceHandle.Nil will return samples of the 'first' instance.
        ///
        /// It will also only take a maximum number of samples in one call.
        ///
        /// Reading a sample will just keep the sample in the DataReaders' buffer.<br>
        /// Taking the sample will remove the sample from the DataReaders' buffer.
        ///
        /// @see Simple DataReader creation and read @ref anchor_foo_bar_datareader_example "example".
        /// @see Simple ReadNextInstance usage @ref anchor_foo_bar_datareader_next_instance_usage "example".
        /// @see Data sequence meta-info @ref anchor_foo_bar_datareader_data_sequence "information".
        /// @see Destination order @ref anchor_foo_bar_datareader_destination_order "information".
        /// @see State masks @ref anchor_foo_bar_datareader_state_masks "information".
        /// @see Instance order @ref anchor_foo_bar_datareader_instance_order "information".
        /// </remarks>
        /// <param name="dataValues">
        ///     Array (will be resized) that'll contain the samples.</param>
        /// <param name="sampleInfos">
        ///     Array (will be resized) that'll contain the samples meta information.</param>
        /// <param name="maxSamples">
        ///     The maximum count of samples that will be taken.</param>
        /// <param name="instanceHandle">
        ///     The single instance, which the samples belong to.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - Data is available.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The DataReader has already been deleted.</item>
        /// <item>DDS.ReturnCode BadParameter - The parameter instanceHandle is not a valid handle.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS has ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode PreconditionNotMet - The instanceHandle has not been registered with this DataReader.</item>
        /// <item>DDS.ReturnCode NotEnabled - The DataReader has not yet been enabled.</item>
        /// <item>DDS.ReturnCode NoData - No samples that meet the constraints are available.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode TakeNextInstance(
            ref Foo[] dataValues,
            ref DDS.SampleInfo[] sampleInfos,
            int maxSamples,
            DDS.InstanceHandle instanceHandle)
        {
            return DDS.ReturnCode.Ok;
        }

        /// <summary>
        /// This operation takes an array of typed samples from the DataReader.
        /// </summary>
        /// <remarks>
        /// The data is put into the given dataValues array, while meta data is put into
        /// the given sampleInfos array.<br>
        /// The given arrays will be resized to fit the number of read samples.
        ///
        /// It will only take samples of a single instance. The behaviour is similar to
        /// TakeInstance (all samples returned belong to a single instance) except that
        /// the actual instance is not directly specified. Rather the samples will all
        /// belong to the ‘next’ instance with DDS.InstanceHandle ‘greater’ (according to
        /// some internal-defined order) than instanceHandle, that has available samples.<br>
        /// Providing DDS.InstanceHandle.Nil will return samples of the 'first' instance.
        ///
        /// It will also only take a maximum number of samples in one call.
        ///
        /// It will also only take samples that have the given states.
        ///
        /// Reading a sample will just keep the sample in the DataReaders' buffer.<br>
        /// Taking the sample will remove the sample from the DataReaders' buffer.
        ///
        /// @see Simple DataReader creation and read @ref anchor_foo_bar_datareader_example "example".
        /// @see Simple ReadNextInstance usage @ref anchor_foo_bar_datareader_next_instance_usage "example".
        /// @see Data sequence meta-info @ref anchor_foo_bar_datareader_data_sequence "information".
        /// @see Destination order @ref anchor_foo_bar_datareader_destination_order "information".
        /// @see State masks @ref anchor_foo_bar_datareader_state_masks "information".
        /// @see Instance order @ref anchor_foo_bar_datareader_instance_order "information".
        /// </remarks>
        /// <param name="dataValues">
        ///     Array (will be resized) that'll contain the samples.</param>
        /// <param name="sampleInfos">
        ///     Array (will be resized) that'll contain the samples meta information.</param>
        /// <param name="maxSamples">
        ///     The maximum count of samples that will be taken.</param>
        /// <param name="instanceHandle">
        ///     The single instance, which the samples belong to.</param>
        /// <param name="sampleStates">
        ///     A mask, which selects only those samples with the desired sample states.</param>
        /// <param name="viewStates">
        ///     A mask, which selects only those samples with the desired view states.</param>
        /// <param name="instanceStates">
        ///     A mask, which selects only those samples with the desired instance states.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - Data is available.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The DataReader has already been deleted.</item>
        /// <item>DDS.ReturnCode BadParameter - The parameter instanceHandle is not a valid handle.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS has ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode PreconditionNotMet - The instanceHandle has not been registered with this DataReader.</item>
        /// <item>DDS.ReturnCode NotEnabled - The DataReader has not yet been enabled.</item>
        /// <item>DDS.ReturnCode NoData - No samples that meet the constraints are available.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode TakeNextInstance(
                ref Foo[] dataValues,
                ref DDS.SampleInfo[] sampleInfos,
                int maxSamples,
                DDS.InstanceHandle instanceHandle,
                DDS.SampleStateKind sampleStates,
                DDS.ViewStateKind viewStates,
                DDS.InstanceStateKind instanceStates)
        {
            return DDS.ReturnCode.Ok;
        }

        /// <summary>
        /// This operation reads an array of typed samples from the DataReader.
        /// </summary>
        /// <remarks>
        /// @note This operation is not yet implemented. It is scheduled for a future release.
        ///
        /// @see Simple DataReader creation and read @ref anchor_foo_bar_datareader_example "example".
        /// @see Simple ReadNextInstance usage @ref anchor_foo_bar_datareader_next_instance_usage "example".
        /// @see Data sequence meta-info @ref anchor_foo_bar_datareader_data_sequence "information".
        /// @see Destination order @ref anchor_foo_bar_datareader_destination_order "information".
        /// @see State masks @ref anchor_foo_bar_datareader_state_masks "information".
        /// @see Instance order @ref anchor_foo_bar_datareader_instance_order "information".
        /// </remarks>
        /// <param name="dataValues">
        ///     Array (will be resized) that'll contain the samples.</param>
        /// <param name="sampleInfos">
        ///     Array (will be resized) that'll contain the samples meta information.</param>
        /// <param name="instanceHandle">
        ///     The single instance, which the samples belong to.</param>
        /// <param name="readCondition">
        ///     A DDS.IReadCondition or DDS.IQueryCondition which filters the data before
        ///     it is returned by the read operation.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Unsupported - Not supported yet.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode ReadNextInstanceWithCondition(
                ref Foo[] dataValues,
                ref DDS.SampleInfo[] sampleInfos,
                DDS.InstanceHandle instanceHandle,
                DDS.IReadCondition readCondition)
        {
            return DDS.ReturnCode.Unsupported;
        }

        /// <summary>
        /// This operation reads an array of typed samples from the DataReader.
        /// </summary>
        /// <remarks>
        /// @note This operation is not yet implemented. It is scheduled for a future release.
        ///
        /// @see Simple DataReader creation and read @ref anchor_foo_bar_datareader_example "example".
        /// @see Simple ReadNextInstance usage @ref anchor_foo_bar_datareader_next_instance_usage "example".
        /// @see Data sequence meta-info @ref anchor_foo_bar_datareader_data_sequence "information".
        /// @see Destination order @ref anchor_foo_bar_datareader_destination_order "information".
        /// @see State masks @ref anchor_foo_bar_datareader_state_masks "information".
        /// @see Instance order @ref anchor_foo_bar_datareader_instance_order "information".
        /// </remarks>
        /// <param name="dataValues">
        ///     Array (will be resized) that'll contain the samples.</param>
        /// <param name="sampleInfos">
        ///     Array (will be resized) that'll contain the samples meta information.</param>
        /// <param name="maxSamples">
        ///     The maximum count of samples that will be read.</param>
        /// <param name="instanceHandle">
        ///     The single instance, which the samples belong to.</param>
        /// <param name="readCondition">
        ///     A DDS.IReadCondition or DDS.IQueryCondition which filters the data before
        ///     it is returned by the read operation.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Unsupported - Not supported yet.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode ReadNextInstanceWithCondition(
                ref Foo[] dataValues,
                ref DDS.SampleInfo[] sampleInfos,
                int maxSamples,
                DDS.InstanceHandle instanceHandle,
                DDS.IReadCondition readCondition)
        {
            return DDS.ReturnCode.Unsupported;
        }

        /// <summary>
        /// This operation takes an array of typed samples from the DataReader.
        /// </summary>
        /// <remarks>
        /// @note This operation is not yet implemented. It is scheduled for a future release.
        ///
        /// @see Simple DataReader creation and read @ref anchor_foo_bar_datareader_example "example".
        /// @see Simple ReadNextInstance usage @ref anchor_foo_bar_datareader_next_instance_usage "example".
        /// @see Data sequence meta-info @ref anchor_foo_bar_datareader_data_sequence "information".
        /// @see Destination order @ref anchor_foo_bar_datareader_destination_order "information".
        /// @see State masks @ref anchor_foo_bar_datareader_state_masks "information".
        /// @see Instance order @ref anchor_foo_bar_datareader_instance_order "information".
        /// </remarks>
        /// <param name="dataValues">
        ///     Array (will be resized) that'll contain the samples.</param>
        /// <param name="sampleInfos">
        ///     Array (will be resized) that'll contain the samples meta information.</param>
        /// <param name="instanceHandle">
        ///     The single instance, which the samples belong to.</param>
        /// <param name="readCondition">
        ///     A DDS.IReadCondition or DDS.IQueryCondition which filters the data before
        ///     it is returned by the read operation.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Unsupported - Not supported yet.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode TakeNextInstanceWithCondition(
                ref Foo[] dataValues,
                ref DDS.SampleInfo[] sampleInfos,
                DDS.InstanceHandle instanceHandle,
                DDS.IReadCondition readCondition)
        {
            return DDS.ReturnCode.Unsupported;
        }

        /// <summary>
        /// This operation takes an array of typed samples from the DataReader.
        /// </summary>
        /// <remarks>
        /// @note This operation is not yet implemented. It is scheduled for a future release.
        ///
        /// @see Simple DataReader creation and read @ref anchor_foo_bar_datareader_example "example".
        /// @see Simple ReadNextInstance usage @ref anchor_foo_bar_datareader_next_instance_usage "example".
        /// @see Data sequence meta-info @ref anchor_foo_bar_datareader_data_sequence "information".
        /// @see Destination order @ref anchor_foo_bar_datareader_destination_order "information".
        /// @see State masks @ref anchor_foo_bar_datareader_state_masks "information".
        /// @see Instance order @ref anchor_foo_bar_datareader_instance_order "information".
        /// </remarks>
        /// <param name="dataValues">
        ///     Array (will be resized) that'll contain the samples.</param>
        /// <param name="sampleInfos">
        ///     Array (will be resized) that'll contain the samples meta information.</param>
        /// <param name="maxSamples">
        ///     The maximum count of samples that will be taken.</param>
        /// <param name="instanceHandle">
        ///     The single instance, which the samples belong to.</param>
        /// <param name="readCondition">
        ///     A DDS.IReadCondition or DDS.IQueryCondition which filters the data before
        ///     it is returned by the read operation.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Unsupported - Not supported yet.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode TakeNextInstanceWithCondition(
                ref Foo[] dataValues,
                ref DDS.SampleInfo[] sampleInfos,
                int maxSamples,
                DDS.InstanceHandle instanceHandle,
                DDS.IReadCondition readCondition)
        {
            return DDS.ReturnCode.Unsupported;
        }

        /// <summary>
        /// This operation indicates to the DataReader that the application is done accessing
        /// the dataValues and sampleInfos arrays.
        /// </summary>
        /// <remarks>
        /// This operation indicates to the DataReader that the application is done accessing
        /// the dataValues and sampleInfos arrays obtained by some earlier invocation
        /// of the operation Read or Take (or any of the similar operations) on the
        /// DataReader.
        ///
        /// The dataValues and sampleInfos must belong to a single related pair. That is, they
        /// should correspond to a pair returned from a single call to the operation Read or
        /// Take. The dataValues and sampleInfos must also have been obtained from the
        /// same DataReader to which they are returned. If either of these conditions is not
        /// met the operation will fail and returns DDS.ReturnCode PreconditionNotMet.
        ///
        /// <i><b>Buffer Loan</b></i><br>
        /// The operation ReturnLoan allows implementations of the Read and Take
        /// operations to “loan” buffers from the Data Distribution Service to the application
        /// and in this manner provide “zero-copy” access to the data. During the loan, the Data
        /// Distribution Service will guarantee that the dataValues and sampleInfos are not
        /// modified.<br>
        /// It is not necessary for an application to return the loans immediately after calling the
        /// operation Read or Take. However, as these buffers can correspond to internal resources
        /// inside the DataReader, the application should not retain them indefinitely.
        ///
        /// @see Simple DataReader creation and read @ref anchor_foo_bar_datareader_example "example".
        /// </remarks>
        /// <param name="dataValues">
        ///     The sample data which was loaned from the DataReader.</param>
        /// <param name="sampleInfos">
        ///     The SampleInfo which was loaned from the DataReader.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok -
        ///     The DataReader is informed that the arrays will not be used any more</item>
        /// <item>DDS.ReturnCode Error -
        ///     An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted -
        ///     The DataReader has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources -
        ///     The DDS has ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode PreconditionNotMet -
        ///     The dataValues and sampleInfos do not belong to a single pair.</item>
        /// <item>DDS.ReturnCode NotEnabled -
        ///     The DataReader has not yet been enabled.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode ReturnLoan(
                ref Foo[] dataValues,
                ref DDS.SampleInfo[] sampleInfos)
        {
            return DDS.ReturnCode.Ok;
        }

        /// <summary>
        /// This operation retrieves the key value of a specific instance.
        /// </summary>
        /// <remarks>
        /// This operation retrieves the key value of the instance referenced to by
        /// instanceHandle. When the operation is called with a DDS.InstanceHandle.Nil
        /// handle value as an instanceHandle, the operation will return DDS.ReturnCode
        /// BadParameter.
        ///
        /// The operation will only fill the data fields that form the key
        /// inside the data instance (Space.Foo in this case). This means that the non-key
        /// fields are not applicable and may contain garbage.
        ///
        /// The operation must only be called on registered instances. Otherwise the
        /// operation returns DDS.ReturnCode PreconditionNotMet.
        /// </remarks>
        /// <param name="key">
        ///     A sample in which the key values are stored.</param>
        /// <param name="instanceHandle">
        ///     The handle to the instance from which to get the key value.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok -
        ///     The key fields are set in the sample</item>
        /// <item>DDS.ReturnCode Error -
        ///     An internal error has occurred.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted -
        ///     The DataReader has already been deleted.</item>
        /// <item>DDS.ReturnCode OutOfResources -
        ///     The DDS has ran out of resources to complete this operation.</item>
        /// <item>DDS.ReturnCode PreconditionNotMet -
        ///     The handle has not been registered with this DataReader.</item>
        /// <item>DDS.ReturnCode BadParameter -
        ///     The parameter handle is not a valid handle.</item>
        /// <item>DDS.ReturnCode NotEnabled -
        ///     The DataReader has not yet been enabled.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode GetKeyValue(
                ref Foo key,
                DDS.InstanceHandle instanceHandle)
        {
            return DDS.ReturnCode.Ok;
        }

        /// <summary>
        /// This operation returns the handle which corresponds to the instance data.
        /// </summary>
        /// <remarks>
        /// The instance handle can be used in read operations that operate on a
        /// specific instance.
        ///
        /// Note that DataReader instance handles are local, and are
        /// not interchangeable with DataWriter instance handles nor with instance handles
        /// of an other DataReader. If the DataReader is already deleted, the handle value
        /// DDS.InstanceHandle.Nil is returned.
        /// </remarks>
        /// <param name="instance">The instance for which the corresponding instance
        ///     handle needs to be looked up.</param>
        /// <returns>Return values are:
        /// <list type="bullet">
        /// <item>DDS.InstanceHandle - Handle which corresponds to given instance.</item>
        /// </list>
        /// </returns>
        public DDS.InstanceHandle LookupInstance(
                Foo instance)
        {
            return DDS.InstanceHandle.Nil;
        }

    }




    /// <summary>
    /// The specialized DataWriter for the fictional type Space::Foo<br>
    /// </summary>
    /// <remarks>
    /// @note Class hierarchy in the documentation has been simplified for clearity purposes.
    ///
    /// <code>
    /// /* Simplest creation of a typed datawriter.
    ///  * Defaults are used and possible errors are ignored. */
    ///
    /// /* Prepare Domain. */
    /// DDS.DomainParticipantFactory factory = DDS.DomainParticipantFactory.Instance;
    /// DDS.IDomainParticipant participant = factory.CreateParticipant(DDS.DomainId.Default);
    ///
    /// /* Add topic data type to the system. */
    /// DDS.ITypeSupport typesupport = new Space.FooTypeSupport();
    /// DDS.ReturnCode retcode = typesupport.RegisterType(participant, "Space.Foo");
    ///
    /// DDS.ITopic topic = participant.CreateTopic("SpaceFooTopic", "Space.Foo");
    ///
    /// /* Create typed datawriter. */
    /// DDS.IPublisher publisher = participant.CreatePublisher();
    /// Space.FooDataWriter writer = (Space.FooDataWriter)publisher.CreateDataWriter(topic);
    ///
    /// /* Write a sample */
    /// Space.Foo sample;
    /// sample.Bar = 42;
    /// retcode = writer.Write(sample);
    /// </code>
    /// </remarks>
    public class FooDataWriter : DDS.IDataWriter
    {
        /// <summary>
        /// This operation informs the Data Distribution Service that the application will be
        /// modifying a particular instance.
        /// </summary>
        /// <remarks>
        /// <para>
        /// This operation informs the Data Distribution Service that the application will be
        /// modifying a particular instance. This operation may be invoked prior to calling any
        /// operation that modifies the instance, such as FooDataWriter.Write, FooDataWriter.WriteWithTimestamp,
        /// FooDataWriter.UnregisterInstance, FooDataWriter.UnregisterInstanceWithTimestamp, FooDataWriter.Dispose,
        /// FooDataWriter.DisposeWithTimestamp, FooDataWriter.WriteDispose and FooDataWriter.WriteDisposeWithTimestamp.
        /// When the application does register the instance before modifying, the Data
        /// Distribution Service will handle the instance more efficiently. It takes as a parameter
        /// (instanceData) an instance (to get the key value) and returns a handle that can
        /// be used in successive FooDataWriter operations. In case of an error, DDS.InstanceHandle.Nil
        /// is returned.
        /// </para><para>
        /// The explicit use of this operation is optional as the application can directly call the
        /// FooDataWriter.Write, FooDataWriter.WriteWithTimestamp,FooDataWriter. UnregisterInstance,
        /// FooDataWriter.UnregisterInstanceWithTimestamp, FooDataWriter.Dispose, FooDataWriter.DisposeWithTimestamp,
        /// FooDataWriter.WriteDispose and FooDataWriter.WriteDisposeWithTimestamp operations and specify a
        /// DDS.InstanceHandle.Nil value to indicate that the sample should be examined to identify the
        /// instance.
        /// When this operation is used, the Data Distribution Service will automatically supply
        /// the value of the SourceTimestamp that is made available to connected
        /// FooDataReader objects. This timestamp is important for the interpretation of the
        /// DDS.DestinationOrderQosPolicy.
        /// </para><para>
        /// <b><i>Blocking</i></b>
        /// </para><para>
        /// If the DDS.HistoryQosPolicy is set to KeepAllHistoryQos, the
        /// FooDataWriter.RegisterInstance operation on the DataWriter may block if the
        /// modification would cause data to be lost because one of the limits, specified in the
        /// DDS.ResourceLimitsQosPolicy, to be exceeded. In case the synchronous attribute
        /// value of the DDS.ReliabilityQosPolicy is set to true for communicating
        /// DataWriters and DataReaders then the DataWriter will wait until all
        /// synchronous DataReaders have acknowledged the data. Under these
        /// circumstances, the MaxBlockingTime attribute of the
        /// DDS.ReliabilityQosPolicy configures the maximum time the
        /// FooDataWriter.RegisterInstance operation may block (either waiting for space to become
        /// available or data to be acknowledged). If MaxBlockingTime elapses before the
        /// DataWriter is able to store the modification without exceeding the limits and all
        /// expected acknowledgements are received, the FooDataWriter.RegisterInstance operation will
        /// fail and returns DDS.InstanceHandle.Nil.
        /// </para><para>
        /// <b><i>Sample Validation</i></b>
        /// </para><para>
        /// Since the sample that is passed as instanceData is merely used to determine the
        /// identity based on the uniqueness of its key values, only the keyfields will be
        /// validated against the restrictions imposed by the IDL to C# language mapping,
        /// where:
        /// <list type="bullet">
        /// <item>a string (bounded or unbounded) may not be null. (Use “” for an empty string
        /// instead)</item>
        /// <item>the length of a bounded string may not exceed the limit specified in IDL</item>
        /// </list>
        /// If any of these restrictions is violated, the operation will fail and return a
        /// DDS.InstanceHandle.Nil. More specific information about the context of this error will be
        /// written to the error log.
        /// </para><para>
        /// <b><i>Multiple Calls</i></b>
        /// </para><para>
        /// If this operation is called for an already registered instance, it just returns the already
        /// allocated instance handle. This may be used to look up and retrieve the handle
        /// allocated to a given instance.
        /// </para>
        /// </remarks>
        /// <param name="instanceData">The instance, which the application writes to or
        /// disposes of.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.InstanceHandle - Handle to the Instance, which may be used for writing and
        /// disposing of. In case of an error, an DDS.InstanceHandle.Nil constant is returned.</item>
        /// </list>
        /// </returns>
        public DDS.InstanceHandle RegisterInstance(
                Foo instanceData)
        {
            return DDS.InstanceHandle.Nil;
        }

        /// <summary>
        /// This operation will inform the Data Distribution Service that the application will be
        /// modifying a particular instance and provides a value for the sourceTimestamp
        /// explicitly.
        /// </summary>
        /// <remarks>
        /// <para>
        /// This operation performs the same functions as FooDataWriter.RegisterInstance except that
        /// the application provides the value for the sourceTimestamp that is made
        /// available to connected DataReader objects. This timestamp is important for the
        /// interpretation of the DDS.DestinationOrderQosPolicy.
        /// </para><para>
        /// <b><i>Multiple Calls</i></b>
        /// </para><para>
        /// If this operation is called for an already registered instance, it just returns the already
        /// allocated instance handle. The sourceTimestamp is ignored in that case.
        /// </para>
        /// </remarks>
        /// <param name="instanceData">The instance, which the application writes to or
        /// disposes of.</param>
        /// <param name="sourceTimestamp">The timestamp used.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.InstanceHandle - Handle to the Instance, which may be used for writing and
        /// disposing of. In case of an error, an DDS.InstanceHandle.Nil constant is returned.</item>
        /// </list>
        /// </returns>
        public DDS.InstanceHandle RegisterInstanceWithTimestamp(
                Foo instanceData,
                DDS.Time sourceTimestamp)
        {
            return DDS.InstanceHandle.Nil;
        }

        /// <summary>
        /// This operation informs the Data Distribution Service that the application will not be
        /// modifying a particular instance any more.
        /// explicitly.
        /// </summary>
        /// <remarks>
        /// <para>
        /// This operation informs the Data Distribution Service that the application will not be
        /// modifying a particular instance any more. Therefore, this operation reverses the
        /// action of FooDataWriter.RegisterInstance or FooDataWriter.RegisterInstanceWithTimestamp. It
        /// should only be called on an instance that is currently registered. This operation
        /// should be called just once per instance, regardless of how many times
        /// FooDataWriter.RegisterInstance was called for that instance. This operation also indicates
        /// that the Data Distribution Service can locally remove all information regarding that
        /// instance. The application should not attempt to use the handle, previously
        /// allocated to that instance, after calling this operation.
        /// When this operation is used, the Data Distribution Service will automatically supply
        /// the value of the SourceTimestamp that is made available to connected
        /// DataReader objects. This timestamp is important for the interpretation of the
        /// DDS.DestinationOrderQosPolicy.
        /// </para><para>
        /// <b><i>Effects</i></b>
        /// </para><para>
        /// If, after unregistering, the application wants to modify (write or dispose) the
        /// instance, it has to register the instance again, or it has to use the special
        /// constant DDS.InstanceHandle.Nil.
        /// </para><para>
        /// This operation does not indicate that the instance should be deleted (that is the
        /// purpose of dispose). This operation just indicates that the DataWriter no longer
        /// has “anything to say” about the instance. If there is no other DataWriter that
        /// has registered the instance as well, then the DDS.InstanceStateKind in all connected
        /// DataReaders will be changed to NotAliveNoWriters InstanceState,
        /// provided this DDS.InstanceStateKind was not already set to
        /// NotAliveDisposed. In the last case the
        /// DDS.InstanceStateKind will not be effected by the UnregisterInstance call.
        /// @see @ref DCPS_Modules_Subscription_SampleInfo
        /// </para><para>
        /// This operation can affect the ownership of the data instance. If the
        /// DataWriter was the exclusive owner of the instance, calling this operation will
        /// release that ownership, meaning ownership may be transferred to another,
        /// possibly lower strength, DataWriter.
        /// The operation must be called only on registered instances. Otherwise the operation
        /// returns the error DDS.ReturnCode PreconditionNotMet.
        /// </para><para>
        /// <b><i>Instance Handle</i></b>
        /// </para><para>
        /// The special constant DDS.InstanceHandle.Nil can be used for the parameter handle. This
        /// indicates that the identity of the instance is automatically deduced from the
        /// instanceData (by means of the key).
        /// If handle is any value other than the special constant DDS.InstanceHandle.Nil, then it must
        /// correspond to the value returned by FooDataWriter.RegisterInstance or
        /// FooDataWriter.RegisterInstanceWithTimestamp when the instance (identified by its key)
        /// was registered. If there is no correspondence, the result of the operation is
        /// unspecified.
        /// The sample that is passed as instanceData is only used to check for consistency
        /// between its key values and the supplied instanceHandle: the sample itself will
        /// not actually be delivered to the connected DataReaders.
        /// </para><para>
        /// <b><i>Blocking</i></b>
        /// </para><para>
        /// If the DDS.HistoryQosPolicy is set to KeepAllHistoryQos, the
        /// UnregisterInstance operation on the DataWriter may block if the
        /// modification would cause data to be lost because one of the limits, specified in the
        /// DDS.ResourceLimitsQosPolicy, to be exceeded. In case the synchronous attribute
        /// value of the DDS.ReliabilityQosPolicy is set to true for communicating
        /// DataWriters and DataReaders then the DataWriter will wait until all
        /// synchronous DataReaders have acknowledged the data. Under these
        /// circumstances, the MaxBlockingTime attribute of the
        /// DDS.ReliabilityQosPolicy configures the maximum time the
        /// UnregisterInstance operation may block (either waiting for space to become
        /// available or data to be acknowledged). If MaxBlockingTime elapses before the
        /// DataWriter is able to store the modification without exceeding the limits and all
        /// expected acknowledgements are received, the UnregisterInstance operation
        /// will fail and returns DDS.InstanceHandle.Nil.
        /// </para><para>
        /// <b><i>Sample Validation</i></b>
        /// </para><para>
        /// Since the sample that is passed as instanceData is merely used to check for
        /// consistency between its key values and the supplied instanceHandle, only
        /// these keyfields will be validated against the restrictions imposed by the IDL to C#
        /// language mapping, where:
        /// <list type="bullet">
        /// <item>a string (bounded or unbounded) may not be null. (Use “” for an empty string
        /// instead)</item>
        /// <item>the length of a bounded string may not exceed the limit specified in IDL</item>
        /// </list>
        /// If any of these restrictions is violated, the operation will fail and return a
        /// DDS.ReturnCode BadParameter. More specific information about the context of this
        /// error will be written to the error log.
        /// </para>
        /// </remarks>
        /// <param name="instanceData">The instance, which the application writes to or
        /// disposes of.</param>
        /// <param name="instanceHandle">The handle to the Instance, which has been used for writing
        /// and disposing.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The Data Distribution Service is informed that the instance will not
        /// be modified any more.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode BadParameter - The instanceHandle is not a valid handle or the instanceData
        /// is not a valid sample.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The DataWriter has already been deleted.</item>
        /// <item>DDS.ReturnCode NotEnabled - The DataWriter is not enabled.</item>
        /// <item>DDS.ReturnCode PreconditionNotMet - The handle has not been registered with
        /// this DataWriter.</item>
        /// <item>DDS.ReturnCode Timeout - Either the current action overflowed the available resources
        /// as specified by the combination of the DDS.ReliablityQosPolicy,
        /// DDS.HistoryQosPolicy and DDS.ResourceLimitsQosPolicy, or the current action
        /// was waiting for data delivery acknowledgement by synchronous DataReaders.
        /// This caused blocking of the UnregisterInstance operation, which could not
        /// be resolved before MaxBlockingTime of the DDS.ReliabilityQosPolicy
        /// elapsed.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode UnregisterInstance(
                Foo instanceData,
                DDS.InstanceHandle instanceHandle)
        {
            return DDS.ReturnCode.Ok;
        }

        /// <summary>
        /// This operation will inform the Data Distribution Service that the application will not
        /// be modifying a particular instance any more and provides a value for the
        /// sourceTimestamp explicitly.
        /// </summary>
        /// <remarks>
        /// <para>
        /// This operation performs the same functions as UnregisterInstance except that
        /// the application provides the value for the sourceTimestamp that is made
        /// available to connected DataReader objects. This timestamp is important for the
        /// interpretation of the DDS.DestinationOrderQosPolicy.
        /// </remarks>
        /// <param name="instanceData">The instance, which the application writes to or
        /// disposes of.</param>
        /// <param name="instanceHandle">The handle to the Instance, which has been used for writing
        /// and disposing.</param>
        /// <param name="sourceTimestamp">The timestamp used.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The Data Distribution Service is informed that the instance will not
        /// be modified any more.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode BadParameter - The instanceHandle is not a valid handle or the instanceData
        /// is not a valid sample.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The DataWriter has already been deleted.</item>
        /// <item>DDS.ReturnCode NotEnabled - The DataWriter is not enabled.</item>
        /// <item>DDS.ReturnCode PreconditionNotMet - The handle has not been registered with
        /// this DataWriter.</item>
        /// <item>DDS.ReturnCode Timeout - Either the current action overflowed the available resources
        /// as specified by the combination of the DDS.ReliablityQosPolicy,
        /// DDS.HistoryQosPolicy and DDS.ResourceLimitsQosPolicy, or the current action
        /// was waiting for data delivery acknowledgement by synchronous DataReaders.
        /// This caused blocking of the UnregisterInstanceWithTimestamp operation,
        /// which could not be resolved before MaxBlockingTime of the
        /// DDS.ReliabilityQosPolicy elapsed.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode UnregisterInstanceWithTimestamp(
                Foo instanceData,
                DDS.InstanceHandle instanceHandle,
                DDS.Time sourceTimestamp)
        {
            return DDS.ReturnCode.Ok;
        }

        /// <summary>
        /// This operation modifies the value of a data instance.
        /// </summary>
        /// <remarks>
        /// This operation behaves simular to the Write(Foo instanceData, DDS.InstanceHandle instanceHandle)
        /// operation where the instanceHandle is specified as DDS.InstanceHandle.Nil.
        /// </remarks>
        /// <param name="instanceData">The data to be written.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - the value of a data instance is modified.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode BadParameter - The handle is not a valid handle or instanceData
        /// is not a valid sample.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The DataWriter has already been deleted.</item>
        /// <item>DDS.ReturnCode NotEnabled - The DataWriter is not enabled.</item>
        /// <item>DDS.ReturnCode PreconditionNotMet - The handle has not been registered with
        /// this DataWriter.</item>
        /// <item>DDS.ReturnCode Timeout - Either the current action overflowed the available resources
        /// as specified by the combination of the DDS.ReliablityQosPolicy,
        /// DDS.HistoryQosPolicy and DDS.ResourceLimitsQosPolicy, or the current action
        /// was waiting for data delivery acknowledgement by synchronous DataReaders.This caused
        /// blocking of the Write operation, which could not be resolved before MaxBlockingTime of the
        /// DDS.ReliabilityQosPolicy elapsed.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode Write(Foo instanceData)
        {
            return DDS.ReturnCode.Ok;
        }

        /// <summary>
        /// This operation modifies the value of a data instance.
        /// </summary>
        /// <remarks>
        /// <para>
        /// This operation modifies the value of a data instance. When this operation is used,
        /// the Data Distribution Service will automatically supply the value of the
        /// SourceTimestamp that is made available to connected DataReader objects.
        /// This timestamp is important for the interpretat ion of the
        /// DDS.DestinationOrderQosPolicy.
        /// As a side effect, this operation asserts liveliness on the DataWriter itself and on
        /// the containing IDomainParticipant.
        /// Before writing data to an instance, the instance may be registered with the
        /// FooDataWriter.RegisterInstance or FooDataWriter.RegisterInstanceWithTimestamp operation. The
        /// handle returned by one of the FooDataWriter.RegisterInstance operations can be supplied to
        /// the parameter handle of the write operation. However, it is also possible to
        /// supply the special constant DDS.InstanceHandle.Nil, which means that the identity of the
        /// instance is automatically deduced from the instanceData (identified by the
        /// key).
        /// </para><para>
        /// <b><i>Instance Handle</i></b>
        /// </para><para>
        /// The special constant DDS.InstanceHandle.Nil can be used for the parameter handle. This
        /// indicates the identity of the instance is automatically deduced from the
        /// instanceData (by means of the key).
        /// If handle is any value other than the special constant DDS.InstanceHandle.Nil, it must
        /// correspond to the value returned by FooDataWriter.RegisterInstance or
        /// FooDataWriter.RegisterInstanceWithTimestamp when the instance (identified by its key)
        /// was registered. Passing such a registered handle helps the Data Distribution
        /// Service to process the sample more efficiently. If there is no correspondence
        /// between handle and sample, the result of the operation is unspecified.
        /// </para><para>
        /// <b><i>Blocking</i></b>
        /// </para><para>
        /// If the DDS.HistoryQosPolicy is set to KeepAllHistoryQos, the write
        /// operation on the DataWriter may block if the modification would cause data to be
        /// lost because one of the limits, specified in the DDS.ResourceLimitsQosPolicy, is
        /// exceeded. In case the synchronous attribute value of the
        /// DDS.ReliabilityQosPolicy is set to true for communicating DataWriters and
        /// DataReaders then the DataWriter will wait until all synchronous
        /// DataReaders have acknowledged the data. Under these circumstances, the
        /// MaxBlockingTime attribute of the DDS.ReliabilityQosPolicy configures the
        /// maximum time the write operation may block (either waiting for space to become
        /// available or data to be acknowledged). If MaxBlockingTime elapses before the
        /// DataWriter is able to store the modification without exceeding the limits and all
        /// expected acknowledgements are received, the write operation will fail and returns
        /// DDS.ReturnCode Timeout.
        /// </para><para>
        /// <b><i>Sample Validation</i></b>
        /// </para><para>
        /// Before the sample is accepted by the DataWriter, it will be validated against the
        /// restrictions imposed by the IDL to C# language mapping, where:
        /// <list type="bullet">
        /// <item>a string (bounded or unbounded) may not be null. (Use “” for an empty string
        /// instead)</item>
        /// <item>the length of a bounded string may not exceed the limit specified in IDL</item>
        /// <item>the length of a bounded sequence may not exceed the limit specified in IDL</item>
        /// <item>the length of an array must exactly match the size specified in IDL</item>
        /// </list>
        /// If any of these restrictions is violated, the operation will fail and return a
        /// DDS.ReturnCode BadParameter. More specific information about the context of this error will be
        /// written to the error log.
        /// </para>
        /// </remarks>
        /// <param name="instanceData">The data to be written.</param>
        /// <param name="instanceHandle">The handle to the instance as supplied by FooDataWriter.RegisterInstance.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - the value of a data instance is modified.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode BadParameter - The handle is not a valid handle or instanceData
        /// is not a valid sample.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The DataWriter has already been deleted.</item>
        /// <item>DDS.ReturnCode NotEnabled - The DataWriter is not enabled.</item>
        /// <item>DDS.ReturnCode PreconditionNotMet - The handle has not been registered with
        /// this DataWriter.</item>
        /// <item>DDS.ReturnCode Timeout - Either the current action overflowed the available resources
        /// as specified by the combination of the DDS.ReliablityQosPolicy,
        /// DDS.HistoryQosPolicy and DDS.ResourceLimitsQosPolicy, or the current action
        /// was waiting for data delivery acknowledgement by synchronous DataReaders.This caused
        /// blocking of the Write operation, which could not be resolved before MaxBlockingTime of the
        /// DDS.ReliabilityQosPolicy elapsed.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode Write(
                Foo instanceData,
                DDS.InstanceHandle instanceHandle)
        {
            return DDS.ReturnCode.Ok;
        }

        /// <summary>
        /// This operation modifies the value of a data instance.
        /// </summary>
        /// <remarks>
        /// This operation behaves simular to the WriteWithTimestamp(Foo instanceData, DDS.InstanceHandle
        /// instanceHandle, DDS.Time sourceTimestamp) operation where the instanceHandle is specified
        /// as DDS.InstanceHandle.Nil.
        /// </remarks>
        /// <param name="instanceData">The data to be written.</param>
        /// <param name="sourceTimestamp">The timestamp used.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - the value of a data instance is modified.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode BadParameter - The handle is not a valid handle or instanceData
        /// is not a valid sample.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The DataWriter has already been deleted.</item>
        /// <item>DDS.ReturnCode NotEnabled - The DataWriter is not enabled.</item>
        /// <item>DDS.ReturnCode PreconditionNotMet - The handle has not been registered with
        /// this DataWriter.</item>
        /// <item>DDS.ReturnCode Timeout - Either the current action overflowed the available resources
        /// as specified by the combination of the DDS.ReliablityQosPolicy,
        /// DDS.HistoryQosPolicy and DDS.ResourceLimitsQosPolicy, or the current action
        /// was waiting for data delivery acknowledgement by synchronous DataReaders.This caused
        /// blocking of the Write operation, which could not be resolved before MaxBlockingTime of the
        /// DDS.ReliabilityQosPolicy elapsed.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode WriteWithTimestamp(
                Foo instanceData,
                DDS.Time sourceTimestamp)
        {
            return DDS.ReturnCode.Ok;
        }

        /// <summary>
        /// This operation modifies the value of a data instance and provides a value for the
        /// sourceTimestamp explicitly.
        /// </summary>
        /// <remarks>
        /// This operation behaves simular to the Write(Foo instanceData, DDS.InstanceHandle instanceHandle)
        /// operation except that the application provides the value for the parameter sourceTimestamp
        /// that is made available toDataReader objects. This timestamp is important for the interpretation
        /// of the DDS.DestinationOrderQosPolicy.
        /// </remarks>
        /// <param name="instanceData">The data to be written.</param>
        /// <param name="instanceHandle">The handle to the instance as supplied by FooDataWriter.RegisterInstance.</param>
        /// <param name="sourceTimestamp">The timestamp used.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - the value of a data instance is modified.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode BadParameter - The handle is not a valid handle or instanceData
        /// is not a valid sample.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The DataWriter has already been deleted.</item>
        /// <item>DDS.ReturnCode NotEnabled - The DataWriter is not enabled.</item>
        /// <item>DDS.ReturnCode PreconditionNotMet - The handle has not been registered with
        /// this DataWriter.</item>
        /// <item>DDS.ReturnCode Timeout - Either the current action overflowed the available resources
        /// as specified by the combination of the DDS.ReliablityQosPolicy,
        /// DDS.HistoryQosPolicy and DDS.ResourceLimitsQosPolicy, or the current action
        /// was waiting for data delivery acknowledgement by synchronous DataReaders.This caused
        /// blocking of the Write operation, which could not be resolved before MaxBlockingTime of the
        /// DDS.ReliabilityQosPolicy elapsed.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode WriteWithTimestamp(
                Foo instanceData,
                DDS.InstanceHandle instanceHandle,
                DDS.Time sourceTimestamp)
        {
            return DDS.ReturnCode.Ok;
        }

        /// <summary>
        /// This operation requests the Data Distribution Service to mark the instance for deletion.
        /// </summary>
        /// <remarks>
        /// <para>
        /// This operation requests the Data Distribution Service to mark the instance for
        /// deletion. Copies of the instance and its corresponding samples, which are stored in
        /// every connected DataReader and, dependent on the QoSPolicy settings, also in
        /// the Transient and Persistent stores, will be marked for deletion by setting their
        /// DDS.InstanceStateKind to NotAliveDisposed.
        /// </para><para>
        /// When this operation is used, the Data Distribution Service will automatically supply
        /// the value of the sourceTimestamp that is made available to connected
        /// DataReader objects. This timestamp is important for the interpretation of the
        /// DDS.DestinationOrderQosPolicy.
        /// </para><para>
        /// As a side effect, this operation asserts liveliness on the DataWriter itself and on
        /// the containing IDomainParticipant.
        /// </para><para>
        /// <b><i>Effects on DataReaders</i></b>
        /// </para><para>
        /// Actual deletion of the instance administration in a connected DataReader will be
        /// postponed until the following conditions have been met:
        /// <list type="bullet">
        /// <item>the instance must be unregistered (either implicitly or explicitly) by all connected
        /// DataWriters that have previously registered it.</item>
        /// <list>
        /// <item>A DataWriter can register an instance explicitly by using one of the special
        /// operations FooDataWriter.RegisterInstance or FooDataWriter.RegisterInstanceWithTimestamp.</item>
        /// <item>A DataWriter can register an instance implicitly by using the special constant
        /// DDS.InstanceHandle.Nil in any of the other DataWriter operations.</item>
        /// <item>A DataWriter can unregister an instance explicitly by using one of the special
        /// operations UnregisterInstance or UnregisterInstanceWithTimestamp.</item>
        /// <item>A DataWriter will unregister all its contained instances implicitly when it is
        /// deleted.</item>
        /// <item>When a DataReader detects a loss of liveliness in one of its connected
        /// DataWriters, it will consider all instances registered by that DataWriter as
        /// being implicitly unregistered.</item>
        /// </list>
        /// <item>and the application must have consumed all samples belonging to the instance,
        /// either implicitly or explicitly.</item>
        /// <list>
        /// <item>An application can consume samples explicitly by invoking the take operation,
        /// or one of its variants, on its DataReaders.</item>
        /// <item>The DataReader can consume disposed samples implicitly when the
        /// AutoPurgeDisposedSamplesDelay of the DDS.ReaderDataLifecycleQosPolicy has expired.</item>
        /// </list>
        /// </list>
        /// The DataReader may also remove instances that haven’t been disposed first: this
        /// happens when the AutopurgeNoWriterSamplesDelay of the
        /// DDS.ReaderDataLifecycleQosPolicy has expired after the instance is considered
        /// unregistered by all connected DataWriters (i.e. when it has a
        /// DDS.InstanceStateKind of NotAliveNoWriters).
        /// @see @ref DCPS_QoS_ReaderDataLifecycle.
        /// </para><para>
        /// <b><i>Effects on Transient/Persistent Stores</i></b>
        /// </para><para>
        /// Persistent stores will be postponed until the following conditions have been met:
        /// <list type="bullet">
        /// <item>the instance must be unregistered (either implicitly or explicitly) by all connected
        /// DataWriters that have previously registered it. (See above.)</item>
        /// <item>and the period of time specified by the ServiceCleanupDelay attribute in
        /// the DDS.DurabilityServiceQosPolicy on the Topic must have elapsed after the
        /// instance is considered unregistered by all connected DataWriters.</item>
        /// </list>
        /// @see @ref DCPS_QoS_DurabilityService.
        /// </para><para>
        /// <b><i>Instance Handle</i></b>
        /// </para><para>
        /// The DDS.InstanceHandle.Nil constant can be used for the parameter instanceHandle.
        /// This indicates the identity of the instance is automatically deduced from the
        /// instanceData (by means of the key).
        /// If instancHandle is any value other than DDS.InstanceHandle.Nil, it must correspond to
        /// the value that was returned by either the FooDataWriter.RegisterInstance operation or the
        /// FooDataWriter.RegisterInstanceWithTimestamp operation, when the instance (identified by
        /// its key) was registered. If there is no correspondence, the result of the operation is
        /// unspecified.
        /// The sample that is passed as instanceData is only used to check for consistency
        /// between its key values and the supplied instanceHandle: the sample itself will
        /// not actually be delivered to the connected DataReaders. Use the FooDataWriter.WriteDispose
        /// operation if the sample itself should be delivered together with the dispose request.
        /// </para><para>
        /// <b><i>Blocking</i></b>
        /// </para><para>
        /// If the DDS.HistoryQosPolicy is set to KeepAllHistoryQos, the write
        /// operation on the DataWriter may block if the modification would cause data to be
        /// lost because one of the limits, specified in the DDS.ResourceLimitsQosPolicy, is
        /// exceeded. In case the synchronous attribute value of the
        /// DDS.ReliabilityQosPolicy is set to true for communicating DataWriters and
        /// DataReaders then the DataWriter will wait until all synchronous
        /// DataReaders have acknowledged the data. Under these circumstances, the
        /// MaxBlockingTime attribute of the DDS.ReliabilityQosPolicy configures the
        /// maximum time the write operation may block (either waiting for space to become
        /// available or data to be acknowledged). If MaxBlockingTime elapses before the
        /// DataWriter is able to store the modification without exceeding the limits and all
        /// expected acknowledgements are received, the Dispose operation will fail and returns
        /// DDS.ReturnCode Timeout.
        /// </para><para>
        /// <b><i>Sample Validation</i></b>
        /// </para><para>
        /// Since the sample that is passed as instanceData is merely used to check for
        /// consistency between its key values and the supplied instanceHandle, only
        /// these keyfields will be validated against the restrictions imposed by the IDL to C#
        /// language mapping, where:
        /// <list type="bullet">
        /// <item>a string (bounded or unbounded) may not be null. (Use “” for an empty string
        /// instead)</item>
        /// <item>the length of a bounded string may not exceed the limit specified in IDL</item>
        /// <item>the length of a bounded sequence may not exceed the limit specified in IDL</item>
        /// <item>the length of an array must exactly match the size specified in IDL</item>
        /// </list>
        /// If any of these restrictions is violated, the operation will fail and return a
        /// DDS.ReturnCode BadParameter. More specific information about the context of this error will be
        /// written to the error log.
        /// </para>
        /// </remarks>
        /// <param name="instanceData">The actual instance to be disposed of.</param>
        /// <param name="instanceHandle">The handle to the instance to be disposed of.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The Data Distribution Service is informed that the instance data
        /// must be disposed of.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode BadParameter - The handle is not a valid handle or instanceData
        /// is not a valid sample.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The DataWriter has already been deleted.</item>
        /// <item>DDS.ReturnCode NotEnabled - The DataWriter is not enabled.</item>
        /// <item>DDS.ReturnCode PreconditionNotMet - The handle has not been registered with
        /// this DataWriter.</item>
        /// <item>DDS.ReturnCode Timeout - Either the current action overflowed the available resources
        /// as specified by the combination of the DDS.ReliablityQosPolicy,
        /// DDS.HistoryQosPolicy and DDS.ResourceLimitsQosPolicy, or the current action
        /// was waiting for data delivery acknowledgement by synchronous DataReaders.This caused
        /// blocking of the Dispose operation, which could not be resolved before MaxBlockingTime of the
        /// DDS.ReliabilityQosPolicy elapsed.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode Dispose(
                Foo instanceData,
                DDS.InstanceHandle instanceHandle)
        {
            return DDS.ReturnCode.Ok;
        }

        /// <summary>
        /// This operation requests the Data Distribution Service to mark the instance for deletion
        /// and provides a value for the sourceTimestamp explicitly.
        /// </summary>
        /// <remarks>
        /// This operation behaves simular to the Dispose(Foo instanceData, DDS.InstanceHandle instanceHandle)
        /// operation except that the application provides the value for the parameter sourceTimestamp
        /// that is made available to DataReader objects. This timestamp is important for the interpretation
        /// of the DDS.DestinationOrderQosPolicy.
        /// </remarks>
        /// <param name="instanceData">The data to be written.</param>
        /// <param name="instanceHandle">The handle to the instance as supplied by FooDataWriter.RegisterInstance.</param>
        /// <param name="sourceTimestamp">The timestamp used.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The Data Distribution Service is informed that the instance data
        /// must be disposed of.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode BadParameter - The handle is not a valid handle or instanceData
        /// is not a valid sample.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The DataWriter has already been deleted.</item>
        /// <item>DDS.ReturnCode NotEnabled - The DataWriter is not enabled.</item>
        /// <item>DDS.ReturnCode PreconditionNotMet - The handle has not been registered with
        /// this DataWriter.</item>
        /// <item>DDS.ReturnCode Timeout - Either the current action overflowed the available resources
        /// as specified by the combination of the DDS.ReliablityQosPolicy,
        /// DDS.HistoryQosPolicy and DDS.ResourceLimitsQosPolicy, or the current action
        /// was waiting for data delivery acknowledgement by synchronous DataReaders.This caused
        /// blocking of the DisposeWithTimestamp operation, which could not be resolved before MaxBlockingTime of the
        /// DDS.ReliabilityQosPolicy elapsed.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode DisposeWithTimestamp(
                Foo instanceData,
                DDS.InstanceHandle instanceHandle,
                DDS.Time sourceTimestamp)
        {
            return DDS.ReturnCode.Ok;
        }

        /// <summary>
        /// This operation requests the Data Distribution Service to modify the instance and
        /// mark it for deletion.
        /// </summary>
        /// <remarks>
        /// This operation behaves simular to the WriteDispose(Foo instanceData, DDS.InstanceHandle instanceHandle)
        /// operation where the instanceHandle is specified as DDS.InstanceHandle.Nil.
        /// </remarks>
        /// <param name="instanceData">The data to be written.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The Data Distribution Service has modified the instance and
        /// marked it for deletion</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode BadParameter - The handle is not a valid handle or instanceData
        /// is not a valid sample.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The DataWriter has already been deleted.</item>
        /// <item>DDS.ReturnCode NotEnabled - The DataWriter is not enabled.</item>
        /// <item>DDS.ReturnCode PreconditionNotMet - The handle has not been registered with
        /// this DataWriter.</item>
        /// <item>DDS.ReturnCode Timeout - Either the current action overflowed the available resources
        /// as specified by the combination of the DDS.ReliablityQosPolicy,
        /// DDS.HistoryQosPolicy and DDS.ResourceLimitsQosPolicy, or the current action
        /// was waiting for data delivery acknowledgement by synchronous DataReaders.This caused
        /// blocking of the Write operation, which could not be resolved before MaxBlockingTime of the
        /// DDS.ReliabilityQosPolicy elapsed.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode WriteDispose(
                Foo instanceData)
        {
            return DDS.ReturnCode.Ok;
        }

        /// <summary>
        /// This operation requests the Data Distribution Service to modify the instance and
        /// mark it for deletion.
        /// </summary>
        /// <remarks>
        /// <para>
        /// This operation requests the Data Distribution Service to modify the instance and
        /// mark it for deletion. Copies of the instance and its corresponding samples, which are
        /// stored in every connected DataReader and, dependent on the QoSPolicy settings,
        /// also in the Transient and Persistent stores, will be modified and marked for deletion
        /// by setting their DDS.InstanceStateKind to NotAliveDisposed.
        /// </para><para>
        /// When this operation is used, the Data Distribution Service will automatically supply
        /// the value of the sourceTimestamp that is made available to connected
        /// DataReader objects. This timestamp is important for the interpretation of the
        /// DDS.DestinationOrderQosPolicy.
        /// </para><para>
        /// As a side effect, this operation asserts liveliness on the DataWriter itself and on
        /// the containing IDomainParticipant.
        /// </para><para>
        /// <b><i>Effects on DataReaders</i></b>
        /// </para><para>
        /// Actual deletion of the instance administration in a connected DataReader will be
        /// postponed until the following conditions have been met:
        /// <list type="bullet">
        /// <item>the instance must be unregistered (either implicitly or explicitly) by all connected
        /// DataWriters that have previously registered it.</item>
        /// <list>
        /// <item>A DataWriter can register an instance explicitly by using one of the special
        /// operations FooDataWriter.RegisterInstance or FooDataWriter.RegisterInstanceWithTimestamp.</item>
        /// <item>A DataWriter can register an instance implicitly by using the special constant
        /// DDS.InstanceHandle.Nil in any of the other DataWriter operations.</item>
        /// <item>A DataWriter can unregister an instance explicitly by using one of the special
        /// operations UnregisterInstance or UnregisterInstanceWithTimestamp.</item>
        /// <item>A DataWriter will unregister all its contained instances implicitly when it is
        /// deleted.</item>
        /// <item>When a DataReader detects a loss of liveliness in one of its connected
        /// DataWriters, it will consider all instances registered by that DataWriter as
        /// being implicitly unregistered.</item>
        /// </list>
        /// <item>and the application must have consumed all samples belonging to the instance,
        /// either implicitly or explicitly.</item>
        /// <list>
        /// <item>An application can consume samples explicitly by invoking the take operation,
        /// or one of its variants, on its DataReaders.</item>
        /// <item>The DataReader can consume disposed samples implicitly when the
        /// AutopurgeDisposedSamplesDelay of the ReaderData
        /// DDS.LifecycleQosPolicy has expired.</item>
        /// </list>
        /// </list>
        /// The DataReader may also remove instances that haven’t been disposed first: this
        /// happens when the AutopurgeNoWriterSamplesDelay of the
        /// DDS.ReaderDataLifecycleQosPolicy has expired after the instance is considered
        /// unregistered by all connected DataWriters (i.e. when it has a
        /// DDS.InstanceStateKind of NotAliveNoWriters).
        /// @see @ref DCPS_QoS_ReaderDataLifecycle.
        /// </para><para>
        /// <b><i>Effects on Transient/Persistent Stores</i></b>
        /// </para><para>
        /// Persistent stores will be postponed until the following conditions have been met:
        /// <list type="bullet">
        /// <item>the instance must be unregistered (either implicitly or explicitly) by all connected
        /// DataWriters that have previously registered it. (See above.)</item>
        /// <item>and the period of time specified by the ServiceCleanupDelay attribute in
        /// the DDS.DurabilityServiceQosPolicy on the Topic must have elapsed after the
        /// instance is considered unregistered by all connected DataWriters.</item>
        /// </list>
        /// @see @ref DCPS_QoS_DurabilityService.
        /// </para><para>
        /// <b><i>Instance Handle</i></b>
        /// </para><para>
        /// The DDS.InstanceHandle.Nil constant can be used for the parameter instanceHandle.
        /// This indicates the identity of the instance is automatically deduced from the
        /// instanceData (by means of the key).
        /// If instancHandle is any value other than DDS.InstanceHandle.Nil, it must correspond to
        /// the value that was returned by either the FooDataWriter.RegisterInstance operation or the
        /// FooDataWriter.RegisterInstanceWithTimestamp operation, when the instance (identified by
        /// its key) was registered. If there is no correspondence, the result of the operation is
        /// unspecified.
        /// The sample that is passed as instanceData will actually be delivered to the
        /// connected DataReaders, but will immediately be marked for deletion..
        /// </para><para>
        /// <b><i>Blocking</i></b>
        /// </para><para>
        /// If the DDS.HistoryQosPolicy is set to KeepAllHistoryQos, the write
        /// operation on the DataWriter may block if the modification would cause data to be
        /// lost because one of the limits, specified in the DDS.ResourceLimitsQosPolicy, is
        /// exceeded. In case the synchronous attribute value of the
        /// DDS.ReliabilityQosPolicy is set to true for communicating DataWriters and
        /// DataReaders then the DataWriter will wait until all synchronous
        /// DataReaders have acknowledged the data. Under these circumstances, the
        /// MaxBlockingTime attribute of the DDS.ReliabilityQosPolicy configures the
        /// maximum time the write operation may block (either waiting for space to become
        /// available or data to be acknowledged). If MaxBlockingTime elapses before the
        /// DataWriter is able to store the modification without exceeding the limits and all
        /// expected acknowledgements are received, the WriteDispose operation will fail and returns
        /// DDS.ReturnCode Timeout.
        /// </para><para>
        /// <b><i>Sample Validation</i></b>
        /// </para><para>
        /// Since the sample that is passed as instanceData is merely used to check for
        /// consistency between its key values and the supplied instanceHandle, only
        /// these keyfields will be validated against the restrictions imposed by the IDL to C#
        /// language mapping, where:
        /// <list type="bullet">
        /// <item>a string (bounded or unbounded) may not be null. (Use “” for an empty string
        /// instead)</item>
        /// <item>the length of a bounded string may not exceed the limit specified in IDL</item>
        /// <item>the length of a bounded sequence may not exceed the limit specified in IDL</item>
        /// <item>the length of an array must exactly match the size specified in IDL</item>
        /// </list>
        /// If any of these restrictions is violated, the operation will fail and return a
        /// DDS.ReturnCode BadParameter. More specific information about the context of this error will be
        /// written to the error log.
        /// </para>
        /// </remarks>
        /// <param name="instanceData">The actual instance to be disposed of.</param>
        /// <param name="instanceHandle">The handle to the instance to be disposed of.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The Data Distribution Service has modified the instance and
        /// marked it for deletion.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode BadParameter - The handle is not a valid handle or instanceData
        /// is not a valid sample.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The DataWriter has already been deleted.</item>
        /// <item>DDS.ReturnCode NotEnabled - The DataWriter is not enabled.</item>
        /// <item>DDS.ReturnCode PreconditionNotMet - The handle has not been registered with
        /// this DataWriter.</item>
        /// <item>DDS.ReturnCode Timeout - Either the current action overflowed the available resources
        /// as specified by the combination of the DDS.ReliablityQosPolicy,
        /// DDS.HistoryQosPolicy and DDS.ResourceLimitsQosPolicy, or the current action
        /// was waiting for data delivery acknowledgement by synchronous DataReaders.This caused
        /// blocking of the Dispose operation, which could not be resolved before MaxBlockingTime of the
        /// DDS.ReliabilityQosPolicy elapsed.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode WriteDispose(
                Foo instanceData,
                DDS.InstanceHandle instanceHandle)
        {
            return DDS.ReturnCode.Ok;
        }

        /// <summary>
        /// This operation requests the Data Distribution Service to modify the instance and
        /// mark it for deletion and provides a value for the sourceTimestamp explicitly.
        /// </summary>
        /// <remarks>
        /// This operation behaves simular to the WriteDisposeWithTimestamp(Foo instanceData,
        /// DDS.InstanceHandle instanceHandle, DDS.Time sourceTimestamp) operation operation where
        /// the instanceHandle is specified as DDS.InstanceHandle.Nil.
        /// </remarks>
        /// <param name="instanceData">The data to be written.</param>
        /// <param name="sourceTimestamp">The timestamp used.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The Data Distribution Service is informed that the instance data
        /// must be disposed of.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode BadParameter - The handle is not a valid handle or instanceData
        /// is not a valid sample.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The DataWriter has already been deleted.</item>
        /// <item>DDS.ReturnCode NotEnabled - The DataWriter is not enabled.</item>
        /// <item>DDS.ReturnCode PreconditionNotMet - The handle has not been registered with
        /// this DataWriter.</item>
        /// <item>DDS.ReturnCode Timeout - Either the current action overflowed the available resources
        /// as specified by the combination of the DDS.ReliablityQosPolicy,
        /// DDS.HistoryQosPolicy and DDS.ResourceLimitsQosPolicy, or the current action
        /// was waiting for data delivery acknowledgement by synchronous DataReaders.This caused
        /// blocking of the DisposeWithTimestamp operation, which could not be resolved before MaxBlockingTime of the
        /// DDS.ReliabilityQosPolicy elapsed.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode WriteDisposeWithTimestamp(
                Foo instanceData,
                DDS.Time sourceTimestamp)
        {
            return DDS.ReturnCode.Ok;
        }

        /// <summary>
        /// This operation requests the Data Distribution Service to modify the instance and
        /// mark it for deletion and provides a value for the sourceTimestamp explicitly.
        /// </summary>
        /// <remarks>
        /// This operation behaves simular to the WriteDispose(Foo instanceData, DDS.InstanceHandle instanceHandle)
        /// operation except that the application provides the value for the parameter sourceTimestamp
        /// that is made available to DataReader objects. This timestamp is important for the interpretation
        /// of the DDS.DestinationOrderQosPolicy.
        /// </remarks>
        /// <param name="instanceData">The data to be written.</param>
        /// <param name="instanceHandle">The handle to the instance as supplied by FooDataWriter.RegisterInstance.</param>
        /// <param name="sourceTimestamp">The timestamp used.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The Data Distribution Service is informed that the instance data
        /// must be disposed of.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode BadParameter - The handle is not a valid handle or instanceData
        /// is not a valid sample.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The DataWriter has already been deleted.</item>
        /// <item>DDS.ReturnCode NotEnabled - The DataWriter is not enabled.</item>
        /// <item>DDS.ReturnCode PreconditionNotMet - The handle has not been registered with
        /// this DataWriter.</item>
        /// <item>DDS.ReturnCode Timeout - Either the current action overflowed the available resources
        /// as specified by the combination of the DDS.ReliablityQosPolicy,
        /// DDS.HistoryQosPolicy and DDS.ResourceLimitsQosPolicy, or the current action
        /// was waiting for data delivery acknowledgement by synchronous DataReaders.This caused
        /// blocking of the DisposeWithTimestamp operation, which could not be resolved before MaxBlockingTime of the
        /// DDS.ReliabilityQosPolicy elapsed.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode WriteDisposeWithTimestamp(
                Foo instanceData,
                DDS.InstanceHandle instanceHandle,
                DDS.Time sourceTimestamp)
        {
            return DDS.ReturnCode.Ok;
        }

        /// <summary>
        /// This operation retrieves the key value of a specific instance.
        /// </summary>
        /// <remarks>
        /// This operation retrieves the key value of the instance referenced to by
        /// instanceHandle. When the operation is called with an DDS.IstanceHandle.Nil
        /// constant as an instanceHandle, the operation will return
        /// DDS.ReturnCode BadParameter. The operation will only fill the fields that form the key
        /// inside the provide key parameter. This means that the non-key fields are not
        /// applicable and may contain garbage.
        /// </remarks>
        /// <param name="key">Reference to a sample in which the key values are stored.</param>
        /// <param name="instanceHandle">The handle to the instance from which to get the key value.</param>
        /// <returns>Return codes are:
        /// <list type="bullet">
        /// <item>DDS.ReturnCode Ok - The key values of the instance are stored in the key parameter.</item>
        /// <item>DDS.ReturnCode Error - An internal error has occurred.</item>
        /// <item>DDS.ReturnCode BadParameter - The handle is not a valid handle.</item>
        /// <item>DDS.ReturnCode AlreadyDeleted - The DataWriter has already been deleted.</item>
        /// <item>DDS.ReturnCode NotEnabled - The DataWriter is not enabled.</item>
        /// <item>DDS.ReturnCode PreconditionNotMet - The handle has not been registered with this DataWriter.</item>
        /// <item>DDS.ReturnCode OutOfResources - The DDS ran out of resources to complete this operation.</item>
        /// </list>
        /// </returns>
        public DDS.ReturnCode GetKeyValue(
                ref Foo key,
                DDS.InstanceHandle instanceHandle)
        {
            return DDS.ReturnCode.Ok;
        }

        /// <summary>
        /// This operation returns the value of the instance handle which corresponds to the
        /// provided instanceData.
        /// </summary>
        /// <remarks>
        /// <para>
        /// This operation returns the value of the instance handle which corresponds to the
        /// instanceData. The instanceData parameter is only used for the purpose of
        /// examining the fields that define the key. The instance handle can be used in any
        /// Write, Dispose or Unregister operations (or their timestamped variants) that
        /// operate on a specific instance. Note that DataWriter instance handles are local,
        /// and are not interchangeable with DataReader instance handles nor with instance
        /// handles of an other DataWriter.
        /// </para><para>
        /// This operation does not register the instance in question. If the instance has not been
        /// previously registered, if the DataWriter is already deleted or if for any other
        /// reason the Service is unable to provide an instance handle, the Service will return
        /// the special value DDS.InstanceHandle.Nil.
        /// </para><para>
        /// <b><i>Sample Validation</i></b>
        /// </para><para>
        /// Since the sample that is passed as instanceData is merely used to check for
        /// consistency between its key values and the supplied instanceHandle, only
        /// these keyfields will be validated against the restrictions imposed by the IDL to C#
        /// language mapping, where:
        /// <list type="bullet">
        /// <item>a string (bounded or unbounded) may not be null. (Use “” for an empty string
        /// instead)</item>
        /// <item>the length of a bounded string may not exceed the limit specified in IDL</item>
        /// </list>
        /// If any of these restrictions is violated, the operation will fail and return a
        /// DDS.InstanceHandle.Nil. More specific information about the context of this error will be
        /// written to the error log.
        /// </para>
        /// </remarks>
        /// <param name="instanceData">A reference to the instance for which the corresponding instance handle
        /// needs to be looked up.</param>
        /// <returns>The instance handle which corresponds to the instanceData.</returns>
        public DDS.InstanceHandle LookupInstance(
            Foo instanceData)
        {
            return DDS.InstanceHandle.Nil;
        }
    }



    /// <summary>
    /// Example of a generated typed TypeSupport, used for documentation.
    /// </summary>
    /// <remarks>
    /// @note Class hierarchy in the documentation has been simplified for clearity purposes.
    ///
    /// <code>
    /// DDS.ITypeSupport typesupport = new Space.FooTypeSupport();
    /// typesupport.RegisterType(participant, "Space.Foo");
    /// </code>
    /// </remarks>
    public class FooTypeSupport : DDS.ITypeSupport
    {
        public FooTypeSupport()
        { }
    }

} // end namespace Space

#endif // DOXYGEN_FOR_CS

