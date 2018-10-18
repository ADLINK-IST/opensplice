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
using DDS.OpenSplice;
using DDS.OpenSplice.User;
using DDS.OpenSplice.Kernel;
using DDS.OpenSplice.kernelModule;
using DDS.OpenSplice.kernelModuleI;
using DDS.OpenSplice.OS;

namespace DDS.OpenSplice.CustomMarshalers
{
    internal class DomainParticipantQosMarshaler : UserMarshaler<DDS.OpenSplice.kernelModuleI.v_participantQos, DomainParticipantQos>
    {
        /**
         * Constuctors
         **/
        internal DomainParticipantQosMarshaler() : base() { }
        internal DomainParticipantQosMarshaler(IntPtr nativePtr, bool cleanupRequired) : base(nativePtr, cleanupRequired) { }

        /**
         * Copy and cleanup functions.
         **/
        internal override DDS.ReturnCode CopyIn(DomainParticipantQos from, ref v_participantQos to)
        {
            DDS.ReturnCode result;
            if (from != null) {
                to._parent = new v_qos();
                to._parent.kind = V_QOSKIND.V_PARTICIPANT_QOS;
                result = UserDataQosPolicyMarshaler.CopyIn(from.UserData, ref to.userData);
                if (result == DDS.ReturnCode.Ok) {
                    result = EntityFactoryQosPolicyMarshaler.CopyIn(
                            from.EntityFactory, ref to.entityFactory);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = SchedulingQosPolicyMarshaler.CopyIn(
                            from.WatchdogScheduling, ref to.watchdogScheduling);
                }
            } else {
                result = DDS.ReturnCode.BadParameter;
                ReportStack.Report(result, "DomainParticipantQos attribute may not be a null pointer.");
            }
            return result;
        }

        internal override void CleanupIn(ref v_participantQos to)
        {
            UserDataQosPolicyMarshaler.CleanupIn(ref to.userData);
            EntityFactoryQosPolicyMarshaler.CleanupIn(ref to.entityFactory);
            SchedulingQosPolicyMarshaler.CleanupIn(ref to.watchdogScheduling);
        }

        internal override void CopyOut(v_participantQos from, ref DomainParticipantQos to)
        {
            UserDataQosPolicyMarshaler.CopyOut(from.userData, ref to.UserData);
            EntityFactoryQosPolicyMarshaler.CopyOut(from.entityFactory, ref to.EntityFactory);
            SchedulingQosPolicyMarshaler.CopyOut(from.watchdogScheduling, ref to.WatchdogScheduling);
        }
    }

    internal class PublisherQosMarshaler : UserMarshaler<DDS.OpenSplice.kernelModuleI.v_publisherQos, PublisherQos>
    {
        /**
         * Constuctors
         **/
        internal PublisherQosMarshaler() : base() { }
        internal PublisherQosMarshaler(IntPtr nativePtr, bool cleanupRequired) : base(nativePtr, cleanupRequired) { }

        /**
         * Copy and cleanup functions.
         **/
        internal override DDS.ReturnCode CopyIn(PublisherQos from, ref v_publisherQos to)
        {
            DDS.ReturnCode result;
            if (from != null) {
                to._parent = new v_qos();
                to._parent.kind = V_QOSKIND.V_PUBLISHER_QOS;
                result = PresentationQosPolicyMarshaler.CopyIn(
                        from.Presentation, ref to.presentation);
                if (result == DDS.ReturnCode.Ok) {
                    result = PartitionQosPolicyMarshaler.CopyIn(
                            from.Partition, ref to.partition.v);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = GroupDataQosPolicyMarshaler.CopyIn(
                            from.GroupData, ref to.groupData);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = EntityFactoryQosPolicyMarshaler.CopyIn(
                            from.EntityFactory, ref to.entityFactory);
                }
            } else {
                result = DDS.ReturnCode.BadParameter;
                ReportStack.Report(result, "PublisherQos attribute may not be a null pointer.");
            }
            return result;
        }

