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
using DDS;
using DDS.OpenSplice.CustomMarshalers;

namespace DDS.OpenSplice
{
    internal class QosManager
    {
        internal static readonly DDS.EntityFactoryQosPolicy defaultEntityFactoryQosPolicy = new DDS.EntityFactoryQosPolicy();
        internal static readonly DDS.UserDataQosPolicy defaultUserDataQosPolicy = new DDS.UserDataQosPolicy();
        internal static readonly DDS.SchedulingClassQosPolicy defaultSchedulingClassQosPolicy = new DDS.SchedulingClassQosPolicy();
        internal static readonly DDS.SchedulingPriorityQosPolicy defaultSchedulingPriorityQosPolicy = new DDS.SchedulingPriorityQosPolicy();
        internal static readonly DDS.SchedulingQosPolicy defaultSchedulingQosPolicy = new DDS.SchedulingQosPolicy();
        internal static readonly DDS.PresentationQosPolicy defaultPresentationQosPolicy = new DDS.PresentationQosPolicy();
        internal static readonly DDS.PartitionQosPolicy defaultPartitionQosPolicy = new DDS.PartitionQosPolicy();
        internal static readonly DDS.GroupDataQosPolicy defaultGroupDataQosPolicy = new DDS.GroupDataQosPolicy();
        internal static readonly DDS.TopicDataQosPolicy defaultTopicDataQosPolicy = new DDS.TopicDataQosPolicy();
        internal static readonly DDS.DurabilityQosPolicy defaultDurabilityQosPolicy = new DDS.DurabilityQosPolicy();
        internal static readonly DDS.DurabilityServiceQosPolicy defaultDurabilityServiceQosPolicy = new DDS.DurabilityServiceQosPolicy();
        internal static readonly DDS.DeadlineQosPolicy defaultDeadlineQosPolicy = new DDS.DeadlineQosPolicy();
        internal static readonly DDS.LatencyBudgetQosPolicy defaultLatencyBudgetQosPolicy = new DDS.LatencyBudgetQosPolicy();
        internal static readonly DDS.LivelinessQosPolicy defaultLivelinessQosPolicy = new DDS.LivelinessQosPolicy();
        internal static readonly DDS.ReliabilityQosPolicy defaultDataReaderReliabilityQosPolicy = new DDS.ReliabilityQosPolicy();
        internal static readonly DDS.ReliabilityQosPolicy defaultDataWriterReliabilityQosPolicy = new DDS.ReliabilityQosPolicy();
        internal static readonly DDS.DestinationOrderQosPolicy defaultDestinationOrderQosPolicy = new DDS.DestinationOrderQosPolicy();
        internal static readonly DDS.HistoryQosPolicy defaultHistoryQosPolicy = new DDS.HistoryQosPolicy();
        internal static readonly DDS.ResourceLimitsQosPolicy defaultResourceLimitsQosPolicy = new DDS.ResourceLimitsQosPolicy();
        internal static readonly DDS.TransportPriorityQosPolicy defaultTransportPriorityQosPolicy = new DDS.TransportPriorityQosPolicy();
        internal static readonly DDS.LifespanQosPolicy defaultLifespanQosPolicy = new DDS.LifespanQosPolicy();
        internal static readonly DDS.OwnershipQosPolicy defaultOwnershipQosPolicy = new DDS.OwnershipQosPolicy();
        internal static readonly DDS.OwnershipStrengthQosPolicy defaultOwnershipStrengthQosPolicy = new DDS.OwnershipStrengthQosPolicy();
        internal static readonly DDS.WriterDataLifecycleQosPolicy defaultWriterDataLifecycleQosPolicy = new DDS.WriterDataLifecycleQosPolicy();
        internal static readonly DDS.TimeBasedFilterQosPolicy defaultTimeBasedFilterQosPolicy = new DDS.TimeBasedFilterQosPolicy();
        internal static readonly DDS.InvalidSampleVisibilityQosPolicy defaultInvalidSampleVisibilityQosPolicy = new DDS.InvalidSampleVisibilityQosPolicy();
        internal static readonly DDS.ReaderDataLifecycleQosPolicy defaultReaderDataLifecycleQosPolicy = new DDS.ReaderDataLifecycleQosPolicy();
        internal static readonly DDS.ShareQosPolicy defaultShareQosPolicy = new DDS.ShareQosPolicy();
        internal static readonly DDS.ReaderLifespanQosPolicy defaultReaderLifespanQosPolicy = new DDS.ReaderLifespanQosPolicy();
        internal static readonly DDS.SubscriptionKeyQosPolicy defaultSubscriptionKeyQosPolicy = new DDS.SubscriptionKeyQosPolicy();
        internal static readonly DDS.ViewKeyQosPolicy defaultViewKeyQosPolicy = new DDS.ViewKeyQosPolicy();

