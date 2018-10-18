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
using System.Diagnostics;
using System.Runtime.InteropServices;
using DDS.OpenSplice.OS;
using DDS.OpenSplice.Kernel;
using DDS.OpenSplice.kernelModuleI;
using DDS.OpenSplice.Database;

namespace DDS.OpenSplice.CustomMarshalers
{
    internal class v_handleToInstanceHandleMarshaler
    {
        internal static V_RESULT CopyOut(ref v_handle_s from, ref InstanceHandle to)
        {
            IntPtr instance;
            V_RESULT result = V_RESULT.INTERNAL_ERROR;

            if (from.server == IntPtr.Zero && from.index == 0 && from.serial == 0)
            {
                to = 0;
                result = V_RESULT.OK;
            }
            else
            {
                if (v_handle.Claim(from, out instance) == v_handleResult.V_HANDLE_OK)
                {
                    to = User.InstanceHandle.New(instance);
                    if (v_handle.Release(from) == v_handleResult.V_HANDLE_OK)
                    {
                       result = V_RESULT.OK;
                    }
                }
            }

            return result;
        }
    }

    internal class InconsistentTopicStatusMarshaler
    {
        private static Type type = typeof(DDS.OpenSplice.kernelModuleI.v_inconsistentTopicInfo);

        internal static V_RESULT CopyOut(IntPtr fromPtr, IntPtr toPtr)
        {
            v_inconsistentTopicInfo from = (v_inconsistentTopicInfo)Marshal.PtrToStructure(fromPtr, type);

            GCHandle toGCHandle = GCHandle.FromIntPtr(toPtr);
            InconsistentTopicStatus to = toGCHandle.Target as InconsistentTopicStatus;

            V_RESULT result = CopyOut(ref from, to);

            toGCHandle.Target = to;
            return result;
        }

        internal static V_RESULT CopyOut(ref v_inconsistentTopicInfo from, InconsistentTopicStatus to)
        {
            to.TotalCount = from.totalCount;
            to.TotalCountChange = from.totalChanged;
            return V_RESULT.OK;
        }
    }

    internal class LivelinessChangedStatusMarshaler
    {
        private static Type type = typeof(DDS.OpenSplice.kernelModuleI.v_livelinessChangedInfo);

        internal static V_RESULT CopyOut(IntPtr fromPtr, IntPtr toPtr)
        {
            v_livelinessChangedInfo from = (v_livelinessChangedInfo)Marshal.PtrToStructure(fromPtr, type);

            GCHandle toGCHandle = GCHandle.FromIntPtr(toPtr);
            LivelinessChangedStatus to = toGCHandle.Target as LivelinessChangedStatus;

            V_RESULT result = CopyOut(ref from, to);

            toGCHandle.Target = to;
            return result;
        }

        internal static V_RESULT CopyOut(ref v_livelinessChangedInfo from, LivelinessChangedStatus to)
        {
            to.AliveCount = from.activeCount;
            to.NotAliveCount = from.inactiveCount;
            to.AliveCountChange = from.activeChanged;
            to.NotAliveCountChange = from.inactiveChanged;
            to.LastPublicationHandle = User.InstanceHandle.FromGID(from.instanceHandle);
            return V_RESULT.OK;
        }
    }

    internal class SubscriptionMatchedStatusMarshaler
    {
        private static Type type = typeof(DDS.OpenSplice.kernelModuleI.v_topicMatchInfo);

        internal static V_RESULT CopyOut(IntPtr fromPtr, IntPtr toPtr)
        {
            v_topicMatchInfo from = (v_topicMatchInfo)Marshal.PtrToStructure(fromPtr, type);

            GCHandle toGCHandle = GCHandle.FromIntPtr(toPtr);
            SubscriptionMatchedStatus to = toGCHandle.Target as SubscriptionMatchedStatus;

            V_RESULT result = CopyOut(ref from, to);

            toGCHandle.Target = to;
            return result;
        }

        internal static V_RESULT CopyOut(ref v_topicMatchInfo from, SubscriptionMatchedStatus to)
        {
            to.TotalCount = from.totalCount;
            to.TotalCountChange = from.totalChanged;
            to.CurrentCount = from.currentCount;
            to.CurrentCountChange = from.currentChanged;
            to.LastPublicationHandle = User.InstanceHandle.FromGID(from.instanceHandle);
            return V_RESULT.OK;
        }
    }