        internal override void CleanupIn(ref v_publisherQos to)
        {
            PresentationQosPolicyMarshaler.CleanupIn(ref to.presentation);
            PartitionQosPolicyMarshaler.CleanupIn(ref to.partition.v);
            GroupDataQosPolicyMarshaler.CleanupIn(ref to.groupData);
            EntityFactoryQosPolicyMarshaler.CleanupIn(ref to.entityFactory);
        }

        internal override void CopyOut(v_publisherQos from, ref PublisherQos to)
        {
            if (to == null) to = new PublisherQos();
            PresentationQosPolicyMarshaler.CopyOut(from.presentation, ref to.Presentation);
            PartitionQosPolicyMarshaler.CopyOut(from.partition.v, ref to.Partition);
            GroupDataQosPolicyMarshaler.CopyOut(from.groupData, ref to.GroupData);
            EntityFactoryQosPolicyMarshaler.CopyOut(from.entityFactory, ref to.EntityFactory);
        }
    }

    internal class SubscriberQosMarshaler : UserMarshaler<DDS.OpenSplice.kernelModuleI.v_subscriberQos, SubscriberQos>
    {
        /**
         * Constuctors
         **/
        internal SubscriberQosMarshaler() : base() { }
        internal SubscriberQosMarshaler(IntPtr nativePtr, bool cleanupRequired) : base(nativePtr, cleanupRequired) { }

        /**
         * Copy and cleanup functions.
         **/
        internal override DDS.ReturnCode CopyIn(SubscriberQos from, ref v_subscriberQos to)
        {
            DDS.ReturnCode result;
            if (from != null) {
               to._parent = new v_qos();
               to._parent.kind = V_QOSKIND.V_SUBSCRIBER_QOS;
                result = PresentationQosPolicyMarshaler.CopyIn(
                        from.Presentation, ref to.presentation);
                if (result == DDS.ReturnCode.Ok) {
                    result = PartitionQosPolicyMarshaler.CopyIn(
                            from.Partition, ref to.partition.v);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = GroupDataQosPolicyMarshaler.CopyIn(
                            from.GroupData, ref to.groupData);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = EntityFactoryQosPolicyMarshaler.CopyIn(
                            from.EntityFactory, ref to.entityFactory);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = ShareQosPolicyMarshaler.CopyIn(
                            from.Share, ref to.share);
                }
            } else {
                result = DDS.ReturnCode.BadParameter;
                ReportStack.Report(result, "SubscriberQos attribute may not be a null pointer.");
            }
            return result;
        }

        internal override void CleanupIn(ref v_subscriberQos to)
        {
            PresentationQosPolicyMarshaler.CleanupIn(ref to.presentation);
            PartitionQosPolicyMarshaler.CleanupIn(ref to.partition.v);
            GroupDataQosPolicyMarshaler.CleanupIn(ref to.groupData);
            EntityFactoryQosPolicyMarshaler.CleanupIn(ref to.entityFactory);
            ShareQosPolicyMarshaler.CleanupIn(ref to.share);

        }

        internal override void CopyOut(v_subscriberQos from, ref SubscriberQos to)
        {
            if (to == null) to = new SubscriberQos();
            PresentationQosPolicyMarshaler.CopyOut(from.presentation, ref to.Presentation);
            PartitionQosPolicyMarshaler.CopyOut(from.partition.v, ref to.Partition);
            GroupDataQosPolicyMarshaler.CopyOut(from.groupData, ref to.GroupData);
            EntityFactoryQosPolicyMarshaler.CopyOut(from.entityFactory, ref to.EntityFactory);
            ShareQosPolicyMarshaler.CopyOut(from.share, ref to.Share);
        }
    }

    internal class TopicQosMarshaler : UserMarshaler<DDS.OpenSplice.kernelModuleI.v_topicQos, TopicQos>
    {
        /**
         * Constuctors
         **/
        internal TopicQosMarshaler() : base() { }
        internal TopicQosMarshaler(IntPtr nativePtr, bool cleanupRequired) : base(nativePtr, cleanupRequired) { }

