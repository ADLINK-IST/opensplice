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
using System.Collections.Generic;
using System.Runtime.InteropServices;
using DDS;

namespace DDS.OpenSplice
{
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate bool SampleCopyInDelegate(IntPtr basePtr, IntPtr from, IntPtr to);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void FakeSampleCopyOutDelegate(IntPtr from, IntPtr to, int offset);
    public delegate void SampleCopyOutDelegate(IntPtr from, ref object to, int offset);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void SampleReaderCopyDelegate(IntPtr samples, IntPtr info);
    public delegate object[] SampleReaderAllocDelegate(int length);

    public abstract class TypeSupport : SacsSuperClass, ITypeSupport
    {
        // cache these types for performance
        private static Type copyInType = typeof(SampleCopyInDelegate);
        private static Type copyOutType = typeof(SampleCopyOutDelegate);
        private static Type readerCopyType = typeof(SampleReaderCopyDelegate);
        private static Type allocType = typeof(SampleReaderAllocDelegate);

        private SampleCopyInDelegate copyInDelegate;
        private SampleCopyOutDelegate copyOutDelegate;
        private FakeSampleCopyOutDelegate fakeCopyOutDelegate;
        private SampleReaderCopyDelegate readerCopyDelegate;
        private SampleReaderAllocDelegate readerAllocDelegate;

        public TypeSupport(
            string typeName,
            string keyList,
            string metaDescriptor,
            SampleCopyInDelegate copyInDelegate,
            SampleCopyOutDelegate copyOutDelegate,
            SampleReaderAllocDelegate readerAllocDelegate)
        {
            // Keeps these delegates alive and away from the GC

            this.copyInDelegate = copyInDelegate;
            this.copyOutDelegate = copyOutDelegate;
            this.readerAllocDelegate = readerAllocDelegate;
            this.readerCopyDelegate = new SampleReaderCopyDelegate(ReaderCopy);

            // HACK: This is a Mono workaround. Currently you cannot marshal a delegate
            // to a function pointer if the parameters include an "object" type. So we
            // will pass a fake delegate, and then internally use the real copyOut, this
            // may give better performance anyways, since we won't have to convert the
            // IntPtr to a Delegate for every ReaderCopy invocation.
            this.fakeCopyOutDelegate = new FakeSampleCopyOutDelegate(FakeCopyOut);

            IntPtr ptr = Gapi.FooTypeSupport.alloc(
               typeName,
               keyList,
               metaDescriptor,
               IntPtr.Zero,        /* type_load */
               copyInDelegate,             /* copyIn: copy from C# types */
               fakeCopyOutDelegate,            /* copyOut: copy to C# types */
               0,                  /* alloc_size */
               IntPtr.Zero,        /* alloc buffer */
               IntPtr.Zero,        /* writer copy */
               readerCopyDelegate,         /* reader copy */
               IntPtr.Zero,        /* create datawriter */
               IntPtr.Zero);

            // Base class handles everything.
            base.SetPeer(ptr);
        }

        public virtual ReturnCode RegisterType(IDomainParticipant participant, string typeName)
        {
            DomainParticipant domainObj = (DomainParticipant)participant;
            ReturnCode result = Gapi.FooTypeSupport.register_type(
                GapiPeer,
                domainObj.GapiPeer,
                typeName);

            // TODO: Is this Type_Name actually going to be different after 
            // register_type is called?
            InitType(domainObj.GapiPeer);

            return result;
        }

        /*public IntPtr GetMetaPtr(IDomainParticipant participant, string typeName)
        {
            DomainParticipant domainObj = (DomainParticipant)participant;
            return domainObj.GetTypeMetaDescription(typeName);
        }*/

        public virtual string TypeName
        {
            get
            {
                IntPtr ptr = Gapi.TypeSupport.get_type_name(GapiPeer);
                string result = Marshal.PtrToStringAnsi(ptr);
                Gapi.GenericAllocRelease.Free(ptr);

                return result;
            }
        }

        public string Description
        {
            get
            {
                IntPtr ptr = Gapi.TypeSupport.get_description(GapiPeer);
                string result = Marshal.PtrToStringAnsi(ptr);
                Gapi.GenericAllocRelease.Free(ptr);

                return result;
            }
        }

        public string KeyList
        {
            get
            {
                IntPtr ptr = Gapi.TypeSupport.get_key_list(GapiPeer);
                string result = Marshal.PtrToStringAnsi(ptr);
                Gapi.GenericAllocRelease.Free(ptr);

                return result;
            }
        }

        protected abstract void InitType(IntPtr participant);

        public abstract DataWriter CreateDataWriter(IntPtr gapiPtr);

        public abstract DataReader CreateDataReader(IntPtr gapiPtr);