    internal class SampleRejectedStatusMarshaler
    {
        private static Type type = typeof(DDS.OpenSplice.kernelModuleI.v_sampleRejectedInfo);

        internal static V_RESULT CopyOut(IntPtr fromPtr, IntPtr toPtr)
        {
            v_sampleRejectedInfo from = (v_sampleRejectedInfo)Marshal.PtrToStructure(fromPtr, type);

            GCHandle toGCHandle = GCHandle.FromIntPtr(toPtr);
            SampleRejectedStatus to = toGCHandle.Target as SampleRejectedStatus;

            V_RESULT result = CopyOut(ref from, to);

            toGCHandle.Target = to;
            return result;
        }

        internal static V_RESULT CopyOut(ref v_sampleRejectedInfo from, SampleRejectedStatus to)
        {
            to.TotalCount = from.totalCount;
            to.TotalCountChange = from.totalChanged;
            to.LastReason = (SampleRejectedStatusKind) from.lastReason;
            to.LastInstanceHandle = User.InstanceHandle.FromGID(from.instanceHandle);
            return V_RESULT.OK;
        }
    }

    internal class RequestedDeadlineMissedStatusMarshaler
    {
        private static Type type = typeof(DDS.OpenSplice.kernelModuleI.v_deadlineMissedInfo);

        internal static V_RESULT CopyOut(IntPtr fromPtr, IntPtr toPtr)
        {
            v_deadlineMissedInfo from = (v_deadlineMissedInfo)Marshal.PtrToStructure(fromPtr, type);

            GCHandle toGCHandle = GCHandle.FromIntPtr(toPtr);
            RequestedDeadlineMissedStatus to = toGCHandle.Target as RequestedDeadlineMissedStatus;

            V_RESULT result = CopyOut(ref from, to);

            toGCHandle.Target = to;
            return result;
        }

        internal static V_RESULT CopyOut(ref v_deadlineMissedInfo from, RequestedDeadlineMissedStatus to)
        {
            to.TotalCount = from.totalCount;
            to.TotalCountChange = from.totalChanged;
            return v_handleToInstanceHandleMarshaler.CopyOut(ref from.instanceHandle, ref to.LastInstanceHandle);
        }
    }

    internal class RequestedIncompatibleQosStatusMarshaler
    {
        private static Type type = typeof(DDS.OpenSplice.kernelModuleI.v_incompatibleQosInfo);

        internal static V_RESULT CopyOut(IntPtr fromPtr, IntPtr toPtr)
        {
            v_incompatibleQosInfo from = (v_incompatibleQosInfo)Marshal.PtrToStructure(fromPtr, type);

            GCHandle toGCHandle = GCHandle.FromIntPtr(toPtr);
            RequestedIncompatibleQosStatus to = toGCHandle.Target as RequestedIncompatibleQosStatus;

            V_RESULT result = CopyOut(ref from, to);

            toGCHandle.Target = to;
            return result;
        }

        internal static V_RESULT CopyOut(ref v_incompatibleQosInfo from, RequestedIncompatibleQosStatus to)
        {
            to.TotalCount = from.totalCount;
            to.TotalCountChange = from.totalChanged;
            to.LastPolicyId = (QosPolicyId) from.lastPolicyId;
            if (from.totalCount > 0)
            {
                int arrSize = 0;
                for (int i = 0; i < Constants.V_POLICY_ID_COUNT; i++)
                {
                    int v = from.policyCount[i];
                    if (v != 0)
                    {
                        arrSize++;
                    }
                }

                int n = 0;
                to.Policies = new QosPolicyCount[arrSize];
                for (int i = 0; i < Constants.V_POLICY_ID_COUNT; i++)
                {
                    int v = from.policyCount[i];

                    if (v != 0)
                    {
                        to.Policies[n] = new QosPolicyCount();
                        to.Policies[n].PolicyId = (QosPolicyId) i;
                        to.Policies[n].Count = v;
                        n++;
                    }
                }
            }
            return V_RESULT.OK;
        }
    }

    internal class SampleLostStatusMarshaler
    {
        private static Type type = typeof(DDS.OpenSplice.kernelModuleI.v_sampleLostInfo);