        /**
         *
         * Default OSPL Qos values.
         *
         *************************************/
        internal static readonly DDS.DomainParticipantFactoryQos defaultDomainParticipantFactoryQos = new DDS.DomainParticipantFactoryQos();
        internal static readonly DDS.DomainParticipantQos defaultDomainParticipantQos = new DDS.DomainParticipantQos();
        internal static readonly DDS.SubscriberQos defaultSubscriberQos = new DDS.SubscriberQos();
        internal static readonly DDS.PublisherQos defaultPublisherQos = new DDS.PublisherQos();
        internal static readonly DDS.TopicQos defaultTopicQos = new DDS.TopicQos();
        internal static readonly DDS.DataWriterQos defaultDataWriterQos = new DDS.DataWriterQos();
        internal static readonly DDS.DataReaderQos defaultDataReaderQos = new DDS.DataReaderQos();
        internal static readonly DDS.DataReaderViewQos defaultDataReaderViewQos = new DDS.DataReaderViewQos();

        static QosManager()
        {
            defaultEntityFactoryQosPolicy.AutoenableCreatedEntities = true;

            defaultUserDataQosPolicy.Value = new byte[0];

            defaultSchedulingClassQosPolicy.Kind = DDS.SchedulingClassQosPolicyKind.ScheduleDefault;
            defaultSchedulingPriorityQosPolicy.Kind = DDS.SchedulingPriorityQosPolicyKind.PriorityRelative;
            defaultSchedulingQosPolicy.SchedulingClass = defaultSchedulingClassQosPolicy;
            defaultSchedulingQosPolicy.SchedulingPriorityKind = defaultSchedulingPriorityQosPolicy;
            defaultSchedulingQosPolicy.SchedulingPriority = 0;

            defaultPresentationQosPolicy.AccessScope = DDS.PresentationQosPolicyAccessScopeKind.InstancePresentationQos;
            defaultPresentationQosPolicy.CoherentAccess = false;
            defaultPresentationQosPolicy.OrderedAccess = false;

            defaultPartitionQosPolicy.Name = new String[0];

            defaultGroupDataQosPolicy.Value = new byte[0];

            defaultTopicDataQosPolicy.Value = new byte[0];

            defaultDurabilityQosPolicy.Kind = DDS.DurabilityQosPolicyKind.VolatileDurabilityQos;

            defaultDurabilityServiceQosPolicy.ServiceCleanupDelay = DDS.Duration.Zero;
            defaultDurabilityServiceQosPolicy.HistoryKind = DDS.HistoryQosPolicyKind.KeepLastHistoryQos;
            defaultDurabilityServiceQosPolicy.HistoryDepth = 1;
            defaultDurabilityServiceQosPolicy.MaxSamples = -1;
            defaultDurabilityServiceQosPolicy.MaxInstances = -1;
            defaultDurabilityServiceQosPolicy.MaxSamplesPerInstance = -1;

            defaultDeadlineQosPolicy.Period = DDS.Duration.Infinite;

            defaultLatencyBudgetQosPolicy.Duration = DDS.Duration.Zero;

            defaultLivelinessQosPolicy.Kind = DDS.LivelinessQosPolicyKind.AutomaticLivelinessQos;
            defaultLivelinessQosPolicy.LeaseDuration = DDS.Duration.Infinite;

            defaultDataReaderReliabilityQosPolicy.Kind = DDS.ReliabilityQosPolicyKind.BestEffortReliabilityQos;
            defaultDataReaderReliabilityQosPolicy.MaxBlockingTime.Sec = 0;
            defaultDataReaderReliabilityQosPolicy.MaxBlockingTime.NanoSec = 100000000;
            defaultDataReaderReliabilityQosPolicy.Synchronous = false;

            defaultDataWriterReliabilityQosPolicy.Kind = DDS.ReliabilityQosPolicyKind.ReliableReliabilityQos;
            defaultDataWriterReliabilityQosPolicy.MaxBlockingTime.Sec = 0;
            defaultDataWriterReliabilityQosPolicy.MaxBlockingTime.NanoSec = 100000000;
            defaultDataWriterReliabilityQosPolicy.Synchronous = false;

            defaultDestinationOrderQosPolicy.Kind = DDS.DestinationOrderQosPolicyKind.ByReceptionTimestampDestinationorderQos;

            defaultHistoryQosPolicy.Kind = DDS.HistoryQosPolicyKind.KeepLastHistoryQos;
            defaultHistoryQosPolicy.Depth = 1;

            defaultResourceLimitsQosPolicy.MaxSamples = -1;
            defaultResourceLimitsQosPolicy.MaxInstances = -1;
            defaultResourceLimitsQosPolicy.MaxSamplesPerInstance = -1;

            defaultTransportPriorityQosPolicy.Value = 0;

            defaultLifespanQosPolicy.Duration = DDS.Duration.Infinite;

            defaultOwnershipQosPolicy.Kind = DDS.OwnershipQosPolicyKind.SharedOwnershipQos;

            defaultOwnershipStrengthQosPolicy.Value = 0;

            defaultWriterDataLifecycleQosPolicy.AutodisposeUnregisteredInstances = true;
            defaultWriterDataLifecycleQosPolicy.AutopurgeSuspendedSamplesDelay = DDS.Duration.Infinite;
            defaultWriterDataLifecycleQosPolicy.AutounregisterInstanceDelay = DDS.Duration.Infinite;

            defaultTimeBasedFilterQosPolicy.MinimumSeparation = DDS.Duration.Zero;

            defaultInvalidSampleVisibilityQosPolicy.Kind = DDS.InvalidSampleVisibilityQosPolicyKind.MinimumInvalidSamples;

            defaultReaderDataLifecycleQosPolicy.AutopurgeNowriterSamplesDelay = DDS.Duration.Infinite;
            defaultReaderDataLifecycleQosPolicy.AutopurgeDisposedSamplesDelay = DDS.Duration.Infinite;
            defaultReaderDataLifecycleQosPolicy.EnableInvalidSamples = true;
            defaultReaderDataLifecycleQosPolicy.InvalidSampleVisibility = defaultInvalidSampleVisibilityQosPolicy;

            defaultShareQosPolicy.Name = "";
            defaultShareQosPolicy.Enable = false;

            defaultReaderLifespanQosPolicy.UseLifespan = false;
            defaultReaderLifespanQosPolicy.Duration = DDS.Duration.Infinite;

            defaultSubscriptionKeyQosPolicy.UseKeyList = false;
            defaultSubscriptionKeyQosPolicy.KeyList = new string[0];

            defaultViewKeyQosPolicy.UseKeyList = false;
            defaultViewKeyQosPolicy.KeyList = new string[0];

            /**
             *
             * Default OSPL Qos values.
             *
             *************************************/
            defaultDomainParticipantFactoryQos.EntityFactory = defaultEntityFactoryQosPolicy;

            defaultDomainParticipantQos.UserData = defaultUserDataQosPolicy;
            defaultDomainParticipantQos.EntityFactory = defaultEntityFactoryQosPolicy;
            defaultDomainParticipantQos.WatchdogScheduling = defaultSchedulingQosPolicy;
            defaultDomainParticipantQos.ListenerScheduling = defaultSchedulingQosPolicy;

            defaultSubscriberQos.Presentation = defaultPresentationQosPolicy;
            defaultSubscriberQos.Partition = defaultPartitionQosPolicy;
            defaultSubscriberQos.GroupData = defaultGroupDataQosPolicy;
            defaultSubscriberQos.EntityFactory = defaultEntityFactoryQosPolicy;
            defaultSubscriberQos.Share = defaultShareQosPolicy;

            defaultPublisherQos.Presentation = defaultPresentationQosPolicy;
            defaultPublisherQos.Partition = defaultPartitionQosPolicy;
            defaultPublisherQos.GroupData = defaultGroupDataQosPolicy;
            defaultPublisherQos.EntityFactory = defaultEntityFactoryQosPolicy;

            defaultTopicQos.TopicData = defaultTopicDataQosPolicy;
            defaultTopicQos.Durability = defaultDurabilityQosPolicy;
            defaultTopicQos.DurabilityService = defaultDurabilityServiceQosPolicy;
            defaultTopicQos.Deadline = defaultDeadlineQosPolicy;
            defaultTopicQos.LatencyBudget = defaultLatencyBudgetQosPolicy;
            defaultTopicQos.Liveliness = defaultLivelinessQosPolicy;
            defaultTopicQos.Reliability = defaultDataReaderReliabilityQosPolicy;
            defaultTopicQos.DestinationOrder = defaultDestinationOrderQosPolicy;
            defaultTopicQos.History = defaultHistoryQosPolicy;
            defaultTopicQos.ResourceLimits = defaultResourceLimitsQosPolicy;
            defaultTopicQos.TransportPriority = defaultTransportPriorityQosPolicy;
            defaultTopicQos.Lifespan = defaultLifespanQosPolicy;
            defaultTopicQos.Ownership = defaultOwnershipQosPolicy;

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

            defaultDataReaderQos.Durability = defaultDurabilityQosPolicy;
            defaultDataReaderQos.Deadline = defaultDeadlineQosPolicy;
            defaultDataReaderQos.LatencyBudget = defaultLatencyBudgetQosPolicy;
            defaultDataReaderQos.Liveliness = defaultLivelinessQosPolicy;
            defaultDataReaderQos.Reliability = defaultDataReaderReliabilityQosPolicy;
            defaultDataReaderQos.DestinationOrder = defaultDestinationOrderQosPolicy;
            defaultDataReaderQos.History = defaultHistoryQosPolicy;
            defaultDataReaderQos.ResourceLimits = defaultResourceLimitsQosPolicy;
            defaultDataReaderQos.UserData = defaultUserDataQosPolicy;
            defaultDataReaderQos.Ownership = defaultOwnershipQosPolicy;
            defaultDataReaderQos.TimeBasedFilter = defaultTimeBasedFilterQosPolicy;
            defaultDataReaderQos.ReaderDataLifecycle = defaultReaderDataLifecycleQosPolicy;
            defaultDataReaderQos.SubscriptionKeys = defaultSubscriptionKeyQosPolicy;
            defaultDataReaderQos.ReaderLifespan = defaultReaderLifespanQosPolicy;
            defaultDataReaderQos.Share = defaultShareQosPolicy;

            defaultDataReaderViewQos.ViewKeys = defaultViewKeyQosPolicy;
        }