        /**
         * Copy and cleanup functions.
         **/
        internal override DDS.ReturnCode CopyIn(TopicQos from, ref v_topicQos to)
        {
            DDS.ReturnCode result;
            if (from != null) {
                to._parent = new v_qos();
                to._parent.kind = V_QOSKIND.V_TOPIC_QOS;
                result = TopicDataQosPolicyMarshaler.CopyIn(
                        from.TopicData, ref to.topicData);
                if (result == DDS.ReturnCode.Ok) {
                    result = DurabilityQosPolicyMarshaler.CopyIn(
                            from.Durability, ref to.durability);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = DurabilityServiceQosPolicyMarshaler.CopyIn(
                            from.DurabilityService, ref to.durabilityService);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = DeadlineQosPolicyMarshaler.CopyIn(
                            from.Deadline, ref to.deadline);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = LatencyBudgetQosPolicyMarshaler.CopyIn(
                            from.LatencyBudget, ref to.latency);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = LivelinessQosPolicyMarshaler.CopyIn(
                            from.Liveliness, ref to.liveliness);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = ReliabilityQosPolicyMarshaler.CopyIn(
                            from.Reliability, ref to.reliability);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = DestinationOrderQosPolicyMarshaler.CopyIn(
                            from.DestinationOrder, ref to._orderby);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = HistoryQosPolicyMarshaler.CopyIn(
                            from.History, ref to.history);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = ResourceLimitsQosPolicyMarshaler.CopyIn(
                            from.ResourceLimits, ref to.resource);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = TransportPriorityQosPolicyMarshaler.CopyIn(
                            from.TransportPriority, ref to.transport);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = LifespanQosPolicyMarshaler.CopyIn(
                            from.Lifespan, ref to.lifespan);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = OwnershipQosPolicyMarshaler.CopyIn(
                            from.Ownership, ref to.ownership);
                }
            } else {
                result = DDS.ReturnCode.BadParameter;
                ReportStack.Report(result, "TopicQos attribute may not be a null pointer.");
            }
            return result;
        }

        internal override void CleanupIn(ref v_topicQos to)
        {
            TopicDataQosPolicyMarshaler.CleanupIn(ref to.topicData);
            DurabilityQosPolicyMarshaler.CleanupIn(ref to.durability);
            DurabilityServiceQosPolicyMarshaler.CleanupIn(ref to.durabilityService);
            DeadlineQosPolicyMarshaler.CleanupIn(ref to.deadline);
            LatencyBudgetQosPolicyMarshaler.CleanupIn(ref to.latency);
            LivelinessQosPolicyMarshaler.CleanupIn(ref to.liveliness);
            ReliabilityQosPolicyMarshaler.CleanupIn(ref to.reliability);
            DestinationOrderQosPolicyMarshaler.CleanupIn(ref to._orderby);
            HistoryQosPolicyMarshaler.CleanupIn(ref to.history);
            ResourceLimitsQosPolicyMarshaler.CleanupIn(ref to.resource);
            TransportPriorityQosPolicyMarshaler.CleanupIn(ref to.transport);
            LifespanQosPolicyMarshaler.CleanupIn(ref to.lifespan);
            OwnershipQosPolicyMarshaler.CleanupIn(ref to.ownership);
        }

        internal override void CopyOut(v_topicQos from, ref TopicQos to)
        {
            if (to == null) to = new TopicQos();
            TopicDataQosPolicyMarshaler.CopyOut(from.topicData, ref to.TopicData);
            DurabilityQosPolicyMarshaler.CopyOut(from.durability, ref to.Durability);
            DurabilityServiceQosPolicyMarshaler.CopyOut(from.durabilityService, ref to.DurabilityService);
            DeadlineQosPolicyMarshaler.CopyOut(from.deadline, ref to.Deadline);
            LatencyBudgetQosPolicyMarshaler.CopyOut(from.latency, ref to.LatencyBudget);
            LivelinessQosPolicyMarshaler.CopyOut(from.liveliness, ref to.Liveliness);
            ReliabilityQosPolicyMarshaler.CopyOut(from.reliability, ref to.Reliability);
            DestinationOrderQosPolicyMarshaler.CopyOut(from._orderby, ref to.DestinationOrder);
            HistoryQosPolicyMarshaler.CopyOut(from.history, ref to.History);
            ResourceLimitsQosPolicyMarshaler.CopyOut(from.resource, ref to.ResourceLimits);
            TransportPriorityQosPolicyMarshaler.CopyOut(from.transport, ref to.TransportPriority);
            LifespanQosPolicyMarshaler.CopyOut(from.lifespan, ref to.Lifespan);
            OwnershipQosPolicyMarshaler.CopyOut(from.ownership, ref to.Ownership);
        }
    }