        public void ReaderCopy(IntPtr samples, IntPtr info)
        {
            if (samples != IntPtr.Zero)
            {
                bool sampleDataArrayChanged = false;
                bool sampleInfoArrayChanged = false;

                DataSample[] dataSamples;
                OpenSplice.CustomMarshalers.SequenceDataSampleMarshaler.CopyOut(samples, out dataSamples, 0);

                OpenSplice.Gapi.gapi_readerInfo readerInfo;
                CustomMarshalers.ReaderInfoMarshaler.CopyOut(info, out readerInfo);

                // TODO: We don't understand why num_samples is zero all the time
                readerInfo.num_samples = dataSamples.Length;

                // See the "Hack"comment in the ctor
                //SampleCopyOutDelegate copyOutDelegate = (SampleCopyOutDelegate)Marshal.GetDelegateForFunctionPointer(
                //    readerInfo.copy_out, copyOutType);

                // grab the data and sample arrays
                IntPtr dataBufferPtr = Marshal.ReadIntPtr(readerInfo.data_buffer);
                GCHandle tmpGCHandleData = GCHandle.FromIntPtr(dataBufferPtr);
                object[] sampleDataArray = tmpGCHandleData.Target as object[];

                IntPtr infoBufferPtr = Marshal.ReadIntPtr(readerInfo.info_buffer);
                GCHandle tmpGCHandleInfo = GCHandle.FromIntPtr(infoBufferPtr);
                SampleInfo[] sampleInfoArray = tmpGCHandleInfo.Target as SampleInfo[];

                int samplesToRead = readerInfo.num_samples;
                if (readerInfo.max_samples != Length.Unlimited)
                {
                    samplesToRead = Math.Min(readerInfo.max_samples, readerInfo.num_samples);
                }

                if (sampleDataArray == null || sampleDataArray.Length == 0)
                {
                    sampleDataArray = this.readerAllocDelegate(samplesToRead);
                    sampleDataArrayChanged = true;
                }

                if (sampleInfoArray == null || sampleInfoArray.Length == 0)
                {
                    sampleInfoArray = new SampleInfo[samplesToRead];
                    sampleInfoArrayChanged = true;

                }

                // TODO: This should have been tested already in the FooDataReader
                //if ((sampleDataArray.Length < samplesToRead) ||
                //    (sampleInfoArray.Length < samplesToRead))
                //{
                //    // TODO: how to setup return codes in readercopy
                //    //send: ReturnCode.PreconditionNotMet ??
                //    return;
                //}

                // copy the data into our sampleDataArray and sampleInfoArray
                int index = 0;
                for (; index < dataSamples.Length; index++)
                {
                    // copy the sample info
                    sampleInfoArray[index] = dataSamples[index].sampleInfo;

                    object sampleObj = sampleDataArray[index];
                    //copyOutDelegate(dataSamples[index].data, ref sampleObj);
                    this.copyOutDelegate(dataSamples[index].data, ref sampleObj, 0);
                    sampleDataArray[index] = sampleObj;
                }

                // now at the end of data need to clean the end of the array
                for (int dataIndex = index; dataIndex < sampleDataArray.Length; dataIndex++)
                {
                    sampleDataArray[dataIndex] = null;
                }

                for (int infoIndex = index; infoIndex < sampleInfoArray.Length; infoIndex++)
                {
                    sampleInfoArray[infoIndex] = null;
                }

                if (sampleDataArrayChanged)
                {
                    GCHandle tmpGCHandle = GCHandle.Alloc(sampleDataArray, GCHandleType.Normal);
                    IntPtr tmpPtr = GCHandle.ToIntPtr(tmpGCHandle);
                    Marshal.WriteIntPtr(readerInfo.data_buffer, tmpPtr);
                }

                if (sampleInfoArrayChanged)
                {
                    GCHandle tmpGCHandle = GCHandle.Alloc(sampleInfoArray, GCHandleType.Normal);
                    IntPtr tmpPtr = GCHandle.ToIntPtr(tmpGCHandle);
                    Marshal.WriteIntPtr(readerInfo.info_buffer, tmpPtr);
                }
            }
        }

        public void FakeCopyOut(IntPtr from, IntPtr to, int offset)
        { }