        internal static int countErrors(Duration o)
        {
            int errorCount = 0;
            if (!((o.Sec == Duration.InfiniteSec &&
                   o.NanoSec == Duration.InfiniteNanoSec) ||
                  (o.Sec != Duration.InfiniteSec &&
                   o.NanoSec < 1000000000)))
            {
                errorCount++;
                ReportStack.Report(DDS.ReturnCode.BadParameter, "Duration { " + o.Sec + ", " + o.NanoSec + "} is not properly normalized");
            }
            return errorCount;
        }

        internal static int countErrors(Time o)
        {
            int errorCount = 0;
            if (o.Sec < 0 || o.NanoSec > 1000000000)
            {
                if (o != Time.Invalid)
                {
                errorCount++;
                ReportStack.Report(DDS.ReturnCode.BadParameter, "Time { " + o.Sec + ", " + o.NanoSec + "} is not properly normalized");
                }
            }
            return errorCount;
        }

        internal static int countErrors(DDS.EntityFactoryQosPolicy o)
        {
            int errorCount = 0;
            return errorCount;
        }

        internal static int countErrors(UserDataQosPolicy o)
        {
            int errorCount = 0;
            if (o.Value == null) errorCount++;
            return errorCount;
        }

        internal static int countErrors(SchedulingQosPolicy o)
        {
            int errorCount = 0;
            if (o.SchedulingClass == null ||
                o.SchedulingClass.Kind > SchedulingClassQosPolicyKind.ScheduleRealtime)
            {
                errorCount++;
            }
            if (o.SchedulingPriorityKind == null ||
                o.SchedulingPriorityKind.Kind > SchedulingPriorityQosPolicyKind.PriorityAbsolute)
            {
                errorCount++;
            }

            return errorCount;
        }

