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

using System;
using System.Text;
using System.Runtime.InteropServices;
using DDS.OpenSplice;
using DDS.OpenSplice.User;
using DDS.OpenSplice.OS;
using DDS.OpenSplice.Kernel;
using DDS.OpenSplice.kernelModule;
using DDS.OpenSplice.kernelModuleI;
using DDS.OpenSplice.Database;

namespace DDS.OpenSplice.CustomMarshalers
{
    internal class DurationMarshaler
    {
        internal static DDS.ReturnCode CopyIn(Duration from, ref c_time to)
        {
            to.seconds = from.Sec;
            to.nanoseconds = from.NanoSec;
            return DDS.ReturnCode.Ok;
        }

        internal static void CleanupIn(ref c_time to)
        {
        }

        internal static void CopyOut(c_time from, ref Duration to)
        {
            to.Sec = from.seconds;
            to.NanoSec = from.nanoseconds;
        }

        internal static DDS.ReturnCode CopyIn(Duration from, ref long to)
        {
            to = from.OsDuration;
            return DDS.ReturnCode.Ok;
        }

        internal static void CleanupIn(ref long to)
        {
        }

        internal static void CopyOut(long from, ref Duration to)
        {
            to.OsDuration = from;
        }
    }

    internal class UserDataQosPolicyMarshaler
    {
        internal static DDS.ReturnCode CopyIn(UserDataQosPolicy from, ref v_userDataPolicyI_s to)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                to.v.size = from.Value.Length;
                SequenceOctetMarshaler.CopyIn(from.Value, ref to.v._value);
            } else {
                result = DDS.ReturnCode.BadParameter;
                ReportStack.Report(result, "UserDataQosPolicy attribute may not be a null pointer.");
            }
            return result;
        }

        internal static void CleanupIn(ref v_userDataPolicyI_s to)
        {
            SequenceOctetMarshaler.CleanupIn(ref to.v._value);
        }

        internal static void CopyOut(v_userDataPolicyI_s from, ref UserDataQosPolicy to)
        {
            if (to == null) to = new UserDataQosPolicy();
            SequenceOctetMarshaler.CopyOut(from.v._value, ref to.Value, from.v.size);
        }
    }

    internal class EntityFactoryQosPolicyMarshaler
    {
        internal static DDS.ReturnCode CopyIn(EntityFactoryQosPolicy from, ref v_entityFactoryPolicyI_s to)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                // Set autoenable_created_entities field
                to.v.autoenable_created_entities = from.AutoenableCreatedEntities;
            } else {
                result = DDS.ReturnCode.BadParameter;
                ReportStack.Report(result, "EntityFactoryQosPolicy attribute may not be a null pointer.");
            }
            return result;
        }

        internal static void CleanupIn(ref v_entityFactoryPolicyI_s to)
        {
            // Currently nothing to cleanup.
        }

        internal static void CopyOut(v_entityFactoryPolicyI_s from, ref EntityFactoryQosPolicy to)
        {
            if (to == null) to = new EntityFactoryQosPolicy();

            // Set autoenable_created_entities field
            to.AutoenableCreatedEntities = from.v.autoenable_created_entities;
        }
    }

    internal class ShareQosPolicyMarshaler
    {
        internal static DDS.ReturnCode CopyIn(ShareQosPolicy from, ref v_sharePolicyI_s to)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null && (from.Enable == false || from.Name != null)) {
                BaseMarshaler.WriteString(ref to.v.name, from.Name);
                to.v.enable = from.Enable;
            } else {
                result = DDS.ReturnCode.BadParameter;
                ReportStack.Report(result, "ShareQosPolicy attribute may not contain a null pointer.");
            }
            return result;
        }

        internal static void CleanupIn(ref v_sharePolicyI_s to)
        {
            if (to.v.name != IntPtr.Zero)
            {
                BaseMarshaler.ReleaseString(ref to.v.name);
            }
        }

        internal static void CopyOut(v_sharePolicyI_s from, ref ShareQosPolicy to)
        {
            if (to == null) to = new ShareQosPolicy();
            to.Name = BaseMarshaler.ReadString(from.v.name);
            to.Enable = from.v.enable;
        }
    }

    internal class SchedulingQosPolicyMarshaler
    {
        internal static DDS.ReturnCode CopyIn(SchedulingQosPolicy from, ref v_schedulePolicyI_s to)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                // Set scheduling_class field
                to.v.kind = (V_SCHEDULEKIND) from.SchedulingClass.Kind;

                // Set scheduling_priority_kind field
                to.v.priorityKind = (V_SCHEDULEPRIORITYKIND) from.SchedulingPriorityKind.Kind;

                // Set scheduling_priority field
                to.v.priority = from.SchedulingPriority;
            } else {
                result = DDS.ReturnCode.BadParameter;
                ReportStack.Report(result, "SchedulingQosPolicy attribute may not be a null pointer.");
            }
            return result;
        }

        internal static void CleanupIn(ref v_schedulePolicyI_s to)
        {
            // Currently nothing to cleanup.
        }

        internal static void CopyOut(v_schedulePolicyI_s from, ref SchedulingQosPolicy to)
        {
            if (to == null) to = new SchedulingQosPolicy();

            // Get scheduling_class field
            to.SchedulingClass.Kind = (SchedulingClassQosPolicyKind) from.v.kind;

            // Get scheduling_priority_kind field
            to.SchedulingPriorityKind.Kind = (SchedulingPriorityQosPolicyKind) from.v.priorityKind;

            // Get scheduling_priority field
            to.SchedulingPriority = from.v.priority;
        }
    }

    internal class PresentationQosPolicyMarshaler
    {
        internal static DDS.ReturnCode CopyIn(PresentationQosPolicy from, ref v_presentationPolicyI_s to)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                to.v.access_scope = (V_PRESENTATIONKIND) from.AccessScope;
                to.v.coherent_access = from.CoherentAccess;
                to.v.ordered_access = from.OrderedAccess;
            } else {
                result = DDS.ReturnCode.BadParameter;
                ReportStack.Report(result, "PresentationQosPolicy attribute may not be a null pointer.");
            }
            return result;
        }

        internal static void CleanupIn(ref v_presentationPolicyI_s to)
        {
            // Currently nothing to cleanup.
        }

        internal static void CopyOut(v_presentationPolicyI_s from, ref PresentationQosPolicy to)
        {
            if (to == null) to = new PresentationQosPolicy();
            to.AccessScope = (PresentationQosPolicyAccessScopeKind) from.v.access_scope;
            to.CoherentAccess = from.v.coherent_access;
            to.OrderedAccess = from.v.ordered_access;
        }
    }

    internal class PartitionQosPolicyMarshaler
    {
        internal static DDS.ReturnCode CopyIn(PartitionQosPolicy from, ref IntPtr to)
        {
            DDS.ReturnCode result;
            if (from != null) {
                result = SequenceStringMarshaler.CopyIn(from.Name, ref to);
            } else {
                result = DDS.ReturnCode.BadParameter;
                ReportStack.Report(result, "PartitionQosPolicy attribute may not be a null pointer.");
            }
            return result;
        }

        internal static void CleanupIn(ref IntPtr to)
        {
            SequenceStringMarshaler.CleanupIn(ref to);
        }

        internal static void CopyOut(IntPtr from, ref PartitionQosPolicy to)
        {
            if (to == null) to = new PartitionQosPolicy();
            SequenceStringMarshaler.CopyOut(from, ref to.Name);
        }
    }

    internal class GroupDataQosPolicyMarshaler
    {
        internal static DDS.ReturnCode CopyIn(GroupDataQosPolicy from, ref v_groupDataPolicyI_s to)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                to.v.size = from.Value.Length;
                SequenceOctetMarshaler.CopyIn(from.Value, ref to.v._value);
            } else {
                result = DDS.ReturnCode.BadParameter;
                ReportStack.Report(result, "GroupDataQosPolicy attribute may not be a null pointer.");
            }
            return result;
        }

        internal static void CleanupIn(ref v_groupDataPolicyI_s to)
        {
            SequenceOctetMarshaler.CleanupIn(ref to.v._value);
        }

        internal static void CopyOut(v_groupDataPolicyI_s from, ref GroupDataQosPolicy to)
        {
            if (to == null) to = new GroupDataQosPolicy();
            SequenceOctetMarshaler.CopyOut(from.v._value, ref to.Value, from.v.size);
        }
    }

    internal class TopicDataQosPolicyMarshaler
    {
        internal static DDS.ReturnCode CopyIn(TopicDataQosPolicy from, ref v_topicDataPolicyI_s to)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                to.v.size = from.Value.Length;
                SequenceOctetMarshaler.CopyIn(from.Value, ref to.v._value);
            } else {
                result = DDS.ReturnCode.BadParameter;
                ReportStack.Report(result, "TopicDataQosPolicy attribute may not be a null pointer.");
            }
            return result;
        }

        internal static void CleanupIn(ref v_topicDataPolicyI_s to)
        {
            SequenceOctetMarshaler.CleanupIn(ref to.v._value);
        }

        internal static void CopyOut(v_topicDataPolicyI_s from, ref TopicDataQosPolicy to)
        {
            if (to == null) to = new TopicDataQosPolicy();
            SequenceOctetMarshaler.CopyOut(from.v._value, ref to.Value, from.v.size);
        }
    }

    internal class DurabilityQosPolicyMarshaler
    {
        internal static DDS.ReturnCode CopyIn(DurabilityQosPolicy from, ref v_durabilityPolicyI_s to)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                to.v.kind = (V_DURABILITYKIND) from.Kind;
            } else {
                result = DDS.ReturnCode.BadParameter;
                ReportStack.Report(result, "DurabilityQosPolicy attribute may not be a null pointer.");
            }
            return result;
        }

        internal static void CleanupIn(ref v_durabilityPolicyI_s to)
        {
        }

        internal static void CopyOut(v_durabilityPolicyI_s from, ref DurabilityQosPolicy to)
        {
            if (to == null) to = new DurabilityQosPolicy();

            // Get kind field
            to.Kind = (DurabilityQosPolicyKind) from.v.kind;
        }
    }

    internal class DurabilityServiceQosPolicyMarshaler
    {
        internal static DDS.ReturnCode CopyIn(DurabilityServiceQosPolicy from, ref v_durabilityServicePolicyI_s to)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                to.v.history_kind = (V_HISTORYQOSKIND) from.HistoryKind;
                to.v.history_depth = from.HistoryDepth;
                to.v.max_samples = from.MaxSamples;
                to.v.max_instances = from.MaxInstances;
                to.v.max_samples_per_instance = from.MaxSamplesPerInstance;
                result = DurationMarshaler.CopyIn(from.ServiceCleanupDelay, ref to.v.service_cleanup_delay);
            } else {
                result = DDS.ReturnCode.BadParameter;
                ReportStack.Report(result, "DurabilityServiceQosPolicy attribute may not be a null pointer.");
            }
            return result;
        }

        internal static void CleanupIn(ref v_durabilityServicePolicyI_s to)
        {
            DurationMarshaler.CleanupIn(ref to.v.service_cleanup_delay);
        }

        internal static void CopyOut(v_durabilityServicePolicyI_s from, ref DurabilityServiceQosPolicy to)
        {
            if (to == null) to = new DurabilityServiceQosPolicy();
            DurationMarshaler.CopyOut(from.v.service_cleanup_delay, ref to.ServiceCleanupDelay);
            to.HistoryKind = (HistoryQosPolicyKind) from.v.history_kind;
            to.HistoryDepth = from.v.history_depth;
            to.MaxSamples = from.v.max_samples;
            to.MaxInstances = from.v.max_instances;
            to.MaxSamplesPerInstance = from.v.max_samples_per_instance;
        }
    }

    internal class DeadlineQosPolicyMarshaler
    {
        internal static DDS.ReturnCode CopyIn(DeadlineQosPolicy from, ref v_deadlinePolicyI_s to)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                result = DurationMarshaler.CopyIn(from.Period, ref to.v.period);
            } else {
                result = DDS.ReturnCode.BadParameter;
                ReportStack.Report(result, "DeadlineQosPolicy attribute may not be a null pointer.");
            }
            return result;
        }

        internal static void CleanupIn(ref v_deadlinePolicyI_s to)
        {
            DurationMarshaler.CleanupIn(ref to.v.period);
        }

        internal static void CopyOut(v_deadlinePolicyI_s from, ref DeadlineQosPolicy to)
        {
            if (to == null) to = new DeadlineQosPolicy();
            DurationMarshaler.CopyOut(from.v.period, ref to.Period);
        }
    }

    internal class LatencyBudgetQosPolicyMarshaler
    {
        internal static DDS.ReturnCode CopyIn(LatencyBudgetQosPolicy from, ref v_latencyPolicyI_s to)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                result = DurationMarshaler.CopyIn(from.Duration, ref to.v.duration);
            } else {
                result = DDS.ReturnCode.BadParameter;
                ReportStack.Report(result, "LatencyBudgetQosPolicy attribute may not be a null pointer.");
            }
            return result;
        }

        internal static void CleanupIn(ref v_latencyPolicyI_s to)
        {
            DurationMarshaler.CleanupIn(ref to.v.duration);
        }

        internal static void CopyOut(v_latencyPolicyI_s from, ref LatencyBudgetQosPolicy to)
        {
            if (to == null) to = new LatencyBudgetQosPolicy();
            DurationMarshaler.CopyOut(from.v.duration, ref to.Duration);
        }
    }

    internal class LivelinessQosPolicyMarshaler
    {
        internal static DDS.ReturnCode CopyIn(LivelinessQosPolicy from, ref v_livelinessPolicyI_s to)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                to.v.kind = (V_LIVELINESSKIND) from.Kind;
                result = DurationMarshaler.CopyIn(from.LeaseDuration, ref to.v.lease_duration);
            } else {
                result = DDS.ReturnCode.BadParameter;
                ReportStack.Report(result, "LivelinessQosPolicy attribute may not be a null pointer.");
            }
            return result;
        }

        internal static void CleanupIn(ref v_livelinessPolicyI_s to)
        {
            DurationMarshaler.CleanupIn(ref to.v.lease_duration);
        }

        internal static void CopyOut(v_livelinessPolicyI_s from, ref LivelinessQosPolicy to)
        {
            if (to == null) to = new LivelinessQosPolicy();

            to.Kind = (LivelinessQosPolicyKind) from.v.kind;
            DurationMarshaler.CopyOut(from.v.lease_duration, ref to.LeaseDuration);
        }
    }

    internal class ReliabilityQosPolicyMarshaler
    {
        internal static DDS.ReturnCode CopyIn(ReliabilityQosPolicy from, ref v_reliabilityPolicyI_s to)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                to.v.kind = (V_RELIABILITYKIND) from.Kind;
                to.v.synchronous = from.Synchronous;
                result = DurationMarshaler.CopyIn(from.MaxBlockingTime, ref to.v.max_blocking_time);
            } else {
                result = DDS.ReturnCode.BadParameter;
                ReportStack.Report(result, "ReliabilityQosPolicy attribute may not be a null pointer.");
            }
            return result;
        }

        internal static void CleanupIn(ref v_reliabilityPolicyI_s to)
        {
            DurationMarshaler.CleanupIn(ref to.v.max_blocking_time);
        }

        internal static void CopyOut(v_reliabilityPolicyI_s from, ref ReliabilityQosPolicy to)
        {
            if (to == null) to = new ReliabilityQosPolicy();
            to.Kind = (ReliabilityQosPolicyKind) from.v.kind;
            to.Synchronous = from.v.synchronous;
            DurationMarshaler.CopyOut(from.v.max_blocking_time, ref to.MaxBlockingTime);
        }
    }

    internal class DestinationOrderQosPolicyMarshaler
    {
        internal static DDS.ReturnCode CopyIn(DestinationOrderQosPolicy from, ref v_orderbyPolicyI_s to)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                to.v.kind = (V_ORDERBYKIND) from.Kind;
            } else {
                result = DDS.ReturnCode.BadParameter;
                ReportStack.Report(result, "DestinationOrderQosPolicy attribute may not be a null pointer.");
            }
            return result;
        }

        internal static void CleanupIn(ref v_orderbyPolicyI_s to)
        {
        }

        internal static void CopyOut(v_orderbyPolicyI_s from, ref DestinationOrderQosPolicy to)
        {
            if (to == null) to = new DestinationOrderQosPolicy();
            to.Kind = (DestinationOrderQosPolicyKind) from.v.kind;
        }
    }

    internal class HistoryQosPolicyMarshaler
    {
        internal static DDS.ReturnCode CopyIn(HistoryQosPolicy from, ref v_historyPolicyI_s to)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                to.v.kind = (V_HISTORYQOSKIND) from.Kind;
                to.v.depth = from.Depth;
            } else {
                result = DDS.ReturnCode.BadParameter;
                ReportStack.Report(result, "HistoryQosPolicy attribute may not be a null pointer.");
            }
            return result;
        }

        internal static void CleanupIn(ref v_historyPolicyI_s to)
        {
        }

        internal static void CopyOut(v_historyPolicyI_s from, ref HistoryQosPolicy to)
        {
            if (to == null) to = new HistoryQosPolicy();

            to.Kind = (HistoryQosPolicyKind) from.v.kind;
            to.Depth = from.v.depth;
        }
    }

    internal class ResourceLimitsQosPolicyMarshaler
    {
        internal static DDS.ReturnCode CopyIn(ResourceLimitsQosPolicy from, ref v_resourcePolicyI_s to)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                to.v.max_samples = from.MaxSamples;
                to.v.max_instances = from.MaxInstances;
                to.v.max_samples_per_instance = from.MaxSamplesPerInstance;
            } else {
                result = DDS.ReturnCode.BadParameter;
                ReportStack.Report(result, "ResourceLimitsQosPolicy attribute may not be a null pointer.");
            }
            return result;
        }

        internal static void CleanupIn(ref v_resourcePolicyI_s to)
        {
        }

        internal static void CopyOut(v_resourcePolicyI_s from, ref ResourceLimitsQosPolicy to)
        {
            if (to == null) to = new ResourceLimitsQosPolicy();
            to.MaxSamples = from.v.max_samples;
            to.MaxInstances = from.v.max_instances;
            to.MaxSamplesPerInstance = from.v.max_samples_per_instance;
        }
    }

    internal class TransportPriorityQosPolicyMarshaler
    {
        internal static DDS.ReturnCode CopyIn(TransportPriorityQosPolicy from, ref v_transportPolicyI_s to)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                to.v._value = from.Value;
            } else {
                result = DDS.ReturnCode.BadParameter;
                ReportStack.Report(result, "TransportPriorityQosPolicy attribute may not be a null pointer.");
            }
            return result;
        }

        internal static void CleanupIn(ref v_transportPolicyI_s to)
        {
        }

        internal static void CopyOut(v_transportPolicyI_s from, ref TransportPriorityQosPolicy to)
        {
            if (to == null) to = new TransportPriorityQosPolicy();
            to.Value = from.v._value;
        }
    }

    internal class LifespanQosPolicyMarshaler
    {
        internal static DDS.ReturnCode CopyIn(LifespanQosPolicy from, ref v_lifespanPolicyI_s to)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
               result = DurationMarshaler.CopyIn(from.Duration, ref to.v.duration);
            } else {
                result = DDS.ReturnCode.BadParameter;
                ReportStack.Report(result, "LifespanQosPolicy attribute may not be a null pointer.");
            }
            return result;
        }

        internal static void CleanupIn(ref v_lifespanPolicyI_s to)
        {
            DurationMarshaler.CleanupIn(ref to.v.duration);
        }

        internal static void CopyOut(v_lifespanPolicyI_s from, ref LifespanQosPolicy to)
        {
            if (to == null) to = new LifespanQosPolicy();
            DurationMarshaler.CopyOut(from.v.duration, ref to.Duration);
        }
    }

    internal class OwnershipQosPolicyMarshaler
    {
        internal static DDS.ReturnCode CopyIn(OwnershipQosPolicy from, ref v_ownershipPolicyI_s to)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                to.v.kind = (V_OWNERSHIPKIND) from.Kind;
            } else {
                result = DDS.ReturnCode.BadParameter;
                ReportStack.Report(result, "OwnershipQosPolicy attribute may not be a null pointer.");
            }
            return result;
        }

        internal static void CleanupIn(ref v_ownershipPolicyI_s to)
        {
        }

        internal static void CopyOut(v_ownershipPolicyI_s from, ref OwnershipQosPolicy to)
        {
            if (to == null) to = new OwnershipQosPolicy();
            to.Kind = (OwnershipQosPolicyKind) from.v.kind;
        }
    }

    internal class TimeBasedFilterQosPolicyMarshaler
    {
        internal static DDS.ReturnCode CopyIn(TimeBasedFilterQosPolicy from, ref v_pacingPolicyI_s to)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                result = DurationMarshaler.CopyIn(from.MinimumSeparation, ref to.v.minSeperation);
            } else {
                result = DDS.ReturnCode.BadParameter;
                ReportStack.Report(result, "TimeBasedFilterQosPolicy attribute may not be a null pointer.");
            }
            return result;
        }

        internal static void CleanupIn(ref v_pacingPolicyI_s to)
        {
            DurationMarshaler.CleanupIn(ref to.v.minSeperation);
        }

        internal static void CopyOut(v_pacingPolicyI_s from, ref TimeBasedFilterQosPolicy to)
        {
            if (to == null) to = new TimeBasedFilterQosPolicy();
            DurationMarshaler.CopyOut(from.v.minSeperation, ref to.MinimumSeparation);
        }
    }

    internal class ReaderDataLifecycleQosPolicyMarshaler
    {
        internal static DDS.ReturnCode CopyIn(ReaderDataLifecycleQosPolicy from, ref v_readerLifecyclePolicyI_s to)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                to.v.invalid_sample_visibility = (V_INVALIDSAMPLEVISIBILITYKIND) from.InvalidSampleVisibility.Kind;
                to.v.enable_invalid_samples = from.EnableInvalidSamples;
                to.v.autopurge_dispose_all = from.AutopurgeDisposeAll;
                result = DurationMarshaler.CopyIn(from.AutopurgeNowriterSamplesDelay, ref to.v.autopurge_nowriter_samples_delay);
                if (result == DDS.ReturnCode.Ok) {
                    result = DurationMarshaler.CopyIn(from.AutopurgeDisposedSamplesDelay, ref to.v.autopurge_disposed_samples_delay);
                }
            } else {
                result = DDS.ReturnCode.BadParameter;
                ReportStack.Report(result, "ReaderDataLifecycleQosPolicy attribute may not be a null pointer.");
            }
            return result;
        }

        internal static void CleanupIn(ref v_readerLifecyclePolicyI_s to)
        {
            DurationMarshaler.CleanupIn(ref to.v.autopurge_nowriter_samples_delay);
            DurationMarshaler.CleanupIn(ref to.v.autopurge_disposed_samples_delay);
        }

        internal static void CopyOut(v_readerLifecyclePolicyI_s from, ref ReaderDataLifecycleQosPolicy to)
        {
            if (to == null) to = new ReaderDataLifecycleQosPolicy();
            DurationMarshaler.CopyOut(from.v.autopurge_nowriter_samples_delay, ref to.AutopurgeNowriterSamplesDelay);
            DurationMarshaler.CopyOut(from.v.autopurge_disposed_samples_delay, ref to.AutopurgeDisposedSamplesDelay);
            to.EnableInvalidSamples = from.v.enable_invalid_samples;
            if (from.v.enable_invalid_samples) {
                to.InvalidSampleVisibility.Kind = DDS.InvalidSampleVisibilityQosPolicyKind.MinimumInvalidSamples;
            } else {
                to.InvalidSampleVisibility.Kind = DDS.InvalidSampleVisibilityQosPolicyKind.NoInvalidSamples;
            }
        }
    }

    internal class OwnershipStrengthQosPolicyMarshaler
    {
        internal static DDS.ReturnCode CopyIn(OwnershipStrengthQosPolicy from, ref v_strengthPolicyI_s to)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                to.v._value = from.Value;
            } else {
                result = DDS.ReturnCode.BadParameter;
                ReportStack.Report(result, "OwnershipStrengthQosPolicy attribute may not be a null pointer.");
            }
            return result;
        }

        internal static void CleanupIn(ref v_strengthPolicyI_s to)
        {
        }

        internal static void CopyOut(v_strengthPolicyI_s from, ref OwnershipStrengthQosPolicy to)
        {
            if (to == null) to = new OwnershipStrengthQosPolicy();
            to.Value = from.v._value;
        }
    }

    internal class WriterDataLifecycleQosPolicyMarshaler
    {
        internal static DDS.ReturnCode CopyIn(WriterDataLifecycleQosPolicy from, ref v_writerLifecyclePolicyI_s to)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                to.v.autodispose_unregistered_instances = from.AutodisposeUnregisteredInstances;
                result = DurationMarshaler.CopyIn(from.AutopurgeSuspendedSamplesDelay, ref to.v.autopurge_suspended_samples_delay);
                if (result == DDS.ReturnCode.Ok)
                {
                    result = DurationMarshaler.CopyIn(from.AutounregisterInstanceDelay, ref to.v.autounregister_instance_delay);
                }
            } else {
                result = DDS.ReturnCode.BadParameter;
                ReportStack.Report(result, "WriterDataLifecycleQosPolicy attribute may not be a null pointer.");
            }
            return result;
        }

        internal static void CleanupIn(ref v_writerLifecyclePolicyI_s to)
        {
            DurationMarshaler.CleanupIn(ref to.v.autopurge_suspended_samples_delay);
            DurationMarshaler.CleanupIn(ref to.v.autounregister_instance_delay);
        }

        internal static void CopyOut(v_writerLifecyclePolicyI_s from, ref WriterDataLifecycleQosPolicy to)
        {
            if (to == null) to = new WriterDataLifecycleQosPolicy();
            to.AutodisposeUnregisteredInstances = from.v.autodispose_unregistered_instances;
            DurationMarshaler.CopyOut(from.v.autopurge_suspended_samples_delay, ref to.AutopurgeSuspendedSamplesDelay);
            DurationMarshaler.CopyOut(from.v.autounregister_instance_delay, ref to.AutounregisterInstanceDelay);
        }
    }

    internal class SubscriptionKeyQosPolicyMarshaler
    {
        internal static DDS.ReturnCode CopyIn(SubscriptionKeyQosPolicy from, ref v_userKeyPolicyI_s to)
        {
            DDS.ReturnCode result;
            if (from != null) {
                to.v.enable = from.UseKeyList;
                result = SequenceStringMarshaler.CopyIn(from.KeyList, ref to.v.expression);
            } else {
                result = DDS.ReturnCode.BadParameter;
                ReportStack.Report(result, "SubscriptionKeyQosPolicy attribute may not be a null pointer.");
            }
            return result;
        }

        internal static void CleanupIn(ref v_userKeyPolicyI_s to)
        {
            SequenceStringMarshaler.CleanupIn(ref to.v.expression);
        }

        internal static void CopyOut(v_userKeyPolicyI_s from, ref SubscriptionKeyQosPolicy to)
        {
            if (to == null) to = new SubscriptionKeyQosPolicy();
            to.UseKeyList = from.v.enable;
            SequenceStringMarshaler.CopyOut(from.v.expression, ref to.KeyList);
        }
    }

    internal class ReaderLifespanQosPolicyMarshaler
    {
        internal static DDS.ReturnCode CopyIn(ReaderLifespanQosPolicy from, ref v_readerLifespanPolicyI_s to)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                to.v.used = from.UseLifespan;
                result = DurationMarshaler.CopyIn(from.Duration, ref to.v.duration);
            } else {
                result = DDS.ReturnCode.BadParameter;
                ReportStack.Report(result, "ReaderLifespanQosPolicy attribute may not be a null pointer.");
            }
            return result;
        }

        internal static void CleanupIn(ref v_readerLifespanPolicyI_s to)
        {
            DurationMarshaler.CleanupIn(ref to.v.duration);
        }

        internal static void CopyOut(v_readerLifespanPolicyI_s from, ref ReaderLifespanQosPolicy to)
        {
            if (to == null) to = new ReaderLifespanQosPolicy();
            to.UseLifespan = from.v.used;
            DurationMarshaler.CopyOut(from.v.duration, ref to.Duration);
        }
    }
}
