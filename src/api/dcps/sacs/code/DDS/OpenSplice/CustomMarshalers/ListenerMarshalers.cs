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
using System.Runtime.InteropServices;

namespace DDS.OpenSplice.CustomMarshalers
{

    internal static class MarshalHelper
    {
        public static IntPtr GetIntPtrForDelegate(Delegate d)
        {
            return d == null ? IntPtr.Zero : Marshal.GetFunctionPointerForDelegate(d);
        }
    }

    internal class TopicListenerMarshaler : IMarshaler
    {
        private static Type topicListenerType = typeof(OpenSplice.Gapi.gapi_topicListener);
        public static readonly int Size = Marshal.SizeOf(topicListenerType);

        private static int offset_listener_data = (int)Marshal.OffsetOf(topicListenerType, "listener_data");
        private static int offset_on_inconsistent_topic = (int)Marshal.OffsetOf(topicListenerType, "on_inconsistent_topic");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public TopicListenerMarshaler(ref OpenSplice.Gapi.gapi_topicListener listener)
            : this()
        {
            CopyIn(ref listener, gapiPtr, 0);
            cleanupRequired = true;
        }

        public TopicListenerMarshaler()
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

        internal static void CopyIn(ref OpenSplice.Gapi.gapi_topicListener from, IntPtr to, int offset)
        {
            // Set listener_data field
            //            Marshal.WriteIntPtr(to, offset + offset_listener_data, from.listener_data);
            TypeSupport.Write(to, offset + offset_listener_data, IntPtr.Zero);

            // Set on_inconsistent_topic field
            TypeSupport.Write(to, offset + offset_on_inconsistent_topic, MarshalHelper.GetIntPtrForDelegate(from.on_inconsistent_topic));
        }

        internal static void CleanupIn(IntPtr pNativeData, int offset)
        {
            // Currently nothing to cleanup.
        }
    }

    internal class PublisherDataWriterListenerMarshaler : IMarshaler
    {
        private static Type type = typeof(OpenSplice.Gapi.gapi_publisherDataWriterListener);
        public static readonly int Size = Marshal.SizeOf(type);

        private static int offset_listener_data = (int)Marshal.OffsetOf(type, "listener_data");
        private static int offset_on_offered_deadline_missed = (int)Marshal.OffsetOf(type, "on_offered_deadline_missed");
        private static int offset_on_offered_incompatible_qos = (int)Marshal.OffsetOf(type, "on_offered_incompatible_qos");
        private static int offset_on_liveliness_lost = (int)Marshal.OffsetOf(type, "on_liveliness_lost");
        private static int offset_on_publication_match = (int)Marshal.OffsetOf(type, "on_publication_match");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public PublisherDataWriterListenerMarshaler(ref OpenSplice.Gapi.gapi_publisherDataWriterListener listener)
            : this()
        {
            CopyIn(ref listener, gapiPtr, 0);
            cleanupRequired = true;
        }

        public PublisherDataWriterListenerMarshaler()
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

        internal static void CopyIn(ref OpenSplice.Gapi.gapi_publisherDataWriterListener from, IntPtr to, int offset)
        {
            // Set listener_data field
            //            Marshal.WriteIntPtr(to, offset + offset_listener_data, from.listener_data);
            TypeSupport.Write(to, offset + offset_listener_data, IntPtr.Zero);

            // Set callback fields
            TypeSupport.Write(to, offset + offset_on_offered_deadline_missed, MarshalHelper.GetIntPtrForDelegate(from.on_offered_deadline_missed));
            TypeSupport.Write(to, offset + offset_on_offered_incompatible_qos, MarshalHelper.GetIntPtrForDelegate(from.on_offered_incompatible_qos));
            TypeSupport.Write(to, offset + offset_on_liveliness_lost, MarshalHelper.GetIntPtrForDelegate(from.on_liveliness_lost));
            TypeSupport.Write(to, offset + offset_on_publication_match, MarshalHelper.GetIntPtrForDelegate(from.on_publication_match));
        }

        internal static void CleanupIn(IntPtr pNativeData, int offset)
        {
            // Currently nothing to cleanup.
        }

    }

    internal class DataReaderListenerMarshaler : IMarshaler
    {
        private static Type type = typeof(OpenSplice.Gapi.gapi_dataReaderListener);
        public static readonly int Size = Marshal.SizeOf(type);

