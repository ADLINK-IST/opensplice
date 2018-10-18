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
 */

using System;
using System.Runtime.InteropServices;

namespace DDS
{
    // ----------------------------------------------------------------------
    // Return codes
    // ----------------------------------------------------------------------
    /// <summary>
    /// This is the enum that represents the various ReturnCode values that DDS operations return.
    /// </summary>
    public enum ReturnCode
    {
        /// <summary>
        /// Successful return
        /// </summary>
        Ok = 0,
        /// <summary>
        /// Generic, unspecified error
        /// </summary>
        Error = 1,
        /// <summary>
        /// Unsupported operation or QosPolicy setting. Can only
        /// be returned by operations that are optional or operations
        /// that uses an optional &ltEntity&gtQoS as a parameter
        /// </summary>
        Unsupported = 2,
        /// <summary>
        /// Illegal parameter value
        /// </summary>
        BadParameter = 3,
        /// <summary>
        /// A pre-condition for the operation was not met
        /// </summary>
        PreconditionNotMet = 4,
        /// <summary>
        /// Service ran out of the resources needed to complete the operation
        /// </summary>
        OutOfResources = 5,
        /// <summary>
        /// Operation invoked on an Entity that is not yet enabled
        /// </summary>
        NotEnabled = 6,
        /// <summary>
        /// Application attempted to modify an immutable QosPolicy
        /// </summary>
        ImmutablePolicy = 7,
        /// <summary>
        /// Application specified a set of policies that are not consistent with each other
        /// </summary>
        InconsistentPolicy = 8,
        /// <summary>
        /// The object target of this operation has already been deleted
        /// </summary>
        AlreadyDeleted = 9,
        /// <summary>
        /// The operation timed out
        /// </summary>
        Timeout = 10,
        /// <summary>
        /// Indicates a situation where the operation did not return any data
        /// </summary>
        NoData = 11,
        /// <summary>
        /// An operation was invoked on an inappropriate object or
        /// at an inappropriate time (as determined by QosPolicies
        /// that control the behaviour of the object in question).
        /// There is no precondition that could be changed to make
        /// the operation succeed. This code can not be returned in C#.
        /// </summary>
        IllegalOperation = 12,
    }

    // ----------------------------------------------------------------------
    // Error codes
    // ----------------------------------------------------------------------
    /// @cond
    /// Deprecated
    public enum ErrorCode
    {
        Undefined = 0,
        Error = 1,
        OutOfResources = 2,
        CreationKernelEntityFailed = 3,
        InvalidValue = 4,
        InvalidDuration = 5,
        InvalidTime = 6,
        EntityInUse = 7,
        ContainsEntities = 8,
        EntityUnknown = 9,
        HandleNotRegistered = 10,
        HandleNotMatched = 11,
        HandleInvalid = 12,
        InvalidSequence = 13,
        UnsupportedValue = 14,
        InconsistentValue = 15,
        ImmutableQosPolicy = 16,
        InconsistentQos = 17,
        UnsupportedQosPolicy = 18,
        ContainsConditions = 19,
        ContainsLoans = 20,
        InconsistentTopic = 21,
    }
    /// @endcond

    // ----------------------------------------------------------------------
    // Status to support listeners and conditions
    // ----------------------------------------------------------------------
    /// <summary>
    /// Each concrete Entity class has a set of Status attributes and for each attribute the
    /// Entity class provides an operation to read the value. Changes to Status attributes
    /// will affect associated StatusCondition and (invoked and associated) Listener
    /// objects.The communication statuses whose changes can be communicated to the application
    /// depend on the Entity.
    /// </summary>
    [Flags]
    public enum StatusKind : uint
    {
        /// <summary>
        /// Another Topic exists with the same name but with different characteristics.
        /// </summary>
        InconsistentTopic = 0x0001 << 0,

        /// <summary>
        /// The deadline that the DataWriter has committed through its DeadlineQosPolicy
        /// was not respected for a specific instance.
        /// </summary>
        OfferedDeadlineMissed = 0x0001 << 1,

        /// <summary>
        /// The deadline that the DataReader was expecting through its DeadlineQosPolicy
        /// was not respected for a specific instance.
        /// </summary>
        RequestedDeadlineMissed = 0x0001 << 2,

