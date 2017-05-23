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
using DDS.OpenSplice.Database;
using DDS.OpenSplice.OS;

namespace DDS.OpenSplice.CustomMarshalers
{
    /*internal class QosPolicyCountSequenceMarshaler : GapiMarshaler
    {
        private static readonly Type seqType = typeof(Gapi.gapi_Seq);
        public static readonly int seqSize = Marshal.SizeOf(seqType);
        private static readonly int offset__maximum = (int)Marshal.OffsetOf(seqType, "_maximum");
        private static readonly int offset__length = (int)Marshal.OffsetOf(seqType, "_length");
        private static readonly int offset__buffer = (int)Marshal.OffsetOf(seqType, "_buffer");
        private static readonly int offset__release = (int)Marshal.OffsetOf(seqType, "_release");

        private static readonly Type bufType = typeof(Gapi.gapi_qosPolicyCount);
        public static readonly int bufSize = Marshal.SizeOf(bufType);
        private static readonly int offset_policy_id = (int)Marshal.OffsetOf(bufType, "policy_id");
        private static readonly int offset_count = (int)Marshal.OffsetOf(bufType, "count");

        public QosPolicyCountSequenceMarshaler() :
                base(Gapi.GenericAllocRelease.sequence_alloc())
        { }

        public override void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(GapiPtr, 0);
            }
            Gapi.GenericAllocRelease.Free(GapiPtr);
        }

        internal DDS.ReturnCode CopyIn(QosPolicyCount[] from)
        {
            cleanupRequired = true;
            return CopyIn(from, GapiPtr, 0);
        }

        internal static DDS.ReturnCode CopyIn(QosPolicyCount[] from, IntPtr to, int offset)
        {
            int index = 0;
            Int32 length = 0;
            IntPtr arrayPtr = IntPtr.Zero;
            DDS.ReturnCode result = DDS.ReturnCode.Ok;

            // guard against null
            if (from != null)
            {
                // Determine the Length of the array.
                length = (Int32)from.Length;
            }

            // Set _max field.
            BaseMarshaler.Write(to, offset + offset__maximum, length);

            // Set _length field.
            BaseMarshaler.Write(to, offset + offset__length, length);

            if (length > 0)
            {
                // Allocate a buffer containing the sequence content starting at element 0
                // up til the full Length of the array.
                arrayPtr = Gapi.GenericAllocRelease.Alloc(bufSize * length);
                for (index = 0; index < length && result == DDS.ReturnCode.Ok; index++)
                {
                    Write(arrayPtr, bufSize * index + offset_policy_id, (int) from[index].PolicyId);
                    Write(arrayPtr, bufSize * index + offset_count, from[index].Count);
                }
            }

            // Set _length field with the amount of elements that are copied.
            BaseMarshaler.Write(to, offset + offset__length, index);

            // _buffer field: set the pointer to the appropriate location.
            BaseMarshaler.Write(to, offset + offset__buffer, arrayPtr);

            // Set _release field
            BaseMarshaler.Write(to, offset + offset__release, (byte)1);

            return result;
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
            // Read the buffer pointer containing the sequence content
            IntPtr arrayPtr = ReadIntPtr(nativePtr, offset + offset__buffer);

            // free array
            OpenSplice.Gapi.GenericAllocRelease.Free(arrayPtr);
            BaseMarshaler.Write(nativePtr, offset + offset__buffer, IntPtr.Zero);
        }

        internal void CopyOut(ref QosPolicyCount[] to)
        {
            CopyOut(GapiPtr, ref to, 0);
        }

        internal static void CopyOut(IntPtr from, ref QosPolicyCount[] to, int offset)
        {
            // Get _length field
            int length = BaseMarshaler.ReadInt32(from, offset + offset__length);

            // Initialize managed array to the correct size.
            if (to == null || to.Length != length)
            {
                to = new DDS.QosPolicyCount[length];
            }

            if (length > 0)
            {
                // Read the buffer pointer containing the sequence content
                IntPtr arrayPtr = BaseMarshaler.ReadIntPtr(from, offset + offset__buffer);

                for (int index = 0; index < length; index++)
                {
                    // Loop through the string pointers, deallocating each
                    to[index].PolicyId = (QosPolicyId) ReadInt32(
                            arrayPtr, (bufSize * index) + offset_policy_id);
                    to[index].Count = ReadInt32(
                            arrayPtr, (bufSize * index) + offset_count);
                }
            }
        }
    }

    internal class SequenceIntPtrMarshaler : GapiMarshaler
    {
        public static readonly int Size = Marshal.SizeOf(typeof(System.IntPtr));
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_Seq);
        public static readonly int StructSize = Marshal.SizeOf(type);

        private static readonly int offset__maximum = (int)Marshal.OffsetOf(type, "_maximum");
        private static readonly int offset__length = (int)Marshal.OffsetOf(type, "_length");
        private static readonly int offset__buffer = (int)Marshal.OffsetOf(type, "_buffer");
        private static readonly int offset__release = (int)Marshal.OffsetOf(type, "_release");

        public SequenceIntPtrMarshaler(IntPtr[] from)
            : this()
        {
            CopyIn(from, GapiPtr, 0);
        }

        public SequenceIntPtrMarshaler() :
                base(Gapi.GenericAllocRelease.sequence_alloc())
        { }

        public override void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(GapiPtr, 0);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(GapiPtr);
        }

        internal void CopyIn(IntPtr[] from)
        {
            cleanupRequired = true;
            CopyIn(from, GapiPtr, 0);
        }

        internal static void CopyIn(IntPtr[] from, IntPtr to, int offset)
        {
            Int32 length = 0;

            // guard against null
            if (from != null)
            {
                // Determine the Length of the array.
                length = (Int32)from.Length;
            }

            // Set _max field.
            BaseMarshaler.Write(to, offset + offset__maximum, length);

            // Set _length field
            BaseMarshaler.Write(to, offset + offset__length, length);

            IntPtr bufPtr = IntPtr.Zero;
            if (length > 0)
            {
                // Allocate a buffer containing the sequence content starting at element 0
                // up til the full Length of the array.
                bufPtr = OpenSplice.Gapi.GenericAllocRelease.Alloc(Size * length);
                Marshal.Copy(from, 0, bufPtr, length);
            }

            // _buffer field: set the pointer to the appropriate location.
            BaseMarshaler.Write(to, offset + offset__buffer, bufPtr);

            // Set _release field
            BaseMarshaler.Write(to, offset + offset__release, (byte)1);
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
            // Read the buffer pointer containing the sequence content and de-allocate it.
            IntPtr bufPtr = BaseMarshaler.ReadIntPtr(nativePtr, offset + offset__buffer);
            OpenSplice.Gapi.GenericAllocRelease.Free(bufPtr);
            BaseMarshaler.Write(nativePtr, offset + offset__buffer, IntPtr.Zero);
        }

        internal void CopyOut(ref IntPtr[] to)
        {
            CopyOut(GapiPtr, ref to, 0);
        }

        internal static void CopyOut(IntPtr from, ref IntPtr[] to, int offset)
        {
            // Get _length field
            int length = BaseMarshaler.ReadInt32(from, offset + offset__length);

            // Initialize managed array to the correct size.
            if (to == null || to.Length != length)
            {
                to = new IntPtr[length];
            }

            if (length > 0)
            {
                // Read the buffer pointer containing the sequence content
                IntPtr arrayPtr = BaseMarshaler.ReadIntPtr(from, offset + offset__buffer);

                for (int index = 0; index < length; index++)
                {
                    // Loop through the string pointers, deallocating each
                    to[index] = BaseMarshaler.ReadIntPtr(arrayPtr, Size * index);
                }
            }
        }
    }*/