        internal static V_RESULT CopyOut(IntPtr fromPtr, IntPtr toPtr)
        {
            v_sampleLostInfo from = (v_sampleLostInfo)Marshal.PtrToStructure(fromPtr, type);

            GCHandle toGCHandle = GCHandle.FromIntPtr(toPtr);
            SampleLostStatus to = toGCHandle.Target as SampleLostStatus;

            V_RESULT result = CopyOut(ref from, to);

            toGCHandle.Target = to;
            return result;
        }

        internal static V_RESULT CopyOut(ref v_sampleLostInfo from, SampleLostStatus to)
        {
            to.TotalCount = from.totalCount;
            to.TotalCountChange = from.totalChanged;
            return V_RESULT.OK;
        }
    }

    internal class LivelinessLostStatusMarshaler
    {
        private static Type type = typeof(DDS.OpenSplice.kernelModuleI.v_livelinessLostInfo);

        internal static V_RESULT CopyOut(IntPtr fromPtr, IntPtr toPtr)
        {
            v_livelinessLostInfo from = (v_livelinessLostInfo)Marshal.PtrToStructure(fromPtr, type);

            GCHandle toGCHandle = GCHandle.FromIntPtr(toPtr);
            LivelinessLostStatus to = toGCHandle.Target as LivelinessLostStatus;

            V_RESULT result = CopyOut(ref from, to);

            toGCHandle.Target = to;
            return result;
        }

        internal static V_RESULT CopyOut(ref v_livelinessLostInfo from, LivelinessLostStatus to)
        {
            to.TotalCount = from.totalCount;
            to.TotalCountChange = from.totalChanged;
            return V_RESULT.OK;
        }
    }

    internal class OfferedDeadlineMissedStatusMarshaler
    {
        private static Type type = typeof(DDS.OpenSplice.kernelModuleI.v_deadlineMissedInfo);

        internal static V_RESULT CopyOut(IntPtr fromPtr, IntPtr toPtr)
        {
            v_deadlineMissedInfo from = (v_deadlineMissedInfo)Marshal.PtrToStructure(fromPtr, type);

            GCHandle toGCHandle = GCHandle.FromIntPtr(toPtr);
            OfferedDeadlineMissedStatus to = toGCHandle.Target as OfferedDeadlineMissedStatus;

            V_RESULT result = CopyOut(ref from, to);

            toGCHandle.Target = to;
            return result;
        }

        internal static V_RESULT CopyOut(ref v_deadlineMissedInfo from, OfferedDeadlineMissedStatus to)
        {
            to.TotalCount = from.totalCount;
            to.TotalCountChange = from.totalChanged;
            return v_handleToInstanceHandleMarshaler.CopyOut(ref from.instanceHandle, ref to.LastInstanceHandle);
        }
    }

    internal class OfferedIncompatibleQosStatusMarshaler
    {
        private static Type type = typeof(DDS.OpenSplice.kernelModuleI.v_incompatibleQosInfo);

        internal static V_RESULT CopyOut(IntPtr fromPtr, IntPtr toPtr)
        {
            v_incompatibleQosInfo from = (v_incompatibleQosInfo)Marshal.PtrToStructure(fromPtr, type);

            GCHandle toGCHandle = GCHandle.FromIntPtr(toPtr);
            OfferedIncompatibleQosStatus to = toGCHandle.Target as OfferedIncompatibleQosStatus;

            V_RESULT result = CopyOut(ref from, to);

            toGCHandle.Target = to;
            return result;
        }

        internal static V_RESULT CopyOut(ref v_incompatibleQosInfo from, OfferedIncompatibleQosStatus to)
        {
            to.TotalCount = from.totalCount;
            to.TotalCountChange = from.totalChanged;
            to.LastPolicyId = (QosPolicyId) from.lastPolicyId;
            if (from.totalCount > 0)
            {
                int arrSize = 0;
                for (int i = 0; i < Constants.V_POLICY_ID_COUNT; i++)
                {
                    int v = from.policyCount[i];

                    if (v != 0)
                    {
                        arrSize++;
                    }
                }

                int n = 0;
                to.Policies = new QosPolicyCount[arrSize];
                for (int i = 0; i < Constants.V_POLICY_ID_COUNT; i++)
                {
                    int v = from.policyCount[i];

                    if (v != 0)
                    {
                        to.Policies[n] = new QosPolicyCount();
                        to.Policies[n].PolicyId = (QosPolicyId) i;
                        to.Policies[n].Count = v;
                        n++;
                    }
                }
            }
            return V_RESULT.OK;
        }
    }

