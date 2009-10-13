// The OpenSplice DDS Community Edition project.
//
// Copyright (C) 2006 to 2009 PrismTech Limited and its licensees.
// Copyright (C) 2009  L-3 Communications / IS
// 
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License Version 3 dated 29 June 2007, as published by the
//  Free Software Foundation.
// 
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
// 
//  You should have received a copy of the GNU Lesser General Public
//  License along with OpenSplice DDS Community Edition; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

using System;
using System.Text;
using System.Runtime.InteropServices;

namespace DDS.OpenSplice.CustomMarshalers
{
    internal class BuiltinTopicKeyMarshaler : IMarshaler
    {
        private static readonly Type type = typeof(BuiltinTopicKey);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_SystemId = (int)Marshal.OffsetOf(type, "SystemId");
        private static readonly int offset_LocalId = (int)Marshal.OffsetOf(type, "LocalId");
        private static readonly int offset_Serial = (int)Marshal.OffsetOf(type, "Serial");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public BuiltinTopicKeyMarshaler(BuiltinTopicKey from)
            : this()
        {
            CopyIn(ref from, gapiPtr, 0);
            cleanupRequired = true;
        }

        public BuiltinTopicKeyMarshaler()
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.Alloc(Size);
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(gapiPtr, 0);
            }

            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
        }

        internal static void CopyIn(ref BuiltinTopicKey from, IntPtr to, int offset)
        {
            BaseMarshaler.Write(to, offset + offset_SystemId, from.SystemId);
            BaseMarshaler.Write(to, offset + offset_LocalId, from.LocalId);
            BaseMarshaler.Write(to, offset + offset_Serial, from.Serial);
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
        }

        internal void CopyOut(out BuiltinTopicKey to)
        {
            CopyOut(gapiPtr, out to, 0);
        }

        internal static void CopyOut(IntPtr from, out BuiltinTopicKey to, int offset)
        {
            to.SystemId = BaseMarshaler.ReadUInt32(from, offset + offset_SystemId);
            to.LocalId = BaseMarshaler.ReadUInt32(from, offset + offset_LocalId);
            to.Serial = BaseMarshaler.ReadUInt32(from, offset + offset_Serial);
        }
    }

    internal class UserDataQosPolicyMarshaler : IMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_userDataQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_value = (int)Marshal.OffsetOf(type, "value");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public UserDataQosPolicyMarshaler(UserDataQosPolicy from)
            : this()
        {
            CopyIn(ref from, gapiPtr, 0);
            cleanupRequired = true;
        }

        public UserDataQosPolicyMarshaler()
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.Alloc(Size);
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(gapiPtr, 0);
            }

            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
        }

        internal static void CopyIn(ref UserDataQosPolicy from, IntPtr to, int offset)
        {
            SequenceOctetMarshaler.CopyIn(from.Value, to, offset + offset_value);
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
            SequenceOctetMarshaler.CleanupIn(nativePtr, offset + offset_value);
        }

        internal void CopyOut(out UserDataQosPolicy to)
        {
            CopyOut(gapiPtr, out to, 0);
        }

        internal static void CopyOut(IntPtr from, out UserDataQosPolicy to, int offset)
        {
            SequenceOctetMarshaler.CopyOut(from, out to.Value, offset + offset_value);
        }
    }

    internal class EntityFactoryQosPolicyMarshaler : IMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_entityFactoryQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_autoenable_created_entities = (int)Marshal.OffsetOf(type, "autoenable_created_entities");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public EntityFactoryQosPolicyMarshaler(EntityFactoryQosPolicy from)
            : this()
        {
            CopyIn(ref from, gapiPtr, 0);
            cleanupRequired = true;
        }

        public EntityFactoryQosPolicyMarshaler()
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.Alloc(Size);
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(gapiPtr, 0);
            }

            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
        }

        internal static void CopyIn(ref EntityFactoryQosPolicy from, IntPtr to, int offset)
        {
            // Set autoenable_created_entities field
            BaseMarshaler.Write(to, offset + offset_autoenable_created_entities, from.AutoEnableCreatedEntities);
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
            // Currently nothing to cleanup.
        }

        internal void CopyOut(out EntityFactoryQosPolicy to)
        {
            CopyOut(gapiPtr, out to, 0);
        }

        internal static void CopyOut(IntPtr from, out EntityFactoryQosPolicy to, int offset)
        {
            // Set autoenable_created_entities field
            to.AutoEnableCreatedEntities = BaseMarshaler.ReadBoolean(from, offset + offset_autoenable_created_entities);
        }
    }

    internal class ShareQosPolicyMarshaler : IMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_shareQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_name = (int)Marshal.OffsetOf(type, "name");
        private static readonly int offset_enable = (int)Marshal.OffsetOf(type, "enable");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public ShareQosPolicyMarshaler(ShareQosPolicy from)
            : this()
        {
            CopyIn(ref from, gapiPtr, 0);
            cleanupRequired = true;
        }

        public ShareQosPolicyMarshaler()
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.Alloc(Size);
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(gapiPtr, 0);
            }

            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
        }

        internal static void CopyIn(ref ShareQosPolicy from, IntPtr to, int offset)
        {
            IntPtr stringPtr = OpenSplice.Gapi.GenericAllocRelease.string_dup(from.Name);
            //            IntPtr stringPtr = Marshal.StringToHGlobalAnsi(from.Name);
            BaseMarshaler.Write(to, offset + offset_name, stringPtr);
            BaseMarshaler.Write(to, offset + offset_enable, from.Enable);
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
            IntPtr stringPtr = BaseMarshaler.ReadIntPtr(nativePtr, offset + offset_name);
            OpenSplice.Gapi.GenericAllocRelease.Free(stringPtr);
            //            Marshal.FreeHGlobal(stringPtr);
            BaseMarshaler.Write(nativePtr, offset + offset_name, IntPtr.Zero);
        }

        internal void CopyOut(out ShareQosPolicy to)
        {
            CopyOut(gapiPtr, out to, 0);
        }

        internal static void CopyOut(IntPtr from, out ShareQosPolicy to, int offset)
        {
            IntPtr ptr = new IntPtr((long)from + offset + offset_name);
            to.Name = Marshal.PtrToStringAnsi(ptr);
            to.Enable = BaseMarshaler.ReadBoolean(from, offset + offset_enable);
        }
    }

    internal class SchedulingQosPolicyMarshaler : IMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_schedulingQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_scheduling_class = (int)Marshal.OffsetOf(type, "scheduling_class");
        private static readonly int offset_scheduling_priority_kind = (int)Marshal.OffsetOf(type, "scheduling_priority_kind");
        private static readonly int offset_scheduling_priority = (int)Marshal.OffsetOf(type, "scheduling_priority");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public SchedulingQosPolicyMarshaler(SchedulingQosPolicy from)
            : this()
        {
            CopyIn(ref from, gapiPtr, 0);
            cleanupRequired = true;
        }

        public SchedulingQosPolicyMarshaler()
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.Alloc(Size);
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(gapiPtr, 0);
            }

            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
        }

        internal static void CopyIn(ref SchedulingQosPolicy from, IntPtr to, int offset)
        {
            // Set scheduling_class field
            BaseMarshaler.Write(to, offset + offset_scheduling_class, (int)from.SchedulingClass.Kind);

            // Set scheduling_priority_kind field
            BaseMarshaler.Write(to, offset + offset_scheduling_priority_kind, (int)from.SchedulingPriorityKind.Kind);

            // Set scheduling_priority field
            BaseMarshaler.Write(to, offset + offset_scheduling_priority, from.SchedulingPriority);
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
            // Currently nothing to cleanup.
        }

        internal void CopyOut(out SchedulingQosPolicy to)
        {
            CopyOut(gapiPtr, out to, 0);
        }

        internal static void CopyOut(IntPtr from, out SchedulingQosPolicy to, int offset)
        {
            // Get scheduling_class field
            to.SchedulingClass.Kind = (SchedulingClassQosPolicyKind)
                BaseMarshaler.ReadInt32(from, offset + offset_scheduling_class);

            // Get scheduling_priority_kind field
            to.SchedulingPriorityKind.Kind = (SchedulingPriorityQosPolicyKind)
                BaseMarshaler.ReadInt32(from, offset + offset_scheduling_priority_kind);

            // Get scheduling_priority field
            to.SchedulingPriority = BaseMarshaler.ReadInt32(from, offset + offset_scheduling_priority);
        }
    }

    internal class PresentationQosPolicyMarshaler : IMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_presentationQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_access_scope = (int)Marshal.OffsetOf(type, "access_scope");
        private static readonly int offset_coherent_access = (int)Marshal.OffsetOf(type, "coherent_access");
        private static readonly int offset_ordered_access = (int)Marshal.OffsetOf(type, "ordered_access");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public PresentationQosPolicyMarshaler(PresentationQosPolicy from)
            : this()
        {
            CopyIn(ref from, gapiPtr, 0);
            cleanupRequired = true;
        }

        public PresentationQosPolicyMarshaler()
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.Alloc(Size);
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(gapiPtr, 0);
            }

            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
        }

        internal static void CopyIn(ref PresentationQosPolicy from, IntPtr to, int offset)
        {
            BaseMarshaler.Write(to, offset + offset_access_scope, (int)from.AccessScope);
            BaseMarshaler.Write(to, offset + offset_coherent_access, from.CoherentAccess);
            BaseMarshaler.Write(to, offset + offset_ordered_access, from.OrderedAccess);
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
            // Currently nothing to cleanup.
        }

        internal void CopyOut(out PresentationQosPolicy to)
        {
            CopyOut(gapiPtr, out to, 0);
        }

        internal static void CopyOut(IntPtr from, out PresentationQosPolicy to, int offset)
        {
            to.AccessScope = (PresentationQosPolicyAccessScopeKind)BaseMarshaler.ReadInt32(from, offset + offset_access_scope);
            to.CoherentAccess = BaseMarshaler.ReadBoolean(from, offset + offset_coherent_access);
            to.OrderedAccess = BaseMarshaler.ReadBoolean(from, offset + offset_ordered_access);
        }
    }

    internal class PartitionQosPolicyMarshaler : IMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_partitionQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_name = (int)Marshal.OffsetOf(type, "name");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public PartitionQosPolicyMarshaler(PartitionQosPolicy from)
            : this()
        {
            CopyIn(ref from, gapiPtr, 0);
            cleanupRequired = true;
        }

        public PartitionQosPolicyMarshaler()
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.Alloc(Size);
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(gapiPtr, 0);
            }

            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
        }

        internal static void CopyIn(ref PartitionQosPolicy from, IntPtr to, int offset)
        {
            SequenceStringMarshaler.CopyIn(from.Name, to, offset + offset_name);
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
            SequenceStringMarshaler.CleanupIn(nativePtr, offset + offset_name);
        }

        internal void CopyOut(out PartitionQosPolicy to)
        {
            CopyOut(gapiPtr, out to, 0);
        }

        internal static void CopyOut(IntPtr from, out PartitionQosPolicy to, int offset)
        {
            SequenceStringMarshaler.CopyOut(from, out to.Name, offset + offset_name);
        }
    }

    internal class GroupDataQosPolicyMarshaler : IMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_groupDataQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_value = (int)Marshal.OffsetOf(type, "value");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public GroupDataQosPolicyMarshaler(GroupDataQosPolicy from)
            : this()
        {
            CopyIn(ref from, gapiPtr, 0);
            cleanupRequired = true;
        }

        public GroupDataQosPolicyMarshaler()
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.Alloc(Size);
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(gapiPtr, 0);
            }

            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
        }

        internal static void CopyIn(ref GroupDataQosPolicy from, IntPtr to, int offset)
        {
            SequenceOctetMarshaler.CopyIn(from.Value, to, offset + offset_value);
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
            SequenceOctetMarshaler.CleanupIn(nativePtr, offset + offset_value);
        }

        internal void CopyOut(out GroupDataQosPolicy to)
        {
            CopyOut(gapiPtr, out to, 0);
        }

        internal static void CopyOut(IntPtr from, out GroupDataQosPolicy to, int offset)
        {
            SequenceOctetMarshaler.CopyOut(from, out to.Value, offset + offset_value);
        }
    }

    internal class TopicDataQosPolicyMarshaler : IMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_topicDataQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_value = (int)Marshal.OffsetOf(type, "value");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public TopicDataQosPolicyMarshaler(TopicDataQosPolicy from)
            : this()
        {
            CopyIn(ref from, gapiPtr, 0);
            cleanupRequired = true;
        }

        public TopicDataQosPolicyMarshaler()
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.Alloc(Size);
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(gapiPtr, 0);
            }

            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
        }

        internal static void CopyIn(ref TopicDataQosPolicy from, IntPtr to, int offset)
        {
            SequenceOctetMarshaler.CopyIn(from.Value, to, offset + offset_value);
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
            SequenceOctetMarshaler.CleanupIn(nativePtr, offset + offset_value);
        }

        internal void CopyOut(out TopicDataQosPolicy to)
        {
            CopyOut(gapiPtr, out to, 0);
        }

        internal static void CopyOut(IntPtr from, out TopicDataQosPolicy to, int offset)
        {
            SequenceOctetMarshaler.CopyOut(from, out to.Value, offset + offset_value);
        }
    }

    internal class DurabilityQosPolicyMarshaler : IMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_durabilityQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_kind = (int)Marshal.OffsetOf(type, "kind");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public DurabilityQosPolicyMarshaler(DurabilityQosPolicy from)
            : this()
        {
            CopyIn(ref from, gapiPtr, 0);
            cleanupRequired = true;
        }

        public DurabilityQosPolicyMarshaler()
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.Alloc(Size);
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(gapiPtr, 0);
            }

            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
        }

        internal static void CopyIn(ref DurabilityQosPolicy from, IntPtr to, int offset)
        {
            BaseMarshaler.Write(to, offset + offset_kind, (int)from.Kind);
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
        }

        internal void CopyOut(out DurabilityQosPolicy to)
        {
            CopyOut(gapiPtr, out to, 0);
        }

        internal static void CopyOut(IntPtr from, out DurabilityQosPolicy to, int offset)
        {
            // Get scheduling_class field
            to.Kind = (DurabilityQosPolicyKind)
                BaseMarshaler.ReadInt32(from, offset + offset_kind);
        }
    }

    internal class DurabilityServiceQosPolicyMarshaler : IMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_durabilityServiceQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_service_cleanup_delay = (int)Marshal.OffsetOf(type, "service_cleanup_delay");
        private static readonly int offset_history_kind = (int)Marshal.OffsetOf(type, "history_kind");
        private static readonly int offset_history_depth = (int)Marshal.OffsetOf(type, "history_depth");
        private static readonly int offset_max_samples = (int)Marshal.OffsetOf(type, "max_samples");
        private static readonly int offset_max_instances = (int)Marshal.OffsetOf(type, "max_instances");
        private static readonly int offset_max_samples_per_instance = (int)Marshal.OffsetOf(type, "max_samples_per_instance");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public DurabilityServiceQosPolicyMarshaler(DurabilityServiceQosPolicy from)
            : this()
        {
            CopyIn(ref from, gapiPtr, 0);
            cleanupRequired = true;
        }

        public DurabilityServiceQosPolicyMarshaler()
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.Alloc(Size);
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(gapiPtr, 0);
            }

            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
        }

        internal static void CopyIn(ref DurabilityServiceQosPolicy from, IntPtr to, int offset)
        {
            BaseMarshaler.Write(to, offset + offset_service_cleanup_delay, from.ServiceCleanupDelay);
            BaseMarshaler.Write(to, offset + offset_history_kind, (int)from.HistoryKind);
            BaseMarshaler.Write(to, offset + offset_history_depth, from.HistoryDepth);
            BaseMarshaler.Write(to, offset + offset_max_samples, from.MaxSamples);
            BaseMarshaler.Write(to, offset + offset_max_instances, from.MaxInstances);
            BaseMarshaler.Write(to, offset + offset_max_samples_per_instance, from.MaxSamplesPerInstance);
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
        }

        internal void CopyOut(out DurabilityServiceQosPolicy to)
        {
            CopyOut(gapiPtr, out to, 0);
        }

        internal static void CopyOut(IntPtr from, out DurabilityServiceQosPolicy to, int offset)
        {
            to.ServiceCleanupDelay = BaseMarshaler.ReadDuration(from, offset + offset_service_cleanup_delay);

            to.HistoryKind = (HistoryQosPolicyKind)
                BaseMarshaler.ReadInt32(from, offset + offset_history_kind);

            to.HistoryDepth = BaseMarshaler.ReadInt32(from, offset + offset_history_depth);

            to.MaxSamples = BaseMarshaler.ReadInt32(from, offset + offset_max_samples);

            to.MaxInstances = BaseMarshaler.ReadInt32(from, offset + offset_max_instances);

            to.MaxSamplesPerInstance = BaseMarshaler.ReadInt32(from, offset + offset_max_samples_per_instance);
        }
    }

    internal class DeadlineQosPolicyMarshaler : IMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_deadlineQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_period = (int)Marshal.OffsetOf(type, "period");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public DeadlineQosPolicyMarshaler(DeadlineQosPolicy from)
            : this()
        {
            CopyIn(ref from, gapiPtr, 0);
            cleanupRequired = true;
        }

        public DeadlineQosPolicyMarshaler()
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.Alloc(Size);
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(gapiPtr, 0);
            }

            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
        }

        internal static void CopyIn(ref DeadlineQosPolicy from, IntPtr to, int offset)
        {
            BaseMarshaler.Write(to, offset + offset_period, from.Period);
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
        }

        internal void CopyOut(out DeadlineQosPolicy to)
        {
            CopyOut(gapiPtr, out to, 0);
        }

        internal static void CopyOut(IntPtr from, out DeadlineQosPolicy to, int offset)
        {
            to.Period = BaseMarshaler.ReadDuration(from, offset + offset_period);
        }
    }

    internal class LatencyBudgetQosPolicyMarshaler : IMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_latencyBudgetQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_duration = (int)Marshal.OffsetOf(type, "duration");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public LatencyBudgetQosPolicyMarshaler(LatencyBudgetQosPolicy from)
            : this()
        {
            CopyIn(ref from, gapiPtr, 0);
            cleanupRequired = true;
        }

        public LatencyBudgetQosPolicyMarshaler()
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.Alloc(Size);
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(gapiPtr, 0);
            }

            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
        }

        internal static void CopyIn(ref LatencyBudgetQosPolicy from, IntPtr to, int offset)
        {
            BaseMarshaler.Write(to, offset + offset_duration, from.Duration);
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
        }

        internal void CopyOut(out LatencyBudgetQosPolicy to)
        {
            CopyOut(gapiPtr, out to, 0);
        }

        internal static void CopyOut(IntPtr from, out LatencyBudgetQosPolicy to, int offset)
        {
            to.Duration = BaseMarshaler.ReadDuration(from, offset + offset_duration);
        }
    }

    internal class LivelinessQosPolicyMarshaler : IMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_livelinessQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_kind = (int)Marshal.OffsetOf(type, "kind");
        private static readonly int offset_lease_duration = (int)Marshal.OffsetOf(type, "lease_duration");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public LivelinessQosPolicyMarshaler(LivelinessQosPolicy from)
            : this()
        {
            CopyIn(ref from, gapiPtr, 0);
            cleanupRequired = true;
        }

        public LivelinessQosPolicyMarshaler()
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.Alloc(Size);
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(gapiPtr, 0);
            }

            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
        }

        internal static void CopyIn(ref LivelinessQosPolicy from, IntPtr to, int offset)
        {
            BaseMarshaler.Write(to, offset + offset_kind, (int)from.Kind);
            BaseMarshaler.Write(to, offset + offset_lease_duration, from.LeaseDuration);

        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
        }

        internal void CopyOut(out LivelinessQosPolicy to)
        {
            CopyOut(gapiPtr, out to, 0);
        }

        internal static void CopyOut(IntPtr from, out LivelinessQosPolicy to, int offset)
        {
            to.Kind = (LivelinessQosPolicyKind)
                BaseMarshaler.ReadInt32(from, offset + offset_kind);

            to.LeaseDuration = BaseMarshaler.ReadDuration(from, offset + offset_lease_duration);
        }
    }

    internal class ReliabilityQosPolicyMarshaler : IMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_reliabilityQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_kind = (int)Marshal.OffsetOf(type, "kind");
        private static readonly int offset_max_blocking_time = (int)Marshal.OffsetOf(type, "max_blocking_time");
        private static readonly int offset_synchronous = (int)Marshal.OffsetOf(type, "synchronous");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public ReliabilityQosPolicyMarshaler(ReliabilityQosPolicy from)
            : this()
        {
            CopyIn(ref from, gapiPtr, 0);
            cleanupRequired = true;
        }

        public ReliabilityQosPolicyMarshaler()
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.Alloc(Size);
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(gapiPtr, 0);
            }

            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
        }

        internal static void CopyIn(ref ReliabilityQosPolicy from, IntPtr to, int offset)
        {
            BaseMarshaler.Write(to, offset + offset_kind, (int)from.Kind);
            BaseMarshaler.Write(to, offset + offset_max_blocking_time, from.MaxBlockingTime);
            BaseMarshaler.Write(to, offset + offset_synchronous, from.synchronous);
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
        }

        internal void CopyOut(out ReliabilityQosPolicy to)
        {
            CopyOut(gapiPtr, out to, 0);
        }

        internal static void CopyOut(IntPtr from, out ReliabilityQosPolicy to, int offset)
        {
            to.Kind = (ReliabilityQosPolicyKind)
                    BaseMarshaler.ReadInt32(from, offset + offset_kind);
            to.MaxBlockingTime = BaseMarshaler.ReadDuration(from, offset + offset_max_blocking_time);
            to.synchronous = BaseMarshaler.ReadBoolean(from, offset + offset_synchronous);
        }
    }

    internal class DestinationOrderQosPolicyMarshaler : IMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_destinationOrderQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_kind = (int)Marshal.OffsetOf(type, "kind");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public DestinationOrderQosPolicyMarshaler(DestinationOrderQosPolicy from)
            : this()
        {
            CopyIn(ref from, gapiPtr, 0);
            cleanupRequired = true;
        }

        public DestinationOrderQosPolicyMarshaler()
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.Alloc(Size);
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(gapiPtr, 0);
            }

            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
        }

        internal static void CopyIn(ref DestinationOrderQosPolicy from, IntPtr to, int offset)
        {
            BaseMarshaler.Write(to, offset + offset_kind, (int)from.Kind);
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
        }

        internal void CopyOut(out DestinationOrderQosPolicy to)
        {
            CopyOut(gapiPtr, out to, 0);
        }

        internal static void CopyOut(IntPtr from, out DestinationOrderQosPolicy to, int offset)
        {
            to.Kind = (DestinationOrderQosPolicyKind)
                BaseMarshaler.ReadInt32(from, offset + offset_kind);
        }
    }

    internal class HistoryQosPolicyMarshaler : IMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_historyQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_kind = (int)Marshal.OffsetOf(type, "kind");
        private static readonly int offset_depth = (int)Marshal.OffsetOf(type, "depth");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public HistoryQosPolicyMarshaler(HistoryQosPolicy from)
            : this()
        {
            CopyIn(ref from, gapiPtr, 0);
            cleanupRequired = true;
        }

        public HistoryQosPolicyMarshaler()
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.Alloc(Size);
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(gapiPtr, 0);
            }

            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
        }

        internal static void CopyIn(ref HistoryQosPolicy from, IntPtr to, int offset)
        {
            BaseMarshaler.Write(to, offset + offset_kind, (int)from.Kind);
            BaseMarshaler.Write(to, offset + offset_depth, from.Depth);
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
        }

        internal void CopyOut(out HistoryQosPolicy to)
        {
            CopyOut(gapiPtr, out to, 0);
        }

        internal static void CopyOut(IntPtr from, out HistoryQosPolicy to, int offset)
        {
            to.Kind = (HistoryQosPolicyKind)
                BaseMarshaler.ReadInt32(from, offset + offset_kind);

            to.Depth = BaseMarshaler.ReadInt32(from, offset + offset_depth);
        }
    }

    internal class ResourceLimitsQosPolicyMarshaler : IMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_resourceLimitsQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_max_samples = (int)Marshal.OffsetOf(type, "max_samples");
        private static readonly int offset_max_instances = (int)Marshal.OffsetOf(type, "max_instances");
        private static readonly int offset_max_samples_per_instance = (int)Marshal.OffsetOf(type, "max_samples_per_instance");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public ResourceLimitsQosPolicyMarshaler(ResourceLimitsQosPolicy from)
            : this()
        {
            CopyIn(ref from, gapiPtr, 0);
            cleanupRequired = true;
        }

        public ResourceLimitsQosPolicyMarshaler()
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.Alloc(Size);
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(gapiPtr, 0);
            }

            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
        }

        internal static void CopyIn(ref ResourceLimitsQosPolicy from, IntPtr to, int offset)
        {
            BaseMarshaler.Write(to, offset + offset_max_samples, from.MaxSamples);
            BaseMarshaler.Write(to, offset + offset_max_instances, from.MaxInstances);
            BaseMarshaler.Write(to, offset + offset_max_samples_per_instance, from.MaxSamplesPerInstance);
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
        }

        internal void CopyOut(out ResourceLimitsQosPolicy to)
        {
            CopyOut(gapiPtr, out to, 0);
        }

        internal static void CopyOut(IntPtr from, out ResourceLimitsQosPolicy to, int offset)
        {
            to.MaxSamples = BaseMarshaler.ReadInt32(from, offset + offset_max_samples);

            to.MaxInstances = BaseMarshaler.ReadInt32(from, offset + offset_max_instances);

            to.MaxSamplesPerInstance = BaseMarshaler.ReadInt32(from, offset + offset_max_samples_per_instance);
        }
    }

    internal class TransportPriorityQosPolicyMarshaler : IMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_transportPriorityQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_value = (int)Marshal.OffsetOf(type, "value");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public TransportPriorityQosPolicyMarshaler(TransportPriorityQosPolicy from)
            : this()
        {
            CopyIn(ref from, gapiPtr, 0);
            cleanupRequired = true;
        }

        public TransportPriorityQosPolicyMarshaler()
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.Alloc(Size);
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(gapiPtr, 0);
            }

            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
        }

        internal static void CopyIn(ref TransportPriorityQosPolicy from, IntPtr to, int offset)
        {
            BaseMarshaler.Write(to, offset + offset_value, from.Value);
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
        }

        internal void CopyOut(out TransportPriorityQosPolicy to)
        {
            CopyOut(gapiPtr, out to, 0);
        }

        internal static void CopyOut(IntPtr from, out TransportPriorityQosPolicy to, int offset)
        {
            to.Value = BaseMarshaler.ReadInt32(from, offset + offset_value);
        }
    }

    internal class LifespanQosPolicyMarshaler : IMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_lifespanQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_duration = (int)Marshal.OffsetOf(type, "duration");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public LifespanQosPolicyMarshaler(LifespanQosPolicy from)
            : this()
        {
            CopyIn(ref from, gapiPtr, 0);
            cleanupRequired = true;
        }

        public LifespanQosPolicyMarshaler()
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.Alloc(Size);
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(gapiPtr, 0);
            }

            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
        }

        internal static void CopyIn(ref LifespanQosPolicy from, IntPtr to, int offset)
        {
            BaseMarshaler.Write(to, offset + offset_duration, from.Duration);
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
        }

        internal void CopyOut(out LifespanQosPolicy to)
        {
            CopyOut(gapiPtr, out to, 0);
        }

        internal static void CopyOut(IntPtr from, out LifespanQosPolicy to, int offset)
        {
            to.Duration = BaseMarshaler.ReadDuration(from, offset + offset_duration);
        }
    }

    internal class OwnershipQosPolicyMarshaler : IMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_ownershipQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_kind = (int)Marshal.OffsetOf(type, "kind");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public OwnershipQosPolicyMarshaler(OwnershipQosPolicy from)
            : this()
        {
            CopyIn(ref from, gapiPtr, 0);
            cleanupRequired = true;
        }

        public OwnershipQosPolicyMarshaler()
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.Alloc(Size);
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(gapiPtr, 0);
            }

            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
        }

        internal static void CopyIn(ref OwnershipQosPolicy from, IntPtr to, int offset)
        {
            BaseMarshaler.Write(to, offset + offset_kind, (int)from.Kind);
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
        }

        internal void CopyOut(out OwnershipQosPolicy to)
        {
            CopyOut(gapiPtr, out to, 0);
        }

        internal static void CopyOut(IntPtr from, out OwnershipQosPolicy to, int offset)
        {
            // Get scheduling_class field
            to.Kind = (OwnershipQosPolicyKind)
                BaseMarshaler.ReadInt32(from, offset + offset_kind);
        }
    }

    internal class TimeBasedFilterQosPolicyMarshaler : IMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_timeBasedFilterQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_minimum_separation = (int)Marshal.OffsetOf(type, "minimum_separation");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public TimeBasedFilterQosPolicyMarshaler(TimeBasedFilterQosPolicy from)
            : this()
        {
            CopyIn(ref from, gapiPtr, 0);
            cleanupRequired = true;
        }

        public TimeBasedFilterQosPolicyMarshaler()
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.Alloc(Size);
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(gapiPtr, 0);
            }

            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
        }

        internal static void CopyIn(ref TimeBasedFilterQosPolicy from, IntPtr to, int offset)
        {
            BaseMarshaler.Write(to, offset + offset_minimum_separation, from.MinimumSeparation);
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
        }

        internal void CopyOut(out TimeBasedFilterQosPolicy to)
        {
            CopyOut(gapiPtr, out to, 0);
        }

        internal static void CopyOut(IntPtr from, out TimeBasedFilterQosPolicy to, int offset)
        {
            to.MinimumSeparation = BaseMarshaler.ReadDuration(from, offset + offset_minimum_separation);
        }
    }

    internal class ReaderDataLifecycleQosPolicyMarshaler : IMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_readerDataLifecycleQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_autopurge_nowriter_samples_delay = (int)Marshal.OffsetOf(type, "autopurge_nowriter_samples_delay");
        private static readonly int offset_autopurge_disposed_samples_delay = (int)Marshal.OffsetOf(type, "autopurge_disposed_samples_delay");
        private static readonly int offset_enable_invalid_samples = (int)Marshal.OffsetOf(type, "enable_invalid_samples");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public ReaderDataLifecycleQosPolicyMarshaler(ReaderDataLifecycleQosPolicy from)
            : this()
        {
            CopyIn(ref from, gapiPtr, 0);
            cleanupRequired = true;
        }

        public ReaderDataLifecycleQosPolicyMarshaler()
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.Alloc(Size);
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(gapiPtr, 0);
            }

            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
        }

        internal static void CopyIn(ref ReaderDataLifecycleQosPolicy from, IntPtr to, int offset)
        {
            BaseMarshaler.Write(to, offset + offset_autopurge_nowriter_samples_delay, from.AutoPurgeNoWriterSamplesDelay);
            BaseMarshaler.Write(to, offset + offset_autopurge_disposed_samples_delay, from.AutoPurgeDisposedSamplesDelay);
            BaseMarshaler.Write(to, offset + offset_enable_invalid_samples, from.EnableInvalidSamples);
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
        }

        internal void CopyOut(out ReaderDataLifecycleQosPolicy to)
        {
            CopyOut(gapiPtr, out to, 0);
        }

        internal static void CopyOut(IntPtr from, out ReaderDataLifecycleQosPolicy to, int offset)
        {
            to.AutoPurgeNoWriterSamplesDelay = BaseMarshaler.ReadDuration(from, offset + offset_autopurge_nowriter_samples_delay);
            to.AutoPurgeDisposedSamplesDelay = BaseMarshaler.ReadDuration(from, offset + offset_autopurge_disposed_samples_delay);
            to.EnableInvalidSamples = BaseMarshaler.ReadBoolean(from, offset + offset_enable_invalid_samples);
        }
    }

    internal class OwnershipStrengthQosPolicyMarshaler : IMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_ownershipStrengthQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_value = (int)Marshal.OffsetOf(type, "value");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public OwnershipStrengthQosPolicyMarshaler(OwnershipStrengthQosPolicy from)
            : this()
        {
            CopyIn(ref from, gapiPtr, 0);
            cleanupRequired = true;
        }

        public OwnershipStrengthQosPolicyMarshaler()
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.Alloc(Size);
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(gapiPtr, 0);
            }

            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
        }

        internal static void CopyIn(ref OwnershipStrengthQosPolicy from, IntPtr to, int offset)
        {
            BaseMarshaler.Write(to, offset + offset_value, from.Value);
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
        }

        internal void CopyOut(out OwnershipStrengthQosPolicy to)
        {
            CopyOut(gapiPtr, out to, 0);
        }

        internal static void CopyOut(IntPtr from, out OwnershipStrengthQosPolicy to, int offset)
        {
            to.Value = BaseMarshaler.ReadInt32(from, offset + offset_value);
        }
    }

    internal class WriterDataLifecycleQosPolicyMarshaler : IMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_writerDataLifecycleQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_autodispose_unregistered_instances =
            (int)Marshal.OffsetOf(type, "autodispose_unregistered_instances");
        private static readonly int offset_autopurge_suspended_samples_delay =
            (int)Marshal.OffsetOf(type, "autopurge_suspended_samples_delay");
        private static readonly int offset_autounregister_instance_delay =
            (int)Marshal.OffsetOf(type, "autounregister_instance_delay");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public WriterDataLifecycleQosPolicyMarshaler(WriterDataLifecycleQosPolicy from)
            : this()
        {
            CopyIn(ref from, gapiPtr, 0);
            cleanupRequired = true;
        }

        public WriterDataLifecycleQosPolicyMarshaler()
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.Alloc(Size);
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(gapiPtr, 0);
            }

            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
        }

        internal static void CopyIn(ref WriterDataLifecycleQosPolicy from, IntPtr to, int offset)
        {
            BaseMarshaler.Write(to, offset + offset_autodispose_unregistered_instances,
                from.AutoDisposeUnregisteredInstances);
            BaseMarshaler.Write(to, offset + offset_autopurge_suspended_samples_delay, from.AutopurgeSuspendedSamplesDelay);
            BaseMarshaler.Write(to, offset + offset_autounregister_instance_delay, from.AutounregisterInstanceDelay);
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
        }

        internal void CopyOut(out WriterDataLifecycleQosPolicy to)
        {
            CopyOut(gapiPtr, out to, 0);
        }

        internal static void CopyOut(IntPtr from, out WriterDataLifecycleQosPolicy to, int offset)
        {
            to.AutoDisposeUnregisteredInstances =
                BaseMarshaler.ReadBoolean(from, offset + offset_autodispose_unregistered_instances);
            to.AutopurgeSuspendedSamplesDelay = BaseMarshaler.ReadDuration(from, offset + offset_autopurge_suspended_samples_delay);
            to.AutounregisterInstanceDelay = BaseMarshaler.ReadDuration(from, offset + offset_autounregister_instance_delay);
        }
    }

    internal class SubscriptionKeyQosPolicyMarshaler : IMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_subscriptionKeyQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_use_key_list = (int)Marshal.OffsetOf(type, "use_key_list");
        private static readonly int offset_key_list = (int)Marshal.OffsetOf(type, "key_list");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public SubscriptionKeyQosPolicyMarshaler(SubscriptionKeyQosPolicy from)
            : this()
        {
            CopyIn(ref from, gapiPtr, 0);
            cleanupRequired = true;
        }

        public SubscriptionKeyQosPolicyMarshaler()
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.Alloc(Size);
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(gapiPtr, 0);
            }

            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
        }

        internal static void CopyIn(ref SubscriptionKeyQosPolicy from, IntPtr to, int offset)
        {
            BaseMarshaler.Write(to, offset + offset_use_key_list, from.UseKeyList);
            SequenceStringMarshaler.CopyIn(from.KeyList, to, offset + offset_key_list);
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
            SequenceStringMarshaler.CleanupIn(nativePtr, offset + offset_key_list);
        }

        internal void CopyOut(out SubscriptionKeyQosPolicy to)
        {
            CopyOut(gapiPtr, out to, 0);
        }

        internal static void CopyOut(IntPtr from, out SubscriptionKeyQosPolicy to, int offset)
        {
            to.UseKeyList = BaseMarshaler.ReadBoolean(from, offset + offset_use_key_list);
            SequenceStringMarshaler.CopyOut(from, out to.KeyList, offset + offset_key_list);
        }
    }

    internal class ReaderLifespanQosPolicyMarshaler : IMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_readerLifespanQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_use_lifespan = (int)Marshal.OffsetOf(type, "use_lifespan");
        private static readonly int offset_duration = (int)Marshal.OffsetOf(type, "duration");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public ReaderLifespanQosPolicyMarshaler(ReaderLifespanQosPolicy from)
            : this()
        {
            CopyIn(ref from, gapiPtr, 0);
            cleanupRequired = true;
        }

        public ReaderLifespanQosPolicyMarshaler()
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.Alloc(Size);
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(gapiPtr, 0);
            }

            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
        }

        internal static void CopyIn(ref ReaderLifespanQosPolicy from, IntPtr to, int offset)
        {
            BaseMarshaler.Write(to, offset + offset_use_lifespan, from.UseLifespan);
            BaseMarshaler.Write(to, offset + offset_duration, from.Duration);
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
        }

        internal void CopyOut(out ReaderLifespanQosPolicy to)
        {
            CopyOut(gapiPtr, out to, 0);
        }

        internal static void CopyOut(IntPtr from, out ReaderLifespanQosPolicy to, int offset)
        {
            to.UseLifespan = BaseMarshaler.ReadBoolean(from, offset + offset_use_lifespan);
            to.Duration = BaseMarshaler.ReadDuration(from, offset + offset_duration);
        }
    }
}
