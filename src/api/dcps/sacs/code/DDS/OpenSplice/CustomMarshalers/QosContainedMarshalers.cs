// The OpenSplice DDS Community Edition project.
//
// Copyright (C) 2006 to 2011 PrismTech Limited and its licensees.
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
using DDS.OpenSplice;

namespace DDS.OpenSplice.CustomMarshalers
{
    internal class BuiltinTopicKeyMarshaler : GapiMarshaler
    {
        private static readonly Type type = typeof(DDS.OpenSplice.Gapi.BuiltinTopicKey);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_SystemId = (int)Marshal.OffsetOf(type, "SystemId");
        private static readonly int offset_LocalId = (int)Marshal.OffsetOf(type, "LocalId");
        private static readonly int offset_Serial = (int)Marshal.OffsetOf(type, "Serial");

        public BuiltinTopicKeyMarshaler() :
                base(Gapi.GenericAllocRelease.Alloc(Size))
        { }

        public override void Dispose()
        {
            if (cleanupRequired) 
            {
                CleanupIn(GapiPtr, 0);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(GapiPtr);
        }

        internal DDS.ReturnCode CopyIn(int[] from)
        {
            cleanupRequired = true;
            return CopyIn(from, GapiPtr, 0);
        }

        internal static DDS.ReturnCode CopyIn(int[] from, IntPtr to, int offset)
        {
            DDS.ReturnCode result;
            
            if (from != null && from.Length == 3)
            {
                BaseMarshaler.Write(to, offset + offset_SystemId, from[0]);
                BaseMarshaler.Write(to, offset + offset_LocalId, from[1]);
                BaseMarshaler.Write(to, offset + offset_Serial, from[2]);
                result = DDS.ReturnCode.Ok;
            }
            else
            {
                result = DDS.ReturnCode.BadParameter;
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.CustomMarshalers.BuiltinTopicKeyMarshaler.CopyIn",
                        "DDS/OpenSplice/CustomMarshalers/QosContainedMarshalers.cs",
                        DDS.ErrorCode.InvalidValue,
                        "BuiltinTopicKey attribute must always be of type int[3].");
            }
            return result; 
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
        }

        internal void CopyOut(ref int[] to)
        {
            CopyOut(GapiPtr, ref to, 0);
        }

        internal static void CopyOut(IntPtr from, ref int[] to, int offset)
        {
            if (to == null || to.Length != 3) to = new int[3];
            to[0] = BaseMarshaler.ReadInt32(from, offset + offset_SystemId);
            to[1] = BaseMarshaler.ReadInt32(from, offset + offset_LocalId);
            to[2] = BaseMarshaler.ReadInt32(from, offset + offset_Serial);
        }
    }

    internal class UserDataQosPolicyMarshaler : GapiMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_userDataQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_value = (int)Marshal.OffsetOf(type, "value");

        public UserDataQosPolicyMarshaler() :
                base(Gapi.GenericAllocRelease.Alloc(Size))
        { }