        internal static int countErrors(PresentationQosPolicy o)
        {
            int errorCount = 0;
            if (o.AccessScope > PresentationQosPolicyAccessScopeKind.GroupPresentationQos)
            {
                errorCount++;
            }
            return errorCount;
        }

        internal static int countErrors(PartitionQosPolicy o)
        {
            int errorCount = 0;
            if (o.Name == null) errorCount++;
            return errorCount;
        }

        internal static int countErrors(GroupDataQosPolicy o)
        {
            int errorCount = 0;
            if (o.Value == null) errorCount++;
            return errorCount;
        }

        internal static int countErrors(TopicDataQosPolicy o)
        {
            int errorCount = 0;
            if (o.Value == null) errorCount++;
            return errorCount;
        }

        internal static int countErrors(DurabilityQosPolicy o)
        {
            int errorCount = 0;
            if (o.Kind > DurabilityQosPolicyKind.PersistentDurabilityQos)
            {
                errorCount++;
            }
            return errorCount;
        }

        internal static int countErrors(DurabilityServiceQosPolicy o)
        {
            int errorCount = 0;
            errorCount += countErrors(o.ServiceCleanupDelay);
            if (o.HistoryKind > HistoryQosPolicyKind.KeepAllHistoryQos)
            {
                errorCount++;
            }
            if (o.HistoryDepth <= 0)
            {
                errorCount++;
            }
            if (o.MaxSamples < 0 && o.MaxSamples != Length.Unlimited)
            {
                errorCount++;
            }
            if (o.MaxInstances < 0 && o.MaxInstances != Length.Unlimited)
            {
                errorCount++;
            }
            if (o.MaxSamplesPerInstance < 0 && o.MaxSamplesPerInstance != Length.Unlimited)
            {
                errorCount++;
            }
            return errorCount;
        }

