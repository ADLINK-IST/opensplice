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
using System.Text;
using DDS.OpenSplice;

namespace DDS.OpenSplice.CustomMarshalers
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
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void SampleCopyOutDelegate(IntPtr from, ref object to, int offset);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void SampleReaderCopyDelegate(ref Gapi.gapi_Seq samples, ref Gapi.gapi_readerInfo info);

    public abstract class DatabaseMarshaler : BaseMarshaler
    {
        private static readonly Type dataSampleType = typeof(Gapi.gapi_dataSample);
        private static readonly int dataSampleSize = Marshal.SizeOf(dataSampleType);
        private static readonly int offset_data = (int)Marshal.OffsetOf(dataSampleType, "data");
        private static readonly int offset_sampleInfo = (int)Marshal.OffsetOf(dataSampleType, "sampleInfo");

        public static Dictionary<KeyValuePair<IntPtr, Type>, DatabaseMarshaler> typeMarshalers = 
                new Dictionary<KeyValuePair<IntPtr, Type>, DatabaseMarshaler>();
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
        
        public Delegate CopyOutDelegate 
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

        public DatabaseMarshaler()
        {
            copyInDelegate = CopyIn;
            copyOutDelegate = CopyOut;
            readerCopyDelegate = ReaderCopy;
        }

        public static DatabaseMarshaler Create(
                IntPtr participant,
                IntPtr metaData,
                Type t, 
                IMarshalerTypeGenerator generator)
        {
            DatabaseMarshaler marshaler;
            
            // Check if a Marshaler for this type already exists, and if not, create it.
            if (!typeMarshalers.TryGetValue(new KeyValuePair<IntPtr, Type>(participant, t), out marshaler))
            {
                // Delegate creation of the marshaler instance to the MarshalerGenerator.
                marshaler = generator.CreateMarshaler(participant, metaData, t);

                // Add the new marshaler to the list of known marshalers.
                typeMarshalers.Add(new KeyValuePair<IntPtr, Type>(participant, t), marshaler);
            }
            return marshaler;
        }
        
        public static DatabaseMarshaler GetMarshaler(
                IntPtr participant,
                Type t)
        {
            DatabaseMarshaler marshaler;
            
            // Check if a Marshaler for this type already exists, and if so return it.
            typeMarshalers.TryGetValue(new KeyValuePair<IntPtr, Type>(participant, t), out marshaler);
            return marshaler;
        }
        
        public virtual void initObjectSeq(object[] src, object[] target)
        {
            if (src != null)
            {
                int nrElements = Math.Min(target.Length, src.Length);
                for (int i = 0; i < nrElements; i++)
                {
                    target[i] = src[i];
                }
            }
        }   
        
        public virtual void ReaderCopy(ref Gapi.gapi_Seq samples, ref Gapi.gapi_readerInfo readerInfo)
        {
            int samplesToRead = (int) samples._length;
            IntPtr dataSampleBuf = samples._buffer;
            
            //if (samples != IntPtr.Zero)
            //{
                //DataSample[] dataSamples;
                //OpenSplice.CustomMarshalers.SequenceDataSampleMarshaler.CopyOut(samples, out dataSamples, 0);

                //OpenSplice.Gapi.gapi_readerInfo readerInfo;
                //CustomMarshalers.ReaderInfoMarshaler.CopyOut(info, out readerInfo);

                // TODO: We don't understand why num_samples is zero all the time
                //readerInfo.num_samples = dataSamples.Length;

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

                //if (readerInfo.max_samples == Length.Unlimited)
                //{
                //    samplesToRead = (sampleDataArray == null || sampleDataArray.Length == 0) ? 
                //            samplesToRead : Math.Min(sampleDataArray.Length, samplesToRead);
                //}
                //else
                //{
                //    samplesToRead = Math.Min(readerInfo.max_samples, samplesToRead);
                //}
            //}

            if (sampleDataArray == null || sampleDataArray.Length != samplesToRead)
            {
                // Initialize the arrays using recycled elements of the pre-allocated ones.
                object[] targetData = SampleReaderAlloc(samplesToRead);
                initObjectSeq(sampleDataArray, targetData);
                sampleDataArray = targetData;
                SampleInfo[] targetSampleInfo = new SampleInfo[samplesToRead];
                initObjectSeq(sampleInfoArray, targetSampleInfo);
                sampleInfoArray = targetSampleInfo;

                // Write the new Data Sequence pointer back into C. 
                GCHandle tmpGCHandle = GCHandle.Alloc(sampleDataArray, GCHandleType.Normal);
                IntPtr tmpPtr = GCHandle.ToIntPtr(tmpGCHandle);
                Marshal.WriteIntPtr(readerInfo.data_buffer, tmpPtr);

                // Write the new SampleInfo Sequence pointer back into C. 
                tmpGCHandle = GCHandle.Alloc(sampleInfoArray, GCHandleType.Normal);
                tmpPtr = GCHandle.ToIntPtr(tmpGCHandle);
                Marshal.WriteIntPtr(readerInfo.info_buffer, tmpPtr);
            }

            // copy the data into our sampleDataArray and sampleInfoArray
            IntPtr samplePtr;
            object sampleObj;
            int cursor = 0;
            for (int i = 0; i < samplesToRead; i++)
            {
                //copyOutDelegate(dataSamples[i].data, ref sampleObj);
                samplePtr = ReadIntPtr(dataSampleBuf, cursor);
                sampleObj = sampleDataArray[i];
                CopyOut(samplePtr, ref sampleObj, cursor + offset_data);
                sampleDataArray[i] = sampleObj;

                // copy the sample info
                //sampleInfoArray[i] = dataSamples[i].sampleInfo;
                SampleInfoMarshaler.CopyOut(dataSampleBuf, ref sampleInfoArray[i], 
                        cursor + offset_sampleInfo);
                cursor += dataSampleSize; 
            }
        }
    }
}