    internal class DataReaderQosMarshaler : UserMarshaler<DDS.OpenSplice.kernelModuleI.v_readerQos, DataReaderQos>
    {
        /**
         * Constuctors
         **/
        internal DataReaderQosMarshaler() : base() { }
        internal DataReaderQosMarshaler(IntPtr nativePtr, bool cleanupRequired) : base(nativePtr, cleanupRequired) { }

        /**
         * Copy and cleanup functions.
         **/
        internal override DDS.ReturnCode CopyIn(DataReaderQos from, ref v_readerQos to)
        {
            DDS.ReturnCode result;
            if (from != null) {
                to._parent = new v_qos();
                to._parent.kind = V_QOSKIND.V_READER_QOS;
                result = DurabilityQosPolicyMarshaler.CopyIn(
                        from.Durability, ref to.durability);
                if (result == DDS.ReturnCode.Ok) {
                    result = DeadlineQosPolicyMarshaler.CopyIn(
                            from.Deadline, ref to.deadline);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = LatencyBudgetQosPolicyMarshaler.CopyIn(
                            from.LatencyBudget, ref to.latency);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = LivelinessQosPolicyMarshaler.CopyIn(
                            from.Liveliness, ref to.liveliness);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = ReliabilityQosPolicyMarshaler.CopyIn(
                            from.Reliability, ref to.reliability);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = DestinationOrderQosPolicyMarshaler.CopyIn(
                            from.DestinationOrder, ref to._orderby);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = HistoryQosPolicyMarshaler.CopyIn(
                            from.History, ref to.history);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = ResourceLimitsQosPolicyMarshaler.CopyIn(
                            from.ResourceLimits, ref to.resource);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = UserDataQosPolicyMarshaler.CopyIn(
                            from.UserData, ref to.userData);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = OwnershipQosPolicyMarshaler.CopyIn(
                            from.Ownership, ref to.ownership);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = TimeBasedFilterQosPolicyMarshaler.CopyIn(
                            from.TimeBasedFilter, ref to.pacing);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = ReaderDataLifecycleQosPolicyMarshaler.CopyIn(
                            from.ReaderDataLifecycle, ref to.lifecycle);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = SubscriptionKeyQosPolicyMarshaler.CopyIn(
                            from.SubscriptionKeys, ref to.userKey);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = ReaderLifespanQosPolicyMarshaler.CopyIn(
                            from.ReaderLifespan, ref to.lifespan);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = ShareQosPolicyMarshaler.CopyIn(
                            from.Share, ref to.share);
                }
            } else {
                result = DDS.ReturnCode.BadParameter;
                ReportStack.Report(result, "DataReaderQos attribute may not be a null pointer.");
            }
            return result;
        }