        internal static int countErrors(DeadlineQosPolicy o)
        {
            int errorCount = 0;
            errorCount += countErrors(o.Period);
            return errorCount;
        }

        internal static int countErrors(LatencyBudgetQosPolicy o)
        {
            int errorCount = 0;
            errorCount += countErrors(o.Duration);
            return errorCount;
        }

        internal static int countErrors(LivelinessQosPolicy o)
        {
            int errorCount = 0;
            if (o.Kind > LivelinessQosPolicyKind.ManualByTopicLivelinessQos)
            {
                errorCount++;
            }
            errorCount += countErrors(o.LeaseDuration);
            return errorCount;
        }

        internal static int countErrors(ReliabilityQosPolicy o)
        {
            int errorCount = 0;
            if (o.Kind > ReliabilityQosPolicyKind.ReliableReliabilityQos)
            {
                errorCount++;
            }
            errorCount += countErrors(o.MaxBlockingTime);
            return errorCount;
        }

        internal static int countErrors(DestinationOrderQosPolicy o)
        {
            int errorCount = 0;
            if (o.Kind > DestinationOrderQosPolicyKind.BySourceTimestampDestinationorderQos)
            {
                errorCount++;
            }
            return errorCount;
        }

        internal static int countErrors(OwnershipQosPolicy o)
        {
            int errorCount = 0;
            if (o.Kind > OwnershipQosPolicyKind.ExclusiveOwnershipQos)
            {
                errorCount++;
            }
            return errorCount;
        }