        public static int[] GetOffsets(IntPtr participant, string typeName)
        {
            // Get the meta-data description.
            IntPtr metaPtr = Gapi.DomainParticipant.get_type_metadescription(participant, typeName);

            // copy the ptr into the c# structure
            OpenSplice.Gapi.c_structure dbType = Marshal.PtrToStructure(metaPtr, typeof(OpenSplice.Gapi.c_structure))
                as OpenSplice.Gapi.c_structure;

            //Console.WriteLine("GetOffsets: dbType.members: {0}", dbType.members);
            //Console.WriteLine("GetOffsets: dbType.objectCount: {0}", dbType.objectCount);

            int memberCount = OpenSplice.Gapi.DDSDatabase.arraySize(dbType.members);
            //Console.WriteLine("GetOffsets: dbType.members Count: {0}", memberCount);

            /*
                        // arraySize gives the member count, but v 3.4.3 it is defined as a macro, 
                        // not a function so cannot p/invoke
            
                        //==== start of version 3.4.3 workaround
                        IntPtr typeMembers = OpenSplice.Gapi.DDSDatabase.getType(dbType.members);
                        OpenSplice.Gapi.c_collectionType dbcollectionType = Marshal.PtrToStructure(typeMembers, typeof(OpenSplice.Gapi.c_collectionType))
                            as OpenSplice.Gapi.c_collectionType;
                        string teststr = Marshal.PtrToStringAnsi(dbcollectionType.name);
                        //Console.WriteLine("GetOffsets: teststr: {0}", teststr);
                        int memberCount = dbcollectionType.maxSize;
                        //==== end of version 3.4.3 workaround
            */
            //if (memberCount != expectedMemberCount) { /* Assert!!!! */ }

            int[] offsets = new int[memberCount];
            string[] names = new string[memberCount];
            //List<KeyValuePair<string, int>> offsetList = new List<KeyValuePair<string, int>>(memberCount);
            int ptrSize = Marshal.SizeOf(typeof(IntPtr));

            // loop through all members
            for (int index = 0; index < memberCount; index++)
            {
                // calculate the ptr in the member array
                IntPtr memberArrayPtr = new IntPtr(dbType.members.ToInt64() + (ptrSize * index));

                // read the ptr to get to the ptr of the member, aka dereference
                IntPtr memberPtr = Marshal.ReadIntPtr(memberArrayPtr);

                // copy ptr to c# structure
                OpenSplice.Gapi.c_member dbMember = Marshal.PtrToStructure(memberPtr, typeof(OpenSplice.Gapi.c_member))
                    as OpenSplice.Gapi.c_member;
                offsets[index] = dbMember.offset.ToInt32();
                names[index] = Marshal.PtrToStringAnsi(dbMember.name);

                //Console.WriteLine("GetOffsets: [index]: {0}", index);
                //Console.WriteLine("GetOffsets: offsets[index]: {0}", offsets[index]);
                //Console.WriteLine("GetOffsets: names[index]: {0}", names[index]);

                //int offset = dbMember.offset.ToInt32();
                //string name = Marshal.PtrToStringAnsi(dbMember.name);
                //offsetList.Add(new KeyValuePair<string, int>(name, offset));
            }

            return offsets;
        }

        #region Writers
        public static void Write(IntPtr to, int offset, long from)
        {
            Marshal.WriteInt64(to, offset, from);
        }

        public static void Write(IntPtr to, int offset, ulong from)
        {
            Marshal.WriteInt64(to, offset, (long)from);
        }

        public static void Write(IntPtr to, int offset, int from)
        {
            Marshal.WriteInt32(to, offset, from);
        }

        public static void Write(IntPtr to, int offset, uint from)
        {
            Marshal.WriteInt32(to, offset, (int)from);
        }

        public static void Write(IntPtr to, int offset, short from)
        {
            Marshal.WriteInt16(to, offset, from);
        }

        public static void Write(IntPtr to, int offset, ushort from)
        {
            Marshal.WriteInt16(to, offset, (short)from);
        }

        public static void Write(IntPtr to, int offset, bool from)
        {
            Marshal.WriteByte(to, offset, (byte)(from ? 1 : 0));
        }

        public static void Write(IntPtr to, int offset, byte from)
        {
            Marshal.WriteByte(to, offset, from);
        }

        public static void Write(IntPtr to, int offset, char from)
        {
            Marshal.WriteByte(to, offset, (byte)from);
        }

        [StructLayout(LayoutKind.Explicit)]
        private struct DoubleToLong
        {
            [FieldOffset(0)]
            public double theDouble;
            [FieldOffset(0)]
            public long theLong;
        }

        public static void Write(IntPtr to, int offset, double from)
        {
            DoubleToLong dtl = new DoubleToLong();
            dtl.theDouble = from;
            Marshal.WriteInt64(to, offset, dtl.theLong);
        }

        [StructLayout(LayoutKind.Explicit)]
        private struct SingleToInt
        {
            [FieldOffset(0)]
            public float theSingle;
            [FieldOffset(0)]
            public int theInt;
        }

        public static void Write(IntPtr to, int offset, float from)
        {
            SingleToInt sti = new SingleToInt();
            sti.theSingle = from;
            Marshal.WriteInt32(to, offset, sti.theInt);
        }

        public static void Write(IntPtr to, int offset, Duration from)
        {
            Marshal.WriteInt32(to, offset, from.Sec);
            Marshal.WriteInt32(to, offset + 4, (int)from.NanoSec);
        }

