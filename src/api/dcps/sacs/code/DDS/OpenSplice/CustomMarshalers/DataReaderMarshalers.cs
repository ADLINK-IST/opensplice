/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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

//using System;
//using System.Runtime.InteropServices;
//using DDS.OpenSplice;
//
//namespace DDS.OpenSplice.CustomMarshalers
//{
//    public class DataReaderMarshaler : IDisposable
//    {
//        private GCHandle dataValueHandle;
//        private GCHandle sampleInfoHandle;
//
//        public IntPtr dataValuesPtr, dataValuesPtrCache;
//        public IntPtr sampleInfosPtr, sampleInfosPtrCache;
//
//        public DataReaderMarshaler(object[] dataValues, SampleInfo[] sampleInfos, ref int maxSamples, ref ReturnCode result)
//        {
//            dataValueHandle = GCHandle.Alloc(dataValues, GCHandleType.Normal);
//            dataValuesPtr = GCHandle.ToIntPtr(dataValueHandle);
//            dataValuesPtrCache = dataValuesPtr;
//
//            sampleInfoHandle = GCHandle.Alloc(sampleInfos, GCHandleType.Normal);
//            sampleInfosPtr = GCHandle.ToIntPtr(sampleInfoHandle);
//            sampleInfosPtrCache = sampleInfosPtr;
//
//            result = validateParameters(dataValues, sampleInfos, ref maxSamples);
//        }
//
//        public static ReturnCode validateParameters(object[] dataValues, SampleInfo[] sampleInfos, ref int maxSamples)
//        {
//            // Determine current length.
//            int length = (dataValues == null ? 0 : dataValues.Length);
//
//            // 1st Check: arrays should both be null, or have equal length.
//            if (dataValues != null && sampleInfos != null && dataValues.Length != sampleInfos.Length)
//                return ReturnCode.PreconditionNotMet;
//
//            // 2nd Check: maxSamples <= length.
//            if  (length > 0 && maxSamples > length)
//                return ReturnCode.PreconditionNotMet;
//
//            // 3rd Check: In case maxSamples == Length.Unlimited, make maxSamples equal to available length.
//            if (maxSamples == Length.Unlimited && length > 0)
//                maxSamples = length;
//
//            return ReturnCode.Ok;
//        }
//
//        public void CopyOut(ref object[] dataValues, ref SampleInfo[] sampleInfos)
//        {
//            if (dataValueHandle.Target != dataValues)
//            {
//                dataValues = dataValueHandle.Target as object[];
//            }
//
//            if (sampleInfoHandle.Target != sampleInfos)
//            {
//                sampleInfos = sampleInfoHandle.Target as SampleInfo[];
//            }
//        }
//
//
//        public void Dispose()
//        {
//            dataValueHandle.Free();
//            sampleInfoHandle.Free();
//        }
//    }
//}