        internal static int countErrors(HistoryQosPolicy o)
        {
            int errorCount = 0;
            if (o.Kind > HistoryQosPolicyKind.KeepAllHistoryQos)
            {
                errorCount++;
            }
            else
            {
                if (o.Kind == HistoryQosPolicyKind.KeepLastHistoryQos)
                {
                    if (o.Depth <= 0) errorCount++;
                }
            }
            return errorCount;
        }

        internal static int countErrors(ResourceLimitsQosPolicy o)
        {
            int errorCount = 0;
            if (o.MaxSamples < 0 &&
                o.MaxSamples != Length.Unlimited)
            {
                errorCount++;
            }
            if (o.MaxInstances < 0 &&
                o.MaxInstances != Length.Unlimited)
            {
                errorCount++;
            }
            if (o.MaxSamplesPerInstance < 0 &&
                o.MaxSamplesPerInstance != Length.Unlimited)
            {
                errorCount++;
            }
            if (o.MaxSamples != Length.Unlimited &&
                o.MaxSamplesPerInstance != Length.Unlimited &&
                o.MaxSamples < o.MaxSamplesPerInstance)
            {
                errorCount++;
            }

            return errorCount;
        }

        internal static int countErrors(TransportPriorityQosPolicy o)
        {
            int errorCount = 0;
            return errorCount;
        }

        internal static int countErrors(LifespanQosPolicy o)
        {
            int errorCount = 0;
            errorCount += countErrors(o.Duration);
            return errorCount;
        }

        internal static int countErrors(OwnershipStrengthQosPolicy o)
        {
            int errorCount = 0;
            return errorCount;
        }

        internal static int countErrors(WriterDataLifecycleQosPolicy o)
        {
            int errorCount = 0;
            errorCount += countErrors(o.AutopurgeSuspendedSamplesDelay);
            errorCount += countErrors(o.AutounregisterInstanceDelay);
            return errorCount;
        }

        internal static int countErrors(TimeBasedFilterQosPolicy o)
        {
            int errorCount = 0;
            errorCount += countErrors(o.MinimumSeparation);
            return errorCount;
        }

        internal static int countErrors(ReaderDataLifecycleQosPolicy o)
        {
            int errorCount = 0;
            errorCount += countErrors(o.AutopurgeNowriterSamplesDelay);
            errorCount += countErrors(o.AutopurgeDisposedSamplesDelay);
            if (o.InvalidSampleVisibility == null  ||
                o.InvalidSampleVisibility.Kind > InvalidSampleVisibilityQosPolicyKind.AllInvalidSamples)
            {
                errorCount++;
            }
            return errorCount;
        }

        internal static int countErrors(ShareQosPolicy o)
        {
            int errorCount = 0;
            if (o.Enable && o.Name == null) errorCount++;
            return errorCount;
        }

        internal static int countErrors(ReaderLifespanQosPolicy o)
        {
            int errorCount = 0;
            errorCount += countErrors(o.Duration);
            return errorCount;
        }

        internal static int countErrors(DDS.SubscriptionKeyQosPolicy o)
        {
            int errorCount = 0;
            if (o.UseKeyList && o.KeyList == null) errorCount++;
            return errorCount;
        }

        internal static ReturnCode checkQos(DomainParticipantQos o)
        {
            ReturnCode result = DDS.ReturnCode.BadParameter;

            if (o != null) {
                int errorCount = 0;
                errorCount += countErrors(o.UserData);
                errorCount += countErrors(o.EntityFactory);
                errorCount += countErrors(o.WatchdogScheduling);
                errorCount += countErrors(o.ListenerScheduling);
                if (errorCount == 0) {
                    result = DDS.ReturnCode.Ok;
                }
            } else {
                ReportStack.Report(result,
                    "DomainParticipantQos 'null' is invalid.");
            }
            return result;
        }