    internal class PublicationMatchedStatusMarshaler
    {
        private static Type type = typeof(DDS.OpenSplice.kernelModuleI.v_topicMatchInfo);

        internal static V_RESULT CopyOut(IntPtr fromPtr, IntPtr toPtr)
        {
            v_topicMatchInfo from = (v_topicMatchInfo)Marshal.PtrToStructure(fromPtr, type);

            GCHandle toGCHandle = GCHandle.FromIntPtr(toPtr);
            PublicationMatchedStatus to = toGCHandle.Target as PublicationMatchedStatus;

            V_RESULT result = CopyOut(ref from, to);

            toGCHandle.Target = to;
            return result;
        }

        internal static V_RESULT CopyOut(ref v_topicMatchInfo from, PublicationMatchedStatus to)
        {
            to.TotalCount = from.totalCount;
            to.TotalCountChange = from.totalChanged;
            to.CurrentCount = from.currentCount;
            to.CurrentCountChange = from.currentChanged;
            to.LastSubscriptionHandle = User.InstanceHandle.FromGID(from.instanceHandle);
            return V_RESULT.OK;
        }
    }

    internal class vEventMarshaler
    {
        internal static StatusKind vEventMaskToStatusMask(uint vMask, V_KIND vKind)
        {
            StatusKind mask = 0;

            switch(vKind) {
            case V_KIND.K_TOPIC:
                if ((vMask & (uint) V_EVENT.INCONSISTENT_TOPIC) != 0) {
                    mask |= StatusKind.InconsistentTopic;
                }
                if ((vMask & (uint) V_EVENT.ALL_DATA_DISPOSED) != 0) {
                    mask |= StatusKind.AllDataDisposed;
                }
                break;
            case V_KIND.K_SUBSCRIBER:
                if ((vMask & (uint) V_EVENT.ON_DATA_ON_READERS) != 0) {
                    mask |= StatusKind.DataOnReaders;
                }
                break;
            case V_KIND.K_WRITER:
                if ((vMask & (uint) V_EVENT.LIVELINESS_LOST) != 0) {
                    mask |= StatusKind.LivelinessLost;
                }
                if ((vMask & (uint) V_EVENT.OFFERED_DEADLINE_MISSED) != 0) {
                    mask |= StatusKind.OfferedDeadlineMissed;
                }
                if ((vMask & (uint) V_EVENT.OFFERED_INCOMPATIBLE_QOS) != 0) {
                    mask |= StatusKind.OfferedIncompatibleQos;
                }
                if ((vMask & (uint) V_EVENT.PUBLICATION_MATCHED) != 0) {
                    mask |= StatusKind.PublicationMatched;
                }
                break;
            case V_KIND.K_READER:
            case V_KIND.K_GROUPQUEUE:
            case V_KIND.K_DATAREADER:
            case V_KIND.K_GROUPSTREAM:
                if ((vMask & (uint) V_EVENT.SAMPLE_REJECTED) != 0) {
                    mask |= StatusKind.SampleRejected;
                }
                if ((vMask & (uint) V_EVENT.LIVELINESS_CHANGED) != 0) {
                    mask |= StatusKind.LivelinessChanged;
                }
                if ((vMask & (uint) V_EVENT.REQUESTED_DEADLINE_MISSED) != 0) {
                    mask |= StatusKind.RequestedDeadlineMissed;
                }
                if ((vMask & (uint) V_EVENT.REQUESTED_INCOMPATIBLE_QOS) != 0) {
                    mask |= StatusKind.RequestedIncompatibleQos;
                }
                if ((vMask & (uint) V_EVENT.SUBSCRIPTION_MATCHED) != 0) {
                    mask |= StatusKind.SubscriptionMatched;
                }
                if ((vMask & (uint) V_EVENT.DATA_AVAILABLE) != 0) {
                    mask |= StatusKind.DataAvailable;
                }
                if ((vMask & (uint) V_EVENT.SAMPLE_LOST) != 0) {
                    mask |= StatusKind.SampleLost;
                }
                break;
            case V_KIND.K_PARTICIPANT:
            case V_KIND.K_PUBLISHER:
            case V_KIND.K_DOMAIN:
            case V_KIND.K_KERNEL:
                break;
            default:
                Debug.Assert(false);
                break;
            }

            return mask;
        }

