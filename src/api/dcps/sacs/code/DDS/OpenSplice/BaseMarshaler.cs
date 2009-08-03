using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

namespace DDS.OpenSplice
{
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate bool SampleCopyInDelegate(IntPtr basePtr, IntPtr from, IntPtr to);

    // HACK: This is a Mono workaround. Currently you cannot marshal a delegate
    // to a function pointer if the parameters include an "object" type. So we
    // will pass a fake delegate, and then internally use the real copyOut, this
    // may give better performance anyways, since we won't have to convert the
    // IntPtr to a Delegate for every ReaderCopy invocation.
    //[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    //public delegate void FakeSampleCopyOutDelegate(IntPtr from, IntPtr to, int offset);

    /* TODO: replace FakeCopyOut as soon as Mono issue mentioned above has been fixed. */
    public delegate void SampleCopyOutDelegate(IntPtr from, ref object to, int offset);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void SampleReaderCopyDelegate(IntPtr samples, IntPtr info);
    
    public abstract class BaseMarshaler
    {
        public static Dictionary<KeyValuePair<IntPtr, string>, BaseMarshaler> typeMarshalers = new Dictionary<KeyValuePair<IntPtr, string>, BaseMarshaler>();
        protected static bool initDone = false;
        
        // Used to instantiate an array of the datatype.
        public abstract object[] SampleReaderAlloc(int length);
        public abstract bool CopyIn(IntPtr basePtr, IntPtr from, IntPtr to);
        public abstract bool CopyIn(IntPtr basePtr, object from, IntPtr to, int offset);
        public abstract void CopyOut(IntPtr from, ref object to, int offset);        

        private SampleCopyInDelegate copyInDelegate;
        private SampleCopyOutDelegate copyOutDelegate;
        private SampleReaderCopyDelegate readerCopyDelegate;

        public SampleCopyInDelegate CopyInDelegate 
        { 
            get
            {
                return copyInDelegate;
            } 
        }
        
        public SampleCopyOutDelegate CopyOutDelegate 
        { 
            get
            {
                return copyOutDelegate;
            }
        }
        
        public SampleReaderCopyDelegate ReaderCopyDelegate 
        { 
            get
            {
                return readerCopyDelegate;
            }
        }

        public BaseMarshaler()
        {
            copyInDelegate = CopyIn;
            copyOutDelegate = CopyOut;
            readerCopyDelegate = ReaderCopy;
        }

        public static BaseMarshaler Create(
                IMarshalerTypeGenerator generator, 
                Type t, 
                string typeName, 
                IntPtr participant)
        {
            BaseMarshaler marshaler;
            
            // Check if a Marshaler for this type already exists, and if not, create it.
            if (!typeMarshalers.TryGetValue(new KeyValuePair<IntPtr, string>(participant, typeName), out marshaler))
            {
                string[] nameArray;
                int[] offsetArray;
            
                // Get the attribute names and offsets of this datatype.
                GetNamesAndOffsets(participant, typeName, out nameArray, out offsetArray);

                // Delegate creation of the marshaler instance to the MarshalerGenerator.
                marshaler = generator.CreateMarshaler(participant, t, typeName, nameArray, offsetArray);

                typeMarshalers.Add(new KeyValuePair<IntPtr, string>(participant, typeName), marshaler);
            }
            return marshaler;
        }

        private static void GetNamesAndOffsets(
                IntPtr participant, 
                string typeName, 
                out string[] names, 
                out int[] offsets)
        {
            // Get the meta-data description.
            IntPtr metaPtr = Gapi.DomainParticipant.get_type_metadescription(participant, typeName);

            // copy the ptr into the c# structure
            OpenSplice.Database.c_structure dbType = Marshal.PtrToStructure(metaPtr, typeof(OpenSplice.Database.c_structure))
                as OpenSplice.Database.c_structure;

            //Console.WriteLine("GetOffsets: dbType.members: {0}", dbType.members);
            //Console.WriteLine("GetOffsets: dbType.objectCount: {0}", dbType.objectCount);

            int memberCount = OpenSplice.Database.c.arraySize(dbType.members);
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

            offsets = new int[memberCount];
            names = new string[memberCount];
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
                OpenSplice.Database.c_member dbMember = Marshal.PtrToStructure(memberPtr, typeof(OpenSplice.Database.c_member))
                    as OpenSplice.Database.c_member;
                offsets[index] = dbMember.offset.ToInt32();
                names[index] = Marshal.PtrToStringAnsi(dbMember.name);

                //Console.WriteLine("GetOffsets: [index]: {0}", index);
                //Console.WriteLine("GetOffsets: offsets[index]: {0}", offsets[index]);
                //Console.WriteLine("GetOffsets: names[index]: {0}", names[index]);

                //int offset = dbMember.offset.ToInt32();
                //string name = Marshal.PtrToStringAnsi(dbMember.name);
                //offsetList.Add(new KeyValuePair<string, int>(name, offset));
            }
        }
        
        public virtual void ReaderCopy(IntPtr samples, IntPtr info)
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
                    sampleDataArray = SampleReaderAlloc(samplesToRead);
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
                    CopyOut(dataSamples[index].data, ref sampleObj, 0);
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
            IntPtr strPtr = Database.c.stringNew(basePtr, from);
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
        
    }
}