        internal static ReturnCode checkQos(TopicQos o)
        {
            ReturnCode result = DDS.ReturnCode.BadParameter;

            if (o != null) {
                int errorCount = 0;
                errorCount += countErrors(o.TopicData);
                errorCount += countErrors(o.Durability);
                errorCount += countErrors(o.DurabilityService);
                errorCount += countErrors(o.Deadline);
                errorCount += countErrors(o.LatencyBudget);
                errorCount += countErrors(o.Liveliness);
                errorCount += countErrors(o.Reliability);
                errorCount += countErrors(o.DestinationOrder);
                errorCount += countErrors(o.History);
                errorCount += countErrors(o.ResourceLimits);
                errorCount += countErrors(o.TransportPriority);
                errorCount += countErrors(o.Lifespan);
                errorCount += countErrors(o.Ownership);
                if (errorCount == 0) {
                    result = DDS.ReturnCode.Ok;
                    if ((o.History.Kind == HistoryQosPolicyKind.KeepLastHistoryQos) &&
                        (o.ResourceLimits.MaxSamplesPerInstance != Length.Unlimited) &&
                        (o.History.Depth > o.ResourceLimits.MaxSamplesPerInstance))
                    {
                        ReportStack.Report(result,
                                "HistoryQosPolicy.depth is greater than " +
                                "ResourceLimitsQosPolicy.max_samples_per_instance.");
                        result = DDS.ReturnCode.InconsistentPolicy;
                    }
                }
            } else {
                ReportStack.Report(result,
                    "TopicQos 'null' is invalid.");
            }
            return result;
        }

        internal static ReturnCode checkQos(PublisherQos o)
        {
            ReturnCode result = DDS.ReturnCode.BadParameter;

            if (o != null) {
                int errorCount = 0;
                errorCount += countErrors(o.Presentation);
                errorCount += countErrors(o.Partition);
                errorCount += countErrors(o.GroupData);
                errorCount += countErrors(o.EntityFactory);
                if (errorCount == 0) {
                    result = DDS.ReturnCode.Ok;
                }
            } else {
                ReportStack.Report(result,
                    "PublisherQos 'null' is invalid.");
            }
            return result;
        }

        internal static ReturnCode checkQos(SubscriberQos o)
        {
            ReturnCode result = DDS.ReturnCode.BadParameter;

            if (o != null) {
                int errorCount = 0;
                errorCount += countErrors(o.Presentation);
                errorCount += countErrors(o.Partition);
                errorCount += countErrors(o.GroupData);
                errorCount += countErrors(o.EntityFactory);
                errorCount += countErrors(o.Share);
                if (errorCount == 0) {
                    result = DDS.ReturnCode.Ok;
                }
            } else {
                ReportStack.Report(result,
                    "SubscriberQos 'null' is invalid.");
            }
            return result;
        }

        internal static ReturnCode checkQos(DataWriterQos o)
        {
           ReturnCode result = DDS.ReturnCode.BadParameter;

            if (o != null) {
                int errorCount = 0;
                errorCount += countErrors(o.Durability);
                errorCount += countErrors(o.Deadline);
                errorCount += countErrors(o.LatencyBudget);
                errorCount += countErrors(o.Liveliness);
                errorCount += countErrors(o.Reliability);
                errorCount += countErrors(o.DestinationOrder);
                errorCount += countErrors(o.History);
                errorCount += countErrors(o.ResourceLimits);
                errorCount += countErrors(o.TransportPriority);
                errorCount += countErrors(o.Lifespan);
                errorCount += countErrors(o.UserData);
                errorCount += countErrors(o.Ownership);
                errorCount += countErrors(o.OwnershipStrength);
                errorCount += countErrors(o.WriterDataLifecycle);
                if (errorCount == 0) {
                    result = DDS.ReturnCode.Ok;
                    if ((o.History.Kind == HistoryQosPolicyKind.KeepLastHistoryQos) &&
                        (o.ResourceLimits.MaxSamplesPerInstance != Length.Unlimited) &&
                        (o.History.Depth > o.ResourceLimits.MaxSamplesPerInstance))
                    {
                        ReportStack.Report(result,
                                "HistoryQosPolicy.depth is greater than " +
                                "ResourceLimitsQosPolicy.max_samples_per_instance.");
                        result = DDS.ReturnCode.InconsistentPolicy;
                    }
                }
            } else {
                ReportStack.Report(result,
                    "DataWriterQos 'null' is invalid.");
            }
            return result;
        }