        /// <summary>
        /// A QosPolicy setting was incompatible with what was requested.
        /// </summary>
        OfferedIncompatibleQos = 0x0001 << 5,

        /// <summary>
        /// A QosPolicy setting was incompatible with what is offered.
        /// </summary>
        RequestedIncompatibleQos = 0x0001 << 6,

        /// <summary>
        /// A sample has been lost (never received).
        /// </summary>
        SampleLost = 0x0001 << 7,

        /// <summary>
        /// A (received) sample has been rejected.
        /// </summary>
        SampleRejected = 0x0001 << 8,

        /// <summary>
        /// New information is available.
        /// </summary>
        DataOnReaders = 0x0001 << 9,

        /// <summary>
        /// New information is available.
        /// </summary>
        DataAvailable = 0x0001 << 10,

        /// <summary>
        /// The liveliness that the DataWriter has committed through its LivelinessQosPolicy
        /// was not respected; thus DataReader objects will consider the DataWriter as no longer "alive".
        /// </summary>
        LivelinessLost = 0x0001 << 11,

        /// <summary>
        /// The liveliness of one or more DataWriter objects that were writing instances read
        /// through the DataReader has changed. Some DataWriter have become "alive" or "not alive".
        /// </summary>
        LivelinessChanged = 0x0001 << 12,

        /// <summary>
        /// The DataWriter has found DataReader that matches the Topic and has compatible QoS.
        /// </summary>
        PublicationMatched = 0x0001 << 13,

        /// <summary>
        /// The DataReader has found a DataWriter that matches the Topic and has compatible QoS.
        /// </summary>
        SubscriptionMatched = 0x0001 << 14,

        /// <summary>
        /// The DisposeAllData() operation has been invoked on the Topic.
        /// </summary>
        AllDataDisposed = 0x80000000,

        /// <summary>
        /// Any_V1_2 status.
        /// </summary>
        Any_V1_2 = 0x7fff,

        /// <summary>
        /// Any status.
        /// </summary>
        Any = 0xffffffff
    }

    /// <summary>
    /// This struct contains the statistics about samples that have been rejected.
    /// </summary>
    public enum SampleRejectedStatusKind
    {
        /// <summary>
        /// no sample has been rejected yet
        /// </summary>
        NotRejected,
        /// <summary>
        /// the sample was rejected because it would exceed the maximum number of instances set by the ResourceLimitsQosPolicy.
        /// </summary>
        RejectedByInstanceLimit,

        /// <summary>
        /// the sample was rejected because it would exceed the maximum number of samples set by the ResourceLimits QosPolicy.
        /// </summary>
        RejectedBySamplesLimit,

        /// <summary>
        /// the sample was rejected because it would exceed the maximum number of samples per instance set by the ResourceLimitsQosPolicy.
        /// </summary>
        RejectedBySamplesPerInstanceLimit,
    };