        internal override void CleanupIn(ref v_readerQos to)
        {
            DurabilityQosPolicyMarshaler.CleanupIn(ref to.durability);
            DeadlineQosPolicyMarshaler.CleanupIn(ref to.deadline);
            LatencyBudgetQosPolicyMarshaler.CleanupIn(ref to.latency);
            LivelinessQosPolicyMarshaler.CleanupIn(ref to.liveliness);
            ReliabilityQosPolicyMarshaler.CleanupIn(ref to.reliability);
            DestinationOrderQosPolicyMarshaler.CleanupIn(ref to._orderby);
            HistoryQosPolicyMarshaler.CleanupIn(ref to.history);
            ResourceLimitsQosPolicyMarshaler.CleanupIn(ref to.resource);
            UserDataQosPolicyMarshaler.CleanupIn(ref to.userData);
            OwnershipQosPolicyMarshaler.CleanupIn(ref to.ownership);
            TimeBasedFilterQosPolicyMarshaler.CleanupIn(ref to.pacing);
            ReaderDataLifecycleQosPolicyMarshaler.CleanupIn(ref to.lifecycle);
            SubscriptionKeyQosPolicyMarshaler.CleanupIn(ref to.userKey);
            ReaderLifespanQosPolicyMarshaler.CleanupIn(ref to.lifespan);
            ShareQosPolicyMarshaler.CleanupIn(ref to.share);
        }

        internal override void CopyOut(v_readerQos from, ref DataReaderQos to)
        {
            if (to == null) to = new DataReaderQos();
            DurabilityQosPolicyMarshaler.CopyOut(from.durability, ref to.Durability);
            DeadlineQosPolicyMarshaler.CopyOut(from.deadline, ref to.Deadline);
            LatencyBudgetQosPolicyMarshaler.CopyOut(from.latency, ref to.LatencyBudget);
            LivelinessQosPolicyMarshaler.CopyOut(from.liveliness, ref to.Liveliness);
            ReliabilityQosPolicyMarshaler.CopyOut(from.reliability, ref to.Reliability);
            DestinationOrderQosPolicyMarshaler.CopyOut(from._orderby, ref to.DestinationOrder);
            HistoryQosPolicyMarshaler.CopyOut(from.history, ref to.History);
            ResourceLimitsQosPolicyMarshaler.CopyOut(from.resource, ref to.ResourceLimits);
            UserDataQosPolicyMarshaler.CopyOut(from.userData, ref to.UserData);
            OwnershipQosPolicyMarshaler.CopyOut(from.ownership, ref to.Ownership);
            TimeBasedFilterQosPolicyMarshaler.CopyOut(from.pacing, ref to.TimeBasedFilter);
            ReaderDataLifecycleQosPolicyMarshaler.CopyOut(from.lifecycle, ref to.ReaderDataLifecycle);
            SubscriptionKeyQosPolicyMarshaler.CopyOut(from.userKey, ref to.SubscriptionKeys);
            ReaderLifespanQosPolicyMarshaler.CopyOut(from.lifespan, ref to.ReaderLifespan);
            ShareQosPolicyMarshaler.CopyOut(from.share, ref to.Share);
        }
    }

    internal class DataWriterQosMarshaler : UserMarshaler<DDS.OpenSplice.kernelModuleI.v_writerQos, DataWriterQos>
    {
        /**
         * Constuctors
         **/
        internal DataWriterQosMarshaler() : base() { }
        internal DataWriterQosMarshaler(IntPtr nativePtr, bool cleanupRequired) : base(nativePtr, cleanupRequired) { }

        /**
         * Copy and cleanup functions.
         **/
        internal override DDS.ReturnCode CopyIn(DataWriterQos from, ref v_writerQos to)
        {
            DDS.ReturnCode result;
            if (from != null) {
                to._parent = new v_qos();
                to._parent.kind = V_QOSKIND.V_WRITER_QOS;
                result = DurabilityQosPolicyMarshaler.CopyIn(
                        from.Durability, ref to.durability);
                if (result == DDS.ReturnCode.Ok) {
                    result = DeadlineQosPolicyMarshaler.CopyIn(
                            from.Deadline, ref to.deadline);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = LatencyBudgetQosPolicyMarshaler.CopyIn(
                            from.LatencyBudget, ref to.latency);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = LivelinessQosPolicyMarshaler.CopyIn(
                            from.Liveliness, ref to.liveliness);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = ReliabilityQosPolicyMarshaler.CopyIn(
                            from.Reliability, ref to.reliability);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = DestinationOrderQosPolicyMarshaler.CopyIn(
                            from.DestinationOrder, ref to._orderby);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = HistoryQosPolicyMarshaler.CopyIn(
                            from.History, ref to.history);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = ResourceLimitsQosPolicyMarshaler.CopyIn(
                            from.ResourceLimits, ref to.resource);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = TransportPriorityQosPolicyMarshaler.CopyIn(
                            from.TransportPriority, ref to.transport);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = LifespanQosPolicyMarshaler.CopyIn(
                            from.Lifespan, ref to.lifespan);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = UserDataQosPolicyMarshaler.CopyIn(
                            from.UserData, ref to.userData);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = OwnershipQosPolicyMarshaler.CopyIn(
                            from.Ownership, ref to.ownership);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = OwnershipStrengthQosPolicyMarshaler.CopyIn(
                            from.OwnershipStrength, ref to.strength);
                }
                if (result == DDS.ReturnCode.Ok) {
                    result = WriterDataLifecycleQosPolicyMarshaler.CopyIn(
                            from.WriterDataLifecycle, ref to.lifecycle);
                }
            } else {
                result = DDS.ReturnCode.BadParameter;
                ReportStack.Report(result, "DataWriterQos attribute may not be a null pointer.");
            }
            return result;
        }