        internal static uint vEventMaskFromStatusMask(StatusKind mask)
        {
            uint vMask = 0;

            if ((mask & StatusKind.InconsistentTopic) != 0) {
                vMask |= (uint) V_EVENT.INCONSISTENT_TOPIC;
            }
            if ((mask & StatusKind.LivelinessLost) != 0) {
                vMask |= (uint) V_EVENT.LIVELINESS_LOST;
            }
            if ((mask & StatusKind.OfferedDeadlineMissed) != 0) {
                vMask |= (uint) V_EVENT.OFFERED_DEADLINE_MISSED;
            }
            if ((mask & StatusKind.OfferedIncompatibleQos) != 0) {
                vMask |= (uint) V_EVENT.OFFERED_INCOMPATIBLE_QOS;
            }
            if ((mask & StatusKind.DataOnReaders) != 0) {
                vMask |= (uint) V_EVENT.ON_DATA_ON_READERS;
            }
            if ((mask & StatusKind.SampleLost) != 0) {
                vMask |= (uint) V_EVENT.SAMPLE_LOST;
            }
            if ((mask & StatusKind.DataAvailable) != 0) {
                vMask |= (uint) V_EVENT.DATA_AVAILABLE;
            }
            if ((mask & StatusKind.SampleRejected) != 0) {
                vMask |= (uint) V_EVENT.SAMPLE_REJECTED;
            }
            if ((mask & StatusKind.LivelinessChanged) != 0) {
                vMask |= (uint) V_EVENT.LIVELINESS_CHANGED;
            }
            if ((mask & StatusKind.RequestedDeadlineMissed) != 0) {
                vMask |= (uint) V_EVENT.REQUESTED_DEADLINE_MISSED;
            }
            if ((mask & StatusKind.RequestedIncompatibleQos) != 0) {
                vMask |= (uint) V_EVENT.REQUESTED_INCOMPATIBLE_QOS;
            }
            if ((mask & StatusKind.PublicationMatched) != 0) {
                vMask |= (uint) V_EVENT.PUBLICATION_MATCHED;
            }
            if ((mask & StatusKind.SubscriptionMatched) != 0) {
                vMask |= (uint) V_EVENT.SUBSCRIPTION_MATCHED;
            }
            if ((mask & StatusKind.AllDataDisposed) != 0) {
                vMask |= (uint) V_EVENT.ALL_DATA_DISPOSED;
            }
            return vMask;
        }
    }


    internal class vStatusMarshaler
    {
        private static Type type = typeof(DDS.OpenSplice.kernelModuleI.v_status);

        internal static V_RESULT CopyOut(IntPtr fromPtr, IntPtr toPtr)
        {
            v_status from = (v_status)Marshal.PtrToStructure(fromPtr, type);

            GCHandle toGCHandle = GCHandle.FromIntPtr(toPtr);
            DDS.OpenSplice.Common.EntityStatus to = toGCHandle.Target as DDS.OpenSplice.Common.EntityStatus;

            V_RESULT result = CopyOut(ref from, to);

            toGCHandle.Target = to;
            return result;
        }

        internal static V_RESULT CopyOut(ref v_status from, DDS.OpenSplice.Common.EntityStatus to)
        {
            to.State = (uint)from.state;
            return V_RESULT.OK;
        }
    }

    internal class vTopicStatusMarshaler
    {
        private static Type type = typeof(DDS.OpenSplice.kernelModuleI.v_topicStatus);

        internal static V_RESULT CopyOut(IntPtr fromPtr, IntPtr toPtr)
        {
            v_topicStatus from = (v_topicStatus)Marshal.PtrToStructure(fromPtr, type);

            GCHandle toGCHandle = GCHandle.FromIntPtr(toPtr);
            DDS.OpenSplice.Common.TopicStatus to = toGCHandle.Target as DDS.OpenSplice.Common.TopicStatus;

            V_RESULT result = CopyOut(ref from, to);

            toGCHandle.Target = to;
            return result;
        }