    /// <summary>
    /// QosPolicy identification numbers
    /// </summary>
    public enum QosPolicyId
    {
        /// <summary>
        /// Invalid Policy ID
        /// </summary>
        InvalidQos = 0,
        /// <summary>
        /// Identifies to UserDataQosPolicy
        /// </summary>
        UserDataQos = 1,
        /// <summary>
        /// Identifies to DurabilityQosPolicy
        /// </summary>
        DurabilityQos = 2,
        /// <summary>
        /// Identifies to PresentationQosPolicy
        /// </summary>
        PresentationQos = 3,
        /// <summary>
        /// Identifies to DeadlineQosPolicy
        /// </summary>
        DeadlineQos = 4,
        /// <summary>
        /// Identifies to LatencyBudgetQosPolicy
        /// </summary>
        LatencyBudgetQos = 5,
        /// <summary>
        /// Identifies to OwnershipQosPolicy
        /// </summary>
        OwnershipQos = 6,
        /// <summary>
        /// Identifies to OwnershipStrengthQosPolicy
        /// </summary>
        OwnershipStrengthQos = 7,
        /// <summary>
        /// Identifies to LivelinessQosPolicy
        /// </summary>
        LivelinessQos = 8,
        /// <summary>
        /// Identifies to TimeBasedFilterQosPolicy
        /// </summary>
        TimeBasedFilterQos = 9,
        /// <summary>
        /// Identifies to PartitionQosPolicy
        /// </summary>
        PartitionQos = 10,
        /// <summary>
        /// Identifies to ReliabilityQosPolicy
        /// </summary>
        ReliabilityQos = 11,
        /// <summary>
        /// Identifies to DestinationOrderQosPolicy
        /// </summary>
        DestinationOrderQos = 12,
        /// <summary>
        /// Identifies to HistoryQosPolicy
        /// </summary>
        HistoryQos = 13,
        /// <summary>
        /// Identifies to ResourceLimitsQosPolicy
        /// </summary>
        ResourceLimitsQos = 14,
        /// <summary>
        /// Identifies to EntityFactoryQosPolicy
        /// </summary>
        EntityFactoryQos = 15,
        /// <summary>
        /// Identifies to WriterDataLifecycleQosPolicy
        /// </summary>
        WriterDataLifecycleQos = 16,
        /// <summary>
        /// Identifies to ReaderDataLifecycleQosPolicy
        /// </summary>
        ReaderDataLifecycleQos = 17,
        /// <summary>
        /// Identifies to TopicDataQosPolicy
        /// </summary>
        TopicDataQos = 18,
        /// <summary>
        /// Identifies to GroupDataQosPolicy
        /// </summary>
        GroupDataQos = 19,
        /// <summary>
        /// Identifies to TransportPriorityQosPolicy
        /// </summary>
        TransportPriorityQos = 20,
        /// <summary>
        /// Identifies to LifespanQosPolicy
        /// </summary>
        LifespanQos = 21,
        /// <summary>
        /// Identifies to DurabilityServiceQosPolicy
        /// </summary>
        DurabilityServiceQos = 22,
        /// <summary>
        /// Identifies to SchedulingQosPolicy
        /// </summary>
        SchedulingQos = 27,
    }

    /// <summary>
    /// For each sample, the Data Distribution Service internally maintains a
    /// SampleState specific to each DataReader. The SampleState can either be
    /// Read or NotRead.
    /// </summary>
    /// <remarks>
    /// @see @ref DCPS_Modules_Subscription_SampleInfo "SampleInfo" for more information
    /// </remarks>
    [Flags]
    public enum SampleStateKind
    {
        /// <summary>
        /// Indicates that the DataReader has already accessed that
        /// sample by means of read. Had the sample been accessed by take it would no
        /// longer be available to the DataReader.
        /// </summary>
        Read = 0x0001 << 0,

        /// <summary>
        /// Indicates that the DataReader has not accessed that sample before.
        /// </summary>
        NotRead = 0x0001 << 1,

        /// <summary>
        /// All flags set.
        /// </summary>
        Any = 0xffff,
    }

    /// <summary>
    /// For each instance (identified by the key), the Data Distribution Service internally
    /// maintains a ViewState relative to each DataReader. The ViewState can
    /// either be New or NotNew
    /// </summary>
    /// <remarks>
    /// @see @ref DCPS_Modules_Subscription_SampleInfo "SampleInfo" for more information
    /// </remarks>
    [Flags]
    public enum ViewStateKind
    {
        /// <summary>
        /// Indicates that either this is the first time that the DataReader
        /// has ever accessed samples of that instance, or else that the DataReader has
        /// accessed previous samples of the instance, but the instance has since been reborn
        /// (i.e. becomes not-alive and then alive again);
        /// </summary>
        New = 0x0001 << 0,

        /// <summary>
        /// Indicates that the DataReader has already accessed samples of the same instance
        /// and that the instance has not been reborn since.
        /// </summary>
        NotNew = 0x0001 << 1,

        /// <summary>
        /// All flags set.
        /// </summary>
        Any = 0xffff,
    }

    /// <summary>
    /// For each instance the Data Distribution Service internally maintains an InstanceState.
    /// </summary>
    /// <remarks>
    /// @see @ref DCPS_Modules_Subscription_SampleInfo "SampleInfo" for more information
    /// </remarks>
    [Flags]
    public enum InstanceStateKind
    {
        /// <summary>
        /// Alive indicates that:
        /// <list type="bullet">
        /// <item>samples have been received for the instance</item>
        /// <item>there are live DataWriter objects writing the instance</item>
        /// <item>the instance has not been explicitly disposed of
        /// (or else samples have been received after it was disposed of)</item>
        /// </list>
        /// </summary>
        Alive = 0x0001 << 0,