        internal override void CleanupIn(ref v_writerQos to)
        {
            DurabilityQosPolicyMarshaler.CleanupIn(ref to.durability);
            DeadlineQosPolicyMarshaler.CleanupIn(ref to.deadline);
            LatencyBudgetQosPolicyMarshaler.CleanupIn(ref to.latency);
            LivelinessQosPolicyMarshaler.CleanupIn(ref to.liveliness);
            ReliabilityQosPolicyMarshaler.CleanupIn(ref to.reliability);
            DestinationOrderQosPolicyMarshaler.CleanupIn(ref to._orderby);
            HistoryQosPolicyMarshaler.CleanupIn(ref to.history);
            ResourceLimitsQosPolicyMarshaler.CleanupIn(ref to.resource);
            TransportPriorityQosPolicyMarshaler.CleanupIn(ref to.transport);
            LifespanQosPolicyMarshaler.CleanupIn(ref to.lifespan);
            UserDataQosPolicyMarshaler.CleanupIn(ref to.userData);
            OwnershipQosPolicyMarshaler.CleanupIn(ref to.ownership);
            OwnershipStrengthQosPolicyMarshaler.CleanupIn(ref to.strength);
            WriterDataLifecycleQosPolicyMarshaler.CleanupIn(ref to.lifecycle);
        }

        internal override void CopyOut(v_writerQos from, ref DataWriterQos to)
        {
            if (to == null) to = new DataWriterQos();
            DurabilityQosPolicyMarshaler.CopyOut(from.durability, ref to.Durability);
            DeadlineQosPolicyMarshaler.CopyOut(from.deadline, ref to.Deadline);
            LatencyBudgetQosPolicyMarshaler.CopyOut(from.latency, ref to.LatencyBudget);
            LivelinessQosPolicyMarshaler.CopyOut(from.liveliness, ref to.Liveliness);
            ReliabilityQosPolicyMarshaler.CopyOut(from.reliability, ref to.Reliability);
            DestinationOrderQosPolicyMarshaler.CopyOut(from._orderby, ref to.DestinationOrder);
            HistoryQosPolicyMarshaler.CopyOut(from.history, ref to.History);
            ResourceLimitsQosPolicyMarshaler.CopyOut(from.resource, ref to.ResourceLimits);
            TransportPriorityQosPolicyMarshaler.CopyOut(from.transport, ref to.TransportPriority);
            LifespanQosPolicyMarshaler.CopyOut(from.lifespan, ref to.Lifespan);
            UserDataQosPolicyMarshaler.CopyOut(from.userData, ref to.UserData);
            OwnershipQosPolicyMarshaler.CopyOut(from.ownership, ref to.Ownership);
            OwnershipStrengthQosPolicyMarshaler.CopyOut(from.strength, ref to.OwnershipStrength);
            WriterDataLifecycleQosPolicyMarshaler.CopyOut(from.lifecycle, ref to.WriterDataLifecycle);
        }
    }
}
