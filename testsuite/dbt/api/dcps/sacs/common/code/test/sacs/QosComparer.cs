namespace test.sacs
{
    /// <date>Jun 2, 2005</date>
    public class QosComparer
    {
        public static readonly DDS.DeadlineQosPolicy defaultDeadlineQosPolicy = new DDS.DeadlineQosPolicy();
        public static readonly DDS.DestinationOrderQosPolicy defaultDestinationOrderQosPolicy = new DDS.DestinationOrderQosPolicy();
        public static readonly DDS.DurabilityQosPolicy defaultDurabilityQosPolicy = new DDS.DurabilityQosPolicy();
        public static readonly DDS.DurabilityServiceQosPolicy defaultDurabilityServiceQosPolicy = new DDS.DurabilityServiceQosPolicy();
        public static readonly DDS.EntityFactoryQosPolicy defaultEntityFactoryQosPolicy = new DDS.EntityFactoryQosPolicy();
        public static readonly DDS.GroupDataQosPolicy defaultGroupDataQosPolicy = new DDS.GroupDataQosPolicy();
        public static readonly DDS.HistoryQosPolicy defaultHistoryQosPolicy = new DDS.HistoryQosPolicy();
        public static readonly DDS.LatencyBudgetQosPolicy defaultLatencyBudgetQosPolicy = new DDS.LatencyBudgetQosPolicy();
        public static readonly DDS.LifespanQosPolicy defaultLifespanQosPolicy = new DDS.LifespanQosPolicy();
        public static readonly DDS.LivelinessQosPolicy defaultLivelinessQosPolicy = new DDS.LivelinessQosPolicy();
        public static readonly DDS.OwnershipQosPolicy defaultOwnershipQosPolicy = new DDS.OwnershipQosPolicy();
        public static readonly DDS.OwnershipStrengthQosPolicy defaultOwnershipStrengthQosPolicy = new DDS.OwnershipStrengthQosPolicy();
        public static readonly DDS.PartitionQosPolicy defaultPartitionQosPolicy = new DDS.PartitionQosPolicy();
        public static readonly DDS.PresentationQosPolicy defaultPresentationQosPolicy = new DDS.PresentationQosPolicy();
        public static readonly DDS.ReaderDataLifecycleQosPolicy defaultReaderDataLifecycleQosPolicy = new DDS.ReaderDataLifecycleQosPolicy();
        public static readonly DDS.ReliabilityQosPolicy defaultTopicAndDataReaderReliabilityQosPolicy = new DDS.ReliabilityQosPolicy();
        public static readonly DDS.ReliabilityQosPolicy defaultDataWriterReliabilityQosPolicy = new DDS.ReliabilityQosPolicy();
        public static readonly DDS.ResourceLimitsQosPolicy defaultResourceLimitsQosPolicy = new DDS.ResourceLimitsQosPolicy();
        public static readonly DDS.TimeBasedFilterQosPolicy defaultTimeBasedFilterQosPolicy = new DDS.TimeBasedFilterQosPolicy();
        public static readonly DDS.TopicDataQosPolicy defaultTopicDataQosPolicy = new DDS.TopicDataQosPolicy();
        public static readonly DDS.TransportPriorityQosPolicy defaultTransportPriorityQosPolicy = new DDS.TransportPriorityQosPolicy();
        public static readonly DDS.UserDataQosPolicy defaultUserDataQosPolicy = new DDS.UserDataQosPolicy();
        public static readonly DDS.WriterDataLifecycleQosPolicy defaultWriterDataLifecycleQosPolicy = new DDS.WriterDataLifecycleQosPolicy();

        public static readonly DDS.SchedulingQosPolicy defaultSchedulingQosPolicy = new DDS.SchedulingQosPolicy ();
        public static readonly DDS.ShareQosPolicy defaultShareQosPolicy = new DDS.ShareQosPolicy ();

        /* Default QoSs */
        public static readonly DDS.DomainParticipantQos defaultDomainParticipantQos = new DDS.DomainParticipantQos ();
        public static readonly DDS.TopicQos defaultTopicQos = new DDS.TopicQos ();
        public static readonly DDS.SubscriberQos defaultSubscriberQos = new DDS.SubscriberQos ();
        public static readonly DDS.DataWriterQos defaultDataWriterQos = new DDS.DataWriterQos ();
        public static readonly DDS.PublisherQos defaultPublisherQos = new DDS.PublisherQos ();
        public static readonly DDS.DataReaderQos defaultDataReaderQos = new DDS.DataReaderQos ();

        static QosComparer()
        {
            defaultDeadlineQosPolicy.Period = DDS.Duration.Infinite;

            defaultDestinationOrderQosPolicy.Kind = DDS.DestinationOrderQosPolicyKind.ByReceptionTimestampDestinationorderQos;

            defaultDurabilityQosPolicy.Kind = DDS.DurabilityQosPolicyKind.VolatileDurabilityQos;

            defaultDurabilityServiceQosPolicy.ServiceCleanupDelay = DDS.Duration.Zero;
            defaultDurabilityServiceQosPolicy.HistoryKind = DDS.HistoryQosPolicyKind.KeepLastHistoryQos;
            defaultDurabilityServiceQosPolicy.HistoryDepth = 1;
            defaultDurabilityServiceQosPolicy.MaxInstances = -1;
            defaultDurabilityServiceQosPolicy.MaxSamples = -1;
            defaultDurabilityServiceQosPolicy.MaxSamplesPerInstance = -1;

            defaultEntityFactoryQosPolicy.AutoenableCreatedEntities = true;

            defaultGroupDataQosPolicy.Value = new byte[0];

            defaultHistoryQosPolicy.Kind = DDS.HistoryQosPolicyKind.KeepLastHistoryQos;
            defaultHistoryQosPolicy.Depth = 1;

            defaultLatencyBudgetQosPolicy.Duration = DDS.Duration.Zero;

            defaultLifespanQosPolicy.Duration = DDS.Duration.Infinite;

            defaultLivelinessQosPolicy.Kind = DDS.LivelinessQosPolicyKind.AutomaticLivelinessQos;
            defaultLivelinessQosPolicy.LeaseDuration = DDS.Duration.Infinite;

            defaultOwnershipQosPolicy.Kind = DDS.OwnershipQosPolicyKind.SharedOwnershipQos;

            defaultOwnershipStrengthQosPolicy.Value = 0;

            defaultPartitionQosPolicy.Name = new string[0];

            defaultPresentationQosPolicy.AccessScope = DDS.PresentationQosPolicyAccessScopeKind.InstancePresentationQos;
            defaultPresentationQosPolicy.CoherentAccess = false;
            defaultPresentationQosPolicy.OrderedAccess = false;

            defaultReaderDataLifecycleQosPolicy.AutopurgeDisposedSamplesDelay = DDS.Duration.Infinite;
            defaultReaderDataLifecycleQosPolicy.AutopurgeNowriterSamplesDelay = DDS.Duration.Infinite;
            defaultReaderDataLifecycleQosPolicy.EnableInvalidSamples = true;

            defaultTopicAndDataReaderReliabilityQosPolicy.Kind = DDS.ReliabilityQosPolicyKind.BestEffortReliabilityQos;
            defaultTopicAndDataReaderReliabilityQosPolicy.MaxBlockingTime = new DDS.Duration(0, 100000000);

            defaultDataWriterReliabilityQosPolicy.Kind = DDS.ReliabilityQosPolicyKind.ReliableReliabilityQos;
            defaultDataWriterReliabilityQosPolicy.MaxBlockingTime = new DDS.Duration(0, 100000000);

            defaultResourceLimitsQosPolicy.MaxInstances = -1;
            defaultResourceLimitsQosPolicy.MaxSamples = -1;
            defaultResourceLimitsQosPolicy.MaxSamplesPerInstance = -1;

            defaultTimeBasedFilterQosPolicy.MinimumSeparation = DDS.Duration.Zero;

            defaultTopicDataQosPolicy.Value = new byte[0];

            defaultTransportPriorityQosPolicy.Value = 0;

            defaultUserDataQosPolicy.Value = new byte[0];

            defaultWriterDataLifecycleQosPolicy.AutodisposeUnregisteredInstances = true;

            defaultSchedulingQosPolicy.SchedulingClass.Kind = DDS.SchedulingClassQosPolicyKind.ScheduleDefault;
            defaultSchedulingQosPolicy.SchedulingPriorityKind.Kind = DDS.SchedulingPriorityQosPolicyKind.PriorityRelative;
            defaultSchedulingQosPolicy.SchedulingPriority = 0;

            defaultShareQosPolicy.Name = string.Empty;
            defaultShareQosPolicy.Enable = false;

            defaultDataWriterQos.Durability = defaultDurabilityQosPolicy;
            defaultDataWriterQos.Deadline = defaultDeadlineQosPolicy;
            defaultDataWriterQos.LatencyBudget = defaultLatencyBudgetQosPolicy;
            defaultDataWriterQos.Liveliness = defaultLivelinessQosPolicy;
            defaultDataWriterQos.Reliability = defaultDataWriterReliabilityQosPolicy;
            defaultDataWriterQos.DestinationOrder = defaultDestinationOrderQosPolicy;
            defaultDataWriterQos.History = defaultHistoryQosPolicy;
            defaultDataWriterQos.ResourceLimits = defaultResourceLimitsQosPolicy;
            defaultDataWriterQos.TransportPriority = defaultTransportPriorityQosPolicy;
            defaultDataWriterQos.Lifespan = defaultLifespanQosPolicy;
            defaultDataWriterQos.UserData = defaultUserDataQosPolicy;
            defaultDataWriterQos.Ownership = defaultOwnershipQosPolicy;
            defaultDataWriterQos.OwnershipStrength = defaultOwnershipStrengthQosPolicy;
            defaultDataWriterQos.WriterDataLifecycle = defaultWriterDataLifecycleQosPolicy;

            defaultSubscriberQos.Presentation = defaultPresentationQosPolicy;
            defaultSubscriberQos.Partition = defaultPartitionQosPolicy;
            defaultSubscriberQos.GroupData = defaultGroupDataQosPolicy;
            defaultSubscriberQos.EntityFactory = defaultEntityFactoryQosPolicy;
            defaultSubscriberQos.Share = defaultShareQosPolicy;

            defaultDataReaderQos.Durability = defaultDurabilityQosPolicy;
            defaultDataReaderQos.Deadline = defaultDeadlineQosPolicy;
            defaultDataReaderQos.LatencyBudget = defaultLatencyBudgetQosPolicy;
            defaultDataReaderQos.Liveliness = defaultLivelinessQosPolicy;
            defaultDataReaderQos.Reliability = defaultTopicAndDataReaderReliabilityQosPolicy;
            defaultDataReaderQos.DestinationOrder = defaultDestinationOrderQosPolicy;
            defaultDataReaderQos.History = defaultHistoryQosPolicy;
            defaultDataReaderQos.ResourceLimits = defaultResourceLimitsQosPolicy;
            defaultDataReaderQos.UserData = defaultUserDataQosPolicy;
            defaultDataReaderQos.Ownership = defaultOwnershipQosPolicy;
            defaultDataReaderQos.TimeBasedFilter = defaultTimeBasedFilterQosPolicy;
            defaultDataReaderQos.ReaderDataLifecycle = defaultReaderDataLifecycleQosPolicy;

            defaultTopicQos.TopicData = defaultTopicDataQosPolicy;
            defaultTopicQos.Durability = defaultDurabilityQosPolicy;
            defaultTopicQos.DurabilityService = defaultDurabilityServiceQosPolicy;
            defaultTopicQos.Deadline = defaultDeadlineQosPolicy;
            defaultTopicQos.LatencyBudget = defaultLatencyBudgetQosPolicy;
            defaultTopicQos.Liveliness = defaultLivelinessQosPolicy;
            defaultTopicQos.Reliability = defaultTopicAndDataReaderReliabilityQosPolicy;
            defaultTopicQos.DestinationOrder = defaultDestinationOrderQosPolicy;
            defaultTopicQos.History = defaultHistoryQosPolicy;
            defaultTopicQos.ResourceLimits = defaultResourceLimitsQosPolicy;
            defaultTopicQos.TransportPriority = defaultTransportPriorityQosPolicy;
            defaultTopicQos.Lifespan = defaultLifespanQosPolicy;
            defaultTopicQos.Ownership = defaultOwnershipQosPolicy;

            /* Initialize default DDS.DomainParticipantQos */
            defaultDomainParticipantQos.UserData = defaultUserDataQosPolicy;
            defaultDomainParticipantQos.EntityFactory = defaultEntityFactoryQosPolicy;
            defaultDomainParticipantQos.WatchdogScheduling = defaultSchedulingQosPolicy;
            defaultDomainParticipantQos.ListenerScheduling = defaultSchedulingQosPolicy;

            /* Initialize default DDS.PublisherQos */
            defaultPublisherQos.Presentation = defaultPresentationQosPolicy;
            defaultPublisherQos.Partition = defaultPartitionQosPolicy;
            defaultPublisherQos.GroupData = defaultGroupDataQosPolicy;
            defaultPublisherQos.EntityFactory = defaultEntityFactoryQosPolicy;
        }

        /// <summary>Compares all attributes of two DomainParticipantQos objects for equality.</summary>
        /// <remarks>Compares all attributes of two DomainParticipantQos objects for equality.</remarks>
        /// <param name="qos1">DomainParticipantQos object to compare.</param>
        /// <param name="qos2">DomainParticipantQos object to compare agains.</param>
        /// <returns><code>true</code> if the two qosses are equal, otherwise <code>false</code>.
        ///     </returns>
        public static bool DomainParticipantQosEquals(
            DDS.DomainParticipantQos a,
            DDS.DomainParticipantQos b)
        {
            if (!UserDataQosPolicyEquals (a.UserData, b.UserData)) {
                System.Console.Error.WriteLine ("'DDS.DomainParticipantQos.UserData' values do not match");
                return false;
            }
            if (!EntityFactoryQosPolicyEquals (a.EntityFactory, b.EntityFactory)) {
                System.Console.Error.WriteLine ("'DDS.DomainParticipantQos.EntityFactory' values do not match");
                return false;
            }
            if (!SchedulingQosPolicyEquals (a.WatchdogScheduling, b.WatchdogScheduling)) {
                System.Console.Error.WriteLine ("'DDS.DomainParticipantQos.WatchdogScheduling' values do not match");
                return false;
            }
            if (!SchedulingQosPolicyEquals (a.ListenerScheduling, b.ListenerScheduling)) {
                System.Console.Error.WriteLine ("'DDS.DomainParticipantQos.ListenerScheduling' values do not match");
                return false;
            }
            return true;
        }

        /// <summary>Compares all attributes of two TopicQos objects for equality.</summary>
        /// <remarks>Compares all attributes of two TopicQos objects for equality.</remarks>
        /// <param name="qos1">First Qos to compare.</param>
        /// <param name="qos2">Second Qos to compare.</param>
        /// <returns><code>true</code> if the two qosses are equal, otherwise <code>false</code>.
        /// 	</returns>
        public static bool TopicQosEquals(DDS.TopicQos qos1, DDS.TopicQos qos2)
        {
            if (!DurationEquals(qos1.Deadline.Period, qos2.Deadline.Period))
            {
                System.Console.Error.WriteLine("'Deadline.Period' values do not match");
                return false;
            }
            if (qos1.DestinationOrder.Kind != qos2.DestinationOrder.Kind)
            {
                System.Console.Error.WriteLine("'DestinationOrder.Kind' values do not match");
                return false;
            }
            if (qos1.Durability.Kind != qos2.Durability.Kind)
            {
                System.Console.Error.WriteLine("'Durability.Kind' values do not match");
                return false;
            }
            if (!DurationEquals(qos1.DurabilityService.ServiceCleanupDelay, qos2.DurabilityService.ServiceCleanupDelay))
            {
                System.Console.Error.WriteLine("'DurabilityService.ServiceCleanupDelay' values do not match");
                return false;
            }
            if (qos1.DurabilityService.HistoryDepth != qos2.DurabilityService.HistoryDepth)
            {
                System.Console.Error.WriteLine("'DurabilityService.HistoryDepth' values do not match"
                    );
                return false;
            }
            if (qos1.DurabilityService.HistoryKind != qos2.DurabilityService.HistoryKind)
            {
                System.Console.Error.WriteLine("'DurabilityService.HistoryKind' values do not match"
                    );
                return false;
            }
            if (qos1.DurabilityService.MaxInstances != qos2.DurabilityService.MaxInstances)
            {
                System.Console.Error.WriteLine("'DurabilityService.MaxInstances' values do not match"
                    );
                return false;
            }
            if (qos1.DurabilityService.MaxSamples != qos2.DurabilityService.MaxSamples)
            {
                System.Console.Error.WriteLine("'DurabilityService.MaxSamples' values do not match"
                    );
                return false;
            }
            if (qos1.DurabilityService.MaxSamplesPerInstance != qos2.DurabilityService.MaxSamplesPerInstance)
            {
                System.Console.Error.WriteLine("'DurabilityService.MaxSamplesPerInstance' values do not match"
                    );
                return false;
            }
            if (qos1.History.Depth != qos2.History.Depth)
            {
                System.Console.Error.WriteLine("'History.Depth' values do not match");
                return false;
            }
            if (qos1.History.Kind != qos2.History.Kind)
            {
                System.Console.Error.WriteLine("'History.Kind' values do not match");
                return false;
            }
            if (!DurationEquals(qos1.LatencyBudget.Duration, qos2.LatencyBudget.Duration))
            {
                System.Console.Error.WriteLine("'LatencyBudget.Duration' values do not match");
                return false;
            }
            if (!DurationEquals(qos1.Lifespan.Duration, qos2.Lifespan.Duration))
            {
                System.Console.Error.WriteLine("'Lifespan.Duration' values do not match");
                return false;
            }
            if (qos1.Liveliness.Kind != qos2.Liveliness.Kind)
            {
                System.Console.Error.WriteLine("'Liveliness.Kind' values do not match");
                return false;
            }
            if (!DurationEquals(qos1.Liveliness.LeaseDuration, qos2.Liveliness.LeaseDuration
                ))
            {
                System.Console.Error.WriteLine("'Liveliness.LeaseDuration' values do not match");
                return false;
            }
            if (qos1.Ownership.Kind != qos2.Ownership.Kind)
            {
                System.Console.Error.WriteLine("'Ownership.Kind' values do not match");
                return false;
            }
            if (qos1.Reliability.Kind != qos2.Reliability.Kind)
            {
                System.Console.Error.WriteLine("'Reliability.Kind' values do not match");
                return false;
            }
            if (!DurationEquals(qos1.Reliability.MaxBlockingTime, qos2.Reliability.MaxBlockingTime
                ))
            {
                System.Console.Error.WriteLine("'Reliability.MaxBlockingTime' values do not match"
                    );
                return false;
            }
            if (qos1.ResourceLimits.MaxInstances != qos2.ResourceLimits.MaxInstances)
            {
                System.Console.Error.WriteLine("'ResourceLimits.MaxInstances' values do not match"
                    );
                return false;
            }
            if (qos1.ResourceLimits.MaxSamples != qos2.ResourceLimits.MaxSamples)
            {
                System.Console.Error.WriteLine("'ResourceLimits.MaxSamples' values do not match"
                    );
                return false;
            }
            if (qos1.ResourceLimits.MaxSamplesPerInstance != qos2.ResourceLimits.MaxSamplesPerInstance)
            {
                System.Console.Error.WriteLine("'ResourceLimits.MaxSamplesPerInstance' values do not match"
                    );
                return false;
            }
            if (!ByteArrayEquals(qos1.TopicData.Value, qos2.TopicData.Value))
            {
                System.Console.Error.WriteLine("'TopicData.Value' values do not match");
                return false;
            }
            if (qos1.TransportPriority.Value != qos2.TransportPriority.Value)
            {
                System.Console.Error.WriteLine("'TransportPriority.Value' values do not match");
                return false;
            }
            return true;
        }

        public static bool SubscriberQosEquals(DDS.SubscriberQos qos1, DDS.SubscriberQos
            qos2)
        {
            if (qos1.EntityFactory.AutoenableCreatedEntities != qos2.EntityFactory.AutoenableCreatedEntities)
            {
                System.Console.Error.WriteLine("'EntityFactory.AutoEnableCreatedEntities' values do not match"
                    );
                return false;
            }
            if (!ByteArrayEquals(qos1.GroupData.Value, qos1.GroupData.Value))
            {
                System.Console.Error.WriteLine("'GroupData.Value' values do not match");
                return false;
            }
            if (!StringArrayEquals(qos1.Partition.Name, qos2.Partition.Name))
            {
                System.Console.Error.WriteLine("'Partition.Name' values do not match");
                return false;
            }
            if (qos1.Presentation.CoherentAccess != qos2.Presentation.CoherentAccess)
            {
                System.Console.Error.WriteLine("'Presentation.CoherentAccess' values do not match"
                    );
                return false;
            }
            if (qos1.Presentation.OrderedAccess != qos2.Presentation.OrderedAccess)
            {
                System.Console.Error.WriteLine("'Presentation.OrderedAccess' values do not match"
                    );
                return false;
            }
            if (qos1.Presentation.AccessScope != qos2.Presentation.AccessScope)
            {
                System.Console.Error.WriteLine("'Presentation.AccessScope' values do not match");
                return false;
            }
            return true;
        }

        public static bool DataReaderQosEquals(DDS.DataReaderQos qos1, DDS.DataReaderQos qos2)
        {
            if (qos1.Durability.Kind != qos2.Durability.Kind)
            {
                System.Console.Error.WriteLine("'Durability.Kind' values do not match");
                return false;
            }
            if (!DurationEquals(qos1.Deadline.Period, qos2.Deadline.Period))
            {
                System.Console.Error.WriteLine("'Deadline.Period' values do not match");
                return false;
            }
            if (!DurationEquals(qos1.LatencyBudget.Duration, qos2.LatencyBudget.Duration))
            {
                System.Console.Error.WriteLine("'LatencyBudget.Duration' values do not match");
                return false;
            }
            if (qos1.Liveliness.Kind != qos2.Liveliness.Kind)
            {
                System.Console.Error.WriteLine("'Liveliness.Kind' values do not match");
                return false;
            }
            if (!DurationEquals(qos1.Liveliness.LeaseDuration, qos2.Liveliness.LeaseDuration
                ))
            {
                System.Console.Error.WriteLine("'Liveliness.LeaseDuration' values do not match");
                return false;
            }
            if (qos1.Reliability.Kind != qos2.Reliability.Kind)
            {
                System.Console.Error.WriteLine("'Liveliness.Kind' values do not match");
                return false;
            }
            if (!DurationEquals(qos1.Reliability.MaxBlockingTime, qos2.Reliability.MaxBlockingTime
                ))
            {
                System.Console.Error.WriteLine("'Reliability.MaxBlockingTime' values do not match"
                    );
                return false;
            }
            if (qos1.DestinationOrder.Kind != qos2.DestinationOrder.Kind)
            {
                System.Console.Error.WriteLine("'DestinationOrder.Kind' values do not match");
                return false;
            }
            if (qos1.History.Kind != qos2.History.Kind)
            {
                System.Console.Error.WriteLine("'history.Kind' values do not match");
                return false;
            }
            if (qos1.History.Depth != qos2.History.Depth)
            {
                System.Console.Error.WriteLine("'History.Depth' values do not match");
                return false;
            }
            if (qos1.ResourceLimits.MaxInstances != qos2.ResourceLimits.MaxInstances)
            {
                System.Console.Error.WriteLine("'ResourceLimits.MaxInstances' values do not match"
                    );
                return false;
            }
            if (qos1.ResourceLimits.MaxSamples != qos2.ResourceLimits.MaxSamples)
            {
                System.Console.Error.WriteLine("'ResourceLimits.MaxSamples' values do not match"
                    );
                return false;
            }
            if (qos1.ResourceLimits.MaxSamplesPerInstance != qos2.ResourceLimits.MaxSamplesPerInstance)
            {
                System.Console.Error.WriteLine("'ResourceLimits.MaxSamplesPerInstance' values do not match"
                    );
                return false;
            }
            if (!OwnershipQosPolicyEquals(qos1.Ownership, qos2.Ownership))
            {
                System.Console.Error.WriteLine("'DataReaderQos.Ownership' differ");
                return false;
            }
            if (!ByteArrayEquals(qos1.UserData.Value, qos2.UserData.Value))
            {
                System.Console.Error.WriteLine("'UserData.Value' values do not match");
                return false;
            }
            if (!DurationEquals(qos1.TimeBasedFilter.MinimumSeparation, qos2.TimeBasedFilter.MinimumSeparation))
            {
                System.Console.Error.WriteLine("'TimeBasedFilter.MinimumSeparation' values do not match"
                    );
                return false;
            }
            if (!DurationEquals(qos1.ReaderDataLifecycle.AutopurgeNowriterSamplesDelay
                , qos2.ReaderDataLifecycle.AutopurgeNowriterSamplesDelay))
            {
                System.Console.Error.WriteLine("'ReaderDataLifecycle.AutoPurgeNoWriterSamplesDelay' values do not match"
                    );
                return false;
            }
            if (!DurationEquals(qos1.ReaderDataLifecycle.AutopurgeDisposedSamplesDelay
                , qos2.ReaderDataLifecycle.AutopurgeDisposedSamplesDelay))
            {
                System.Console.Error.WriteLine("'ReaderDataLifecycle.AutoPurgeDisposedSamplesDelay' values do not match"
                    );
                return false;
            }
            return true;

        }

        /// <summary>Compares all attributes of two PublisherQos objects for equality.</summary>
        /// <remarks>Compares all attributes of two PublisherQos objects for equality.</remarks>
        /// <param name="qos1">PublisherQos object to compare.</param>
        /// <param name="qos2">PublisherQos object to compare agains.</param>
        /// <returns><code>true</code> if the two qosses are equal, otherwise <code>false</code>.
        ///     </returns>
        public static bool PublisherQosEquals (
            DDS.PublisherQos a,
            DDS.PublisherQos b)
        {
            if (!PresentationQosPolicyEquals (a.Presentation, b.Presentation)) {
                System.Console.Error.WriteLine ("'DDS.PublisherQos.Presentation' values do not match");
                return false;
            }
            if (!PartitionQosPolicyEquals (a.Partition, b.Partition)) {
                System.Console.Error.WriteLine ("'DDS.PublisherQos.Partition' values do not match");
                return false;
            }
            if (!GroupDataQosPolicyEquals (a.GroupData, b.GroupData)) {
                System.Console.Error.WriteLine ("'DDS.PublisherQos.GroupData' values do not match");
                return false;
            }
            if (!EntityFactoryQosPolicyEquals (a.EntityFactory, b.EntityFactory)) {
                System.Console.Error.WriteLine ("'DDS.PublisherQos.EntityFactory' values do not match");
                return false;
            }
            return true;
        }

        public static bool DataWriterQosEquals(DDS.DataWriterQos qos1, DDS.DataWriterQos qos2)
        {
            if (!DurabilityQosPolicyEquals(qos1.Durability, qos2.Durability))
            {
                System.Console.Error.WriteLine("'DataWriterQos.Durability' differ");
                return false;
            }
            if (!DeadlineQosPolicyEquals(qos1.Deadline, qos2.Deadline))
            {
                System.Console.Error.WriteLine("'DataWriterQos.Deadline' differ");
                return false;
            }
            if (!LatencyBudgetQosPolicyEquals(qos1.LatencyBudget, qos2.LatencyBudget))
            {
                System.Console.Error.WriteLine("'DataWriterQos.LatencyBudget' differ");
                return false;
            }
            if (!LivelinessQosPolicyEquals(qos1.Liveliness, qos2.Liveliness))
            {
                System.Console.Error.WriteLine("'DataWriterQos.Liveliness' differ");
                return false;
            }
            if (!ReliabilityQosPolicyEquals(qos1.Reliability, qos2.Reliability))
            {
                System.Console.Error.WriteLine("'DataWriterQos.Reliability' differ");
                return false;
            }
            if (!DestinationOrderQosPolicyEquals(qos1.DestinationOrder, qos2.DestinationOrder
                ))
            {
                System.Console.Error.WriteLine("'DataWriterQos.DestinationOrder' differ");
                return false;
            }
            if (!HistoryQosPolicyEquals(qos1.History, qos2.History))
            {
                System.Console.Error.WriteLine("'DataWriterQos.History' differ");
                return false;
            }
            if (!ResourceLimitsQosPolicyEquals(qos1.ResourceLimits, qos2.ResourceLimits))
            {
                System.Console.Error.WriteLine("'DataWriterQos.ResourceLimits' differ");
                return false;
            }
            if (!TransportPriorityQosPolicyEquals(qos1.TransportPriority, qos2.TransportPriority
                ))
            {
                System.Console.Error.WriteLine("'DataWriterQos.TransportPriority' differ");
                return false;
            }
            if (!LifespanQosPolicyEquals(qos1.Lifespan, qos2.Lifespan))
            {
                System.Console.Error.WriteLine("'DataWriterQos.Lifespan' differ");
                return false;
            }
            if (!UserDataQosPolicyEquals(qos1.UserData, qos2.UserData))
            {
                System.Console.Error.WriteLine("'DataWriterQos.UserData' differ");
                return false;
            }
            if (!OwnershipQosPolicyEquals(qos1.Ownership, qos2.Ownership))
            {
                System.Console.Error.WriteLine("'DataWriterQos.Ownership' differ");
                return false;
            }
            if (!OwnershipStrengthQosPolicyEquals(qos1.OwnershipStrength, qos2.OwnershipStrength
                ))
            {
                System.Console.Error.WriteLine("'DataWriterQos.OwnershipStrength' differ");
                return false;
            }
            if (!WriterDataLifecycleQosPolicyEquals(qos1.WriterDataLifecycle, qos2.WriterDataLifecycle
                ))
            {
                System.Console.Error.WriteLine("'DataWriterQos.WriterDataLifecycle' differ");
                return false;
            }
            return true;
        }

        public static bool ByteArrayEquals(byte[] arr1, byte[] arr2)
        {
            if (arr1.Length != arr2.Length)
            {
                System.Console.Error.WriteLine("Byte array lengths not equal.(" + arr1.Length + " != "
                     + arr2.Length + ")");
                return false;
            }
            for (int i = 0; i < arr1.Length; i++)
            {
                if (arr1[i] != arr2[i])
                {
                    System.Console.Error.WriteLine("Byte arrays not equal (index: " + i + ")");
                    return false;
                }
            }
            return true;
        }

        public static bool StringArrayEquals(string[] arr1, string[] arr2)
        {
            if (arr1.Length != arr2.Length)
            {
                System.Console.Error.WriteLine("String array lengths not equal. (" + arr1.Length
                    + " != " + arr2.Length + ")");
                return false;
            }
            for (int i = 0; i < arr1.Length; i++)
            {
                if (!arr1[i].Equals(arr2[i]))
                {
                    System.Console.Error.WriteLine("String arrays not equal (index: " + i + ")");
                    return false;
                }
            }
            return true;
        }

        /// <summary>Compares two DDS.SchedulingQosPolicy objects.</summary>
        /// <remarks>Compares two DDS.SchedulingQosPolicy objects.</remarks>
        /// <param name="a"></param>
        /// <param name="b"></param>
        /// <returns>true if the two objects have equal values.</returns>
        public static bool SchedulingQosPolicyEquals (
            DDS.SchedulingQosPolicy a,
            DDS.SchedulingQosPolicy b)
        {
            if (a.SchedulingClass.Kind != b.SchedulingClass.Kind) {
                System.Console.Error.WriteLine ("'DDS.SchedulingQosPolicy.SchedulingClass.Kind' values do not match");
                return false;
            }
            if (a.SchedulingPriorityKind.Kind != b.SchedulingPriorityKind.Kind) {
                System.Console.Error.WriteLine ("'DDS.SchedulingQosPolicy.SchedulingPriorityKind.Kind' values do not match");
                return false;
            }
            if (a.SchedulingPriority != b.SchedulingPriority) {
                System.Console.Error.WriteLine ("'DDS.SchedulingQosPolicy.SchedulingPriority' values do not match");
                return false;
            }
            return true;
        }

        /// <summary>Compares two DDS.ShareQosPolicy objects.</summary>
        /// <remarks>Compares two DDS.ShareQosPolicy objects.</remarks>
        /// <param name="a"></param>
        /// <param name="b"></param>
        /// <returns>true if the two objects have equal values.</returns>
        public static bool ShareQosPolicyEquals (
            DDS.ShareQosPolicy a,
            DDS.ShareQosPolicy b)
        {
            if (string.Compare (a.Name, b.Name) != 0) {
                System.Console.Error.WriteLine ("'DDS.ShareQosPolicy.Name' values do not match");
                return false;
            }
            if (a.Enable != b.Enable) {
                System.Console.Error.WriteLine ("'DDS.ShareQosPolicy.Enable' values do not match");
                return false;
            }
            return true;
        }

        /// <summary>Compares two Duration_t object.</summary>
        /// <remarks>Compares two Duration_t object.</remarks>
        /// <param name="duration1"></param>
        /// <param name="duration2"></param>
        /// <returns>true if the two objects have equal values.</returns>
        public static bool DurationEquals(DDS.Duration duration1, DDS.Duration duration2
            )
        {
            if (duration1.NanoSec != duration2.NanoSec)
            {
                System.Console.Error.WriteLine("duration.NanoSec differ");
                System.Console.Error.WriteLine("duration1 sec: " + duration1.Sec + "\tNanoSec: "
                    + duration1.NanoSec);
                System.Console.Error.WriteLine("duration2 sec: " + duration2.Sec + "\tNanoSec: "
                    + duration2.NanoSec);
                return false;
            }
            if (duration1.Sec != duration2.Sec)
            {
                System.Console.Error.WriteLine("duration.Sec differ");
                return false;
            }
            return true;
        }

        /// <summary>Compares if the values within policy1 are the same as in policy2.</summary>
        /// <remarks>Compares if the values within policy1 are the same as in policy2.</remarks>
        /// <param name="policy1"></param>
        /// <param name="policy2"></param>
        /// <returns><code>true</code> if the values are equal.</returns>
        public static bool DeadlineQosPolicyEquals(DDS.DeadlineQosPolicy policy1, DDS.DeadlineQosPolicy
             policy2)
        {
            if (!DurationEquals(policy1.Period, policy2.Period))
            {
                System.Console.Error.WriteLine("'DeadlineQosPolicy.Period' differ");
                return false;
            }
            return true;
        }

        /// <summary>Compares if the values within policy1 are the same as in policy2.</summary>
        /// <remarks>Compares if the values within policy1 are the same as in policy2.</remarks>
        /// <param name="policy1"></param>
        /// <param name="policy2"></param>
        /// <returns><code>true</code> if the values are equal.</returns>
        public static bool DestinationOrderQosPolicyEquals(DDS.DestinationOrderQosPolicy
            policy1, DDS.DestinationOrderQosPolicy policy2)
        {
            if (policy1.Kind != policy2.Kind)
            {
                System.Console.Error.WriteLine("'DestinationOrderQosPolicy.Kind' differ");
                return false;
            }
            return true;
        }

        /// <summary>Compares if the values within policy1 are the same as in policy2.</summary>
        /// <remarks>Compares if the values within policy1 are the same as in policy2.</remarks>
        /// <param name="policy1"></param>
        /// <param name="policy2"></param>
        /// <returns><code>true</code> if the values are equal.</returns>
        public static bool DurabilityQosPolicyEquals(DDS.DurabilityQosPolicy policy1, DDS.DurabilityQosPolicy
             policy2)
        {
            if (policy1.Kind != policy2.Kind)
            {
                System.Console.Error.WriteLine("'DurabilityQosPolicy.Kind' differ");
                return false;
            }
            return true;
        }

        /// <summary>Compares if the values within policy1 are the same as in policy2.</summary>
        /// <remarks>Compares if the values within policy1 are the same as in policy2.</remarks>
        /// <param name="policy1"></param>
        /// <param name="policy2"></param>
        /// <returns><code>true</code> if the values are equal.</returns>
        public static bool DurabilityServiceQosPolicyEquals(DDS.DurabilityServiceQosPolicy
             policy1, DDS.DurabilityServiceQosPolicy policy2)
        {
            if (!DurationEquals(policy1.ServiceCleanupDelay, policy2.ServiceCleanupDelay
                ))
            {
                System.Console.Error.WriteLine("'DurabilityQosPolicy.ServiceCleanupDelay' differ"
                    );
                return false;
            }
            if (policy1.HistoryKind != policy2.HistoryKind)
            {
                System.Console.Error.WriteLine("'DurabilityServiceQosPolicy.HistoryKind' differ"
                    );
                return false;
            }
            if (policy1.HistoryDepth != policy2.HistoryDepth)
            {
                System.Console.Error.WriteLine("'DurabilityServiceQosPolicy.HistoryDepth' differ"
                    );
                return false;
            }
            if (policy1.MaxInstances != policy2.MaxInstances)
            {
                System.Console.Error.WriteLine("'DurabilityServiceQosPolicy.MaxInstances' differ"
                    );
                return false;
            }
            if (policy1.MaxSamples != policy2.MaxSamples)
            {
                System.Console.Error.WriteLine("'DurabilityServiceQosPolicy.MaxSamples' differ");
                return false;
            }
            if (policy1.MaxSamplesPerInstance != policy2.MaxSamplesPerInstance)
            {
                System.Console.Error.WriteLine("'DurabilityServiceQosPolicy.MaxSamplesPerInstance' differ"
                    );
                return false;
            }
            return true;
        }

        /// <summary>Compares if the values within policy1 are the same as in policy2.</summary>
        /// <remarks>Compares if the values within policy1 are the same as in policy2.</remarks>
        /// <param name="policy1"></param>
        /// <param name="policy2"></param>
        /// <returns><code>true</code> if the values are equal.</returns>
        public static bool EntityFactoryQosPolicyEquals(DDS.EntityFactoryQosPolicy policy1
            , DDS.EntityFactoryQosPolicy policy2)
        {
            if (policy1.AutoenableCreatedEntities != policy2.AutoenableCreatedEntities)
            {
                System.Console.Error.WriteLine("'EntityFactoryQosPolicy.AutoEnableCreatedEntities' differ"
                    );
                return false;
            }
            return true;
        }

        /// <summary>Compares if the values within policy1 are the same as in policy2.</summary>
        /// <remarks>Compares if the values within policy1 are the same as in policy2.</remarks>
        /// <param name="policy1"></param>
        /// <param name="policy2"></param>
        /// <returns><code>true</code> if the values are equal.</returns>
        public static bool GroupDataQosPolicyEquals(DDS.GroupDataQosPolicy policy1, DDS.GroupDataQosPolicy
             policy2)
        {
            if (!ByteArrayEquals(policy1.Value, policy2.Value))
            {
                System.Console.Error.WriteLine("'GroupDataQosPolicy.Value' differ");
                return false;
            }
            return true;
        }

        /// <summary>Compares if the values within policy1 are the same as in policy2.</summary>
        /// <remarks>Compares if the values within policy1 are the same as in policy2.</remarks>
        /// <param name="policy1"></param>
        /// <param name="policy2"></param>
        /// <returns><code>true</code> if the values are equal.</returns>
        public static bool HistoryQosPolicyEquals(DDS.HistoryQosPolicy policy1, DDS.HistoryQosPolicy
             policy2)
        {
            if (policy1.Kind != policy2.Kind)
            {
                System.Console.Error.WriteLine("'DurabilityQosPolicy.Kind' differ");
                return false;
            }
            if (policy1.Depth != policy2.Depth)
            {
                System.Console.Error.WriteLine("'DurabilityQosPolicy.Depth' differ");
                return false;
            }
            return true;
        }

        /// <summary>Compares if the values within policy1 are the same as in policy2.</summary>
        /// <remarks>Compares if the values within policy1 are the same as in policy2.</remarks>
        /// <param name="policy1"></param>
        /// <param name="policy2"></param>
        /// <returns><code>true</code> if the values are equal.</returns>
        public static bool LatencyBudgetQosPolicyEquals(DDS.LatencyBudgetQosPolicy policy1
            , DDS.LatencyBudgetQosPolicy policy2)
        {
            if (!DurationEquals(policy1.Duration, policy2.Duration))
            {
                System.Console.Error.WriteLine("'LatencyBudgetQosPolicy.Duration' differ");
                return false;
            }
            return true;
        }

        /// <summary>Compares if the values within policy1 are the same as in policy2.</summary>
        /// <remarks>Compares if the values within policy1 are the same as in policy2.</remarks>
        /// <param name="policy1"></param>
        /// <param name="policy2"></param>
        /// <returns><code>true</code> if the values are equal.</returns>
        public static bool LifespanQosPolicyEquals(DDS.LifespanQosPolicy policy1, DDS.LifespanQosPolicy
             policy2)
        {
            if (!DurationEquals(policy1.Duration, policy2.Duration))
            {
                System.Console.Error.WriteLine("'LifespanQosPolicy.Duration' differ");
                return false;
            }
            return true;
        }

        /// <summary>Compares if the values within policy1 are the same as in policy2.</summary>
        /// <remarks>Compares if the values within policy1 are the same as in policy2.</remarks>
        /// <param name="policy1"></param>
        /// <param name="policy2"></param>
        /// <returns><code>true</code> if the values are equal.</returns>
        public static bool LivelinessQosPolicyEquals(DDS.LivelinessQosPolicy policy1, DDS.LivelinessQosPolicy
             policy2)
        {
            if (policy1.Kind != policy2.Kind)
            {
                System.Console.Error.WriteLine("'LivelinessQosPolicy.Kind' differ");
                return false;
            }
            if (!DurationEquals(policy1.LeaseDuration, policy2.LeaseDuration))
            {
                System.Console.Error.WriteLine("'LivelinessQosPolicy.LeaseDuration' differ");
                return false;
            }
            return true;
        }

        /// <summary>Compares if the values within policy1 are the same as in policy2.</summary>
        /// <remarks>Compares if the values within policy1 are the same as in policy2.</remarks>
        /// <param name="policy1"></param>
        /// <param name="policy2"></param>
        /// <returns><code>true</code> if the values are equal.</returns>
        public static bool OwnershipQosPolicyEquals(DDS.OwnershipQosPolicy policy1, DDS.OwnershipQosPolicy
             policy2)
        {
            if (policy1.Kind != policy2.Kind)
            {
                System.Console.Error.WriteLine("'OwnershipQosPolicy.Kind' differ");
                return false;
            }
            return true;
        }

        /// <summary>Compares if the values within policy1 are the same as in policy2.</summary>
        /// <remarks>Compares if the values within policy1 are the same as in policy2.</remarks>
        /// <param name="policy1"></param>
        /// <param name="policy2"></param>
        /// <returns><code>true</code> if the values are equal.</returns>
        public static bool OwnershipStrengthQosPolicyEquals(DDS.OwnershipStrengthQosPolicy
             policy1, DDS.OwnershipStrengthQosPolicy policy2)
        {
            if (policy1.Value != policy2.Value)
            {
                System.Console.Error.WriteLine("'OwnershipStrengthQosPolicy.Value' differ");
                return false;
            }
            return true;
        }

        /// <summary>Compares if the values within policy1 are the same as in policy2.</summary>
        /// <remarks>Compares if the values within policy1 are the same as in policy2.</remarks>
        /// <param name="policy1"></param>
        /// <param name="policy2"></param>
        /// <returns><code>true</code> if the values are equal.</returns>
        public static bool PartitionQosPolicyEquals(DDS.PartitionQosPolicy policy1, DDS.PartitionQosPolicy
             policy2)
        {
            if (!StringArrayEquals(policy1.Name, policy2.Name))
            {
                System.Console.Error.WriteLine("'PartitionQosPolicy.Name' differ");
                return false;
            }
            return true;
        }

        /// <summary>Compares if the values within policy1 are the same as in policy2.</summary>
        /// <remarks>Compares if the values within policy1 are the same as in policy2.</remarks>
        /// <param name="policy1"></param>
        /// <param name="policy2"></param>
        /// <returns><code>true</code> if the values are equal.</returns>
        public static bool PresentationQosPolicyEquals(DDS.PresentationQosPolicy policy1,
            DDS.PresentationQosPolicy policy2)
        {
            if (policy1.AccessScope != policy2.AccessScope)
            {
                System.Console.Error.WriteLine("'PresentationQosPolicy.AccessScope' differ");
                return false;
            }
            if (policy1.CoherentAccess != policy2.CoherentAccess)
            {
                System.Console.Error.WriteLine("'PresentationQosPolicy.CoherentAccess' differ");
                return false;
            }
            if (policy1.OrderedAccess != policy2.OrderedAccess)
            {
                System.Console.Error.WriteLine("'PresentationQosPolicy.OrderedAccess' differ");
                return false;
            }
            return true;
        }

        /// <summary>Compares if the values within policy1 are the same as in policy2.</summary>
        /// <remarks>Compares if the values within policy1 are the same as in policy2.</remarks>
        /// <param name="policy1"></param>
        /// <param name="policy2"></param>
        /// <returns><code>true</code> if the values are equal.</returns>
        public static bool ReaderDataLifecycleQosPolicyEquals(DDS.ReaderDataLifecycleQosPolicy
             policy1, DDS.ReaderDataLifecycleQosPolicy policy2)
        {
            if (!DurationEquals(policy1.AutopurgeNowriterSamplesDelay, policy2.AutopurgeNowriterSamplesDelay
                ))
            {
                System.Console.Error.WriteLine("'ReaderDataLifecycleQosPolicy.AutoPurgeNoWriterSamplesDelay' differ"
                    );
                return false;
            }
            if (!DurationEquals(policy1.AutopurgeDisposedSamplesDelay, policy2.AutopurgeDisposedSamplesDelay
                ))
            {
                System.Console.Error.WriteLine("'ReaderDataLifecycleQosPolicy.AutoPurgeDisposedSamplesDelay' differ"
                    );
                return false;
            }
            if (policy1.EnableInvalidSamples != policy2.EnableInvalidSamples)
            {
                System.Console.Error.WriteLine("'ReaderDataLifecycleQosPolicy.EnableInvalidSamples' differ"
                    );
                return false;
            }
            return true;
        }

        /// <summary>Compares if the values within policy1 are the same as in policy2.</summary>
        /// <remarks>Compares if the values within policy1 are the same as in policy2.</remarks>
        /// <param name="policy1"></param>
        /// <param name="policy2"></param>
        /// <returns><code>true</code> if the values are equal.</returns>
        public static bool ReliabilityQosPolicyEquals(DDS.ReliabilityQosPolicy policy1, DDS.ReliabilityQosPolicy
             policy2)
        {
            if (policy1.Kind != policy2.Kind)
            {
                System.Console.Error.WriteLine("'ReliabilityQosPolicy.Kind' differ");
                return false;
            }
            if (!DurationEquals(policy1.MaxBlockingTime, policy2.MaxBlockingTime))
            {
                System.Console.Error.WriteLine("'ReliabilityQosPolicy.MaxBlockingTime' differ");
                return false;
            }
            return true;
        }

        /// <summary>Compares if the values within policy1 are the same as in policy2.</summary>
        /// <remarks>Compares if the values within policy1 are the same as in policy2.</remarks>
        /// <param name="policy1"></param>
        /// <param name="policy2"></param>
        /// <returns><code>true</code> if the values are equal.</returns>
        public static bool ResourceLimitsQosPolicyEquals(DDS.ResourceLimitsQosPolicy policy1
            , DDS.ResourceLimitsQosPolicy policy2)
        {
            if (policy1.MaxInstances != policy2.MaxInstances)
            {
                System.Console.Error.WriteLine("'ResourceLimitsQosPolicy.MaxInstances' differ");
                return false;
            }
            if (policy1.MaxSamples != policy2.MaxSamples)
            {
                System.Console.Error.WriteLine("'ResourceLimitsQosPolicy.MaxSamples' differ");
                return false;
            }
            if (policy1.MaxSamplesPerInstance != policy2.MaxSamplesPerInstance)
            {
                System.Console.Error.WriteLine("'ResourceLimitsQosPolicy.MaxSamplesPerInstance' differ"
                    );
                return false;
            }
            return true;
        }

        /// <summary>Compares if the values within policy1 are the same as in policy2.</summary>
        /// <remarks>Compares if the values within policy1 are the same as in policy2.</remarks>
        /// <param name="policy1"></param>
        /// <param name="policy2"></param>
        /// <returns><code>true</code> if the values are equal.</returns>
        public static bool TimeBasedFilterQosPolicyEquals(DDS.TimeBasedFilterQosPolicy policy1
            , DDS.TimeBasedFilterQosPolicy policy2)
        {
            if (!DurationEquals(policy1.MinimumSeparation, policy2.MinimumSeparation))
            {
                System.Console.Error.WriteLine("'TimeBasedFilterQosPolicy.MinimumSeparation' differ"
                    );
                return false;
            }
            return true;
        }

        /// <summary>Compares if the values within policy1 are the same as in policy2.</summary>
        /// <remarks>Compares if the values within policy1 are the same as in policy2.</remarks>
        /// <param name="policy1"></param>
        /// <param name="policy2"></param>
        /// <returns><code>true</code> if the values are equal.</returns>
        public static bool TopicDataQosPolicyEquals(DDS.TopicDataQosPolicy policy1, DDS.TopicDataQosPolicy
             policy2)
        {
            if (!ByteArrayEquals(policy1.Value, policy2.Value))
            {
                System.Console.Error.WriteLine("'TopicDataQosPolicy.Value' differ");
                return false;
            }
            return true;
        }

        /// <summary>Compares if the values within policy1 are the same as in policy2.</summary>
        /// <remarks>Compares if the values within policy1 are the same as in policy2.</remarks>
        /// <param name="policy1"></param>
        /// <param name="policy2"></param>
        /// <returns><code>true</code> if the values are equal.</returns>
        public static bool TransportPriorityQosPolicyEquals(DDS.TransportPriorityQosPolicy
             policy1, DDS.TransportPriorityQosPolicy policy2)
        {
            if (policy1.Value != policy2.Value)
            {
                System.Console.Error.WriteLine("'TransportPriorityQosPolicy.Value' differ");
                return false;
            }
            return true;
        }

        /// <summary>Compares if the values within policy1 are the same as in policy2.</summary>
        /// <remarks>Compares if the values within policy1 are the same as in policy2.</remarks>
        /// <param name="policy1"></param>
        /// <param name="policy2"></param>
        /// <returns><code>true</code> if the values are equal.</returns>
        public static bool UserDataQosPolicyEquals(DDS.UserDataQosPolicy policy1, DDS.UserDataQosPolicy
             policy2)
        {
            if (!ByteArrayEquals(policy1.Value, policy2.Value))
            {
                System.Console.Error.WriteLine("'UserDataQosPolicy.Value' differ");
                return false;
            }
            return true;
        }

        /// <summary>Compares if the values within policy1 are the same as in policy2.</summary>
        /// <remarks>Compares if the values within policy1 are the same as in policy2.</remarks>
        /// <param name="policy1"></param>
        /// <param name="policy2"></param>
        /// <returns><code>true</code> if the values are equal.</returns>
        public static bool WriterDataLifecycleQosPolicyEquals(DDS.WriterDataLifecycleQosPolicy
             policy1, DDS.WriterDataLifecycleQosPolicy policy2)
        {
            if (policy1.AutodisposeUnregisteredInstances != policy2.AutodisposeUnregisteredInstances)
            {
                System.Console.Error.WriteLine("'WriterDataLifecycleQosPolicy.AutoDisposeUnregisteredInstances' differ"
                    );
                return false;
            }
            return true;
        }
    }
}