        /// <summary>
        /// The instance was disposed
        /// of by a DataWriter, either explicitly by means of the dispose operation or
        /// implicitly in case the AutoDisposeUnregisteredInstances field of the
        /// WriterDataLifecycleQosPolicy equals true when the instance gets
        /// unregistered, WriterDataLifecycleQosPolicy and no new
        /// samples for that instance have been written afterwards.
        /// </summary>
        NotAliveDisposed = 0x0001 << 1,

        /// <summary>
        /// Indicates the instance has been declared as not-alive by the
        /// IDataReader because it detected that there are no live IDataWriter objects writing that instance.
        /// </summary>
        NotAliveNoWriters = 0x0001 << 2,

        /// <summary>
        /// Both NotAliveDisposed and NotAliveNoWriters flags set.
        /// </summary>
        NotAlive = 0x006,

        /// <summary>
        /// All flags set.
        /// </summary>
        Any = 0xffff,
    }


#if DOXYGEN_FOR_CS
//
// The above compile switch is never (and must never) be defined in normal compilation.
//
// QoS and Policy related enums are part of the generated code for builtin topics.
// They are repeated here for easy documentation generation.
//


    public enum DurabilityQosPolicyKind
    {
        /// <summary>
        /// The samples are not available to late-joining
        /// DataReaders. In other words, only DataReaders, which were present at the
        /// time of the writing and have subscribed to this Topic, will receive the sample.
        /// When a DataReader subscribes afterwards (late-joining), it will only be able to
        /// read the next written sample. This setting is typically used for data, which is
        /// updated quickly.
        /// </summary>
        VolatileDurabilityQos,

        /// <summary>
        /// Currently behaves identically to the
        /// TransientDurabilityQos, except for its RxO properties. The desired
        /// behaviour of TransientLocalDurabilityQos can be achieved from the
        /// TransientDurabilityQos with the default (true) setting of the
        /// AutodisposeUnregisteredInstances flag on the DataWriter and the
        /// ServiceCleanupDelay set to 0 on the durability service. This is because for
        /// TransientLocal, the data should only remain available for late-joining
        /// readers during the lifetime of its source writer, so it is not required to survive after
        /// its source writer has been deleted. Since the deletion of a writer implicitly
        /// unregisters all its instances, an AutodisposeUnregisteredInstances
        /// value of true will also dispose the affected data from the durability store, and
        /// thus prevent it from remaining available to late joining readers.
        /// </summary>
        TransientLocalDurabilityQos,

        /// <summary>
        /// Some samples are available to late-joining
        /// DataReaders (stored in memory). This means that the late-joining
        /// DataReaders are able to read these previously written samples. The
        /// DataReader does not necessarily have to exist at the time of writing. Not all
        /// samples are stored (depending on QosPolicy History and QosPolicy
        /// ResourceLimits). The storage does not depend on the DataWriter and will
        /// outlive the DataWriter. This may be used to implement reallocation of
        /// applications because the data is saved in the Data Distribution Service (not in the
        /// DataWriter). This setting is typically used for state related information of an
        /// application. In this case also the DurabilityServiceQosPolicy settings are
        /// relevant for the behaviour of the Data Distribution Service.
        /// </summary>
        TransientDurabilityQos,

        /// <summary>
        /// The data is stored in permanent storage (e.g.
        /// hard disk). This means that the samples are also available after a system restart.
        /// The samples not only outlives the DataWriters, but even the Data Distribution
        /// Service and the system. This setting is typically used for attributes and settings for
        /// an application or the system. In this case also the
        /// DurabilityServiceQosPolicy settings are relevant for the behaviour of the
        /// Data Distribution Service.
        /// </summary>
        PersistentDurabilityQos
    }

    public enum PresentationQosPolicyAccessScopeKind
    {
        /// <summary>
        /// Presentation Access Scope is per instance.
        /// </summary>
        InstancePresentationQos,

        /// <summary>
        /// Presentation Access Scope is per topic.
        /// </summary>
        TopicPresentationQos,

        /// <summary>
        /// Presentation Access Scope is per group.
        /// </summary>
        GroupPresentationQos
    }