        internal static ReturnCode checkQos(DataReaderQos o)
        {
            ReturnCode result = DDS.ReturnCode.BadParameter;

            if (o != null) {
                int errorCount = 0;
                errorCount += countErrors(o.Durability);
                errorCount += countErrors(o.Deadline);
                errorCount += countErrors(o.LatencyBudget);
                errorCount += countErrors(o.Liveliness);
                errorCount += countErrors(o.Reliability);
                errorCount += countErrors(o.DestinationOrder);
                errorCount += countErrors(o.History);
                errorCount += countErrors(o.ResourceLimits);
                errorCount += countErrors(o.UserData);
                errorCount += countErrors(o.Ownership);
                errorCount += countErrors(o.TimeBasedFilter);
                errorCount += countErrors(o.ReaderDataLifecycle);
                errorCount += countErrors(o.SubscriptionKeys);
                errorCount += countErrors(o.ReaderLifespan);
                errorCount += countErrors(o.Share);
                if (errorCount == 0) {
                    result = DDS.ReturnCode.Ok;
                    if ((o.History.Kind == HistoryQosPolicyKind.KeepLastHistoryQos) &&
                        (o.ResourceLimits.MaxSamplesPerInstance != Length.Unlimited) &&
                        (o.History.Depth > o.ResourceLimits.MaxSamplesPerInstance))
                    {
                        ReportStack.Report(result,
                                "HistoryQosPolicy.depth is greater than " +
                                "ResourceLimitsQosPolicy.max_samples_per_instance.");
                        result = DDS.ReturnCode.InconsistentPolicy;
                    }
                    if ((o.Deadline.Period.Sec < o.TimeBasedFilter.MinimumSeparation.Sec) ||
                        ((o.Deadline.Period.Sec == o.TimeBasedFilter.MinimumSeparation.Sec) &&
                         (o.Deadline.Period.NanoSec < o.TimeBasedFilter.MinimumSeparation.NanoSec)))
                    {
                        ReportStack.Report(result,
                                "DeadlineQosPolicy.period is less than " +
                                "TimeBasedFilterQosPolicy.separation");
                        result = DDS.ReturnCode.InconsistentPolicy;
                    }
                    if (o.ReaderDataLifecycle.EnableInvalidSamples != true)
                    {
                        //TODO: add correct deprecated report
                        if (o.ReaderDataLifecycle.InvalidSampleVisibility.Kind != InvalidSampleVisibilityQosPolicyKind.MinimumInvalidSamples)
                        {
                            ReportStack.Report(result,
                                    "ReaderDataLifecycle.InvalidSampleVisibility.Kind inconsistent with " +
                                    "InvalidSampleVisibilityQosPolicyKind.invalid_sample_visibility.");
                            result = DDS.ReturnCode.InconsistentPolicy;
                        }
                    }
                    if (o.ReaderDataLifecycle.InvalidSampleVisibility.Kind == InvalidSampleVisibilityQosPolicyKind.AllInvalidSamples)
                    {
                        ReportStack.Report(result,
                            "ReaderDataLifecycle.InvalidSampleVisibility.kind 'ALL_INVALID_SAMPLES' is unsupported.");
                        result = DDS.ReturnCode.Unsupported; // See CYCL-15
                    }
                }
            } else {
                ReportStack.Report(result,
                    "DataReaderQos 'null' is invalid.");
            }
            return result;
        }

        internal static ErrorCode ResultToErrorCode(ReturnCode o)
        {
            ErrorCode err;

            switch (o)
            {
                case DDS.ReturnCode.BadParameter:
                    err = DDS.ErrorCode.InvalidValue;
                    break;
                case DDS.ReturnCode.InconsistentPolicy:
                    err = DDS.ErrorCode.InconsistentQos;
                    break;
                case DDS.ReturnCode.Unsupported:
                    err = DDS.ErrorCode.UnsupportedQosPolicy;
                    break;
                default:
                    err = DDS.ErrorCode.Error;
                    break;
            }
            return err;
        }

    }
}