        private static int offset_listener_data = (int)Marshal.OffsetOf(type, "listener_data");
        private static int offset_on_requested_deadline_missed = (int)Marshal.OffsetOf(type, "on_requested_deadline_missed");
        private static int offset_on_requested_incompatible_qos = (int)Marshal.OffsetOf(type, "on_requested_incompatible_qos");
        private static int offset_on_sample_rejected = (int)Marshal.OffsetOf(type, "on_sample_rejected");
        private static int offset_on_liveliness_changed = (int)Marshal.OffsetOf(type, "on_liveliness_changed");
        private static int offset_on_data_available = (int)Marshal.OffsetOf(type, "on_data_available");
        private static int offset_on_subscription_match = (int)Marshal.OffsetOf(type, "on_subscription_match");
        private static int offset_on_sample_lost = (int)Marshal.OffsetOf(type, "on_sample_lost");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public DataReaderListenerMarshaler(ref OpenSplice.Gapi.gapi_dataReaderListener listener)
            : this()
        {
            CopyIn(ref listener, gapiPtr, 0);
            cleanupRequired = true;
        }

        public DataReaderListenerMarshaler()
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

        internal static void CopyIn(ref OpenSplice.Gapi.gapi_dataReaderListener from, IntPtr to, int offset)
        {
            // Set listener_data field
            //            Marshal.WriteIntPtr(to, offset + offset_listener_data, from.listener_data);
            TypeSupport.Write(to, offset + offset_listener_data, IntPtr.Zero);

            // Set callback fields
            TypeSupport.Write(to, offset + offset_on_requested_deadline_missed, MarshalHelper.GetIntPtrForDelegate(from.on_requested_deadline_missed));
            TypeSupport.Write(to, offset + offset_on_requested_incompatible_qos, MarshalHelper.GetIntPtrForDelegate(from.on_requested_incompatible_qos));
            TypeSupport.Write(to, offset + offset_on_sample_rejected, MarshalHelper.GetIntPtrForDelegate(from.on_sample_rejected));
            TypeSupport.Write(to, offset + offset_on_liveliness_changed, MarshalHelper.GetIntPtrForDelegate(from.on_liveliness_changed));
            TypeSupport.Write(to, offset + offset_on_data_available, MarshalHelper.GetIntPtrForDelegate(from.on_data_available));
            TypeSupport.Write(to, offset + offset_on_subscription_match, MarshalHelper.GetIntPtrForDelegate(from.on_subscription_match));
            TypeSupport.Write(to, offset + offset_on_sample_lost, MarshalHelper.GetIntPtrForDelegate(from.on_sample_lost));
        }

        internal static void CleanupIn(IntPtr pNativeData, int offset)
        {
            // Currently nothing to cleanup.
        }

    }

    internal class SubscriberListenerMarshaler : IMarshaler
    {
        private static Type type = typeof(OpenSplice.Gapi.gapi_subscriberListener);
        public static readonly int Size = Marshal.SizeOf(type);

        private static int offset_dataReader = (int)Marshal.OffsetOf(type, "dataReader");
        private static int offset_on_data_on_readers = (int)Marshal.OffsetOf(type, "on_data_on_readers");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public SubscriberListenerMarshaler(ref OpenSplice.Gapi.gapi_subscriberListener listener)
            : this()
        {
            CopyIn(ref listener, gapiPtr, 0);
            cleanupRequired = true;
        }

        public SubscriberListenerMarshaler()
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

        internal static void CopyIn(ref OpenSplice.Gapi.gapi_subscriberListener from, IntPtr to, int offset)
        {
            DataReaderListenerMarshaler.CopyIn(ref from.dataReader, to, offset_dataReader);

            // set on_data_on_readers field
            TypeSupport.Write(to, offset + offset_on_data_on_readers, MarshalHelper.GetIntPtrForDelegate(from.on_data_on_readers));
        }

        internal static void CleanupIn(IntPtr pNativeData, int offset)
        {
            // Currently nothing to cleanup.
        }
    }

    internal class DomainParticipantListenerMarshaler : IMarshaler
    {
        private static Type type = typeof(OpenSplice.Gapi.gapi_domainParticipantListener);
        public static readonly int Size = Marshal.SizeOf(type);

