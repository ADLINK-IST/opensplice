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
using System.Runtime.InteropServices;

namespace DDS.OpenSplice.CustomMarshalers
{
    internal class OfferedIncompatibleQosStatusMarshaler : GapiMarshaler
    {
        private static readonly Type seqType = typeof(Gapi.gapi_Seq);
        public static readonly int seqSize = Marshal.SizeOf(seqType);

        private static readonly Type dataType = typeof(Gapi.gapi_offeredRequestedIncompatibleQosStatus);
        public static readonly int dataSize = Marshal.SizeOf(dataType);
        private static readonly int offset_total_count = (int)Marshal.OffsetOf(dataType, "total_count");
        private static readonly int offset_total_count_change = (int)Marshal.OffsetOf(dataType, "total_count_change");
        private static readonly int offset_last_policy_id = (int)Marshal.OffsetOf(dataType, "last_policy_id");
        private static readonly int offset_policies = (int)Marshal.OffsetOf(dataType, "policies");

        public OfferedIncompatibleQosStatusMarshaler() : 
                base(Gapi.GenericAllocRelease.Alloc(dataSize))
        { }

        public override void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(GapiPtr, 0);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(GapiPtr);
        }

        internal DDS.ReturnCode CopyIn(DDS.OfferedIncompatibleQosStatus from)
        {
            cleanupRequired = true;
            return CopyIn(from, GapiPtr, 0);
        }

        internal static DDS.ReturnCode CopyIn(
                DDS.OfferedIncompatibleQosStatus from, 
                IntPtr to, 
                int offset)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                Write(to, offset_total_count, from.TotalCount);
                Write(to, offset_total_count_change, from.TotalCountChange);
                Write(to, offset_last_policy_id, (int) from.LastPolicyId);
                result = QosPolicyCountSequenceMarshaler.CopyIn(from.Policies, to, offset_policies);
            } else {
                result = DDS.ReturnCode.BadParameter;
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.CustomMarshalers.OfferedIncompatibleQosStatusMarshaler.CopyIn",
                        "DDS/OpenSplice/CustomMarshalers/StatusMarshalers.cs",
                        DDS.ErrorCode.InvalidValue,
                        "OfferedIncompatibleQosStatus attribute may not be a null pointer.");
            }
            return result; 
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
            QosPolicyCountSequenceMarshaler.CleanupIn(nativePtr, offset + offset_policies);
        }

        internal void CopyOut(ref DDS.OfferedIncompatibleQosStatus to)
        {
            CopyOut(GapiPtr, ref to, 0);
        }

        internal static void CopyOut(
                IntPtr from, 
                ref DDS.OfferedIncompatibleQosStatus to,
                int offset)
        {
            // Initialize managed array to the correct size.
            if (to == null)
            {
                to = new DDS.OfferedIncompatibleQosStatus();
            }

            to.TotalCount = ReadInt32(from, offset + offset_total_count);
            to.TotalCountChange = ReadInt32(from, offset + offset_total_count_change);
            to.LastPolicyId = (QosPolicyId) ReadInt32(from, offset + offset_last_policy_id);
            QosPolicyCount[] policies = to.Policies;
            QosPolicyCountSequenceMarshaler.CopyOut(from, ref to.Policies, offset + offset_policies);
            if (policies != to.Policies)
            {
                to.Policies = policies;
            }
        }
    }

    internal class RequestedIncompatibleQosStatusMarshaler : GapiMarshaler
    {
        private static readonly Type seqType = typeof(Gapi.gapi_Seq);
        public static readonly int seqSize = Marshal.SizeOf(seqType);

        private static readonly Type dataType = typeof(Gapi.gapi_offeredRequestedIncompatibleQosStatus);
        public static readonly int dataSize = Marshal.SizeOf(dataType);
        private static readonly int offset_total_count = (int)Marshal.OffsetOf(dataType, "total_count");
        private static readonly int offset_total_count_change = (int)Marshal.OffsetOf(dataType, "total_count_change");
        private static readonly int offset_last_policy_id = (int)Marshal.OffsetOf(dataType, "last_policy_id");
        private static readonly int offset_policies = (int)Marshal.OffsetOf(dataType, "policies");

        public RequestedIncompatibleQosStatusMarshaler() :
                base(Gapi.GenericAllocRelease.Alloc(dataSize))
        { }

        public override void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(GapiPtr, 0);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(GapiPtr);
        }

        internal DDS.ReturnCode CopyIn(DDS.RequestedIncompatibleQosStatus from)
        {
            cleanupRequired = true;
            return CopyIn(from, GapiPtr, 0);
        }

        internal static DDS.ReturnCode CopyIn(
                DDS.RequestedIncompatibleQosStatus from, 
                IntPtr to, 
                int offset)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            if (from != null) {
                Write(to, offset_total_count, from.TotalCount);
                Write(to, offset_total_count_change, from.TotalCountChange);
                Write(to, offset_last_policy_id, (int) from.LastPolicyId);
                result = QosPolicyCountSequenceMarshaler.CopyIn(from.Policies, to, offset_policies);
            } else {
                result = DDS.ReturnCode.BadParameter;
                DDS.OpenSplice.OS.Report(
                        DDS.OpenSplice.ReportType.OS_ERROR,
                        "DDS.OpenSplice.CustomMarshalers.RequestedIncompatibleQosStatusMarshaler.CopyIn",
                        "DDS/OpenSplice/CustomMarshalers/StatusMarshalers.cs",
                        DDS.ErrorCode.InvalidValue,
                        "RequestedIncompatibleQosStatus attribute may not be a null pointer.");
            }
            return result; 
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
            QosPolicyCountSequenceMarshaler.CleanupIn(nativePtr, offset + offset_policies);
        }

        internal void CopyOut(ref DDS.RequestedIncompatibleQosStatus to)
        {
            CopyOut(GapiPtr, ref to, 0);
        }

        internal static void CopyOut(
                IntPtr from, 
                ref DDS.RequestedIncompatibleQosStatus to,
                int offset)
        {
            // Initialize managed array to the correct size.
            if (to == null)
            {
                to = new DDS.RequestedIncompatibleQosStatus();
            }

            to.TotalCount = ReadInt32(from, offset + offset_total_count);
            to.TotalCountChange = ReadInt32(from, offset + offset_total_count_change);
            to.LastPolicyId = (QosPolicyId) ReadInt32(from, offset + offset_last_policy_id);
            QosPolicyCount[] policies = to.Policies;
            QosPolicyCountSequenceMarshaler.CopyOut(from, ref to.Policies, offset + offset_policies);
            if (policies != to.Policies)
            {
                to.Policies = policies;
            }
        }
    }
}