        public override void Dispose()
        {
            if (cleanupRequired) 
            {
                CleanupIn(GapiPtr, 0);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(GapiPtr);
        }

        internal DDS.ReturnCode CopyIn(UserDataQosPolicy from)
        {
            cleanupRequired = true;
            return CopyIn(from, GapiPtr, 0);
        }

        internal static DDS.ReturnCode CopyIn(UserDataQosPolicy from, IntPtr to, int offset)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                SequenceOctetMarshaler.CopyIn(from.Value, to, offset + offset_value);
            } else {
                result = DDS.ReturnCode.BadParameter;
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.CustomMarshalers.UserDataQosPolicyMarshaler.CopyIn",
                        "DDS/OpenSplice/CustomMarshalers/QosContainedMarshalers.cs",
                        DDS.ErrorCode.InvalidValue,
                        "UserDataQosPolicy attribute may not be a null pointer.");
            }
            return result; 
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
            SequenceOctetMarshaler.CleanupIn(nativePtr, offset + offset_value);
        }

        internal void CopyOut(ref UserDataQosPolicy to)
        {
            CopyOut(GapiPtr, ref to, 0);
        }

        internal static void CopyOut(IntPtr from, ref UserDataQosPolicy to, int offset)
        {
            if (to == null) to = new UserDataQosPolicy();
            SequenceOctetMarshaler.CopyOut(from, ref to.Value, offset + offset_value);
        }
    }

    internal class EntityFactoryQosPolicyMarshaler : GapiMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_entityFactoryQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_autoenable_created_entities = (int)Marshal.OffsetOf(type, "autoenable_created_entities");

        public EntityFactoryQosPolicyMarshaler() :
                base(Gapi.GenericAllocRelease.Alloc(Size))
        { }

        public override void Dispose()
        {
            if (cleanupRequired) 
            {
                CleanupIn(GapiPtr, 0);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(GapiPtr);
        }

        internal DDS.ReturnCode CopyIn(EntityFactoryQosPolicy from)
        {
            cleanupRequired = true;
            return CopyIn(from, GapiPtr, 0);
        }

        internal static DDS.ReturnCode CopyIn(EntityFactoryQosPolicy from, IntPtr to, int offset)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                // Set autoenable_created_entities field
                BaseMarshaler.Write(to, offset + offset_autoenable_created_entities, from.AutoenableCreatedEntities);
            } else {
                result = DDS.ReturnCode.BadParameter;
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.CustomMarshalers.EntityFactoryQosPolicyMarshaler.CopyIn",
                        "DDS/OpenSplice/CustomMarshalers/QosContainedMarshalers.cs",
                        DDS.ErrorCode.InvalidValue,
                        "EntityFactoryQosPolicy attribute may not be a null pointer.");
            }
            return result; 
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
            // Currently nothing to cleanup.
        }

        internal void CopyOut(ref EntityFactoryQosPolicy to)
        {
            CopyOut(GapiPtr, ref to, 0);
        }

        internal static void CopyOut(IntPtr from, ref EntityFactoryQosPolicy to, int offset)
        {
            if (to == null) to = new EntityFactoryQosPolicy();

            // Set autoenable_created_entities field
            to.AutoenableCreatedEntities = BaseMarshaler.ReadBoolean(from, offset + offset_autoenable_created_entities);
        }
    }

    internal class ShareQosPolicyMarshaler : GapiMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_shareQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_name = (int)Marshal.OffsetOf(type, "name");
        private static readonly int offset_enable = (int)Marshal.OffsetOf(type, "enable");

        public ShareQosPolicyMarshaler() :
                base(Gapi.GenericAllocRelease.Alloc(Size))
        { }

        public override void Dispose()
        {
            if (cleanupRequired) 
            {
                CleanupIn(GapiPtr, 0);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(GapiPtr);
        }

        internal DDS.ReturnCode CopyIn(ShareQosPolicy from)
        {
            cleanupRequired = true;
            return CopyIn(from, GapiPtr, 0);
        }

        internal static DDS.ReturnCode CopyIn(ShareQosPolicy from, IntPtr to, int offset)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null && from.Name != null) {
                IntPtr stringPtr = OpenSplice.Gapi.GenericAllocRelease.string_dup(from.Name);
                BaseMarshaler.Write(to, offset + offset_name, stringPtr);
                BaseMarshaler.Write(to, offset + offset_enable, from.Enable);
            } else {
                result = DDS.ReturnCode.BadParameter;
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.CustomMarshalers.ShareQosPolicyMarshaler.CopyIn",
                        "DDS/OpenSplice/CustomMarshalers/QosContainedMarshalers.cs",
                        DDS.ErrorCode.InvalidValue,
                        "ShareQosPolicy attribute may not contain a null pointer.");
            }
            return result; 
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
            IntPtr stringPtr = BaseMarshaler.ReadIntPtr(nativePtr, offset + offset_name);
            OpenSplice.Gapi.GenericAllocRelease.Free(stringPtr);
            BaseMarshaler.Write(nativePtr, offset + offset_name, IntPtr.Zero);
        }

        internal void CopyOut(ref ShareQosPolicy to)
        {
            CopyOut(GapiPtr, ref to, 0);
        }

        internal static void CopyOut(IntPtr from, ref ShareQosPolicy to, int offset)
        {
            IntPtr ptr = new IntPtr((long)from + offset + offset_name);
            if (to == null) to = new ShareQosPolicy();
            to.Name = Marshal.PtrToStringAnsi(ptr);
            if (to.Name == null) to.Name = String.Empty;
            to.Enable = BaseMarshaler.ReadBoolean(from, offset + offset_enable);
        }
    }

    internal class SchedulingQosPolicyMarshaler : GapiMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_schedulingQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_scheduling_class = (int)Marshal.OffsetOf(type, "scheduling_class");
        private static readonly int offset_scheduling_priority_kind = (int)Marshal.OffsetOf(type, "scheduling_priority_kind");
        private static readonly int offset_scheduling_priority = (int)Marshal.OffsetOf(type, "scheduling_priority");

        public SchedulingQosPolicyMarshaler() :
                base(Gapi.GenericAllocRelease.Alloc(Size))
        { }

        public override void Dispose()
        {
            if (cleanupRequired) 
            {
                CleanupIn(GapiPtr, 0);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(GapiPtr);
        }

        internal DDS.ReturnCode CopyIn(SchedulingQosPolicy from)
        {
            cleanupRequired = true;
            return CopyIn(from, GapiPtr, 0);
        }

        internal static DDS.ReturnCode CopyIn(SchedulingQosPolicy from, IntPtr to, int offset)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                // Set scheduling_class field
                BaseMarshaler.Write(to, offset + offset_scheduling_class, (int)from.SchedulingClass.Kind);

                // Set scheduling_priority_kind field
                BaseMarshaler.Write(to, offset + offset_scheduling_priority_kind, (int)from.SchedulingPriorityKind.Kind);

                // Set scheduling_priority field
                BaseMarshaler.Write(to, offset + offset_scheduling_priority, from.SchedulingPriority);
            } else {
                result = DDS.ReturnCode.BadParameter;
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.CustomMarshalers.SchedulingQosPolicyMarshaler.CopyIn",
                        "DDS/OpenSplice/CustomMarshalers/QosContainedMarshalers.cs",
                        DDS.ErrorCode.InvalidValue,
                        "SchedulingQosPolicy attribute may not be a null pointer.");
            }
            return result; 
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
            // Currently nothing to cleanup.
        }

        internal void CopyOut(ref SchedulingQosPolicy to)
        {
            CopyOut(GapiPtr, ref to, 0);
        }

        internal static void CopyOut(IntPtr from, ref SchedulingQosPolicy to, int offset)
        {
        	if (to == null) to = new SchedulingQosPolicy();
        	
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

    internal class PresentationQosPolicyMarshaler : GapiMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_presentationQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_access_scope = (int)Marshal.OffsetOf(type, "access_scope");
        private static readonly int offset_coherent_access = (int)Marshal.OffsetOf(type, "coherent_access");
        private static readonly int offset_ordered_access = (int)Marshal.OffsetOf(type, "ordered_access");

        public PresentationQosPolicyMarshaler() :
                base(Gapi.GenericAllocRelease.Alloc(Size))
        { }

        public override void Dispose()
        {
            if (cleanupRequired) 
            {
                CleanupIn(GapiPtr, 0);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(GapiPtr);
        }

        internal DDS.ReturnCode CopyIn(PresentationQosPolicy from)
        {
            cleanupRequired = true;
            return CopyIn(from, GapiPtr, 0);
        }

        internal static DDS.ReturnCode CopyIn(PresentationQosPolicy from, IntPtr to, int offset)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                BaseMarshaler.Write(to, offset + offset_access_scope, (int)from.AccessScope);
                BaseMarshaler.Write(to, offset + offset_coherent_access, from.CoherentAccess);
                BaseMarshaler.Write(to, offset + offset_ordered_access, from.OrderedAccess);
            } else {
                result = DDS.ReturnCode.BadParameter;
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.CustomMarshalers.PresentationQosPolicyMarshaler.CopyIn",
                        "DDS/OpenSplice/CustomMarshalers/QosContainedMarshalers.cs",
                        DDS.ErrorCode.InvalidValue,
                        "PresentationQosPolicy attribute may not be a null pointer.");
            }
            return result; 
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
            // Currently nothing to cleanup.
        }

        internal void CopyOut(ref PresentationQosPolicy to)
        {
            CopyOut(GapiPtr, ref to, 0);
        }

        internal static void CopyOut(IntPtr from, ref PresentationQosPolicy to, int offset)
        {
        	if (to == null) to = new PresentationQosPolicy();
            to.AccessScope = (PresentationQosPolicyAccessScopeKind)BaseMarshaler.ReadInt32(from, offset + offset_access_scope);
            to.CoherentAccess = BaseMarshaler.ReadBoolean(from, offset + offset_coherent_access);
            to.OrderedAccess = BaseMarshaler.ReadBoolean(from, offset + offset_ordered_access);
        }
    }

    internal class PartitionQosPolicyMarshaler : GapiMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_partitionQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_name = (int)Marshal.OffsetOf(type, "name");

        public PartitionQosPolicyMarshaler() :
                base(Gapi.GenericAllocRelease.Alloc(Size))
        { }

        public override void Dispose()
        {
            if (cleanupRequired) 
            {
                CleanupIn(GapiPtr, 0);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(GapiPtr);
        }

        internal DDS.ReturnCode CopyIn(PartitionQosPolicy from)
        {
            cleanupRequired = true;
            return CopyIn(from, GapiPtr, 0);
        }

        internal static DDS.ReturnCode CopyIn(PartitionQosPolicy from, IntPtr to, int offset)
        {
            DDS.ReturnCode result;
            if (from != null) {
                result = SequenceStringMarshaler.CopyIn(from.Name, to, offset + offset_name);
            } else {
                result = DDS.ReturnCode.BadParameter;
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.CustomMarshalers.PartitionQosPolicyMarshaler.CopyIn",
                        "DDS/OpenSplice/CustomMarshalers/QosContainedMarshalers.cs",
                        DDS.ErrorCode.InvalidValue,
                        "PartitionQosPolicy attribute may not be a null pointer.");
            }
            return result; 
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
            SequenceStringMarshaler.CleanupIn(nativePtr, offset + offset_name);
        }

        internal void CopyOut(ref PartitionQosPolicy to)
        {
            CopyOut(GapiPtr, ref to, 0);
        }

        internal static void CopyOut(IntPtr from, ref PartitionQosPolicy to, int offset)
        {
        	if (to == null) to = new PartitionQosPolicy();
            SequenceStringMarshaler.CopyOut(from, ref to.Name, offset + offset_name);
        }
    }

    internal class GroupDataQosPolicyMarshaler : GapiMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_groupDataQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_value = (int)Marshal.OffsetOf(type, "value");

        public GroupDataQosPolicyMarshaler() :
                base(Gapi.GenericAllocRelease.Alloc(Size))
        { }

        public override void Dispose()
        {
            if (cleanupRequired) 
            {
                CleanupIn(GapiPtr, 0);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(GapiPtr);
        }

        internal DDS.ReturnCode CopyIn(GroupDataQosPolicy from)
        {
            cleanupRequired = true;
            return CopyIn(from, GapiPtr, 0);
        }

        internal static DDS.ReturnCode CopyIn(GroupDataQosPolicy from, IntPtr to, int offset)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                SequenceOctetMarshaler.CopyIn(from.Value, to, offset + offset_value);
            } else {
                result = DDS.ReturnCode.BadParameter;
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.CustomMarshalers.GroupDataQosPolicyMarshaler.CopyIn",
                        "DDS/OpenSplice/CustomMarshalers/QosContainedMarshalers.cs",
                        DDS.ErrorCode.InvalidValue,
                        "GroupDataQosPolicy attribute may not be a null pointer.");
            }
            return result; 
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
            SequenceOctetMarshaler.CleanupIn(nativePtr, offset + offset_value);
        }

        internal void CopyOut(ref GroupDataQosPolicy to)
        {
            CopyOut(GapiPtr, ref to, 0);
        }

        internal static void CopyOut(IntPtr from, ref GroupDataQosPolicy to, int offset)
        {
        	if (to == null) to = new GroupDataQosPolicy();
            SequenceOctetMarshaler.CopyOut(from, ref to.Value, offset + offset_value);
        }
    }

    internal class TopicDataQosPolicyMarshaler : GapiMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_topicDataQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_value = (int)Marshal.OffsetOf(type, "value");

        public TopicDataQosPolicyMarshaler() :
                base(Gapi.GenericAllocRelease.Alloc(Size))
        { }

        public override void Dispose()
        {
            if (cleanupRequired) 
            {
                CleanupIn(GapiPtr, 0);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(GapiPtr);
        }

        internal DDS.ReturnCode CopyIn(TopicDataQosPolicy from)
        {
            cleanupRequired = true;
            return CopyIn(from, GapiPtr, 0);
        }

        internal static DDS.ReturnCode CopyIn(TopicDataQosPolicy from, IntPtr to, int offset)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                SequenceOctetMarshaler.CopyIn(from.Value, to, offset + offset_value);
            } else {
                result = DDS.ReturnCode.BadParameter;
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.CustomMarshalers.TopicDataQosPolicyMarshaler.CopyIn",
                        "DDS/OpenSplice/CustomMarshalers/QosContainedMarshalers.cs",
                        DDS.ErrorCode.InvalidValue,
                        "TopicDataQosPolicy attribute may not be a null pointer.");
            }
            return result; 
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
            SequenceOctetMarshaler.CleanupIn(nativePtr, offset + offset_value);
        }

        internal void CopyOut(ref TopicDataQosPolicy to)
        {
            CopyOut(GapiPtr, ref to, 0);
        }

        internal static void CopyOut(IntPtr from, ref TopicDataQosPolicy to, int offset)
        {
        	if (to == null) to = new TopicDataQosPolicy();
            SequenceOctetMarshaler.CopyOut(from, ref to.Value, offset + offset_value);
        }
    }

    internal class DurabilityQosPolicyMarshaler : GapiMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_durabilityQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_kind = (int)Marshal.OffsetOf(type, "kind");

        public DurabilityQosPolicyMarshaler() :
                base(Gapi.GenericAllocRelease.Alloc(Size))
        { }

        public override void Dispose()
        {
            if (cleanupRequired) 
            {
                CleanupIn(GapiPtr, 0);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(GapiPtr);
        }

        internal DDS.ReturnCode CopyIn(DurabilityQosPolicy from)
        {
            cleanupRequired = true;
            return CopyIn(from, GapiPtr, 0);
        }

        internal static DDS.ReturnCode CopyIn(DurabilityQosPolicy from, IntPtr to, int offset)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                BaseMarshaler.Write(to, offset + offset_kind, (int)from.Kind);
            } else {
                result = DDS.ReturnCode.BadParameter;
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.CustomMarshalers.DurabilityQosPolicyMarshaler.CopyIn",
                        "DDS/OpenSplice/CustomMarshalers/QosContainedMarshalers.cs",
                        DDS.ErrorCode.InvalidValue,
                        "DurabilityQosPolicy attribute may not be a null pointer.");
            }
            return result; 
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
        }

        internal void CopyOut(ref DurabilityQosPolicy to)
        {
            CopyOut(GapiPtr, ref to, 0);
        }

        internal static void CopyOut(IntPtr from, ref DurabilityQosPolicy to, int offset)
        {
        	if (to == null) to = new DurabilityQosPolicy();

            // Get scheduling_class field
            to.Kind = (DurabilityQosPolicyKind)
                BaseMarshaler.ReadInt32(from, offset + offset_kind);
        }
    }

    internal class DurabilityServiceQosPolicyMarshaler : GapiMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_durabilityServiceQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_service_cleanup_delay = (int)Marshal.OffsetOf(type, "service_cleanup_delay");
        private static readonly int offset_history_kind = (int)Marshal.OffsetOf(type, "history_kind");
        private static readonly int offset_history_depth = (int)Marshal.OffsetOf(type, "history_depth");
        private static readonly int offset_max_samples = (int)Marshal.OffsetOf(type, "max_samples");
        private static readonly int offset_max_instances = (int)Marshal.OffsetOf(type, "max_instances");
        private static readonly int offset_max_samples_per_instance = (int)Marshal.OffsetOf(type, "max_samples_per_instance");

        public DurabilityServiceQosPolicyMarshaler() :
                base(Gapi.GenericAllocRelease.Alloc(Size))
        { }

        public override void Dispose()
        {
            if (cleanupRequired) 
            {
                CleanupIn(GapiPtr, 0);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(GapiPtr);
        }

        internal DDS.ReturnCode CopyIn(DurabilityServiceQosPolicy from)
        {
            cleanupRequired = true;
            return CopyIn(from, GapiPtr, 0);
        }

        internal static DDS.ReturnCode CopyIn(DurabilityServiceQosPolicy from, IntPtr to, int offset)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                BaseMarshaler.Write(to, offset + offset_service_cleanup_delay, from.ServiceCleanupDelay);
                BaseMarshaler.Write(to, offset + offset_history_kind, (int)from.HistoryKind);
                BaseMarshaler.Write(to, offset + offset_history_depth, from.HistoryDepth);
                BaseMarshaler.Write(to, offset + offset_max_samples, from.MaxSamples);
                BaseMarshaler.Write(to, offset + offset_max_instances, from.MaxInstances);
                BaseMarshaler.Write(to, offset + offset_max_samples_per_instance, from.MaxSamplesPerInstance);
            } else {
                result = DDS.ReturnCode.BadParameter;
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.CustomMarshalers.DurabilityServiceQosPolicyMarshaler.CopyIn",
                        "DDS/OpenSplice/CustomMarshalers/QosContainedMarshalers.cs",
                        DDS.ErrorCode.InvalidValue,
                        "DurabilityServiceQosPolicy attribute may not be a null pointer.");
            }
            return result; 
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
        }

        internal void CopyOut(ref DurabilityServiceQosPolicy to)
        {
            CopyOut(GapiPtr, ref to, 0);
        }

        internal static void CopyOut(IntPtr from, ref DurabilityServiceQosPolicy to, int offset)
        {
        	if (to == null) to = new DurabilityServiceQosPolicy();

            to.ServiceCleanupDelay = BaseMarshaler.ReadDuration(from, offset + offset_service_cleanup_delay);

            to.HistoryKind = (HistoryQosPolicyKind)
                BaseMarshaler.ReadInt32(from, offset + offset_history_kind);

            to.HistoryDepth = BaseMarshaler.ReadInt32(from, offset + offset_history_depth);

            to.MaxSamples = BaseMarshaler.ReadInt32(from, offset + offset_max_samples);

            to.MaxInstances = BaseMarshaler.ReadInt32(from, offset + offset_max_instances);

            to.MaxSamplesPerInstance = BaseMarshaler.ReadInt32(from, offset + offset_max_samples_per_instance);
        }
    }

    internal class DeadlineQosPolicyMarshaler : GapiMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_deadlineQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_period = (int)Marshal.OffsetOf(type, "period");

        public DeadlineQosPolicyMarshaler() :
                base(Gapi.GenericAllocRelease.Alloc(Size))
        { }

        public override void Dispose()
        {
            if (cleanupRequired) 
            {
                CleanupIn(GapiPtr, 0);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(GapiPtr);
        }

        internal DDS.ReturnCode CopyIn(DeadlineQosPolicy from)
        {
            cleanupRequired = true;
            return CopyIn(from, GapiPtr, 0);
        }

        internal static DDS.ReturnCode CopyIn(DeadlineQosPolicy from, IntPtr to, int offset)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                BaseMarshaler.Write(to, offset + offset_period, from.Period);
            } else {
                result = DDS.ReturnCode.BadParameter;
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.CustomMarshalers.DeadlineQosPolicyMarshaler.CopyIn",
                        "DDS/OpenSplice/CustomMarshalers/QosContainedMarshalers.cs",
                        DDS.ErrorCode.InvalidValue,
                        "DeadlineQosPolicy attribute may not be a null pointer.");
            }
            return result; 
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
        }

        internal void CopyOut(ref DeadlineQosPolicy to)
        {
            CopyOut(GapiPtr, ref to, 0);
        }

        internal static void CopyOut(IntPtr from, ref DeadlineQosPolicy to, int offset)
        {
        	if (to == null) to = new DeadlineQosPolicy();
            to.Period = BaseMarshaler.ReadDuration(from, offset + offset_period);
        }
    }

    internal class LatencyBudgetQosPolicyMarshaler : GapiMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_latencyBudgetQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_duration = (int)Marshal.OffsetOf(type, "duration");

        public LatencyBudgetQosPolicyMarshaler() :
                base(Gapi.GenericAllocRelease.Alloc(Size))
        { }

        public override void Dispose()
        {
            if (cleanupRequired) 
            {
                CleanupIn(GapiPtr, 0);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(GapiPtr);
        }

        internal DDS.ReturnCode CopyIn(LatencyBudgetQosPolicy from)
        {
            cleanupRequired = true;
            return CopyIn(from, GapiPtr, 0);
        }

        internal static DDS.ReturnCode CopyIn(LatencyBudgetQosPolicy from, IntPtr to, int offset)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                BaseMarshaler.Write(to, offset + offset_duration, from.Duration);
            } else {
                result = DDS.ReturnCode.BadParameter;
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.CustomMarshalers.LatencyBudgetQosPolicyMarshaler.CopyIn",
                        "DDS/OpenSplice/CustomMarshalers/QosContainedMarshalers.cs",
                        DDS.ErrorCode.InvalidValue,
                        "LatencyBudgetQosPolicy attribute may not be a null pointer.");
            }
            return result; 
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
        }

        internal void CopyOut(ref LatencyBudgetQosPolicy to)
        {
            CopyOut(GapiPtr, ref to, 0);
        }

        internal static void CopyOut(IntPtr from, ref LatencyBudgetQosPolicy to, int offset)
        {
        	if (to == null) to = new LatencyBudgetQosPolicy();
            to.Duration = BaseMarshaler.ReadDuration(from, offset + offset_duration);
        }
    }

    internal class LivelinessQosPolicyMarshaler : GapiMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_livelinessQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_kind = (int)Marshal.OffsetOf(type, "kind");
        private static readonly int offset_lease_duration = (int)Marshal.OffsetOf(type, "lease_duration");

        public LivelinessQosPolicyMarshaler() :
                base(Gapi.GenericAllocRelease.Alloc(Size))
        { }

        public override void Dispose()
        {
            if (cleanupRequired) 
            {
                CleanupIn(GapiPtr, 0);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(GapiPtr);
        }

        internal DDS.ReturnCode CopyIn(LivelinessQosPolicy from)
        {
            cleanupRequired = true;
            return CopyIn(from, GapiPtr, 0);
        }

        internal static DDS.ReturnCode CopyIn(LivelinessQosPolicy from, IntPtr to, int offset)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                BaseMarshaler.Write(to, offset + offset_kind, (int)from.Kind);
                BaseMarshaler.Write(to, offset + offset_lease_duration, from.LeaseDuration);
            } else {
                result = DDS.ReturnCode.BadParameter;
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.CustomMarshalers.LivelinessQosPolicyMarshaler.CopyIn",
                        "DDS/OpenSplice/CustomMarshalers/QosContainedMarshalers.cs",
                        DDS.ErrorCode.InvalidValue,
                        "LivelinessQosPolicy attribute may not be a null pointer.");
            }
            return result; 
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
        }

        internal void CopyOut(ref LivelinessQosPolicy to)
        {
            CopyOut(GapiPtr, ref to, 0);
        }

        internal static void CopyOut(IntPtr from, ref LivelinessQosPolicy to, int offset)
        {
        	if (to == null) to = new LivelinessQosPolicy();
        	
            to.Kind = (LivelinessQosPolicyKind)
                BaseMarshaler.ReadInt32(from, offset + offset_kind);

            to.LeaseDuration = BaseMarshaler.ReadDuration(from, offset + offset_lease_duration);
        }
    }

    internal class ReliabilityQosPolicyMarshaler : GapiMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_reliabilityQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_kind = (int)Marshal.OffsetOf(type, "kind");
        private static readonly int offset_max_blocking_time = (int)Marshal.OffsetOf(type, "max_blocking_time");
        private static readonly int offset_synchronous = (int)Marshal.OffsetOf(type, "synchronous");

        public ReliabilityQosPolicyMarshaler() :
                base(Gapi.GenericAllocRelease.Alloc(Size))
        { }

        public override void Dispose()
        {
            if (cleanupRequired) 
            {
                CleanupIn(GapiPtr, 0);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(GapiPtr);
        }

        internal DDS.ReturnCode CopyIn(ReliabilityQosPolicy from)
        {
            cleanupRequired = true;
            return CopyIn(from, GapiPtr, 0);
        }

        internal static DDS.ReturnCode CopyIn(ReliabilityQosPolicy from, IntPtr to, int offset)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                BaseMarshaler.Write(to, offset + offset_kind, (int)from.Kind);
                BaseMarshaler.Write(to, offset + offset_max_blocking_time, from.MaxBlockingTime);
                BaseMarshaler.Write(to, offset + offset_synchronous, from.Synchronous);
            } else {
                result = DDS.ReturnCode.BadParameter;
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.CustomMarshalers.ReliabilityQosPolicyMarshaler.CopyIn",
                        "DDS/OpenSplice/CustomMarshalers/QosContainedMarshalers.cs",
                        DDS.ErrorCode.InvalidValue,
                        "ReliabilityQosPolicy attribute may not be a null pointer.");
            }
            return result; 
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
        }

        internal void CopyOut(ref ReliabilityQosPolicy to)
        {
            CopyOut(GapiPtr, ref to, 0);
        }

        internal static void CopyOut(IntPtr from, ref ReliabilityQosPolicy to, int offset)
        {
        	if (to == null) to = new ReliabilityQosPolicy();
            to.Kind = (ReliabilityQosPolicyKind)
                    BaseMarshaler.ReadInt32(from, offset + offset_kind);
            to.MaxBlockingTime = BaseMarshaler.ReadDuration(from, offset + offset_max_blocking_time);
            to.Synchronous = BaseMarshaler.ReadBoolean(from, offset + offset_synchronous);
        }
    }

    internal class DestinationOrderQosPolicyMarshaler : GapiMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_destinationOrderQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_kind = (int)Marshal.OffsetOf(type, "kind");

        public DestinationOrderQosPolicyMarshaler() :
                base(Gapi.GenericAllocRelease.Alloc(Size))
        { }

        public override void Dispose()
        {
            if (cleanupRequired) 
            {
                CleanupIn(GapiPtr, 0);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(GapiPtr);
        }

        internal DDS.ReturnCode CopyIn(DestinationOrderQosPolicy from)
        {
            cleanupRequired = true;
            return CopyIn(from, GapiPtr, 0);
        }

        internal static DDS.ReturnCode CopyIn(DestinationOrderQosPolicy from, IntPtr to, int offset)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                BaseMarshaler.Write(to, offset + offset_kind, (int)from.Kind);
            } else {
                result = DDS.ReturnCode.BadParameter;
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.CustomMarshalers.DestinationOrderQosPolicyMarshaler.CopyIn",
                        "DDS/OpenSplice/CustomMarshalers/QosContainedMarshalers.cs",
                        DDS.ErrorCode.InvalidValue,
                        "DestinationOrderQosPolicy attribute may not be a null pointer.");
            }
            return result; 
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
        }

        internal void CopyOut(ref DestinationOrderQosPolicy to)
        {
            CopyOut(GapiPtr, ref to, 0);
        }

        internal static void CopyOut(IntPtr from, ref DestinationOrderQosPolicy to, int offset)
        {
        	if (to == null) to = new DestinationOrderQosPolicy();
            to.Kind = (DestinationOrderQosPolicyKind)
                BaseMarshaler.ReadInt32(from, offset + offset_kind);
        }
    }

    internal class HistoryQosPolicyMarshaler : GapiMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_historyQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_kind = (int)Marshal.OffsetOf(type, "kind");
        private static readonly int offset_depth = (int)Marshal.OffsetOf(type, "depth");

        public HistoryQosPolicyMarshaler() :
                base(Gapi.GenericAllocRelease.Alloc(Size))
        { }

        public override void Dispose()
        {
            if (cleanupRequired) 
            {
                CleanupIn(GapiPtr, 0);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(GapiPtr);
        }

        internal DDS.ReturnCode CopyIn(HistoryQosPolicy from)
        {
            cleanupRequired = true;
            return CopyIn(from, GapiPtr, 0);
        }

        internal static DDS.ReturnCode CopyIn(HistoryQosPolicy from, IntPtr to, int offset)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                BaseMarshaler.Write(to, offset + offset_kind, (int)from.Kind);
                BaseMarshaler.Write(to, offset + offset_depth, from.Depth);
            } else {
                result = DDS.ReturnCode.BadParameter;
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.CustomMarshalers.HistoryQosPolicyMarshaler.CopyIn",
                        "DDS/OpenSplice/CustomMarshalers/QosContainedMarshalers.cs",
                        DDS.ErrorCode.InvalidValue,
                        "HistoryQosPolicy attribute may not be a null pointer.");
            }
            return result; 
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
        }

        internal void CopyOut(ref HistoryQosPolicy to)
        {
            CopyOut(GapiPtr, ref to, 0);
        }

        internal static void CopyOut(IntPtr from, ref HistoryQosPolicy to, int offset)
        {
        	if (to == null) to = new HistoryQosPolicy();

            to.Kind = (HistoryQosPolicyKind)
                BaseMarshaler.ReadInt32(from, offset + offset_kind);

            to.Depth = BaseMarshaler.ReadInt32(from, offset + offset_depth);
        }
    }

    internal class ResourceLimitsQosPolicyMarshaler : GapiMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_resourceLimitsQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_max_samples = (int)Marshal.OffsetOf(type, "max_samples");
        private static readonly int offset_max_instances = (int)Marshal.OffsetOf(type, "max_instances");
        private static readonly int offset_max_samples_per_instance = (int)Marshal.OffsetOf(type, "max_samples_per_instance");

        public ResourceLimitsQosPolicyMarshaler() :
                base(Gapi.GenericAllocRelease.Alloc(Size))
        { }

        public override void Dispose()
        {
            if (cleanupRequired) 
            {
                CleanupIn(GapiPtr, 0);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(GapiPtr);
        }

        internal DDS.ReturnCode CopyIn(ResourceLimitsQosPolicy from)
        {
            cleanupRequired = true;
            return CopyIn(from, GapiPtr, 0);
        }

        internal static DDS.ReturnCode CopyIn(ResourceLimitsQosPolicy from, IntPtr to, int offset)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                BaseMarshaler.Write(to, offset + offset_max_samples, from.MaxSamples);
                BaseMarshaler.Write(to, offset + offset_max_instances, from.MaxInstances);
                BaseMarshaler.Write(to, offset + offset_max_samples_per_instance, from.MaxSamplesPerInstance);
            } else {
                result = DDS.ReturnCode.BadParameter;
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.CustomMarshalers.ResourceLimitsQosPolicyMarshaler.CopyIn",
                        "DDS/OpenSplice/CustomMarshalers/QosContainedMarshalers.cs",
                        DDS.ErrorCode.InvalidValue,
                        "ResourceLimitsQosPolicy attribute may not be a null pointer.");
            }
            return result; 
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
        }

        internal void CopyOut(ref ResourceLimitsQosPolicy to)
        {
            CopyOut(GapiPtr, ref to, 0);
        }

        internal static void CopyOut(IntPtr from, ref ResourceLimitsQosPolicy to, int offset)
        {
        	if (to == null) to = new ResourceLimitsQosPolicy();
            to.MaxSamples = BaseMarshaler.ReadInt32(from, offset + offset_max_samples);
            to.MaxInstances = BaseMarshaler.ReadInt32(from, offset + offset_max_instances);
            to.MaxSamplesPerInstance = BaseMarshaler.ReadInt32(from, offset + offset_max_samples_per_instance);
        }
    }

    internal class TransportPriorityQosPolicyMarshaler : GapiMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_transportPriorityQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_value = (int)Marshal.OffsetOf(type, "value");

        public TransportPriorityQosPolicyMarshaler() :
                base(Gapi.GenericAllocRelease.Alloc(Size))
        { }

        public override void Dispose()
        {
            if (cleanupRequired) 
            {
                CleanupIn(GapiPtr, 0);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(GapiPtr);
        }

        internal DDS.ReturnCode CopyIn(TransportPriorityQosPolicy from)
        {
            cleanupRequired = true;
            return CopyIn(from, GapiPtr, 0);
        }

        internal static DDS.ReturnCode CopyIn(TransportPriorityQosPolicy from, IntPtr to, int offset)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                BaseMarshaler.Write(to, offset + offset_value, from.Value);
            } else {
                result = DDS.ReturnCode.BadParameter;
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.CustomMarshalers.TransportPriorityQosPolicyMarshaler.CopyIn",
                        "DDS/OpenSplice/CustomMarshalers/QosContainedMarshalers.cs",
                        DDS.ErrorCode.InvalidValue,
                        "TransportPriorityQosPolicy attribute may not be a null pointer.");
            }
            return result; 
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
        }

        internal void CopyOut(ref TransportPriorityQosPolicy to)
        {
            CopyOut(GapiPtr, ref to, 0);
        }

        internal static void CopyOut(IntPtr from, ref TransportPriorityQosPolicy to, int offset)
        {
        	if (to == null) to = new TransportPriorityQosPolicy();
            to.Value = BaseMarshaler.ReadInt32(from, offset + offset_value);
        }
    }

    internal class LifespanQosPolicyMarshaler : GapiMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_lifespanQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_duration = (int)Marshal.OffsetOf(type, "duration");

        public LifespanQosPolicyMarshaler() :
                base(Gapi.GenericAllocRelease.Alloc(Size))
        { }

        public override void Dispose()
        {
            if (cleanupRequired) 
            {
                CleanupIn(GapiPtr, 0);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(GapiPtr);
        }

        internal DDS.ReturnCode CopyIn(LifespanQosPolicy from)
        {
            cleanupRequired = true;
            return CopyIn(from, GapiPtr, 0);
        }

        internal static DDS.ReturnCode CopyIn(LifespanQosPolicy from, IntPtr to, int offset)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                BaseMarshaler.Write(to, offset + offset_duration, from.Duration);
            } else {
                result = DDS.ReturnCode.BadParameter;
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.CustomMarshalers.LifespanQosPolicyMarshaler.CopyIn",
                        "DDS/OpenSplice/CustomMarshalers/QosContainedMarshalers.cs",
                        DDS.ErrorCode.InvalidValue,
                        "LifespanQosPolicy attribute may not be a null pointer.");
            }
            return result; 
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
        }

        internal void CopyOut(ref LifespanQosPolicy to)
        {
            CopyOut(GapiPtr, ref to, 0);
        }

        internal static void CopyOut(IntPtr from, ref LifespanQosPolicy to, int offset)
        {
        	if (to == null) to = new LifespanQosPolicy();
            to.Duration = BaseMarshaler.ReadDuration(from, offset + offset_duration);
        }
    }

    internal class OwnershipQosPolicyMarshaler : GapiMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_ownershipQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_kind = (int)Marshal.OffsetOf(type, "kind");

        public OwnershipQosPolicyMarshaler() :
                base(Gapi.GenericAllocRelease.Alloc(Size))
        { }

        public override void Dispose()
        {
            if (cleanupRequired) 
            {
                CleanupIn(GapiPtr, 0);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(GapiPtr);
        }

        internal DDS.ReturnCode CopyIn(OwnershipQosPolicy from)
        {
            cleanupRequired = true;
            return CopyIn(from, GapiPtr, 0);
        }

        internal static DDS.ReturnCode CopyIn(OwnershipQosPolicy from, IntPtr to, int offset)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                BaseMarshaler.Write(to, offset + offset_kind, (int)from.Kind);
            } else {
                result = DDS.ReturnCode.BadParameter;
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.CustomMarshalers.OwnershipQosPolicyMarshaler.CopyIn",
                        "DDS/OpenSplice/CustomMarshalers/QosContainedMarshalers.cs",
                        DDS.ErrorCode.InvalidValue,
                        "OwnershipQosPolicy attribute may not be a null pointer.");
            }
            return result; 
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
        }

        internal void CopyOut(ref OwnershipQosPolicy to)
        {
            CopyOut(GapiPtr, ref to, 0);
        }

        internal static void CopyOut(IntPtr from, ref OwnershipQosPolicy to, int offset)
        {
        	if (to == null) to = new OwnershipQosPolicy();
        	
            // Get scheduling_class field
            to.Kind = (OwnershipQosPolicyKind)
                BaseMarshaler.ReadInt32(from, offset + offset_kind);
        }
    }

    internal class TimeBasedFilterQosPolicyMarshaler : GapiMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_timeBasedFilterQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_minimum_separation = (int)Marshal.OffsetOf(type, "minimum_separation");

        public TimeBasedFilterQosPolicyMarshaler() :
                base(Gapi.GenericAllocRelease.Alloc(Size))
        { }

        public override void Dispose()
        {
            if (cleanupRequired) 
            {
                CleanupIn(GapiPtr, 0);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(GapiPtr);
        }

        internal DDS.ReturnCode CopyIn(TimeBasedFilterQosPolicy from)
        {
            cleanupRequired = true;
            return CopyIn(from, GapiPtr, 0);
        }

        internal static DDS.ReturnCode CopyIn(TimeBasedFilterQosPolicy from, IntPtr to, int offset)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                BaseMarshaler.Write(to, offset + offset_minimum_separation, from.MinimumSeparation);
            } else {
                result = DDS.ReturnCode.BadParameter;
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.CustomMarshalers.TimeBasedFilterQosPolicyMarshaler.CopyIn",
                        "DDS/OpenSplice/CustomMarshalers/QosContainedMarshalers.cs",
                        DDS.ErrorCode.InvalidValue,
                        "TimeBasedFilterQosPolicy attribute may not be a null pointer.");
            }
            return result; 
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
        }

        internal void CopyOut(ref TimeBasedFilterQosPolicy to)
        {
            CopyOut(GapiPtr, ref to, 0);
        }

        internal static void CopyOut(IntPtr from, ref TimeBasedFilterQosPolicy to, int offset)
        {
        	if (to == null) to = new TimeBasedFilterQosPolicy();
            to.MinimumSeparation = BaseMarshaler.ReadDuration(from, offset + offset_minimum_separation);
        }
    }

    internal class ReaderDataLifecycleQosPolicyMarshaler : GapiMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_readerDataLifecycleQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_autopurge_nowriter_samples_delay = (int)Marshal.OffsetOf(type, "autopurge_nowriter_samples_delay");
        private static readonly int offset_autopurge_disposed_samples_delay = (int)Marshal.OffsetOf(type, "autopurge_disposed_samples_delay");
        private static readonly int offset_enable_invalid_samples = (int)Marshal.OffsetOf(type, "enable_invalid_samples");
        private static readonly int offset_invalid_sample_visibility = (int)Marshal.OffsetOf(type, "invalid_sample_visibility");

        public ReaderDataLifecycleQosPolicyMarshaler() :
                base(Gapi.GenericAllocRelease.Alloc(Size))
        { }

        public override void Dispose()
        {
            if (cleanupRequired) 
            {
                CleanupIn(GapiPtr, 0);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(GapiPtr);
        }

        internal DDS.ReturnCode CopyIn(ReaderDataLifecycleQosPolicy from)
        {
            cleanupRequired = true;
            return CopyIn(from, GapiPtr, 0);
        }

        internal static DDS.ReturnCode CopyIn(ReaderDataLifecycleQosPolicy from, IntPtr to, int offset)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                BaseMarshaler.Write(to, offset + offset_autopurge_nowriter_samples_delay, from.AutopurgeNowriterSamplesDelay);
                BaseMarshaler.Write(to, offset + offset_autopurge_disposed_samples_delay, from.AutopurgeDisposedSamplesDelay);
                BaseMarshaler.Write(to, offset + offset_enable_invalid_samples, from.EnableInvalidSamples);
                BaseMarshaler.Write(to, offset + offset_invalid_sample_visibility, (int)from.InvalidSampleVisibility.Kind);
            } else {
                result = DDS.ReturnCode.BadParameter;
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.CustomMarshalers.ReaderDataLifecycleQosPolicyMarshaler.CopyIn",
                        "DDS/OpenSplice/CustomMarshalers/QosContainedMarshalers.cs",
                        DDS.ErrorCode.InvalidValue,
                        "ReaderDataLifecycleQosPolicy attribute may not be a null pointer.");
            }
            return result; 
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
        }

        internal void CopyOut(ref ReaderDataLifecycleQosPolicy to)
        {
            CopyOut(GapiPtr, ref to, 0);
        }

        internal static void CopyOut(IntPtr from, ref ReaderDataLifecycleQosPolicy to, int offset)
        {
        	if (to == null) to = new ReaderDataLifecycleQosPolicy();
            to.AutopurgeNowriterSamplesDelay = BaseMarshaler.ReadDuration(from, offset + offset_autopurge_nowriter_samples_delay);
            to.AutopurgeDisposedSamplesDelay = BaseMarshaler.ReadDuration(from, offset + offset_autopurge_disposed_samples_delay);
            to.EnableInvalidSamples = BaseMarshaler.ReadBoolean(from, offset + offset_enable_invalid_samples);
            to.InvalidSampleVisibility.Kind = (InvalidSampleVisibilityQosPolicyKind)BaseMarshaler.ReadInt32(from, offset + offset_invalid_sample_visibility);
        }
    }

    internal class OwnershipStrengthQosPolicyMarshaler : GapiMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_ownershipStrengthQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_value = (int)Marshal.OffsetOf(type, "value");

        public OwnershipStrengthQosPolicyMarshaler() :
                base(Gapi.GenericAllocRelease.Alloc(Size))
        { }

        public override void Dispose()
        {
            if (cleanupRequired) 
            {
                CleanupIn(GapiPtr, 0);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(GapiPtr);
        }

        internal DDS.ReturnCode CopyIn(OwnershipStrengthQosPolicy from)
        {
            cleanupRequired = true;
            return CopyIn(from, GapiPtr, 0);
        }

        internal static DDS.ReturnCode CopyIn(OwnershipStrengthQosPolicy from, IntPtr to, int offset)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                BaseMarshaler.Write(to, offset + offset_value, from.Value);
            } else {
                result = DDS.ReturnCode.BadParameter;
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.CustomMarshalers.OwnershipStrengthQosPolicyMarshaler.CopyIn",
                        "DDS/OpenSplice/CustomMarshalers/QosContainedMarshalers.cs",
                        DDS.ErrorCode.InvalidValue,
                        "OwnershipStrengthQosPolicy attribute may not be a null pointer.");
            }
            return result; 
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
        }

        internal void CopyOut(ref OwnershipStrengthQosPolicy to)
        {
            CopyOut(GapiPtr, ref to, 0);
        }

        internal static void CopyOut(IntPtr from, ref OwnershipStrengthQosPolicy to, int offset)
        {
        	if (to == null) to = new OwnershipStrengthQosPolicy();
            to.Value = BaseMarshaler.ReadInt32(from, offset + offset_value);
        }
    }

    internal class WriterDataLifecycleQosPolicyMarshaler : GapiMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_writerDataLifecycleQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_autodispose_unregistered_instances =
            (int)Marshal.OffsetOf(type, "autodispose_unregistered_instances");
        private static readonly int offset_autopurge_suspended_samples_delay =
            (int)Marshal.OffsetOf(type, "autopurge_suspended_samples_delay");
        private static readonly int offset_autounregister_instance_delay =
            (int)Marshal.OffsetOf(type, "autounregister_instance_delay");

        public WriterDataLifecycleQosPolicyMarshaler() :
                base(Gapi.GenericAllocRelease.Alloc(Size))
        { }

        public override void Dispose()
        {
            if (cleanupRequired) 
            {
                CleanupIn(GapiPtr, 0);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(GapiPtr);
        }

        internal DDS.ReturnCode CopyIn(WriterDataLifecycleQosPolicy from)
        {
            cleanupRequired = true;
            return CopyIn(from, GapiPtr, 0);
        }

        internal static DDS.ReturnCode CopyIn(WriterDataLifecycleQosPolicy from, IntPtr to, int offset)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                BaseMarshaler.Write(
                        to, 
                        offset + offset_autodispose_unregistered_instances,
                        from.AutodisposeUnregisteredInstances);
                BaseMarshaler.Write(
                        to, 
                        offset + offset_autopurge_suspended_samples_delay, 
                        from.AutopurgeSuspendedSamplesDelay);
                BaseMarshaler.Write(
                        to, 
                        offset + offset_autounregister_instance_delay, 
                        from.AutounregisterInstanceDelay);
            } else {
                result = DDS.ReturnCode.BadParameter;
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.CustomMarshalers.WriterDataLifecycleQosPolicyMarshaler.CopyIn",
                        "DDS/OpenSplice/CustomMarshalers/QosContainedMarshalers.cs",
                        DDS.ErrorCode.InvalidValue,
                        "WriterDataLifecycleQosPolicy attribute may not be a null pointer.");
            }
            return result; 
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
        }

        internal void CopyOut(ref WriterDataLifecycleQosPolicy to)
        {
            CopyOut(GapiPtr, ref to, 0);
        }

        internal static void CopyOut(IntPtr from, ref WriterDataLifecycleQosPolicy to, int offset)
        {
        	if (to == null) to = new WriterDataLifecycleQosPolicy();
            to.AutodisposeUnregisteredInstances =
                BaseMarshaler.ReadBoolean(from, offset + offset_autodispose_unregistered_instances);
            to.AutopurgeSuspendedSamplesDelay = BaseMarshaler.ReadDuration(from, offset + offset_autopurge_suspended_samples_delay);
            to.AutounregisterInstanceDelay = BaseMarshaler.ReadDuration(from, offset + offset_autounregister_instance_delay);
        }
    }

    internal class SubscriptionKeyQosPolicyMarshaler : GapiMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_subscriptionKeyQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_use_key_list = (int)Marshal.OffsetOf(type, "use_key_list");
        private static readonly int offset_key_list = (int)Marshal.OffsetOf(type, "key_list");

        public SubscriptionKeyQosPolicyMarshaler() :
                base(Gapi.GenericAllocRelease.Alloc(Size))
        { }

        public override void Dispose()
        {
            if (cleanupRequired) 
            {
                CleanupIn(GapiPtr, 0);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(GapiPtr);
        }

        internal DDS.ReturnCode CopyIn(SubscriptionKeyQosPolicy from)
        {
            cleanupRequired = true;
            return CopyIn(from, GapiPtr, 0);
        }

        internal static DDS.ReturnCode CopyIn(SubscriptionKeyQosPolicy from, IntPtr to, int offset)
        {
            DDS.ReturnCode result;
            if (from != null) {
                BaseMarshaler.Write(to, offset + offset_use_key_list, from.UseKeyList);
                result = SequenceStringMarshaler.CopyIn(from.KeyList, to, offset + offset_key_list);
            } else {
                result = DDS.ReturnCode.BadParameter;
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.CustomMarshalers.SubscriptionKeyQosPolicyMarshaler.CopyIn",
                        "DDS/OpenSplice/CustomMarshalers/QosContainedMarshalers.cs",
                        DDS.ErrorCode.InvalidValue,
                        "SubscriptionKeyQosPolicy attribute may not be a null pointer.");
            }
            return result; 
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
            SequenceStringMarshaler.CleanupIn(nativePtr, offset + offset_key_list);
        }

        internal void CopyOut(ref SubscriptionKeyQosPolicy to)
        {
            CopyOut(GapiPtr, ref to, 0);
        }

        internal static void CopyOut(IntPtr from, ref SubscriptionKeyQosPolicy to, int offset)
        {
        	if (to == null) to = new SubscriptionKeyQosPolicy();
            to.UseKeyList = BaseMarshaler.ReadBoolean(from, offset + offset_use_key_list);
            SequenceStringMarshaler.CopyOut(from, ref to.KeyList, offset + offset_key_list);
        }
    }

    internal class ReaderLifespanQosPolicyMarshaler : GapiMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_readerLifespanQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_use_lifespan = (int)Marshal.OffsetOf(type, "use_lifespan");
        private static readonly int offset_duration = (int)Marshal.OffsetOf(type, "duration");

        public ReaderLifespanQosPolicyMarshaler() :
                base(Gapi.GenericAllocRelease.Alloc(Size))
        { }

        public override void Dispose()
        {
            if (cleanupRequired) 
            {
                CleanupIn(GapiPtr, 0);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(GapiPtr);
        }

        internal DDS.ReturnCode CopyIn(ReaderLifespanQosPolicy from)
        {
            cleanupRequired = true;
            return CopyIn(from, GapiPtr, 0);
        }

        internal static DDS.ReturnCode CopyIn(ReaderLifespanQosPolicy from, IntPtr to, int offset)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                BaseMarshaler.Write(to, offset + offset_use_lifespan, from.UseLifespan);
                BaseMarshaler.Write(to, offset + offset_duration, from.Duration);
            } else {
                result = DDS.ReturnCode.BadParameter;
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.CustomMarshalers.ReaderLifespanQosPolicyMarshaler.CopyIn",
                        "DDS/OpenSplice/CustomMarshalers/QosContainedMarshalers.cs",
                        DDS.ErrorCode.InvalidValue,
                        "ReaderLifespanQosPolicy attribute may not be a null pointer.");
            }
            return result; 
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
        }

        internal void CopyOut(ref ReaderLifespanQosPolicy to)
        {
            CopyOut(GapiPtr, ref to, 0);
        }

        internal static void CopyOut(IntPtr from, ref ReaderLifespanQosPolicy to, int offset)
        {
        	if (to == null) to = new ReaderLifespanQosPolicy();
            to.UseLifespan = BaseMarshaler.ReadBoolean(from, offset + offset_use_lifespan);
            to.Duration = BaseMarshaler.ReadDuration(from, offset + offset_duration);
        }
    }

    internal class InvalidSampleVisibilityQosPolicyMarshaler : GapiMarshaler
    {
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_invalidSampleVisibilityQosPolicy);
        public static readonly int Size = Marshal.SizeOf(type);

        private static readonly int offset_kind = (int)Marshal.OffsetOf(type, "kind");

        public InvalidSampleVisibilityQosPolicyMarshaler() :
                base(Gapi.GenericAllocRelease.Alloc(Size))
        { }

        public override void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(GapiPtr, 0);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(GapiPtr);
        }

        internal DDS.ReturnCode CopyIn(InvalidSampleVisibilityQosPolicy from)
        {
            cleanupRequired = true;
            return CopyIn(from, GapiPtr, 0);
        }

        internal static DDS.ReturnCode CopyIn(InvalidSampleVisibilityQosPolicy from, IntPtr to, int offset)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                BaseMarshaler.Write(to, offset + offset_kind, (int)from.Kind);
            } else {
                result = DDS.ReturnCode.BadParameter;
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.CustomMarshalers.InvalidSampleVisibilityQosPolicyMarshaler.CopyIn",
                        "DDS/OpenSplice/CustomMarshalers/QosContainedMarshalers.cs",
                        DDS.ErrorCode.InvalidValue,
                        "InvalidSampleVisibilityQosPolicy attribute may not be a null pointer.");
            }
            return result;
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
        }

        internal void CopyOut(ref InvalidSampleVisibilityQosPolicy to)
        {
            CopyOut(GapiPtr, ref to, 0);
        }

        internal static void CopyOut(IntPtr from, ref InvalidSampleVisibilityQosPolicy to, int offset)
        {
            if (to == null) to = new InvalidSampleVisibilityQosPolicy();
            to.Kind = (InvalidSampleVisibilityQosPolicyKind)
                BaseMarshaler.ReadInt32(from, offset + offset_kind);
        }
    }
}