    public enum OwnershipQosPolicyKind
    {
        /// <summary>
        /// The same instance can be written by
        /// multiple DataWriter objects. All updates will be made available to the
        /// DataReader objects. In other words it does not have a specific owner.
        /// </summary>
        SharedOwnershipQos,

        /// <summary>
        /// The instance will only be accepted from one
        /// DataWriter which is the only one whose modifications will be visible to the
        /// DataReader objects.
        /// </summary>
        ExclusiveOwnershipQos
    }

    public enum LivelinessQosPolicyKind
    {
        /// <summary>
        /// The Data Distribution Service will take care of
        /// reporting the Liveliness automatically with a rate determined by the
        /// LeaseDuration.
        /// </summary>
        AutomaticLivelinessQos,

        /// <summary>
        /// The application must take care
        /// of reporting the liveliness before the LeaseDuration expires. If an Entity
        /// reports its liveliness, all Entities within the same DomainParticipant that
        /// have their liveliness kind set to ManualByParticipantLivelinessQos,
        /// can be considered alive by the Data Distribution Service. Liveliness can reported
        /// explicitly by calling the operation AssertLiveliness on the
        /// DomainParticipant or implicitly by writing some data.
        /// </summary>
        ManualByParticipantLivelinessQos,

        /// <summary>
        /// The application must take care of
        /// reporting the liveliness before the LeaseDuration expires. This can explicitly
        /// be done by calling the operation AssertLiveliness on the DataWriter or
        /// implicitly by writing some data.
        /// </summary>
        ManualByTopicLivelinessQos
    }

    public enum ReliabilityQosPolicyKind
    {
        /// <summary>
        /// The Data Distribution Service will only
        /// attempt to deliver the data; no arrival-checks are being performed and any lost
        /// data is not re-transmitted (non-reliable). Presumably new values for the samples
        /// are generated often enough by the application so that it is not necessary to resent
        /// or acknowledge any samples.
        /// </summary>
        BestEffortReliabilityQos,

        /// <summary>
        /// The Data Distribution Service will attempt to
        /// deliver all samples in the DataWriters history; arrival-checks are performed
        /// and data may get re-transmitted in case of lost data. In the steady-state (no
        /// modifications communicated via the DataWriter) the Data Distribution Service
        /// guarantees that all samples in the DataWriter history will eventually be
        /// delivered to the all DataReader objects. Outside the steady-state the
        /// HistoryQosPolicy and ResourceLimitsQosPolicy determine how
        /// samples become part of the history and whether samples can be discarded from it.
        /// In this case also the MaxBlockingTime must be set.
        /// </summary>
        ReliableReliabilityQos
    }

    public enum DestinationOrderQosPolicyKind
    {
        /// <summary>
        /// The order is based on the timestamp, at the moment the sample was
        /// received by the DataReader.
        /// </summary>
        ByReceptionTimestampDestinationOrderQos,

        /// <summary>
        /// The order is based on the timestamp, which was set by the
        /// DataWriter. This means that the system needs some time synchronization.
        /// </summary>
        BySourceTimestampDestinationOrderQos
    }

    public enum HistoryQosPolicyKind
    {
        /// <summary>
        /// The Data Distribution Service will only attempt to
        /// keep the latest values of the instance and discard the older ones. The attribute
        /// “depth” determines how many samples in history will be stored. In other words,
        /// only the most recent samples in history are stored. On the publishing side, the
        /// Data Distribution Service will only keep the most recent “depth” samples of each
        /// instance of data (identified by its key) managed by the DataWriter. On the
        /// subscribing side, the DataReader will only keep the most recent “depth”
        /// samples received for each instance (identified by its key) until the application
        /// “takes” them via the DataReader take operation.
        /// KeepLastHistoryQos - is the default kind. The default value of depth is
        /// 1, indicating that only the most recent value should be delivered. If a depth other
        /// than 1 is specified, it should be compatible with the settings of the
        /// ResourcelimitsQosPolicy MaxSamplesPerInstance. For these two
        /// QosPolicy settings to be compatible, they must verify that depth <=
        /// MaxSamplesPerInstance, otherwise a
        /// DDS.ReturnCode InconsistentPolicy is generated on relevant operations.
        /// </summary>
        KeepLastHistoryQos,