    internal class SequenceOctetMarshaler : BaseMarshaler, IDisposable
    {
        public static readonly int Size = Marshal.SizeOf(typeof(System.IntPtr));

        protected bool cleanupRequired = false;
        private IntPtr userPtr;

        public SequenceOctetMarshaler()
        {
            userPtr = IntPtr.Zero;
        }

        public SequenceOctetMarshaler(IntPtr nativePtr)
        {
            userPtr = nativePtr;
        }

        public IntPtr UserPtr
        {
            get { return userPtr; }
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(ref userPtr);
            }
        }

        internal DDS.ReturnCode copyIn(byte[] from)
        {
            cleanupRequired = true;
            return CopyIn(from, ref userPtr);
        }

        internal static DDS.ReturnCode CopyIn(byte[] from, ref IntPtr to)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            Int32 length = 0;

            // guard against null
            if (from != null)
            {
                // Determine the Length of the array.
                length = (Int32)from.Length;
            }

            if (length > 0)
            {
                // Allocate a buffer containing the sequence content starting at element 0 up til the full Length of the array.
                to = os.malloc(new IntPtr(Size * length));
                if (to != IntPtr.Zero) {
                    Marshal.Copy(from, 0, to, length);
                } else {
                    result = DDS.ReturnCode.OutOfResources;
                }
            } else {
                to = IntPtr.Zero;
            }
            return result;
        }

        internal static void CleanupIn(ref IntPtr to)
        {
            // Read the buffer pointer containing the sequence content and de-allocate it.
            os.free(to);
            to = IntPtr.Zero;
        }

        internal void CopyOut(ref byte[] to, int length)
        {
            CopyOut(userPtr, ref to, length);
        }

        internal static void CopyOut(IntPtr from, ref byte[] to, int length)
        {
            // Initialize managed array to the correct size.
            if (to == null || to.Length != length)
            {
                to = new byte[length];
            }

            if (length > 0)
            {
                // Read the buffer pointer containing the sequence content
                Marshal.Copy(from, to, 0, length);
            }
        }
    }

    internal class SequenceStringMarshaler : BaseMarshaler, IDisposable
    {
        protected bool cleanupRequired = false;
        private IntPtr userPtr;

        public SequenceStringMarshaler()
        {
            userPtr = IntPtr.Zero;
        }

        public SequenceStringMarshaler(IntPtr nativePtr)
        {
            userPtr = nativePtr;
        }

        public IntPtr UserPtr
        {
            get { return userPtr; }
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(ref userPtr);
            }
        }

        internal DDS.ReturnCode CopyIn(string[] from)
        {
            cleanupRequired = true;
            return CopyIn(from, ref userPtr);
        }

        internal static DDS.ReturnCode CopyIn(string[] from, ref IntPtr to)
        {
            int i = 0, totLen = 0, fromLen = 0;
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            IntPtr index;

            // guard against null
            if (from != null) fromLen = from.Length;
            for (i = 0; i < fromLen; i++)
            {
                // Determine the Length of the array.
                totLen += from[i].Length;
            }

            // Total string length + delimitters (i - 1) + terminator '\0'.
            if (totLen > 0)
            {
                to = os.malloc(new IntPtr(totLen + i));
                if (to != IntPtr.Zero) {
                    int strLen = from[0].Length;
                    index = to;
                    Marshal.Copy(Encoding.ASCII.GetBytes(from[0]), 0, index, strLen);
                    for (i = 1; i < from.Length; i++)
                    {
                        Write(index, strLen, ',');
                        index = new IntPtr(index.ToInt64() + strLen + 1);
                        strLen = from[i].Length;
                        Marshal.Copy(Encoding.ASCII.GetBytes(from[i]), 0, index, strLen);
                    }
                    Write(index, strLen, '\0');
                }
                else
                {
                   result = DDS.ReturnCode.OutOfResources;
                }
            }
            else
            {
                // When no partitions, write just the '\0' terminator.
                to = os.malloc(new IntPtr(1));
                Write(to, 0, '\0');
            }

            return result;
        }

        internal static void CleanupIn(ref IntPtr to)
        {
            // Set _max field.
            os.free(to);
            to = IntPtr.Zero;
        }

        internal void CopyOut(ref string[] to)
        {
            CopyOut(userPtr, ref to);
        }

        internal static void CopyOut(IntPtr from, ref string[] to)
        {
            string partitionList = null;
            if (from != IntPtr.Zero)
            {
                partitionList = ReadString(from);
            }

            if (String.IsNullOrEmpty(partitionList))
            {
                to = new string[0];
            }
            else
            {
                to = partitionList.Split(',');
            }
        }
    }

    internal class SequenceStringToArrMarshaler : BaseMarshaler, IDisposable
    {
        protected bool cleanupRequired = false;
        private IntPtr userPtr;
        private int nrElements;

        public SequenceStringToArrMarshaler()
        {
            userPtr = IntPtr.Zero;
            nrElements = 0;
        }

        public SequenceStringToArrMarshaler(IntPtr nativePtr, int size)
        {
            userPtr = nativePtr;
            nrElements = size;
        }

        public IntPtr UserPtr
        {
            get { return userPtr; }
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(ref userPtr, nrElements);
            }
        }

        internal DDS.ReturnCode CopyIn(string[] from)
        {
            ReturnCode result = DDS.ReturnCode.Ok;

            if (from != null)
            {
                cleanupRequired = true;
                nrElements = from.Length;
                result = CopyIn(from, ref userPtr);
            }
            return result;
        }

        internal static DDS.ReturnCode CopyIn(string[] from, ref IntPtr to)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            IntPtr element = IntPtr.Zero;


            if (from.Length > 0) {
               // Number of elements times the size of IntPtr.
	            to = os.malloc(new IntPtr(from.Length * IntPtr.Size));
	            if (to != IntPtr.Zero)
	            {
	                for (int i = 0, offset = 0; i < from.Length; i++, offset += IntPtr.Size)
	                {
	                    WriteString(ref element, from[i]);
	                    Write(to, offset, element);
	                }
	            }
	            else
	            {
	                result = DDS.ReturnCode.OutOfResources;
	            }
            }

            return result;
        }

        internal static void CleanupIn(ref IntPtr to, int size)
        {
            for (int i = 0, offset = 0; i < size; i++, offset += IntPtr.Size)
            {
                IntPtr element = ReadIntPtr(to, offset);
                ReleaseString(ref element);
            }
            os.free(to);
            to = IntPtr.Zero;
        }

        internal void CopyOut(ref string[] to)
        {
            CopyOut(userPtr, ref to, nrElements);
        }

        internal static void CopyOut(IntPtr from, ref string[] to, int size)
        {
            to = new string[size];
            for (int i = 0, offset = 0; i < size; i++, offset += IntPtr.Size)
            {
                IntPtr element = ReadIntPtr(from, offset);
                to[i] = ReadString(element);
            }
        }
    }

    internal class SequenceStringToCValueArrMarshaler : BaseMarshaler, IDisposable
    {
        private static Type type = typeof(c_value);
        private static int CValueSize = Marshal.SizeOf(type);
        private static int OffsetString = (int) Marshal.OffsetOf(type, "WString");

        protected bool cleanupRequired = false;
        private IntPtr userPtr;
        private int nrElements;

        public SequenceStringToCValueArrMarshaler()
        {
            userPtr = IntPtr.Zero;
            nrElements = 0;
        }

        public SequenceStringToCValueArrMarshaler(IntPtr nativePtr, int size)
        {
            userPtr = nativePtr;
            nrElements = size;
        }

        public IntPtr UserPtr
        {
            get { return userPtr; }
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(ref userPtr, nrElements);
            }
        }

        internal DDS.ReturnCode CopyIn(string[] from)
        {
            ReturnCode result = DDS.ReturnCode.Ok;

            if (from != null)
            {
                cleanupRequired = true;
                nrElements = from.Length;
                result = CopyIn(from, ref userPtr);
            }
            return result;
        }

        internal static DDS.ReturnCode CopyIn(string[] from, ref IntPtr to)
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            IntPtr element = IntPtr.Zero;

            if (from.Length > 0) {
	            to = os.malloc(new IntPtr(from.Length * CValueSize));
	            if (to != IntPtr.Zero)
	            {
	                for (int i = 0, offset = 0; i < from.Length; i++, offset += CValueSize)
	                {
	                    Write(to, offset, (int) c_valueKind.V_STRING);
	                    WriteString(ref element, from[i]);
	                    Write(to, offset + OffsetString, element);
	                }
	            }
	            else
	            {
	                result = DDS.ReturnCode.OutOfResources;
	            }
            }

            return result;
        }

        internal static void CleanupIn(ref IntPtr to, int size)
        {
            for (int i = 0, offset = 0; i < size; i++, offset += CValueSize)
            {
                IntPtr element = ReadIntPtr(to, offset + OffsetString);
                ReleaseString(ref element);
            }
            os.free(to);
            to = IntPtr.Zero;
        }

        internal void CopyOut(ref string[] to)
        {
            CopyOut(userPtr, ref to, nrElements);
        }

        internal static void CopyOut(IntPtr from, ref string[] to, int size)
        {
            to = new string[size];
            for (int i = 0, offset = 0; i < size; i++, offset += CValueSize)
            {
                if ((c_valueKind) ReadInt32(from, offset) == c_valueKind.V_STRING)
                {
                    IntPtr element = ReadIntPtr(from, offset + OffsetString);
                    to[i] = ReadString(element);
                }
                else
                {
                    to[i] = null;
                }
            }
        }
    }
}
