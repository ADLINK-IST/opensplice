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
    internal class QosPolicyCountSequenceMarshaler
    {
        private static readonly Type seqType = typeof(OpenSplice.Gapi.gapi_Seq);
        public static readonly int seqSize = Marshal.SizeOf(seqType);
        private static readonly int offset_seq_maximum = (int)Marshal.OffsetOf(seqType, "_maximum");
        private static readonly int offset_seq_length = (int)Marshal.OffsetOf(seqType, "_length");
        private static readonly int offset_seq_buffer = (int)Marshal.OffsetOf(seqType, "_buffer");
        private static readonly int offset_seq_release = (int)Marshal.OffsetOf(seqType, "_release");

        private static readonly Type dataType = typeof(OpenSplice.Gapi.gapi_qosPolicyCount);
        public static readonly int dataSize = Marshal.SizeOf(dataType);
        private static readonly int offset_policy_id = (int)Marshal.OffsetOf(dataType, "policy_id");
        private static readonly int offset_count = (int)Marshal.OffsetOf(dataType, "count");

        static public void CopyOut(Gapi.gapi_Seq from, ref QosPolicyCount[] to)
        {
            // Initialize managed array to the correct size.
            if (to.Length != from._length)
            {
                to = new QosPolicyCount[from._length];
            }

            for (int index = 0; index < from._length; index++)
            {
                to[index].PolicyId = (QosPolicyId)BaseMarshaler.ReadInt32(from._buffer, (dataSize * index) + offset_policy_id);
                to[index].Count = BaseMarshaler.ReadInt32(from._buffer, (dataSize * index) + offset_count);
            }
        }
    }

    internal class SequenceIntPtrMarshaler : IMarshaler
    {
        public static readonly int Size = Marshal.SizeOf(typeof(System.IntPtr));
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_Seq);
        public static readonly int StructSize = Marshal.SizeOf(type);

        private static readonly int offset__maximum = (int)Marshal.OffsetOf(type, "_maximum");
        private static readonly int offset__length = (int)Marshal.OffsetOf(type, "_length");
        private static readonly int offset__buffer = (int)Marshal.OffsetOf(type, "_buffer");
        private static readonly int offset__release = (int)Marshal.OffsetOf(type, "_release");

        private bool cleanupRequired = false;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public SequenceIntPtrMarshaler(IntPtr[] from)
            : this()
        {
            CopyIn(from, gapiPtr, 0);
        }

        public SequenceIntPtrMarshaler()
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.sequence_alloc();
        }

        public void Dispose()
        {
            if (cleanupRequired) 
            {
                CleanupIn(gapiPtr, 0);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
        }

        internal void CopyIn(IntPtr[] from)
        {
            cleanupRequired = true;
            CopyIn(from, gapiPtr, 0);
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
                // Allocate a buffer containing the sequence content starting at element 0 up til the full Length of the array.
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
            CopyOut(gapiPtr, ref to, 0);
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
    }

    internal class SequenceOctetMarshaler : IMarshaler
    {
        public static readonly int Size = Marshal.SizeOf(typeof(System.Byte));
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_Seq);
        public static readonly int StructSize = Marshal.SizeOf(type);

        private static readonly int offset__maximum = (int)Marshal.OffsetOf(type, "_maximum");
        private static readonly int offset__length = (int)Marshal.OffsetOf(type, "_length");
        private static readonly int offset__buffer = (int)Marshal.OffsetOf(type, "_buffer");
        private static readonly int offset__release = (int)Marshal.OffsetOf(type, "_release");

        private bool cleanupRequired = false;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public SequenceOctetMarshaler(byte[] from)
            : this()
        {
            cleanupRequired = true;
            CopyIn(from, gapiPtr, 0);
        }

        public SequenceOctetMarshaler()
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.sequence_alloc();
        }

        public void Dispose()
        {
            if (cleanupRequired) 
            {
                CleanupIn(gapiPtr, 0);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
        }

        internal static void CopyIn(byte[] from, IntPtr to, int offset)
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
                // Allocate a buffer containing the sequence content starting at element 0 up til the full Length of the array.
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

        internal void CopyOut(ref byte[] to)
        {
            CopyOut(gapiPtr, ref to, 0);
        }

        internal static void CopyOut(IntPtr from, ref byte[] to, int offset)
        {
            // Get _length field
            int length = BaseMarshaler.ReadInt32(from, offset + offset__length);

            // Initialize managed array to the correct size.
            if (to == null || to.Length != length)
            {
                to = new byte[length];
            }

            if (length > 0)
            {
                // Read the buffer pointer containing the sequence content
                IntPtr arrayPtr = BaseMarshaler.ReadIntPtr(from, offset + offset__buffer);

                for (int index = 0; index < length; index++)
                {
                    // Loop through the string pointers, deallocating each
                    to[index] = BaseMarshaler.ReadByte(arrayPtr, Size * index);
                }
            }
        }
    }

    internal class SequenceStringMarshaler : IMarshaler
    {
        public static readonly int Size = Marshal.SizeOf(typeof(System.IntPtr));
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_Seq);
        public static readonly int StructSize = Marshal.SizeOf(type);

        private static readonly int offset__maximum = (int)Marshal.OffsetOf(type, "_maximum");
        private static readonly int offset__length = (int)Marshal.OffsetOf(type, "_length");
        private static readonly int offset__buffer = (int)Marshal.OffsetOf(type, "_buffer");
        private static readonly int offset__release = (int)Marshal.OffsetOf(type, "_release");

        private bool cleanupRequired = false;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public SequenceStringMarshaler()
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.sequence_alloc();
        }

        public void Dispose()
        {
            if (cleanupRequired) 
            {
                CleanupIn(gapiPtr, 0);
            }
            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
        }

        internal DDS.ReturnCode CopyIn(string[] from)
        {
            cleanupRequired = true;
            return CopyIn(from, gapiPtr, 0);
        }
        
        internal static DDS.ReturnCode CopyIn(string[] from, IntPtr to, int offset)
        {
            Int32 length = 0;
            IntPtr arrayPtr = IntPtr.Zero;

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

            if (length > 0)
            {
                // Allocate a buffer containing the sequence content starting at element 0 up til the full Length of the array.
                arrayPtr = OpenSplice.Gapi.GenericAllocRelease.Alloc(Size * length);
                for (int index = 0; index < length; index++)
                {
                    if (from[index] == null) return DDS.ReturnCode.BadParameter;
                    IntPtr stringPtr = Marshal.StringToHGlobalAnsi(from[index]);
                    BaseMarshaler.Write(arrayPtr, Size * index, stringPtr);
                }
            }

            // _buffer field: set the pointer to the appropriate location.
            BaseMarshaler.Write(to, offset + offset__buffer, arrayPtr);

            // Set _release field
            BaseMarshaler.Write(to, offset + offset__release, (byte)1);
            
            return DDS.ReturnCode.Ok;
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
            // Set _max field.
            int length = BaseMarshaler.ReadInt32(nativePtr, offset + offset__length);

            // Read the buffer pointer containing the sequence content
            IntPtr arrayPtr = BaseMarshaler.ReadIntPtr(nativePtr, offset + offset__buffer);

            for (int index = 0; index < length; index++)
            {
                // Loop through the string pointers, deallocating each
                IntPtr stringPtr = BaseMarshaler.ReadIntPtr(arrayPtr, Size * index);
                //OpenSplice.Gapi.GenericAllocRelease.Free(stringPtr);
                Marshal.FreeHGlobal(stringPtr);
                BaseMarshaler.Write(arrayPtr, Size * index, IntPtr.Zero);
            }

            // free array
            OpenSplice.Gapi.GenericAllocRelease.Free(arrayPtr);
        }

        internal void CopyOut(ref string[] to)
        {
            CopyOut(gapiPtr, ref to, 0);
        }

        internal static void CopyOut(IntPtr from, ref string[] to, int offset)
        {
            // Get _length field
            int length = BaseMarshaler.ReadInt32(from, offset + offset__length);

            // Initialize managed array to the correct size.
            if (to == null || to.Length != length)
            {
                to = new string[length];
            }

            if (length > 0)
            {
                // Read the buffer pointer containing the sequence content
                IntPtr arrayPtr = BaseMarshaler.ReadIntPtr(from, offset + offset__buffer);

                for (int index = 0; index < length; index++)
                {
                    // Loop through the string pointers, deallocating each
                    IntPtr stringPtr = BaseMarshaler.ReadIntPtr(arrayPtr, Size * index);
                    to[index] = Marshal.PtrToStringAnsi(stringPtr);
                }
            }
        }
    }

    internal class SequenceInstanceHandleMarshaler : IMarshaler
    {
        public static readonly int Size = Marshal.SizeOf(typeof(System.Int64));
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_Seq);
        public static readonly int StructSize = Marshal.SizeOf(type);

        private static readonly int offset__maximum = (int)Marshal.OffsetOf(type, "_maximum");
        private static readonly int offset__length = (int)Marshal.OffsetOf(type, "_length");
        private static readonly int offset__buffer = (int)Marshal.OffsetOf(type, "_buffer");
        private static readonly int offset__release = (int)Marshal.OffsetOf(type, "_release");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public SequenceInstanceHandleMarshaler(InstanceHandle[] from)
            : this()
        {
            cleanupRequired = true;
            CopyIn(from, gapiPtr, 0);
        }

        public SequenceInstanceHandleMarshaler()
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.sequence_alloc();
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(gapiPtr, 0);
            }

            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
        }

        internal static void CopyIn(InstanceHandle[] from, IntPtr to, int offset)
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

            // Allocate a buffer containing the sequence content starting at element 0 up til the full Length of the array.
            IntPtr arrayPtr = OpenSplice.Gapi.GenericAllocRelease.Alloc(Size * length);
            //IntPtr arrayPtr = Marshal.AllocHGlobal(Size * length);
            for (int index = 0; index < length; index++)
            {
                BaseMarshaler.Write(arrayPtr, Size * index, from[index]);
            }

            // _buffer field: set the pointer to the appropriate location.
            BaseMarshaler.Write(to, offset + offset__buffer, arrayPtr);

            // Set _release field
            BaseMarshaler.Write(to, offset + offset__release, (byte)1);
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
            // Read the buffer pointer containing the sequence content and de-allocate it.
            IntPtr arrayPtr = BaseMarshaler.ReadIntPtr(nativePtr, offset + offset__buffer);
            OpenSplice.Gapi.GenericAllocRelease.Free(arrayPtr);
            BaseMarshaler.Write(nativePtr, offset + offset__buffer, IntPtr.Zero);
        }

        internal void CopyOut(ref InstanceHandle[] to)
        {
            CopyOut(gapiPtr, ref to, 0);
        }

        internal static void CopyOut(IntPtr from, ref InstanceHandle[] to, int offset)
        {
            // Get _length field
            int length = BaseMarshaler.ReadInt32(from, offset + offset__length);

            // Initialize managed array to the correct size.
            // Initialize managed array to the correct size.
            if (to == null || to.Length != length)
            {
                to = new InstanceHandle[length];
            }

            if (length > 0)
            {
                // Read the buffer pointer containing the sequence content
                IntPtr arrayPtr = BaseMarshaler.ReadIntPtr(from, offset + offset__buffer);

                for (int index = 0; index < length; index++)
                {
                    // Loop through the string pointers, deallocating each
                    long handle = BaseMarshaler.ReadInt64(arrayPtr, Size * index);
                    to[index] = handle;
                }
            }
        }
    }

    internal class SequenceMarshaler<TInterface, T> : IMarshaler
        where T : SacsSuperClass, TInterface
    //where TInterface : IEntity
    {
        public static readonly int Size = Marshal.SizeOf(typeof(System.IntPtr));
        private static readonly Type type = typeof(OpenSplice.Gapi.gapi_Seq);
        public static readonly int StructSize = Marshal.SizeOf(type);

        private static readonly int offset__maximum = (int)Marshal.OffsetOf(type, "_maximum");
        private static readonly int offset__length = (int)Marshal.OffsetOf(type, "_length");
        private static readonly int offset__buffer = (int)Marshal.OffsetOf(type, "_buffer");
        private static readonly int offset__release = (int)Marshal.OffsetOf(type, "_release");

        private bool cleanupRequired;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public SequenceMarshaler(TInterface[] from)
            : this()
        {
            cleanupRequired = true;
            CopyIn(from, gapiPtr, 0);
        }

        public SequenceMarshaler()
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.sequence_alloc();
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                CleanupIn(gapiPtr, 0);
            }

            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
        }

        internal static void CopyIn(TInterface[] from, IntPtr to, int offset)
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

            // Allocate a buffer containing the sequence content starting at element 0 up til the full Length of the array.
            IntPtr arrayPtr = OpenSplice.Gapi.GenericAllocRelease.Alloc(Size * length);
            //IntPtr arrayPtr = Marshal.AllocHGlobal(Size * length);
            for (int index = 0; index < length; index++)
            {
                T objectType = (T)from[index];
                BaseMarshaler.Write(arrayPtr, Size * index, objectType == null ? IntPtr.Zero : objectType.GapiPeer);
            }

            // _buffer field: set the pointer to the appropriate location.
            BaseMarshaler.Write(to, offset + offset__buffer, arrayPtr);

            // Set _release field
            BaseMarshaler.Write(to, offset + offset__release, (byte)1);
        }

        internal static void CleanupIn(IntPtr nativePtr, int offset)
        {
            // Read the buffer pointer containing the sequence content and de-allocate it.
            IntPtr arrayPtr = BaseMarshaler.ReadIntPtr(nativePtr, offset + offset__buffer);
            OpenSplice.Gapi.GenericAllocRelease.Free(arrayPtr);
            BaseMarshaler.Write(nativePtr, offset + offset__buffer, IntPtr.Zero);
        }

        internal void CopyOut(ref TInterface[] to)
        {
            CopyOut(gapiPtr, ref to, 0);
        }

        internal static void CopyOut(IntPtr from, ref TInterface[] to, int offset)
        {
            // Get _length field
            int length = BaseMarshaler.ReadInt32(from, offset + offset__length);

            // Initialize managed array to the correct size.
            // Initialize managed array to the correct size.
            if (to == null || to.Length != length)
            {
                to = new TInterface[length];
            }

            if (length > 0)
            {
                // Read the buffer pointer containing the sequence content
                IntPtr arrayPtr = BaseMarshaler.ReadIntPtr(from, offset + offset__buffer);

                for (int index = 0; index < length; index++)
                {
                    // Loop through the string pointers, deallocating each
                    IntPtr gapiPtr = BaseMarshaler.ReadIntPtr(arrayPtr, Size * index);
                    to[index] = (TInterface)(T)SacsSuperClass.fromUserData(gapiPtr);
                }
            }
        }
    }
}
