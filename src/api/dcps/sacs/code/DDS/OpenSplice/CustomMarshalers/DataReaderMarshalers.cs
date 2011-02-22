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
using DDS.OpenSplice;

namespace DDS.OpenSplice.CustomMarshalers
{
    public class DataReaderMarshaler : IDisposable
    {
        private GCHandle dataValueHandle;
        private GCHandle sampleInfoHandle;

        public IntPtr dataValuesPtr, dataValuesPtrCache;
        public IntPtr sampleInfosPtr, sampleInfosPtrCache;

        public DataReaderMarshaler(object[] dataValues, SampleInfo[] sampleInfos, ref int maxSamples, ref ReturnCode result)
        {
            dataValueHandle = GCHandle.Alloc(dataValues, GCHandleType.Normal);
            dataValuesPtr = GCHandle.ToIntPtr(dataValueHandle);
            dataValuesPtrCache = dataValuesPtr;

            sampleInfoHandle = GCHandle.Alloc(sampleInfos, GCHandleType.Normal);
            sampleInfosPtr = GCHandle.ToIntPtr(sampleInfoHandle);
            sampleInfosPtrCache = sampleInfosPtr;

            result = validateParameters(dataValues, sampleInfos, ref maxSamples);
        }

        public static ReturnCode validateParameters(object[] dataValues, SampleInfo[] sampleInfos, ref int maxSamples)
        {
            // Determine current length.
            int length = (dataValues == null ? 0 : dataValues.Length);

            // 1st Check: arrays should both be null, or have equal length.
            if (dataValues != null && sampleInfos != null && dataValues.Length != sampleInfos.Length)
                return ReturnCode.PreconditionNotMet;

            // 2nd Check: maxSamples <= length.
            if  (length > 0 && maxSamples > length)
                return ReturnCode.PreconditionNotMet;

            // 3rd Check: In case maxSamples == Length.Unlimited, make maxSamples equal to available length.
            if (maxSamples == Length.Unlimited && length > 0)
                maxSamples = length;

            return ReturnCode.Ok;
        }

        public void CopyOut(ref object[] dataValues, ref SampleInfo[] sampleInfos)
        {
            if (dataValueHandle.Target != dataValues)
            {
                dataValues = dataValueHandle.Target as object[];
            }

            if (sampleInfoHandle.Target != sampleInfos)
            {
                sampleInfos = sampleInfoHandle.Target as SampleInfo[];
            }
        }


        public void Dispose()
        {
            dataValueHandle.Free();
            sampleInfoHandle.Free();
        }
    }
}