        /// <summary>
        /// All samples are stored, provided, the resources are
        /// available. On the publishing side, the Data Distribution Service will attempt to
        /// keep all samples (representing each value written) of each instance of data
        /// (identified by its key) managed by the DataWriter until they can be delivered to
        /// all subscribers. On the subscribing side, the Data Distribution Service will
        /// attempt to keep all samples of each instance of data (identified by its key)
        /// managed by the DataReader. These samples are kept until the application
        /// “takes” them from the Data Distribution Service via the DataReader take
        /// operation. The setting of depth has no effect. Its implied value is
        /// Length.Unlimited. The resources that the Data Distribution Service can use to
        /// keep this history are limited by the settings of the ResourceLimitsQosPolicy.
        /// If the limit is reached, the behaviour of the Data Distribution Service will depend
        /// on the ReliabilityQosPolicy. If the ReliabilityQosPolicy is
        /// BestEffortReliabilityQos, the old values are discarded. If
        /// ReliabilityQosPolicy is ReliableReliabilityQos, the Data
        /// Distribution Service will block the DataWriter until it can deliver the necessary
        /// old values to all subscribers.
        /// </summary>
        KeepAllHistoryQos
    }

    /**
     * @note Proprietary policy enumeration
     */
    public enum SchedulingClassQosPolicyKind
    {
        /// <summary>
        /// Underlying OS default scheduling.
        /// </summary>
        ScheduleDefault,

        /// <summary>
        /// Time-sharing scheduling (whether this is supported depends
        /// on the underlying OS).
        /// </summary>
        ScheduleTimesharing,

        /// <summary>
        /// Real-time scheduling (whether this is supported depends
        /// on the underlying OS).
        /// </summary>
        ScheduleRealtime
    }

    /**
     * @note Proprietary policy enumeration
     */
    public enum SchedulingPriorityQosPolicyKind
    {
        /// <summary>
        /// The given scheduling priority for the thread (in
        /// SchedulingQosPolicy) is relative to the
        /// process priority.
        /// </summary>
        PriorityRelative,

        /// <summary>
        /// The given scheduling priority for the thread (in
        /// SchedulingQosPolicy) is an absolute
        /// value.
        /// </summary>
        PriorityAbsolute
    }

    /// <remarks>
    /// @note Proprietary policy enumeration
    ///
    /// A normal dispose results in an event. There is no sample accompanying the event.
    ///
    /// Although on the receiver side the event is processed correctly and the instance state
    /// is modified accordingly, there is still a problem representing the event since there
    /// is no valid sample accompanying it. That is why an event will always try to piggyback
    /// on valid samples that still happen to be around in the reader.
    ///
    /// This InvalidSampleVisibilityQosPolicy (proprietary part of the
    /// ReaderDataLifecycleQosPolicy) is used to decide what to do when the instance
    /// in question has no valid sample to piggyback the event.
    /// <remarks>
    public enum InvalidSampleVisibilityQosPolicyKind
    {
        /// <summary>
        /// In this case the event is lost, since there is no valid
        /// sample to piggyback on, and no invalid samples may be
        /// created to accompany the event.
        /// </summary>
        NoInvalidSamples,

        /// <summary>
        /// This is the default setting. In this case one (and only
        /// one) invalid sample is created to piggyback all pending
        /// events on. The sample is only displayed once, and is
        /// then discarded. So even when you perform a read
        /// operation, the invalid sample will be consumed as
        /// soon as it is accessed.
        /// </summary>
        MinimumInvalidSamples,

        /// <summary>
        /// This is a new setting that we have envisaged, but not
        /// yet implemented. Currently it will throw an
        /// UnsupportedError. The idea is that when implemented,
        /// every event is represented by its own invalid sample.
        /// Keep in mind that invalid samples have no real content,
        /// and therefore do not consume resource limits. That
        /// means a KeepLast reader with depth 1 can contain 1
        /// valid sample and any number of pending events. This
        /// allows you to see each event separately and removes
        /// the need to maintain administration for the last known
        /// DisposeCount. Also since every event will have its own
        /// accompanying sample (even in case there are still valid
        /// samples around), each event will have a NotRead sample
        /// state which makes it easy to intercept with a single
        /// ReadCondition for NotRead data.
        /// </summary>
        AllInvalidSamples
    };

#endif // DOXYGEN_FOR_CS

} // end namespace DDS
