using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

namespace DDS.OpenSplice.CustomMarshalers
{
    public class DDSTopicBuiltinTopicMarshaler : BaseMarshaler
    {
        private static Type type = typeof(DDS.OpenSplice.Gapi.gapi_topicBuiltinTopicData);
        public static readonly int Size = Marshal.SizeOf(type);

        private static int offset_key = (int)Marshal.OffsetOf(type, "key");
        private static int offset_name = (int)Marshal.OffsetOf(type, "name");
        private static int offset_typename = (int)Marshal.OffsetOf(type, "type_name");
        private static int offset_durability = (int)Marshal.OffsetOf(type, "durability");
        private static int offset_durability_servie = (int)Marshal.OffsetOf(type, "durability_service");
        private static int offset_deadline = (int)Marshal.OffsetOf(type, "deadline");
        private static int offset_latencybudget = (int)Marshal.OffsetOf(type, "latency_budget");
        private static int offset_liveliness = (int)Marshal.OffsetOf(type, "liveliness");
        private static int offset_reliability = (int)Marshal.OffsetOf(type, "reliability");
        private static int offset_transportpriority = (int)Marshal.OffsetOf(type, "transport_priority");
        private static int offset_lifespan = (int)Marshal.OffsetOf(type, "lifespan");
        private static int offset_destinationorder = (int)Marshal.OffsetOf(type, "destination_order");
        private static int offset_history = (int)Marshal.OffsetOf(type, "history");
        private static int offset_resourcelimits = (int)Marshal.OffsetOf(type, "resource_limits");
        private static int offset_ownership = (int)Marshal.OffsetOf(type, "ownership");
        private static int offset_topicdata = (int)Marshal.OffsetOf(type, "topic_data");
        private static int offset_meta_data = (int)Marshal.OffsetOf(type, "meta_data");
        private static int offset_key_list = (int)Marshal.OffsetOf(type, "key_list");

        override public object[] SampleReaderAlloc(int length)
        {
            return new DDS.TopicBuiltinTopicData[length];
        }

        public override void CopyOut(IntPtr from, ref object toObject, int offset)
        {
            DDS.TopicBuiltinTopicData toSample;

            // CopyOut may be priced an existing destination object to recycle.
            // Although it would be functionally correct to always allocate a new
            // destination sample structure, comply with caller's request. 
            
            if (toObject == null)
            {
                toSample = new DDS.OpenSplice.TopicBuiltinTopicData() as DDS.TopicBuiltinTopicData;
                toObject = toSample;
            }
            else
                toSample = toObject as DDS.TopicBuiltinTopicData;

            BuiltinTopicKeyMarshaler.CopyOut(from, out toSample.Key, offset + offset_key);
            toSample.Name = BaseMarshaler.ReadString(from, offset + offset_name);
            toSample.TypeName = BaseMarshaler.ReadString(from, offset + offset_typename);
            DurabilityQosPolicyMarshaler.CopyOut(from, out toSample.Durability, offset + offset_durability);
            DurabilityServiceQosPolicyMarshaler.CopyOut(from, out toSample.DurabilityService, offset + offset_durability_servie);
            DeadlineQosPolicyMarshaler.CopyOut(from, out toSample.Deadline, offset + offset_deadline);
            LatencyBudgetQosPolicyMarshaler.CopyOut(from, out toSample.LatencyBudget, offset + offset_latencybudget);
            LivelinessQosPolicyMarshaler.CopyOut(from, out toSample.Liveliness, offset + offset_liveliness);
            ReliabilityQosPolicyMarshaler.CopyOut(from, out toSample.Reliability, offset + offset_reliability);
            TransportPriorityQosPolicyMarshaler.CopyOut(from, out toSample.TransportPriority, offset + offset_transportpriority);
            LifespanQosPolicyMarshaler.CopyOut(from, out toSample.Lifespan, offset + offset_lifespan);
            DestinationOrderQosPolicyMarshaler.CopyOut(from, out toSample.DestinationOrder, offset + offset_destinationorder);
            OwnershipQosPolicyMarshaler.CopyOut(from, out toSample.Ownership, offset + offset_ownership);

            // The topic_data member has a shared memory database sequence (not a Gapi Sequence), so we can't use the 
            // gapi *QosPolicyMarshalers for this field.  Instead we extract the field position and use
            // a marshaler for a database sequence.
            DatabaseSequenceOctetMarshaler.CopyOut(from, out toSample.TopicData.Value, offset + offset_topicdata);

            // The OpenSplice TopicBuiltinTopicData contains a few more additional non-standard fields.
            DDS.OpenSplice.TopicBuiltinTopicData toOsplSample = toSample as DDS.OpenSplice.TopicBuiltinTopicData;
            if (toOsplSample != null)
            {
                toOsplSample.meta_data = BaseMarshaler.ReadString(from, offset + offset_meta_data);
                toOsplSample.key_list = BaseMarshaler.ReadString(from, offset + offset_key_list);
            }
        }

        public override bool CopyIn(IntPtr basePtr, IntPtr from, IntPtr to)
        {
            throw new NotImplementedException();
        }

        public override bool CopyIn(System.IntPtr basePtr, object from, System.IntPtr to, int offset)
        {
            throw new NotImplementedException();
        }
    }
}