        public static void Write(IntPtr to, int offset, Time from)
        {
            Marshal.WriteInt32(to, offset, from.Sec);
            Marshal.WriteInt32(to, offset + 4, (int)from.NanoSec);
        }

        public static void Write(IntPtr to, int offset, InstanceHandle from)
        {
            Marshal.WriteInt64(to, offset, from);
        }

        public static void Write(IntPtr basePtr, IntPtr to, int offset, ref string from)
        {
            IntPtr strPtr = Gapi.DDSDatabase.stringNew(basePtr, from);
            Marshal.WriteIntPtr(to, offset, strPtr);
        }

        public static void Write(IntPtr basePtr, IntPtr to, int offset, ref object[] from)
        {
            //IntPtr strPtr = Gapi.DDSDatabase.stringNew(basePtr, from);
            //Marshal.WriteIntPtr(to, offset, strPtr);
        }

        public static void Write(IntPtr to, int offset, IntPtr from)
        {
            Marshal.WriteIntPtr(to, offset, from);
        }

        #endregion

        #region Readers
        public static long ReadInt64(IntPtr from, int offset)
        {
            return Marshal.ReadInt64(from, offset);
        }

        public static ulong ReadUInt64(IntPtr from, int offset)
        {
            return (ulong)Marshal.ReadInt64(from, offset);
        }

        public static int ReadInt32(IntPtr from, int offset)
        {
            return Marshal.ReadInt32(from, offset);
        }

        public static uint ReadUInt32(IntPtr from, int offset)
        {
            return (uint)Marshal.ReadInt32(from, offset);
        }

        public static short ReadInt16(IntPtr from, int offset)
        {
            return Marshal.ReadInt16(from, offset);
        }

        public static ushort ReadUInt16(IntPtr from, int offset)
        {
            return (ushort)Marshal.ReadInt16(from, offset);
        }

        public static bool ReadBoolean(IntPtr from, int offset)
        {
            return Marshal.ReadByte(from, offset) != 0;
        }

        public static byte ReadByte(IntPtr from, int offset)
        {
            return Marshal.ReadByte(from, offset);
        }

        public static char ReadChar(IntPtr from, int offset)
        {
            return (char)Marshal.ReadByte(from, offset);
        }

        public static double ReadDouble(IntPtr from, int offset)
        {
            DoubleToLong dtl = new DoubleToLong();
            dtl.theLong = Marshal.ReadInt64(from, offset);
            return dtl.theDouble;
        }

        public static float ReadSingle(IntPtr from, int offset)
        {
            SingleToInt sti = new SingleToInt();
            sti.theInt = Marshal.ReadInt32(from, offset);
            return sti.theSingle;
        }

        public static Duration ReadDuration(IntPtr from, int offset)
        {
            return new Duration(Marshal.ReadInt32(from, offset),
                (uint)Marshal.ReadInt32(from, offset + 4));
        }

        public static Time ReadTime(IntPtr from, int offset)
        {
            return new Time(Marshal.ReadInt32(from, offset),
                (uint)Marshal.ReadInt32(from, offset + 4));
        }

        public static InstanceHandle ReadInstanceHandle(IntPtr from, int offset)
        {
            return new InstanceHandle(Marshal.ReadInt64(from, offset));
        }

        public static string ReadString(IntPtr from, int offset)
        {
            IntPtr stringPtr = Marshal.ReadIntPtr(new IntPtr(from.ToInt64() + offset));
            return Marshal.PtrToStringAnsi(stringPtr);
        }

        public static object[] ReadArray(IntPtr basePtr, IntPtr from, int offset)
        {
            throw new NotImplementedException();
        }

        public static IntPtr ReadIntPtr(IntPtr from, int offset)
        {
            return Marshal.ReadIntPtr(from, offset);
        }

        #endregion


		public static IntPtr GetArrayType(IntPtr basePtr, string arrayType, string subType, int maxLength)
		{
			//subtype0 = c_type(c_metaResolve(c_metaObject(base), "c_long"));
			//type0 = c_metaArrayTypeNew(c_metaObject(base), "C_SEQUENCE<c_long>", subtype0, 0);
			//c_free(subtype0);

			IntPtr subPtr = Gapi.DDSDatabase.metaResolve(basePtr, subType);
			IntPtr arrayPtr = Gapi.DDSDatabase.metaArrayTypeNew(basePtr, arrayType, subPtr, maxLength);
			Gapi.DDSDatabase.free(subPtr);

			return arrayPtr;
		}

		public static IntPtr NewArray(IntPtr dataType, int size)
		{
			return Gapi.DDSDatabase.newArray(dataType, size);
		}

		public static int ArraySize(IntPtr arrayPtr)
		{
			return Gapi.DDSDatabase.arraySize(arrayPtr);
		}
	}
}