        internal static V_RESULT CopyOut(ref v_topicStatus from, DDS.OpenSplice.Common.TopicStatus to)
        {
            to.State = (uint)from._parent.state;
            to.InconsistentTopic = new InconsistentTopicStatus();

            InconsistentTopicStatusMarshaler.CopyOut(ref from.inconsistentTopic, to.InconsistentTopic);
            return V_RESULT.OK;
        }
    }

    internal class vWriterStatusMarshaler
    {
        private static Type type = typeof(DDS.OpenSplice.kernelModuleI.v_writerStatus);

        internal static V_RESULT CopyOut(IntPtr fromPtr, IntPtr toPtr)
        {
            v_writerStatus from = (v_writerStatus)Marshal.PtrToStructure(fromPtr, type);

            GCHandle toGCHandle = GCHandle.FromIntPtr(toPtr);
            DDS.OpenSplice.Common.WriterStatus to = toGCHandle.Target as DDS.OpenSplice.Common.WriterStatus;

            V_RESULT result = CopyOut(ref from, to);

            toGCHandle.Target = to;
            return result;
        }

        internal static V_RESULT CopyOut(ref v_writerStatus from, DDS.OpenSplice.Common.WriterStatus to)
        {
            to.State = (uint)from._parent.state;
            to.LivelinessLost = new LivelinessLostStatus();
            to.DeadlineMissed = new OfferedDeadlineMissedStatus();
            to.IncompatibleQos = new OfferedIncompatibleQosStatus();
            to.PublicationMatch = new PublicationMatchedStatus();

            LivelinessLostStatusMarshaler.CopyOut(ref from.livelinessLost, to.LivelinessLost);
            OfferedDeadlineMissedStatusMarshaler.CopyOut(ref from.deadlineMissed, to.DeadlineMissed);
            OfferedIncompatibleQosStatusMarshaler.CopyOut(ref from.incompatibleQos, to.IncompatibleQos);
            PublicationMatchedStatusMarshaler.CopyOut(ref from.publicationMatch, to.PublicationMatch);
            return V_RESULT.OK;
        }
    }

    internal class vReaderStatusMarshaler
    {
        private static Type type = typeof(DDS.OpenSplice.kernelModuleI.v_readerStatus);

        internal static V_RESULT CopyOut(IntPtr fromPtr, IntPtr toPtr)
        {
            v_readerStatus from = (v_readerStatus)Marshal.PtrToStructure(fromPtr, type);

            GCHandle toGCHandle = GCHandle.FromIntPtr(toPtr);
            DDS.OpenSplice.Common.ReaderStatus to = toGCHandle.Target as DDS.OpenSplice.Common.ReaderStatus;

            V_RESULT result = CopyOut(ref from, to);

            toGCHandle.Target = to;
            return result;
        }

        internal static V_RESULT CopyOut(ref v_readerStatus from, DDS.OpenSplice.Common.ReaderStatus to)
        {
            to.State = (uint)from._parent.state;
            to.LivelinessChanged = new LivelinessChangedStatus();
            to.SampleRejected = new SampleRejectedStatus();
            to.SampleLost = new SampleLostStatus();
            to.DeadlineMissed = new RequestedDeadlineMissedStatus();
            to.IncompatibleQos = new RequestedIncompatibleQosStatus();
            to.SubscriptionMatch = new SubscriptionMatchedStatus();

            LivelinessChangedStatusMarshaler.CopyOut(ref from.livelinessChanged, to.LivelinessChanged);
            SampleRejectedStatusMarshaler.CopyOut(ref from.sampleRejected, to.SampleRejected);
            SampleLostStatusMarshaler.CopyOut(ref from.sampleLost, to.SampleLost);
            RequestedDeadlineMissedStatusMarshaler.CopyOut(ref from.deadlineMissed, to.DeadlineMissed);
            RequestedIncompatibleQosStatusMarshaler.CopyOut(ref from.incompatibleQos, to.IncompatibleQos);
            SubscriptionMatchedStatusMarshaler.CopyOut(ref from.subscriptionMatch, to.SubscriptionMatch);
            return V_RESULT.OK;
        }
    }
}