        private static int offset_listener_data = (int)Marshal.OffsetOf(type, "listener_data");
        private static int offset_on_inconsistent_topic = (int)Marshal.OffsetOf(type, "on_inconsistent_topic");
        private static int offset_on_offered_deadline_missed = (int)Marshal.OffsetOf(type, "on_offered_deadline_missed");
        private static int offset_on_offered_incompatible_qos = (int)Marshal.OffsetOf(type, "on_offered_incompatible_qos");
        private static int offset_on_liveliness_lost = (int)Marshal.OffsetOf(type, "on_liveliness_lost");
        private static int offset_on_publication_match = (int)Marshal.OffsetOf(type, "on_publication_match");
        private static int offset_on_requested_deadline_missed = (int)Marshal.OffsetOf(type, "on_requested_deadline_missed");
        private static int offset_on_requested_incompatible_qos = (int)Marshal.OffsetOf(type, "on_requested_incompatible_qos");
        private static int offset_on_sample_rejected = (int)Marshal.OffsetOf(type, "on_sample_rejected");
        private static int offset_on_liveliness_changed = (int)Marshal.OffsetOf(type, "on_liveliness_changed");
        private static int offset_on_data_available = (int)Marshal.OffsetOf(type, "on_data_available");
        private static int offset_on_subscription_match = (int)Marshal.OffsetOf(type, "on_subscription_match");
        private static int offset_on_sample_lost = (int)Marshal.OffsetOf(type, "on_sample_lost");
        private static int offset_on_data_on_readers = (int)Marshal.OffsetOf(type, "on_data_on_readers");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public DomainParticipantListenerMarshaler(ref OpenSplice.Gapi.gapi_domainParticipantListener listener)
            : this()
        {
            CopyIn(ref listener, gapiPtr, 0);
            cleanupRequired = true;
        }

        public DomainParticipantListenerMarshaler()
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

        public static void CopyIn(ref Gapi.gapi_domainParticipantListener from, IntPtr to, int offset)
        {
            // Set listener_data field
            TypeSupport.Write(to, offset + offset_listener_data, IntPtr.Zero);

            // Set callback fields
            TypeSupport.Write(to, offset + offset_on_inconsistent_topic, MarshalHelper.GetIntPtrForDelegate(from.on_inconsistent_topic));
            TypeSupport.Write(to, offset + offset_on_offered_deadline_missed, MarshalHelper.GetIntPtrForDelegate(from.on_offered_deadline_missed));
            TypeSupport.Write(to, offset + offset_on_offered_incompatible_qos, MarshalHelper.GetIntPtrForDelegate(from.on_offered_incompatible_qos));
            TypeSupport.Write(to, offset + offset_on_liveliness_lost, MarshalHelper.GetIntPtrForDelegate(from.on_liveliness_lost));
            TypeSupport.Write(to, offset + offset_on_publication_match, MarshalHelper.GetIntPtrForDelegate(from.on_publication_match));
            TypeSupport.Write(to, offset + offset_on_requested_deadline_missed, MarshalHelper.GetIntPtrForDelegate(from.on_requested_deadline_missed));
            TypeSupport.Write(to, offset + offset_on_requested_incompatible_qos, MarshalHelper.GetIntPtrForDelegate(from.on_requested_incompatible_qos));
            TypeSupport.Write(to, offset + offset_on_sample_rejected, MarshalHelper.GetIntPtrForDelegate(from.on_sample_rejected));
            TypeSupport.Write(to, offset + offset_on_liveliness_changed, MarshalHelper.GetIntPtrForDelegate(from.on_liveliness_changed));
            TypeSupport.Write(to, offset + offset_on_data_available, MarshalHelper.GetIntPtrForDelegate(from.on_data_available));
            TypeSupport.Write(to, offset + offset_on_subscription_match, MarshalHelper.GetIntPtrForDelegate(from.on_subscription_match));
            TypeSupport.Write(to, offset + offset_on_sample_lost, MarshalHelper.GetIntPtrForDelegate(from.on_sample_lost));
            TypeSupport.Write(to, offset + offset_on_data_on_readers, MarshalHelper.GetIntPtrForDelegate(from.on_data_on_readers));
        }

        public static void CleanupIn(IntPtr nativeData, int offset)
        {
            // Currently nothing to cleanup.
        }
    }
}